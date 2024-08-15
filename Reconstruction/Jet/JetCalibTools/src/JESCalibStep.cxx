///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EtaMassJESCalibStep.cxx 
// Implementation file for class EtaMassJESCalibStep
// Author: Max Swiatlowski <mswiatlo@cern.ch>
/////////////////////////////////////////////////////////////////// 

#include "JetCalibTools/JESCalibStep.h"

EtaMassJESCalibStep::EtaMassJESCalibStep(const std::string& name)
  : asg::AsgTool( name ){ }


StatusCode EtaMassJESCalibStep::initialize(){
    ATH_MSG_DEBUG ("Initializing " << name() );

    ATH_CHECK( m_textTool_JES.retrieve() ); 
    ATH_CHECK( m_textTool_Eta.retrieve() );
    ATH_CHECK( m_textTool_EmaxJES.retrieve() );

    return StatusCode::SUCCESS;
}

StatusCode EtaMassJESCalibStep::calibrate(xAOD::JetContainer& jets) const {
    ATH_MSG_DEBUG("Calibrating jet collection.");


    // Change this to EtaMassJES? Or only if Mass is applied?
    const xAOD::JetAttributeAccessor::AccessorWrapper<xAOD::JetFourMom_t> jesScaleMomAcc("JetEtaJESScaleMomentum"); 

    for(const auto jet: jets){

        const xAOD::JetFourMom_t jetStartP4 = jet->jetP4();

        // Extract the maximum energy, and store in the context
        JetHelper::JetContext jc;
        jc.setValue("Emax", m_textTool_EmaxJES->getValue(*jet, jc));

        // Extract JES from the text handling tool
        // TODO: Check that textTool uses DetectorEta properly
        float jesCorrection = m_textTool_JES->getValue(*jet, jc);
        ATH_MSG_INFO(jesCorrection);

        ATH_MSG_INFO("above is JES, below is eta");

        xAOD::JetFourMom_t calibP4 = jetStartP4 * jesCorrection;


        const float etaCorr = calibP4.eta() + m_textTool_Eta->getValue(*jet, jc);
        ATH_MSG_INFO(etaCorr);

        // Apply the eta correction, use TLV from ROOT to do some math for us
        TLorentzVector TLVjet;
        TLVjet.SetPtEtaPhiM( calibP4.P()/cosh(etaCorr), etaCorr, calibP4.phi(), calibP4.M() );
        calibP4.SetPxPyPzE( TLVjet.Px(), TLVjet.Py(), TLVjet.Pz(), TLVjet.E() );

        // Set the decorations of this scale
        jesScaleMomAcc.setAttribute(*jet, calibP4);
        jet->setJetP4(calibP4);
    }

    return StatusCode::SUCCESS;   
}