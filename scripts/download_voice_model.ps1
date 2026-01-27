$baseUrl = "https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/lessac/medium"
$files = @("en_US-lessac-medium.onnx", "en_US-lessac-medium.onnx.json")

Write-Host "Downloading Piper Voice Model (Lessac Medium)..."

foreach ($file in $files) {
    $url = "$baseUrl/$file"
    # Resolve absolute path to project root (one level up from scripts/)
    $projectRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
    $output = Join-Path $projectRoot $file
    
    Write-Host "Fetching $file to $output..."
    Invoke-WebRequest -Uri $url -OutFile $output -UseBasicParsing
}

Write-Host "Done! Voice models placed in project root."
