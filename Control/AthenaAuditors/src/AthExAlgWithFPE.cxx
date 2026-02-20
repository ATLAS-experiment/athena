//  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#include "AthExAlgWithFPE.h"

StatusCode AthExAlgWithFPE::execute() {

  float value = 42;
  float byZero=divide (value, 0);
  ATH_MSG_INFO("Division of " << value << " by zero is " << byZero);

  return StatusCode::SUCCESS;
}
