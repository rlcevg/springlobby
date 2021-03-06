#!/usr/bin/env bash

set -e
cd $(dirname ${0})/..

git submodule update --init

DIR=build-${1}
shift
REV=${1}
shift
echo -n creating ${DIR} ...

if [ ! -d ${DIR} ] ; then
	mkdir ${DIR}
	echo done
else
	echo skipped
fi

PARAMS="-DSPRINGLOBBY_REV=${REV} ${@}"

echo configuring ${DIR} with $PARAMS
cd ${DIR}
rm -fv CMakeCache.txt CPackConfig.cmake CPackSourceConfig.cmake
find -name cmake_install.cmake -delete
cmake $PARAMS ..

