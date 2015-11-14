include(./config.pri)

!isEmpty(HELP) {
    # Output help
    help_string = \
        Important build options: \
        \\n\\t PREFIX : Install prefix (default: /usr) \
        \\n\\t {BIN,LIB,INCLUDE,DOC}DIR : Install prefix for specific types of files \
        \\nRecognised CONFIG flags: \
        \\n\\t x11 : Compile with X11 support \
        \\n\\t disable-gtk-cache-update : Do not update GTK2/3 input method caches (used for packaging) \
        \\n\\t local-install : Install everything underneath PREFIX, nothing to system directories reported by GTK+, Qt, DBus etc. \
        \\nInfluential environment variables: \
        \\n\\t PKG_CONFIG_PATH : Override standard directories to look for pkg-config information \
        \\nExamples: \
        \\n\\t qmake \
        \\n\\t qmake PREFIX=/usr LIBDIR=/usr/lib64 CONFIG+=disable-gtk-cache-update \
        \\n\\t qmake PREFIX=/usr MALIIT_DEFAULT_PLUGIN=libmykeyboard.so

    !build_pass:system(echo -e \"$$help_string\")
} else {
    config_string = Tip: Run qmake HELP=1 for a list of all supported build options

    !build_pass:system(echo -e \"$$config_string\")
}

CONFIG += ordered
TEMPLATE = subdirs

SUBDIRS = gtk-input-context

QMAKE_EXTRA_TARGETS += check-xml
check-xml.target = check-xml
check-xml.CONFIG = recursive

QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive

DIST_NAME = $$MALIIT_PACKAGENAME-$$MALIIT_VERSION
DIST_PATH = $$OUT_PWD/$$DIST_NAME
TARBALL_SUFFIX = .tar.bz2
TARBALL_PATH = $$DIST_PATH$$TARBALL_SUFFIX

# The 'make dist' target
# Creates a tarball
QMAKE_EXTRA_TARGETS += release
release.target = release
release.commands += git archive HEAD --prefix=$$DIST_NAME/ | bzip2 > $$TARBALL_PATH;
release.commands += md5sum $$TARBALL_PATH | cut -d \' \' -f 1 > $$DIST_PATH\\.md5

OTHER_FILES += NEWS README INSTALL.local
