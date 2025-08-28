/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSCALIBRATION_CALIBRATIONCONTEXT_H
#define ACTSCALIBRATION_CALIBRATIONCONTEXT_H

 #include "Acts/Utilities/CalibrationContext.hpp"
 #include "GaudiKernel/EventContext.h"
 namespace ActsTrk{
    /** @brief The Acts::Calibration context is piped through the Acts fitters to (re)calibrate
     *         the Acts::SourceLinks during the track State upate. In ATLAS Athena, the EventContext
     *         is needed to access the calibration context from the conditions. This function 
     *         packs the pointer of the current EventContext into a Acts::CalibrationContext
     * @param ctx: EventContext to transform into a Context */
    inline Acts::CalibrationContext getCalibrationContext(const EventContext& ctx){
        return Acts::CalibrationContext{&ctx};
    }
 }
#endif