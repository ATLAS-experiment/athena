#!/bin/sh
#
# art-description: Runs ZDC reconstruction, including time and energy calibration, on ZDCCalib stream, using 2015 HI data.
# art-athena-mt: 8
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8

athena ZdcRec/ZdcRecConfig.py --filesInput=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ZdcRec/data15_hi.00287321.calibration_zdcCalib.daq.RAW._lb0000._SFO-2._0001.data --evtMax=10

#Remember retval of transform as art result
RES=$?
xAODDigest.py AOD.pool.root digest.txt
echo "art-result: $RES reco"

