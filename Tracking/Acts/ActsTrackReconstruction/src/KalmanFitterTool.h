/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_KALMANFITTERTOOL_H
#define ACTSTRACKRECONSTRUCTION_KALMANFITTERTOOL_H

#include "src/detail/FitterHelperFunctions.h"
#include "AthenaBaseComps/AthAlgTool.h"

// ACTS
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/TrackFitting/KalmanFitter.hpp"
#include "Acts/Propagator/SympyStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"

// PACKAGE
#include "ActsEvent/TrackContainer.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/IGeometryRealmConvTool.h"

#include "ActsCalibrators/TrkMeasSurfaceAccessor.h"
#include "ActsCalibrators/TrkPrepRawDataCalibrator.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "ActsCalibrators/TrkPrepRawDataSurfaceAcc.h"
#include "src/detail/OnTrackCalibrator.h"

#include "ActsToolInterfaces/IFitterTool.h"


#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"

class EventContext;

namespace Trk{
  class Track;
  class PrepRawData;
}

namespace ActsTrk {

class KalmanFitterTool
  : public extends<AthAlgTool, ActsTrk::IFitterTool> { 
public:

  using base_class::base_class;
  virtual ~KalmanFitterTool() = default;

  // standard Athena methods
  virtual StatusCode initialize() override;
  
  //! fit a set of xAOD uncalibrated Measurements
  virtual  
      std::unique_ptr< ActsTrk::MutableTrackContainer >
      fit(const std::vector<const xAOD::UncalibratedMeasurement*> & clusterList,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext,      
      const Acts::Surface* targetSurface = nullptr  // optional target surface - defaults to perigee in global origin
      ) const override;

  //! Acts seed fit
  virtual
    std::unique_ptr< ActsTrk::MutableTrackContainer >
    fit(const ActsTrk::Seed &seed,
        const Acts::BoundTrackParameters& initialParams,
        const Acts::GeometryContext& tgContext,
        const Acts::MagneticFieldContext& mfContext,
        const Acts::CalibrationContext& calContext,
	const Acts::Surface& targetSurface) const override;

    virtual StatusCode fit(
        const EventContext& ctx,
	const ActsTrk::TrackContainer::ConstTrackProxy& track,          
        ActsTrk::MutableTrackContainer& trackContainer,
	const Acts::PerigeeSurface& pSurface) const override;

    //! Fit a set of source links
  virtual
    std::unique_ptr< ActsTrk::MutableTrackContainer >
    fit(const std::vector<Acts::SourceLink>& sourceLinks,
        const Acts::BoundTrackParameters& initialParams,
        const Acts::GeometryContext& tgContext,
        const Acts::MagneticFieldContext& mfContext,
        const Acts::CalibrationContext& calContext,
        const Acts::Surface* targetSurface = nullptr) const override;
  
  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
private:
    /** @brief Abrivate the track state proxy */
    using TrackState_t = MutableTrackStateBackend::TrackStateProxy;
    /** @brief Abbrivation of the fitter extensions */
    using FitterExtension_t = Acts::KalmanFitterExtensions<MutableTrackStateBackend>;
    /** @brief Abbrivation of the configuration to launch the fit  */
    using FitterOptions_t = Acts::KalmanFitterOptions<MutableTrackStateBackend>;
    /** @brief Helper method to pack the last information (Calibration, Alignment, B-Field, etc.)
     *         for the fit. The parsed context objects need to prevail the call of the fit
     * @param tgContext: Reference to the geometry context
     * @param mfContext: Reference to the passed magnetic field context
     * @param calContext: Reference to the calibration context
     * @param surface: Target surface on which the fit result is expressed.
     * @param slType: Switch of which source link type shall be used to access the surfaces from
     *                the measurements & for calibration */
    FitterOptions_t configureFit(const Acts::GeometryContext& tgContext,
                                 const Acts::MagneticFieldContext& mfContext,
                                 const Acts::CalibrationContext& calContext,
                                 const Acts::Surface* surface,
                                 detail::SourceLinkType slType) const;

  ServiceHandle<ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

  PublicToolHandle<IGeometryRealmConvTool> m_geometryConvTool{this, "GeometryRealmConvTool", ""};

  ToolHandle<MuonR4::ISpacePointCalibrator> m_muonCalibrator{this, "MuonCalibrationTool", ""};

  ToolHandle<Trk::IRIO_OnTrackCreator> m_ROTcreator {this, "RotCreatorTool", ""};

  // the settable job options
  Gaudi::Property< double > m_option_outlierChi2Cut {this, "OutlierChi2Cut", 12.5, 
      "Chi2 cut used by the outlier finder" };
  Gaudi::Property< double > m_option_ReverseFilteringPt {this, "ReverseFilteringPt", 1.0 * Acts::UnitConstants::GeV,
      "Pt cut used for the ReverseFiltering logic"};
  Gaudi::Property< int > m_option_maxPropagationStep {this, "MaxPropagationStep", 5000, 
      "Maximum number of steps for one propagate call"};
  Gaudi::Property<bool> m_useDirectNavigation{this, "UseDirectNavigation", true,
      "GSF with direct navigation when refitting measurements"};
  /** @brief Stop the propagation as soon as the envelope is left (Gen3 only) */
  Gaudi::Property<std::uint32_t> m_envelopeConstraint{this, "EnvelopeConstaint",
                                                      Acts::toUnderlying(ActsTrk::SystemEnvelope::ITkExit)};
  /** @brief Calibrator for the Trk::MeasurementBase track states (legacy EDM) */
  detail::TrkMeasurementCalibrator m_trkCalibrator{};
  /** @brief Accessor to fetch surfaces from the Trk::MeasurementBase track states (legacy EDM) */
  detail::TrkMeasSurfaceAccessor m_trkSurfAcc{};
  /** @brief Calibrator for the Trk::PrepRawData track states (legacy EDM) */
  detail::TrkPrepRawDataCalibrator m_prdCalibrator{};
  /** @brief Surface accessor for the Trk::PrepRawData track states (legacy EDM) */
  detail::TrkPrepRawDataSurfaceAcc m_prdSurfAcc{};
  /** @brief Accessor to fetch surfaces from the xAOD::UncalibratedMeasurements (Phase-II EDM) */
  detail::xAODUncalibMeasSurfAcc m_unalibMeasSurfAcc{};
   /** @brief Calibrator for the uncalibrated xAOD::UnCalibratedMeasurement objects */
  detail::xAODUncalibMeasCalibrator m_uncalibMeasCalibrator{};
  /** @brief Calibrator of the ID / ITk measurements */
  using xAODItkCalibrator_t = detail::OnTrackCalibrator<ActsTrk::MutableTrackStateBackend> ;
  xAODItkCalibrator_t m_idCalibrator{};


  /// Type erased track fitter function.
  using Fitter = Acts::KalmanFitter<Acts::Propagator<Acts::SympyStepper, Acts::Navigator>, ActsTrk::MutableTrackStateBackend>;
  std::unique_ptr<Fitter> m_fitter {nullptr};

  using DirectFitter = Acts::KalmanFitter<Acts::Propagator<Acts::SympyStepper, Acts::DirectNavigator>, ActsTrk::MutableTrackStateBackend>;
  std::unique_ptr<DirectFitter> m_directFitter {nullptr};

  /** @brief Array of all configured fitter extensions depending on which source link type is in use */
  static constexpr unsigned s_nExtensions = static_cast<unsigned>(detail::SourceLinkType::nTypes);
  std::array<FitterExtension_t, s_nExtensions>  m_kfExtensions{};

  ActsTrk::detail::FitterHelperFunctions::ATLASOutlierFinder m_outlierFinder{0};
  ActsTrk::detail::FitterHelperFunctions::ReverseFilteringLogic m_reverseFilteringLogic{0};

  /// Private access to the logger
  const Acts::Logger& logger() const {
    return *m_logger;
  }

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger;

  std::uint32_t m_endOfWorldId{std::numeric_limits<std::uint32_t>::max()};

  

}; // end of namespace

}
#endif

