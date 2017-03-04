#!/bin/sh

VERSION=$1
CPU=$2
NODEBUG=$3

#launchctl load auto_build.plist
cd ~/geoda_trunk/
git checkout rc/GdaAppResources.cpp
git pull
cd ~/geoda_trunk/o
rm *
cd ~/geoda_trunk/BuildTools/macosx
./build.sh $CPU $NODEBUG
cd ~/geoda_trunk/BuiltTools/macosx/create-dmg
./geoda.sh $VERSION
mv GeoDa$VERSION-Installer.dmg ~/Dropbox
cd ..
