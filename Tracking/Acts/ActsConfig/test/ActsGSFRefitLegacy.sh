#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# single electron pt 10 GeV
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-04-00-00/mc21_14TeV.900494.PG_single_epm_Pt10_etaFlatnp0_43.recon.RDO.e8481_s4494_r16632/RDO.45451600._000028.pool.root.1
n_events=100

ignore_pattern="Acts.+ERROR.+Propagation.+reached.+the.+step.+count.+limit"

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
   --preExec "flags.Exec.FPE=-1;" "flags.Acts.GsfRefitLegacyTrk=True;" \
   --preInclude egammaConfig.ConfigurationHelpers.egammaOnlyFromRaw \
   --autoConfiguration="everything" \
   --inputRDOFile ${input_rdo} \
   --outputAODFile AOD.pool.root \
   --maxEvents ${n_events} \
   --ignorePatterns "${ignore_pattern}" \
   --multithreaded

