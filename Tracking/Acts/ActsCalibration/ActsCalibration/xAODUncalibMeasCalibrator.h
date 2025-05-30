/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSCALIBRATION_DETAIL_XAODUNCALIBMEASCALIBRATOR_H
#define ACTSCALIBRATION_DETAIL_XAODUNCALIBMEASCALIBRATOR_H

#include "ActsCalibration/MeasurementCalibratorBase.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "ActsEvent/TrackContainer.h"
#include "Acts/TrackFitting/GlobalChiSquareFitter.hpp"
#include "CxxUtils/ArrayHelper.h"

namespace ActsTrk::detail{
    /** @brief Source link calibrator implementation for xAOD::Uncalibrated measurements. The class provides the
     *         methods to pack & unpack Acts::SourceLinks from Uncalibrated measurements and also the calibrate method
     *         which will be connected to the Acts::Fitter's calibrator delegator. However, this method is nothing else than
     *         a forward dispatch table, which is calling other instances of calibrators doing the actual work. 
     *         For each measurement type to calibrate, the actual calibrator needs to be connected with this class instance */
    class xAODUncalibMeasCalibrator : public MeasurementCalibratorBase {
        public:
          /** @brief Empty default constructor */
          xAODUncalibMeasCalibrator();
          /** @brief Underlying source link type of the uncalibrated measurement  */
          using SourceLink_t = const xAOD::UncalibratedMeasurement*;
          /** @brief Helper method to pack an uncalibrated measurement to an Acts source link
           *  @param meas: Pointer to the measurement to unpack */
          static Acts::SourceLink pack(const xAOD::UncalibratedMeasurement* meas);
          /** @brief Helper method to unpack an Acts source link to an uncalibrated measurement
           *  @param sl: Reference to the source link pointing to the uncalibrated measurement */
          static const xAOD::UncalibratedMeasurement* unpack(const Acts::SourceLink& sl);
          /** @brief Register a calibrator implementation instance for a given measurement type. The function 
           *         signature must satisfy the requirements of a Calibration delegate defined by the Acts::Fitters
           * @param type: Measurment type for which this instance of the calibrator shall be called
           * @param instance: Pointer to the calibrator instance. */
          template <auto Callable, typename Type>
              void connect(const xAOD::UncalibMeasType type, const Type* instance) {
                assert(instance);
                m_calibrators[static_cast<int>(type)].connect<Callable>(instance);
          }
          /** @brief: Interface method for the Acts fitter to calibrate the trajectory track states from the source link
           *          The source link is unpacked to access the measurement type and then to forward the calibration to the
           *          registered calibrator. If no calibrator has been connected, then the invalidCalib method is called which
           *          is throwing an exception.
           *  @param gctx: Geometry context passed to access the global alignment
           *  @param cctx: Calibration context which is a wrapped Gaudi::EventContext* to retrieve
           *               extra information from store gate
           *  @param sl: Reference to the source link having the calibrated measurement packed
           *  @param trackState: Proxy to the track state into which the calibrated measurement parameters
           *                     are copied. */
          void calibrate(const Acts::GeometryContext& gctx,
                         const Acts::CalibrationContext& cctx,
                         const Acts::SourceLink& sl,
                         const MutableTrackStateBackend::TrackStateProxy trackState) const;
        private:
            /** @brief Delegate method that's assigned during the construction phase of this class.
             *         If called, the method throws an exception reminding the user that he needs to
             *         connect a calibrator with the requested measruement type.
             *  @param gctx: Geometry context passed to access the global alignment
             *  @param cctx: Calibration context which is a wrapped Gaudi::EventContext* to retrieve
             *               extra information from store gate
             *  @param sl: Reference to the source link having the calibrated measurement packed
             *  @param trackState: Proxy to the track state into which the calibrated measurement parameters
             *                     are copied. */
            void invalidCalibrator(const Acts::GeometryContext& gctx,
                                   const Acts::CalibrationContext& cctx,
                                   const Acts::SourceLink& sl,
                                   const MutableTrackStateBackend::TrackStateProxy trackState) const;
            /** @brief Abrivation for the calibrator delegate. The signature of all delegates should
             *         be shared accross all fitters implemented in Acts */
            using CalibDelegate = Acts::Experimental::Gx2FitterExtensions<MutableTrackStateBackend>::Calibrator; 
            constexpr static int s_nMeasTypes = static_cast<int>(xAOD::UncalibMeasType::nTypes);
            /** @brief Dispatch table of the calibrators per measurement type. In the construction phase
             *         of this class all delegates are connected with the `invalidCalibrator` method. */
            std::array<CalibDelegate, s_nMeasTypes> m_calibrators{};


          
    };
}

#endif