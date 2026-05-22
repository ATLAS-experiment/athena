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
   /** @brief Sensitive detector implementation to record G4 hits in the
     *        Tgc detectors. The ProcessHits hook is called by Geant4
     *        if the track enters a sensible Rpc gas gap volume. The TouchableHistory is 
     *        used to deduce the associated readout element and then to identify
     *        the actual gas gap. The hit is then passed to the `MuonSensitiveDetector` 
     *        class for event record */
  class TgcSensitiveDetector : public MuonSensitiveDetector {
    public:
        /** @brief Recycle the constructor from the MuonSensitiveDetector */
        using MuonSensitiveDetector::MuonSensitiveDetector;
        /** @brief Default destructor */  
        ~TgcSensitiveDetector()=default;
        /** @copydoc MuonSensitiveDetector::ProcessHits */
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
      Identifier getIdentifier(const ActsTrk::GeometryContext& gctx,
                               const MuonGMR4::TgcReadoutElement* readOutEle, 
                               const Amg::Vector3D& hitAtGapPlane, bool phiGap) const;
};
}

#endif