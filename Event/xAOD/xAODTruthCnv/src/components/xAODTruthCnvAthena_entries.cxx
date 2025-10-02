// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "../HepMCTruthReader.h"
#include "../RedoTruthLinksAlg.h"
#include "../xAODTruthCnvAlg.h"
#include "../xAODTruthReader.h"

DECLARE_COMPONENT(HepMCTruthReader)
DECLARE_COMPONENT(xAODMaker::RedoTruthLinksAlg)
DECLARE_COMPONENT(xAODMaker::xAODTruthCnvAlg)
DECLARE_COMPONENT(xAODReader::xAODTruthReader)
