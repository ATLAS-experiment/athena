/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1NSWSimTools/MMT_struct.h"
#include "MuonReadoutGeometry/MuonChannelDesign.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

MMT_Parameters::MMT_Parameters(const std::string& layerSetup, char wedgeSize, const MuonGM::MuonDetectorManager* detManager)
  : AthMessaging(Athena::getMessageSvc(), "MMT_Parameters") {

  // Get the modules for each multiplet, the sector and side arguement (third and fifth argument, respectively) shouldn't matter
  // since the algorithm computes locally
  std::string wedgeString = (wedgeSize=='L' ? "MML" : "MMS");
  m_sector = wedgeSize;

  Identifier mm_top_ele_id = detManager->mmIdHelper()->elementID(wedgeString, 2, 5);
  Identifier mm_bottom_ele_id = detManager->mmIdHelper()->elementID(wedgeString, 1, 5);

  const MuonGM::MMReadoutElement* mm_ReadOut_top = detManager->getMMReadoutElement(mm_top_ele_id);
  const MuonGM::MMReadoutElement* mm_ReadOut_bottom = detManager->getMMReadoutElement(mm_bottom_ele_id);

  m_lWidth = mm_ReadOut_top->getLongSsize(); 
  m_pitch = mm_ReadOut_top->getDesign(mm_top_ele_id)->inputPitch;

  //Needed to get the max channels for the deadZone in the x planes... a little annoying...
  //Only changes for small or large and per module. Phi, layer, etc don't matter (just set to 1)

  m_innerRadiusEta1 = mm_ReadOut_bottom->absTransform().translation().perp() - 0.5*mm_ReadOut_bottom->getRsize();
  m_innerRadiusEta2 = mm_ReadOut_top->absTransform().translation().perp() - 0.5*mm_ReadOut_top->getRsize();

  const auto design = mm_ReadOut_bottom->getDesign(mm_bottom_ele_id);
  m_missedBottomEta = design->nMissedBottomEta;
  m_missedBottomStereo = design->nMissedBottomStereo;

  // retrieve the z-position of the planes
  std::vector<double> z_nominal;
  int eta = 1;
  for (const auto& pos: MM_firststrip_positions(detManager, wedgeString, eta)) z_nominal.push_back(pos.Z());

  if(z_nominal.size() != layerSetup.size()){
    ATH_MSG_WARNING("Number of planes in setup is "<< layerSetup.size() << ", but we have a nominal " << z_nominal.size() << " planes.");
    throw std::runtime_error("MMT_Parameters: Invalid number of planes");
  }

  float stereo_degree = design->stereoAngle();

  // MM_firststrip_positions returns the position for phi sector 1.
  // => for the small sectors, rotate this by -1/16 of a rotation to make our lives easier.
  // in this coordinate basis, x is up/down the wedge (radial), and y is left/right (phi).
  float cos_rotation = std::cos(-2*M_PI / 16.0);
  float sin_rotation = std::sin(-2*M_PI / 16.0);
  float x_rotated = 0.0;
  float y_rotated = 0.0;

  float st_angle = 0.0;

  float radial_pos      = 0;
  float radial_pos_xx_1 = 0, radial_pos_xx_2 = 0;
  float radial_pos_uv_1 = 0, radial_pos_uv_2 = 0;

  for (unsigned int eta = 1; eta <= 2; eta++){
    unsigned int layer = 1;
    for (const auto& pos: MM_firststrip_positions(detManager, wedgeString, eta)){

      if (wedgeString=="MMS"){
        x_rotated = pos.X()*cos_rotation - pos.Y()*sin_rotation;
        y_rotated = pos.X()*sin_rotation + pos.Y()*cos_rotation;
      }
      else{
        x_rotated = pos.X();
        y_rotated = pos.Y();
      }

      if      (is_u(layer)) st_angle = -1*std::abs(stereo_degree);
      else if (is_v(layer)) st_angle =    std::abs(stereo_degree);
      else                  st_angle = 0;

      // walk from the center of the strip to the position of the strip at the center of the wedge.
      // NB: for X-planes, this is simply the center of the strip: tan(0) = 0.
      radial_pos = std::abs(x_rotated - y_rotated*std::tan(st_angle * M_PI/180.0));

      if (is_x(layer) && eta==1) radial_pos_xx_1 = radial_pos;
      else if (is_x(layer) && eta==2) radial_pos_xx_2 = radial_pos;
      if (is_u(layer) && eta==1) radial_pos_uv_1 = radial_pos;
      else if (is_u(layer) && eta==2) radial_pos_uv_2 = radial_pos;

      layer++;
    }
  }

  // store radial positions as fixed point
  std::array<float, 2> radial_pos_xx = {radial_pos_xx_1, radial_pos_xx_2};
  std::array<float, 2> radial_pos_uv = {radial_pos_uv_1, radial_pos_uv_2};
  m_ybases[0] = radial_pos_xx;
  m_ybases[1] = radial_pos_xx;
  m_ybases[2] = radial_pos_uv;
  m_ybases[3] = radial_pos_uv;
  m_ybases[4] = radial_pos_uv;
  m_ybases[5] = radial_pos_uv;
  m_ybases[6] = radial_pos_xx;
  m_ybases[7] = radial_pos_xx;
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

evInf_entry::evInf_entry(int event,int pdg,double e,double p,double ieta,double peta,double eeta,double iphi,double pphi,double ephi,double ithe,double pthe,double ethe,double dth,
                         int trn,int mun,const ROOT::Math::XYZVector& tex):
   athena_event(event),pdg_id(pdg),E(e),pt(p),eta_ip(ieta),eta_pos(peta),eta_ent(eeta),phi_ip(iphi),phi_pos(pphi),phi_ent(ephi),theta_ip(ithe),theta_pos(pthe),theta_ent(ethe),
   dtheta(dth),truth_n(trn),mu_n(mun),vertex(tex) {}


hitData_entry::hitData_entry(int ev, double gt, double q, int vmm, int mmfe, int pl, int st, int est, int phi, int mult, int gg, double locX, double tr_the, double tru_phi,
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
