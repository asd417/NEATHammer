# Define the application to be monitored and the batch script to run
$applicationName = "Starcraft"  # Name of the application to monitor
$batchScriptPath = "C:\Users\ylee5\Desktop\STARTcraft\starcraft\RunStarcraftWithBWAPI.bat"

# Function to get the PIDs of running instances of the application
function Get-ApplicationPIDs {
    param (
        [string]$appName
    )
    $pids = Get-Process -Name $appName -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id
    return $pids
}

# Function to run the batch script
function Run-BatchScript {
    param (
        [string]$scriptPath
    )
    Start-Process -FilePath "cmd.exe" -ArgumentList "/c `"$scriptPath`"" -NoNewWindow -Wait
}

# Get the initial PIDs of the application instances
$pids = Get-ApplicationPIDs -appName $applicationName

if ($pids -eq $null -or $pids.Count -eq 0) {
    Write-Output "No instances of $applicationName found. Exiting..."
    exit 1
}

# Function to check if a process is running by PID
function Is-ProcessRunning {
    param (
        [int]$processId
    )
    $process = Get-Process -Id $processId -ErrorAction SilentlyContinue
    return $process -ne $null
}

# Function to monitor and reopen crashed instances
function Monitor-Processes {
    param (
        [array]$processIds,
        [string]$scriptPath
    )
    while ($true) {
        for ($i = 0; $i -lt $processIds.Count; $i++) {
            if ($processIds[$i] -eq $null) {
                continue
            }

            if (-not (Is-ProcessRunning -processId $processIds[$i])) {
                Write-Output "Process with PID $($processIds[$i]) has crashed. Restarting..."
                Run-BatchScript -scriptPath $scriptPath
                
                # Rescan for all instances and update the PID
                $newPids = Get-ApplicationPIDs -appName $applicationName
                
            }

            # Find the new PID that was added
            foreach ($newPid in $newPids) {
                if ($newPid -notin $processIds) {
                    $processIds[$i] = $newPid
                    Write-Output "New process with PID $($processIds[$i]) started."
                    break
                }
            }
        }
        Start-Sleep -Seconds 30
    }
}

# Start monitoring the processes
Monitor-Processes -processIds $pids -scriptPath $batchScriptPath