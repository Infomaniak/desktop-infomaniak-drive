#! /bin/bash

set -xe

mkdir -p /app
mkdir -p /build

#Set Qt-5.12
export QT_BASE_DIR=/opt/qt512
export QTDIR=$QT_BASE_DIR
export PATH=$QT_BASE_DIR/bin:$PATH
export LD_LIBRARY_PATH=$QT_BASE_DIR/lib/x86_64-linux-gnu:$QT_BASE_DIR/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$QT_BASE_DIR/lib/pkgconfig:$PKG_CONFIG_PATH

#set defaults
export SUFFIX=${DRONE_PULL_REQUEST:=master}
if [ $SUFFIX != "master" ]; then
    SUFFIX="PR-$SUFFIX"
fi

apt update
apt install --yes --no-install-recommends libkf5config-dev kio-dev extra-cmake-modules python-nautilus libsecret-1-dev libgnome-keyring-dev

#QtKeyChain 0.9.1
cd /build
git clone https://github.com/frankosterfeld/qtkeychain.git
cd qtkeychain
git checkout v0.9.1
mkdir build
cd build
cmake -D CMAKE_INSTALL_PREFIX=/usr ../
make -j4
make DESTDIR=/app install

#Build client
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

cmake -D CMAKE_INSTALL_PREFIX=/usr \
    -DNO_SHIBBOLETH=1 \
    -DUNIT_TESTING=0 \
    -DQTKEYCHAIN_LIBRARY=/app/usr/lib/x86_64-linux-gnu/libqt5keychain.so \
    -DQTKEYCHAIN_INCLUDE_DIR=/app/usr/include/qt5keychain/ \
    -DMIRALL_VERSION_SUFFIX=$SUFFIX \
    -DOEM_THEME_DIR="/src/infomaniak" \
    "${CMAKE_PARAMS[@]}" \
    $DRONE_WORKSPACE
#    -DMIRALL_VERSION_BUILD=$DRONE_BUILD_NUMBER \
make -j4
make DESTDIR=/app install

# Move stuff around
cd /app

mkdir -p ./usr/lib/x86_64-linux-gnu/plugins
mv ./usr/lib/x86_64-linux-gnu/kDrive/plugins/* ./usr/lib/x86_64-linux-gnu/plugins
rmdir ./usr/lib/x86_64-linux-gnu/kDrive/plugins
mv ./usr/lib/x86_64-linux-gnu/* ./usr/lib/
rm -rf ./usr/lib/kDrive
rm -rf ./usr/lib/cmake
rm -rf ./usr/include
rm -rf ./usr/mkspecs
rm -rf ./usr/lib/x86_64-linux-gnu/

# Don't bundle kDrivecmd as we don't run it anyway
rm -rf ./usr/bin/kDrivecmd

# Don't bundle the explorer extentions as we can't do anything with them in the AppImage
rm -rf ./usr/share/caja-python/
rm -rf ./usr/share/nautilus-python/
rm -rf ./usr/share/nemo-python/

# Move sync exlucde to right location
mv ./etc/kDrive/sync-exclude.lst ./usr/bin/
rm -rf ./etc

# sed -i -e 's|Icon=nextcloud|Icon=Nextcloud|g' usr/share/applications/kDrive.desktop # Bug in desktop file?
cp ./usr/share/icons/hicolor/512x512/apps/infomaniak.png . # Workaround for linuxeployqt bug, FIXME


# Because distros need to get their shit together
cp -R /lib/x86_64-linux-gnu/libssl.so* ./usr/lib/
cp -R /lib/x86_64-linux-gnu/libcrypto.so* ./usr/lib/
cp -P /usr/local/lib/libssl.so* ./usr/lib/
cp -P /usr/local/lib/libcrypto.so* ./usr/lib/

# NSS fun
cp -P -r /usr/lib/x86_64-linux-gnu/nss ./usr/lib/

# Use linuxdeployqt to deploy
cd /build
wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt*.AppImage
./linuxdeployqt-continuous-x86_64.AppImage --appimage-extract
rm ./linuxdeployqt-continuous-x86_64.AppImage
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/app/usr/lib/
./squashfs-root/AppRun /app/usr/share/applications/kDrive.desktop -bundle-non-qt-libs

# Set origin
./squashfs-root/usr/bin/patchelf --set-rpath '$ORIGIN/' /app/usr/lib/libkDrivesync.so.0

# Build AppImage
./squashfs-root/AppRun /app/usr/share/applications/kDrive.desktop -appimage

rm -rf ./squashfs-root
mv Infomaniak_Drive*.AppImage /install/kDrive-${SUFFIX}-${DRONE_COMMIT}-x86_64.AppImage
