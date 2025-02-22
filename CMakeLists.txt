# Lagrange - A Beautiful Gemini Client
# Copyright: 2020-2024 Jaakko Keränen <jaakko.keranen@iki.fi>
#
# Notes:
# - Required dependencies: SDL 2, OpenSSL 1.1.1, libpcre,
#   GNU libunistring, zlib.
# - the_Foundation is built as a static library from 'lib/the_Foundation',
#   if it exists in that location. The Git repository has it as a submodule.
# - Windows builds require MSYS2. In theory, Clang could be set up on
#   Windows for compiling everything, but the_Foundation still lacks
#   native Win32 implementations for the Socket and Process classes.
# - Windows builds should use the SDL 2 library precompiled for native
#   Windows (MSVC variant) instead of the version from MSYS2 (get it from
#   https://libsdl.org/). To make configuration easier, consider writing
#   for your personal use a pkg-config sdl2.pc file that uses the Windows
#   version of the library.

cmake_minimum_required (VERSION 3.9)

project (Lagrange
    VERSION 1.18.0
    DESCRIPTION "A Beautiful Gemini Client"
    LANGUAGES C
)
set (COPYRIGHT_YEAR 2024)
if (IOS)
    set (PROJECT_VERSION 1.17)
    set (IOS_BUNDLE_VERSION 13)
    set (IOS_BUILD_DATE "2024-07-06")
endif ()
if (ANDROID)
    set (PROJECT_VERSION 1.17)
    set (ANDROID_BUILD_VERSION b30)  # remember to update Gradle, AndroidManifest.xml
    set (ANDROID_BUILD_DATE "2024-07-07")
endif ()

# Load modules from the source tree
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Defaults that depend on environment.
set (DEFAULT_RESIZE_DRAW ON)
if (HAIKU OR ANDROID)
    set (DEFAULT_RESIZE_DRAW OFF)
endif ()
set (DEFAULT_IDLE_SLEEP OFF)
if (IOS)
    set (DEFAULT_IDLE_SLEEP ON)
endif ()

# Build configuration.
option (ENABLE_CUSTOM_FRAME     "Draw a custom window frame (Windows)" OFF)
option (ENABLE_DOWNLOAD_EDIT    "Allow changing the Downloads directory" ON)
option (ENABLE_GUI              "Build the GUI application" ON)
option (ENABLE_IDLE_SLEEP       "While idle, sleep in the main thread instead of waiting for events" ${DEFAULT_IDLE_SLEEP})
option (ENABLE_IPC              "Use IPC to communicate between running instances" ON)
option (ENABLE_KERNING          "Enable kerning in font renderer (slower)" ON)
option (ENABLE_MAC_MENUS        "Use native context menus (macOS)" ON)
option (ENABLE_MOBILE_PHONE     "Use the phone mobile UI design instead of desktop UI" OFF)
option (ENABLE_MOBILE_TABLET    "Use the tablet mobile UI design instead of desktop UI" OFF)
option (ENABLE_POPUP_MENUS      "Use popup windows for context menus (if OFF, menus are confined inside main window)" ON)
option (ENABLE_RELATIVE_EMBED   "Resources should always be found via relative path" OFF)
option (ENABLE_RESIZE_DRAW      "Force window to redraw during resizing" ${DEFAULT_RESIZE_DRAW})
option (ENABLE_STATIC           "Prefer linking dependencies statically" OFF)
option (ENABLE_TUI              "Build clagrange (TUI based on ncurses)" OFF)
option (ENABLE_WINDOWPOS_FIX    "Set position after showing window (workaround for SDL bug)" OFF)
option (ENABLE_X11_SWRENDER     "Use software rendering (X11)" OFF)
option (ENABLE_X11_XLIB         "Use Xlib directly, if available (X11)" ON)
set    (CACERT_PEM_PATH         "" CACHE FILEPATH "Root CA cacert.pem file to include as a built-in resource")
set    (RESOURCES_LGR_PATH      "" CACHE STRING "Additional location to check when searching 'resources.lgr'")

