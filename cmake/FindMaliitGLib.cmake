find_package(PkgConfig)
pkg_check_modules(PC_MALIIT_GLIB maliit-glib QUIET)

set(MALIIT_GLIB_DEFINITIONS ${PC_MALIIT_GLIB_CFLAGS_OTHER})
set(MALIIT_GLIB_INCLUDE_DIRS ${PC_MALIIT_GLIB_INCLUDE_DIRS})

foreach(COMP ${PC_MALIIT_GLIB_LIBRARIES})
    find_library(MALIIT_GLIB_${COMP} NAMES ${COMP} HINTS ${PC_MALIIT_GLIB_LIBRARY_DIRS})
    list(APPEND MALIIT_GLIB_LIBRARIES ${MALIIT_GLIB_${COMP}})
endforeach()

# handle the QUIETLY and REQUIRED arguments and set MALIIT_GLIB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MALIIT_GLIB DEFAULT_MSG MALIIT_GLIB_LIBRARIES MALIIT_GLIB_INCLUDE_DIRS)

mark_as_advanced(MALIIT_GLIB_INCLUDE_DIRS MALIIT_GLIB_LIBRARIES)

if(PC_MALIIT_GLIB_FOUND AND NOT TARGET Maliit::GLib)
    add_library(Maliit::GLib INTERFACE IMPORTED)

    set_property(TARGET Maliit::GLib PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${MALIIT_GLIB_INCLUDE_DIRS})
    set_property(TARGET Maliit::GLib PROPERTY INTERFACE_LINK_LIBRARIES ${MALIIT_GLIB_LIBRARIES})
endif()