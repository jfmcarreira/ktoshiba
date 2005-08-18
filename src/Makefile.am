SUBDIRS = . kcmktoshiba images

# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# these are the headers for your project
noinst_HEADERS = ktoshiba.h hci.h ktoshibasmminterface.h sci.h synshm.h \
			modelid.h fnactions.h procactions.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = ktoshiba ktosh_helper

# the application source, library search path, and link libraries
lib_LTLIBRARIES = libktoshibasmminterface.la
libktoshibasmminterface_la_LDFLAGS = $(all_libraries)
libktoshibasmminterface_la_SOURCES = hci.c sci.c ktoshibasmminterface.cpp

ktoshiba_SOURCES = main.cpp ktoshiba.cpp settingswidget.ui statuswidget.ui \
			modelid.cpp fnactions.cpp procactions.cpp
ktoshiba_LDFLAGS = $(KDE_RPATH) $(all_libraries)
ktoshiba_LDADD = libktoshibasmminterface.la $(LIB_KDEUI)

# this is where the desktop file will go 
shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = ktoshiba.desktop

ktosh_helper_SOURCES = ktosh_helper.cpp

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/ktoshiba.pot
