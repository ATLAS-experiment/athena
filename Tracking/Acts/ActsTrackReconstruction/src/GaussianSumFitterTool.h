/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_GAUSSIANSUMFITTERTOOL_H
#define ACTSTRACKRECONSTRUCTION_GAUSSIANSUMFITTERTOOL_H

// ATHENA 
#include "AthenaBaseComps/AthAlgTool.h"

#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
// ACTS
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/TrackFitting/GaussianSumFitter.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/MultiEigenStepperLoop.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/TrackFitting/GsfOptions.hpp"
#include "Acts/Utilities/Logger.hpp"

// PACKAGE
#include "ActsEvent/TrackContainer.h"

#include "ActsEvent/TrackParametersContainer.h"
#include "ActsEvent/ContextUtility.h"


#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometryInterfaces/IGeometryRealmConvTool.h"
#include "ActsToolInterfaces/IFitterTool.h"
#include "src/detail/FitterHelperFunctions.h"

#include "ActsCalibrators/TrkMeasurementCalibrator.h"
#include "ActsCalibrators/TrkMeasSurfaceAccessor.h"
#include "ActsCalibrators/TrkPrepRawDataCalibrator.h"
#include "ActsCalibrators/TrkPrepRawDataSurfaceAcc.h"
#include "src/detail/OnTrackCalibrator.h"
#include "src/detail/RefittingCalibrator.h"

namespace ActsTrk {

class GaussianSumFitterTool
  : public extends<AthAlgTool, ActsTrk::IFitterTool> {
public:
  
  using base_class::base_class;
  virtual ~GaussianSumFitterTool() = default;

  // standard Athena methods
  virtual StatusCode initialize() override;

  //! Acts seed fit
  virtual
    std::unique_ptr< ActsTrk::MutableTrackContainer >
    fit(const ActsTrk::Seed &seed,
        const Acts::BoundTrackParameters& initialParams,
        const Acts::GeometryContext& tgContext,
        const Acts::MagneticFieldContext& mfContext,
        const Acts::CalibrationContext& calContext,
	const Acts::Surface& targetSurface) const override;

  virtual
  std::unique_ptr< ActsTrk::MutableTrackContainer >
  fit(const std::vector< const xAOD::UncalibratedMeasurement*> & clusterList,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext,      
      const Acts::Surface* targetSurface) const override;
  
  virtual StatusCode fit(
    const EventContext& ctx,
    const ActsTrk::TrackContainer::ConstTrackProxy& track,          
    ActsTrk::MutableTrackContainer& trackContainer,
    const Acts::PerigeeSurface& pSurface) const override;

  //! fit a set of source links
  virtual std::unique_ptr<MutableTrackContainer> fit(
      const std::vector<Acts::SourceLink>& sourceLinks,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext,
      const Acts::Surface* targetSurface =
          nullptr  // optional target surface - defaults to perigee in global
                   // origin
  ) const override;

  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
private:
  using FitterOptions_t = Acts::GsfOptions<ActsTrk::MutableTrackStateBackend>;
  FitterOptions_t configureFit(const Acts::GeometryContext& tgContext,
                               const Acts::MagneticFieldContext& mfContext,
                               const Acts::CalibrationContext& calContext,
                               const Acts::PerigeeSurface& surface,
                               detail::SourceLinkType slType) const;
  /// Private access to the logger
  const Acts::Logger& logger() const;

 private:
  /** @brief Abrivate the track state proxy */
  using TrackState_t = MutableTrackStateBackend::TrackStateProxy;
  PublicToolHandle<ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
  PublicToolHandle<IGeometryRealmConvTool> m_geometryConvTool{this, "GeometryRealmConvTool", ""};
  /** @brief Utility to fetch the geometry, magnetic field and calibration context in the event */
  ContextUtility m_ctxProvider{this};

  ToolHandle<Trk::IRIO_OnTrackCreator> m_ROTcreator{this, "RotCreatorTool", ""};
  // the settable job options
  Gaudi::Property< double > m_option_outlierChi2Cut {this, "OutlierChi2Cut", 12.5, 
      "Chi2 cut used by the outlier finder" };
  Gaudi::Property< int > m_option_maxPropagationStep {this, "MaxPropagationStep", 5000, 
      "Maximum number of steps for one propagate call"};

  Gaudi::Property< int > m_maxComponents {this, "MaxComponents", 12,
      "Maximum number of components in GSF"};

  Gaudi::Property<bool> m_useDirectNavigation{this, "UseDirectNavigation", false,
                "GSF with direct navigation when refitting measurements"};

  Gaudi::Property< double > m_weightCutOff {this, "WeightCutOff", 1.e-4,
              "component weight cut off"};

  Gaudi::Property<std::string> m_option_componentMergeMethod{this, "ComponentMergeMethod", "MaxWeight"
                  , "method to merge components {Mean, MaxWeight}"};

  Acts::ComponentMergeMethod m_componentMergeMethod;

  /** @brief Calibrator for the Trk::MeasurementBase track states (legacy EDM) */
  detail::TrkMeasurementCalibrator m_trkCalibrator {};
  /** @brief Calibrator for the Trk::PrepRawData track states (legacy EDM) */
  detail::TrkPrepRawDataCalibrator m_prdCalibrator{};
  /** @brief Calibrator of the uncalibrated measurements */
  using xAODUnCalibrator_t = detail::OnTrackCalibrator<MutableTrackStateBackend> ;
  xAODUnCalibrator_t m_uncalibMeasCalibrator{};
  std::unique_ptr<detail::RefittingCalibrator> m_refitCalibrator{nullptr};

  /// Type erased track fitter function.
  using Fitter = Acts::GaussianSumFitter< Acts::Propagator<Acts::MultiEigenStepperLoop<>, Acts::Navigator>,
                                                        ActsTrk::MutableTrackStateBackend >;
  std::unique_ptr<Fitter> m_fitter {nullptr};

  using DirectFitter = Acts::GaussianSumFitter< Acts::Propagator<Acts::MultiEigenStepperLoop<>, Acts::DirectNavigator>,
            ActsTrk::MutableTrackStateBackend >;
  std::unique_ptr<DirectFitter> m_directFitter {nullptr};

  using FitterExtension_t = Acts::GsfExtensions<ActsTrk::MutableTrackStateBackend>;
  static constexpr unsigned s_nExtensions = static_cast<unsigned>(detail::SourceLinkType::nTypes);
  std::array<FitterExtension_t, s_nExtensions>  m_gsfExtensions{};

  /** @brief Accessor to fetch surfaces from the Trk::MeasurementBase track states (legacy EDM) */
  detail::TrkMeasSurfaceAccessor m_trkSurfAcc{};
  /** @brief Surface accessor for the Trk::PrepRawData track states (legacy EDM) */
  detail::TrkPrepRawDataSurfaceAcc m_prdSurfAcc{};
  /** @brief Accessor to fetch surfaces from the xAOD::UncalibratedMeasurements (Phase-II EDM) */
  detail::xAODUncalibMeasSurfAcc m_unalibMeasSurfAcc{};

  ActsTrk::detail::FitterHelperFunctions::ATLASOutlierFinder m_outlierFinder{0};

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger {nullptr};

}; // end of namespace

}

#endif

