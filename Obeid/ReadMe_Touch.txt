README:

ESTE README ASUME QUE TODO YA ESTÁ INSTALADO.
ESTA GUÍA SIRVE PARA CREAR UN PROYECTO DESDE 0.

------------CORRER UTILITIES PARA PODER USAR LAS LIBRERÍAS----------------
1. Ir a la siguiente dirección C:\OpenHaptics\Developer\3.4.0\utilities\src 
2. Abrir el archivo "Utilities_VS2008" (el año depende de la versión de
VS).
3. Dentro de VS 2008 ir a la pestaña Build --> Build Solution o F7
4. Debe salir que los 4 archivos fueron compilados de satisfactoriamente.
En caso contrario, se deben seguir las guías de instalación. 
5. Al tener los 4 archivos compilados ya se puede iniciar un proyecto nuevo
y acceder a las librerías compiladas. 

-----------------------CREAR UN NUEVO PROYECTO----------------------------
1.  File --> New --> Project
2.  Visual C++ --> Win32 --> Win32 Project
3.  Dar un nombre adecuado y guardarlo en una carpeta.
4.  Dejar los valores por default o dar finish. 
5.  Click derecho en el nombre --> Propiedades
6.  Dentro de C/C++ --> Additional Include Directories
7.  Dentro de Additional Include Directories añadir:
	+ include
	+ $(OH_SDK_BASE)\include
	+ $(OH_SDK_BASE)\utilities\include
8.  Dar click en "OK"
9.  Dentro de Linker --> General --> Additional Library Directories
10. Dentro de Additional Library Directories añadir:
	+ $(OH_SDK_BASE)\lib\$(PlatformName)\$(ConfigurationName)
	+ $(OH_SDK_BASE)\utilities\lib\$(PlatformName)\$(ConfigurationName)
	+ $(OH_SDK_BASE)\lib\$(PlatformName)\
11. Dentro de Linker --> Input --> Additional Dependencies añadir:
	+ hl.lib
	+ hlu.lib
	+ hd.lib
	+ hdu.lib
	+ glut32.lib
12. En caso de obtener este error: error LNK2019: unresolved external symbol _WinMain@16 referenced in function ___tmainCRTStartup
Se debe ir a NombreDelProyecto --> Propiedades --> Linker --> System 
Cambiar el Subsystem de Windows a Console. 
13. Dar Build Solution o F7
14. Start Debugging o F5. 
