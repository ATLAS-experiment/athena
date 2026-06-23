/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSENSITIVEDETECTORSR4_MMSENSITIVEDETECTOR_H
#define MUONSENSITIVEDETECTORSR4_MMSENSITIVEDETECTOR_H

#include "MuonSensitiveDetector.h"


#include <MuonReadoutGeometryR4/MmReadoutElement.h>

namespace MuonG4R4 {
/** @brief Sensitive detector implementation to record G4 hits in the
 *         micromega detectors. The ProcessHits hook is called by Geant4
 *         if the track enters a sensible Micromega gas gap volume
 *         The TouchableHistory is used to deduce the associated readout element
 *         and then to identify the concrete gas gap. The hit is then passed to the 
 *         `MuonSensitiveDetector` class for event record */
class MmSensitiveDetector : public MuonSensitiveDetector {
    public:
        /** @brief Recycle the constructor from the MuonSensitiveDetector */
        using MuonSensitiveDetector::MuonSensitiveDetector;
        /** @brief Default destructor */  
        ~MmSensitiveDetector() = default;
        /** @copydoc MuonSensitiveDetector::ProcessHits */
        virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override final;
    private:
        /** @brief Retrieves the readout element matching the Micromega multiplet
         *         in which the G4 energy depsoit is taking place. The sector and the
         *         station eta can be deduced from the Touchable history. The actual
         *         multiplet is deduced from a distance comparison of the transform
         *         in the touchable history and the particular readout elements
         *  @param gctx: The Geometry context to fetch the multiplet centers described
         *               by the muon readout element
         *  @param touchHist: The touchable history attributed to the G4Track used to 
         *                    identify the Micromega wedge and the associated G4 volume
         *                    transforms */
        const MuonGMR4::MmReadoutElement* getReadoutElement(const ActsTrk::GeometryContext& gctx,
                                                            const G4TouchableHistory* touchHist) const;
        /** @brief Identify the gas gap in which the G4 hit produced
         *  @param gctx: Geometry context to retrieve the center positions of the 
         *               readout element's gas gaps
         * @param readOutEle: The previously identified readout element
         * @param hitAtGapPlane: Position of the G4 volume within the ATLAS
         *                       coordinate system */
        Identifier getIdentifier(const ActsTrk::GeometryContext& gctx,
                                const MuonGMR4::MmReadoutElement* readOutEle, 
                                const Amg::Vector3D& hitAtGapPlane) const;
   
    };
}
#endif
