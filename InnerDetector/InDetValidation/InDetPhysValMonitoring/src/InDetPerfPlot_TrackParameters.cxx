/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetPerfPlot_TrackParameters.cxx
 * @author shaun roe
 **/


#include "InDetPhysValMonitoringUtilities.h"

#include "InDetPerfPlot_TrackParameters.h"
#include <cmath>
using namespace IDPVM;

InDetPerfPlot_TrackParameters::InDetPerfPlot_TrackParameters(InDetPlotBase* pParent, const std::string& sDir) :
  InDetPlotBase(pParent, sDir){
  //nop
  //variable initialised at declaration
}

void
InDetPerfPlot_TrackParameters::initializePlots() {

  book(m_reco_d0,     "reco_d0");
  book(m_reco_z0,     "reco_z0");
  book(m_reco_z0sin,  "reco_z0sin");
  book(m_reco_phi,    "reco_phi");
  book(m_reco_theta,  "reco_theta");
  book(m_reco_eta,    "reco_eta");
  book(m_reco_qoverp, "reco_qoverp");
  book(m_reco_pt,     "reco_pt");
  book(m_reco_lowpt,     "reco_lowpt");
  book(m_reco_chi2,   "reco_chi2");
  book(m_reco_ndof,   "reco_ndof");
  book(m_reco_chi2Overndof, "reco_chi2Overndof");
  book(m_reco_author,  "reco_author");
  book(m_reco_time,    "reco_time");
  book(m_reco_hasValidTime_eff_vs_eta, "reco_hasValidTime_eff_vs_eta");

  book(m_truth_d0,     "truth_d0");
  book(m_truth_z0,     "truth_z0");
  book(m_truth_z0sin,  "truth_z0sin");
  book(m_truth_phi,    "truth_phi");
  book(m_truth_theta,  "truth_theta");
  book(m_truth_eta,    "truth_eta");
  book(m_truth_qoverp, "truth_qoverp");
  book(m_truth_pt,     "truth_pt");
  book(m_truth_lowpt,     "truth_lowpt");
  book(m_truth_prodR,  "truth_prodR");
  book(m_truth_prodZ,  "truth_prodZ");

  book(m_reco_pt_vs_eta, "reco_pt_vs_eta");
  book(m_reco_phi_vs_eta, "reco_phi_vs_eta");

  book(m_reco_d0_z0, "reco_d0_vs_z0");
  book(m_reco_d0_z0sin, "reco_d0_vs_z0sin");

  book(m_truth_pt_vs_eta, "truth_pt_vs_eta");
  book(m_truth_phi_vs_eta, "truth_phi_vs_eta");

}

void
InDetPerfPlot_TrackParameters::fill(const xAOD::TruthParticle& particle, float weight) {

  // quantities with xAOD::TruthParticle accessors:
  float eta = particle.eta();
  float pt = particle.pt() / Gaudi::Units::GeV;
  float phi = particle.phi();

  float d0 = (particle.isAvailable<float>("d0")) ? particle.auxdata<float>("d0") : -9999.;
  float z0 = (particle.isAvailable<float>("z0")) ? particle.auxdata<float>("z0") : -9999.;
  float theta = (particle.isAvailable<float>("theta")) ? particle.auxdata<float>("theta") : -9999.;
  float z0sin = (particle.isAvailable<float>("z0") && particle.auxdata<float>("theta")) ? z0 * std::sin(theta) : -9999.;
  float qOverP = (particle.isAvailable<float>("qOverP")) ? particle.auxdata<float>("qOverP") : -9999.;
  float prodR = (particle.isAvailable<float>("prodR")) ? particle.auxdata<float>("prodR") : -9999.;
  float prodZ = (particle.isAvailable<float>("prodZ")) ? particle.auxdata<float>("prodZ") : -9999.;

  if(d0 > -9000.) fillHisto(m_truth_d0, d0, weight);
  if(z0 > -9000.) fillHisto(m_truth_z0, z0, weight);
  if(theta > -9000.) fillHisto(m_truth_theta, theta, weight);
  if(phi > -9000.) fillHisto(m_truth_phi, phi, weight);
  if(z0sin > -9000.) fillHisto(m_truth_z0sin, z0sin, weight);
  if(qOverP > -9000.) fillHisto(m_truth_qoverp, qOverP, weight);
  if(prodR > -9000.) fillHisto(m_truth_prodR, prodR, weight);
  if(prodZ > -9000.) fillHisto(m_truth_prodZ, prodZ, weight);

  fillHisto(m_truth_eta, eta, weight);
  fillHisto(m_truth_pt, pt, weight);
  fillHisto(m_truth_lowpt, pt, weight);

  fillHisto(m_truth_pt_vs_eta, pt, eta, weight);
  fillHisto(m_truth_phi_vs_eta, phi, eta, weight);

}

void
InDetPerfPlot_TrackParameters::fill(const xAOD::TrackParticle& particle, float weight) {

  float pt = particle.pt() / Gaudi::Units::GeV;
  float eta = particle.eta();
  float phi = particle.phi0();

  float chi2 = particle.chiSquared();
  float ndof = particle.numberDoF();
  float chi2Overndof = ndof > 0 ? chi2 / ndof : 0;


  fillHisto(m_reco_d0, particle.d0(), weight);
  fillHisto(m_reco_z0, particle.z0(), weight);
  fillHisto(m_reco_z0sin, particle.z0()* std::sin(particle.theta()), weight);

  fillHisto(m_reco_phi, phi, weight);
  fillHisto(m_reco_theta, particle.theta(), weight);
  fillHisto(m_reco_eta, eta, weight);
  fillHisto(m_reco_qoverp, particle.qOverP(), weight);
  fillHisto(m_reco_pt, pt, weight);
  fillHisto(m_reco_lowpt, pt, weight);

  fillHisto(m_reco_pt_vs_eta, pt, eta, weight);
  fillHisto(m_reco_phi_vs_eta, phi, eta, weight);

  fillHisto(m_reco_d0_z0,    particle.d0(), particle.z0(), weight);
  fillHisto(m_reco_d0_z0sin, particle.d0(), particle.z0()*std::sin(particle.theta()), weight);

  fillHisto(m_reco_chi2, chi2, weight);
  fillHisto(m_reco_ndof, ndof, weight);
  fillHisto(m_reco_chi2Overndof, chi2Overndof, weight);

  std::bitset<xAOD::TrackPatternRecoInfo::NumberOfTrackRecoInfo>  patternInfo = particle.patternRecoInfo();
  for(unsigned int i = 0; i < xAOD::TrackPatternRecoInfo::NumberOfTrackRecoInfo; i++){
    if(patternInfo.test(i)) fillHisto(m_reco_author, i, weight);
  }

  static const SG::AuxElement::Accessor< uint8_t > accValidTime("hasValidTime");
  static const SG::AuxElement::Accessor< float > accTime("time");
  if( accValidTime.isAvailable(particle) && accTime.isAvailable(particle) ) {
    if (particle.hasValidTime()) {
      fillHisto(m_reco_time, particle.time(), weight);
    }
  }

  if( accValidTime.isAvailable(particle) ) {
    fillHisto(m_reco_hasValidTime_eff_vs_eta, eta, particle.hasValidTime(), weight);
  }

}

void
InDetPerfPlot_TrackParameters::finalizePlots() {

  //
  //Pie
  //

}
