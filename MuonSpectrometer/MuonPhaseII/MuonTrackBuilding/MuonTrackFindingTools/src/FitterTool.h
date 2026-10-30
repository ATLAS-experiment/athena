/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTOOLS_FITTERTOOL_H
#define MUONTRACKFINDINGTOOLS_FITTERTOOL_H


#include "GeoPrimitives/GeoPrimitives.h"
///

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IFitterTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"

#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"


// ACTS
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/StraightLineStepper.hpp"
#include "Acts/Propagator/EigenStepper.hpp"


////
#include "MuonTrackFindingTools/Chi2Fitter.h"

namespace MuonR4 {
    class FitterTool : public extends<AthAlgTool, ActsTrk::IFitterTool> {
        public:
            using base_class::base_class;
            virtual StatusCode initialize() override final;
            /** @copydoc ActsTrk::IFitterTool::fit */
            virtual std::unique_ptr<ActsTrk::MutableTrackContainer> 
                    fit(const ActsTrk::Seed &seed,
                        const Acts::BoundTrackParameters& initialParams,
                        const Acts::GeometryContext& tgContext,
                        const Acts::MagneticFieldContext& mfContext,
                        const Acts::CalibrationContext& calContext,
                        const Acts::Surface& targetSurface) const override final;

             /** @copydoc ActsTrk::IFitterTool::fit */
            virtual std::unique_ptr<ActsTrk::MutableTrackContainer > 
                    fit(const std::vector<const xAOD::UncalibratedMeasurement* > & measList,
                        const Acts::BoundTrackParameters& initialParams,
                        const Acts::GeometryContext& tgContext,
                        const Acts::MagneticFieldContext& mfContext,
                        const Acts::CalibrationContext& calContext,
                        const Acts::Surface* targetSurface = nullptr) const override final;
            /** @copydoc ActsTrk::IFitterTool::fit */
            virtual std::unique_ptr<ActsTrk::MutableTrackContainer > 
                    fit(const std::vector<Acts::SourceLink>& sourceLinks,
                        const Acts::BoundTrackParameters& initialParams,
                        const Acts::GeometryContext& tgContext,
                        const Acts::MagneticFieldContext& mfContext,
                        const Acts::CalibrationContext& calContext,
                        const Acts::Surface* targetSurface = nullptr) const override final;
            /** @copydoc ActsTrk::IFitterTool::fit */
            virtual StatusCode fit(const EventContext& ctx,
                                   const ActsTrk::TrackContainer::ConstTrackProxy& track,          
                                   ActsTrk::MutableTrackContainer& trackContainer,
	                               const Acts::PerigeeSurface& pSurface) const override final;
            
            /** @brief Abrivation for the straight line propagator */
            using StraightPropagator_t = Acts::Propagator<Acts::StraightLineStepper, Acts::Navigator>;
            /** @brief Abrivation for the curved standard propagator */
            using CurvedPropagator_t   = Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;
            /** @brief Abrivation of the fitter using the straight line propagator  */
            using StraightFitter_t = MuonExperimental::Gx2Fitter<StraightPropagator_t, ActsTrk::MutableTrackStateBackend>;
            /** @brief Abrivation of the fitter using the curved line propagator */
            using CurvedFitter_t   = MuonExperimental::Gx2Fitter<CurvedPropagator_t, ActsTrk::MutableTrackStateBackend>;

            /** @brief Abbrivation of the configuration to launch the fit  */
            using Gx2FitterOptions_t = MuonExperimental::Gx2FitterOptions<ActsTrk::MutableTrackStateBackend>;
            /** @brief Abbrivation of the fitter extensions */
            using Gx2FitterExtension_t = MuonExperimental::Gx2FitterExtensions<ActsTrk::MutableTrackStateBackend>;
        private:
            /** @brief Do the straight line fit  */
            Gaudi::Property<bool> m_doStraightLineFit{this, "doStraightLine", false};
            /** @brief How ,many steps to go */
            Gaudi::Property<unsigned> m_doMaxSteps{this, "maxSteps", 1000000 };
            /** @brief How many surfaces to try */
            Gaudi::Property<unsigned> m_maxSurfaces{this, "maxSurfaces", 10000};
            /** @brief How many iterations to use in the fit */
            Gaudi::Property<unsigned> m_maxIter{this, "nIterMax" , 250};
            /** @brief Do energy loss modelling in the fit */
            Gaudi::Property<bool> m_doEnergyLoss{this, "doEnergyLoss" , false};
            /** @brief Increase the nuissances by introducing scattering angles */
            Gaudi::Property<bool> m_doScattering{this, "doScattering" , false};
            /** @brief Tracking geometry service */
            ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", 
                                                                                "ActsTrackingGeometrySvc"};
            /** @brief Tool handle to calibrate the muon measurmeents */
            ToolHandle<MuonR4::ISpacePointCalibrator> m_muonCalibrator{this, "MuonCalibrationTool", ""};
            
            /** @brief Surface accessor delegate for xAOD::UncalibratedMeasurement objects */
            ActsTrk::detail::xAODUncalibMeasSurfAcc m_unalibMeasSurfAcc{};
            /** @brief Calibrator for the uncalibrated xAOD::UnCalibratedMeasurement objects */
            ActsTrk::detail::xAODUncalibMeasCalibrator m_uncalibMeasCalibrator{};

            Gx2FitterExtension_t m_extensions{};
            
            using Fitter_t = std::variant<std::monostate,
                                         StraightFitter_t, 
                                         CurvedFitter_t>;
            Fitter_t m_fitter{};




    };
}
#endif