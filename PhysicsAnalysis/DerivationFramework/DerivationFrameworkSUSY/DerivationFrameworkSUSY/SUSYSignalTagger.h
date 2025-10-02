/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file DerivationFrameworkSUSY/SUSYSignalTagger.h
 * @author Martin Tripiana
 * @date May. 2015
 * @brief tool to decorate EventInfo with the SUSY signal process information
*/


#ifndef DerivationFramework_SUSYSignalTagger_H
#define DerivationFramework_SUSYSignalTagger_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "xAODTruth/TruthParticleContainer.h"

namespace DerivationFramework {

  class SUSYSignalTagger : public extends<AthAlgTool, IAugmentationTool> {

  public:
    SUSYSignalTagger(const std::string& t, const std::string& n, const IInterface* p);
    ~SUSYSignalTagger() = default;
    virtual StatusCode addBranches() const override;

  private:
    Gaudi::Property<std::string> m_eventInfoName{ this, "EventInfoName", "EventInfo", "Event Info Key"};
    Gaudi::Property<std::string> m_mcName{ this,"MCCollectionName", "TruthParticles", "MC Collection Key"};

    bool FindSusyHardProc(const xAOD::TruthParticleContainer* truthP, int& pdgid1, int& pdgid2) const;
  }; /// class

} /// namespace


#endif
