/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetPerfPlot_TRTExtension.cxx
 * @author shaun roe
 **/


#include "InDetPhysValMonitoringUtilities.h"

#include "InDetPerfPlot_TRTExtension.h"
#include "AthContainers/ConstAccessor.h"

#include <cmath>

using namespace IDPVM;

InDetPerfPlot_TRTExtension::InDetPerfPlot_TRTExtension(InDetPlotBase* pParent, const std::string& sDir) :
  InDetPlotBase(pParent, sDir),
  m_resolutionMethod(IDPVM::ResolutionHelper::iterRMS_convergence),
  m_fracTRTExtensions_vs_eta{},
  m_fracTRTExtensions_vs_pt{},
  m_fracTRTExtensions_vs_mu{},
  m_fracTRTExtensions_vs_nvertices{},
  m_fracTRTExtensions_matched_vs_eta{},
  m_fracTRTExtensions_matched_vs_pt{},
  m_fracFindableTRTExtensions_vs_eta{},
  m_fracFindableTRTExtensions_vs_pt{},
  m_fracFindableTRTExtensions_vs_mu{},
  m_fracFindableTRTExtensions_vs_nvertices{},
  m_fracFindableTRTExtensions_matched_vs_eta{},
  m_fracFindableTRTExtensions_matched_vs_pt{},
  m_chi2ndofTRTExtensions{},
  m_chi2ndofNoTRTExtensions{},
  m_ptresTRTExtensions_vs_eta{},
  m_ptresTRTExtensions_vs_pt{},
  m_ptresNoTRTExtensions_vs_eta{},
  m_ptresNoTRTExtensions_vs_pt{},
  m_reswidthTRTExtensions_vs_eta{},
  m_resmeanTRTExtensions_vs_eta{},
  m_reswidthTRTExtensions_vs_pt{},
  m_resmeanTRTExtensions_vs_pt{},
  m_reswidthNoTRTExtensions_vs_eta{},
  m_resmeanNoTRTExtensions_vs_eta{},
  m_reswidthNoTRTExtensions_vs_pt{},
  m_resmeanNoTRTExtensions_vs_pt{},
  m_ptpullTRTExtensions_vs_eta{},
  m_ptpullTRTExtensions_vs_pt{},
  m_ptpullNoTRTExtensions_vs_eta{},
  m_ptpullNoTRTExtensions_vs_pt{},
  m_pullwidthTRTExtensions_vs_eta{},
  m_pullmeanTRTExtensions_vs_eta{},
  m_pullwidthTRTExtensions_vs_pt{},
  m_pullmeanTRTExtensions_vs_pt{},
  m_pullwidthNoTRTExtensions_vs_eta{},
  m_pullmeanNoTRTExtensions_vs_eta{},
  m_pullwidthNoTRTExtensions_vs_pt{},
  m_pullmeanNoTRTExtensions_vs_pt{}


  {
}

void
InDetPerfPlot_TRTExtension::initializePlots() {

  book(m_fracTRTExtensions_vs_eta, "fracTRTExtensions_vs_eta");
  book(m_fracTRTExtensions_vs_pt, "fracTRTExtensions_vs_pt");
  book(m_fracTRTExtensions_vs_mu, "fracTRTExtensions_vs_mu");
  book(m_fracTRTExtensions_vs_nvertices, "fracTRTExtensions_vs_nvertices");

  book(m_fracTRTExtensions_matched_vs_eta, "fracTRTExtensions_matched_vs_eta");
  book(m_fracTRTExtensions_matched_vs_pt, "fracTRTExtensions_matched_vs_pt");

  book(m_fracFindableTRTExtensions_vs_eta, "fracFindableTRTExtensions_vs_eta");
  book(m_fracFindableTRTExtensions_vs_pt, "fracFindableTRTExtensions_vs_pt");
  book(m_fracFindableTRTExtensions_vs_mu, "fracFindableTRTExtensions_vs_mu");
  book(m_fracFindableTRTExtensions_vs_nvertices, "fracFindableTRTExtensions_vs_nvertices");

  book(m_fracFindableTRTExtensions_matched_vs_eta, "fracFindableTRTExtensions_matched_vs_eta");
  book(m_fracFindableTRTExtensions_matched_vs_pt, "fracFindableTRTExtensions_matched_vs_pt");

  book(m_chi2ndofTRTExtensions, "chi2ndofTRTExtensions");
  book(m_chi2ndofNoTRTExtensions, "chi2ndofNoTRTExtensions");

  book(m_ptresTRTExtensions_vs_eta, "resHelper_eta_pt_TRTExtension");
  book(m_ptresTRTExtensions_vs_pt, "resHelper_pt_pt_TRTExtension");
  book(m_ptresNoTRTExtensions_vs_eta, "resHelper_eta_pt_NoTRTExtension");
  book(m_ptresNoTRTExtensions_vs_pt, "resHelper_pt_pt_NoTRTExtension");

  book(m_reswidthTRTExtensions_vs_eta, "resolution_vs_eta_pt_TRTExtension");
  book(m_resmeanTRTExtensions_vs_eta, "resmean_vs_eta_pt_TRTExtension");
  book(m_reswidthTRTExtensions_vs_pt, "resolution_vs_pt_pt_TRTExtension");
  book(m_resmeanTRTExtensions_vs_pt, "resmean_vs_pt_pt_TRTExtension");

  book(m_reswidthNoTRTExtensions_vs_eta, "resolution_vs_eta_pt_NoTRTExtension");
  book(m_resmeanNoTRTExtensions_vs_eta, "resmean_vs_eta_pt_NoTRTExtension");
  book(m_reswidthNoTRTExtensions_vs_pt, "resolution_vs_pt_pt_NoTRTExtension");
  book(m_resmeanNoTRTExtensions_vs_pt, "resmean_vs_pt_pt_NoTRTExtension");


  book(m_ptpullTRTExtensions_vs_eta, "pullHelper_eta_pt_TRTExtension");
  book(m_ptpullTRTExtensions_vs_pt, "pullHelper_pt_pt_TRTExtension");
  book(m_ptpullNoTRTExtensions_vs_eta, "pullHelper_eta_pt_NoTRTExtension");
  book(m_ptpullNoTRTExtensions_vs_pt, "pullHelper_pt_pt_NoTRTExtension");

  book(m_pullwidthTRTExtensions_vs_eta, "pullwidth_vs_eta_pt_TRTExtension");
  book(m_pullmeanTRTExtensions_vs_eta, "pullmean_vs_eta_pt_TRTExtension");
  book(m_pullwidthTRTExtensions_vs_pt, "pullwidth_vs_pt_pt_TRTExtension");
  book(m_pullmeanTRTExtensions_vs_pt, "pullmean_vs_pt_pt_TRTExtension");

  book(m_pullwidthNoTRTExtensions_vs_eta, "pullwidth_vs_eta_pt_NoTRTExtension");
  book(m_pullmeanNoTRTExtensions_vs_eta, "pullmean_vs_eta_pt_NoTRTExtension");
  book(m_pullwidthNoTRTExtensions_vs_pt, "pullwidth_vs_pt_pt_NoTRTExtension");
  book(m_pullmeanNoTRTExtensions_vs_pt, "pullmean_vs_pt_pt_NoTRTExtension");

}

void
InDetPerfPlot_TRTExtension::fill(const xAOD::TrackParticle& particle, float weight) {

  double eta = particle.eta();
  double pt = particle.pt() / Gaudi::Units::GeV;
  float chi2 = particle.chiSquared();
  float ndof = particle.numberDoF();
  float chi2Overndof = ndof > 0 ? chi2 / ndof : 0;

  uint8_t iTrtHits = 0;
  uint8_t iTrtOutliers = 0;
  particle.summaryValue(iTrtHits, xAOD::numberOfTRTHits);
  particle.summaryValue(iTrtOutliers, xAOD::numberOfTRTOutliers);

  bool hasTRTHits = iTrtHits + iTrtOutliers > 0;
  // failed extensions will have only outlier hits and iTRThits will be 0
  bool isTRTExtension = iTrtHits > 0;

  fillHisto(m_fracTRTExtensions_vs_eta, eta, isTRTExtension, weight);
  fillHisto(m_fracTRTExtensions_vs_pt, pt, isTRTExtension, weight);
  if (hasTRTHits) {
    fillHisto(m_fracFindableTRTExtensions_vs_eta, eta, isTRTExtension, weight);
    fillHisto(m_fracFindableTRTExtensions_vs_pt, pt, isTRTExtension, weight);
  }

  if(isTRTExtension) fillHisto(m_chi2ndofTRTExtensions, chi2Overndof, weight);
  else { fillHisto(m_chi2ndofNoTRTExtensions, chi2Overndof, weight); }

}

void
InDetPerfPlot_TRTExtension::fill(const xAOD::TrackParticle& particle, const float mu, const unsigned int nvertices, float weight) {

  uint8_t iTrtHits = 0;
  uint8_t iTrtOutliers = 0;
  particle.summaryValue(iTrtHits, xAOD::numberOfTRTHits);
  particle.summaryValue(iTrtOutliers, xAOD::numberOfTRTOutliers);

  bool hasTRTHits = iTrtHits + iTrtOutliers > 0;
  // failed extensions will have only outlier hits and iTRThits will be 0
  bool isTRTExtension = iTrtHits > 0;

  fillHisto(m_fracTRTExtensions_vs_mu, mu, isTRTExtension, weight);
  fillHisto(m_fracTRTExtensions_vs_nvertices, nvertices, isTRTExtension, weight);
  if (hasTRTHits) {
    fillHisto(m_fracFindableTRTExtensions_vs_mu, mu, isTRTExtension, weight);
    fillHisto(m_fracFindableTRTExtensions_vs_nvertices, nvertices, isTRTExtension, weight);
  }

}

void
InDetPerfPlot_TRTExtension::fill(const xAOD::TrackParticle& particle, const xAOD::TruthParticle& truthParticle, float weight) {

  //Fraction of extended for truth matched tracks

  uint8_t iTrtHits = 0;
  uint8_t iTrtOutliers = 0;
  particle.summaryValue(iTrtHits, xAOD::numberOfTRTHits);
  particle.summaryValue(iTrtOutliers, xAOD::numberOfTRTOutliers);

  bool hasTRTHits = iTrtHits + iTrtOutliers > 0;
  // failed extensions will have only outlier hits and iTRThits will be 0
  bool isTRTExtension = iTrtHits > 0;
  
  //Get pT resolution for TRT extensions versus without
  const float undefinedValue = -9999;
  const float smallestAllowableTan = 1e-8; 
  const float sinTheta{std::sin(particle.theta())};
  const bool saneSineValue = (std::abs(sinTheta) > 1e-8);
  const float inverseSinTheta = saneSineValue ? 1./sinTheta : undefinedValue;
  float track_qopt = saneSineValue ? particle.qOverP()*inverseSinTheta : undefinedValue;
  const float qopterr = std::sqrt(particle.definingParametersCovMatrix()(4, 4)) * inverseSinTheta;

  static const SG::ConstAccessor<float> qOverPAcc("qOverP");
  static const SG::ConstAccessor<float> thetaAcc("theta");
  const float truth_qop = qOverPAcc.isAvailable(truthParticle) ? qOverPAcc(truthParticle) : undefinedValue;
  const float truth_theta = thetaAcc.isAvailable(truthParticle) ? thetaAcc(truthParticle) : undefinedValue;
  float truth_qopt = std::abs(truth_theta) > 0 ? truth_qop * 1/(std::sin(truth_theta)) : undefinedValue;

  float ptres = (track_qopt - truth_qopt) * ( 1 / truth_qopt);
  float ptpull = qopterr > smallestAllowableTan ? (track_qopt - truth_qopt) / qopterr : undefinedValue;
  float pt = truthParticle.pt() / Gaudi::Units::GeV;
  const float tanHalfTheta = std::tan(truth_theta * 0.5);
  const bool tanThetaIsSane = std::abs(tanHalfTheta) > smallestAllowableTan;
  float eta = undefinedValue;
  if (tanThetaIsSane) eta = -std::log(tanHalfTheta);


  if(isTRTExtension){
    fillHisto(m_ptresTRTExtensions_vs_eta, eta, ptres, weight);
    fillHisto(m_ptresTRTExtensions_vs_pt, pt, ptres, weight);

    fillHisto(m_ptpullTRTExtensions_vs_eta, eta, ptpull, weight);
    fillHisto(m_ptpullTRTExtensions_vs_pt, pt, ptpull, weight);
  } else {
    fillHisto(m_ptresNoTRTExtensions_vs_eta, eta, ptres, weight);
    fillHisto(m_ptresNoTRTExtensions_vs_pt, pt, ptres, weight);

    fillHisto(m_ptpullNoTRTExtensions_vs_eta, eta, ptpull, weight);
    fillHisto(m_ptpullNoTRTExtensions_vs_pt, pt, ptpull, weight);
  } 

  fillHisto(m_fracTRTExtensions_matched_vs_eta, eta, isTRTExtension, weight);
  fillHisto(m_fracTRTExtensions_matched_vs_pt, pt, isTRTExtension, weight);
  if (hasTRTHits) {
    fillHisto(m_fracFindableTRTExtensions_matched_vs_eta, eta, isTRTExtension, weight);
    fillHisto(m_fracFindableTRTExtensions_matched_vs_pt, pt, isTRTExtension, weight);
  }

}


void
InDetPerfPlot_TRTExtension::finalizePlots() {

  m_resolutionHelper.makeResolutions(m_ptresTRTExtensions_vs_eta, m_reswidthTRTExtensions_vs_eta, m_resmeanTRTExtensions_vs_eta, m_resolutionMethod); 
  m_resolutionHelper.makeResolutions(m_ptresTRTExtensions_vs_pt, m_reswidthTRTExtensions_vs_pt, m_resmeanTRTExtensions_vs_pt, m_resolutionMethod);
  m_resolutionHelper.makeResolutions(m_ptresNoTRTExtensions_vs_eta, m_reswidthNoTRTExtensions_vs_eta, m_resmeanNoTRTExtensions_vs_eta, m_resolutionMethod);
  m_resolutionHelper.makeResolutions(m_ptresNoTRTExtensions_vs_pt, m_reswidthNoTRTExtensions_vs_pt, m_resmeanNoTRTExtensions_vs_pt, m_resolutionMethod);


  m_resolutionHelper.makeResolutions(m_ptpullTRTExtensions_vs_eta, m_pullwidthTRTExtensions_vs_eta, m_pullmeanTRTExtensions_vs_eta, m_resolutionMethod); 
  m_resolutionHelper.makeResolutions(m_ptpullTRTExtensions_vs_pt, m_pullwidthTRTExtensions_vs_pt, m_pullmeanTRTExtensions_vs_pt, m_resolutionMethod);
  m_resolutionHelper.makeResolutions(m_ptpullNoTRTExtensions_vs_eta, m_pullwidthNoTRTExtensions_vs_eta, m_pullmeanNoTRTExtensions_vs_eta, m_resolutionMethod);
  m_resolutionHelper.makeResolutions(m_ptpullNoTRTExtensions_vs_pt, m_pullwidthNoTRTExtensions_vs_pt, m_pullmeanNoTRTExtensions_vs_pt, m_resolutionMethod);

}
