#!/bin/bash

# Simple script to configure and build ossia-max
# Available command optional arguments:
#    release : build in release mode
#    clean : remove any previous build folder and start from scratch
#    silent : silently install all needed dependencies

set -ex
export LANG=en_US.UTF-8

for var in "$@"
do
  if [[ $var = clean ]]; then
    OSSIA_CLEAN_BUILD=1
  elif	[[ $var = release ]]; then
  	OSSIA_BUILD_TYPE=release
  elif  [[ $var = silent ]]; then
    OSSIA_SILENT_INSTALL=true
  fi
done


if [ -z ${CI+x} ];
then
ask_permission()
{
if [[ $OSSIA_SILENT_INSTALL ]]
then 
  echo 1; return;
fi

while true; do
    read -p "'$1' is missing, do you want to install it?" yn
    case $yn in
        [Yy]* ) echo 1; break;;
        [Nn]* ) echo 0; break;;
        * ) echo "Please answer yes or no.";;
    esac
done
}

# install deps
command -v brew > /dev/null 2>&1 || [ $(ask_permission brew) -eq 1 ] && /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
command -v greadlink > /dev/null 2>&1 || [ $(ask_permission coreutils) -eq 1 ] && brew install coreutils
command -v ninja > /dev/null 2>&1 ||  [ $(ask_permission ninja) -eq 1 ] && brew install ninja
command -v cmake > /dev/null 2>&1 || [ $(ask_permission ninja) -eq 1 ] && brew install cmake
else
# install without asking persmission on CI
command -v brew > /dev/null 2>&1 || /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
command -v greadlink > /dev/null 2>&1 || brew install coreutils
command -v ninja > /dev/null 2>&1 || brew install ninja
command -v cmake > /dev/null 2>&1 || brew install cmake
fi # CI

OSSIA_BUILD_TYPE=debug

SCRIPT_FOLDER=`dirname "$(greadlink -f "$0")"`
REPO_ROOT=${SCRIPT_FOLDER}/../
OSSIA_BUILD_FOLDER=${REPO_ROOT}/build-ossia-max

if [ -z ${CI+x} ];
then
  INSTALL_FOLDER="${HOME}/Documents/Max 8/Packages/ossia"
else
  INSTALL_FOLDER="${REPO_ROOT}/artifacts/ossia"
fi

if [[ -n ${OSSIA_CLEAN_BUILD} ]]
then
  rm -rf ${OSSIA_BUILD_FOLDER}
fi

mkdir -p ${OSSIA_BUILD_FOLDER}
cd ${OSSIA_BUILD_FOLDER}
cmake -GNinja "${REPO_ROOT}" \
  -DCMAKE_BUILD_TYPE=${OSSIA_BUILD_TYPE} \
  -DOSSIA_MAX_INSTALL_FOLDER="${INSTALL_FOLDER}" \
  -DOSSIA_MAX_ONLY=1 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13
ninja 
ninja install