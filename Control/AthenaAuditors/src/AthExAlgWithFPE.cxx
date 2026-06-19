//  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#include "AthExAlgWithFPE.h"

StatusCode AthExAlgWithFPE::execute(const EventContext& /*ctx*/) {

  float value = 42;
  //coverity[DIVIDE_BY_ZERO]
  float byZero=divide (value, 0);
  ATH_MSG_INFO("Division of " << value << " by zero is " << byZero);

  return StatusCode::SUCCESS;
}
