/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUON_VERSIONS_MUONAUXCONTAINER_V6_H
#define XAODMUON_VERSIONS_MUONAUXCONTAINER_V6_H
 
// System include(s):
#include <stdint.h>
#include <vector>

// Core include(s):
#include "AthLinks/ElementLink.h"
#include "xAODCore/AuxContainerBase.h"
 
// xAOD include(s):
#include "xAODTracking/TrackParticleContainer.h" 
#include "xAODCaloEvent/CaloClusterContainer.h" 
#include "xAODMuon/MuonSegmentContainer.h" 
 
namespace xAOD {
   /** @brief Auxiliary container defining the set of standard
    *         muon attributes saved in the Phase II software */
   class MuonAuxContainer_v6 : public AuxContainerBase {
 
   public:
      /// Default constructor
      MuonAuxContainer_v6();
 
   private:
      /** @brief Abrivate the link to the track particles */
      using TrackLink_t = ElementLink<xAOD::TrackParticleContainer>;
      using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
      /// @name iParticle values
      /// @{                          
     std::vector<float> pt{};
     std::vector<float> eta{};
     std::vector<float> phi{};
     std::vector<float> charge{};
     /// @}  
     /// @name Muon summary information
     /// @{ 	
     std::vector<std::uint16_t> allAuthors{};
     std::vector<std::uint16_t> author{};
     std::vector<std::uint16_t> muonType{};
     std::vector<std::uint8_t> quality{}; //quality, passesIDCuts and passesHighPtCuts are packed in here.

      /// @}
      /** @brief Precision hits on track sorted by layer 
       *         and large/small sectors */
      std::vector<std::uint8_t> innerSmallHits{};
      std::vector<std::uint8_t> innerLargeHits{};
      std::vector<std::uint8_t> middleSmallHits{};
      std::vector<std::uint8_t> middleLargeHits{};
      std::vector<std::uint8_t> outerSmallHits{};
      std::vector<std::uint8_t> outerLargeHits{};
      std::vector<std::uint8_t> extendedSmallHits{};
      std::vector<std::uint8_t> extendedLargeHits{};
      /** @brief Detected precision holes albeit there 
       *         should be a hit on track */
      std::vector<std::uint8_t> innerSmallHoles{};
      std::vector<std::uint8_t> innerLargeHoles{};
      std::vector<std::uint8_t> middleSmallHoles{};
      std::vector<std::uint8_t> middleLargeHoles{};
      std::vector<std::uint8_t> outerSmallHoles{};
      std::vector<std::uint8_t> outerLargeHoles{};
      std::vector<std::uint8_t> extendedSmallHoles{};
      std::vector<std::uint8_t> extendedLargeHoles{};
      /** @brief Precision hits that are counted as outliers */
      std::vector<std::uint8_t> innerClosePrecisionHits{};
      std::vector<std::uint8_t> middleClosePrecisionHits{}; 
      std::vector<std::uint8_t> outerClosePrecisionHits{}; 
      std::vector<std::uint8_t> extendedClosePrecisionHits{};

      /** @brief Trigger eta and phi hits in the inner station (RPC/TGC/sTGC - wire) */
      std::vector<std::uint8_t> innerTriggerEtaHits{};
      std::vector<std::uint8_t> innerTriggerPhiHits{};
      std::vector<std::uint8_t> innerTriggerEtaHoles{};
      std::vector<std::uint8_t> innerTriggerPhiHoles{};

      /** @brief eta and phi trigger hits in the middle station (TGC/RPC) */
      std::vector<std::uint8_t> middleTriggerEtaHits{};
      std::vector<std::uint8_t> middleTriggerPhiHits{};
      std::vector<std::uint8_t> middleTriggerEtaHoles{};
      std::vector<std::uint8_t> middleTriggerPhiHoles{};

      /** @brief Trigger eta and phi hits in the outer station (RPC) */
      std::vector<std::uint8_t> outerTriggerEtaHits{};
      std::vector<std::uint8_t> outerTriggerPhiHits{};
      std::vector<std::uint8_t> outerTriggerEtaHoles{};
      std::vector<std::uint8_t> outerTriggerPhiHoles{};
      /** @brief Count the number of associated sTgc pad hits */
      std::vector<std::uint8_t> sTgcPadHits{};
      /// @name Links 
      /// @{    
      std::vector<TrackLink_t> inDetTrackParticleLink{};
      std::vector<TrackLink_t> combinedTrackParticleLink{};
      std::vector<TrackLink_t> muonSpectrometerTrackParticleLink{};

      std::vector<std::vector<SegLink_t>> muonSegmentLinks{};
      /// @}
      
      /// @name Energy loss 
      /// @{
      std::vector<std::uint8_t> energyLossType{};
      /// @}
      
      /// @name Param defs 
      /// @{
      std::vector<float> spectrometerFieldIntegral{};
      std::vector<float> scatteringCurvatureSignificance{};
      std::vector<float> scatteringNeighbourSignificance{};
      std::vector<float> momentumBalanceSignificance{};
      std::vector<float> segmentDeltaEta{};
      std::vector<float> segmentDeltaPhi{};

      std::vector<float> CaloMuonScore{};
      std::vector<float> EnergyLoss{};
      std::vector<float> EnergyLossSigma{};
      /// @}
 
   }; // class MuonAuxContainer_v5
 
} // namespace xAOD
 
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::MuonAuxContainer_v6, xAOD::AuxContainerBase ); 
 
#endif
