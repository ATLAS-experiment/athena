/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JET_LINKMATCHER_ALG_H
#define JET_LINKMATCHER_ALG_H

#include "JetTagDerivationUtils/VariableMule.h"

#include "xAODBase/IParticleContainer.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

namespace ftag {

  class JetLinkMatcherAlg: public AthReentrantAlgorithm
  {
  public:
    JetLinkMatcherAlg(const std::string& name, ISvcLocator* pSvcLocator);

    // these are the functions inherited from Algorithm
    virtual StatusCode initialize () override;
    virtual StatusCode execute (const EventContext&) const override;
    virtual StatusCode finalize () override;
  private:
    using IPC = xAOD::IParticleContainer;
    using IPL = ElementLink<xAOD::IParticleContainer>;
    using IPLV = std::vector<IPL>;
    VariableMule<float,IPC> m_floats{NAN};
    VariableMule<double,IPC> m_doubles{NAN};
    VariableMule<int,IPC> m_ints{-1};
    VariableMule<uint,IPC> m_uints{0};
    VariableMule<ulong,IPC> m_ulongs{0};
    VariableMule<char,IPC> m_chars{0};
    VariableMule<IPLV,IPC> m_iparticles{{}};
    SG::ReadHandleKey<IPC> m_targetJet {
      this, "targetJet", "", "target jet"
    };
    SG::ReadHandleKeyArray<IPC> m_sourceJets {
      this, "sourceJets", {}, "source jets"
    };
    SG::ReadDecorHandleKey<IPC> m_link {
      this, "linkName", m_targetJet, "", "name of link to match"
    };
    SG::WriteDecorHandleKey<IPC> m_matchDecorator {
      this, "isMatched", m_targetJet, "",
      "1 if matched, 0 if not. If unspecified fail on no match"};
    using JV = std::vector<const xAOD::IParticle*>;
  };

} // end namespace ftag

#endif
