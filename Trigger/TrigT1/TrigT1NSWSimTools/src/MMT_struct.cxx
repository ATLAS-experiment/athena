/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1NSWSimTools/MMT_struct.h"
#include "MuonAGDDDescription/MMDetectorDescription.h"
#include "MuonAGDDDescription/MMDetectorHelper.h"
#include "MuonReadoutGeometry/MuonChannelDesign.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

MMT_Parameters::MMT_Parameters(std::string layerSetup, char wedgeSize, const MuonGM::MuonDetectorManager* detManager)
  : AthMessaging(Athena::getMessageSvc(), "MMT_Parameters") {

  // Get the modules for each multiplet, the sector and side arguement (third and fifth argument, respectively) shouldn't matter
  // since the algorithm computes locally
  char side   = 'A';
  m_sector = wedgeSize;
  MMDetectorHelper aHelper;
  MMDetectorDescription* mm_top_mult1    = aHelper.Get_MMDetector(m_sector, 2, 5, 1, side); // Parameters: sector, eta, phi, layer/multiplet
  MMDetectorDescription* mm_top_mult2    = aHelper.Get_MMDetector(m_sector, 2, 5, 2, side);
  MMDetectorDescription* mm_bottom_mult1 = aHelper.Get_MMDetector(m_sector, 1, 5, 1, side);
  MMDetectorDescription* mm_bottom_mult2 = aHelper.Get_MMDetector(m_sector, 1, 5, 2, side);

  MMReadoutParameters roParam_top_mult1    = mm_top_mult1->GetReadoutParameters();
  MMReadoutParameters roParam_top_mult2    = mm_top_mult2->GetReadoutParameters();
  MMReadoutParameters roParam_bottom_mult1 = mm_bottom_mult1->GetReadoutParameters();
  MMReadoutParameters roParam_bottom_mult2 = mm_bottom_mult2->GetReadoutParameters();

  //Needed to get the max channels for the deadZone in the x planes... a little annoying...
  //Only changes for small or large and per module. Phi, layer, etc don't matter (just set to 1)
  std::string wedgeString = (wedgeSize=='L' ? "MML" : "MMS");

  m_pitch = roParam_bottom_mult1.stripPitch;
  m_lWidth = mm_top_mult1->lWidth();
  m_innerRadiusEta1 = roParam_bottom_mult1.distanceFromZAxis;
  m_innerRadiusEta2 = roParam_top_mult1.distanceFromZAxis;
  m_missedBottomEta = roParam_bottom_mult1.nMissedBottomEta;
  m_missedBottomStereo = roParam_bottom_mult1.nMissedBottomStereo;
  if (m_sector == 'L') {
    ATH_MSG_DEBUG("LM1 \n" <<
                 "\t\t\t\t Total Strips: " << roParam_bottom_mult1.tStrips << " with pitch: " << roParam_bottom_mult1.stripPitch << "\n" <<
                 "\t\t\t\t KO strips TopEta: " << roParam_bottom_mult1.nMissedTopEta << " - BottomEta: " << roParam_bottom_mult1.nMissedBottomEta << "\n" <<
                 "\t\t\t\t KO strips TopStereo: " << roParam_bottom_mult1.nMissedTopStereo << " - BottomStereo: " << roParam_bottom_mult1.nMissedBottomStereo << "\n" <<
                 "\t\t\t\t (Top, Bottom, Length): (" << mm_bottom_mult1->lWidth() << ", " << mm_bottom_mult1->sWidth() << ", " << mm_bottom_mult1->Length() << ")\n" <<
                 "\t\t\t\t Zpos - Distance from ZAxis: " << roParam_bottom_mult1.zpos << " - " << roParam_bottom_mult1.distanceFromZAxis << "\n" <<
                 "\t\t\t\t dlStereoTop/Bottom: " << roParam_bottom_mult1.dlStereoTop << " " << roParam_bottom_mult1.dlStereoBottom << "\n" <<
                 "\t\t\t\t Active area --> (Bottom, Top, Height) : (" << roParam_bottom_mult1.activeBottomLength << ", " << roParam_bottom_mult1.activeTopLength << ", " << roParam_bottom_mult1.activeH << ")");
  } else if (m_sector == 'S') {
    ATH_MSG_DEBUG("SM1 \n" <<
                 "\t\t\t\t KO strips TopEta: " << roParam_bottom_mult1.nMissedTopEta << " - BottomEta: " << roParam_bottom_mult1.nMissedBottomEta << "\n" <<
                 "\t\t\t\t KO strips TopStereo: " << roParam_bottom_mult1.nMissedTopStereo << " - BottomStereo: " << roParam_bottom_mult1.nMissedBottomStereo << "\n" <<
                 "\t\t\t\t Total Strips: " << roParam_bottom_mult1.tStrips << " with pitch: " << roParam_bottom_mult1.stripPitch << "\n" <<
                 "\t\t\t\t (Top, Bottom, Length): (" << mm_bottom_mult1->lWidth() << ", " << mm_bottom_mult1->sWidth() << ", " << mm_bottom_mult1->Length() << ")\n" <<
                 "\t\t\t\t Zpos - Distance from ZAxis: " << roParam_bottom_mult1.zpos << " - " << roParam_bottom_mult1.distanceFromZAxis << "\n" <<
                 "\t\t\t\t dlStereoTop/Bottom: " << roParam_bottom_mult1.dlStereoTop << " " << roParam_bottom_mult1.dlStereoBottom << "\n" <<
                 "\t\t\t\t Active area --> (Bottom, Top, Height) : (" << roParam_bottom_mult1.activeBottomLength << ", " << roParam_bottom_mult1.activeTopLength << ", " << roParam_bottom_mult1.activeH << ")");
  }

  if (m_sector == 'L') {
    ATH_MSG_DEBUG("LM2 \n" <<
                 "\t\t\t\t Total Strips: " << roParam_top_mult1.tStrips << " with pitch: " << roParam_top_mult1.stripPitch << "\n" <<
                 "\t\t\t\t KO strips TopEta: " << roParam_top_mult1.nMissedTopEta << " - BottomEta: " << roParam_top_mult1.nMissedBottomEta << "\n" <<
                 "\t\t\t\t KO strips TopStereo: " << roParam_top_mult1.nMissedTopStereo << " - BottomStereo: " << roParam_top_mult1.nMissedBottomStereo << "\n" <<
                 "\t\t\t\t (Top, Bottom, Length): (" << mm_top_mult1->lWidth() << ", " << mm_top_mult1->sWidth() << ", " << mm_top_mult1->Length() << ")\n" <<
                 "\t\t\t\t Zpos - Distance from ZAxis: " << roParam_top_mult1.zpos << " - " << roParam_top_mult1.distanceFromZAxis << "\n" <<
                 "\t\t\t\t dlStereoTop/Bottom: " << roParam_top_mult1.dlStereoTop << " " << roParam_top_mult1.dlStereoBottom << "\n" <<
                 "\t\t\t\t Active area --> (Top, Bottom, Height) : (" << roParam_top_mult1.activeTopLength << ", " << roParam_top_mult1.activeBottomLength << ", " << roParam_top_mult1.activeH << ")");
  } else if (m_sector == 'S') {
    ATH_MSG_DEBUG("SM2 \n" <<
                 "\t\t\t\t KO strips TopEta: " << roParam_top_mult1.nMissedTopEta << " - BottomEta: " << roParam_top_mult1.nMissedBottomEta << "\n" <<
                 "\t\t\t\t KO strips TopStereo: " << roParam_top_mult1.nMissedTopStereo << " - BottomStereo: " << roParam_top_mult1.nMissedBottomStereo << "\n" <<
                 "\t\t\t\t (Top, Bottom, Length): (" << mm_top_mult1->lWidth() << ", " << mm_top_mult1->sWidth() << ", " << mm_top_mult1->Length() << ")\n" <<
                 "\t\t\t\t Zpos - Distance from ZAxis: " << roParam_top_mult1.zpos << " - " << roParam_top_mult1.distanceFromZAxis << "\n" <<
                 "\t\t\t\t dlStereoTop/Bottom: " << roParam_top_mult1.dlStereoTop << " " << roParam_top_mult1.dlStereoBottom << "\n" <<
                 "\t\t\t\t Active area --> (Top, Bottom, Height) : (" << roParam_top_mult1.activeTopLength << ", " << roParam_top_mult1.activeBottomLength << ", " << roParam_top_mult1.activeH << ")");
    for (const auto &angle : roParam_top_mult1.stereoAngle) ATH_MSG_DEBUG("Stereo angle: " << angle);
  }

  // retrieve the z-position of the planes
  std::vector<double> z_nominal;
  int eta = 1;
  for (const auto& pos: MM_firststrip_positions(detManager, wedgeString, eta)) z_nominal.push_back(pos.Z());

  if(z_nominal.size() != layerSetup.size()){
    ATH_MSG_WARNING("Number of planes in setup is "<< layerSetup.size() << ", but we have a nominal " << z_nominal.size() << " planes.");
    throw std::runtime_error("MMT_Parameters: Invalid number of planes");
  }
}

