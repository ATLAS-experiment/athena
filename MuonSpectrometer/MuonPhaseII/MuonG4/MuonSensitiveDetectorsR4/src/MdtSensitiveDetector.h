/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

Two cases are given:
1) the particle over-passed the wire: here the drift radius (the impact 
   parameter) is computed as the local direction distance from the wire;  
2) the step doesn't allow the particle to over-pass the wire: here the shortest 
   distance to the wire, which is one of the two
   end-points, is calculated for each step which occurs inside the sensitive 
   volumes and only the closer one is kept; this case includes also the hard
   scattering case inside the sensitive volume.

Navigating with the touchableHistory method GetHistoryDepth()
through the hierarchy of the volumes crossed by the particles, the 
MdtSensitiveDetector determinates the
correct set of geometry parameters to be folded in the Simulation Identifier
associated to each hit.

We describe in the following, how each field of the identifier is retrieved.

1) stationName, stationEta, stationPhi: when a volume is found in the hierarchy,
   whose name contains the substring "station", the stationName is extracted from
   the volume's name; stationPhi and stationEta are calculated starting from the
   volume copy number, assigned by MuonGeoModel.

2) multilayer: when a volume is found in the hierarchy,
   whose name contains the substring "component", the multilayer is set to 1 or to
   2, according to the component number (multilayer=1 if the component is 1, 5 or
   8; multilayer=2 if the component is 3, 7, 8, 10, 11 or 14).

3) tubeLayer and tube: when a volume is found in the hierarchy,
   whose name contains the substring "Drift", tubeLayer and tube are calculated 
   starting from the Drift volume copy number.

    @section Some notes:

1) this implementation of the MDT Sensitive Detectors can handle different
   situations: hard scattering of the muon on the sensitive volume (its direction
   changes), soft secondary particles completely included in the sensitive volume,
   muon hits masked by secondaries, like delta rays.

2) for each hit, the time of flight (the G4 globalTime), is recorded and
   associated to the hit.

3) more than none MDT hit can occur in the same tube. The hit selection is done
   at the level of the digitization procedure.

4) the MDTHit object contains: the SimID, the drift radius and the globalTime.


*/

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
         using MuonSensitiveDetector::MuonSensitiveDetector;
   
         ~MdtSensitiveDetector() = default;
    
         virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override final;
    
      private:
         /// Retrieves the matching readout element to a G4 hit
         const MuonGMR4::MdtReadoutElement* getReadoutElement(const G4TouchableHistory* touchHist) const;
         /// Retrieves from the Readoutelement & the touchable history the Identifier
         Identifier getIdentifier(const ActsGeometryContext& gctx,
                                 const MuonGMR4::MdtReadoutElement* reElement,
                                 const G4TouchableHistory* touchHist) const;
   };

}
#endif
