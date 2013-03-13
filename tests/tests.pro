include(../config.pri)

CONFIG += ordered

TEMPLATE = subdirs

SUBDIRS += \
          sanitychecks \
          ut_maliit_glib_settings \

QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive
