///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// InSituCalibStep.cxx 
// Implementation file for class InSituCalibStep
/////////////////////////////////////////////////////////////////// 

#include "JetCalibTools/InSituCalibStep.h"
#include "PathResolver/PathResolver.h"
#include "AsgDataHandles/ReadDecorHandle.h"

InSituCalibStep::InSituCalibStep(const std::string& name)
  : asg::AsgTool( name ){ }



/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

StatusCode InSituCalibStep::initialize() {
  ATH_MSG_DEBUG ("Initializing " << name() );
  ATH_MSG_DEBUG("Reading from " << m_jetInScale << " and writing to " << m_jetOutScale);
  // Initialise ReadHandle(s)
  ATH_CHECK( m_evtInfoKey.initialize() );

  if ( m_isMC )
    ATH_MSG_WARNING("InSituCalibStep::calibrate : Running over MC, will not calibrate unless expert option CalibrateMC is set to true");

  ATH_CHECK( m_histTool_EtaInter.retrieve() ); 
  ATH_CHECK( m_histTool_Abs.retrieve() ); 
  
  if( m_histTool_Abs.size() != m_histTool_EtaInter.size() )
    return StatusCode::FAILURE;
  unsigned int nHist = m_histTool_Abs.size();
  
  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  // This multiplies the histogram and matches the behaviour in the old InsituDataCorrection step. 
  // This block of code should be removed entirely (along with the combineCalibration() function) once we have validated the simpler multiplication in getInsituCorr()
  for(unsigned int i = 0 ; i < nHist ; i++){
    JetHelper::HistoInputBase* histoTool_EtaInter = dynamic_cast<JetHelper::HistoInputBase*>( &(*m_histTool_EtaInter[i]) );
    JetHelper::HistoInputBase* histoTool_Abs = dynamic_cast<JetHelper::HistoInputBase*>(  &(*m_histTool_Abs[i]) );
    TH1D *h_Abs = dynamic_cast<TH1D*>( &histoTool_Abs->getHistogram());
    TH2D *h_EtaInter = dynamic_cast<TH2D*>( &histoTool_EtaInter->getHistogram());
    // combine relative and absolute calibration (central + eta-intercalibration)
    std::unique_ptr<const TH2> h_insituCorr = combineCalibration(h_EtaInter, h_Abs);
    m_insituCorr_vec.push_back(std::move(h_insituCorr) );
    // combined histogram boundaries
    double etaMax = m_insituCorr_vec[i] -> GetYaxis() -> GetBinUpEdge(m_insituCorr_vec[i] -> GetYaxis() -> GetLast());
    double etaMin = m_insituCorr_vec[i] -> GetYaxis() -> GetBinLowEdge(m_insituCorr_vec[i] -> GetYaxis() -> GetFirst());
    double ptMax = m_insituCorr_vec[i] -> GetXaxis() -> GetBinUpEdge(m_insituCorr_vec[i] -> GetXaxis() -> GetLast());
    double ptMin = m_insituCorr_vec[i] -> GetXaxis() -> GetBinLowEdge(m_insituCorr_vec[i] -> GetXaxis() -> GetFirst());
    m_etaMax_vec.push_back(etaMax);
    m_etaMin_vec.push_back(etaMin);
    m_ptMax_vec.push_back(ptMax);
    m_ptMin_vec.push_back(ptMin);
  }
  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

  return StatusCode::SUCCESS;
}


