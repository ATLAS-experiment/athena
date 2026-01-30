/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRACKPARAMSESTIMATIONTOOL_H
#define ACTSTRACKRECONSTRUCTION_TRACKPARAMSESTIMATIONTOOL_H

// ATHENA
#include "ActsToolInterfaces/ITrackParamsEstimationTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsInterop/Logger.h"

// ACTS
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/SympyStepper.hpp"

namespace ActsTrk {
  
  class TrackParamsEstimationTool :
    public extends<AthAlgTool, ActsTrk::ITrackParamsEstimationTool> {
    
  public:
    TrackParamsEstimationTool(const std::string& type, const std::string& name,
			      const IInterface* parent);
    virtual ~TrackParamsEstimationTool() = default;
    
    virtual StatusCode initialize() override;

    virtual
      std::optional<Acts::BoundTrackParameters>
      estimateTrackParameters(
			      const ActsTrk::Seed& seed,
			      bool useTopSp,
			      const Acts::GeometryContext& geoContext,
			      const Acts::MagneticFieldContext& magFieldContext,
			      std::function<const Acts::Surface&(const ActsTrk::Seed& seed, bool useTopSp)> retrieveSurface) const override;

    virtual
      std::optional<Acts::BoundTrackParameters>
      estimateTrackParameters(
			      const ActsTrk::Seed& seed,
			      bool useTopSp,
			      const Acts::GeometryContext& geoContext,
			      const Acts::MagneticFieldContext& magFieldContext,
			      const Acts::Surface& surface,
			      const Acts::Vector3& bField) const override;

    SpacePointIndicesFun_t spacePointIndicesFun() const override;

    // *********************************************************************

  private:
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
        "Inflate tracks"};
    Gaudi::Property< int > m_useLongSeeds {this, "useLongSeeds", 2,
        "0=use 1st 3 SPs, 1=use first,middle,last SPs to improve pT measurement, 2=use for all parameters"};
    Gaudi::Property<int> m_bFieldMode{this, "bFieldMode", 0,
        "B-field mode: 0=B-field at first SP in search order; 1=z-component of B-field; 2=B-field at innermost SP, regardless of search direction"};
    Gaudi::Property<std::size_t> m_firstSp{this, "firstSp", 0ul,
        "Index of first SP to use"};
    Gaudi::Property<bool> m_allowPropagatorFailure{this, "allowPropagatorFailure", false,
        "Use curvilinear parameters when propagation fails instead of returning null"};

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

