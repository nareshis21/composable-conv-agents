$dest = "libs"
if (!(Test-Path $dest)) { New-Item -ItemType Directory -Path $dest }

$url = "https://github.com/microsoft/onnxruntime/releases/download/v1.19.0/onnxruntime-win-x64-1.19.0.zip"
$zip = "$dest\onnxruntime.zip"

Write-Host "Downloading ONNX Runtime v1.19.0..."
Invoke-WebRequest -Uri $url -OutFile $zip

Write-Host "Extracting..."
Expand-Archive -Path $zip -DestinationPath $dest -Force

Write-Host "Cleaning up..."
Remove-Item $zip

Write-Host "ONNX Runtime ready in libs/onnxruntime-win-x64-1.19.0"
