#!/bin/sh
#
# art-description: MC20-style simulation of decaying staus using FullG4_LongLived (tests the Sleptons + Gauginos packages)
# art-type: grid
# art-architecture:  '#x86_64-intel'
# art-include: 23.0/Athena
# art-include: 23.0/AthSimulation
# art-include: main/Athena
# art-include: main/AthSimulation
# art-output: *.root
# art-output: PDGTABLE.MeV.*
# art-output: *.HITS.pool.root
# art-output: log.*
# art-output: Config*.pkl

# MC16 setup
# ATLAS-R2-2016-01-00-01 and OFLCOND-MC16-SDR-14
Sim_tf.py \
--CA \
--conditionsTag 'default:OFLCOND-MC16-SDR-14' \
--truthStrategy 'MC15aPlusLLP' \
--simulator 'FullG4MT_QS' \
--postInclude 'PyJobTransforms.UseFrontier' \
--preInclude 'EVNTtoHITS:Campaigns.MC16Simulation' \
--preExec 'EVNTtoHITS:ConfigFlags.Input.SpecialConfiguration={"NonInteractingPDGCodes":"[51,52,-53,53]"}' \
--geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
--inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mc16_13TeV.999999.DMsimp_s_spin1_SVJ.test.EVNT.pool.root" \
--outputHITSFile "CA.HITS.pool.root" \
--maxEvents 10 \
--postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
--imf False

rc=$?
mv PDGTABLE.MeV PDGTABLE.MeV.CA
mv log.EVNTtoHITS log.EVNTtoHITS.CA
echo  "art-result: $rc simCA"
status=$rc

Sim_tf.py \
    --conditionsTag 'default:OFLCOND-MC16-SDR-14' \
    --truthStrategy 'MC15aPlusLLP' \
    --simulator 'FullG4MT_QS' \
    --postExec 'EVNTtoHITS:ServiceMgr.ISF_InputConverter.GenParticleFilters["ISF_GenParticleInteractingFilter"].AdditionalNonInteractingParticleTypes=[51,52,-53,53]' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC16Simulation.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py' \
    --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mc16_13TeV.999999.DMsimp_s_spin1_SVJ.test.EVNT.pool.root" \
    --outputHITSFile "CA.HITS.pool.root" \
    --maxEvents 10 \
    --imf False \
    --athenaopts '"--config-only=ConfigSimCG.pkl"'

Sim_tf.py \
    --conditionsTag 'default:OFLCOND-MC16-SDR-14' \
    --truthStrategy 'MC15aPlusLLP' \
    --simulator 'FullG4MT_QS' \
    --postExec 'EVNTtoHITS:ServiceMgr.ISF_InputConverter.GenParticleFilters["ISF_GenParticleInteractingFilter"].AdditionalNonInteractingParticleTypes=[51,52,-53,53]' \
    --postInclude 'default:PyJobTransforms/UseFrontier.py' \
    --preInclude 'EVNTtoHITS:Campaigns/MC16Simulation.py,SimulationJobOptions/preInclude.ExtraParticles.py,SimulationJobOptions/preInclude.G4ExtraProcesses.py' \
    --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/mc16_13TeV.999999.DMsimp_s_spin1_SVJ.test.EVNT.pool.root" \
    --outputHITSFile "CG.HITS.pool.root" \
    --maxEvents 10 \
    --imf False

rc2=$?
status=$rc2
mv PDGTABLE.MeV PDGTABLE.MeV.CG
mv log.EVNTtoHITS log.EVNTtoHITS.CG
echo  "art-result: $rc2 simOLD"

rc3=-9999
if [ $rc -eq 0 ] && [ $rc2 -eq 0 ]
then
    acmd.py diff-root CG.HITS.pool.root CA.HITS.pool.root --error-mode resilient --mode=semi-detailed --order-trees --ignore-leaves RecoTimingObj_p1_EVNTtoHITS_timings index_ref
    rc3=$?
    status=$rc3
fi
echo  "art-result: $rc3 OLDvsCA"

rc4=-9999
if [ $rc2 -eq 0 ]
then
    ArtPackage=$1
    ArtJobName=$2
    art.py compare grid --entries 10 ${ArtPackage} ${ArtJobName} --mode=semi-detailed --file=CG.HITS.pool.root
    rc4=$?
    status=$rc4
fi
echo  "art-result: $rc4 regression"

exit $status
