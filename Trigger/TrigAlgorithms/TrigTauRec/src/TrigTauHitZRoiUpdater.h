/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUHYPO_TrigTauHitZRoiUpdater_H
#define TRIGTAUHYPO_TrigTauHitZRoiUpdater_H

#include <iostream>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "xAODTau/TauJetContainer.h"

/**
 * @class TrigTauHitZRoiUpdater
 * @brief Update the input RoI's z-coordinate and z-width to the HitZ regression estimation,
 *        if the regression sigma is below the indicated threshold.
 **/

class TrigTauHitZRoiUpdater : public AthReentrantAlgorithm
{
public:
    TrigTauHitZRoiUpdater(const std::string&, ISvcLocator*);
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext&) const override;

private:
    Gaudi::Property<float> m_z0HalfWidth  {this, "z0HalfWidth", 10.0, "z0 half width for the output RoI, if the HitZ regression is successful, in mm"};
    Gaudi::Property<float> m_etaHalfWidth {this, "etaHalfWidth", 0.1, "eta half width for the output RoI, if the HitZ regression is successful"};
    Gaudi::Property<float> m_phiHalfWidth {this, "phiHalfWidth", 0.1, "phi half width for the output RoI, if the HitZ regression is successful"};

    Gaudi::Property<float> m_maxPt {this, "maxPt", 1e5, "Maximum Pt of the taus to apply the HitZ regression RoI update, in GeV"};
    Gaudi::Property<float> m_maxSigma {this, "maxSigma", 5.0, "Maximum HitZ regression sigma to update the RoI, in mm"};

    SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roIInputKey {this, "RoIInputKey", "", "Input RoI key"};
    SG::WriteHandleKey<TrigRoiDescriptorCollection> m_roIOutputKey {this, "RoIOutputKey", "", "Output RoI key"};

    SG::ReadHandleKey<xAOD::TauJetContainer> m_tauKey {this, "TauKey", "", "Input Tau container key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_zDecorKey {this, "zDecorKey", "", "HitZ regression z decoration key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_sigmaDecorKey {this, "sigmaDecorKey", "", "HitZ regression sigma decoration key"};
};

#endif

