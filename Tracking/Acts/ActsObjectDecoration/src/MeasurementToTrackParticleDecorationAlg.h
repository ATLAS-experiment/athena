/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Header file for class MeasurementToTrackParticleDecoration
//
// The algorithm extends xAOD::TrackParticles
// with additional decorations associated to measurements,
// i.e. residuals, pulls, modules
//
///////////////////////////////////////////////////////////////////

#ifndef MEASUREMENTTOTRACKPARTICLEDECORATIONALG_H
#define MEASUREMENTTOTRACKPARTICLEDECORATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"

#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "Acts/Definitions/Units.hpp"
#include "ActsEvent/TrackContainer.h"
#include "Acts/EventData/TrackStateProxy.hpp"


namespace ActsTrk {

    class MeasurementToTrackParticleDecorationAlg : public AthReentrantAlgorithm  {
    public:
        MeasurementToTrackParticleDecorationAlg(const std::string &name,ISvcLocator *pSvcLocator);
        virtual ~MeasurementToTrackParticleDecorationAlg() = default;

        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;

    private:      
        enum Subdetector {
            INVALID_DETECTOR=-1, INNERMOST_PIXEL, PIXEL, STRIP, N_SUBDETECTORS
        };
        enum Region {
            INVALID_REGION=-1, BARREL, ENDCAP
        };
        enum MeasurementType {
	    INVALID_MEASUREMENT=-1, HIT, OUTLIER, HOLE, BIASED, UNBIASED
        };

      float getChi2Contribution(const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy &state) const;
      std::pair<Acts::BoundVector, Acts::BoundMatrix> getUnbiasedTrackParameters(const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy &state,
										 bool useSmoothed = true) const;

      float evaluatePull(const float residual,
			 const float measurementCovariance,
			 const float trackParameterCovariance,
			 const bool evaluateUnbiased) const;
      
    private:
      
      ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
      
      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticlesKey {
	this, "TrackParticleKey", "", "Input track particle collection"};
      
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_measurementRegionKey{
	this, "MeasurementRegionKey", "measurement_region",
	"Decorate track particle with region of the measurement (barrel, ec)"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_measurementDetectorKey{
	this, "MeasurementDetectorKey", "measurement_det",
	"Decorate track particle with measurement detector id (innermost pix, pix, strip)"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_measurementLayerKey{
	this, "MeasurementLayerKey", "measurement_iLayer",
	"Decorate track particle with measurement layer"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_chi2HitPredictedKey{
	this, "Chi2HitPredictedKey", "chi2_hit_predicted",
	"Predicted Chi2 contribution for each hit"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_chi2HitFilteredKey{
	this, "Chi2HitFilteredKey", "chi2_hit_filtered",
	"Filtered Chi2 contribution for each hit"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_measurementTypeKey{
	this, "MeasurementTypeKey", "measurement_type",
	"Decorate track particle with type of track state (outlier,hole, biased/unbiased)"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_measurementPhiWidthKey{
	this, "MeasurementPhiWidthKey", "hitResiduals_phiWidth",
	"Decorate track particle with measurement cluster size (in r-phi)"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_measurementEtaWidthKey{
	this, "MeasurementEtaWidthKey", "hitResiduals_etaWidth",
	"Decorate track particle with measurement cluster size (in eta)"};
      
      
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_residualLocXkey{
	this, "ResidualLocXkey", "hitResiduals_residualLocX",
	"Decorate track particle with unbiased residual in local x"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_pullLocXkey{
	this, "PullLocXkey", "hitResiduals_pullLocX",
	"Decorate track particle with unbiased pull in local x"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_measurementLocXkey{
	this, "MeasurementLocXkey", "measurementLocX",
	"Decorate track particle with measurement local x"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trackParameterLocXkey{
	this, "TrackParameterLocXkey", "trackParamLocX",
	"Decorate track particle with unbiased prediction in local x"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_measurementLocCovXkey{
	this, "MeasurementLocCovXkey", "measurementLocCovX",
	"Decorate track particle with local x measurement covariance"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trackParameterLocCovXkey{
	this, "TrackParameterLocCovXkey", "trackParameterLocCovX",
	"Decorate track particle with unbiased local x prediction covariance"};
      
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_residualLocYkey{
	this, "ResidualLocYkey", "hitResiduals_residualLocY",
	"Decorate track particle with unbiased residual in local y"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_pullLocYkey{
	this, "PullLocYkey", "hitResiduals_pullLocY",
	"Decorate track particle with unbiased pull in local y"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_measurementLocYkey{
	this, "MeasurementLocYkey", "measurementLocY",
	"Decorate track particle with measurement local y"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trackParameterLocYkey{
	this, "TrackParameterLocYkey", "trackParamLocY",
	"Decorate track particle with unbiased prediction in local y"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_measurementLocCovYkey{
	this, "MeasurementLocCovYkey", "measurementLocCovY",
	"Decorate track particle with local y measurement covariance"};
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trackParameterLocCovYkey{
	this, "TrackParameterLocCovYkey", "trackParameterLocCovY",
	"Decorate track particle with unbiased local y prediction covariance"};
    };
}

#endif


