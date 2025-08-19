/*
Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration 
*/

#include "src/TrigBtagEmulationJet.h"
#include "xAODBTagging/BTaggingUtilities.h"

//**********************************************************************

namespace Trig {

TrigBtagEmulationJet::TrigBtagEmulationJet(const xAOD::Jet& jet, const std::string& btagLink)
  : TrigBtagEmulationJet(jet, xAOD::BTaggingUtilities::getBTagging(jet, btagLink))
{}

TrigBtagEmulationJet::TrigBtagEmulationJet(const xAOD::Jet& jet, const xAOD::BTagging* btag)
  :  m_jet(&jet),
     m_btag(btag),
     m_pt(jet.pt()),
     m_et(jet.p4().Et()),
     m_eta(jet.eta()),
     m_phi(jet.phi()),
     m_p4(jet.p4())
{}

bool TrigBtagEmulationJet::satisfy(const std::string& tagger_name,
				   double workingPoint) const 
{
  if (not m_btag) return false;
  if (tagger_name == "offperf") return true;

  double tagger_weight = -999;
  if (tagger_name.substr(0, 4) == "mv2c") {
    m_btag->MVx_discriminant("MV"+tagger_name.substr(2, 4), tagger_weight);
  }
  else {
    double pu = -1;
    double pb = -1;
    double pc = -1;

    m_btag->pu(tagger_name, pu);
    m_btag->pb(tagger_name, pb);
    m_btag->pc(tagger_name, pc);

    tagger_weight = dl1r_weight(pu, pb, pc);
  }
  return tagger_weight > workingPoint;
}
 
}

//**********************************************************************

