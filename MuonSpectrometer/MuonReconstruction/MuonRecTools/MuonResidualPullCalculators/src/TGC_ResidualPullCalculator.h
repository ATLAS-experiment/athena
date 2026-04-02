/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_RESIDUALPULLCALCULATOR_H
#define MUONTGC_RESIDUALPULLCALCULATOR_H

#include "TrkToolInterfaces/IResidualPullCalculator.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkEventPrimitives/ResidualPull.h"

namespace Muon
{

  /** @brief TGC-specific tool to calculate hit residual and pull from a
      RIO_OnTrack/TrackParameter pair.

      Uses Muon-specific info to code the strip type (measuring phi/eta)
      in the 2nd coordinate of the residual and to un-do the rotation from
      strip to chamber system. That is, the residual and pull is calculated in
      the direction perpendicular to the strip allowing e.g. a validation of
      the strip errors.

      @author  Wolfgang Liebig <http://consult.cern.ch/xwho/people/54608>
  */

  class TGC_ResidualPullCalculator final:  public extends<AthAlgTool, Trk::IResidualPullCalculator>
    {
    public:
      using base_class::base_class;
      virtual ~TGC_ResidualPullCalculator()=default;

      virtual StatusCode initialize() override;

      using IResidualPullCalculator::residualPull;
      /** @brief This function returns (creates!) a Trk::ResidualPull
          object, which contains the values of residual and pull for
          the given measurement and track state.

          The track state can be an unbiased one (which can be retrieved
          by the Trk::IUpdator), a biased one (which contains the measurement),
          or a truth state.
          The enum ResidualType must be set according to this, otherwise the pulls will be wrong.
          Residuals differ in all three cases; please be aware of this!!!
     */
    virtual std::optional<Trk::ResidualPull> residualPull(
        const Trk::MeasurementBase* measurement,
        const Trk::TrackParameters* trkPar,
        const Trk::ResidualPull::ResidualType,
        const Trk::TrackState::MeasurementType) const override;

    /** This function is a light-weight version of the function above, designed for track fitters
     * where speed is critical.
     */
    virtual std::array<double,5> residuals(
        const Trk::MeasurementBase* measurement,
        const Trk::TrackParameters* trkPar,
        const Trk::ResidualPull::ResidualType,
        const Trk::TrackState::MeasurementType) const override;

    private:

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

      //! internal structuring: common code for calculating hit pulls
      double calcPull(const double residual,
                      const double locMesCov,
                      const double locTrkCov,
                      const Trk::ResidualPull::ResidualType&) const;

    };
} // end of namespace

#endif
