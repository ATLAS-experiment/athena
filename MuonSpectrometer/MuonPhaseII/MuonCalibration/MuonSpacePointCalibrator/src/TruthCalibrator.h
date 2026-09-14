/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSPACEPOINTCALIBRATOR_TRUTHCALIBRATOR_H
#define MUONSPACEPOINTCALIBRATOR_TRUTHCALIBRATOR_H

#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"

#include "AthenaBaseComps/AthAlgTool.h"

#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "ActsEvent/ContextUtility.h"
#include "ActsCalibBase/MeasurementCalibratorBase.h"


namespace MuonR4 {
    /** @brief The TruthCalibrator calibrates the measurements according to their available truth information.
     *         If a truth hit is associated to the space point, the truth parameters are taken to calibrate 
     *         the measurement. Otherwise the measurement is set exactly onto the predicted position eliminating
     *         its contribution to the chi2 and respective derivatives.
     */
    class TruthCalibrator : public extends<AthAlgTool, ISpacePointCalibrator>,
                            public ActsTrk::detail::MeasurementCalibratorBase {
        public:
            using base_class::base_class;
            /** @copydoc AthAlgTool::initialize */
            virtual StatusCode initialize() override final;
            /** @copydoc ISpacePointCalibrator::calibrate  */
            virtual CalibSpacePointPtr calibrate(const EventContext& ctx,
                                                 const SpacePoint* spacePoint,
                                                 const Amg::Vector3D& seedPosInChamb,
                                                 const Amg::Vector3D& seedDirInChamb,
                                                 const double timeDelay) const override final;
            /** @copydoc ISpacePointCalibrator::calibrate  */
            virtual CalibSpacePointPtr calibrate(const EventContext& ctx,
                                                 const CalibratedSpacePoint& spacePoint,
                                                 const Amg::Vector3D& seedPosInChamb,
                                                 const Amg::Vector3D& seedDirInChamb,
                                                 const double timeDelay) const override final;
            /** @copydoc ISpacePointCalibrator::calibrate  */
            virtual CalibSpacePointVec calibrate(const EventContext& ctx,
                                                 const std::vector<const SpacePoint*>& spacePoints,
                                                 const Amg::Vector3D& seedPosInChamb,
                                                 const Amg::Vector3D& seedDirInChamb,
                                                 const double timeDelay) const override final;
    
            /** @copydoc ISpacePointCalibrator::calibrate  */
            virtual CalibSpacePointVec calibrate(const Acts::CalibrationContext& cctx,                                            
                                                  const Amg::Vector3D& seedPosInChamb,
                                                  const Amg::Vector3D& seedDirInChamb,
                                                  const double timeDelay,
                                                  const CalibSpacePointVec& spacePoints) const override final;

            /** @copydoc ISpacePointCalibrator::driftVelocity */
            virtual double driftVelocity(const Acts::CalibrationContext& cctx,
                                         const CalibratedSpacePoint& spacePoint) const override final;
            /** @copydoc ISpacePointCalibrator::calibrateSourceLink */
            virtual void calibrateSourceLink(const Acts::GeometryContext& geoctx,
                                             const Acts::CalibrationContext& cctx,
                                             const Acts::SourceLink& link,
                                             ActsTrk::MutableTrackStateBackend::TrackStateProxy state) const override final;
            /** @copydoc ISpacePointCalibrator::updateSigns */
            virtual void updateSigns(const Amg::Vector3D& trackPos,
                                     const Amg::Vector3D& trackDir,
                                     CalibSpacePointVec& hitsToCalib) const override final;
            /** @copydoc ISpacePointCalibrator::stampSignsOnMeasurements */
            virtual void stampSignsOnMeasurements(const xAOD::MuonSegment& segment) const override final;
            /** @copydoc ISpacePointCalibrator::driftRadius */
            virtual double driftRadius(const Acts::CalibrationContext& cctx,
                                       const CalibratedSpacePoint& spacePoint, 
                                       const double timeDelay) const override final;
            /** @copydoc ISpacePointCalibrator::driftVelocity */
            virtual double driftVelocity(const Acts::CalibrationContext& cctx,
                                         const CalibratedSpacePoint& spacePoint, 
                                         const double timeDelay) const override final;
            /** @copydoc ISpacePointCalibrator::driftAcceleration */
            virtual double driftAcceleration(const Acts::CalibrationContext& cctx,
                                             const CalibratedSpacePoint& spacePoint, 
                                             const double timeDelay) const override final;
            /** @copydoc ISpacePointCalibrator::driftAcceleration */
            virtual double driftAcceleration(const Acts::CalibrationContext& cctx,
                                             const CalibratedSpacePoint& spacePoint) const override final;
        private:
            /** @brief Context utility object to retrieve the geometry context */
            ActsTrk::ContextUtility m_ctxProvider{this};
            /** @brief Dependencies on the truth matching of the uncalibrated measurement containers */
            SG::ReadDecorHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_truthLinks{this, "truthLinks", {}};
            /** @brief Container names of the muon prd containers */
            Gaudi::Property<std::vector<std::string>> m_prdContainers{this, "prdContainers", {}};
            /** @brief Decoration of the truth link */
            Gaudi::Property<std::string> m_simLinkDecor{this, "simHitLinkDecor", "simHitLink"};


    };
}

#endif