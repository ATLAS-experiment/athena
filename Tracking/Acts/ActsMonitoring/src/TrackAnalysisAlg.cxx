/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "AthenaMonitoringKernel/MonitoredCollection.h"

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

    return AthMonitorAlgorithm::initialize();
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
    auto monitor_ntracks = Monitored::Scalar<int>("Ntracks", tracks->size());
    auto monitor_theta = Monitored::Collection("theta", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.theta()); } );
    auto monitor_qoverp = Monitored::Collection("qoverp", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.qOverP()); } );
    auto monitor_phi = Monitored::Collection("phi", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.phi()); } );
    auto monitor_chi2 = Monitored::Collection("chi2", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.chi2()); } );
    auto monitor_chi2OverNdof = Monitored::Collection("chi2OverNdof", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.chi2()/tp.nDoF()); } );
    auto monitor_ndof = Monitored::Collection("ndof", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.nDoF()); } );
    auto monitor_nstates = Monitored::Collection("nStates", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.nTrackStates()); } );
    auto monitor_nmeas = Monitored::Collection("nMeasurements", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.nMeasurements()); } );
    // TODO this needs to be fixed in the followup MR
    // auto monitor_surftype = Monitored::Collection("surfaceType", proxies, [](const ConstTrackProxy& tp){ return static_cast<double>(tp.referenceSurface().type()); } );


    fill(m_monGroupName.value(), monitor_ntracks, monitor_theta, monitor_phi, 
        monitor_qoverp, monitor_nstates, monitor_phi, monitor_chi2, monitor_chi2OverNdof, 
        monitor_ndof, monitor_nstates, monitor_nmeas);
    
    return StatusCode::SUCCESS;
  }

}
