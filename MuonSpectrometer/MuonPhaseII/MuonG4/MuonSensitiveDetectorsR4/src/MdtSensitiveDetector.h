/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSENSITIVEDETECTORSR4_MDTSENSITIVEDETECTOR_H
#define MUONSENSITIVEDETECTORSR4_MDTSENSITIVEDETECTOR_H


/**
    @section MdtSensitiveDetector Class methods and properties
 **   
The method MdtSensitiveDetector::ProcessHits is executed by the G4 kernel each
time a charged particle (or a geantino) crosses one of the MDT Sensitive Gas
volumes.

Once a G4Step is perfomed by the particle in the sensitive volume (both when
the particle leaves the tube or stops in it), Pre and
PostStepPositions are tranformed into local coordinates (chamber reference
system, with Z along the tube direction and XY tranversal plane) and used to
calculate the local direction of the track. 

The class navigates the TouchableHistory upstream to identfy the readout element 
on the Athena side which is traversed by the Geant4 track. The actual crossed tube
is then identified from comparing the translational part of the G4 volume w.r.t.
the first tube in the first tube layer. Finally, the hit is send to the 
`MuonSensitiveDetector` mother clas for recording. */

#include <GeoPrimitives/GeoPrimitives.h>

#include <StoreGate/WriteHandle.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>
#include <AthenaBaseComps/AthMessaging.h>

#include "MuonSensitiveDetector.h"



class G4TouchableHistory;


namespace MuonG4R4 {


   class MdtSensitiveDetector : public MuonSensitiveDetector {
      public:
         /** @brief Recylce the constructor from the MuonSensitiveDetector */
         using MuonSensitiveDetector::MuonSensitiveDetector;
         /** @brief Default destructor */
         ~MdtSensitiveDetector() = default;
         /** @copydoc MuonSensitiveDetector::ProcessHits */
         virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override final;
      private:
         /** @brief Retrieves the MuonReadoutElement associated to the multilayer in which
          *         the energy deposit is taking place
          *  @param touchHist: Touchable history of the G4 track to find the proper volume name
          *                     from which the re element can be deduced */   
         const MuonGMR4::MdtReadoutElement* getReadoutElement(const G4TouchableHistory* touchHist) const;
         /** @brief Constructs the identifier of the actual tube where the energy deposit
          *         is taking place.
          *  @gctx: Geometry context to access the local -> global transform of the 
          *         readout element
          *  @param reElement: The readout element associated with the multi layer
          *  @param touchHist: Touchable history of the G4 track to fetch the transform
          *                    of the G4 volume */
         Identifier getIdentifier(const ActsTrk::GeometryContext& gctx,
                                 const MuonGMR4::MdtReadoutElement* reElement,
                                 const G4TouchableHistory* touchHist) const;
   };

}
#endif