std::vector<ROOT::Math::XYZVector> MMT_Parameters::MM_firststrip_positions(const MuonGM::MuonDetectorManager* detManager, const std::string& wedge, int eta) const {

  std::vector<ROOT::Math::XYZVector> positions;
  positions.reserve(8);

  Identifier strip_id;
  int phi = 1;
  int strip = 200;
  unsigned int n_multiplets = 2;
  unsigned int n_layers     = 4;

  for (unsigned int mult = 1; mult <= n_multiplets; mult++) {
    for (unsigned int layer = 1; layer <= n_layers; layer++) {

      Amg::Vector3D pos(0.0, 0.0, 0.0);
      strip_id = detManager->mmIdHelper()->channelID(wedge, eta, phi, mult, layer, strip);
      const MuonGM::MMReadoutElement* readout = detManager->getMMReadoutElement(strip_id);
      ROOT::Math::XYZVector coordinates(0.,0.,0.);
      if (readout->stripGlobalPosition(strip_id, pos)) coordinates.SetXYZ(pos.x(), pos.y(), pos.z());
      else ATH_MSG_WARNING("Wedge " << wedge << " phi " << phi << " multiplet " << mult << " layer " << layer <<  " | Unable to retrieve global positions");
      positions.push_back(coordinates);

      ATH_MSG_DEBUG( "global z-pos. (using MMReadoutElement) for"
                           << " wedge size "       << wedge
                           << " multiplet "        << mult
                           << " layer "            << layer
                           << " | z = "            << coordinates.Z() );
    }
  }

   return positions;
}

