/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUHYPO_TrigTauTrackRoiUpdater_H
#define TRIGTAUHYPO_TrigTauTrackRoiUpdater_H

#include <iostream>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "xAODTracking/TrackParticleContainer.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"

/**
 * @class TrigTauTrackRoiUpdater
 * @brief Update the input RoI direction in (eta, phi) to the leading RoI track (if any), and update the size
 *
 * Input RoIs in 2024 have the size of the tauCore setting: \Delta\eta/2 = \Delta\eta/2 = 0.1, 
 * and \Delta z_0/2 = 180mm. For the RoI update, FTF tracks are used.
 *
 * The logic of the RoI update is as follows:
 * 
 * If tracks were found:
 *   - Update the RoI position to that of the leading track.
 *   - Update the RoI angular size. During 2024, this is \Delta\eta/2 = \Delta\phi/2 = 0.2.
 *   - Update the RoI longitudinal size. During 2024, this is \Delta z_0/2 = 7mm.
 * If no tracks were found:
 *   - Update the RoI angular size. During 2024, this is \Delta\eta/2 = \Delta\phi/2 = 0.2.
 *     Maintain the longitudinal size
 **/

class TrigTauTrackRoiUpdater : public AthReentrantAlgorithm
{
public:
    TrigTauTrackRoiUpdater(const std::string&, ISvcLocator*);
    ~TrigTauTrackRoiUpdater();
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext&) const override;

private:
    Gaudi::Property<float> m_z0HalfWidth  {this, "z0HalfWidth", 7.0, "z0 half width for the output RoI"};
    Gaudi::Property<float> m_etaHalfWidth {this, "etaHalfWidth", 0.4, "eta half width for the output RoI"};
    Gaudi::Property<float> m_phiHalfWidth {this, "phiHalfWidth", 0.4, "phi half width for the output RoI"};
    Gaudi::Property<int> m_nHitPix {this, "nHitPix", 2, "Minimum number of hits in the Pixel detector on the lead track"};
    Gaudi::Property<int> m_nSiHoles {this, "nSiHoles", 2, "Maximum number of Si holes on the lead track"};

    SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roIInputKey {this, "RoIInputKey", "", "Input RoI key"};
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_tracksKey {this, "TracksKey", "", "FTF tracks" };
    SG::WriteHandleKey<TrigRoiDescriptorCollection> m_roIOutputKey {this, "RoIOutputKey", "", "Output RoI key"};
};

#endif
