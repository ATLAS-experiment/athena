#!/bin/bash

export CLING_STANDARD_PCH=none #See bug ROOT-10789
NOSEPATH=`python -c 'import nose; f=nose.__file__; i=f.find("/lib/"); print (f[0:i])'`
$NOSEPATH/bin/nosetests -v DQDefects
