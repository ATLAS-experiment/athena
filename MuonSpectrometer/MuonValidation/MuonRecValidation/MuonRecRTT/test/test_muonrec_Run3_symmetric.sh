#!/bin/sh
#
# art-description: Test muon reconstruction on best knowledge symmetric Run3 layout
# 
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: trkPerformance_MuonSpectrometerTracks.txt
# art-output: trkPerformance_ExtrapolatedMuonTracks.txt
# art-output: trkPerformance_MSOnlyExtrapolatedMuonTracks.txt
# art-output: trkPerformance_CombinedMuonTracks.txt
# art-output: muonPerformance_segments.txt
# art-output: muonPerformance_xAOD.txt
# art-output: warningCount.txt
# art-output: NSWRecoCheck.txt
# art-output: NSWPRDValAlg.reco.ntuple.root

#####################################################################
# run reconstruction on 2000 di-muon events (0.9<|eta|<2.8) using the best knowledge symmetric Run3 layout (ATLAS-R3S-2021-01-00-02)
# the input RDO was produced from /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/EVNT_DiMuon_10k__AbsEta_09_28__Pt_10_1000GeV.root
# simulation/digitisation was executed in Athena,22.0.34 

Reco_tf.py --CA True \
           --inputRDOFile /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/Run3/RDO/RDO_DiMuon_Endcap_R3LatestLayout_sym_v1.root \
           --autoConfiguration everything \
           --preExec "flags.Detector.GeometryMM = True;flags.Detector.EnableMM = True" \
           --imf False \
           --postInclude MuonPRDTest.HitValAlgReco.HitValAlgRecoCfg \
           --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-11' \
           --outputESDFile OUT_ESD.root
exit_code=$?
echo  "art-result: ${exit_code} Reco_tf.py"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
# check the log file for WARNING/ERROR/FATAL
LOG_RECO="log.RAWtoALL"
NWARNING="$(cat ${LOG_RECO} | grep WARNING | wc -l)"
NERROR="$(cat ${LOG_RECO} | grep ERROR | wc -l)"
NFATAL="$(cat ${LOG_RECO} | grep FATAL | wc -l)"
echo "Found ${NWARNING} WARNING, ${NERROR} ERROR and ${NFATAL} FATAL messages in ${LOG_RECO}" > warningCount.txt
#####################################################################
# check the NSW validation ntuple
python $Athena_DIR/bin/checkNSWValTree.py -i NSWPRDValAlg.reco.ntuple.root --checkPRD &> NSWRecoCheck.txt
exit_code=$?
echo  "art-result: ${exit_code} NSWRecoCheck"
if [ ${exit_code} -ne 0 ]
then
    exit ${exit_code}
fi
#####################################################################

echo "art-result: $?"
