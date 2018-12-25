### Generated by Winemaker 0.8.4
###
### Invocation command line was
### /usr/bin/winemaker --lower-uppercase --console -I/usr/include/wine-development/wine/msvcrt/ .


SRCDIR                = .
SUBDIRS               =
DLLS                  =
LIBS                  =
EXES                  = 816.exe



### Common settings

CEXTRA                = -mno-cygwin -fno-builtin 
CXXEXTRA              = -mno-cygwin -fno-builtin 
RCEXTRA               =
DEFINES               =
INCLUDE_PATH          = -I/usr/include/wine -I/usr/include/wine/msvcrt/ -I/usr/include/wine/windows/ -I.
DLL_PATH              =
DLL_IMPORTS           =
LIBRARY_PATH          =
LIBRARIES             = ./816lib/src/lib65816.a


### 816.exe sources and settings

816_exe_MODULE        = 816.exe
816_exe_C_SRCS        =
816_exe_CXX_SRCS      = ResponseRange.cpp \
			XR88C681.cpp \
			device.cpp \
			device_aggregator.cpp \
			main.cpp \
			ram_device.cpp \
			rom_device.cpp \
			system_controller_cpld.cpp
816_exe_RC_SRCS       =
816_exe_LDFLAGS       = -mconsole -mno-cygwin -fno-builtin
816_exe_ARFLAGS       =
816_exe_DLL_PATH      =
816_exe_DLLS          = odbc32 \
			ole32 \
			oleaut32 \
			winspool \
			odbccp32
816_exe_LIBRARY_PATH  =
816_exe_LIBRARIES     = uuid

816_exe_OBJS          = $(816_exe_C_SRCS:.c=.o) \
			$(816_exe_CXX_SRCS:.cpp=.o) \
			$(816_exe_RC_SRCS:.rc=.res)



### Global source lists

C_SRCS                = $(816_exe_C_SRCS)
CXX_SRCS              = $(816_exe_CXX_SRCS)
RC_SRCS               = $(816_exe_RC_SRCS)


### Tools

CC = winegcc
CXX = wineg++
RC = wrc
AR = ar


### Generic targets

all: $(SUBDIRS) $(DLLS:%=%.so) $(LIBS) $(EXES)

### Build rules

.PHONY: all clean dummy

$(SUBDIRS): dummy
	@cd $@ && $(MAKE)

# Implicit rules

.SUFFIXES: .cpp .cxx .rc .res
DEFINCL = $(INCLUDE_PATH) $(DEFINES) $(OPTIONS)

.c.o:
	$(CC) -c $(CFLAGS) $(CEXTRA) $(DEFINCL) -o $@ $<

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(CXXEXTRA) $(DEFINCL) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(CXXEXTRA) $(DEFINCL) -o $@ $<

.rc.res:
	$(RC) $(RCFLAGS) $(RCEXTRA) $(DEFINCL) -fo$@ $<

# Rules for cleaning

CLEAN_FILES     = y.tab.c y.tab.h lex.yy.c core *.orig *.rej \
                  \\\#*\\\# *~ *% .\\\#*

clean:: $(SUBDIRS:%=%/__clean__) $(EXTRASUBDIRS:%=%/__clean__)
	$(RM) $(CLEAN_FILES) $(RC_SRCS:.rc=.res) $(C_SRCS:.c=.o) $(CXX_SRCS:.cpp=.o)
	$(RM) $(DLLS:%=%.so) $(LIBS) $(EXES) $(EXES:%=%.so)

$(SUBDIRS:%=%/__clean__): dummy
	cd `dirname $@` && $(MAKE) clean

$(EXTRASUBDIRS:%=%/__clean__): dummy
	-cd `dirname $@` && $(RM) $(CLEAN_FILES)

### Target specific build rules
DEFLIB = $(LIBRARY_PATH) $(LIBRARIES) $(DLL_PATH) $(DLL_IMPORTS:%=-l%)

$(816_exe_MODULE): $(816_exe_OBJS)
	$(CXX) $(816_exe_LDFLAGS) -o $@ $(816_exe_OBJS) $(816_exe_LIBRARY_PATH) $(816_exe_DLL_PATH) $(DEFLIB) $(816_exe_DLLS:%=-l%) $(816_exe_LIBRARIES:%=-l%)


