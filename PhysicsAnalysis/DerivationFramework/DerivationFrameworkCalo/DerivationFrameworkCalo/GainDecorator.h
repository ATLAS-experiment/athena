/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Decorate egamma objects with the energy and number of cells per layer per
// gain

#ifndef DERIVATIONFRAMEWORK_GainDecorator_H
#define DERIVATIONFRAMEWORK_GainDecorator_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "xAODEgamma/EgammaContainer.h"
#include <map>
#include <string>
#include <vector>

class CaloCell;

namespace DerivationFramework {

  class GainDecorator : public extends<AthAlgTool, IAugmentationTool>
  {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;
    static int getLayer(const CaloCell* cell); // TODO Why is this public?

    struct calculation
    {
      std::map<std::pair<int, int>, float> EnoW;
      std::map<std::pair<int, int>, float> E;
      std::map<std::pair<int, int>, uint8_t> nCells;
    };

  private:
    SG::ReadHandleKey<xAOD::EgammaContainer> m_SGKey_photons{
      this, "SGKey_photons", "", "SG key of photon container" };

    SG::ReadHandleKey<xAOD::EgammaContainer> m_SGKey_electrons{
      this, "SGKey_electrons", "", "SG key of electron container" };

    Gaudi::Property< std::map<int, std::string> > m_gainNames{this, "gain_names",
                                                              { { CaloGain::LARHIGHGAIN, "Hi" },
                                                                { CaloGain::LARMEDIUMGAIN, "Med" },
                                                                { CaloGain::LARLOWGAIN, "Low" } } };
    Gaudi::Property< std::vector<unsigned int> > m_layers{this, "layers", { 0, 1, 2, 3 }};

    // // Name of the decorations
    std::vector<std::pair<int, int>> m_names_E;

    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
    m_SGKey_photons_decorations{
      this,
      "SGKey_photons_decorations",
      m_SGKey_photons, {},
      "SG keys for photon decorations not really configurable"
    };

    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
    m_SGKey_electrons_decorations{
      this,
      "SGKey_electrons_decorations",
      m_SGKey_electrons, {},
      "SG keys for electrons decorations not really configurable"
    };

    calculation decorateObject(const xAOD::Egamma*& egamma) const;
  };
}

#endif // DERIVATIONFRAMEWORK_GainDecorator_H
