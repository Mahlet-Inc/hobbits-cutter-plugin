HEADERS        += hobbitsdisplayplugin.h
SOURCES        += hobbitsdisplayplugin.cpp

# cutter headers (!! This assumes the cutter repo is at ../../cutter !!)
INCLUDEPATH    += ../../cutter/src \
                  ../../cutter/src/core \
                  ../../cutter/src/widgets \
                  ../../cutter/src/dialogs \
                  ../../cutter/src/common \
                  ../../cutter/src/menus \
                  ../../cutter/src/plugins

# radare2 dependency (!! THIS PROBABLY NEEDS TO BE ADJUSTED FOR YOUR SYSTEM !!)
INCLUDEPATH += /usr/include/libr
include(../../cutter/src/lib_radare2.pri)

# hobbits core dependency (!! THIS PROBABLY NEEDS TO BE ADJUSTED FOR YOUR SYSTEM !!)
INCLUDEPATH += $$(HOME)/.local/include/hobbits
LIBS += $$(HOME)/.local/lib/hobbits/libhobbits-core.so

TEMPLATE        = lib
CONFIG         += plugin
QT             += widgets
TARGET          = HobbitsDisplayCutterPlugin

target.path = $$(HOME)/.local/share/RadareOrg/Cutter/plugins/native
INSTALLS += target
