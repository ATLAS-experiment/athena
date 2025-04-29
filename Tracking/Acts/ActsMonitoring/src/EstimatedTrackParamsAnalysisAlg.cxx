/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "EstimatedTrackParamsAnalysisAlg.h"

namespace ActsTrk {

  EstimatedTrackParamsAnalysisAlg::EstimatedTrackParamsAnalysisAlg(const std::string& name, 
								   ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) 
  {}
  
  StatusCode EstimatedTrackParamsAnalysisAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name() << " ...");

    ATH_CHECK( m_inputTrackParamsColletionKey.initialize() );

    ATH_MSG_DEBUG("Monitoring settings ...");
    ATH_MSG_DEBUG(m_monGroupName);
    
    return AthMonitorAlgorithm::initialize();
  }

  StatusCode EstimatedTrackParamsAnalysisAlg::fillHistograms(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Filling Histograms for " << name() << " ... " );

    SG::ReadHandle< ActsTrk::BoundTrackParametersContainer > trackParamsHandle = SG::makeHandle( m_inputTrackParamsColletionKey, ctx);
    ATH_CHECK( trackParamsHandle.isValid() );
    const ActsTrk::BoundTrackParametersContainer *trackParams = trackParamsHandle.get();
    ATH_MSG_DEBUG( "Retrieved " << trackParams->size() << " input parameters with key " << m_inputTrackParamsColletionKey.key() );

    auto monitor_nparams = Monitored::Scalar<int>("Nparams", trackParams->size());
    fill(m_monGroupName.value(), monitor_nparams);

    auto monitor_pt = Monitored::Collection("track_param_pt", *trackParams,
					    [] (const auto* param) -> double
					    { return param == nullptr ? std::numeric_limits<double>::quiet_NaN() : param->transverseMomentum(); });
    auto monitor_eta = Monitored::Collection("track_param_eta", *trackParams,
					     [] (const auto* param) -> double
					     { return param == nullptr ? std::numeric_limits<double>::quiet_NaN() : -std::log( std::tan(0.5 * param->parameters()[Acts::eBoundTheta]) ); });
    auto monitor_loc0 = Monitored::Collection("track_param_loc0", *trackParams,
					      [] (const auto* param) -> double
					      { return param == nullptr ? std::numeric_limits<double>::quiet_NaN() : param->parameters()[Acts::eBoundLoc0]; });
    auto monitor_loc1 = Monitored::Collection("track_param_loc1", *trackParams,
					      [] (const auto* param) -> double
					      { return param == nullptr ? std::numeric_limits<double>::quiet_NaN() : param->parameters()[Acts::eBoundLoc1]; });
    auto monitor_phi = Monitored::Collection("track_param_phi", *trackParams,
					     [] (const auto* param) -> double
					     { return param == nullptr ? std::numeric_limits<double>::quiet_NaN() : param->parameters()[Acts::eBoundPhi]; });
    auto monitor_theta = Monitored::Collection("track_param_theta", *trackParams,
					       [] (const auto* param) -> double 
					       { return param == nullptr ? std::numeric_limits<double>::quiet_NaN() : param->parameters()[Acts::eBoundTheta]; });
    auto monitor_qOverP = Monitored::Collection("track_param_qoverp", *trackParams,
						[] (const auto* param) -> double
						{ return param == nullptr ? std::numeric_limits<double>::quiet_NaN() : param->parameters()[Acts::eBoundQOverP]; });
    auto monitor_time = Monitored::Collection("track_param_time", *trackParams,
					      [] (const auto* param) -> double
					      { return  param == nullptr ? std::numeric_limits<double>::quiet_NaN() : param->parameters()[Acts::eBoundTime]; });

    auto monitor_charge = Monitored::Collection("track_param_charge", *trackParams,
						[] (const auto* param) -> int
						{ return param == nullptr ? 0 : param->charge(); });

    fill(m_monGroupName.value(),
	 monitor_pt, monitor_eta,
	 monitor_loc0, monitor_loc1,
	 monitor_phi, monitor_theta,
	 monitor_qOverP, 
	 monitor_time,
	 monitor_charge);
    
    return StatusCode::SUCCESS;
  }

}
