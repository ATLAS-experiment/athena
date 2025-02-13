#!/bin/bash
#
# art-description: Simulation test with R4 MS geometry + ITK
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-architecture:  '#x86_64-intel'
# art-output: log.*
# art-output: MuonSimHitNtuple.root
# art-output: SimHitsR4.pool.root


geo_db="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/ATLAS-R4-MUONTEST.db"
geo_tag=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN4)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
validNTuple="MuonSimHitNtuple.root"

export ATHENA_PROC_NUMBER=8
export ATHENA_CORE_NUMBER=8
Sim_tf.py \
      --CA True \
      --multithreaded True \
      --geometrySQLite True \
      --geometrySQLiteFullPath "${geo_db}" \
      --conditionsTag "default:${conditions}" \
      --simulator 'FullG4MT_QS' \
      --postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
      --preExec "all:flags.Scheduler.CheckDependencies = True;flags.Scheduler.ShowDataDeps = True;flags.Scheduler.ShowDataFlow = True;flags.Scheduler.ShowControlFlow = True;" \
      --postExec "all:flags.dump(evaluate=True);from MuonPRDTestR4.MuonHitTestConfig import MuonHitTesterCfg;cfg.merge(MuonHitTesterCfg(flags,dumpSimHits=True, outFile=\"${validNTuple}\"));cfg.printConfig(withDetails=True, summariseProps=True);" \
      --geometryVersion "default:${geo_tag}" \
      --inputEVNTFile '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/EVNT/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8453/EVNT.29328277._003902.pool.root.1' \
      --outputHITSFile 'SimHitsR4.pool.root' \
      --maxEvents 100 \
      --skipEvents 0 \
      --randomSeed 10 \
      --imf False

rc=$?
echo  "art-result: $rc simulation"

exit ${rc}
