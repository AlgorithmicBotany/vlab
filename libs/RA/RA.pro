TEMPLATE = lib
CONFIG  += staticlib
TARGET   = RA
SOURCES  = RA.cpp RAaccess.cpp RAarchive_object.cpp RAcomparefiles.cpp      \
	RAconnection.cpp RAcopyfile.cpp RAdelete_object.cpp RAdeltree.cpp  \
	RAfetchfile.cpp RAfile.cpp RAgetdir.cpp RAgetextensions.cpp        \
        RAislink.cpp RAmkdir.cpp RApaste_object.cpp                        \
	RAprototype_object.cpp RAputfile.cpp RAreadfile.cpp RAreadlink.cpp \
	RArealpath.cpp RArename.cpp RArename_object.cpp RAstat.cpp         \
	RAsymlink.cpp RAunlink.cpp RAwritefile.cpp \
	delete_object.cpp get_extensions.cpp paste_object.cpp prototype_object.cpp \
	rename_object.cpp 
# UUID stuff
SOURCES += RA_getUUID.cpp getUUID.cpp quuid.cpp
HEADERS += getUUID.h
SOURCES += uuid.cpp
HEADERS += uuid.h
SOURCES += local_search.cpp RAsearch.cpp
HEADERS += local_search.h
SOURCES += fixoofs.cpp
HEADERS += fixoofs.h
SOURCES += nodeinfo.cpp
HEADERS += nodeinfo.h
SOURCES += RAfixOofs.cpp

#QT = core

MY_BASE  = ../..
MY_LIBS = message misc
include( $${MY_BASE}/common.pri )

