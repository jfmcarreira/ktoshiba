# set the include path for X, qt and KDE
INCLUDES = -I$(top_builddir)/src/ $(all_includes)

# let automoc handle all of the meta source files (moc)
kcm_ktoshiba_la_METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/ktoshiba.pot

# the application source, library search path, and link libraries
kde_module_LTLIBRARIES = kcm_ktoshiba.la
kcm_ktoshiba_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
kcm_ktoshiba_la_SOURCES = main.cpp kcmtoshiba_general.ui
kcm_ktoshiba_la_LIBADD = ../libktoshibasmminterface.la $(LIB_KDEUI)

# this is where the desktop file will go
xdg_apps_DATA = ktoshiba.desktop
