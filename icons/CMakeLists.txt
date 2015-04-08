ecm_install_icons(ICONS
			16-apps-ktoshiba.png
			22-apps-ktoshiba.png
			32-apps-ktoshiba.png
			sc-apps-ktoshiba.svgz
		DESTINATION ${ICON_INSTALL_DIR}
		THEME hicolor)

_ecm_update_iconcache(${ICON_INSTALL_DIR} hicolor)