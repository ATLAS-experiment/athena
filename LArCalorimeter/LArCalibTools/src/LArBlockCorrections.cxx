/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArBlockCorrections.h"
#include "LArRawConditions/LArConditionsContainerBase.h"

StatusCode LArBlockCorrections::initialize() {
  bool setFlag =   LArConditionsContainerBase::applyCorrectionsAtInit(true, false);

  ATH_MSG_INFO ( "LArConditionsContainerBase::applyCorrectionsAtInit set to " << setFlag );

  return StatusCode::SUCCESS;
}

