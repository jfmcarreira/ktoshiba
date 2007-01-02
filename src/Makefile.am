SUBDIRS = . kcmktoshiba images

if with_keyhandler
   KEYHANDLER = ktosh_keyhandler
   ktosh_keyhandler_SOURCES = ktosh_keyhandler.cpp
   ktosh_keyhandler_LDFLAGS = $(all_libraries)
   ktosh_keyhandler_LDADD = $(LIB_KDEUI) -lXext -lXtst
endif

# set the include path for X, qt and KDE
INCLUDES = @DBUS_CFLAGS@ $(all_includes)

# these are the headers for your project
noinst_HEADERS = ktoshiba.h hci.h ktoshibasmminterface.h sci.h modelid.h \
					ktoshibaprocinterface.h toshibafnactions.h omnibookfnactions.h ktoshibadcopinterface.h \
					suspend.h ktoshibadbusinterface.h ktoshibaomnibookinterface.h fnactions.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = ktoshiba $(HELPER) $(KEYHANDLER)

# the application source, library search path, and link libraries
lib_LTLIBRARIES = libktoshibasmminterface.la libktoshibaomnibookinterface.la libktoshibaprocinterface.la
libktoshibasmminterface_la_LDFLAGS = $(all_libraries)
libktoshibasmminterface_la_SOURCES = hci.c sci.c ktoshibasmminterface.cpp
libktoshibaomnibookinterface_la_LDFLAGS = $(all_libraries)
libktoshibaomnibookinterface_la_SOURCES = ktoshibaomnibookinterface.cpp
libktoshibaprocinterface_la_LDFLAGS = $(all_libraries)
libktoshibaprocinterface_la_SOURCES = ktoshibaprocinterface.cpp

ktoshiba_SOURCES = settingswidget.ui statuswidget.ui ktoshiba.cpp main.cpp \
				toshibafnactions.cpp omnibookfnactions.cpp ktoshibadcopinterface.cpp \
				ktoshibadcopinterface.skel suspend.cpp ktoshibadbusinterface.cpp fnactions.cpp
ktoshiba_LDFLAGS = $(KDE_RPATH) $(all_libraries)
ktoshiba_LDADD = $(LIB_KDEUI) $(LIB_KIO) -ldbus-qt-1 \
		 libktoshibaprocinterface.la \
		 libktoshibasmminterface.la \
		 libktoshibaomnibookinterface.la \
		 @DBUS_LIBS@ \
		 @SYNAPTICS@

# this is where the desktop file will go
xdg_apps_DATA     = ktoshiba.desktop

autostartdir      = $(datadir)/autostart
autostart_DATA    = ktoshiba.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/ktoshiba.pot
