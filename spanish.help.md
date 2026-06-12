# Manual de Usuario : Asistente IA
## Versión Alfa 0.0.0.1

El plugin **Asistente IA** transforma Notepad++ en un entorno de desarrollo asistido por inteligencia artificial. Le permite interactuar con varios modelos de lenguaje directamente desde su editor para analizar, corregir o generar código.
El plugin se ofrece con, como mínimo, 4 asistentes (API): groq, openrouter, anthropic y gemini.

___

## Instalación

### Prerrequisitos
Versión (c) Windows 64 bits & Notepad++ x64: **como mínimo Windows 10** (versión **1809** o superior) o Windows 11**(*)**.
**(*)** Python: en las versiones **3.13** y **3.14**, el soporte de **Windows 7, 8** y **8.1** ha sido definitivamente abandonado.
Este plugin está compilado con la opción `/MD` y está diseñado exclusivamente para arquitecturas **x64**. 
- **Verificar la compatibilidad**: En Notepad++, vaya al menú `?` (Ayuda) > `Información de depuración...`. Verifique que la línea de arquitectura mencione `64-bit x64`.
- **Dependencia del sistema**: Al estar compilado en `/MD`, el paquete ***Microsoft Visual C++ Redistributable*** debe estar instalado en su máquina para que Windows pueda cargar el plugin.

### Descarga e instalación
1. Descargue la última versión del plugin desde GitHub.
2. Descomprima el archivo descargado en un directorio temporal de su elección.
3. Copie la carpeta completa `AssistantIA` (que contiene la DLL y sus dependencias).
4. Pegue esta carpeta en el directorio de plugins de Notepad++ (generalmente ubicado en `C:\Program Files\Notepad++\plugins`).
5. Reinicie Notepad++.

___

## Uso

### Primer inicio
Al iniciar Notepad++ por primera vez después de la instalación, el plugin detectará que aún no tiene parámetros. Se abrirá automáticamente un cuadro de diálogo para guiarlo en la configuración inicial de sus claves API.
Por defecto, el plugin está en francés; si detecta otro idioma de configuración de Notepad++, traducirá los archivos de idioma y ayuda al idioma correspondiente al de Notepad++. 

### Configuración
Accesible en cualquier momento a través de la página de configuración, puede gestionar sus parámetros allí:
- **Asistente predeterminado**: El plugin utiliza **Groq** de forma predeterminada para respuestas casi instantáneas.
- **Claves API**: Debe ingresar su clave para comunicarse con la IA. Para Groq, puede obtener una clave gratis en la consola de desarrollador (*console.groq.com*).
- **Modelos**: La interfaz le permite agregar, modificar o actualizar los modelos de lenguaje disponibles para cada proveedor.
- **Registro de logs**: Una casilla de verificación le permite activar el guardado del historial de sus interacciones con la IA directamente en los archivos de configuración.

___

## Uso diario

### El Panel de Chat
Es su centro de comando principal para chatear con la IA:
- **atajo de apertura**: `ALT + I`.
- **Modelo activo**: Use la lista desplegable en la parte superior del panel para cambiar rápidamente entre los diferentes modelos ofrecidos por su proveedor.
- **Entrada**: Escriba sus instrucciones en el área de texto situada abajo.
- **Envío**: Presione el botón *Enviar* o use su atajo de teclado habitual para enviar la solicitud.
- **Historial**: La ventana central muestra sus intercambios en tiempo real. Si la opción está marcada en la configuración, estos registros se conservarán.

### Menú Contextual
El menú contextual (clic derecho) le permite interactuar rápidamente con su **código seleccionado**:
- **Código seleccionado**: Está limitado a los primeros 20,000 caracteres. Los proveedores (API) incluidos son versiones gratuitas. Notepad++ le indica en su barra de estado la longitud de la cadena seleccionada y el número de líneas: ej: *Sel : 787 : 7* .
- **atajo de apertura**: `ALT + Q`.
- **Hacer una pregunta a la IA**: Abre el panel para hacer su pregunta (solicitud).
- **Validar**: Envía su solicitud a la IA inyectando directamente el contexto de su selección actual en segundo plano.
- **Respuesta de la IA**: La respuesta de la IA se muestra en vista dividida en un documento temporal *AssistantIA.txt* frente al documento de su selección.

___


## La consola:

El plugin incorpora una consola. Esta se puede mostrar u ocultar y se puede desacoplar en una ventana flotante. Muestra información sobre las acciones en curso y los posibles errores. También dispone de un prompt de python.
Es útil principalmente para fines de depuración.
___

## Funcionalidades Avanzadas: Scripts y Plantillas

El plugin está diseñado para ser extensible por usuarios avanzados que deseen personalizar su experiencia y agregar nuevos proveedores (APIs).

- **Creación de nuevos proveedores**: A través del menú del plugin, seleccione `Nuevo script` para integrar una nueva API de IA.
- **Estructura de un script**: Cada nuevo script se construye sobre una base común generada automáticamente. Solo le queda finalizar la lógica de solicitud específica del proveedor.
- **Modificación de los prompts**: Puede modificar el *System Prompt* directamente en los archivos Python de cada proveedor para adaptar el comportamiento de la IA a sus necesidades modificando el valor traducido de: `system_prompt = _T("MESSAGES", "MSG_PROV_SYS_PROMPT_CODE")`. ej: *`system_prompt ="Eres un experto en PHP"*, *"Responde solo en español, ..."`*, etc.

___

## Solución de Problemas (Troubleshooting)

Si el plugin no se carga o muestra un error, verifique los siguientes puntos:

- **Ausencia de respuesta**: Verifique primero que está conectado a la red de Internet.
- **Lista de modelos vacía**: Verifique su conexión a internet, asegúrese de que su clave API sea válida y verifique que el nombre exacto del modelo sea reconocido por el proveedor.
- **Fallo al iniciar o Plugin invisible**: Asegúrese de que el *Visual C++ Redistributable (x64)* esté instalado en su máquina. Esta instalación es estrictamente necesaria para el funcionamiento de la DLL y la carga de las librerías de Python asociadas (`.pyd`).

___

*Nota: Este plugin es una herramienta de asistencia para la codificación. Mantenga un espíritu crítico. Asegúrese de siempre releer y probar el código generado antes de integrarlo en sus proyectos de producción.*
