$ErrorActionPreference = "Stop"

function Check-Command ($cmd) {
    if (Get-Command $cmd -ErrorAction SilentlyContinue) {
        return $true
    }
    return $false
}

# Try to find CMake in standard locations if not in PATH
if (-not (Check-Command "cmake")) {
    $possiblePaths = @(
        "C:\Program Files\CMake\bin",
        "C:\Program Files (x86)\CMake\bin"
    )
    foreach ($path in $possiblePaths) {
        if (Test-Path "$path\cmake.exe") {
            try {
                $env:Path += ";$path"
                Write-Host "✅ Auto-detected CMake at $path and added to PATH." -ForegroundColor Green
                break
            } catch {
                Write-Host "Failed to update path."
            }
        }
    }
}

# Run the original verification logic
.\scripts\verify_setup.ps1
