/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONG4R4_sTgcSENSITIVEDETECTORTOOL_H
#define MUONG4R4_sTgcSENSITIVEDETECTORTOOL_H

#include "MuonSensitiveDetectorTool.h"

namespace MuonG4R4 {
    /**  @brief Tool implementation that creates the sTgc sensitive detector. */
    class sTgcSensitiveDetectorTool : public MuonSensitiveDetectorTool {
        public:
            /** @brief Use the standard Athena tool constructor */
            using MuonSensitiveDetectorTool::MuonSensitiveDetectorTool;
            /** @brief Default the destructor */
            ~sTgcSensitiveDetectorTool()=default;
        protected:
            /** @brief Override the hook creating the sensitive detector 
             *         to return a new sTgcSensitiveDetector instance */
            G4VSensitiveDetector* makeSD() const override final;
    };
}

#endif
