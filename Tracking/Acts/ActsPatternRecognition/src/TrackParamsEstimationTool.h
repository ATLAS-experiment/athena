/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRACKPARAMSESTIMATIONTOOL_H
#define ACTSTRACKRECONSTRUCTION_TRACKPARAMSESTIMATIONTOOL_H

// ATHENA
#include "ActsToolInterfaces/ITrackParamsEstimationTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsInterop/Logger.h"

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
      estimateTrackParameters(const EventContext& ctx,
			      const ActsTrk::Seed& seed,
			      const Acts::GeometryContext& geoContext,
			      const Acts::MagneticFieldContext& magFieldContext,
			      std::function<const Acts::Surface&(const ActsTrk::Seed&)> retrieveSurface,
			      bool useTopSp) const override;

    virtual
      std::optional<Acts::BoundTrackParameters>
      estimateTrackParameters(const EventContext& ctx,
			      const ActsTrk::Seed& seed,
			      const Acts::GeometryContext& geoContext,
			      const Acts::Surface& surface,
			      const Acts::Vector3& bField,
			      bool useTopSp) const override;

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
  };
  
} // namespace

#endif

