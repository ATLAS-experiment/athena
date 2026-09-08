/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AsgDataHandles/ReadDecorHandle.h"

#include "JetCalibTools/IJetCalibrationTool.h"
#include "JetCalibTools/PileupCalibStep.h"
#include "JetCalibTools/JetCalibUtils.h"

#include "xAODJet/JetAccessors.h"

#include "PathResolver/PathResolver.h"

#include <memory>
#include <utility>

PileupCalibStep::PileupCalibStep(const std::string& name)
  : asg::AsgTool( name ) { }

StatusCode PileupCalibStep::initialize() {

  ATH_MSG_DEBUG("Reading from " << m_jetInScale << " and writing to " << m_jetOutScale);

  ATH_CHECK( m_muKey.initialize() );
  ATH_CHECK( m_pvKey.initialize() );
  ATH_CHECK( m_rhoKey.initialize() );

  if(m_doJetArea)  ATH_MSG_DEBUG("Jet area pile up correction will be applied.");
  if(m_doResidual) ATH_MSG_DEBUG("Residual pile up correction will be applied.");
  
  if(m_doMuOnly)   ATH_MSG_INFO("Only the pileup mu-based calibration will be applied.");
  if(m_doNPVOnly)  ATH_MSG_INFO("Only the pileup NPV-based calibration will be applied.");

  // Protections
  CHECK_THEN_ERROR(!m_doJetArea && !m_doResidual, "No correction requested to be applied, must be a misconfiguration!");
  CHECK_THEN_ERROR(m_doMuOnly && m_doNPVOnly,
		   "It was requested to apply only the mu-based AND the NPV-based calibrations.");

  CHECK_THEN_ERROR((m_mu_ref==-99 && !m_doNPVOnly), "OffsetCorrection.DefaultMuRef not specified.");

  CHECK_THEN_ERROR( (m_NPV_ref==-99 && !m_doMuOnly), "OffsetCorrection.DefaultNPVRef not specified.");

  return StatusCode::SUCCESS;
}

StatusCode PileupCalibStep::calibrate(xAOD::JetContainer& jetCont) const {

  ATH_MSG_DEBUG("Applying the pile up correction");

  double rho=0;
  if(m_doJetArea){
    SG::ReadHandle<xAOD::EventShape> eventShape(m_rhoKey);
    CHECK_THEN_ERROR(!eventShape.isValid() , "Could not retrieve xAOD::EventShape : "<< m_rhoKey.key());
    CHECK_THEN_ERROR(!eventShape->getDensity(xAOD::EventShape::Density, rho ),
		     "Could not retrieve xAOD::EventShape::Density from xAOD::EventShape "<< m_rhoKey.key() );
    ATH_MSG_DEBUG("  Rho = " << 0.001*rho << " GeV");
  }

  double muCorr=0;
  int NPV=0;
  if(m_doResidual){
    SG::ReadDecorHandle<xAOD::EventInfo,float> eventInfoDecor(m_muKey);
    CHECK_THEN_ERROR( ! eventInfoDecor.isPresent() , "EventInfo decoration not available! "<< m_muKey.key() );
    double mu = eventInfoDecor(0);
    //mu rescaling
    muCorr = m_isData ? mu : mu*m_muSF;
    SG::ReadHandle<xAOD::VertexContainer> PVCont(m_pvKey);
    CHECK_THEN_ERROR( ! PVCont.isValid() , "No Primary Vertices "<< m_pvKey.key() );
    NPV = JetCalibUtils::countNPV(*PVCont);
  }

  const xAOD::JetAttributeAccessor::AccessorWrapper<xAOD::JetFourMom_t> areaAcc("ActiveArea4vec");
  const xAOD::JetAttributeAccessor::AccessorWrapper<xAOD::JetFourMom_t> startScaleMomAcc(m_jetInScale);
  const xAOD::JetAttributeAccessor::AccessorWrapper<xAOD::JetFourMom_t> outScaleMomAcc(m_jetOutScale);  
  const xAOD::JetAttributeAccessor::AccessorWrapper<xAOD::JetFourMom_t> outAreaScaleMomAcc(m_jetAreaOutScale);

  JetHelper::JetContext jc;

  for( xAOD::Jet * jet : jetCont){

    xAOD::JetFourMom_t jetStartP4 = startScaleMomAcc.getAttribute(*jet);

    const double E_det = jetStartP4.e();
    const double pT_det = jetStartP4.pt();
    const double mass_det = jetStartP4.mass();

    if ( E_det < mass_det ) {
      ATH_MSG_ERROR("PileupCalibStep: jet mass (" << mass_det << " MeV) is greater than E (" << E_det << " MeV!) Aborting.");
      return StatusCode::FAILURE;
    }

    double pT_offset = pT_det; // pT difference before/after pileup corrections
    double pileup_SF = 1; // final calibration factor applied to the four vector

    xAOD::JetFourMom_t calibP4;
    
    if(m_doJetArea){
      xAOD::JetFourMom_t jetareaP4 = areaAcc.getAttribute(*jet);
      ATH_MSG_VERBOSE("    Area = " << jetareaP4);

      xAOD::JetFourMom_t rhoAreaP4;
      pT_offset = pT_det - rho*jetareaP4.pt();
      rhoAreaP4 = jetStartP4*pT_offset/pT_det;
      outAreaScaleMomAcc.setAttribute(*jet, rhoAreaP4);
    }

    if(m_doResidual){
      double alpha = 0.0, beta = 0.0;

      if(!m_doNPVOnly){
	alpha = m_histTool_mu->getValue(*jet, jc);
      }
      if(!m_doMuOnly){
	beta = m_histTool_NPV->getValue(*jet, jc);
      }

      double offsetET = (alpha*(muCorr-m_mu_ref) + beta*(NPV-m_NPV_ref))*m_GeV;
      pT_offset = pT_offset - offsetET;
    }
    
    // Set the jet pT to 10 MeV if the pT is negative after the jet area and residual offset corrections
    pileup_SF = pT_offset >= 0 ? pT_offset / pT_det : 10./pT_det;

    calibP4 = jetStartP4*pileup_SF;

    //Transfer calibrated jet properties to the jet
    outScaleMomAcc.setAttribute(*jet, calibP4 );
    jet->setJetP4( calibP4 );        
  }

  return StatusCode::SUCCESS;
}