StatusCode InSituCalibStep::calibrate(xAOD::JetContainer& jets) const {

  ATH_MSG_DEBUG("calibrating jet collection.");
  // Retrieve EventInfo object, for time-dependent calibration and isMC flag
  unsigned int rNumber = 0;
  if (retrieveEventInfo(rNumber).isFailure())
    return StatusCode::FAILURE;
  if( (bool)(m_isMC) && !( (bool)(m_CalibrateMC)) ) //no calibration
    return StatusCode::SUCCESS;
  unsigned int runNumber = static_cast<unsigned int>(rNumber+0.5);
  // Pick up the correct time-dependent histogram
  unsigned int periodInd = 9999;
  for(unsigned int i=0; i<m_RunNumBoundaries.size()-1; i++){
    unsigned int firstRun = m_RunNumBoundaries[i] + 1.5;
    unsigned int lastRun = m_RunNumBoundaries[i+1]+0.5;
    if (firstRun<=runNumber && runNumber <= lastRun ){
      periodInd = i;
      break;
    }
  }

  if (periodInd==9999){ // periodInd could not be set
    ATH_MSG_WARNING("No calibration found for run number "<<runNumber);
  }

  if( periodInd >= m_histTool_Abs.size() ) //outside run numbers, return no calibration
    return StatusCode::SUCCESS; 
  
  JetHelper::JetContext jc;
  for (xAOD::Jet* jet : jets){
    const xAOD::JetFourMom_t jetStartP4 = jet->getAttribute<xAOD::JetFourMom_t>(m_jetInScale);
    jet->setJetP4(jetStartP4);
    double s = 1.0; // scale
    if (getInsituCorr(*jet, jc, periodInd, s).isFailure())
        return StatusCode::FAILURE;
    xAOD::JetFourMom_t calibP4=jet->jetP4();
    calibP4 = calibP4 * s;
    // Set the output scale
    jet->setAttribute<xAOD::JetFourMom_t>(m_jetOutScale,calibP4);
    jet->setJetP4(calibP4);
  }  
  return StatusCode::SUCCESS;
}
StatusCode InSituCalibStep::retrieveEventInfo(unsigned int &r) const {
  
  const xAOD::EventInfo * eventObj = nullptr;
  static std::atomic<unsigned int> eventInfoWarnings = 0;
  SG::ReadHandle<xAOD::EventInfo> rhEvtInfo(m_evtInfoKey);
  if ( rhEvtInfo.isValid() ) {
    eventObj = rhEvtInfo.cptr();
    r = eventObj->runNumber();
  } else {
    ++eventInfoWarnings;
    if ( eventInfoWarnings < 20 )
      ATH_MSG_ERROR("   InSituCalibStep::calibrate : Failed to retrieve event information.");
    return StatusCode::SUCCESS; //error is recoverable, so return SUCCESS
  }
  return StatusCode::SUCCESS;
}

StatusCode InSituCalibStep::getInsituCorr(const xAOD::Jet& jet,  JetHelper::JetContext& jc, unsigned int periodIndex, double &scale) const{

  scale = 1.0 ;

  // Multiply histograms on-the-fly - this will not perfectly match the old InsituDataCorrection step, but is simpler and arguably more correct.
  // Note that JetToolHelpers handles the protection against values outside the histogram range and the interpolation.
  const double R_abs = m_histTool_Abs[periodIndex]->getValue(jet,jc);
  double c_rel = m_histTool_EtaInter[periodIndex]->getValue(jet,jc);
  scale = c_rel/R_abs;

  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  // This uses the histogram created in initialize, which matches the behaviour in the old InsituDataCorrection step. 
  // However this does not make best use of the available JetToolHelpers functionality.
  // This block of code can be removed once we have validated the simpler on-the-fly multiplication above.
  double v1 = m_vartool1->getValue(jet,jc);
  double v2 = m_vartool2->getValue(jet,jc);
  //protection against values outside the histogram range - snap back to the lowest/highest bin edge
  if ( v1 <= m_ptMin_vec[periodIndex] ) v1 = m_ptMin_vec[periodIndex] + 1e-6;
  else if ( v1 >= m_ptMax_vec[periodIndex] ) v1 = m_ptMax_vec[periodIndex] - 1e-6;

  if (v2 <= m_etaMin_vec[periodIndex]) v2 = 1e-6 + m_etaMin_vec[periodIndex];
  else if (v2 >= m_etaMax_vec[periodIndex]) v2 = m_etaMax_vec[periodIndex] - 1e-6;

  double scale_OG = m_insituCorr_vec[periodIndex] -> Interpolate(v1,v2);

  double scale_diff_perc = 100*abs(scale_OG - scale)/scale_OG;
  if (scale_diff_perc > 0.01){ ATH_MSG_WARNING("Simplified InsituScale calculation differs by "<<scale_diff_perc<<"% --> original scale = "<<scale_OG<<", simplified scale = "<<scale);}

  // m_useOriginalHistCombination=false by default, so by default this value will not be used.
  if (m_useOriginalHistCombination){ scale = scale_OG;}
  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

  return StatusCode::SUCCESS;
}


std::unique_ptr<const TH2> InSituCalibStep::combineCalibration(const TH2* h2d, const TH1* h) {
  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  // This function can be removed once we have validated the simpler on-the-fly multiplication in getInsituCorr()
  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  std::unique_ptr<TH2> prod(static_cast<TH2*>( h2d->Clone()  ) );
  for (int xi=1;xi<=prod->GetNbinsX();xi++) {
    double pt=prod->GetXaxis()->GetBinCenter(xi);
    const double R_abs=h->Interpolate(pt); // Rdata/RMC for the absolute scale
    const double inv_R_abs = 1. / R_abs;
    //printf("pT = %7.1f GeV, abs calib: %.4f\n",pt,abs);
    for (int yi=1;yi<=prod->GetNbinsY();yi++) {
      double c_rel = h2d->GetBinContent(xi,yi); // 1/Rrel = RMC/Rdata
      prod->SetBinContent(xi,yi,c_rel*inv_R_abs);
    }
  }
  return prod;
}


