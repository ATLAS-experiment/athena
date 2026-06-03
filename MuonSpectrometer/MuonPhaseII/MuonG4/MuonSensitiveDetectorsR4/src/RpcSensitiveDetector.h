#ifndef MUONG4R4_RPCSensitiveDetector_H
#define MUONG4R4_RPCSensitiveDetector_H
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonSensitiveDetector.h"

#include <MuonReadoutGeometryR4/RpcReadoutElement.h>

namespace MuonG4R4 {
   /** @brief Sensitive detector implementation to record G4 hits in the
     *        Rpc detectors. The ProcessHits hook is called by Geant4
     *        if the track enters a sensible Rpc gas gap volume. The TouchableHistory is 
     *        used to deduce the associated readout element and then to identify
     *        the actual gas gap. The hit is then passed to the `MuonSensitiveDetector` 
     *        class for event record */
   class RpcSensitiveDetector : public MuonSensitiveDetector {
      public:
        /** @brief Recycle the constructor from the MuonSensitiveDetector */
        using MuonSensitiveDetector::MuonSensitiveDetector;
        /** @brief Default destructor */  
        ~RpcSensitiveDetector() = default;
        /** @copydoc MuonSensitiveDetector::ProcessHits */
        virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override final;
      private:
         /** @brief Retrieves the MuonReadoutElement associated to the rpc chamber in which
          *         the energy deposit is taking place
          *  @param touchHist: Touchable history of the G4 track to find the proper volume name
          *                    from which the readout element can be deduced */
         const MuonGMR4::RpcReadoutElement* getReadoutElement(const G4TouchableHistory* touchHist) const;
         /** @brief Identify the gas gap in which the G4 hit produced
           * @param gctx: Geometry context to retrieve the center positions of the 
           *               readout element's gas gaps
           * @param readOutEle: The previously identified readout element
           * @param hitAtGapPlane: Position of the G4 volume within the ATLAS
           *                       coordinate system */
         Identifier getIdentifier(const ActsTrk::GeometryContext& gctx,
                                  const MuonGMR4::RpcReadoutElement* readOutEle, 
                                  const Amg::Vector3D& hitAtGapPlane) const;
   };
}
#endif
