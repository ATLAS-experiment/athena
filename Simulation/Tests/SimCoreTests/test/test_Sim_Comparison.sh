#!/bin/sh
#
# art-description: Simulation example to test agreement between ComponentAccumulator script and job transform configurations.
# art-include: 24.0/Athena
# art-include: main/Athena
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-output: test.HITS.pool.root
# art-output: myHITSnew.pool.root

# ComponentAccumulator Script
G4AtlasAlgConfig_Test.py
rc=$?
status=$rc
echo "art-result: $rc CAsim_script"

# Job Transform
AtlasG4_tf.py \
    --CA \
    --conditionsTag 'default:OFLCOND-MC16-SDR-14' \
    --physicsList 'FTFP_BERT_ATL' \
    --truthStrategy 'MC15aPlus' \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --postInclude 'default:PyJobTransforms.UseFrontier' \
    --preExec 'AtlasG4Tf:flags.Sim.TightMuonStepping=True;from SimulationConfig.SimEnums import CalibrationRun;flags.Sim.CalibrationRun=CalibrationRun.Off;from SimuJobTransforms.SimulationHelpers import enableBeamPipeKill,enableFrozenShowersFCalOnly;enableBeamPipeKill(flags);enableFrozenShowersFCalOnly(flags)' \
    --DataRunNumber '284500' \
    --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1" \
    --outputHITSFile "test.HITS.pool.root" \
    --maxEvents 4 \
    --imf False
rc2=$?
if [ $status -eq 0 ]
then
    status=$rc2
fi
echo "art-result: $rc2 simCA"

rc3=-9999
if [ $status -eq 0 ]
then
    acmd.py diff-root test.HITS.pool.root myHITSnew.pool.root --mode=semi-detailed --order-trees --error-mode resilient
    rc3=$?
    status=$rc3
fi
echo "art-result: $rc3 script_vs_tf"

exit $status
