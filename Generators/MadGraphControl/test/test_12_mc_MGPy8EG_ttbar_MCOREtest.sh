#!/bin/sh

# art-include: main/AthGeneration
# art-description: MadGraph Event Generation Test - Multicore LO
# art-type: grid
# art-athena-mt: 8
# art-output: EVNT.root

# On the grid, this will happen in the pilot... adding this so that it gets done for local runs too
export ATHENA_CORE_NUMBER=8

Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputEVNTFile=EVNT.root --jobConfig=950112
echo "art-result: $? generation"

# Run tests on the log file
env -u PYTHONPATH -u PYTHONHOME python3 /cvmfs/atlas.cern.ch/repo/sw/Generators/MCJobOptions/scripts/logParser.py -s -i log.generate

echo "art-result: $? log-check"
