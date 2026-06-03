/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONG4R4_MmSENSITIVEDETECTORTOOL_H
#define MUONG4R4_MmSENSITIVEDETECTORTOOL_H
#include "MuonSensitiveDetectorTool.h"

namespace MuonG4R4 {
    /**  @brief Tool implementation that creates the Micromega sensitive detector. */
    class MmSensitiveDetectorTool : public MuonSensitiveDetectorTool {
        public:
            /** @brief Use the standard Athena tool constructor */
            using MuonSensitiveDetectorTool::MuonSensitiveDetectorTool;
            /** @brief Default the destructor */
            ~MmSensitiveDetectorTool() = default;

        protected:
            /** @brief Override the hook creating the sensitive detector 
             *         to return a new MmSensitiveDetector instance */
            virtual G4VSensitiveDetector* makeSD() const override final;
    };
}

#endif
