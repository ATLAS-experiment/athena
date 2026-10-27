/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
// Copies jet decorations, e.g. the truth labels, from the seed jet of each
// tau to the tau. The seed jet is found through a link on the tau, or by
// index if the seed jets are in the tau order, as from the TauSeedJetAlg.

#ifndef PARTICLEJETTOOLS_TAUSEEDJETLABELCOPYALG_H
#define PARTICLEJETTOOLS_TAUSEEDJETLABELCOPYALG_H

#include "AthContainers/ConstAccessor.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"

#include <memory>
#include <string>
#include <vector>

namespace ParticleJetTools {

class TauSeedJetLabelCopyAlg : public AthReentrantAlgorithm {
public:
  TauSeedJetLabelCopyAlg(const std::string& name, ISvcLocator* loc);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  SG::ReadHandleKey<xAOD::TauJetContainer> m_tauKey{
    this, "TauJets", "TauJets", "Taus to decorate"};
  SG::ReadHandleKey<xAOD::JetContainer> m_jetKey{
    this, "SeedJets", "TauSeedJets", "Seed jets holding the decorations"};
  Gaudi::Property<std::string> m_linkName{
    this, "SeedJetLink", "", "Tau link to its seed jet, empty to match by index"};
  Gaudi::Property<std::vector<std::string>> m_intNames{
    this, "IntDecorations", {}, "Integer jet decorations to copy, -1 without a seed jet"};
  Gaudi::Property<std::vector<std::string>> m_floatNames{
    this, "FloatDecorations", {}, "Float jet decorations to copy, NaN without a seed jet"};

  SG::WriteDecorHandleKeyArray<xAOD::TauJetContainer, int> m_intDecorKeys{
    this, "IntDecorKeys", {}, "Declared int decorations, filled in initialize"};
  SG::WriteDecorHandleKeyArray<xAOD::TauJetContainer, float> m_floatDecorKeys{
    this, "FloatDecorKeys", {}, "Declared float decorations, filled in initialize"};

  std::unique_ptr<SG::ConstAccessor<ElementLink<xAOD::JetContainer>>> m_linkAcc;
  std::vector<SG::ConstAccessor<int>> m_intAccs;
  std::vector<SG::ConstAccessor<float>> m_floatAccs;
};

}

#endif
