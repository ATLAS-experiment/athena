/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
// Author: Chengxi Yang (cxyang@berkeley.edu)

#ifndef DERIVATIONFRAMEWORK_EGAMMAENERGYCALIBRATIONWRAPPER_H
#define DERIVATIONFRAMEWORK_EGAMMAENERGYCALIBRATIONWRAPPER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
//
#include "StoreGate/ReadHandleKey.h"
#include "GaudiKernel/ServiceHandle.h"
#include "xAODEgamma/EgammaContainer.h"
// name space of global event info is defined here
#include "EgammaAnalysisInterfaces/IegammaMVASvc.h"
#include "xAODEventInfo/EventInfo.h"
//
#include <string>


namespace DerivationFramework {

  class EGammaEnergyCalibrationWrapper :
    public AthReentrantAlgorithm
  {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    StatusCode initialize() override final;
    StatusCode execute(const EventContext& ctx) const override final;

  private:

    SG::ReadHandleKey<xAOD::EgammaContainer> m_electronContainerKey{
      this, "ElectronContainerName", "Electrons", "Electron container"
    };

    SG::ReadHandleKey<xAOD::EgammaContainer> m_photonContainerKey{
      this, "PhotonContainerName", "Photons", "Photon container"
    };

    SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_electronEnergyDecoKey{
      this, "decoratorTransformerEnergy", m_electronContainerKey, "", "Calibrated energy decoration for electrons"
    };

    SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_photonEnergyDecoKey{
      this, "decoratorTransformerEnergyPhoton", m_photonContainerKey, "", "Calibrated energy decoration for photons"
    };

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key{this, "EventInfo", "EventInfo", "Input event information"};

    ServiceHandle<IegammaMVASvc> m_MVACalibSvc{
      this, "TransformerCalibSvc", "", "Calibration service using Transformer model"
    };
  };

} // namespace DerivationFramework

#endif
