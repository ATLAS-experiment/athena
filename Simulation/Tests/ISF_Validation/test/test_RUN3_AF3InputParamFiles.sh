#!/bin/sh
#
# art-description: MC21-style simulation using FullG4 for producing input samples needed for the Fast Calorimeter Simulation parametrisation
# art-type: build
# art-include: 24.0/Athena
# art-include: main/Athena

# Full chain with special flags
# Deactivated G4Optimizations: MuonFieldOnlyInCalo, NRR, PRR, FrozenShowers
# ATLAS-R3S-2021-03-02-00 and OFLCOND-MC23-SDR-RUN3-01

Sim_tf.py \
    --CA \
    --simulator 'FullG4MT'  \
    --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-01' \
    --postInclude 'all:PyJobTransforms.UseFrontier' 'EVNTtoHITS:ISF_FastCaloSimSD.ISF_FastCaloSimSDToolConfig.PostIncludeParametrizationInputSim_1mm' \
    --preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoV,ISF_FastCaloSimParametrization.ISF_FastCaloSimParametrizationConfig.ISF_FastCaloSimParametrization_SimPreInclude' \
    --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc15_13TeV.431004.ParticleGun_pid22_E65536_disj_eta_m25_m20_20_25_zv_0.evgen.EVNT.e6556.EVNT.13283012._000001.pool.root.1" \
    --outputHITSFile "Hits.CA.pool.root" \
    --postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
    --maxEvents=2

rc=$?
status=$rc
echo  "art-result: $rc simCA"

rc2=-9999
if [ $rc -eq 0 ]
then
    Reco_tf.py \
        --CA \
        --inputHITSFile "Hits.CA.pool.root" \
        --outputRDOFile RDO.CA.pool.root \
        --outputESDFile ESD.CA.pool.root \
        --conditionsTag "default:OFLCOND-MC23-SDR-RUN3-01" \
        --geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
        --preInclude 'all:Campaigns.MC23NoPileUp' \
        --preExec 'all:flags.LAr.ROD.NumberOfCollisions=20;flags.LAr.ROD.UseHighestGainAutoCorr=True;' 'HITtoRDO:flags.Digitization.DoCaloNoise=False' 'RAWtoALL:flags.Reco.EnableTrigger=False' \
        --postInclude 'all:PyJobTransforms.UseFrontier' 'HITtoRDO:ISF_FastCaloSimParametrization.ISF_FastCaloSimParametrizationConfig.PostIncludeISF_FastCaloSimParametrizationDigi' 'RAWtoALL:ISF_FastCaloSimParametrization.ISF_FastCaloSimParametrizationConfig.PostIncludeISF_FastCaloSimParametrizationReco' \
        --maxEvents -1 \
        --autoConfiguration everything
    rc2=$?
    if [ $status -eq 0 ]
    then
        status=$rc2
    fi
fi
echo  "art-result: $rc2 reco_simCA"

rc3=-9999
if [ $rc2 -eq 0 ]
then
    FCS_Ntup_tf.py \
        --CA \
        --inputESDFile 'ESD.CA.pool.root' \
        --outputNTUP_FCSFile 'calohit.CA.root' \
        --doG4Hits true
    rc3=$?
    if [ $status -eq 0 ]
    then
        status=$rc3
    fi
fi

echo  "art-result: $rc3 fcs_ntupCA"
exit $status
