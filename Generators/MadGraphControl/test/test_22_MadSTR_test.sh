#!/bin/sh

# art-include: main/AthGeneration
# art-description: MadGraph Event Generation Test - SUSY gen from LHE with MadSpin
# art-type: grid
# art-output: test_lhe_events.tar.gz
# art-output: output_hists.root
# art-output: dcube
# art-html: dcube

Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputEVNTFile=EVNT.root --jobConfig=421499
echo "art-result: $? Gen_tf"

# Run tests on the log file
env -u PYTHONPATH -u PYTHONHOME python3 /cvmfs/atlas.cern.ch/repo/sw/Generators/MCJobOptions/scripts/logParser.py -s -i log.generate
echo "art-result: $? log-check"