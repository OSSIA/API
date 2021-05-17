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
cd "$repoRoot\3rdparty"

# Download portaudio, ffmpeg
if ( $env:configuration -eq "Debug" )
{
  $ossia_sdk_archive = "sdk-msvc-debug.zip"
}
else
{
  $ossia_sdk_archive = "sdk-msvc-release.zip"
}

Invoke-WebRequest -Uri "https://github.com/ossia/sdk/releases/download/sdk18/$ossia_sdk_archive" -OutFile "$repoRoot\3rdparty\$ossia_sdk_archive"
7z x $ossia_sdk_archive

cd ..

mkdir build
cd build

$CommonFlags = "-Thost=x64","-DOSSIA_SDK=$repoRoot","-DBOOST_ROOT=""${env:BOOST_ROOT}""","-DOSSIA_C=1","-DOSSIA_CPP=1","-DOSSIA_EDITOR=0","-DOSSIA_DATAFLOW=0","-DCMAKE_BUILD_TYPE=Release","-DOSSIA_CI=1","-DOSSIA_TESTING=0","-DOSSIA_EXAMPLES=0","-DOSSIA_PD=0","-DOSSIA_PYTHON=0","-DOSSIA_QT=0","-DOSSIA_PROTOCOL_AUDIO=0","-DOSSIA_PROTOCOL_JOYSTICK=0","-DOSSIA_PROTOCOL_WIIMOTE=0","-DOSSIA_PROTOCOL_ARTNET=0","-DCMAKE_INSTALL_PREFIX=""${env:APPVEYOR_BUILD_FOLDER}/install"""
$32bitgen = "-G""Visual Studio 16 2019""","-A","Win32"
$64bitgen = "-G""Visual Studio 16 2019""","-A","x64"

if ( $env:APPVEYOR_BUILD_TYPE -eq "testing" ){

  if ( Test-Path ${env:QTDIR}\bin\ ) {
    set $env:PATH=${env:QTDIR}\bin;${env:PATH};
  }

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\config-${env:APPVEYOR_BUILD_TYPE}-${env:configuration}.log"

  cmake $64bitgen -T host=x64 -DOSSIA_C=1 -DOSSIA_CPP=1 -DOSSIA_PD=0 -DOSSIA_CI=1 -DOSSIA_TESTING=1 -DOSSIA_EDITOR=1 -DOSSIA_DATAFLOW=1 -DOSSIA_QT=1 -DOSSIA_QML=1 -DCMAKE_PREFIX_PATH="${env:QTDIR}\lib\cmake\Qt5" -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" $repoRoot > $LogFile

  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "Release" ) {
  if ( Test-Path ${env:QTDIR}\bin\ ) {
    set $env:PATH=${env:QTDIR}\bin;${env:PATH};
  }

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\config-${env:APPVEYOR_BUILD_TYPE}-win64.log"
  cmake $64bitgen $CommonFlags $repoRoot > $LogFile
  CheckLastExitCode

  # now configure 32 bit version
  cd ..
  mkdir build-32bit
  cd build-32bit

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\config-${env:APPVEYOR_BUILD_TYPE}-win32.log"
  cmake $32bitgen $CommonFlags $repoRoot > $LogFile
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "max" ) {
  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\config-${env:APPVEYOR_BUILD_TYPE}-win64.log"
  cmake $64bitgen -DOSSIA_MAX_ONLY=1 -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" $repoRoot > $LogFile
  CheckLastExitCode

  # now configure 32 bit version
  cd ..
  mkdir build-32bit
  cd build-32bit

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\config-${env:APPVEYOR_BUILD_TYPE}-win32.log"
  cmake $32bitgen -DOSSIA_MAX_ONLY=1 -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" $repoRoot > $LogFile
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "ossia-cpp" ) {
  $LogFile = "$repoRoot\configure-opp.log"
  cmake $64bitgen -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" -DOSSIA_STATIC=0 -DOSSIA_CPP_ONLY=1 $repoRoot > $LogFile
  CheckLastExitCode

  # now configure 32 bit version
  cd ..
  mkdir build-32bit
  cd build-32bit

  $LogFile = "$repoRoot\configure-opp-32bit.log"
  cmake $32bitgen -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install-32bit" -DOSSIA_STATIC=0 -DOSSIA_CPP_ONLY=1 $repoRoot > $LogFile
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "pd" ) {
  $LogFile = "$repoRoot\configure-pd.log"
  cmake $64bitgen -DOSSIA_PD_ONLY=1 -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" $repoRoot > $LogFile
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "pd-32bit" ) {
  $LogFile = "$repoRoot\configure-pd.log"
  cmake $32bitgen -DOSSIA_PD_ONLY=1 -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" $repoRoot > $LogFile
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "pd-test" ) {
  $LogFile = "$repoRoot\configure-pd.log"
  cmake $32bitgen -DOSSIA_PD_ONLY=1 -DOSSIA_TESTING=1 -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" $repoRoot > $LogFile
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "purrdata" ) {
  $LogFile = "$repoRoot\configure-pd.log"
  cmake $64bitgen -DOSSIA_PD_ONLY=1 -DOSSIA_PURR_DATA=1 -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" $repoRoot > $LogFile
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "purrdata-32bit" ) {
  $LogFile = "$repoRoot\configure-pd.log"
  cmake $32bitgen -DOSSIA_PD_ONLY=1 -DOSSIA_PURR_DATA=1 -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" $repoRoot > $LogFile
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "python" ) {
  $LogFile = "$repoRoot\configure-${env:APPVEYOR_BUILD_TYPE}-${env:platform}.log"

  if ( "${env:platform}" -eq "x64" ) {

    cd "C:\${env:python}-x64\Scripts"
    pip.exe install wheel
    pip.exe install twine

    cd $repoRoot\build
    cmake  $64bitgen -DPYTHON_EXECUTABLE:FILEPATH=C:\${env:python}-x64\python.exe -DPYTHON_LIBRARY=C:\${env:python}-x64\lib${env:python}.a -DOSSIA_PYTHON_ONLY=1 $repoRoot > $LogFile
    CheckLastExitCode
  } else {

    cd "C:\${env:python}\Scripts"
    pip.exe install wheel
    pip.exe install twine

    cd $repoRoot\build
    cmake $32bitgen -DPYTHON_EXECUTABLE:FILEPATH=C:\${env:python}\python.exe -DPYTHON_LIBRARY=C:\${env:python}\lib${env:python}.a -DOSSIA_PYTHON_ONLY=1 $repoRoot > $LogFile
    CheckLastExitCode
  }

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "qml" ) {
  if ( Test-Path ${env:QTDIR}\bin\ ) {
    set $env:PATH=${env:QTDIR}\bin;${env:PATH};
  }

  $LogFile = "$repoRoot\configure-${env:APPVEYOR_BUILD_TYPE}.log"
  cmake $64bitgen -DCMAKE_INSTALL_PREFIX="${env:APPVEYOR_BUILD_FOLDER}/install" -DCMAKE_PREFIX_PATH="${env:QTDIR}\lib\cmake\Qt5" -DOSSIA_QML_ONLY=1 $repoRoot > $LogFile
  CheckLastExitCode
}
