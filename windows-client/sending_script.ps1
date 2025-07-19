$rpiUser = "avopadla"
$rpiHost = "192.168.196.173"
$destinationPath = "/home/avopadla/ota_upg/files"
$remotePythonScript = "/home/avopadla/ota_upg/flash_firmware.py"
$remoteLogPath = "/home/avopadla/ota_upg/changelog.txt"

try {
    if ($args.Count -ne 1) {
        throw "Error: Please drag exactly ONE file onto this script. You provided $($args.Count) items."
    }

    $sourcePath = $args[0]
    $item = Get-Item $sourcePath -ErrorAction Stop

    if ($item.PSIsContainer) {
        throw "Error: Folders are not supported. Please drag a single file."
    }

    Write-Host "--- Starting transfer to Raspberry Pi ---`n" -ForegroundColor Cyan
    Write-Host "Processing file: $($item.FullName)" -ForegroundColor White
    
    scp $item.FullName "$($rpiUser)@$($rpiHost):$($destinationPath)"

    if ($LASTEXITCODE -eq 0) {
        Write-Host "SUCCESSFULLY transferred!`n" -ForegroundColor Green
        
        $remoteFilePath = "$destinationPath/$($item.Name)"
        
        $logMessage = "[$(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')] File received: $remoteFilePath"
        $sshLogCommand = "echo `"$logMessage`" >> $remoteLogPath"
        ssh "$rpiUser@$rpiHost" $sshLogCommand

        $sshPythonCommand = "sudo python3 $remotePythonScript `"$remoteFilePath`""
        Write-Host "Running remote script: $sshPythonCommand" -ForegroundColor Cyan
        ssh "$rpiUser@$rpiHost" $sshPythonCommand
    }
    else {
        throw "ERROR: SCP command failed with exit code $LASTEXITCODE."
    }
}
catch {
    Write-Host "A CRITICAL ERROR occurred:" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Yellow
}
finally {
    Write-Host "`n--- All operations are complete. ---" -ForegroundColor Cyan
    Read-Host "Press Enter to exit..."
    exit
}