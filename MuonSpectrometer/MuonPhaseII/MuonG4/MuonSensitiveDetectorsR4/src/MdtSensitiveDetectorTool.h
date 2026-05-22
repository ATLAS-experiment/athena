/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONG4R4_MDTSENSITIVEDETECTORTOOL_H
#define MUONG4R4_MDTSENSITIVEDETECTORTOOL_H

#include "MuonSensitiveDetectorTool.h"

namespace MuonG4R4 {
    /**  @brief Tool implementation that creates the MDT sensitive detector. */
    class MdtSensitiveDetectorTool : public MuonSensitiveDetectorTool {
        public:
            /** @brief Use the standard Athena tool constructor */
            using MuonSensitiveDetectorTool::MuonSensitiveDetectorTool;
            /** @brief default the destructor */
            ~MdtSensitiveDetectorTool()=default;
        protected:
            /** @brief Override the hook creating the sensitive detector 
             *         to return a new MdtSensitiveDetector instance */
            virtual G4VSensitiveDetector* makeSD() const override final;
   
};
}

#endif
