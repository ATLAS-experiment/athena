/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDING_TRACKTOTRUTHASSOCIATIONALG_H
#define ACTSTRKFINDING_TRACKTOTRUTHASSOCIATIONALG_H 1

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "Gaudi/Property.h"

// Handle Keys
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "ActsEvent/TrackToTruthParticleAssociation.h"
#include "ActsEvent/MeasurementToTruthParticleAssociation.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
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
  constexpr bool TrackToTruthParticleAssociationDebugHists = false;

  class TrackToTruthAssociationAlg : public AthReentrantAlgorithm
  {
  public:
    TrackToTruthAssociationAlg(const std::string &name,
                               ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
     PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool
        {this, "TrackingGeometryTool", ""};

     SG::ReadHandleKey<ActsTrk::TrackContainer> m_tracksContainerKey
        {this, "ACTSTracksLocation", "","Track collection (ActsTrk variant)"};

     SG::ReadHandleKey<MeasurementToTruthParticleAssociation>  m_pixelClustersToTruth
        {this, "PixelClustersToTruthAssociationMap", "", "Association map from pixel measurements to generator particles." };
     SG::ReadHandleKey<MeasurementToTruthParticleAssociation>  m_stripClustersToTruth
        {this, "StripClustersToTruthAssociationMap", "", "Association map from strip measurements to generator particles." };
     SG::ReadHandleKey<MeasurementToTruthParticleAssociation>  m_hgtdClustersToTruth
        {this, "HgtdClustersToTruthAssociationMap", "", "Association map from HGTD measurements to generator particles." };

     SG::WriteHandleKey<TrackToTruthParticleAssociation>  m_trackToTruthOut
        {this, "AssociationMapOut", "", "Output association map from measurements to generator particles." };

     Gaudi::Property<float> m_maxEnergyLoss
        {this, "MaxEnergyLoss", 10e12, "Stop moving up the decay chain if the energy loss is above  this value." };

     ElasticDecayUtil<TrackToTruthParticleAssociationDebugHists> m_elasticDecayUtil;
     static constexpr float s_unitGeV = 1e3;
     std::conditional<TrackToTruthParticleAssociationDebugHists,
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
                                          Empty>::type m_measPerTrack ATLAS_THREAD_SAFE {20,-.5,40.-.5};
        mutable typename std::conditional<IsDebug,
                                          ActsUtils::StatHist,
                                          Empty>::type m_truthParticlesPerTrack ATLAS_THREAD_SAFE {20,-.5,20.-.5};

        template <class T_OutStream>
        void dumpStatistics(T_OutStream &out) const;
        void fillStatistics(unsigned int n_measurements, unsigned int n_particles) const;
     };
     AssociationCounter<TrackToTruthParticleAssociationDebugHists> m_associationCounter;
     static constexpr unsigned int s_NCounterForAssociatedTruth = 4;
     mutable std::array<std::atomic<std::size_t>,s_NCounterForAssociatedTruth> m_nTracksWithAssociatedTruth ATLAS_THREAD_SAFE {};
     mutable std::atomic<std::size_t> m_nIncompatibleMeasurementContainer{};
     mutable std::atomic<std::size_t> m_nCcompatibleMeasurementContainer{};
  };

} // namespace

#endif
