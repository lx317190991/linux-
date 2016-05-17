AUTOMAKE_OPTIONS=foreign

bin_PROGRAMS=minimonitor

BOOSTINCLUDEDIR ?= /usr/local/include

BOOSTLIBDIR ?= /usr/local/lib

BMCOINCLUDEDIR ?= /usr/local/include

BMCOLIBDIR ?= /usr/local/lib

NCURSES_INCLUDE ?= /usr/local/include

NCURSES_LIB ?= /usr/local/lib

NCURSES_CDK_INCLUDE ?= /usr/local/include

NCURSES_CDK_LIB ?= /usr/local/lib

THECPPS = BprocessBOL.cpp\
Markup.cpp\
UbpcbInfo.cpp\
Umem.cpp\
Umessage.cpp\
UmessageGroup.cpp\
Umonitor.cpp\
UmonitorApp.cpp\
UprogramDef.cpp\
UregionMem.cpp\
UregularProcessTask.cpp\
UMQDef.cpp\
UMQRela.cpp\
UMQTopic.cpp\
UMQService.cpp

THEHEADERS = BprocessBOL.h\
Markup.h\
UbpcbInfo.h\
Umem.h\
Umessage.h\
UmessageGroup.h\
Umonitor.h\
UmonitorApp.h\
UprogramDef.h\
UregionMem.h\
UregularProcessTask.h\
UMQDef.h\
UMQRela.h\
UMQTopic.h\
UMQService.h

minimonitor_SOURCES = $(THECPPS)
include_HEADERS = $(THEHEADERS)

minimonitor_LDADD = ${NEDIR}/lib/libBolCommon.la -L${NCURSES_CDK_LIB} -lcdk -L${NCURSES_LIB} -lmenu_g -lpanel_g -lform_g -lncurses_g

minimonitor_CPPFLAGS = -I${BOOSTINCLUDEDIR} -I${BMCOINCLUDEDIR}/Util/include -I${BMCOINCLUDEDIR}/Net/include -I${BMCOINCLUDEDIR}/Foundation/include -I../../../bol/ -I${NCURSES_INCLUDE} -I${NCURSES_CDK_INCLUDE}

minimonitor_LDFLAGS =  -lboost_date_time -lBmcoNet -lBmcoUtil -lBmcoData -lBmcoFoundation -lprotobuf -lprotobuf-lite -lprotoc 

if OPERATORSYSTEM_IS_AIX
    minimonitor_LDFLAGS += -liconv
endif