KToshiba
=====
KToshiba esta protegido bajo la licencia GPL version 2 o mejor. Una copia
de esta licencia puede ser encontrada en el archivo COPYING en el directorio base


¿Qué es esto?
=====
KToshiba es un monitor para KDE de las teclas especiales encontradas en los portatiles
de Toshiba (Fn-F#) y el panel multimedia frontal.

Este programa a sido probado en un modelo Satellite(R) A25-S279,
es posible que los modelos mas recientes del modelo Satellite funcionen
con este programa mas sin embargo no han sido probados, ya que la funcionalidad de
la tecla Fn es la misma en todos ellos hasta donde he visto.


Requisitos
=====
- Un portatil Toshiba (indispensable)
- Compilador y utilerias (autoconf, automake, etc.)
- El monitor powersave y librerias (para STR y STD)
- Mucho valor (solo una broma)


Preparativos
=====
Para que este programa funcione eficientemente se necesita especificar
el puerto de la tecla Fn al momento de la carga del controlador del kernel
('insmod toshiba tosh_fn=0x62').
Actualmente solo se conocen dos puertos para la tecla Fn 0x62 y 0x68,
tu puedes intentar con cualquiera de estos hasta que encuentres el puerto
que trabaja con tu portatil.

Si no surge ningun problema al momento de ejecutar 'insmod toshiba tosh_fn=0x62'
entonces van a poder ver algo como esto si ejecutan 'cat /proc/toshiba'

	1.1 0xfcf8 2.82 1.10 0x24ca 0x00

Si encontraste un puerto que funciona entonces tu querras automatizar esto
en vez de estarlo tecleando cada vez (si soy algo flojo).
Asi que basicamente solo necesitas editar modules.conf o modprobe.conf
dependiendo de la version del kernel que estes corriendo, busca algo similar a:

	alias char-major-10-181 toshiba

y despues solo adhiere lo siguiente "options toshiba tosh_fn=0x62"
sin las comillas y se vera algo asi:

	alias char-major-10-181 toshiba
	options toshiba tosh_fn=0x62

Si tu usas SuSE entonces tu querras editar modules.conf.local o
modprobe.conf.local (dependiendo del kernel) entonces.


Instalacion
=====
Si todo salio bien hasta este punto entonces la instalacion es muy
sencilla, descomprime el archivo descargado y haz lo siguiente:

	cd ktoshiba-0.X 	(donde X es la version actual)
	./configure --prefix=/lugar/donde/esta/instalado/kde
	make
	su -c "make install"	(este paso va arequerir de la contraseña del administrador)

o si descargaste el rpm en tonces haz lo siguiente:

	rpm -ivh ktoshiba-0.X.i386.rpm 	(esto se tiene que realizar como administrador)


Modo de uso
=====
Si la instalacion se llevo acabo sin problemas tu podras encontrar KToshiba
en el menu de Utilidades, da click en el y un icono aparecera en la barra
de tareas, da click con el boton derecho del raton y selecciona configurar,
una nueva ventana aparecera en la cual tu podras configurar algunos valores
de monitoreo de bateria asi como seleccionar la aplicacion de audio para
usarse con los botones del panel frontal.


Soporte, Bichos y/o Pedidos
=====
Tengo planes de dar mas soporte a otros portatiles de Toshiba en futuras
versiones, si te gusto el programa y deseas obtener soporte para tu portatil
manda un correo electronico a neftali@utep.edu e incluye las combinaciones
Fn-F[1,2,3,4,5,6,7,8,9] y que hacen (ej: Fn-F1 Bloquea pantalla o 
Fn-F7 Aumenta el brillo de pantalla).

Si encontraste algun error o solamente quieres soporte para algo en futuras
versiones manda un correo electronico a la direccion dada con la descripcion 
del problema o la caracteristica de soporte deseada, o simplemente accesa
a la pagina en SourceForge.net y reportalo ahi.

Contacto
=====
Me puedes contactar por correo electronico en
neftali@utep.edu or nonoxinol900@hotmail.com

La pagina del proyecto esta en SourceForge.net
http://sourceforge.net/projects/ktoshiba
http://ktoshiba.sourceforge.net/