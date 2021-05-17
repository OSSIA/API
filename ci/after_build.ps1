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

if ( $env:APPVEYOR_BUILD_TYPE -eq "testing" ){

  # add Qt/bin dir to PATH because tests need Qt DLL.
  ls ${env:QTDIR}\bin
  [Environment]::SetEnvironmentVariable("Path",$env:Path + ";${env:QTDIR}/bin/","Process")


  cd ${env:APPVEYOR_BUILD_FOLDER}\build

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install.log"
  cmake --build . --config "${env:configuration}" --target install > "$LogFile"
  CheckLastExitCode

  if ( $env:configuration -eq "Release" ){
    copy ${env:APPVEYOR_BUILD_FOLDER}\build\OSSIA\Release\ossia.dll "${env:APPVEYOR_BUILD_FOLDER}\build\Tests\Release\"
  } else {
    copy ${env:APPVEYOR_BUILD_FOLDER}\build\OSSIA\Debug\ossiad.dll "${env:APPVEYOR_BUILD_FOLDER}\build\Tests\Debug\"
  }

  echo "============================================================"
  ls "${env:APPVEYOR_BUILD_FOLDER}\build\Tests\${env:configuration}\"
  echo "============================================================"

  copy ${env:APPVEYOR_BUILD_FOLDER}\install\Ossia ${env:QTDIR}\QML

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\test.log"
  cmake --build . --config "${env:configuration}" --target RUN_TESTS > "$LogFile"
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "Release" ){

  cd ${env:APPVEYOR_BUILD_FOLDER}\build
  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-${env:APPVEYOR_BUILD_TYPE}-win64.log"
  cmake --build . --config Release --target install > "$LogFile"
  CheckLastExitCode

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-debug-win64.log"
  cmake --build . --config Debug --target install > "$LogFile"
  CheckLastExitCode

  cd ${env:APPVEYOR_BUILD_FOLDER}\install
  ls
  ls lib
  ls bin

  cd ${env:APPVEYOR_BUILD_FOLDER}\build-32bit
  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-${env:APPVEYOR_BUILD_TYPE}-win32.log"
  cmake --build . --config Release --target install > "$LogFile"
  CheckLastExitCode

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-debug-win32.log"
  cmake --build . --config Debug --target install > "$LogFile"
  CheckLastExitCode

  cd ${env:APPVEYOR_BUILD_FOLDER}\install
  ls
  ls lib
  ls bin
  
  7z a ${env:APPVEYOR_BUILD_FOLDER}\libossia-native-win.zip .


} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "ossia-cpp" ){

  cd ${env:APPVEYOR_BUILD_FOLDER}\build
  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-${env:APPVEYOR_BUILD_TYPE}-win64.log"
  cmake --build . --config Release --target install > "$LogFile"
  CheckLastExitCode

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-debug-win64.log"
  cmake --build . --config Debug --target install > "$LogFile"
  CheckLastExitCode

  cd ${env:APPVEYOR_BUILD_FOLDER}\build-32bit
  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-${env:APPVEYOR_BUILD_TYPE}-win32.log"
  cmake --build . --config Release --target install > "$LogFile"
  CheckLastExitCode

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-debug-win32.log"
  cmake --build . --config Debug --target install > "$LogFile"
  CheckLastExitCode

  cd ${env:APPVEYOR_BUILD_FOLDER}\install
  7z a ${env:APPVEYOR_BUILD_FOLDER}\ossia-cpp-win.zip .

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "pd" ){
  cd ${env:APPVEYOR_BUILD_FOLDER}\build

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-pd.log"
  cmake --build . --config "${env:configuration}" --target install > "$LogFile"
  CheckLastExitCode

  cd ${env:APPVEYOR_BUILD_FOLDER}\install\ossia-pd-package\
  ls .

  7z a ${env:APPVEYOR_BUILD_FOLDER}\ossia-pd-win64.zip ossia

  if (${env:APPVEYOR_REPO_TAG}){
    $env:PATH="C:\msys64\usr\bin;${env:PATH}"

    $VERSION=${env:APPVEYOR_REPO_TAG_NAME}

    nuget install secure-file -ExcludeVersion
    secure-file\tools\secure-file -decrypt ${env:APPVEYOR_BUILD_FOLDER}\ci\codesigning.asc.appveyor.enc -out ${env:APPVEYOR_BUILD_FOLDER}\ci\codesigning.asc -secret ${env:GPG_DECODE_KEY}

    gpg.exe --fast-import ${env:APPVEYOR_BUILD_FOLDER}\ci\codesigning.asc

    curl.exe --user "ossia:${env:DEKEN_PASSWORD}" -X MKCOL  "https://puredata.info/Members/ossia/software/ossia/${VERSION}/"
    $ARCHIVE_NAME="ossia-v${VERSION}-(Windows-amd64-32)-externals.zip"
    copy ${env:APPVEYOR_BUILD_FOLDER}\ossia-pd-win64.zip ${ARCHIVE_NAME}

    gpg.exe -ab --batch --yes ${ARCHIVE_NAME}
    $SHA = sha256.exe ${ARCHIVE_NAME}
    $SHA.substring(0,64) > "${ARCHIVE_NAME}.sha"

    curl.exe --user ossia:${env:DEKEN_PASSWORD} -T "${ARCHIVE_NAME}"     "https://puredata.info/Members/ossia/software/ossia/${VERSION}/${ARCHIVE_NAME}"     --basic
    curl.exe --user ossia:${env:DEKEN_PASSWORD} -T "${ARCHIVE_NAME}.asc" "https://puredata.info/Members/ossia/software/ossia/${VERSION}/${ARCHIVE_NAME}.asc" --basic
    curl.exe --user ossia:${env:DEKEN_PASSWORD} -T "${ARCHIVE_NAME}.sha" "https://puredata.info/Members/ossia/software/ossia/${VERSION}/${ARCHIVE_NAME}.sha" --basic
  }
} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "pd-32bit" ){
  cd ${env:APPVEYOR_BUILD_FOLDER}\build

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-pd.log"
  cmake --build . --config "${env:configuration}" --target install > "$LogFile"
  CheckLastExitCode

  cd ${env:APPVEYOR_BUILD_FOLDER}\install\ossia-pd-package\
  ls .

  7z a ${env:APPVEYOR_BUILD_FOLDER}\ossia-pd-win32.zip ossia

  if (${env:APPVEYOR_REPO_TAG}){
    $env:PATH="C:\msys64\usr\bin;${env:PATH}"

    $VERSION=${env:APPVEYOR_REPO_TAG_NAME}

    nuget install secure-file -ExcludeVersion
    secure-file\tools\secure-file -decrypt ${env:APPVEYOR_BUILD_FOLDER}\ci\codesigning.asc.appveyor.enc -out ${env:APPVEYOR_BUILD_FOLDER}\ci\codesigning.asc -secret ${env:GPG_DECODE_KEY}

    gpg.exe --fast-import ${env:APPVEYOR_BUILD_FOLDER}\ci\codesigning.asc

    curl.exe --user "ossia:${env:DEKEN_PASSWORD}" -X MKCOL  "https://puredata.info/Members/ossia/software/ossia/${VERSION}/"
    $ARCHIVE_NAME="ossia-v${VERSION}-(Windows-i386-32)-externals.zip"
    copy ${env:APPVEYOR_BUILD_FOLDER}\ossia-pd-win32.zip ${ARCHIVE_NAME}

    gpg.exe -ab --batch --yes ${ARCHIVE_NAME}
    $SHA = sha256.exe ${ARCHIVE_NAME}
    $SHA.substring(0,64) > "${ARCHIVE_NAME}.sha"

    curl.exe --user ossia:${env:DEKEN_PASSWORD} -T "${ARCHIVE_NAME}"     "https://puredata.info/Members/ossia/software/ossia/${VERSION}/${ARCHIVE_NAME}"     --basic
    curl.exe --user ossia:${env:DEKEN_PASSWORD} -T "${ARCHIVE_NAME}.asc" "https://puredata.info/Members/ossia/software/ossia/${VERSION}/${ARCHIVE_NAME}.asc" --basic
    curl.exe --user ossia:${env:DEKEN_PASSWORD} -T "${ARCHIVE_NAME}.sha" "https://puredata.info/Members/ossia/software/ossia/${VERSION}/${ARCHIVE_NAME}.sha" --basic
  }
} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "pd-test" ){
  cd ${env:APPVEYOR_BUILD_FOLDER}\build

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-pd.log"
  cmake --build . --config "${env:configuration}" --target install > "$LogFile"
  CheckLastExitCode

  mkdir -p ${env:HOME}\Documents\Pd\externals

  mv ${env:APPVEYOR_BUILD_FOLDER}\install\ossia-pd-package\ossia ${env:HOME}\Documents\Pd\externals\

  # add pd/bin folder to environment path variable
  [Environment]::SetEnvironmentVariable("Path",$env:Path + ";${env:APPVEYOR_BUILD_FOLDER}/build/src/ossia-pd/pd/bin/","Process")

  cd ${env:APPVEYOR_BUILD_FOLDER}\build

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\test-pd.log"
  cmake --build . --config "${env:configuration}" --target RUN_TESTS > "$LogFile"
  CheckLastExitCode

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "purrdata-32bit" ){
  cd ${env:APPVEYOR_BUILD_FOLDER}\build

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-pd.log"
  cmake --build . --config "${env:configuration}" --target install > "$LogFile"
  CheckLastExitCode

  cd ${env:APPVEYOR_BUILD_FOLDER}\install\ossia-pd-package\
  ls .

  7z a ${env:APPVEYOR_BUILD_FOLDER}\ossia-purrdata-win32.zip ossia

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "purrdata" ){
  cd ${env:APPVEYOR_BUILD_FOLDER}\build

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-pd.log"
  cmake --build . --config "${env:configuration}" --target install > "$LogFile"
  CheckLastExitCode

  cd ${env:APPVEYOR_BUILD_FOLDER}\install\ossia-pd-package\
  ls .

  7z a ${env:APPVEYOR_BUILD_FOLDER}\ossia-purrdata-win64.zip ossia

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "qml" ){
  cd ${env:APPVEYOR_BUILD_FOLDER}\build

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-qml.log"
  cmake --build . --config Release --target install > "$LogFile"
  CheckLastExitCode

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-qml-debug.log"
  cmake --build . --config Debug --target install > "$LogFile"
  CheckLastExitCode

  cd ${env:APPVEYOR_BUILD_FOLDER}\install\
  ls

  7z a ${env:APPVEYOR_BUILD_FOLDER}\ossia-qml-win64.zip .

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "python" ){

  cd ${env:APPVEYOR_BUILD_FOLDER}\build

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-python.log"
  cmake --build . --config "${env:configuration}" --target install > "$LogFile"
  CheckLastExitCode

  if (${env:APPVEYOR_REPO_TAG}){
    if ( "${env:platform}" -eq "x64" ){
      cd "C:\${env:python}-x64\"
      python.exe -m twine upload ${env:APPVEYOR_BUILD_FOLDER}/build/src/ossia-python/dist/pyossia*.whl
    } else {
      cd "C:\${env:python}\"
      python.exe -m twine upload ${env:APPVEYOR_BUILD_FOLDER}/build/src/ossia-python/dist/pyossia*.whl
    }
  }

  mv ${env:APPVEYOR_BUILD_FOLDER}/build/src/ossia-python/dist/pyossia*.whl ${env:APPVEYOR_BUILD_FOLDER}/

} elseif ( $env:APPVEYOR_BUILD_TYPE -eq "max" ){

  cd ${env:APPVEYOR_BUILD_FOLDER}\build

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-max.log"
  cmake --build . --config "${env:configuration}" --target install > "$LogFile"
  CheckLastExitCode

  cd ${env:APPVEYOR_BUILD_FOLDER}\build-32bit

  $LogFile = "${env:APPVEYOR_BUILD_FOLDER}\install-max-32bit.log"
  cmake --build . --config "${env:configuration}" --target install > "$LogFile"
  CheckLastExitCode

  cd ${env:APPVEYOR_BUILD_FOLDER}\install\ossia-max-package\
  ls

  7z a ${env:APPVEYOR_BUILD_FOLDER}\ossia-max-win.zip .
}
