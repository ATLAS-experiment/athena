#/bin/bash
#
# Copyright (C) 2002-2024  CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


set -e
set -u

dataType=$1
shift

inputFile=$2
shift

rm -rf physlite_test_$dataType
mkdir physlite_test_$dataType
cd physlite_test_$dataType

# This is the transform to run for producing PHYSLITE from AOD, whereas
# the line below runs the transform on PHYS.
# Reco_tf.py --maxEvents=500 --inputAODFile $input_file --outputDAODFile test_file.pool.root --reductionConf PHYS PHYSLITE --preExec 'default:from AthenaCommon.DetFlags import DetFlags; DetFlags.detdescr.all_setOff(); DetFlags.BField_setOn(); DetFlags.digitize.all_setOff(); DetFlags.detdescr.Calo_setOn(); DetFlags.simulate.all_setOff(); DetFlags.pileup.all_setOff(); DetFlags.overlay.all_setOff();DetFlags.detdescr.Muon_setOn();' --postExec 'from DerivationFrameworkJetEtMiss.JetCommon import swapAlgsInSequence;swapAlgsInSequence(topSequence,"jetalg_ConstitModCorrectPFOCSSKCHS_GPFlowCSSK", "UFOInfoAlgCSSK" );' 2>&1 | tee mylog.txt
Derivation_tf.py --CA --maxEvents 500 --inputDAOD_PHYSFile $inputFile --outputD2AODFile test_mc.root --formats PHYSLITE 2>&1 | tee my_log_physlite.txt

inputPhys=/data/krumnack/test_files/DAOD_PHYS.test_file.pool.root 
inputPhyslite=/data/krumnack/test_files/DAOD_PHYSLITE.test_file.pool.root

athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py --evtMax=500 - --data-type $dataType --no-physlite-broken --force-input $inputPhys --force-output test_ntuple_phys_$dataType.root "$@"
athena.py AnalysisAlgorithmsConfig/FullCPAlgorithmsTest_jobOptions.py --evtMax=500 - --data-type $dataType --physlite --no-physlite-broken --force-input $inputPhyslite --force-output test_ntuple_physlite_$dataType.root "$@"

acmd.py diff-root -t analysis test_ntuple_phys_$dataType.root test_ntuple_physlite_$dataType.root
