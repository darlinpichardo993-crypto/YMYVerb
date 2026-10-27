[README-ES.md](https://github.com/user-attachments/files/34052254/README-ES.md)
# YMYVerb — Bachata Vocal Space (versión de desarrollo 1.0.0)

**By Made ORYMY · YMY Records**

YMYVerb es un efecto de reverb vocal para bachata. La idea es colocar la voz en un espacio cálido, suave y musical, con control del pre-delay, cola, tono y anchura estéreo; no es un clon de la voz ni usa muestras de los artistas.

## Controles

- **Preset:** CUSTOM / RESET; ROMEO SANTOS - SILKY; RAULIN RODRIGUEZ - WARM; JOE VERAS - SWEET; BACHATA REVERB 2000s; BACHATA REVERB 90s; BACHATA MODERNA - LONG.
- **Pre-Delay:** tiempo manual entre la voz seca y la entrada de la reverb. Cuando `TEMPO SYNC` está encendido, el tiempo se calcula a partir del BPM que informa el DAW y la división musical seleccionada.
- **Tempo Sync / Sync Division:** sigue el tempo del proyecto en el host; divisiones cortas sirven para el espacio vocal íntimo, y divisiones largas para una separación más evidente.
- **Decay:** tamaño/longitud percibida de la cola. Es un control musical aproximado, no una medición certificada de RT60.
- **Damping:** oscurece las reflexiones y suaviza la cola.
- **Stereo Width:** anchura de la reverb.
- **Wet Mix:** mezcla seca/húmeda para una pista vocal normal.
- **Low Cut / High Cut:** limitan graves y agudos que alimentan la reverb.
- **Warmth:** saturación suave antes de la reverb.
- **Ducking:** baja un poco la cola mientras la señal de entrada está fuerte y permite que vuelva entre frases.
- **AUX / Wet Only:** elimina la señal seca para usar YMYVerb en un canal auxiliar/retorno. En un aux, normalmente sube la señal enviada desde la pista vocal y mezcla el retorno en el DAW.
- **Micrófono luminoso:** responde al nivel de audio que entra al efecto. Funciona tanto en una pista insertada como en un retorno que reciba señal.

## Cómo compilar en tu Windows 11

### Requisitos

1. Windows 10/11 de 64 bits.
2. Visual Studio 2022/2026 o la versión correspondiente de Build Tools, con **Desktop development with C++** y un Windows SDK. El script detecta automáticamente el generador disponible.
3. CMake 3.22 o superior y Git disponibles en `PATH`.
4. Conexión a Internet la primera vez para descargar JUCE 9.0.4 del repositorio oficial.
5. Inno Setup 6 para generar `YMYVerb-Setup.exe`. Si no está instalado, el script genera el bundle VST3 y explica que falta ese paso.

### Compilación

1. Extrae el ZIP a una ruta simple, por ejemplo `C:\YMYVerb-Source`.
2. Instala los requisitos anteriores si no los tienes.
3. Haz doble clic en `BUILD-WINDOWS.bat`.
4. Al terminar, el plugin estará en `dist\YMYVerb.vst3`. Si Inno Setup 6 está instalado, el instalador estará en `dist\YMYVerb-Setup.exe`.
5. Ejecuta `YMYVerb-Setup.exe` como administrador. El instalador coloca el plugin en `C:\Program Files\Common Files\VST3\YMYVerb.vst3`.
6. En FL Studio, abre **Options → Manage plugins → Find installed plugins** y vuelve a escanear.

La compilación se hace en tu equipo Windows porque el artefacto VST3 de Windows y el instalador `.exe` requieren la toolchain de Windows. Este paquete es el código fuente y la automatización de construcción, no un binario precompilado.

**Compilación automática en GitHub:** este repositorio incluye `.github/workflows/build-windows.yml`. Cada push, pull request o ejecución manual intenta compilar el VST3 y crear `YMYVerb-Setup.exe` en Windows. Cuando el workflow termine correctamente, abre **Actions → Build YMYVerb for Windows → Artifacts** y descarga `YMYVerb-Windows-x64`.

## Primeros ajustes recomendados

- **Pista vocal insertada:** empieza con `BACHATA REVERB 2000s`, mezcla alrededor de 18–28% y prueba activar Sync según el tempo.
- **Canal AUX:** activa `AUX / WET ONLY`; usa el control de envío de la voz para dosificar la reverb.
- **Bachata antigua / íntima:** `BACHATA REVERB 90s`; cola corta, ancho moderado y agudos suavizados.
- **Cola grande moderna:** `BACHATA MODERNA - LONG`; baja el envío o mezcla si invade las consonantes.

Los nombres de artistas describen referencias estéticas de presets, no ajustes originales confirmados de sus sesiones de grabación. El resultado depende del BPM, afinación, micro, sala, compresión, ecualización y nivel de envío.

## Estado de esta entrega

Incluye: procesador JUCE/C++, VST3 via CMake, presets iniciales, sincronización al host, DSP de pre-delay, filtro de entrada, control de mezcla, ducking, indicador reactivo y script del instalador.

**Verificación pendiente:** el workflow automático permite comprobar la compilación real en Windows; un resultado correcto de CI confirma que se generaron los entregables, pero no sustituye una prueba auditiva del plugin en FL Studio. Esa última validación debe hacerse con voces y proyectos reales.

## Licencia y dependencias

YMYVerb es código de desarrollo. JUCE es una dependencia independiente y tiene sus propias condiciones de licencia; revísalas antes de distribuir comercialmente un binario cerrado. El paquete no incluye archivos binarios de JUCE ni contenido de audio con derechos de terceros.
