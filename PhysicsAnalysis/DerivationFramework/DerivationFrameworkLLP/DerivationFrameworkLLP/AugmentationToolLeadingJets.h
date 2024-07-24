/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// AugmentationLeadingJets.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_AUGMENTATIONTOOLLEADINGJETS_H
#define DERIVATIONFRAMEWORK_AUGMENTATIONTOOLLEADINGJETS_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

namespace DerivationFramework {

  class AugmentationToolLeadingJets : public AthAlgTool, public IAugmentationTool {
    public: 
      AugmentationToolLeadingJets(const std::string& t, const std::string& n, const IInterface* p);
      virtual StatusCode addBranches() const;
  }; 
}

#endif // DERIVATIONFRAMEWORK_AUGMENTATIONTOOLLEADINGJETS_H
