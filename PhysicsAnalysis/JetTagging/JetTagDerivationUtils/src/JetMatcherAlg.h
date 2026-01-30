/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JET_MATCHER_ALG_H
#define JET_MATCHER_ALG_H

#include "JetTagDerivationUtils/VariableMule.h"

#include "xAODBase/IParticleContainer.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

namespace ftag {

  class JetMatcherAlg: public AthReentrantAlgorithm
  {
  public:
    JetMatcherAlg(const std::string& name, ISvcLocator* pSvcLocator);

    // these are the functions inherited from Algorithm
    virtual StatusCode initialize () override;
    virtual StatusCode execute (const EventContext&) const override;
    virtual StatusCode finalize () override;
  private:
    using JC = xAOD::IParticleContainer;
    using IPL = ElementLink<xAOD::IParticleContainer>;
    using IPLV = std::vector<IPL>;
    VariableMule<float,JC> m_floats{NAN};
    VariableMule<double,JC> m_doubles{NAN};
    VariableMule<int,JC> m_ints{-1};
    VariableMule<uint,JC> m_uints{0};
    VariableMule<ulong,JC> m_ulongs{0};
    VariableMule<char,JC> m_chars{0};
    VariableMule<IPLV,JC> m_iparticles{{}};
    SG::ReadHandleKey<JC> m_targetJet {this, "targetJet", "", "target jet"};
    SG::ReadHandleKeyArray<JC> m_sourceJets {
      this, "sourceJets", {}, "source jets"
    };
    SG::WriteDecorHandleKey<JC> m_drDecorator {
      this, "dR", m_targetJet, "deltaRToMatchedJet",
      "decorator for delta R to match"};
    SG::WriteDecorHandleKey<JC> m_dEtaDecorator {
      this, "dEta", m_targetJet, "deltaEtaToMatchedJet",
      "decorator for delta Eta to match"};
    SG::WriteDecorHandleKey<JC> m_dPhiDecorator {
      this, "dPhi", m_targetJet, "deltaPhiToMatchedJet",
      "decorator for delta Phi to match"};
    SG::WriteDecorHandleKey<JC> m_dPtDecorator {
      this, "dPt", m_targetJet, "deltaPtToMatchedJet",
      "decorator for delta pt to match"};
    SG::WriteDecorHandleKey<JC> m_linkDecorator {
      this, "particleLink", m_targetJet, "",
      "decorator for matched IParticle"};
    SG::WriteDecorHandleKey<JC> m_matchDecorator {
      this, "isMatched", m_targetJet, "jetIsMatched",
      "1 if matched, 0 if not"};
    SG::WriteDecorHandleKey<JC> m_nMatchDecoragor {
      this, "nMatch", m_targetJet, "nMatches",
      "number of matches"};
    Gaudi::Property<float> m_ptPriorityWithDeltaR {
      this, "ptPriorityWithDeltaR", -1,
      "Give priority to higher pt truth jets, with some delta-R cut"
    };
    Gaudi::Property<float> m_sourceMinimumPt {
      this, "sourceMinimumPt", 0, "Set minimum pt value for incoming jets"
    };
    using JV = std::vector<const xAOD::IParticle*>;
    using Match = std::pair<unsigned int, const xAOD::IParticle*>;
    std::function<Match(const xAOD::IParticle*, const JV&)> m_jetSelector;
  };

} // end namespace ftag

#endif
