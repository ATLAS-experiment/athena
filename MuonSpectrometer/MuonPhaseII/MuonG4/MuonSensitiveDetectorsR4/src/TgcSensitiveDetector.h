/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONG4R4_TGCSensitiveDetector_H
#define MUONG4R4_TGCSensitiveDetector_H

#include "MuonSensitiveDetector.h"

#include <StoreGate/WriteHandle.h>
#include <AthenaBaseComps/AthMessaging.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>

namespace MuonG4R4 {

  class TgcSensitiveDetector : public MuonSensitiveDetector {
    public:
      using MuonSensitiveDetector::MuonSensitiveDetector;
      ~TgcSensitiveDetector()=default;
    
      virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override final;

    
    private:
      /** @brief Retrieves the readout element that's associates with the TouchableHistory. 
       *         The readout element is decoded from the volume name in the history.
       *  @param touchHist: History of the hit to determine the readout element from */
      const MuonGMR4::TgcReadoutElement* getReadoutElement(const G4TouchableHistory* touchHist) const;
      /** @brief Constructs the Identifier of the gasGap using the readoutElement and the hit expressed 
       *         in it's local coordinate frame
       *  @param gctx: Geometry context to transform the hit accordingly
       *  @param readOutEle: ReadoutElement that's identified from the TouchableHistory
       *  @param hitAtGapPlane: Position of the hit expressed at the gasGap centre in global coordinates  */
      Identifier getIdentifier(const ActsGeometryContext& gctx,
                               const MuonGMR4::TgcReadoutElement* readOutEle, 
                               const Amg::Vector3D& hitAtGapPlane, bool phiGap) const;
};
}

#endif