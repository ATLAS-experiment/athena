/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_TRUTHPARTICLEHITCOUNT_H
#define ACTSTRK_TRUTHPARTICLEHITCOUNT_H 1

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "Gaudi/Property.h"

// Handle Keys
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "ActsEvent/TruthParticleHitCounts.h"
#include "ActsEvent/MeasurementToTruthParticleAssociation.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsTruth/ElasticDecayUtil.h"

#include <mutex>
#include "ActsInterop/StatUtils.h"

#include <string>
#include <memory>
#include <array>
#include <atomic>
#include <type_traits>

#include <cmath>
#include <iomanip>
#include <ostream>
#include <string>
#include <sstream>
#include <vector>

namespace ActsTrk
{
  constexpr bool TruthParticleHitCountDebugHists = false;

  class TruthParticleHitCountAlg : public AthReentrantAlgorithm
  {
  public:
    TruthParticleHitCountAlg(const std::string &name,
                               ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
     ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool
        {this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};

     SG::ReadHandleKey<MeasurementToTruthParticleAssociation>  m_pixelClustersToTruth
        {this, "PixelClustersToTruthAssociationMap","", "Association map from pixel measurements to generator particles." };
     SG::ReadHandleKey<MeasurementToTruthParticleAssociation>  m_stripClustersToTruth
        {this, "StripClustersToTruthAssociationMap","", "Association map from strip measurements to generator particles." };

     SG::WriteHandleKey<TruthParticleHitCounts>  m_truthHitCountsOut
        {this, "TruthParticleHitCountsOut","", "Map from truth particle to hit counts." };

     Gaudi::Property<float> m_maxEnergyLoss
        {this, "MaxEnergyLoss", 10e12, "Stop moving up the decay chain if the energy loss is above  this value." };
     Gaudi::Property<unsigned int> m_nHitsMin
        {this, "NHitsMin", 1, "Minimum number of hits associated to a truth particle to consider the particle." };

     ElasticDecayUtil<TruthParticleHitCountDebugHists> m_elasticDecayUtil;
     static constexpr float s_unitGeV = 1e3;
     std::conditional<TruthParticleHitCountDebugHists,
                      Gaudi::Property<std::vector<float> >,
                      EmptyProperty >::type              m_energyLossBinning
        {this, "EnergyLossBinning", {20.,0.,5.*s_unitGeV}, "Binning to be used for the energy loss histograms." };

     template <bool IsDebug>
     struct AssociationCounter {
        struct Empty {
           template <typename... T_Args>
           Empty(T_Args... ) {}
        };
        mutable typename std::conditional<IsDebug,
                                          std::mutex,
                                          Empty>::type m_mutex ATLAS_THREAD_SAFE;
        mutable typename std::conditional<IsDebug,
                                          ActsUtils::StatHist,
                                          Empty>::type m_measPerTruthParticle ATLAS_THREAD_SAFE {20,-.5,40.-.5};

        template <class T_OutStream>
        void dumpStatistics(T_OutStream &out) const;
        void fillStatistics(unsigned int n_measurements) const;
     };
     AssociationCounter<TruthParticleHitCountDebugHists> m_associationCounter;
     mutable std::atomic<std::size_t> m_nTruthParticlesWithHits ATLAS_THREAD_SAFE {};
  };

} // namespace

#endif
