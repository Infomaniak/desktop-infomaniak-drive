#! /bin/bash

set -xe

mkdir -p /app
mkdir -p /build

# Set Qt-5.12
export QT_BASE_DIR=/opt/qt5.12.10
export QTDIR=$QT_BASE_DIR
export PATH=$QT_BASE_DIR/bin:$PATH
export LD_LIBRARY_PATH=$QT_BASE_DIR/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$QT_BASE_DIR/lib/pkgconfig:$PKG_CONFIG_PATH

# Set defaults
export SUFFIX=${DRONE_PULL_REQUEST:=master}
if [ $SUFFIX != "master" ]; then
    SUFFIX="PR-$SUFFIX"
fi

# Build QtKeyChain 0.12.0
cd /build
git clone https://github.com/frankosterfeld/qtkeychain.git
cd qtkeychain
git checkout v0.12.0
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=$QT_BASE_DIR \
    -DCMAKE_INSTALL_PREFIX=/usr \
    ../
make -j4
make DESTDIR=/app install
rm -Rf /build/qtkeychain

# Build client
cd /build
mkdir -p client
cd client

CMAKE_PARAMS=()

if [ -n "$APPLICATION_SERVER_URL" ]; then
	CMAKE_PARAMS+=(-DAPPLICATION_SERVER_URL="$APPLICATION_SERVER_URL")
fi

if [ -n "$MIRALL_VERSION_BUILD" ]; then
	CMAKE_PARAMS+=(-DMIRALL_VERSION_BUILD="$MIRALL_VERSION_BUILD")
fi

cmake -DCMAKE_PREFIX_PATH=$QT_BASE_DIR \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DNO_SHIBBOLETH=1 \
    -DBUILD_TESTING=0 \
    -DQTKEYCHAIN_LIBRARY=/app/usr/lib/x86_64-linux-gnu/libqt5keychain.so \
    -DQTKEYCHAIN_INCLUDE_DIR=/app/usr/include/qt5keychain/ \
    -DMIRALL_VERSION_SUFFIX=$SUFFIX \
    -DOEM_THEME_DIR="/src/infomaniak" \
    -DWITH_CRASHREPORTER=1 \
    "${CMAKE_PARAMS[@]}" \
    $DRONE_WORKSPACE
#    -DMIRALL_VERSION_BUILD=$DRONE_BUILD_NUMBER \
make -j4
make DESTDIR=/app install

# Move stuff around
cd /app

mkdir -p ./usr/plugins
mv ./usr/lib/x86_64-linux-gnu/kDrive/plugins/* ./usr/plugins/
mv ./usr/lib/x86_64-linux-gnu/plugins/* ./usr/plugins/
mv ./usr/lib/x86_64-linux-gnu/* ./usr/lib/
rm -rf ./usr/lib/x86_64-linux-gnu/
rm -rf ./usr/lib/kDrive
rm -rf ./usr/lib/cmake
rm -rf ./usr/include
rm -rf ./usr/mkspecs

# Don't bundle kDrivecmd as we don't run it anyway
rm -rf ./usr/bin/kDrivecmd

# Move file managers plugins to install directory
# Nautilus
cp -P -r ./usr/share/nautilus-python/ /install/
rm -rf ./usr/share/nautilus-python
# Caja
cp -P -r ./usr/share/caja-python/ /install/
rm -rf ./usr/share/caja-python
# Nemo
cp -P -r ./usr/share/nemo-python/ /install/
rm -rf ./usr/share/nemo-python
# Dolphin
cp -P -r ./usr/share/kservices5/ /install/
rm -rf ./usr/share/kservices5
mkdir -p /install/dolphin/usr/plugins
mv ./usr/plugins/kDrivedolphinactionplugin.so /install/dolphin/usr/plugins/
cp -P -r ./usr/plugins/kf5 /install/dolphin/usr/plugins/
rm -rf ./usr/plugins/kf5
mkdir -p /install/dolphin/usr/lib
mv ./usr/lib/libkDrivedolphinpluginhelper.so /install/dolphin/usr/lib/

# Move sync exclude to right location
mv ./etc/kDrive/sync-exclude.lst ./usr/bin/
rm -rf ./etc

# sed -i -e 's|Icon=nextcloud|Icon=Nextcloud|g' usr/share/applications/kDrive.desktop # Bug in desktop file?
cp ./usr/share/icons/hicolor/512x512/apps/kdrive-win.png . # Workaround for linuxeployqt bug, FIXME

# Because distros need to get their shit together
cp -P /usr/local/lib/libssl.so* ./usr/lib/
cp -P /usr/local/lib/libcrypto.so* ./usr/lib/

# NSS fun
cp -P -r /usr/lib/x86_64-linux-gnu/nss ./usr/lib/

# Use linuxdeployqt to deploy
cd /build
wget --no-check-certificate -c "https://github.com/probonopd/linuxdeployqt/releases/download/6/linuxdeployqt-6-x86_64.AppImage"
chmod a+x linuxdeployqt*.AppImage
./linuxdeployqt-6-x86_64.AppImage --appimage-extract
rm ./linuxdeployqt-6-x86_64.AppImage
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/app/usr/lib/
./squashfs-root/AppRun /app/usr/share/applications/kDrive.desktop -bundle-non-qt-libs -extra-plugins=iconengines,imageformats -unsupported-allow-new-glibc

# Set origin
./squashfs-root/usr/bin/patchelf --set-rpath '$ORIGIN/' /app/usr/lib/libkDrivesync.so.0
./squashfs-root/usr/bin/patchelf --set-rpath '$ORIGIN/../lib/' /app/usr/bin/kDrive_crash_reporter

# Build AppImage
./squashfs-root/AppRun /app/usr/share/applications/kDrive.desktop -appimage -unsupported-allow-new-glibc

rm -rf ./squashfs-root
mv kDrive*.AppImage /install/kDrive-${SUFFIX}-${DRONE_COMMIT}-x86_64.AppImage
