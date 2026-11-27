/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PFlowUtils/ActsCaloParametersChecker.h"

#include <cmath>

void checkMultipleActsCaloParameters(const std::vector<Acts::BoundTrackParameters>& caloPars,
                                     CaloSampling::CaloSample sample,
                                     const Acts::GeometryContext& gctx,
                                     MsgStream& msg) {
  if (caloPars.size() <= 1) return;

  const bool debug = msg.level() <= MSG::DEBUG;
  //print eta, phi and details of naming etc
  if (debug) {
    msg << MSG::DEBUG << "Multiple parameters for CaloSample: " << static_cast<int>(sample) << " (" << CaloSampling::getSamplingName(sample)
        << ") with size: " << caloPars.size() << endmsg;
  }
  const double etaNominal = caloPars[0].position(gctx).eta();
  const double phiNominal = caloPars[0].position(gctx).phi();
  //for now we print the difference if larger than smallest possible cell width/2 in the calorimeter,
  //which is 0.025 in both eta and phi
  constexpr double cellWidthReference = 0.025/2.0;
  for (const Acts::BoundTrackParameters& pars : caloPars) {
    const double etaDiff = std::abs(pars.position(gctx).eta() - etaNominal);
    //wrap the phi difference into [-pi, pi] before taking its magnitude
    const double phiDiff = std::abs(std::remainder(pars.position(gctx).phi() - phiNominal, 2.0 * M_PI));
    if (etaDiff > cellWidthReference || phiDiff > cellWidthReference) {
      msg << MSG::WARNING << "Eta, phi difference from nominal: " << etaDiff << ", " << phiDiff << endmsg;
    }
  }
  if (debug) msg << MSG::DEBUG << "Using the first one" << endmsg;
}
