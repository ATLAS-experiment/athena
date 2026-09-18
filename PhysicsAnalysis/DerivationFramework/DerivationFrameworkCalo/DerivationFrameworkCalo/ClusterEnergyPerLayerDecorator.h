/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_ClusterEnergyPerLayerDecorator_H
#define DERIVATIONFRAMEWORK_ClusterEnergyPerLayerDecorator_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloUtils/CaloClusterProcessor.h"
#include "CaloClusterCorrection/CaloFillRectangularCluster.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "xAODEgamma/EgammaContainer.h"

namespace DerivationFramework {

/** Decorate egamma objects with the energy per layer for a rectangular cluster
 * of size neta X nphi built on the fly
 **/
class ClusterEnergyPerLayerDecorator : public AthReentrantAlgorithm
{
public:

  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;

private:
  Gaudi::Property<std::vector<unsigned int>> m_layers{this, "layers", { 0, 1, 2, 3 } };

  SG::ReadHandleKey<xAOD::EgammaContainer>
    m_SGKey_photons{ this, "SGKey_photons", "", "SG key of photon container" };

  SG::ReadHandleKey<xAOD::EgammaContainer> m_SGKey_electrons{
    this,
    "SGKey_electrons",
    "",
    "SG key of electron container"
  };

  SG::ReadHandleKey<CaloCellContainer> m_SGKey_caloCells{
    this,
    "SGKey_caloCells",
    "AllCalo",
    "SG key of the cell container"
  };

  ToolHandle<CaloClusterProcessor> m_caloFillRectangularTool{
    this,
    "CaloFillRectangularClusterTool",
    "",
    "Handle of the CaloFillRectangularClusterTool"
  };

  SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
    m_SGKey_photons_decorations{
      this,
      "SGKey_photons_decorations",
      m_SGKey_photons, {},
      "SG keys for photon decorations"
    };

  SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
    m_SGKey_electrons_decorations{
      this,
      "SGKey_electrons_decorations",
      m_SGKey_electrons, {},
      "SG keys for electrons decorations"
    };

  const CaloFillRectangularCluster* m_tool{};

  std::vector<float> decorateObject(const EventContext& ctx,
                                    const xAOD::Egamma* egamma,
                                    const CaloCellContainer* cellCont) const;
};
}

#endif // DERIVATIONFRAMEWORK_ClusterEnergyPerLayerDecorator_H
