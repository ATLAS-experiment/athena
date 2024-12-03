/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_UTILFUNCTIONS_H
#define XAODMUONPREPDATA_UTILFUNCTIONS_H

#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "MuonReadoutGeometryR4/MuonReadoutElement.h"
namespace xAOD{
    /** @brief Returns the position of the uncalibrated muon measurement in the attached
     *         Muon chamber frame
     * @param gctx: Geometry context to calculate the relative alignment between chamber & measurement
     * @param meas: Uncalibrated muon measurement
     */
    Amg::Vector3D positionInChamber(const ActsGeometryContext& gctx,
                                    const UncalibratedMeasurement* meas);

    /*** @brief Returns the direction of the measurement channel in the attached muon chamber frame
     *   @param gctx: Geometry context to calculate the relative alignment between chamber & measurement
     *   @param meas: Uncalibrated muon measurement
     */
    Amg::Vector3D channelDirInChamber(const ActsGeometryContext& gctx,
                                      const UncalibratedMeasurement* meas);
    /*** @brief Returns the precision axis of the measurement, i.e. the vector pointing to the
     *          next strip or tube, in the attached muon chamber frame
     *   @param gctx: Geometry context to calculate the relative alignment between chamber & measurement
     *   @param meas: Uncalibrated muon measurement         
    */
    Amg::Vector3D channelNormalInChamber(const ActsGeometryContext& gctx,
                                         const UncalibratedMeasurement* meas);
    
    /** @brief Returns the associated readout element to the measurement*/
    const MuonGMR4::MuonReadoutElement* readoutElement(const UncalibratedMeasurement* meas);
    /** @brief Returns the associated identifier from the muon measurement */
    const Identifier& identify(const UncalibratedMeasurement* meas);
}

#endif