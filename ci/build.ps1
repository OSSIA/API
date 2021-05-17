Set-PSDebug -Trace 1

function CheckLastExitCode {
    param ([int[]]$SuccessCodes = @(0), [scriptblock]$CleanupScript=$null)

    if (${env:APPVEYOR}) {
      Push-AppveyorArtifact "$LogFile"
      Push-AppveyorArtifact "$repoRoot/build/CMakeFiles/CMakeOutput.log"
      Push-AppveyorArtifact "$repoRoot/build/CMakeFiles/CMakeError.log"
    }

    if ($SuccessCodes -notcontains $LastExitCode) {
        if ($CleanupScript) {
            "Executing cleanup script: $CleanupScript"
            &$CleanupScript
        }
        $msg = @"
EXE RETURNED EXIT CODE $LastExitCode
CALLSTACK:$(Get-PSCallStack | Out-String)
"@
        throw $msg
    }
}

$scriptPath = split-path -parent $MyInvocation.MyCommand.Definition
$repoRoot = "$scriptPath\.."

cd $repoRoot\build\
$LogFile = "$repoRoot\build-${env:APPVEYOR_BUILD_TYPE}-${env:platform}.log"
cmake --build . --config "${env:configuration}" > "$LogFile"
CheckLastExitCode

if ( $env:APPVEYOR_BUILD_TYPE -eq "max" -Or $env:APPVEYOR_BUILD_TYPE -eq "Release" -Or $env:APPVEYOR_BUILD_TYPE -eq "ossia-cpp"){
  cd $repoRoot\build-32bit
  $LogFile = "$repoRoot\build-${env:APPVEYOR_BUILD_TYPE}-32bit.log"
  cmake --build . --config "${env:configuration}" > "$LogFile"
  CheckLastExitCode
}

if ( $env:APPVEYOR_BUILD_TYPE -eq "Release" -Or $env:APPVEYOR_BUILD_TYPE -eq "ossia-cpp"){
  cd $repoRoot\build\
  $LogFile = "$repoRoot\build-Debug-${env:platform}.log"
  cmake --build . --config Debug > "$LogFile"
  CheckLastExitCode

  cd $repoRoot\build-32bit
  $LogFile = "$repoRoot\build-Debug-32bit.log"
  cmake --build . --config Debug > "$LogFile"
  CheckLastExitCode
}

if ( $env:APPVEYOR_BUILD_TYPE -eq "qml" )
{
  cd $repoRoot\build\
  $LogFile = "$repoRoot\build-Debug-${env:platform}.log"
  cmake --build . --config Debug > "$LogFile"
  CheckLastExitCode
}