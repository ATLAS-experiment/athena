/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSENSITIVEDETECTORSR4_MMSENSITIVEDETECTOR_H
#define MUONSENSITIVEDETECTORSR4_MMSENSITIVEDETECTOR_H

/** @class MmSensitiveDetector
    @section MmSensitiveDetector Class methods and properties

The method MmSensitiveDetector::ProcessHits is executed by the G4 kernel each
time a particle crosses one of the Mm gas gaps.
Navigating with the touchableHistory method GetHistoryDepth()
through the hierarchy of volumes crossed by the particle,
the Sensitive Detector determines the correct set of Simulation Identifiers
to associate to each hit. The Mm SimIDs are 32-bit unsigned integers, built 
using the MuonSimEvent/MmHitIdHelper class
which inherits from the MuonHitIdHelper base class. 


*/

#include "MuonSensitiveDetector.h"


#include <MuonReadoutGeometryR4/MmReadoutElement.h>

namespace MuonG4R4 {

class MmSensitiveDetector : public MuonSensitiveDetector {


    public:
        using MuonSensitiveDetector::MuonSensitiveDetector;
    
        ~MmSensitiveDetector()=default;
    
        /** member functions */
        virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override final;

    
    private:
        /// Retrieves the matching readout element to a G4 hit
        const MuonGMR4::MmReadoutElement* getReadoutElement(const ActsGeometryContext& gctx,
                                                            const G4TouchableHistory* touchHist) const;
        /// Identify the gasGap layer of the hit
        Identifier getIdentifier(const ActsGeometryContext& gctx,
                                const MuonGMR4::MmReadoutElement* readOutEle, 
                                const Amg::Vector3D& hitAtGapPlane) const;
   
    };
}
#endif
