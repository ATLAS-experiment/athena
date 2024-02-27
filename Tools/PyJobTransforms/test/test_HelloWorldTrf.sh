#!/bin/bash

set -e

# ST test
HelloWorld_tf.py --maxEvents=5

grep 'runArgs.threads = 0' runargs.athena.py
grep 'runArgs.concurrentEvents = 0' runargs.athena.py

# MT test 1
HelloWorld_tf.py --maxEvents=5 --athenaopts="--threads=2 --concurrent-events=1"

grep 'runArgs.threads = 2' runargs.athena.py
grep 'runArgs.concurrentEvents = 1' runargs.athena.py

# MT test 2
ATHENA_CORE_NUMBER=2 HelloWorld_tf.py --maxEvents=5 --multithreaded

grep 'runArgs.threads = 2' runargs.athena.py
grep 'runArgs.concurrentEvents = 2' runargs.athena.py

# MT test 3
ATHENA_CORE_NUMBER=2 HelloWorld_tf.py --maxEvents=5 --multithreaded=True --multiprocess=False

grep 'runArgs.threads = 2' runargs.athena.py
grep 'runArgs.concurrentEvents = 2' runargs.athena.py

# MP test
ATHENA_CORE_NUMBER=2 HelloWorld_tf.py --maxEvents=5 --multiprocess=True

grep 'runArgs.nprocs = 2' runargs.athena.py
grep 'runArgs.threads = 0' runargs.athena.py
grep 'runArgs.concurrentEvents = 0' runargs.athena.py

# postInclude/Exec
HelloWorld_tf.py --maxEvents=5 --postInclude 'AthenaConfiguration.JobOptsDumper.JobOptsDumperCfg,PyJobTransforms.DumpPickle'

# CA arg test 1
HelloWorld_tf.py --maxEvents=5 --CA HelloWorld:True

# CA arg test 2
HelloWorld_tf.py --maxEvents=5 --CA HelloWorld:True HelloWorldSecond:False

# CA arg test 3
set +e

if HelloWorld_tf.py --CA False --maxEvents=5; then
  exit 1
fi
