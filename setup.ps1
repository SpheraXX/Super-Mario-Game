# setup.ps1 - Environment Verification Script for Super Mario Project (Windows)
# Run this script to verify dependencies and setup the build directory.

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "   Super Mario Project: Setup & Verification" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# 1. Check CMake
Write-Host "[1/3] Checking CMake..." -ForegroundColor Yellow
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    $cmakeVer = cmake --version | Select-Object -First 1
    Write-Host "✓ CMake found: $cmakeVer" -ForegroundColor Green
} else {
    Write-Warning "✗ CMake is not installed or not in PATH! Please install CMake."
}

# 2. Check compiler
Write-Host "[2/3] Checking C++ Compiler..." -ForegroundColor Yellow
if (Get-Command g++ -ErrorAction SilentlyContinue) {
    $cppVer = g++ --version | Select-Object -First 1
    Write-Host "✓ GCC compiler found: $cppVer" -ForegroundColor Green
} elseif (Get-Command cl -ErrorAction SilentlyContinue) {
    Write-Host "✓ MSVC compiler found." -ForegroundColor Green
} else {
    Write-Warning "✗ No standard C++ Compiler (g++ / MSVC) found in PATH!"
}

# 3. Setup build directory
Write-Host "[3/3] Setting up build directory..." -ForegroundColor Yellow
$BuildDir = Join-Path $PSScriptRoot "build"
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
    Write-Host "✓ Created build/ directory." -ForegroundColor Green
} else {
    Write-Host "✓ build/ directory already exists." -ForegroundColor Green
}

Write-Host ""
Write-Host "Setup verification complete!" -ForegroundColor Cyan
Write-Host "To build the project, run:" -ForegroundColor Yellow
Write-Host "  cd build" -ForegroundColor Gray
Write-Host "  cmake .." -ForegroundColor Gray
Write-Host "  cmake --build ." -ForegroundColor Gray
Write-Host "=========================================" -ForegroundColor Cyan