# Optional dependencies.
option (ENABLE_FRIBIDI          "Use the GNU FriBidi library for bidirectional text" ON)
option (ENABLE_FRIBIDI_BUILD    "Build the GNU FriBidi library (if OFF, try pkg-config)" OFF)
option (ENABLE_HARFBUZZ         "Use the HarfBuzz library to shape text" ON)
option (ENABLE_HARFBUZZ_MINIMAL "Build the HarfBuzz library with minimal dependencies (if OFF, try pkg-config)" OFF)
option (ENABLE_MPG123           "Use mpg123 for decoding MPEG audio" ON)
option (ENABLE_SPARKLE          "Use Sparkle for automatic updates (macOS)" OFF)
option (ENABLE_WEBP             "Use libwebp to decode .webp images (via pkg-config)" ON)
option (ENABLE_WINSPARKLE       "Use WinSparkle for automatic updates (Windows)" OFF)
option (ENABLE_OPUS             "Use llibopusfile for decoding Opus audio (via pkg-config)" ON)

include (BuildType)
include (Resources)
include (Depends)
include (CheckCCompilerFlag)

check_c_compiler_flag (-Werror=incompatible-pointer-types CFLAG_INCOMPATIBLE_POINTER_TYPES)

# Package resources.
message (STATUS "Preparing resources...")
set (RESOURCES
    res/about/about.gmi
    res/about/lagrange.gmi
    res/about/license.gmi
    res/lang/cs.bin
    res/lang/de.bin
    res/lang/en.bin
    res/lang/eo.bin
    res/lang/es.bin
    res/lang/es_MX.bin
    res/lang/eu.bin
    res/lang/fi.bin
    res/lang/fr.bin
    res/lang/gl.bin
    res/lang/hu.bin
    res/lang/ia.bin
    res/lang/ie.bin
    res/lang/isv.bin
    res/lang/it.bin
    res/lang/ja.bin
    res/lang/nl.bin
    res/lang/pl.bin
    res/lang/ru.bin
    res/lang/sk.bin
    res/lang/sr.bin
    res/lang/tok.bin
    res/lang/tr.bin
    res/lang/uk.bin
    res/lang/zh_Hans.bin
    res/lang/zh_Hant.bin
    res/logo.png
    res/shadow.png
    res/fontpack.ini
    res/macos-system-fonts.ini
)
if (EXISTS ${CACERT_PEM_PATH})
    message (STATUS "  ${CACERT_PEM_PATH}")
    list (APPEND RESOURCES ${CACERT_PEM_PATH})
