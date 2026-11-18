/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRACKPARAMSESTIMATIONTOOL_H
#define ACTSTRACKRECONSTRUCTION_TRACKPARAMSESTIMATIONTOOL_H

#include "ActsToolInterfaces/ITrackParamsEstimationTool.h"

// ATHENA
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsToolInterfaces/IFitterTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsInterop/Logger.h"
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"

// ACTS
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/SympyStepper.hpp"

#include <utility>
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace ActsTrk {
  
  class TrackParamsEstimationTool :
    public extends<AthAlgTool, ActsTrk::ITrackParamsEstimationTool> {
    
  public:
    TrackParamsEstimationTool(const std::string& type, const std::string& name,
			      const IInterface* parent);
    virtual ~TrackParamsEstimationTool() = default;
    
    virtual StatusCode initialize() override;

    virtual
      std::pair<std::optional<Acts::BoundTrackParameters>, EstimationStatus>
      estimateTrackParameters(
			      const ActsTrk::Seed& seed,
			      std::size_t spIndex,
			      const Acts::GeometryContext& geoContext,
			      const Acts::MagneticFieldContext& magFieldContext,
			      const Acts::CalibrationContext& calContext,
			      RetrieveSurfaceFun_t retrieveSurface) const override;

    virtual
      std::pair<std::optional<Acts::BoundTrackParameters>, EstimationStatus>
      estimateTrackParameters(
			      const ActsTrk::Seed& seed,
			      std::size_t spIndex,
			      const Acts::GeometryContext& geoContext,
			      const Acts::MagneticFieldContext& magFieldContext,
			      const Acts::CalibrationContext& calContext,
			      const Acts::Surface& surface,
			      const Acts::Vector3& bField) const override;

    SpacePointIndicesFun_t spacePointIndicesFun() const override;

    // *********************************************************************

  private:
    ToolHandle<ActsTrk::IFitterTool> m_fitterTool{this, "FitterTool", "", "Fitter Tool for Seeds"};
    ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

    // Properties
    Gaudi::Property< double > m_sigmaLoc0 {this, "sigmaLoc0", 1 * Acts::UnitConstants::mm,
        "Constant term of the loc0 resolution"};
    Gaudi::Property< double > m_sigmaLoc1 {this, "sigmaLoc1", 1 * Acts::UnitConstants::mm,
        "Constant term of the loc1 resolution"};
    Gaudi::Property< double > m_sigmaPhi {this, "sigmaPhi", 0.1 * Acts::UnitConstants::degree,
        "Phi angular resolution"};
    Gaudi::Property< double > m_sigmaTheta {this, "sigmaTheta", 0.1 * Acts::UnitConstants::degree,
        "Theta angular resolution"};
    Gaudi::Property< double > m_sigmaQOverP {this, "sigmaQOverP", 0.1 * Acts::UnitConstants::e / Acts::UnitConstants::GeV,
        "q/p resolution"};
    Gaudi::Property< double > m_sigmaT0 {this, "sigmaT0", 1 * Acts::UnitConstants::ns,
        "Time resolution"};
    Gaudi::Property< double > m_initialSigmaPtRel {this, "initialSigmaPtRel", 0.1,
        "Initial relative pT resolution"};
    Gaudi::Property< std::vector<double> > m_initialVarInflation {this, "initialVarInflation", {1., 1., 1., 1., 1., 1.},
        "Inflate track variances"};
    Gaudi::Property< int > m_parameterEstimationMode {this, "parameterEstimationMode", 2,
        "0=use 1st 3 SPs, 1=use first,middle,last SPs to improve pT measurement, 2=use for all parameters, "
        "3=use 1st 3 SPs separated by more than minDeltaR, starting from the first SP of the estimate, "
        "4=fit all SPs"};
    Gaudi::Property<int> m_bFieldMode{this, "bFieldMode", 0,
        "B-field mode: 0=B-field at the first SP of the estimate; 1=z-component of it; 2=B-field at the innermost SP"};
    Gaudi::Property<std::size_t> m_firstSp{this, "firstSp", 0ul,
        "Index of first SP to use"};
    Gaudi::Property<double> m_minDeltaR{this, "minDeltaR", 15 * Acts::UnitConstants::mm,
        "Minimum difference in distance from the origin between the SPs used for the estimate (parameterEstimationMode=3)"};
    Gaudi::Property<bool> m_allowPropagatorFailure{this, "allowPropagatorFailure", false,
        "Use curvilinear parameters when propagation fails instead of returning null"};
    Gaudi::Property<std::size_t> m_stripCalibrationIterations{this, "stripCalibrationIterations", 1ul,
        "Number of strip calibration iterations"};
    Gaudi::Property<std::size_t> m_geometricRefineIterations{this, "geometricRefineIterations", 0ul,
        "Number of geometric refinement iterations of the circle fit (parameterEstimationMode=4)"};
    Gaudi::Property<double> m_spacePointWeightExponent{this, "spacePointWeightExponent", 0.,
        "Weight each SP in the fit by 1/r^exponent, where 0 gives uniform weights (parameterEstimationMode=4)"};
    Gaudi::Property<bool> m_refitSeeds{this, "refitSeeds", false, "Run KalmanFitter on seeds"};

    std::optional<Acts::BoundTrackParameters> doRefit(
        const ActsTrk::Seed &measurement,
        const Acts::BoundTrackParameters &initialParameters,
        const Acts::GeometryContext& geometry,
        const Acts::MagneticFieldContext& magField,
        const Acts::CalibrationContext& calib,
        std::size_t spIndex) const;

    /// The estimate at the last SP uses reverse SP order; the refit starts at the first SP
    bool reverseOrder(std::size_t nSp, std::size_t spIndex) const {
      return !m_refitSeeds && spIndex > 0 && spIndex + 1 == nSp;
    }

    /// An inward search starts at the outer measurement of its SP
    static std::size_t measurementIndex(const xAOD::SpacePoint& sp, std::size_t spIndex) {
      return spIndex > 0 ? sp.measurements().size() - 1 : 0;
    }

    detail::xAODUncalibMeasSurfAcc m_uncalibMeasSurfAcc {};

    using Stepper = Acts::SympyStepper;
    using Navigator = Acts::VoidNavigator;
    using Extrapolator = Acts::Propagator<Stepper>;

    std::optional<Extrapolator> m_extrapolator;

    /// Private access to the logger
    const Acts::Logger &logger() const
    {
      return *m_logger;
    }

    /// logging instance
    std::unique_ptr<const Acts::Logger> m_logger;

    SpacePointIndicesFun_t m_spacePointIndicesFun{};
  };
  
} // namespace

#endif

