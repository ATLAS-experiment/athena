/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSCALIBRATION_DETAIL_PREPRAWDATACALIBRATOR_H
#define ACTSCALIBRATION_DETAIL_PREPRAWDATACALIBRATOR_H

#include "ActsCalibrators/TrkMeasurementCalibrator.h"
#include "ActsGeometryInterfaces/IGeometryRealmConvTool.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"

#include <memory>
namespace ActsTrk::detail {
   /** @brief Class to calibrate the Acts track states with uncalibrated Trk::PrepRaw data objects.
    *         Essentially, this class is reproducing the calibration work flow during the ATLAS fit. */
   class TrkPrepRawDataCalibrator : public MeasurementCalibratorBase {
      public:

         /** @brief Empty constructor not configuring any tool -> crash if not later overwritten */
         TrkPrepRawDataCalibrator() = default;
         /** @brief Constructor taking the Acts <-> Trk conversion tool & 
          *         a preconfigured rot creator to calibrate the measurements
          *  @param convTool: Pointer to the configured track conversion tool
          *  @param rotCreator: Pointer to the configured ROT creator */
         TrkPrepRawDataCalibrator(const ActsTrk::IGeometryRealmConvTool* convTool,
                                  const Trk::IRIO_OnTrackCreator* rotCreator);
         /** @brief Calibrator delegate implementation to calibrate the ActsTrk fit from Trk::PrepRawData objects
          *  @tparam trajectory_t: Tepmlate parameter of the underlying MultTrajectory container backend
          *  @param gctx: Geometry context to access the alignment of the surface
          *  @param cctx: Calibration context to access the calibration constants from the conditions store
          *  @param sl: Reference to the packed SourceLink (a.k.a Trk::MeasurementBase)
          *  @param trackState: Reference to the multi trajectory track state to fill */
         template <Acts::TrackStateProxyConcept proxy_t>
         void calibrate(const Acts::GeometryContext &gctx,
                        const Acts::CalibrationContext & cctx,
                        const Acts::SourceLink& sl,
                        proxy_t trackState) const;
         /** @brief Create a Track Raw Input object (ROT) from the source link
          *         to the PRD measurement and the predicted track parameters of the track state.
          *  @tparam trajectory_t: Tepmlate parameter of the underlying MultTrajectory container backend
          *  @param gctx: Geometry context to access the alignment of the surface
          *  @param cctx: Calibration context to access the calibration constants from the conditions store
          *  @param sl: Reference to the packed SourceLink (a.k.a Trk::PrepRawData)
          *  @param trackState: Reference to the multi trajectory track state to read
          *                     the predicted parameters from.  */
         template <Acts::TrackStateProxyConcept proxy_t>
            std::unique_ptr<Trk::RIO_OnTrack> createROT(const Acts::GeometryContext &gctx,
                                                        const Acts::CalibrationContext & cctx,
                                                        const Acts::SourceLink& sl,
                                                        const proxy_t& trackState) const;
         /** @brief Unpack the prepraw data measurement from the source link
          *  @param sl: Reference to the source link to unpack */
         static const Trk::PrepRawData* unpack(const Acts::SourceLink& sl);
      private:
         TrkMeasurementCalibrator m_rotCalib{};
         /** @brief Pointer to the track conversion tool */
         const ActsTrk::IGeometryRealmConvTool* m_convTool{nullptr};
         /** @brief ROT creator */
         const Trk::IRIO_OnTrackCreator* m_rotCreator{nullptr};
   };
}
#include "ActsCalibrators/TrkPrepRawDataCalibrator.icc"
#endif