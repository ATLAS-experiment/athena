/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "AthenaMonitoringKernel/MonitoredCollection.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "ActsGeometry/ATLASSourceLink.h"

#include "TrackAnalysisAlg.h"

template <> 
struct Monitored::detail::get_value_type<ActsTrk::TrackContainer> { typedef typename ActsTrk::TrackContainer::TrackProxy value_type; };


namespace ActsTrk {

  TrackAnalysisAlg::TrackAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) 
  {}

  StatusCode TrackAnalysisAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name() << " ...");

    ATH_MSG_DEBUG("Properties:");
    ATH_MSG_DEBUG(m_tracksKey);

    ATH_CHECK(m_tracksKey.initialize());
     
    ATH_MSG_DEBUG("Monitoring settings ...");
    ATH_MSG_DEBUG(m_monGroupName);

    ATH_CHECK(detStore()->retrieve(m_pixelID,"PixelID"));
    ATH_CHECK(detStore()->retrieve(m_stripID,"SCT_ID"));

    return AthMonitorAlgorithm::initialize();
  }

  namespace {
     std::array<uint8_t,static_cast<unsigned int>(xAOD::UncalibMeasType::nTypes)> countHits(
          const ActsTrk::TrackContainer &tracksContainer,
          const typename ActsTrk::TrackContainer::ConstTrackProxy& track) {
        std::array<uint8_t,static_cast<unsigned int>(xAOD::UncalibMeasType::nTypes)> hit_counts_out{};
        tracksContainer.trackStateContainer().visitBackwards(
          track.tipIndex(),
          [
           &hit_counts_out
           ](const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy &state) -> void
          {
             if (state.hasUncalibratedSourceLink()) {
                try {
                   const xAOD::UncalibratedMeasurement *
                      measurement = &(ActsTrk::getUncalibratedMeasurement(state.getUncalibratedSourceLink().get<ATLASUncalibSourceLink>()));
                   assert( static_cast<unsigned int >(measurement->type() < xAOD::UncalibMeasType::nTypes));
                   ++hit_counts_out[static_cast<unsigned int >(measurement->type())];
                }
                catch (std::bad_any_cast &) {
                }
             }
          });
        return hit_counts_out;
     }
  }

  StatusCode TrackAnalysisAlg::fillHistograms(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Filling Histograms for " << name() << " ... " );

    // Retrieve the tracks
    SG::ReadHandle<ActsTrk::TrackContainer> trackHandle = SG::makeHandle(m_tracksKey, ctx);
    ATH_CHECK(trackHandle.isValid());
    const ActsTrk::TrackContainer *tracks = trackHandle.cptr();
    using ConstTrackProxy = ActsTrk::TrackContainer::ConstTrackProxy;

    // TODO this  copy will be eliminated once the TrackContainer has [] operator
    std::vector<ConstTrackProxy> proxies;
    for (auto proxy: *tracks ) {
      proxies.push_back(proxy);
    }

    std::vector<int> nSharedPerTrack {};
    nSharedPerTrack.reserve(tracks->size());

    std::vector<int> nSharedPixelBarrelLayer;
    std::vector<int> nSharedPixelEndCapLayer;
    std::vector<int> nSharedStripBarrelLayer;
    std::vector<int> nSharedStripEndCapLayer;
    nSharedPixelBarrelLayer.reserve(5*tracks->size());
    nSharedPixelEndCapLayer.reserve(5*tracks->size());
    nSharedStripBarrelLayer.reserve(5*tracks->size());
    nSharedStripEndCapLayer.reserve(5*tracks->size());
    
    for (const auto track_proxy : proxies) {
      int nShared = 0;
      track_proxy.container().trackStateContainer()
	.visitBackwards(track_proxy.tipIndex(),
			[this,
			 &nShared,
			 &nSharedPixelBarrelLayer, &nSharedPixelEndCapLayer,
			 &nSharedStripBarrelLayer, &nSharedStripEndCapLayer]
			(const auto& state)
			{
			  auto flags = state.typeFlags();
			  if (not flags.test(Acts::TrackStateFlag::SharedHitFlag)) return;
			  ++nShared;
			  // get measurement -> if barrel/endcap and layer number
			  auto sl = state.getUncalibratedSourceLink().template get<ATLASUncalibSourceLink>();
			  assert( sl != nullptr);
			  const xAOD::UncalibratedMeasurement &cluster = getUncalibratedMeasurement(sl);
			  xAOD::DetectorIDHashType idHash = cluster.identifierHash();

			  xAOD::UncalibMeasType clusterType = cluster.type();
			  if (clusterType == xAOD::UncalibMeasType::PixelClusterType) {
			    const Identifier& id = m_pixelID->wafer_id(idHash);
			    auto barrel_or_endcap = m_pixelID->barrel_ec(id);
			    auto layer = m_pixelID->layer_disk(id);
			    if (barrel_or_endcap == 0) {
			      nSharedPixelBarrelLayer.push_back(layer);
			    } else {
			      nSharedPixelEndCapLayer.push_back(layer);
			    }
			  } else if (clusterType == xAOD::UncalibMeasType::StripClusterType) {
			    const Identifier& id = m_stripID->wafer_id(idHash);
			    auto barrel_or_endcap = m_stripID->barrel_ec(id);
                            auto layer = m_stripID->layer_disk(id);
			    if (barrel_or_endcap == 0) {
                              nSharedStripBarrelLayer.push_back(layer);
			    } else {
                              nSharedStripEndCapLayer.push_back(layer);
			    }
			  }
			  
			} );
      nSharedPerTrack.push_back(nShared);
    }
    
    auto monitor_nsharedpertrack = Monitored::Collection("NsharedPerTrack", nSharedPerTrack);
    auto monitor_nsharedperlayer_pixelbarrel = Monitored::Collection("NsharedPerLayer_pixelBarrel", nSharedPixelBarrelLayer);
    auto monitor_nsharedperlayer_pixelendcap = Monitored::Collection("NsharedPerLayer_pixelEndCap", nSharedPixelEndCapLayer);
    auto monitor_nsharedperlayer_stripbarrel = Monitored::Collection("NsharedPerLayer_stripBarrel", nSharedStripBarrelLayer);
    auto monitor_nsharedperlayer_stripendcap = Monitored::Collection("NsharedPerLayer_stripEndCap", nSharedStripEndCapLayer);
    
    auto monitor_ntracks = Monitored::Scalar<int>("Ntracks", tracks->size());
    auto monitor_theta = Monitored::Collection("theta", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.theta()); } );
    auto monitor_eta = Monitored::Collection("eta", proxies,
					     [](const ConstTrackProxy& tp)
					     { return static_cast<double>(  -std::log(std::tan(tp.theta() * 0.5)) ); } );
    auto monitor_qoverp = Monitored::Collection("qoverp", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.qOverP()); } );
    auto monitor_phi = Monitored::Collection("phi", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.phi()); } );
    auto monitor_chi2 = Monitored::Collection("chi2", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.chi2()); } );
    auto monitor_chi2OverNdof = Monitored::Collection("chi2OverNdof", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.chi2()/tp.nDoF()); } );
    auto monitor_ndof = Monitored::Collection("ndof", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.nDoF()); } );
    auto monitor_nstates = Monitored::Collection("nStates", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.nTrackStates()); } );
     // @TODO not particular efficient to count the hits twice ...
    auto monitor_nPixelHits = Monitored::Collection("nPixelHits", proxies, [&tracks](const ConstTrackProxy& tp){
       return countHits(*tracks,tp)[static_cast<unsigned int>(xAOD::UncalibMeasType::PixelClusterType)]; } );
    auto monitor_nStripHits = Monitored::Collection("nStripHits", proxies, [&tracks](const ConstTrackProxy& tp){
       return countHits(*tracks,tp)[static_cast<unsigned int>(xAOD::UncalibMeasType::StripClusterType)]; } );
    auto monitor_nmeas = Monitored::Collection("nMeasurements", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.nMeasurements()); } );
    auto monitor_surftype = Monitored::Collection("surfaceType", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.referenceSurface().type()); } );


    fill(m_monGroupName.value(),
	 monitor_nsharedpertrack,
	 monitor_nsharedperlayer_pixelbarrel, monitor_nsharedperlayer_pixelendcap,
	 monitor_nsharedperlayer_stripbarrel, monitor_nsharedperlayer_stripendcap,
	 monitor_ntracks, monitor_theta, monitor_eta, monitor_phi, 
	 monitor_qoverp, monitor_nstates, monitor_phi, monitor_chi2, monitor_chi2OverNdof, 
	 monitor_ndof, monitor_nstates, monitor_nmeas, monitor_surftype,
	 monitor_nPixelHits, monitor_nStripHits);
    
    return StatusCode::SUCCESS;
  }

}
