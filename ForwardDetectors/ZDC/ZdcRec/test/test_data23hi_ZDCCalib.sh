#!/bin/sh
#
# art-description: Runs ZDC reconstruction, including time and energy calibration, on ZDCCalib stream, using 2023 HI data.
# art-athena-mt: 8
# art-type: local
# art-include: main/Athena
# art-include: 24.0/Athena

export ATHENA_CORE_NUMBER=8

#athena ZdcRec/ZdcRecConfig.py --CA --filesInput /eos/atlas/atlascerngroupdisk/data-art/grid-input/ZdcRec/data23_hi.00463427.calibration_ZDCCalib.daq.RAW._lb0000._SFO-19._0001.data --evtMax=10
athena ZdcRec/ZdcRecConfig.py --CA --filesInput /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ZdcRec/data23_hi.00463427.calibration_ZDCCalib.daq.RAW._lb0000._SFO-19._0001.data --evtMax=10

#Remember retval of transform as art result
RES=$?
xAODDigest.py AOD.pool.root digest.txt
echo "art-result: $RES Reco"

