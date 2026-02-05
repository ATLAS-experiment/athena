//  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

// This needs to be in a separate compilation unit.
// Otherwise it can be inlined and a division by zero handled
// at compilation time rathern than runtime.

#include "AthExAlgWithFPE.h"

float AthExAlgWithFPE::divide (float a, float b)
{
  return a/b;
}
