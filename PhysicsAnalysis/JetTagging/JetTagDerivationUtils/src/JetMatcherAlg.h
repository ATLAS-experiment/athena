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
    SG::ReadHandleKeyArray<JC> m_sourceJets;
    Gaudi::Property<std::string> m_dRKey {
      this, "dR", "deltaRToMatchedJet", "decorator for delta R to match"};
    Gaudi::Property<std::string> m_dEtaKey {
      this, "dEta", "deltaEtaToMatchedJet", "decorator for delta Eta to match"};
    Gaudi::Property<std::string> m_dPhiKey {
      this, "dPhi", "deltaPhiToMatchedJet", "decorator for delta Phi to match"};
    Gaudi::Property<std::string> m_dPtKey {
      this, "dPt", "deltaPtToMatchedJet", "decorator for delta pt to match"};
    Gaudi::Property<std::string> m_linkKey {
      this, "particleLink", "", "decorator for matched IParticle"};
    Gaudi::Property<std::string> m_matchKey {
      this, "match", "jetIsMatched", "1 if matched, 0 if not"};
    Gaudi::Property<float> m_ptPriorityWithDeltaR {
      this, "ptPriorityWithDeltaR", -1,
      "Give priority to higher pt truth jets, with some delta-R cut"
    };
    Gaudi::Property<float> m_sourceMinimumPt {
      this, "sourceMinimumPt", 0, "Set minimum pt value for incoming jets"
    };
    SG::WriteDecorHandleKey<JC> m_drDecorator;
    SG::WriteDecorHandleKey<JC> m_dEtaDecorator;
    SG::WriteDecorHandleKey<JC> m_dPhiDecorator;
    SG::WriteDecorHandleKey<JC> m_dPtDecorator;
    SG::WriteDecorHandleKey<JC> m_linkDecorator;
    SG::WriteDecorHandleKey<JC> m_matchDecorator;
    using JV = std::vector<const xAOD::IParticle*>;
    std::function<const xAOD::IParticle*(const xAOD::IParticle*, const JV&)> m_jetSelector;
  };

} // end namespace ftag

#endif
