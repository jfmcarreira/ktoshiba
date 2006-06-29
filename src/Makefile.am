SUBDIRS = . kcmktoshiba images

if with_helper
  HELPER = ktosh_helper
  ktosh_helper_SOURCES = ktosh_helper.cpp
endif

# set the include path for X, qt and KDE
INCLUDES = $(all_includes) @DBUS_CFLAGS@

# these are the headers for your project
noinst_HEADERS = ktoshiba.h hci.h ktoshibasmminterface.h sci.h modelid.h \
	ktoshibaprocinterface.h toshibafnactions.h omnibookfnactions.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = ktoshiba $(HELPER)

# the application source, library search path, and link libraries
lib_LTLIBRARIES = libktoshibasmminterface.la libktoshibaprocinterface.la
libktoshibasmminterface_la_LDFLAGS = $(all_libraries)
libktoshibasmminterface_la_SOURCES = hci.c sci.c ktoshibasmminterface.cpp
libktoshibaprocinterface_la_LDFLAGS = $(all_libraries)
libktoshibaprocinterface_la_SOURCES = ktoshibaprocinterface.cpp

ktoshiba_SOURCES = settingswidget.ui statuswidget.ui modelid.cpp ktoshiba.cpp \
	main.cpp toshibafnactions.cpp omnibookfnactions.cpp
ktoshiba_LDFLAGS = $(KDE_RPATH) $(all_libraries)
ktoshiba_LDADD = libktoshibasmminterface.la \
		 libktoshibaprocinterface.la \
		 $(LIB_KDEUI) \
		 @SYNAPTICS@ \
		 @POWERSAVE_LIBS@ \
		 @POWERSAVE_DBUS_LIBS@ \
		 @DBUS_LIBS@

# this is where the desktop file will go 
shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = ktoshiba.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/ktoshiba.pot
