$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

$cmake = Join-Path $PSScriptRoot '.tools/cmake-3.31.6-windows-x86_64/bin/cmake.exe'
if (!(Test-Path $cmake)) {
    $cmake = (Get-Command cmake -ErrorAction Stop).Source
}

& $cmake -S . -B build -G 'MinGW Makefiles'
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
& $cmake --build build --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
& (Join-Path $PSScriptRoot 'build/drone_simulator_enhanced.exe')
