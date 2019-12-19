set( APPLICATION_NAME       "Infomaniak Drive" )

# if (NOT DEFINED APPLICATION_SHORTNAME)
#     set(APPLICATION_SHORTNAME "${APPLICATION_NAME}")
# endif()

set( APPLICATION_SHORTNAME  "kDrive" )
set( APPLICATION_EXECUTABLE "kDrive" )
set( APPLICATION_DOMAIN     "infomaniak.com" )
set( APPLICATION_VENDOR     "Infomaniak Network SA" )
set( APPLICATION_UPDATE_URL "https://www.infomaniak.com/drive/update/desktopclient" CACHE STRING "URL for updater" )
#set( APPLICATION_HELP_URL   "https://faq.infomaniak.com/2366" CACHE STRING "URL for the help menu" )
set( APPLICATION_ICON_NAME  "infomaniak" )
set( APPLICATION_SERVER_URL "https://connect.drive.infomaniak.com" CACHE STRING "URL for the server to use. If entered the server can only connect to this instance" )
set( APPLICATION_VIRTUALFILE_SUFFIX "kdrive" CACHE STRING "Virtual file suffix (not including the .)")

set( LINUX_PACKAGE_SHORTNAME "infomaniakdrive" )

set( THEME_CLASS            "InfomaniakTheme" )
set( APPLICATION_REV_DOMAIN "com.infomaniak.drive.desktopclient" )
set( WIN_SETUP_BITMAP_PATH  "${CMAKE_SOURCE_DIR}/admin/win/nsi" )

# set( MAC_INSTALLER_BACKGROUND_FILE "${CMAKE_SOURCE_DIR}/admin/osx/installer-background.png" CACHE STRING "The MacOSX installer background image")

set( THEME_INCLUDE          "infomaniaktheme.h" )
# set( THEME_INCLUDE          "${OEM_THEME_DIR}/mytheme.h" )
set( APPLICATION_LICENSE    "${OEM_THEME_DIR}/license.txt" )

option( WITH_CRASHREPORTER "Build crashreporter" OFF )
#set( CRASHREPORTER_SUBMIT_URL "https://crash-reports.owncloud.com/submit" CACHE STRING "URL for crash reporter" )
#set( CRASHREPORTER_ICON ":/owncloud-icon.png" )
