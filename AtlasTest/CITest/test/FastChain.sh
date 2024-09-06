#!/usr/bin/bash
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

FastChain_tf.py \
    --simulator ATLFAST3F_G4MS \
    --useISF True \
    --randomSeed 123 \
    --inputEVNTFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ISF_Validation/mc12_valid.110401.PowhegPythia_P2012_ttbar_nonallhad.evgen.EVNT.e3099.01517252._000001.pool.root.1" \
    --inputRDO_BKGFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/PresampledPileUp/22.0/Run2/large/mc20_13TeV.900149.PG_single_nu_Pt50.digit.RDO.e8307_s3482_s3136_d1713/RDO.26811885._035498.pool.root.1" \
    --outputRDOFile RDO_CG.pool.root \
    --maxEvents 2 \
    --skipEvents 0 \
    --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
    --conditionsTag 'default:OFLCOND-MC16-SDR-RUN2-12'  \
    --preInclude 'Campaigns.MC20a' 'Campaigns.MC16SimulationNoIoV' \
    --postInclude 'PyJobTransforms.UseFrontier' \
    --imf False
