# ================== SETTINGS ==================
$rpiUser = "avopadla"
$rpiHost = "192.168.196.173"
$destinationPath = "/home/avopadla/ota_upg/files"
# ===============================================

# We will use the built-in $args array instead of a param() block.
# This is more reliable for handling drag-and-drop arguments.

try {
    # Check if the $args array is empty.
    if ($args.Count -eq 0) {
        throw "Error: No files or folders were dragged onto the shortcut. Please run the script by dragging items onto it."
    }

    Write-Host "--- Starting transfer to Raspberry Pi ---`n" -ForegroundColor Cyan

    # Loop through each argument passed to the script.
    foreach ($path in $args) {
      
        if (-not (Test-Path $path)) {
            Write-Host "WARNING: Object not found at path: '$path'. It might be a part of a multi-file drag. Skipping." -ForegroundColor Yellow
            continue # Skip to the next file in the loop
        }
        
        $item = Get-Item $path
        
        Write-Host "Processing: $($item.FullName)" -ForegroundColor White
        
        $scpArgs = @() # Initialize an empty array for arguments

        if ($item.PSIsContainer) {
            Write-Host "This is a directory. Using recursive copy (-r)..."
            $scpArgs += "-r"
        }
        else {
            Write-Host "This is a file. Copying..."
        }

        # Add the source and destination to the arguments
        $scpArgs += $item.FullName
        $scpArgs += "$($rpiUser)@$($rpiHost):$($destinationPath)"

        # Execute scp with the arguments
        scp @scpArgs

        # Check the exit code of the LAST executed program (scp).
        # 0 means success. Any other value is an error.
        if ($LASTEXITCODE -eq 0) {
            Write-Host "SUCCESSFULLY transferred!`n" -ForegroundColor Green
        }
        else {
            Write-Host "ERROR: SCP command failed with exit code $LASTEXITCODE.`n" -ForegroundColor Red
        }
    }
}
catch {
    Write-Host "A CRITICAL ERROR occurred:" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Yellow
}
finally {
    # This block will ALWAYS run, whether there was an error or not.
    Write-Host "--- All operations are complete. ---" -ForegroundColor Cyan
    Read-Host "Press Enter to exit..."
    exit
}