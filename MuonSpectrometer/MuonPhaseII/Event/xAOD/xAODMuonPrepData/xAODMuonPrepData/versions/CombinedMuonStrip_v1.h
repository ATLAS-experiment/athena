/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_COMBINEDMUONSTRIP_V1_H
#define XAODMUONPREPDATA_VERSION_COMBINEDMUONSTRIP_V1_H

//
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
namespace xAOD{
    /** @brief The Acts fitters running on the Uncalibrated measurements are uncapable of 
     *         producing two track states on the same surface or alternatively, the propagation
     *         onto two surfaces at the same position is numerically challenging due to the 
     *         zeroish step length. Therefore, the R4 geometry only produces one set of surfaces
     *         where local x is oriented with the eta measurement's direction. The CombinedMuonStrip
     *         provides a mechanism to pipe the two eta & phi measurements in the same gas gap through
     *         the fitting infrastructre. By convention, the combined Muon strip returns the same measurement
     *         type as the prds that they're carrying but the dimension is always zero providing a handle
     *         to properly distinguish them in the SpacePointCalibrator without the useage of dynamic_casts */
    class CombinedMuonStrip_v1 : public UncalibratedMeasurement {
        public:
            /** @brief Empty constructor */
            CombinedMuonStrip_v1() = default;
            /** @brief virtual destructor */
            virtual ~CombinedMuonStrip_v1() = default;
            /** @brief */
            virtual xAOD::UncalibMeasType type() const override final;
            /** @brief Specify the number of dimensions as zero -> handle in the calibrator */
            unsigned numDimensions() const override { return 0; } 
            /** @brief Returns the primary associated measurement */
            const xAOD::UncalibratedMeasurement* primaryStrip() const;
            /** @brief Links a prd measurement as primary meaurement */
            void setPrimaryStrip(const xAOD::UncalibratedMeasurement* meas);
            /** @brief Returns the secondary associated measurement */
            const xAOD::UncalibratedMeasurement* secondaryStrip() const;
            /** @brief Linkt a prd measurement as secondary measurement */
            void setSecondaryStrip(const xAOD::UncalibratedMeasurement* meas);
    };
}

#endif