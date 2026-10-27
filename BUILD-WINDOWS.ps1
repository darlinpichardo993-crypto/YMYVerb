$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'
Set-Location $PSScriptRoot

function Fail([string]$message) {
    Write-Host "`nERROR: $message" -ForegroundColor Red
    Write-Host "Review README-ES.md for requirements and troubleshooting."
    exit 1
}

$git = Get-Command git -ErrorAction SilentlyContinue
if (-not $git) { Fail 'Git no esta instalado o no aparece en PATH.' }
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) { Fail 'CMake 3.22+ no esta instalado o no aparece en PATH.' }

$jucePath = Join-Path $PSScriptRoot '.deps\JUCE'
if (-not (Test-Path (Join-Path $jucePath 'CMakeLists.txt'))) {
    New-Item -ItemType Directory -Force -Path (Split-Path $jucePath) | Out-Null
    if (Test-Path $jucePath) { Remove-Item $jucePath -Recurse -Force }
    Write-Host 'Descargando JUCE 9.0.4 desde el repositorio oficial...' -ForegroundColor Cyan
    & git clone --depth 1 --branch 9.0.4 https://github.com/juce-framework/JUCE.git $jucePath
    if ($LASTEXITCODE -ne 0) { Fail 'No se pudo descargar JUCE. Verifica la conexion a Internet y vuelve a ejecutar.' }
}

$buildPath = Join-Path $PSScriptRoot 'build'
Write-Host 'Configurando YMYVerb para Windows x64 (Visual Studio 2022)...' -ForegroundColor Cyan
& cmake -S $PSScriptRoot -B $buildPath -G 'Visual Studio 17 2022' -A x64 "-DJUCE_PATH=$jucePath"
if ($LASTEXITCODE -ne 0) { Fail 'CMake no pudo configurar el proyecto. Confirma que esten instaladas las herramientas C++ de Visual Studio 2022.' }

Write-Host 'Compilando el VST3 en Release...' -ForegroundColor Cyan
& cmake --build $buildPath --config Release --target YMYVerb_VST3 --parallel
if ($LASTEXITCODE -ne 0) { Fail 'La compilacion del VST3 fallo. Copia el error completo para corregirlo.' }

$builtPlugin = Get-ChildItem -Path $buildPath -Directory -Filter 'YMYVerb.vst3' -Recurse -ErrorAction SilentlyContinue |
    Where-Object { $_.FullName -match '[\\/]VST3[\\/]YMYVerb\.vst3$' } |
    Select-Object -First 1
if (-not $builtPlugin) {
    $builtPlugin = Get-ChildItem -Path $buildPath -Directory -Filter 'YMYVerb.vst3' -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
}
if (-not $builtPlugin) { Fail 'La compilacion termino, pero no encontre el bundle YMYVerb.vst3 dentro de build.' }

$distPath = Join-Path $PSScriptRoot 'dist'
$pluginOut = Join-Path $distPath 'YMYVerb.vst3'
New-Item -ItemType Directory -Force -Path $distPath | Out-Null
if (Test-Path $pluginOut) { Remove-Item $pluginOut -Recurse -Force }
Copy-Item -Path $builtPlugin.FullName -Destination $pluginOut -Recurse -Force
Write-Host "VST3 creado: $pluginOut" -ForegroundColor Green

$isccPath = $null
$isccCommand = Get-Command ISCC.exe -ErrorAction SilentlyContinue
if ($isccCommand) { $isccPath = $isccCommand.Source }
if (-not $isccPath) {
    $candidates = @(
        "$env:ProgramFiles (x86)\Inno Setup 6\ISCC.exe",
        "$env:ProgramFiles\Inno Setup 6\ISCC.exe",
        "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe"
    )
    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) { $isccPath = $candidate; break }
    }
}

if ($isccPath) {
    Write-Host 'Creando el instalador SETUP.EXE...' -ForegroundColor Cyan
    & $isccPath (Join-Path $PSScriptRoot 'installer\YMYVerb.iss')
    if ($LASTEXITCODE -ne 0) { Fail 'El VST3 se compilo, pero Inno Setup no pudo crear el instalador.' }
    Write-Host "`nLISTO: $distPath\YMYVerb-Setup.exe" -ForegroundColor Green
} else {
    Write-Host "`nEl VST3 se compilo correctamente, pero no se encontro Inno Setup 6." -ForegroundColor Yellow
    Write-Host 'Instala Inno Setup 6 y ejecuta BUILD-WINDOWS.bat otra vez para generar YMYVerb-Setup.exe.'
    Write-Host "VST3: $pluginOut"
}

Write-Host "`nPulsa una tecla para cerrar..."
$null = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown')
