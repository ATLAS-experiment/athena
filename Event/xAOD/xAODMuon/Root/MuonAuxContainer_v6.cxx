/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODMuon/versions/MuonAuxContainer_v6.h"
 
namespace xAOD {
 
   MuonAuxContainer_v6::MuonAuxContainer_v6()
      : AuxContainerBase() {

     AUX_VARIABLE(pt);
     AUX_VARIABLE(eta);
     AUX_VARIABLE(phi);
     AUX_VARIABLE(charge);
     /// @}  
     /// @name Muon summary information
     /// @{                                      	
     AUX_VARIABLE(allAuthors);
     AUX_VARIABLE(author);
     AUX_VARIABLE(muonType);
     AUX_VARIABLE(quality); //quality, passesIDCuts and passesHighPtCuts are packed in here.

      /// @}
      /** @brief Precision hits on track sorted by layer 
       *         and large/small sectors */
      AUX_VARIABLE(innerSmallHits);
      AUX_VARIABLE(innerLargeHits);
      AUX_VARIABLE(middleSmallHits);
      AUX_VARIABLE(middleLargeHits);
      AUX_VARIABLE(outerSmallHits);
      AUX_VARIABLE(outerLargeHits);
      AUX_VARIABLE(extendedSmallHits);
      AUX_VARIABLE(extendedLargeHits);
      /** @brief Detected precision holes albeit there 
       *         should be a hit on track */
      AUX_VARIABLE(innerSmallHoles);
      AUX_VARIABLE(innerLargeHoles);
      AUX_VARIABLE(middleSmallHoles);
      AUX_VARIABLE(middleLargeHoles);
      AUX_VARIABLE(outerSmallHoles);
      AUX_VARIABLE(outerLargeHoles);
      AUX_VARIABLE(extendedSmallHoles);
      AUX_VARIABLE(extendedLargeHoles);
      /** @brief Precision hits that are counted as outliers */
      AUX_VARIABLE(innerClosePrecisionHits);
      AUX_VARIABLE(middleClosePrecisionHits); 
      AUX_VARIABLE(outerClosePrecisionHits); 
      AUX_VARIABLE(extendedClosePrecisionHits);

      /** @brief Trigger eta and phi hits in the inner station (RPC/TGC/sTGC - wire) */
      AUX_VARIABLE(innerTriggerEtaHits);
      AUX_VARIABLE(innerTriggerPhiHits);
      AUX_VARIABLE(innerTriggerEtaHoles);
      AUX_VARIABLE(innerTriggerPhiHoles);

      /** @brief eta and phi trigger hits in the middle station (TGC/RPC) */
      AUX_VARIABLE(middleTriggerEtaHits);
      AUX_VARIABLE(middleTriggerPhiHits);
      AUX_VARIABLE(middleTriggerEtaHoles);
      AUX_VARIABLE(middleTriggerPhiHoles);

      /** @brief Trigger eta and phi hits in the outer station (RPC) */
      AUX_VARIABLE(outerTriggerEtaHits);
      AUX_VARIABLE(outerTriggerPhiHits);
      AUX_VARIABLE(outerTriggerEtaHoles);
      AUX_VARIABLE(outerTriggerPhiHoles);
      /** @brief Count the number of associated sTgc pad hits */
      AUX_VARIABLE(sTgcPadHits);
      /// @name Links 
      /// @{    
      AUX_VARIABLE(inDetTrackParticleLink);
      AUX_VARIABLE(combinedTrackParticleLink);
      AUX_VARIABLE(muonSpectrometerTrackParticleLink);

      AUX_VARIABLE(muonSegmentLinks);
      /// @}
      
      AUX_VARIABLE(energyLossType);
      
      AUX_VARIABLE(spectrometerFieldIntegral);
      AUX_VARIABLE(scatteringCurvatureSignificance);
      AUX_VARIABLE(scatteringNeighbourSignificance);
      AUX_VARIABLE(momentumBalanceSignificance);
      AUX_VARIABLE(segmentDeltaEta);
      AUX_VARIABLE(segmentDeltaPhi);

      AUX_VARIABLE(CaloMuonScore);
      AUX_VARIABLE(EnergyLoss);
      AUX_VARIABLE(EnergyLossSigma);
      
   }
 
} // namespace xAOD
