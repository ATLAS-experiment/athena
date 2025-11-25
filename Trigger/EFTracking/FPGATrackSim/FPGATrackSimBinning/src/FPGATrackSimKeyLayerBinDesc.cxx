
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
#include <cmath>



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
            m_keylyrtool.xExpected(half_xm_bin_pars, storedhit.hitptr->getR(), remainder(storedhit.hitptr->getGPhi()+m_phiOffset,2*M_PI));
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
        ATH_MSG_VERBOSE("Phi qpt pad: " << storedhit.phiShift << " " << hitr*m_qptpad*1000.0*FPGATrackSimBinUtil::GeomHelpers::dPhidQOverPt(hitr) << " " << m_qptpad);
        if (isTruthBin && !passesPhi) ATH_MSG_DEBUG("Hit fails Phi cut, lyr=" << storedhit.hitptr->getPhysLayer() << " "
                        << storedhit.phiShift << " " << xrange + padding << " " <<xrange << " "<< padding
                        << " " << m_d0pad << " "  << hitr*m_phipad  << " "  << hitr*m_qptpad*FPGATrackSimBinUtil::GeomHelpers::dPhidQOverPt(hitr)
                        << " "  << ((storedhit.hitptr->getDetType() == SiliconTech::strip) ?  (stripLength*std::abs(FPGATrackSimBinUtil::GeomHelpers::dPhiHitTrkFromPars(hitr,trackpars))) : 99999)
                        << " " << hitr << " " << trackpars);

        // Firmware x-check
        phiLUTConsts phiconsts = getPhiLUTConsts(step,step.stepIdx(idx));
        double fw_phiShift = phiconsts.phiShift(remainder(storedhit.hitptr->getGPhi() + m_phiOffset,2*M_PI),  hitr);
        double fw_phiWindow = phiconsts.phiWindow( hitr);        
        ATH_MSG_VERBOSE("FW x-check: phiShift orig: " << storedhit.phiShift << " fwcalc: " << fw_phiShift << " diff: " << storedhit.phiShift-fw_phiShift);
        ATH_MSG_VERBOSE("FW " << phiconsts.w_in << " " << phiconsts.dw_dr*(hitr-phiconsts.r_in) << " " <<  phiconsts.w_x*(hitr-phiconsts.r_in)*(phiconsts.r_out-hitr) 
        << "    Orig: " <<  r1 * step.binWidth(2) / 2.0 << " " << ((r2*step.binWidth(3) - r1*step.binWidth(2)) / (r2 - r1) * (hitr - r1))/2.0 << " " << std::abs(xshift));
        ATH_MSG_VERBOSE("FW x-check: phiWindow orig: " << xrange << " fwcalc: " << fw_phiWindow << " diff: " << xrange-fw_phiWindow);
        
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

        // Firmware x-check
        etaLUTConsts etaconsts = getEtaLUTConsts(step,step.stepIdx(idx));
        double fw_etaShift = etaconsts.etaShift(storedhit.hitptr->getZ(),  hitr);
        double fw_etaWindow = etaconsts.etaWindow( hitr);
        ATH_MSG_VERBOSE("FW x-check: etaShift orig: " << storedhit.etaShift << " fwcalc: " << fw_etaShift << " diff: " << storedhit.etaShift-fw_etaShift);
        ATH_MSG_VERBOSE("FW x-check: etaWindow orig: " << zrange << " fwcalc: " << fw_etaWindow << " diff: " << zrange-fw_etaWindow);
        

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
//     Calculate the Firmware LUT for a bin
// 
//---------------------------------------------------------------------------------------
FPGATrackSimKeyLayerBinDesc::phiLUTConsts FPGATrackSimKeyLayerBinDesc::getPhiLUTConsts(const FPGATrackSimBinStep &step, const std::vector<unsigned>& idx) const {
  phiLUTConsts retv;

  FPGATrackSimKeyLayerTool::KeyLyrPars keypars;
  keypars.phi1 = step.binCenter(2,idx[0]);
  keypars.phi2 = step.binCenter(3,idx[1]);
  keypars.xm = step.binCenter(4, idx[2]);

  auto rotated_coords = m_keylyrtool.getRotatedConfig(keypars);

  retv.y = rotated_coords.y;
  retv.x1p = rotated_coords.xy1p.first;
  retv.y1p = rotated_coords.xy1p.second;
  retv.cosb = rotated_coords.rotang.first;
  retv.sinb = rotated_coords.rotang.second;
  retv.x_m = keypars.xm;
  retv.x_factor = 4.0 * keypars.xm / (rotated_coords.y * rotated_coords.y);

  double r_in = m_keylyrtool.R1();
  double r_out = m_keylyrtool.R2();
  retv.r_in = r_in;
  retv.r_out= r_out;

  double w_in = r_in * step.binWidth(2) / 2.0;
  double w_out = r_out * step.binWidth(3) / 2.0;
  double w_x = step.binWidth(4) / 2.0;

  retv.w_x = 4.0 * w_x / ((r_out - r_in) * (r_out - r_in));
  retv.w_in = w_in;
  retv.dw_dr = (w_out - w_in) / (r_out - r_in);

  return retv;
}

