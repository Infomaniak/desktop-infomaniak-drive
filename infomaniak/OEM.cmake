set( APPLICATION_NAME       "kDrive" )

# if (NOT DEFINED APPLICATION_SHORTNAME)
#     set(APPLICATION_SHORTNAME "${APPLICATION_NAME}")
# endif()

set( APPLICATION_SHORTNAME  "kDrive" )
set( APPLICATION_EXECUTABLE "kDrive" )
set( APPLICATION_DOMAIN     "infomaniak.com" )
set( APPLICATION_VENDOR     "Infomaniak Network SA" )
set( APPLICATION_UPDATE_URL "https://www.infomaniak.com/drive/update/desktopclient" CACHE STRING "URL for updater" )
#set( APPLICATION_HELP_URL   "https://faq.infomaniak.com/2366" CACHE STRING "URL for the help menu" )
if( APPLE )
    set( APPLICATION_ICON_NAME  "kdrive-mac" )
else()
    set( APPLICATION_ICON_NAME  "kdrive-win" )
endif()
set( APPLICATION_SERVER_URL "https://connect.drive.infomaniak.com" CACHE STRING "URL for the server to use. If entered the server can only connect to this instance" )
set( APPLICATION_VIRTUALFILE_SUFFIX "kdrive" CACHE STRING "Virtual file suffix (not including the .)")
set( APPLICATION_DOWNLOAD_URL "https://drive.infomaniak.com/app/drive/sync" CACHE STRING "App download URL" )
set( APPLICATION_TRASH_URL "https://drive.infomaniak.com/app/drive/%s/trash" CACHE STRING "App trash URL" )
set( APPLICATION_THUMBNAIL_URL "index.php/apps/files/api/v1/thumbnail/%1/%1/%2" CACHE STRING "App thumbnail URL" )

set( LINUX_PACKAGE_SHORTNAME "infomaniakdrive" )

set( THEME_CLASS            "InfomaniakTheme" )
set( APPLICATION_REV_DOMAIN "com.infomaniak.drive.desktopclient" )
set( WIN_SETUP_BITMAP_PATH  "${CMAKE_SOURCE_DIR}/admin/win/nsi" )

# set( MAC_INSTALLER_BACKGROUND_FILE "${CMAKE_SOURCE_DIR}/admin/osx/installer-background.png" CACHE STRING "The MacOSX installer background image")

set( THEME_INCLUDE          "infomaniaktheme.h" )
# set( THEME_INCLUDE          "${OEM_THEME_DIR}/mytheme.h" )
set( APPLICATION_LICENSE    "${OEM_THEME_DIR}/license.txt" )

option( WITH_CRASHREPORTER "Build crashreporter" ON )
set( CRASHREPORTER_SUBMIT_URL "https://www.infomaniak.com/report/drive/crash" CACHE STRING "URL for crash reporter" )
set( CRASHREPORTER_ICON ":/infomaniak.png" )

set( DEBUGREPORTER_SUBMIT_URL "https://www.infomaniak.com/report/drive/logs" CACHE STRING "URL for debug reporter" )

set( LEARNMORE_LITESYNC_URL "https://faq.infomaniak.com/2523" CACHE STRING "URL for Lite Sync FAQ" )
set( LEARNMORE_LITESYNC_COMPATIBILITY_URL "https://faq.infomaniak.com/2523" CACHE STRING "URL for Lite Sync compatibility FAQ" )

set( VFS_DIRECTORY "F:\\Projects\\kdrive-ext-win\\x64\\Release" )
set( VFS_APPX_DIRECTORY "F:\\Projects\\kdrive-ext-win\\FileExplorerExtensionPackage\\AppPackages\\FileExplorerExtensionPackage_${MIRALL_VERSION_MAJOR}.${MIRALL_VERSION_MINOR}.${MIRALL_VERSION_PATCH}.0_Test" )
set( VFS_APPX_BUNDLE "FileExplorerExtensionPackage_${MIRALL_VERSION_MAJOR}.${MIRALL_VERSION_MINOR}.${MIRALL_VERSION_PATCH}.0_x64.msixbundle" )
