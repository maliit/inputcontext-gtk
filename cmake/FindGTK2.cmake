find_package(PkgConfig)
pkg_check_modules(PC_GTK2 gtk+-2.0 QUIET)

set(GTK2_DEFINITIONS ${PC_GTK2_CFLAGS_OTHER})
set(GTK2_INCLUDE_DIRS ${PC_GTK2_INCLUDE_DIRS})

foreach(COMP ${PC_GTK2_LIBRARIES})
    find_library(GTK2_${COMP} NAMES ${COMP} HINTS ${PC_GTK2_LIBRARY_DIRS})
    list(APPEND GTK2_LIBRARIES ${GTK2_${COMP}})
endforeach()

execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable gtk_binary_version gtk+-2.0
                OUTPUT_VARIABLE GTK2_BINARY_VERSION)

string(REGEX REPLACE "[\r\n]" " " GTK2_BINARY_VERSION "${GTK2_BINARY_VERSION}")
string(REGEX REPLACE " +$" "" GTK2_BINARY_VERSION "${GTK2_BINARY_VERSION}")

# handle the QUIETLY and REQUIRED arguments and set GTK2_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GTK2 DEFAULT_MSG GTK2_LIBRARIES GTK2_INCLUDE_DIRS)

mark_as_advanced(GTK2_DEFINITIONS GTK2_INCLUDE_DIRS GTK2_LIBRARIES GTK2_BINARY_VERSION)

if(PC_GTK2_FOUND AND NOT TARGET Gtk2::Gtk)
    add_library(Gtk2::Gtk INTERFACE IMPORTED)

    set_property(TARGET Gtk2::Gtk PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
    set_property(TARGET Gtk2::Gtk PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${GTK2_INCLUDE_DIRS})
    set_property(TARGET Gtk2::Gtk PROPERTY INTERFACE_LINK_LIBRARIES ${GTK2_LIBRARIES})
endif()