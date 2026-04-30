/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RPC_ResidualPullCalculator.h"

#include "TrkEventUtils/IdentifierExtractor.h"
#include "TrkEventPrimitives/LocalParameters.h"

//================ Initialisation =================================================

StatusCode Muon::RPC_ResidualPullCalculator::initialize()
{
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_MSG_DEBUG ("initialize() successful in " << name());
  return StatusCode::SUCCESS;
}

//================ calculate residuals for RPC ==================================
std::array<double,5>
Muon::RPC_ResidualPullCalculator::residuals(
    const Trk::MeasurementBase* measurement,
    const Trk::TrackParameters* trkPar,
    const Trk::ResidualPull::ResidualType /*resType*/,
    const Trk::TrackState::MeasurementType) const {
  std::array<double, 5> residuals{};
  if (!trkPar || !measurement) return residuals;
  Identifier ID = Trk::IdentifierExtractor::extract(measurement);

  if( m_idHelperSvc->isRpc(ID) ) {

    if (measurement->localParameters().parameterKey() == 1) {
      // convention to be interpreted by TrkValTools: 2nd coordinate codes orientation of RPC
        residuals[Trk::loc1] = measurement->localParameters()[Trk::loc1]
          - trkPar->parameters()[Trk::loc1];
    } else {
      residuals[Trk::loc1] = measurement->localParameters()[Trk::loc1]
          - trkPar->parameters()[Trk::loc1];
      residuals[Trk::loc2] = measurement->localParameters()[Trk::loc2]
          - trkPar->parameters()[Trk::loc2];
    }

  } else {
    ATH_MSG_WARNING( "Input problem measurement is not RPC. "
                    <<m_idHelperSvc->toString(ID) );
    return residuals;
  }
  return residuals;
}

//================ calculate residuals and pulls for RPC ==================================
std::optional<Trk::ResidualPull> Muon::RPC_ResidualPullCalculator::residualPull(
    const Trk::MeasurementBase* measurement,
    const Trk::TrackParameters* trkPar,
    const Trk::ResidualPull::ResidualType resType,
    const Trk::TrackState::MeasurementType) const {

  if (!trkPar || !measurement) {
    return std::nullopt;
  }
  Identifier ID = Trk::IdentifierExtractor::extract(measurement);

  if (!m_idHelperSvc->isRpc(ID)) {
    ATH_MSG_DEBUG ("Input problem measurement is not RPC but "
                  <<m_idHelperSvc->toString(ID));
    return std::nullopt; 
  }
 
  
  // if no covariance for the track parameters is given the pull calculation is not valid
  const AmgSymMatrix(5)* trkCov = trkPar->covariance();
 
  // calculate residual
  const auto & localParameters = measurement->localParameters();
  const std::size_t nParams = localParameters.dimension();
  std::vector<double> residual(nParams), pull(nParams);
    
  switch (nParams) {
    case 1: {
      residual[Trk::loc1] = localParameters[Trk::loc1]
                          - trkPar->parameters()[Trk::loc1];
      break;
    } case 2: {
      residual[Trk::loc1] = localParameters[Trk::loc1]
                          - trkPar->parameters()[Trk::loc1];
      residual[Trk::loc2] = localParameters[Trk::loc2]
                          - trkPar->parameters()[Trk::loc2];
      break;
    } default:
      ATH_MSG_WARNING ( "RPC ClusterOnTrack does not carry the expected "
                      << "LocalParameters structure!" );
      return std::nullopt;
    }
    
    // calculate pull
    for (std::size_t l = 0 ; l < residual.size(); ++l) {
        pull[l] = calcPull(residual[l], measurement->localCovariance()(l,l),
                            trkCov ? (*trkCov)(l,l) : 0., resType);
    }
 
    // create the Trk::ResidualPull.
    ATH_MSG_DEBUG ( "Calculating Pull for channel " << m_idHelperSvc->toString(ID) << " residual " << residual[Trk::loc1] << " pull " << pull[Trk::loc1] );
    return std::make_optional<Trk::ResidualPull>(std::move(residual), 
                                                 std::move(pull), 
                                                 trkCov != nullptr, 
                                                 resType, 1);
}


/////////////////////////////////////////////////////////////////////////////
/// calc pull in 1 dimension
/////////////////////////////////////////////////////////////////////////////
double Muon::RPC_ResidualPullCalculator::calcPull(
    const double residual,
    const double locMesCov,
    const double locTrkCov,
    const Trk::ResidualPull::ResidualType& resType ) {

    double ErrorSum(0.0);
    if (resType == Trk::ResidualPull::Unbiased) {
      if( locMesCov + locTrkCov > 0 ) ErrorSum = std::sqrt(locMesCov + locTrkCov);
    } else if (resType == Trk::ResidualPull::Biased) {
      if ((locMesCov - locTrkCov) < 0.) {
            return 0;
        }
        ErrorSum = std::sqrt(locMesCov - locTrkCov);
    } else ErrorSum = std::sqrt(locMesCov);
    if (ErrorSum != 0) return residual/ErrorSum;
    return 0;
}
