/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JET_LINKMATCHER_ALG_H
#define JET_LINKMATCHER_ALG_H

#include "JetTagDerivationUtils/VariableMule.h"

#include "xAODBase/IParticleContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
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
    using JC = xAOD::JetContainer;
    using IPL = ElementLink<xAOD::IParticleContainer>;
    using IPLV = std::vector<IPL>;
    using TPLV = std::vector<ElementLink<xAOD::TrackParticleContainer>>;
    VariableMule<float,JC> m_floats{NAN};
    VariableMule<double,JC> m_doubles{NAN};
    VariableMule<int,JC> m_ints{-1};
    VariableMule<uint,JC> m_uints{0};
    VariableMule<ulong,JC> m_ulongs{0};
    VariableMule<char,JC> m_chars{0};
    VariableMule<std::vector<char>,JC> m_charVectors{{}};
    VariableMule<IPLV,JC> m_iparticles{{}};
    VariableMule<TPLV,JC> m_trackLinks{{}};
    SG::ReadHandleKey<JC> m_targetJet {
      this, "targetJet", "", "target jet"
    };
    SG::ReadHandleKeyArray<JC> m_sourceJets {
      this, "sourceJets", {}, "source jets"
    };
    SG::ReadDecorHandleKey<JC> m_link {
      this, "linkName", m_targetJet, "", "name of link to match"
    };
    SG::WriteDecorHandleKey<JC> m_matchDecorator {
      this, "isMatched", m_targetJet, "",
      "1 if matched, 0 if not. If unspecified fail on no match"};
    using JV = std::vector<const xAOD::IParticle*>;
  };

} // end namespace ftag

#endif