FPGATrackSimKeyLayerBinDesc::etaLUTConsts FPGATrackSimKeyLayerBinDesc::getEtaLUTConsts(const FPGATrackSimBinStep &step, const std::vector<unsigned>& idx) const {
  etaLUTConsts retv;

  double r_in = m_keylyrtool.R1();
  double r_out = m_keylyrtool.R2();
  retv.r_in = r_in;
  retv.r_out= r_out;

  double z_in = step.binCenter(0, idx[0]);
  double z_out = step.binCenter(1, idx[1]);
  double dz_dr = (z_out - z_in) / (r_out - r_in);

  retv.z_in = z_in;
  retv.dz_dr = dz_dr;

  double w_in = step.binWidth(0) / 2.0;
  double w_out = step.binWidth(1) / 2.0;
  double dw_dr = (w_out - w_in) / (r_out - r_in);
  retv.w_in = w_in;
  retv.dw_dr = dw_dr;

  return retv;
}

double FPGATrackSimKeyLayerBinDesc::phiLUTConsts::phiShift(double phi, double r) {
    double xc = r*cos(phi);
    double yc = r*sin(phi);
    double xh = xc*cosb+yc*sinb-x1p;
    double yh = -xc*sinb+yc*cosb-y1p;
    return xh - x_factor*yh*(y-yh);
}
double FPGATrackSimKeyLayerBinDesc::phiLUTConsts::phiWindow(double r) { 
    return w_in + dw_dr*(r-r_in) + w_x*(r-r_in)*(r_out-r);
}
double FPGATrackSimKeyLayerBinDesc::etaLUTConsts::etaShift(double z, double r) {     
    return z - z_in - dz_dr*(r-r_in);
}
double FPGATrackSimKeyLayerBinDesc::etaLUTConsts::etaWindow(double r) {
  return w_in + dw_dr*(r-r_in);
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
    std::string keylayerName = "reg_" + std::to_string(m_region.value()) + "_KeyLayer";
    FPGATrackSimBinUtil::StreamManager sm(keylayerName);
    sm.writeVar("r_in",r_in);
    sm.writeVar("r_out",r_out);
    sm.writeVar("phi_offset",m_phiOffset.value());
  }

  if (stepIsRPhi(step)) {

    std::string stepName = "reg_" + std::to_string(m_region.value()) + "_" + step.stepName();
    FPGATrackSimBinUtil::StreamManager sm(stepName);
    int nbins = 0;
    for (FPGATrackSimBinArray<int>::ConstIterator &bin : step.validBinsLocal()) {
      if (!bin.data())
        continue;      
      phiLUTConsts phiconsts = getPhiLUTConsts(step,bin.idx());

      sm.writeVar("phi_bin", bin.idx());
  
      sm.writeVar("y", phiconsts.y);
      sm.writeVar("x1p", phiconsts.x1p);
      sm.writeVar("y1p", phiconsts.y1p);
      sm.writeVar("cosb", phiconsts.cosb);
      sm.writeVar("sinb", phiconsts.sinb);

      sm.writeVar("x_m", phiconsts.x_m);
      sm.writeVar("x_factor", phiconsts.x_factor);

      if (nbins == 0) {
        sm.writeVar("w_x", phiconsts.w_x);
        sm.writeVar("w_in", phiconsts.w_in);
        sm.writeVar("dw_dr", phiconsts.dw_dr);
      }

      nbins++;
    }    

    sm.writeVar("nbins", nbins);
  }

  if (stepIsREta(step)) {

    std::string stepName = "reg_" + std::to_string(m_region.value()) + "_" + step.stepName();
    FPGATrackSimBinUtil::StreamManager sm(stepName);

    int nbins = 0;
    for (FPGATrackSimBinArray<int>::ConstIterator &bin : step.validBinsLocal()) {
      if (!bin.data())
        continue;

      etaLUTConsts etaconsts = getEtaLUTConsts(step,bin.idx());

      // write just this steps idxs
      sm.writeVar("z_bin", bin.idx());      
      sm.writeVar("z_in", etaconsts.z_in);
      sm.writeVar("dz_dr", etaconsts.dz_dr);
    
      if (nbins==0) {
        sm.writeVar("w_in", etaconsts.w_in);
        sm.writeVar("dw_dr", etaconsts.dw_dr);
      };
      
      nbins++;
    }
    sm.writeVar("nbins", nbins);
      
  }

}
    
