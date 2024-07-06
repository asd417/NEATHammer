# Define the application to be monitored and the batch script to run
$applicationName = "Starcraft"  # Name of the application to monitor
$batchScriptPath = ".\RunStarcraftWithBWAPI.bat"

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
    Start-Process -FilePath "cmd.exe" -ArgumentList "/c `"$scriptPath`""
}

# Get the initial PIDs of the application instances
$pids = Get-ApplicationPIDs -appName $applicationName

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
        # Rescan for all instances and update the PID
        $newPids = Get-ApplicationPIDs -appName $applicationName
	if($newPids.count -lt 6){
		Run-BatchScript -scriptPath $scriptPath
		Write-Output "Process Count less than 6. Opening new process"
	}
        Start-Sleep -Seconds 30
    }
}

# Start monitoring the processes
Monitor-Processes -processIds $pids -scriptPath $batchScriptPath