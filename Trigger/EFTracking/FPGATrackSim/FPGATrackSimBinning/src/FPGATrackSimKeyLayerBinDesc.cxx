
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimBinUtil.cxx
 * @author Elliot Lipeles
 * @date Feb 13, 2025
 * @brief See header file.
 */

#include "FPGATrackSimKeyLayerBinDesc.h"
#include "FPGATrackSimBinning/FPGATrackSimBinStep.h"



bool FPGATrackSimKeyLayerBinDesc::hitInBin(const FPGATrackSimBinStep &step,
                                           const IdxSet &idx,
                                           StoredHit &storedhit) const
{
    double r1 = m_keylyrtool.R1();
    double r2 = m_keylyrtool.R2();
    double hitr= storedhit.hitptr->getR();

    bool passesPhi = true;
    bool passesEta = true;

    if (stepIsRPhi(step)) {
        // distance of hit from bin center
        storedhit.phiShift = phiResidual(step.binCenter(idx),storedhit.hitptr.get());

        // Get expected curvature shift from bin center    
        auto half_xm_bin_pars = parSetToKeyPars(step.binCenter(idx));
        half_xm_bin_pars.xm = step.binWidth(4)/2.0; // 4 = xm par
        double xshift =
            m_keylyrtool.xExpected(half_xm_bin_pars, storedhit.hitptr.get());
        double xrange = xshift + r1 * step.binWidth(2) / 2.0
                        + ((r2*step.binWidth(3) - r1*step.binWidth(2)) / (r2 - r1) * (hitr - r1))/2.0;

        passesPhi = std::abs(storedhit.phiShift) < xrange;
    }

    if (stepIsREta(step)) {
        // distance of hit from bin center
        storedhit.etaShift = etaResidual(step.binCenter(idx),storedhit.hitptr.get());
    
        double width_z_in  = step.binWidth(0);
        double width_z_out = step.binWidth(1);
        double zrange = width_z_in + (width_z_out-width_z_in) * (hitr-r1)/(r2-r1);
        
        passesEta = std::abs(storedhit.etaShift) < zrange;

    }

    return passesPhi && passesEta;
}

// figure out if step is r-phi or r-eta plan
bool FPGATrackSimKeyLayerBinDesc::stepIsRPhi(
    const FPGATrackSimBinStep &step) const {
  for (const unsigned &steppar : step.stepPars()) {
    for (const unsigned &phipar : m_phipars) {
      if (steppar == phipar)
        return true;
    }
  }
  return false;
}

bool FPGATrackSimKeyLayerBinDesc::stepIsREta(
    const FPGATrackSimBinStep &step) const {
  for (const unsigned &steppar : step.stepPars()) {
    for (const unsigned &etapar : m_etapars) {
      if (steppar == etapar)
        return true;
    }
  }
  return false;
}