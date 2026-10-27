param([switch]$NoPause)

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
$programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
$vswherePath = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
$generator = $null
if (Test-Path $vswherePath) {
    $installationVersion = & $vswherePath -latest -property installationVersion
    if ($LASTEXITCODE -eq 0 -and $installationVersion) {
        $majorVersion = [int](($installationVersion -split '[.]')[0])
        if ($majorVersion -ge 18) { $generator = 'Visual Studio 18 2026' }
        elseif ($majorVersion -ge 17) { $generator = 'Visual Studio 17 2022' }
        elseif ($majorVersion -ge 16) { $generator = 'Visual Studio 16 2019' }
    }
}
if (-not $generator) {
    Fail 'No se encontro una instalacion compatible de Visual Studio. Instala Visual Studio 2022/2026 o Build Tools con Desktop development with C++ y Windows SDK.'
}

Write-Host "Configurando YMYVerb para Windows x64 ($generator)..." -ForegroundColor Cyan
& cmake -S $PSScriptRoot -B $buildPath -G $generator -A x64 "-DJUCE_PATH=$jucePath"
if ($LASTEXITCODE -ne 0) { Fail "CMake no pudo configurar el proyecto con $generator. Comprueba el componente Desktop development with C++ y el Windows SDK." }

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

if (-not $NoPause) {
    Write-Host "`nPulsa una tecla para cerrar..."
    try { $null = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown') } catch { }
}
