# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# let automoc handle all of the meta source files (moc)
kcm_ktoshibam_la_METASOURCES = AUTO

# the application source, library search path, and link libraries
kde_module_LTLIBRARIES = kcm_ktoshibam.la
kcm_ktoshibam_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kcm_ktoshibam_la_SOURCES = main.cpp kcmtoshiba_general.ui
kcm_ktoshibam_la_LIBADD = ../libktoshibasmminterface.la \
			../libktoshibaprocinterface.la $(LIB_KDEUI)

# this is where the desktop file will go

xdg_apps_DATA = ktoshibam.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmktoshiba.pot