endif ()
file (GLOB FONTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} res/fonts/*)
list (APPEND RESOURCES ${FONTS})
if (IOS)
    list (APPEND RESOURCES
        res/about/ios-help.gmi
        res/about/ios-version.gmi
    )
elseif (ANDROID)
    list (APPEND RESOURCES
        res/about/android-help.gmi
        res/about/android-version.gmi
    )
else ()
    list (APPEND RESOURCES
        res/about/help.gmi
        res/about/version-0.13.gmi
        res/about/version-1.5.gmi
        res/about/version-1.10.gmi
        res/about/version.gmi
        res/arg-help.txt
    )
endif ()
if ((UNIX AND NOT APPLE) OR MSYS)
    list (APPEND RESOURCES res/lagrange-64.png)
endif ()
set (EMB_BIN ${CMAKE_CURRENT_BINARY_DIR}/resources.lgr)
make_resources (${EMB_BIN} ${RESOURCES})
set_source_files_properties (${EMB_BIN} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
if (IOS)
    set (EMB_FONTS
        res/fonts/Roboto-Regular.ttf
        res/fonts/IosevkaTerm-Extended.ttf
    )
    set_source_files_properties (${EMB_FONTS}
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources
    )
endif ()

# Source files.
set (SOURCES
    src/main.c
    src/app.c
    src/app.h
    src/bookmarks.c
    src/bookmarks.h
    src/defs.h
    src/export.c
    src/export.h
    src/feeds.c
    src/feeds.h
    src/fontpack.c
    src/fontpack.h
    src/gempub.c
    src/gempub.h
    src/gmcerts.c
    src/gmcerts.h
    src/gmdocument.c
    src/gmdocument.h
    src/gmrequest.c
    src/gmrequest.h
    src/gmtypesetter.c
    src/gmtypesetter.h
    src/gmutil.c
    src/gmutil.h
    src/gopher.c
    src/gopher.h
    src/history.c
    src/history.h
    src/lang.c
    src/lang.h
    src/lookup.c
    src/lookup.h
    src/media.c
    src/media.h
    src/mimehooks.c
    src/mimehooks.h
    src/misfin.c
    src/misfin.h
    src/periodic.c
    src/periodic.h
    src/prefs.c
    src/prefs.h
    src/resources.c
    src/resources.h
    src/sitespec.c
    src/sitespec.h
    src/snippets.c
    src/snippets.h
    src/stb_image.h
    src/stb_image_resize2.h
    src/stb_truetype.h
    src/updater.h
    src/visited.c
    src/visited.h
    # User interface:
    src/ui/banner.c
    src/ui/banner.h
    src/ui/bindingswidget.c
    src/ui/bindingswidget.h
    src/ui/certimportwidget.c
    src/ui/certimportwidget.h
    src/ui/certlistwidget.c
    src/ui/certlistwidget.h
    src/ui/color.c
    src/ui/color.h
    src/ui/command.c
    src/ui/command.h
    src/ui/documentview.c
    src/ui/documentview.h
    src/ui/documentwidget.c
    src/ui/documentwidget.h
    src/ui/indicatorwidget.c
    src/ui/indicatorwidget.h
    src/ui/font.c
    src/ui/font.h
    src/ui/linkinfo.c
    src/ui/linkinfo.h
    src/ui/listwidget.c
    src/ui/listwidget.h
    src/ui/lookupwidget.c
    src/ui/lookupwidget.h
    src/ui/keys.c
    src/ui/keys.h
    src/ui/metrics.c
    src/ui/metrics.h
    src/ui/paint.c
    src/ui/paint.h
    src/ui/root.c
    src/ui/root.h
    src/ui/mediaui.c
    src/ui/mediaui.h
    src/ui/mobile.c
    src/ui/mobile.h
    src/ui/scrollwidget.c
    src/ui/scrollwidget.h
    src/ui/sidebarwidget.c
    src/ui/sidebarwidget.h
    src/ui/snippetwidget.c
    src/ui/snippetwidget.h
    src/ui/text.c
    src/ui/text.h
    src/ui/touch.c
    src/ui/touch.h
    src/ui/translation.c
    src/ui/translation.h
    src/ui/uploadwidget.c
    src/ui/uploadwidget.h
    src/ui/util.c
    src/ui/util.h
    src/ui/visbuf.c
    src/ui/visbuf.h
    src/ui/window.c
    src/ui/window.h
    # Widgets:
    src/ui/widget.c
    src/ui/widget.h
    src/ui/inputwidget.c
    src/ui/inputwidget.h
    src/ui/labelwidget.c
    src/ui/labelwidget.h
    # Resources:
    res/about/about.gmi
    res/about/help.gmi
    res/about/lagrange.gmi
    res/about/license.gmi
    res/about/version.gmi
    ${EMB_BIN}
)

if (IOS OR NOT APPLE) # macos.m has Sparkle updater
    list (APPEND SOURCES src/updater.c)
endif ()
if (ENABLE_IPC)
    list (APPEND SOURCES
        src/ipc.c
        src/ipc.h
    )
endif ()

set (TUI_SOURCES
    ${SOURCES}
    src/ui/text_terminal.c
    src/updater.c
)

set (ENABLE_STB_TRUETYPE YES)
list (APPEND SOURCES
    src/ui/text_stb.c
)
# Audio playback
list (APPEND SOURCES
    src/audio/buf.c
    src/audio/buf.h
    src/audio/player.c
    src/audio/player.h
    src/audio/stb_vorbis.c
)
if (ENABLE_X11_XLIB AND XLIB_FOUND AND NOT APPLE AND NOT MSYS)
    list (APPEND SOURCES
        src/x11.c
        src/x11.h
    )
endif ()
if (ANDROID)
    set (MOBILE 1)
    add_definitions (-DiPlatformAndroidMobile=1)
    list (APPEND SOURCES
        src/android.c
        src/android.h
    )
endif ()
if (IOS)
    set (MOBILE 1)
    add_definitions (-DiPlatformAppleMobile=1)
    list (APPEND SOURCES
        src/ios.m
        src/ios.h
        app/Images.xcassets
        res/LaunchScreen.storyboard
        res/PrivacyInfo.xcprivacy
    )
    set_source_files_properties(
        app/Images.xcassets
        res/LaunchScreen.storyboard
        res/PrivacyInfo.xcprivacy
    PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
elseif (APPLE)
    list (APPEND SOURCES src/macos.m src/macos.h)
    list (APPEND RESOURCES "res/Lagrange.icns")
    set_source_files_properties ("res/Lagrange.icns" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif ()
if (MSYS)
    set (WINRC_FILE_VERSION ${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0)
    set (WINRC_PRODUCT_VERSION ${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},0,0)
    configure_file (res/lagrange.rc.in ${CMAKE_CURRENT_BINARY_DIR}/lagrange.rc NEWLINE_STYLE WIN32)
    list (APPEND SOURCES src/win32.c src/win32.h ${CMAKE_CURRENT_BINARY_DIR}/lagrange.rc)
    list (APPEND TUI_SOURCES src/win32.c src/win32.h)
endif ()
if (MSYS OR (UNIX AND NOT MOBILE AND NOT APPLE))
    add_definitions (-DiPlatformPcDesktop=1)
endif ()
if (MOBILE)
    add_definitions (-DiPlatformMobile=1)
else ()
    add_definitions (-DiPlatformDesktop=1)
    # Forced UI design.
    if (ENABLE_MOBILE_PHONE OR ENABLE_MOBILE_TABLET)
        add_definitions (-DiPlatformMobile=1)
    endif ()
    if (ENABLE_MOBILE_PHONE)
        add_definitions (-DiPlatformMobilePhone=1)
    endif ()
    if (ENABLE_MOBILE_TABLET)
        add_definitions (-DiPlatformMobileTablet=1)
    endif ()
endif ()

macro (set_common_app_properties target)
    set_property (TARGET ${target} PROPERTY C_STANDARD 11)
    if (TARGET ext-deps)
        add_dependencies (${target} ext-deps)
    endif ()
    target_include_directories (${target} PUBLIC
        src
        ${CMAKE_CURRENT_BINARY_DIR}
    )
    target_compile_options (${target} PUBLIC
        -Werror=implicit-function-declaration
        -Wno-deprecated-declarations
    )
    target_compile_definitions (${target} PUBLIC LAGRANGE_APP_VERSION="${PROJECT_VERSION}")
    if (CFLAG_INCOMPATIBLE_POINTER_TYPES)
        target_compile_options (${target} PUBLIC -Werror=incompatible-pointer-types)
    endif ()

    if (APPLE AND NOT IOS)
        if (CMAKE_OSX_DEPLOYMENT_TARGET)
            target_compile_options (${target} PUBLIC -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
            target_link_options (${target} PUBLIC -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
        endif ()
    endif ()

    # Build options that affect both GUI and TUI targets.
    if (ENABLE_DOWNLOAD_EDIT)
        target_compile_definitions (${target} PUBLIC LAGRANGE_ENABLE_DOWNLOAD_EDIT=1)
    endif ()
    if (ENABLE_IDLE_SLEEP)
        target_compile_definitions (${target} PUBLIC LAGRANGE_ENABLE_IDLE_SLEEP=1)
    endif ()
    if (ENABLE_IPC)
        target_compile_definitions (${target} PUBLIC LAGRANGE_ENABLE_IPC=1)
    endif ()

    # Common dependencies.
    target_link_libraries (${target} PUBLIC the_Foundation::the_Foundation)
    if (HAIKU)
        target_link_libraries (${target} PUBLIC network bsd)
    endif ()
endmacro ()

if (ENABLE_GUI)
    if (NOT ANDROID)
        add_executable (app ${SOURCES} ${RESOURCES} ${EMB_FONTS})
    else ()
        # The whole app becomes one shared library, based on this static one.
        add_library (app SHARED ${SOURCES})
    endif ()

    set_common_app_properties (app)
    set_target_properties (app PROPERTIES OUTPUT_NAME lagrange)
    target_compile_options (app PUBLIC
        -DLAGRANGE_ENABLE_AUDIO=1
        -DSTB_VORBIS_NO_STDIO=1
        -DSTB_VORBIS_NO_INTEGER_CONVERSION=1
    )
    if (APPLE AND NOT MOBILE)
        target_compile_definitions (app PUBLIC -DiPlatformAppleDesktop=1)
    endif ()
    if (ENABLE_CUSTOM_FRAME AND MSYS)
        target_compile_definitions (app PUBLIC LAGRANGE_ENABLE_CUSTOM_FRAME=1)
    endif ()
    if (ENABLE_KERNING)
        target_compile_definitions (app PUBLIC LAGRANGE_ENABLE_KERNING=1)
    endif ()
    if (ENABLE_MAC_MENUS)
        target_compile_definitions (app PUBLIC LAGRANGE_ENABLE_MAC_MENUS=1)
    endif ()
    if (ENABLE_POPUP_MENUS)
        target_compile_definitions (app PUBLIC LAGRANGE_ENABLE_POPUP_MENUS=1)
    endif ()
    if (ENABLE_RESIZE_DRAW)
        target_compile_definitions (app PUBLIC LAGRANGE_ENABLE_RESIZE_DRAW=1)
    endif ()
    if (ENABLE_STB_TRUETYPE)
        target_compile_definitions (app PUBLIC LAGRANGE_ENABLE_STB_TRUETYPE=1)
    endif ()
    if (ENABLE_MPG123 AND MPG123_FOUND)
        target_compile_definitions (app PUBLIC LAGRANGE_ENABLE_MPG123=1)
        target_link_libraries (app PUBLIC PkgConfig::MPG123)
    endif ()
    if (ENABLE_OPUS AND OPUSFILE_FOUND)
        target_compile_definitions (app PUBLIC LAGRANGE_ENABLE_OPUS=1)
        target_link_libraries (app PUBLIC PkgConfig::OPUSFILE)
    endif ()
    if (ENABLE_WEBP AND WEBP_FOUND)
        target_compile_definitions (app PUBLIC LAGRANGE_ENABLE_WEBP=1)
        if (TARGET PkgConfig::WEBP)
            target_link_libraries (app PUBLIC PkgConfig::WEBP)
        else ()
            target_link_libraries (app PUBLIC ${WEBP_LIBRARIES})
            target_include_directories (app PUBLIC ${WEBP_INCLUDE_DIRS})
        endif ()
    endif ()
    if (ENABLE_WINDOWPOS_FIX)
        target_compile_definitions (app PUBLIC LAGRANGE_ENABLE_WINDOWPOS_FIX=1)
    endif ()
    if (TARGET x11-lib)
        target_link_libraries (app PUBLIC x11-lib)
    endif ()
    if (ENABLE_X11_SWRENDER)
        target_compile_definitions (app PUBLIC LAGRANGE_ENABLE_X11_SWRENDER=1)
    endif ()
    target_include_directories (app PUBLIC ${SDL2_INCLUDE_DIRS})
    target_compile_options (app PUBLIC ${SDL2_CFLAGS})
    target_link_libraries (app PUBLIC ${SDL2_LDFLAGS})
    if (ENABLE_HARFBUZZ AND HARFBUZZ_FOUND)
        if (TARGET harfbuzz-lib)
            target_link_libraries (app PUBLIC harfbuzz-lib)
        else ()
            target_link_libraries (app PUBLIC ${HARFBUZZ_LIBRARIES})
            target_include_directories (app PUBLIC ${HARFBUZZ_INCLUDE_DIRS})
        endif ()
        target_compile_definitions (app PUBLIC LAGRANGE_ENABLE_HARFBUZZ=1)
    endif ()
    if (ENABLE_FRIBIDI AND FRIBIDI_FOUND)
        if (TARGET fribidi-lib)
            target_link_libraries (app PUBLIC fribidi-lib)
        else ()
            target_link_libraries (app PUBLIC ${FRIBIDI_LIBRARIES})
            target_include_directories (app PUBLIC ${FRIBIDI_INCLUDE_DIRS})
        endif ()
        target_compile_definitions (app PUBLIC LAGRANGE_ENABLE_FRIBIDI=1)
    endif ()
    if (APPLE)
        if (IOS)
            target_link_libraries (app PUBLIC "-framework UIKit")
        else ()
            target_link_libraries (app PUBLIC "-framework AppKit")
        endif ()
        if (ENABLE_SPARKLE)
            target_link_libraries (app PUBLIC sparkle)
        endif ()
        if (SDL2_LIBRARY_DIRS)
            set_property (TARGET app PROPERTY BUILD_RPATH ${SDL2_LIBRARY_DIRS})
        endif ()
        set_target_properties (app PROPERTIES
            OUTPUT_NAME "Lagrange"
            MACOSX_BUNDLE YES
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/res/MacOSXBundleInfo.plist.in"
            MACOSX_BUNDLE_BUNDLE_NAME "Lagrange"
            MACOSX_BUNDLE_INFO_STRING "${PROJECT_VERSION}"
            MACOSX_BUNDLE_ICON_FILE "Lagrange.icns"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
            MACOSX_BUNDLE_GUI_IDENTIFIER "fi.skyjake.lagrange"
            MACOSX_BUNDLE_COPYRIGHT "© ${COPYRIGHT_YEAR} Jaakko Keränen"
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "fi.skyjake.lagrange"
        )
        if (IOS)
            target_compile_definitions (app PUBLIC
                LAGRANGE_IOS_VERSION="${IOS_BUNDLE_VERSION}"
                LAGRANGE_IOS_BUILD_DATE="${IOS_BUILD_DATE}"
            )
            set_target_properties (app PROPERTIES
                MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/res/iOSBundleInfo.plist.in"
                MACOSX_BUNDLE_BUNDLE_VERSION "${IOS_BUNDLE_VERSION}"
                XCODE_ATTRIBUTE_SKIP_INSTALL "NO"
                XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
                XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
                XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
                XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "$(inherited)" # fix missing dSYM issue
                MACOSX_BUNDLE_ICON_FILE "AppIcon"
            )
        endif ()
        if (XCODE_DEVELOPMENT_TEAM)
            set_property (TARGET app PROPERTY
                XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${XCODE_DEVELOPMENT_TEAM}
            )
        endif ()
    endif ()
    if (ANDROID)
        if (CMAKE_SIZEOF_VOID_P EQUAL 4)
            target_compile_options (app PUBLIC -mfpu=neon-vfpv4) # 32-bit stb_image_resize2
        endif ()
        target_compile_definitions (app PUBLIC
            LAGRANGE_ANDROID_VERSION="${ANDROID_BUILD_VERSION}"
            LAGRANGE_ANDROID_BUILD_DATE="${ANDROID_BUILD_DATE}"
        )
    endif ()
    if (MSYS)
        target_link_libraries (app PUBLIC d2d1 uuid dwmapi) # querying DPI
        if (ENABLE_WINSPARKLE)
            target_link_libraries (app PUBLIC winsparkle)
        endif ()
    endif ()
endif ()

if (ENABLE_TUI)
    # TUI is its own target that links with SEALCurses instead of SDL2.
    add_executable (tuiapp ${TUI_SOURCES} ${RESOURCES})
    target_compile_definitions (tuiapp PUBLIC -DiPlatformTerminal=1)
    set_target_properties (tuiapp PROPERTIES OUTPUT_NAME clagrange)
    if (TARGET sealcurses-static)
        target_link_libraries (tuiapp PUBLIC sealcurses-static)
    elseif (TARGET sealcurses)
        target_link_libraries (tuiapp PUBLIC sealcurses)
    else ()
        target_include_directories (tuiapp PUBLIC ${SEALCURSES_INCLUDE_DIRS})
        target_compile_options (tuiapp PUBLIC ${SEALCURSES_CFLAGS})
        target_link_libraries (tuiapp PUBLIC ${SEALCURSES_LDFLAGS})
    endif ()
    set_common_app_properties (tuiapp)
endif ()

# Deployment.
if (MSYS)
    if (TARGET app)
        install (TARGETS app DESTINATION .)
        install (PROGRAMS ${SDL2_LIBDIR}/SDL2.dll DESTINATION .)
    endif ()
    if (TARGET PkgConfig::MPG123)
        install (PROGRAMS ${MPG123_LIBDIR}/../bin/msys-mpg123-0.dll DESTINATION .)
    endif ()
    install (FILES ${EMB_BIN} DESTINATION .)
    install (PROGRAMS res/urlopen.bat DESTINATION .)
    if (INSTALL_THE_FOUNDATION)
        install (PROGRAMS $<TARGET_FILE:the_Foundation::the_Foundation> DESTINATION .)
    endif ()
elseif (HAIKU)
    if (TARGET app)
        install (TARGETS app DESTINATION .)
        target_compile_definitions (app PUBLIC
            LAGRANGE_EMB_BIN="${CMAKE_INSTALL_PREFIX}/resources.lgr")
    endif ()
    install (FILES ${EMB_BIN} DESTINATION .)
elseif (ANDROID)
    file (COPY ${EMB_BIN} DESTINATION ${CMAKE_SOURCE_DIR}/../src/main/assets)
elseif (UNIX AND NOT APPLE)
    include (GNUInstallDirs)
    if (TARGET app)
        set_target_properties (app PROPERTIES
            INSTALL_RPATH_USE_LINK_PATH YES
        )
        set (desktop ${CMAKE_CURRENT_BINARY_DIR}/fi.skyjake.Lagrange.desktop)
        file (WRITE ${desktop} "[Desktop Entry]
Name=Lagrange
GenericName=Gemini Client
Comment=${PROJECT_DESCRIPTION}
Categories=Network;
Exec=${CMAKE_INSTALL_PREFIX}/bin/lagrange %U
Terminal=false
Type=Application
StartupWMClass=lagrange
Icon=fi.skyjake.Lagrange
MimeType=x-scheme-handler/gemini;x-scheme-handler/gopher;x-scheme-handler/spartan;
")
        install (TARGETS app DESTINATION ${CMAKE_INSTALL_BINDIR})
        install (FILES ${desktop} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
        install (FILES res/lagrange-256.png
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
            RENAME fi.skyjake.Lagrange.png
        )
        install (FILES res/fi.skyjake.Lagrange.appdata.xml
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
        )
        if (NOT ENABLE_RELATIVE_EMBED)
            target_compile_definitions (app PUBLIC
                    LAGRANGE_EMB_BIN="${CMAKE_INSTALL_FULL_DATADIR}/lagrange/resources.lgr")
        endif ()
    endif ()
    install (FILES res/lagrange.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
    install (FILES ${EMB_BIN} DESTINATION ${CMAKE_INSTALL_DATADIR}/lagrange)
endif ()

if (TARGET tuiapp)
    if (MSYS)
        install (TARGETS tuiapp DESTINATION .)
    else ()
        if (UNIX AND NOT APPLE)
            include (GNUInstallDirs)
        endif ()
        install (TARGETS tuiapp DESTINATION ${CMAKE_INSTALL_BINDIR})

        set (desktop ${CMAKE_CURRENT_BINARY_DIR}/fi.skyjake.clagrange.desktop)
        file (WRITE ${desktop} "[Desktop Entry]
Name=clagrange
GenericName=Gemini Client
Comment=${PROJECT_DESCRIPTION}
Categories=Network;
Exec=${CMAKE_INSTALL_PREFIX}/bin/clagrange %U
Terminal=true
Type=Application
StartupWMClass=clagrange
Icon=fi.skyjake.clagrange
MimeType=x-scheme-handler/gemini;x-scheme-handler/gopher;x-scheme-handler/spartan;
")
        install (FILES ${desktop} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
        install (FILES res/lagrange-256.png
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
            RENAME fi.skyjake.clagrange.png
        )
    endif ()
    if (RESOURCES_LGR_PATH)
        target_compile_definitions (tuiapp PUBLIC
                LAGRANGE_EMB_BIN="${RESOURCES_LGR_PATH}/resources.lgr")
    elseif (NOT ENABLE_RELATIVE_EMBED)
        target_compile_definitions (tuiapp PUBLIC
                LAGRANGE_EMB_BIN="${CMAKE_INSTALL_FULL_DATADIR}/lagrange/resources.lgr")
    endif ()
endif ()
