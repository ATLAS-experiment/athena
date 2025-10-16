/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include "StoreGate/WriteDecorHandleKey.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthParticleContainer.h"

namespace DerivationFramework {

  class SUSYSignalTagger : public extends<AthAlgTool, IAugmentationTool> {

  public:
    SUSYSignalTagger(const std::string& t, const std::string& n, const IInterface* p);
    ~SUSYSignalTagger() = default;
    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override;

  private:
    bool FindSusyHardProc(const xAOD::TruthParticleContainer& truthP, int& pdgid1, int& pdgid2) const;

    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_mcName{ this,"MCCollectionName", "TruthParticles", "MC Collection Key"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoName{ this, "EventInfoName", "EventInfo", "Event Info Key"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_dec_procIDKey{this, "SUSY_procIDKey", m_eventInfoName, "SUSY_procID"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_dec_pdgId1Key{this, "SUSY_pid1Key", m_eventInfoName, "SUSY_pid1"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_dec_pdgId2Key{this, "SUSY_pid2Key",  m_eventInfoName, "SUSY_pid2"};
  }; /// class

} /// namespace


#endif
