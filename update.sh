#!/bin/sh

export https_proxy="http://wwwcache.gla.ac.uk:8080"
git pull https://github.com/richardmitic/rpi.git
qmake
make