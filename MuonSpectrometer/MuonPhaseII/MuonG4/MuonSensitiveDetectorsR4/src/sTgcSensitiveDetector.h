#ifndef MUONG4R4_sTgcSensitiveDetector_H
#define MUONG4R4_sTgcSensitiveDetector_H
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/** @class sTgcSensitiveDetector
    @section sTgcSensitiveDetector Class methods and properties

The method sTgcSensitiveDetector::ProcessHits is executed by the G4 kernel each
time a particle crosses one of the sTgc gas gaps.
Navigating with the touchableHistory method GetHistoryDepth()
through the hierarchy of volumes crossed by the particle,
the Sensitive Detector determines the correct set of Simulation Identifiers
to associate to each hit. The sTgc SimIDs are 32-bit unsigned integers, built 
using the MuonSimEvent/sTgcHitIdHelper class
which inherits from the MuonHitIdHelper base class. 

We describe here how each field of the identifier is determined.

1) stationName, stationEta, stationPhi: when a volume is found in the hierarchy
   whose name contains the substring "station", the stationName is extracted
   from the volume's name. stationEta and stationPhi values are calculated
   starting from the volume's copy number.
   
2) doubletR:  when a volume is found in the hierarchy whose name contains
   the substring "sTgccomponent", its copy number is used to calulate the
   doubletR of the hit.
   
3) gasGap: the substring "layer" is searched through the names of the volumes
   in the hierarchy. When a the volume is found, its copy number is used,
   together with the sign of the copy number of the "sTgccomponent" to decide
   what is the correct gasGap value.
   
4) doubletPhi: when the volume with the substring "gas volume" in its name is
   found, its copy number is enough to determine the doubletPhi of the hit
   if the hit is registered in a standard chamber. For special chambers (i.e.
   chambers with only one gas gap per layer readout by two strip panels per
   direction, or chambers in the ribs) some a special attribution is done
   using also the stationName.
   
5) doubletZ: "gazGap" is the required substring in the volume's name. doubletZ
   attribution is fairly complicated. For standard chambers it just uses the
   sign of the copy number of the "sTgccomponent", but severa
   arrangements (based on the stationName and the technology name) are needed
   for special chambers.
   
6) the doubletPhi calculated as described above needs one further correction
   for some chambers to take into account chamber rotation before placement
   in the spectrometer. This is done just before creating the hit.

    @section Some notes:

1) presently, chamber efficiency is assumed to be 100% at the level of the
   Sensitive Detector, i.e. two hits (one in eta and one in phi) are created
   each time the sensitive detector is created.

2) the hits created as described above contain information about their local
   position *in the gas gap* and the simulation identifier of *the strip panel*

3) the strip number is not assigned by the sensitive detector, and must be
   calculated by the digitization algorithm.

4) points 2 and 3 are due to the necessity of not introducing any dependency
   on the geometry description in the Sensitive Detector.

5) the present version of the Sensitive Detector produces hits which are
   different depending on whether hand coded G4 geometry or GeoModel is used
   for the geometrical description of the setup. When hand coded geometry
   is used, both GeoModel and the old MuonDetDescr can be used for digitization
   (using a proper tag of sTgc_Digitization), while if GeoModel is used in the
   simulation, *it must be used* also in the digitization.

6) for each hit, the time of flight (the G4 globalTime), is recorded and
   associated to the hit.

7) the sTgcHit object contains: the SimID, the globalTime, the hit local position
   and the track barcode.


*/

#include "MuonSensitiveDetector.h"
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>

namespace MuonG4R4 {
   /** @brief Sensitive detector implementation to record G4 hits in the
     *        sTgc detectors. The ProcessHits hook is called by Geant4
     *        if the track enters a sensible Rpc gas gap volume. The TouchableHistory is 
     *        used to deduce the associated readout element and then to identify
     *        the actual gas gap. The hit is then passed to the `MuonSensitiveDetector` 
     *        class for event record */
   class sTgcSensitiveDetector : public MuonSensitiveDetector {
      public:
         /** @brief Recycle the constructor from the MuonSensitiveDetector */
         using MuonSensitiveDetector::MuonSensitiveDetector;
         /** @brief Default destructor */  
         ~sTgcSensitiveDetector()=default;
         /** @copydoc MuonSensitiveDetector::ProcessHits */
         virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override final;
      private:
         /** @brief Retrieves the MuonReadoutElement associated to the rpc chamber in which
          *         the energy deposit is taking place
          *  @param touchHist: Touchable history of the G4 track to find the proper volume name
          *                    from which the readout element can be deduced */
         const MuonGMR4::sTgcReadoutElement* getReadoutElement(const ActsTrk::GeometryContext& gctx,
                                                               const G4TouchableHistory* touchHist) const;
         /** @brief Identify the gas gap in which the G4 hit produced
           * @param gctx: Geometry context to retrieve the center positions of the 
           *               readout element's gas gaps
           * @param readOutEle: The previously identified readout element
           * @param hitAtGapPlane: Position of the G4 volume within the ATLAS
           *                       coordinate system */
         Identifier getIdentifier(const ActsTrk::GeometryContext& gctx,
                                  const MuonGMR4::sTgcReadoutElement* readOutEle, 
                                  const Amg::Vector3D& hitAtGapPlane) const;
   };
}
#endif
