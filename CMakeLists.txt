cmake_minimum_required(VERSION 3.1)

project(maliit-inputcontext-gtk VERSION 0.99.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(ENABLE_X11 "Compile with X11 support" OFF)
option(ENABLE_GTK2 "Build input method for Gtk+ 2" ON)
option(ENABLE_GTK3 "Build input method for Gtk+ 3" ON)

if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

add_definitions(-DG_LOG_DOMAIN="Maliit")

if(ENABLE_X11)
    add_definitions(-DHAVE_X11)
endif()

find_package(Qt6Gui REQUIRED)
find_package(MaliitGLib REQUIRED)

if(ENABLE_GTK2)
    find_package(GTK2 REQUIRED)

    set(SOURCE_FILES
        gtk-input-context/client-gtk/client-imcontext-gtk.c
        gtk-input-context/client-gtk/client-imcontext-gtk.h
        gtk-input-context/client-gtk/debug.c
        gtk-input-context/client-gtk/debug.h
        gtk-input-context/client-gtk/gtk-imcontext-plugin.c
        gtk-input-context/client-gtk/qt-gtk-translate.cpp
        gtk-input-context/client-gtk/qt-gtk-translate.h
        gtk-input-context/client-gtk/qt-keysym-map.cpp
        gtk-input-context/client-gtk/qt-keysym-map.h)

    add_library(im-maliit2 MODULE ${SOURCE_FILES})
    target_link_libraries(im-maliit2 PRIVATE Gtk2::Gtk Qt6::Gui Maliit::GLib)
    set_property(TARGET im-maliit2 PROPERTY OUTPUT_NAME im-maliit)
    set_property(TARGET im-maliit2 PROPERTY PREFIX "")
    set_property(TARGET im-maliit2 PROPERTY LIBRARY_OUTPUT_DIRECTORY gtk-2.0)
    install(TARGETS im-maliit2
            LIBRARY DESTINATION ${LIB_INSTALL_DIR}/gtk-2.0/${GTK2_BINARY_VERSION}/immodules)
endif()

if(ENABLE_GTK3)
    find_package(GTK3 REQUIRED)

    set(SOURCE_FILES
            gtk-input-context/client-gtk/client-imcontext-gtk.c
            gtk-input-context/client-gtk/client-imcontext-gtk.h
            gtk-input-context/client-gtk/debug.c
            gtk-input-context/client-gtk/debug.h
            gtk-input-context/client-gtk/gtk-imcontext-plugin.c
            gtk-input-context/client-gtk/qt-gtk-translate.cpp
            gtk-input-context/client-gtk/qt-gtk-translate.h
            gtk-input-context/client-gtk/qt-keysym-map.cpp
            gtk-input-context/client-gtk/qt-keysym-map.h)

    add_library(im-maliit3 MODULE ${SOURCE_FILES})
    target_link_libraries(im-maliit3 PRIVATE Gtk3::Gtk Qt6::Gui Maliit::GLib)
    set_property(TARGET im-maliit3 PROPERTY OUTPUT_NAME im-maliit)
    set_property(TARGET im-maliit3 PROPERTY PREFIX "")
    set_property(TARGET im-maliit3 PROPERTY LIBRARY_OUTPUT_DIRECTORY gtk-3.0)
    install(TARGETS im-maliit3
            LIBRARY DESTINATION ${LIB_INSTALL_DIR}/gtk-3.0/${GTK3_BINARY_VERSION}/immodules)
endif()
