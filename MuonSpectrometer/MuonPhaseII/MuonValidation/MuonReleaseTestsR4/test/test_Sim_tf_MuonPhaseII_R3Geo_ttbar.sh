#!/bin/bash
#
# art-description: Simulation test with R3 MS geometry + ID
# art-type: grid
# art-include: main/Athena
# art-architecture:  '#x86_64-intel'
# art-athena-mt: 8
# art-output: log.*
# art-output: MuonSimHitNtuple.root
# art-output: SimHitsR3.pool.root


GEOMODEL_DB_FILE=$(python -c "from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults; print(MuonPhaseIITestDefaults.GEODB_R3)")
ATLAS_CONDDB_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
ATLAS_GEO_TAG=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")

validNTuple="MuonSimHitNtuple.root"

export ATHENA_PROC_NUMBER=8
export ATHENA_CORE_NUMBER=8


export IN_FILE="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1"
NEVENTS=${1}
if [ ${NEVENTS} -eq ]; then
   NEVENTS=100
fi

Sim_tf.py \
    --CA \
    --multithreaded True \
    --geometrySQLite True \
    --geometrySQLiteFullPath ${GEOMODEL_DB_FILE} \
    --geometryVersion "default:${ATLAS_GEO_TAG}" \
    --conditionsTag "default:${ATLAS_CONDDB_TAG}" \
    --simulator 'FullG4MT_QS'  \
    --preInclude 'EVNTtoHITS:Campaigns.MC23aSimulationMultipleIoV' \
    --postInclude "PyJobTransforms.TransformUtils.UseFrontier" \
    --preExec "all:flags.Output.HISTFileName='ValidNtuple.sim.root';flags.Scheduler.CheckDependencies=True;flags.Scheduler.ShowDataDeps=True;flags.Scheduler.ShowDataFlow=True;flags.Scheduler.ShowControlFlow = True;flags.Exec.FPE= 500;" \
    --postExec "default:flags.dump(evaluate=True);from HitAnalysis.PostIncludes import SimHitAnalysis;cfg.merge(SimHitAnalysis(flags));cfg.printConfig(withDetails=True, summariseProps=True);" \
    --randomSeed 12345 \
    --inputEVNTFile ${IN_FILE} \
    --outputHitsFile SimHitsR3.pool.root \
    --firstEvent 0 \
    --skipEvents 0 \
    --maxEvents ${NEVENTS} \
    --imf False

rc=$?
echo  "art-result: $rc simulation"

exit ${rc}
