#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

AOD_File=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN3_MC[0])")

Derivation_tf.py \
    --formats "PHYSVAL" \
    --inputAODFile ${AOD_File} \
    --outputDAODFile "OUT.root" \
    --maxEvents 10

Derivation_tf.py \
    --inputDAOD_PHYSVALFile "DAOD_PHYSVAL.OUT.root" \
    --outputNTUP_PHYSVALFile "NTUP_PHYSVAL.root" \
    --validationFlags doExample,doMET,doEgamma,doInDet,doTau,doJet,doBtag,doMuon,doTopoCluster,doPFlow_FlowElements,doLLPSecVtx,doDiTau \
    --format NTUP_PHYSVAL \
    --maxEvents 10
