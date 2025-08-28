
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimBinUtil.cxx
 * @author Elliot Lipeles
 * @date Feb 13, 2025
 * @brief See header file.
 */

#include "FPGATrackSimKeyLayerBinDesc.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "FPGATrackSimBinning/FPGATrackSimBinStep.h"
#include "FPGATrackSimBinning/FPGATrackSimBinUtil.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "src/FPGATrackSimKeyLayerTool.h"



using FPGATrackSimBinUtil::IdxSet;
using FPGATrackSimBinUtil::StoredHit;

StatusCode FPGATrackSimKeyLayerBinDesc::initialize()
{
  // Dump the configuration to make sure it propagated through right
  const std::vector<Gaudi::Details::PropertyBase*> props = this->getProperties();
  for( Gaudi::Details::PropertyBase* prop : props ) {
    if (prop->ownerTypeName()==this->type()) {      
      ATH_MSG_DEBUG("Property:\t" << prop->name() << "\t : \t"<< prop->toString());
    }
  }

  m_keylyrtool.setR1(m_rin);
  m_keylyrtool.setR2(m_rout);

  return StatusCode::SUCCESS;
}

bool FPGATrackSimKeyLayerBinDesc::hitInBin(const FPGATrackSimBinStep &step,
                                           const IdxSet &idx,
                                           StoredHit &storedhit) const
{
    double r1 = m_keylyrtool.R1();
    double r2 = m_keylyrtool.R2();
    double hitr= storedhit.hitptr->getR();

    bool passesPhi = true;
    bool passesEta = true;

    FPGATrackSimTrackPars trackpars = parSetToTrackPars(step.binCenter(idx));

    bool isTruthBin = ((m_truthbin.size()>step.stepNum())&&(m_truthbin[step.stepNum()]==idx));
    
    if (stepIsRPhi(step)) {
        // distance of hit from bin center
        storedhit.phiShift =
            phiResidual(step.binCenter(idx), storedhit.hitptr.get());
        
        // Get expected curvature shift from bin center    
        auto half_xm_bin_pars = parSetToKeyPars(step.binCenter(idx));
        half_xm_bin_pars.xm = step.binWidth(4)/2.0; // 4 = xm par
        double xshift =
            m_keylyrtool.xExpected(half_xm_bin_pars, storedhit.hitptr.get());
        double xrange = std::abs(xshift) + r1 * step.binWidth(2) / 2.0
                        + ((r2*step.binWidth(3) - r1*step.binWidth(2)) / (r2 - r1) * (hitr - r1))/2.0;

        if (xrange < 0) {
          ATH_MSG_ERROR("Negative xrange: " << std::abs(xshift) << " " << r1 * step.binWidth(2) / 2.0 << " " 
            << ((r2*step.binWidth(3) - r1*step.binWidth(2)) / (r2 - r1) * (hitr - r1))/2.0);
        }

        double padding = 0.0;
        double stripLength = 25.0;
        if (storedhit.hitptr->getDetType() == SiliconTech::strip) {
          if (!storedhit.hitptr->isBarrel()) {
              // varying strip lengths per eta mod in endcap
              int etamod = storedhit.hitptr->getEtaModule();
              stripLength = m_slPerEtaMod[etamod]/2.0;
          }
          padding += stripLength*std::abs(FPGATrackSimBinUtil::GeomHelpers::dPhiHitTrkFromPars(hitr,trackpars));
        }
        // add phiShift resolution padding, 1000.0 is the GeV to MeV conversion
        padding += m_d0pad + hitr*m_phipad + hitr*m_qptpad*1000.0*FPGATrackSimBinUtil::GeomHelpers::dPhidQOverPt(hitr);
        passesPhi = std::abs(storedhit.phiShift) < (xrange+padding);
        ATH_MSG_DEBUG("Phi qpt pad: " << storedhit.phiShift << " " << hitr*m_qptpad*1000.0*FPGATrackSimBinUtil::GeomHelpers::dPhidQOverPt(hitr) << " " << m_qptpad);
        if (isTruthBin && !passesPhi) ATH_MSG_DEBUG("Hit fails Phi cut, lyr=" << storedhit.hitptr->getPhysLayer() << " "
                        << storedhit.phiShift << " " << xrange + padding << " " <<xrange << " "<< padding
                        << " " << m_d0pad << " "  << hitr*m_phipad  << " "  << hitr*m_qptpad*FPGATrackSimBinUtil::GeomHelpers::dPhidQOverPt(hitr)
                        << " "  << ((storedhit.hitptr->getDetType() == SiliconTech::strip) ?  (stripLength*std::abs(FPGATrackSimBinUtil::GeomHelpers::dPhiHitTrkFromPars(hitr,trackpars))) : 99999)
                        << " " << hitr << " " << trackpars);
    }

    if (stepIsREta(step)) {
        // distance of hit from bin center
        storedhit.etaShift = etaResidual(step.binCenter(idx),storedhit.hitptr.get());
    
        double width_z_in  = step.binWidth(0)/2.0;
        double width_z_out = step.binWidth(1)/2.0;
        double zrange = width_z_in + (width_z_out-width_z_in) * std::abs((hitr-r1))/(r2-r1);

        // pad for strip length or imprecise SP.
        double padding = 0;
        double stripLength = 25.0; //barrel strip length
        if (storedhit.hitptr->getDetType() == SiliconTech::strip) {
          if (!storedhit.hitptr->isBarrel()) {
              // varying strip lengths per eta mod in endcap
              int etamod = storedhit.hitptr->getEtaModule();
              stripLength = m_slPerEtaMod[etamod]/2.0;
          }
          // length of longest correspinding to endcap or barrel strip
          padding += stripLength;
        }
        // add etaShift resolution padding
        padding += (m_z0pad + std::abs(hitr*m_etapad*FPGATrackSimBinUtil::GeomHelpers::dZdEta(trackpars.eta)));
        passesEta = std::abs(storedhit.etaShift) < (zrange+padding);
        if (isTruthBin && !passesEta) ATH_MSG_DEBUG("Hit fails Eta cut , lyr=" << storedhit.hitptr->getPhysLayer() << " r=" << hitr << " " 
                        << storedhit.etaShift << " " << zrange + padding << " " <<zrange << " "<< padding << " " << hitr << " " << trackpars);        
    }
   
    
    if (isTruthBin && !(passesPhi && passesEta)) ATH_MSG_DEBUG("Hit in truth bin fails cuts: " << " passesPhi=" << passesPhi << " passesEta=" << passesEta);

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

//---------------------------------------------------------------------------------------
//
//     Write the relevant LUT tables for firmware
// 
//---------------------------------------------------------------------------------------
void FPGATrackSimKeyLayerBinDesc::writeLUTs(const FPGATrackSimBinStep &step) const {
  double r_in = m_keylyrtool.R1();
  double r_out = m_keylyrtool.R2();

  ATH_MSG_INFO("Writing constants for step:" << step.stepName() << " isRPhi="<< stepIsRPhi(step) << " isREta="<< stepIsREta(step));
  
  // write the keylayer definition
  if (step.isFirstStep()) {
    FPGATrackSimBinUtil::StreamManager sm("KeyLayer");
    sm.writeVar("r_in",r_in);
    sm.writeVar("r_out",r_out);
  }

  if (stepIsRPhi(step)) {

    FPGATrackSimBinUtil::StreamManager sm(step.stepName());
    int nbins = 0;
    for (FPGATrackSimBinArray<int>::ConstIterator &bin : step.validBinsLocal()) {
      if (!bin.data())
        continue;      

      sm.writeVar("phi_bin", bin.idx());

      FPGATrackSimKeyLayerTool::KeyLyrPars keypars;
      keypars.phi1 = step.binCenter(2,bin.idx()[0]);
      keypars.phi2 = step.binCenter(3,bin.idx()[1]);
      keypars.xm = step.binCenter(4, bin.idx()[2]);

      auto rotated_coords = m_keylyrtool.getRotatedConfig(keypars);

      sm.writeVar("y", rotated_coords.y);
      sm.writeVar("x1p", rotated_coords.xy1p.first);
      sm.writeVar("y1p", rotated_coords.xy1p.second);
      sm.writeVar("cosb", rotated_coords.rotang.first);
      sm.writeVar("sinb", rotated_coords.rotang.second);

      sm.writeVar("x_m", keypars.xm);
      sm.writeVar("x_factor", 4.0 * keypars.xm / (rotated_coords.y * rotated_coords.y));

      nbins++;
    }

    double w_in = r_in * step.binWidth(2) / 2.0;
    double w_out = r_out * step.binWidth(3) / 2.0;
    double w_x = step.binWidth(4) / 2.0;
    double dw_dr = (w_out - w_in) / (r_out - r_in);

    sm.writeVar("w_x", 4.0 * w_x / ((r_out - r_in) * (r_out - r_in)));
    sm.writeVar("w_in", w_in);
    sm.writeVar("dw_dr", dw_dr);

    sm.writeVar("nbins", nbins);
  }

  if (stepIsREta(step)) {

    FPGATrackSimBinUtil::StreamManager sm(step.stepName());

    int nbins = 0;
    for (FPGATrackSimBinArray<int>::ConstIterator &bin : step.validBinsLocal()) {
      if (!bin.data())
        continue;

      // write just this steps idxs
      sm.writeVar("z_bin", bin.idx());

      double z_in = step.binCenter(0, bin.idx()[0]);
      double z_out = step.binCenter(1, bin.idx()[1]);
      double dz_dr = (z_out - z_in) / (r_out - r_in);
      
      sm.writeVar("z_in", z_in);
      sm.writeVar("dz_dr", dz_dr);
    
      nbins++;
    }
    sm.writeVar("nbins", nbins);

    // same for all bins
    double w_in = step.binWidth(0) / 2.0;
    double w_out = step.binWidth(1) / 2.0;
    double dw_dr = (w_out - w_in) / (r_out - r_in);
    sm.writeVar("w_in", w_in);
    sm.writeVar("dw_dr", dw_dr);

  }

    


}
    
