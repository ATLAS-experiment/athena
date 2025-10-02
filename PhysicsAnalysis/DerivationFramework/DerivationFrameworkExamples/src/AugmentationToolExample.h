/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_AUGMENTATIONTOOLEXAMPLE_H
#define DERIVATIONFRAMEWORK_AUGMENTATIONTOOLEXAMPLE_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

namespace DerivationFramework {

  class AugmentationToolExample : public extends<AthAlgTool, IAugmentationTool> {
    public:
      using base_class::base_class;

      virtual StatusCode addBranches() const;
  }; 
}

#endif // DERIVATIONFRAMEWORK_AUGMENTATIONTOOLEXAMPLE_H
