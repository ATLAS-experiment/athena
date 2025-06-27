/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "xAODTracking/TrackStateValidationContainer.h"
#include "xAODTracking/TrackStateValidationAuxContainer.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "ActsEvent/TrackContainer.h"

namespace ActsTrk {

  class ActsTrackStateOnSurfaceDecoratorAlg
    : public AthReentrantAlgorithm {
  public:
    ActsTrackStateOnSurfaceDecoratorAlg(const std::string& name,
					ISvcLocator *pSvcLocator);
    virtual ~ActsTrackStateOnSurfaceDecoratorAlg() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    xAOD::UncalibMeasType getDetectorType(std::uint64_t volumeId) const;

    StatusCode storeTrackState(const typename ActsTrk::TrackContainer::ConstTrackStateProxy& state,
			       const xAOD::TrackMeasurementValidationContainer& measurements,
			       std::vector< ElementLink< xAOD::TrackStateValidationContainer > >& msosLinks,
			       xAOD::TrackStateValidationContainer& msosContainer) const;
    
  private:
    SG::ReadHandleKey< xAOD::TrackParticleContainer > m_trackParticlesKey {this, "TrackParticles", "", "Input xAOD::TrackParticles"};
    SG::ReadHandleKey< xAOD::TrackMeasurementValidationContainer > m_pixelMeasurementsKey {this, "PixelMeasurements", ""};
    SG::ReadHandleKey< xAOD::TrackMeasurementValidationContainer > m_stripMeasurementsKey {this, "StripMeasurements", ""};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_decorator_actsTracks {this, "ActsTrackLink", m_trackParticlesKey, "actsTrack"};
    
    SG::WriteHandleKey< xAOD::TrackStateValidationContainer > m_pixelMsosKey {this, "PixelMSOSs", ""};
    SG::WriteHandleKey< xAOD::TrackStateValidationContainer > m_stripMsosKey {this, "StripMSOSs", ""};
    SG::WriteDecorHandleKey< xAOD::TrackParticleContainer > m_trackMsosLink {this, "msosLink", m_trackParticlesKey, "Reco_msosLink"};

  };

}
