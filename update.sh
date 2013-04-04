#!/bin/sh

export https_proxy="http://wwwcache.gla.ac.uk:8080"
git pull https://github.com/richardmitic/drum.git
qmake
make