$ErrorActionPreference = "Stop"

function Check-File ($path, $desc) {
    if (Test-Path $path) {
        Write-Host "✅ FOUND $desc : $path" -ForegroundColor Green
    } else {
        Write-Host "❌ ISSING $desc : $path" -ForegroundColor Red
        return $false
    }
    return $true
}

function Check-Command ($cmd) {
    if (Get-Command $cmd -ErrorAction SilentlyContinue) {
        Write-Host "✅ FOUND Command : $cmd" -ForegroundColor Green
    } else {
        Write-Host "❌ MISSING Command : $cmd (Please add to PATH)" -ForegroundColor Red
        return $false
    }
    return $true
}

Write-Host "--- Verifying Voice Agent Setup ---"

$allGood = $true

# 1. Check Tools
if (Test-Path "C:\piper\piper.exe") {
    Write-Host "✅ FOUND Piper : C:\piper\piper.exe" -ForegroundColor Green
} elseif (Check-Command "piper.exe") {
    # Found in PATH
} else {
    Write-Host "❌ MISSING Piper : Not in C:\piper or PATH" -ForegroundColor Red
    $allGood = $false
}

$allGood = (Check-Command "cmake") -and $allGood

# 2. Check Voice Models (in project root)
$allGood = (Check-File "$PSScriptRoot\..\en_US-lessac-medium.onnx" "Piper ONNX") -and $allGood
$allGood = (Check-File "$PSScriptRoot\..\en_US-lessac-medium.onnx.json" "Piper JSON") -and $allGood

# 3. Check AI Models (in models/)
$allGood = (Check-File "$PSScriptRoot\..\models\qwen2.5-3b-instruct-q4_k_m.gguf" "Qwen LLM") -and $allGood
# Note: Whisper model name might vary depending on what user downloaded, assuming base.en
# If missing, we warn but user might have a different one.
if (-not (Test-Path "$PSScriptRoot\..\models\ggml-base.en.bin")) {
    Write-Host "⚠️  WARNING: Whisper model 'ggml-base.en.bin' not found in models/. Check filename." -ForegroundColor Yellow
} else {
    Write-Host "✅ FOUND Whisper Model" -ForegroundColor Green
}

if ($allGood) {
    Write-Host "`n🎉 READY TO BUILD! Run the build commands now." -ForegroundColor Green
} else {
    Write-Host "`n🛑 Setup incomplete. Please fix the missing items above." -ForegroundColor Red
}
