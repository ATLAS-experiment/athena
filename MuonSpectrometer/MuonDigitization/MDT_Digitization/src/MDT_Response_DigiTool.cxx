/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MDT_Digitization/MDT_Response_DigiTool.h"

#include <iostream>

#include "MDT_Digitization/MdtDigiToolInput.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "StoreGate/ReadCondHandle.h"
using namespace MuonGM;

MDT_Response_DigiTool::MDT_Response_DigiTool(const std::string& type, const std::string& name, const IInterface* parent) :
    AthAlgTool(type, name, parent) {
    declareInterface<IMDT_DigitizationTool>(this);
}

MdtDigiToolOutput MDT_Response_DigiTool::digitize(const EventContext& ctx,
                                                  const MdtDigiToolInput& input, 
                                                  CLHEP::HepRandomEngine* rndmEngine) const {
    
    SG::ReadCondHandle<MuonGM::MuonDetectorManager> detMgr{m_detMgrKey, ctx};
    MDT_Response responseTube{m_DoUpdatedMdtDigi};

    // initialize MDT_Response
    responseTube.SetTubeRadius(detMgr->getMdtReadoutElement(input.getHitID())->innerTubeRadius());
    responseTube.SetTriggerElectron(m_threshold);
    
    if (m_DoUpdatedMdtDigi) {
        responseTube.SetClusterDensity(10.);
        responseTube.SetAttLength(30000.);
        responseTube.SetIntegrationWindow(18.5);
        responseTube.SetAdcOffset(35.); // constants determined Z -> mumu data in 2025
        responseTube.SetAdcFactor(150.); // constants determined Z -> mumu data in 2025
        responseTube.SetDoUpdatedMdtDigi(true);
    }
    else {
        responseTube.SetClusterDensity(m_clusterDensity);
        responseTube.SetAttLength(m_attenuationLength);
    }
    
    responseTube.SetSegment(input.radius(), input.positionAlongWire());
    ATH_MSG_DEBUG("Digitizing input ");
    if (m_DoQballGamma) {
        double ParticleCharge = input.electriccharge();
        double ParticleGamma = input.gamma();
        if (ParticleGamma > 0.) {
            if (responseTube.GetSignal(ParticleCharge, ParticleGamma, rndmEngine)) {
                MdtDigiToolOutput output(true, responseTube.DriftTime(), responseTube.AdcResponse());
                return output;
            }
        } else {
            if (responseTube.GetSignal(rndmEngine)) {
                MdtDigiToolOutput output(true, responseTube.DriftTime(), responseTube.AdcResponse());
                return output;
            }
        }

    } else {
        if (responseTube.GetSignal(rndmEngine)) {
            MdtDigiToolOutput output(true, responseTube.DriftTime(), responseTube.AdcResponse());
            return output;
        }
    }
    MdtDigiToolOutput output(false, 0., 0.);
    return output;
}

StatusCode MDT_Response_DigiTool::initialize() {
    ATH_CHECK(m_detMgrKey.initialize());
    return StatusCode::SUCCESS;
}

