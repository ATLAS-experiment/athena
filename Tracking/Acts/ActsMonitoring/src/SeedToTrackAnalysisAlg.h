/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKANALYSIS_SEEDTOTRACKANALYSISALG_H
#define ACTSTRKANALYSIS_SEEDTOTRACKANALYSISALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "ActsEvent/SeedContainer.h"
#include "ActsEvent/TrackParametersContainer.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "ActsEvent/MeasurementToTruthParticleAssociation.h"
#include "ActsTruth/ElasticDecayUtil.h"

namespace ActsTrk {

  class SeedToTrackAnalysisAlg final :
    public AthMonitorAlgorithm {    
  public:
    SeedToTrackAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~SeedToTrackAnalysisAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:
    StatusCode getTruthProbability(const ActsTrk::Seed& seed,
				   const std::vector< const ActsTrk::MeasurementToTruthParticleAssociation* >& associationMaps,
				   float& probability) const;
  
  private:
    SG::ReadHandleKey< ActsTrk::SeedContainer > m_seedsKey {this, "InputSeedCollection", ""};
    SG::ReadHandleKey< ActsTrk::BoundTrackParametersContainer > m_paramsKey {this, "InputTrackParamsCollection", ""};
    SG::ReadHandleKey< std::vector<int> > m_destiniesKey {this, "InputDestinyCollection", ""};
    SG::ReadCondHandleKey< InDet::BeamSpotData > m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};

    // Truth
    enum DetectorType : std::uint8_t {PIXEL=0, STRIP, nTypes};
    SG::ReadHandleKey<ActsTrk::MeasurementToTruthParticleAssociation> m_pixelAssociuationMapKey {this, "PixelTruthAssociationMap", ""};
    SG::ReadHandleKey<ActsTrk::MeasurementToTruthParticleAssociation> m_stripAssociuationMapKey {this, "StripTruthAssociationMap", ""};
    
    static const int m_nLayers{5};
    std::vector<int> m_seedVars {};

    static constexpr float s_unitGeV = 1e3;
    EmptyProperty m_energyLossBinning {this, "EnergyLossBinning", {20.,0.,5.*s_unitGeV}, "Binning to be used for the energy loss histograms." }; 
    Gaudi::Property< float > m_maxEnergyLoss {this, "MaxEnergyLoss", 10e12, "Stop moving up the decay chain if the energy loss is above  this value." };
    ElasticDecayUtil< false > m_elasticDecayUtil {};
  };
  
}

#endif