evInf_entry::evInf_entry(uint64_t event,int pdg,double e,double p,double ieta,double peta,double eeta,double iphi,double pphi,double ephi,double ithe,double pthe,double ethe,double dth,
                         int trn,int mun,const ROOT::Math::XYZVector& tex):
   athena_event(event),pdg_id(pdg),E(e),pt(p),eta_ip(ieta),eta_pos(peta),eta_ent(eeta),phi_ip(iphi),phi_pos(pphi),phi_ent(ephi),theta_ip(ithe),theta_pos(pthe),theta_ent(ethe),
   dtheta(dth),truth_n(trn),mu_n(mun),vertex(tex) {}


hitData_entry::hitData_entry(uint64_t ev, double gt, double q, int vmm, int mmfe, int pl, int st, int est, int phi, int mult, int gg, double locX, double tr_the, double tru_phi,
                             bool q_tbg, int bct, const ROOT::Math::XYZVector& tru, const ROOT::Math::XYZVector& rec):
  event(ev),gtime(gt),charge(q),VMM_chip(vmm),MMFE_VMM(mmfe),plane(pl),strip(st),station_eta(est),station_phi(phi),multiplet(mult),gasgap(gg),localX(locX),tru_theta_ip(tr_the),tru_phi_ip(tru_phi),truth_nbg(q_tbg),BC_time(bct),truth(tru),recon(rec) {}

digitWrapper::digitWrapper(const MmDigit* digit,
                           const std::string &stationName,
                           double tmpGTime,
                           const ROOT::Math::XYZVector& truthLPos,
                           const ROOT::Math::XYZVector& stripLPos,
                           const ROOT::Math::XYZVector& stripGPos
                           ):
  digit(digit),
  stName(stationName),
  gTime(tmpGTime),
  truth_lpos(truthLPos),
  strip_lpos(stripLPos),
  strip_gpos(stripGPos){}
