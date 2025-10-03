/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***********************************************************************
//                            eFEXFillEDM - description
//                              ---------------------
//      begin                 : 22 04 2021
//      email                 : nluongo@uoregon.edu
//***********************************************************************

#include "eFEXFillEDM.h"

namespace LVL1 {

  // default constructor for persistency
  
  eFEXFillEDM::eFEXFillEDM(const std::string& type, const std::string& name, const IInterface* parent):
    AthAlgTool(type, name, parent) 
  {
    declareInterface<eFEXFillEDM>(this);
  }

  //----------------- Initialization ----------------------------

  StatusCode eFEXFillEDM::initialize()
  {
    return StatusCode::SUCCESS;
  }

  //----------------- Execute ----------------------------

  StatusCode eFEXFillEDM::execute()
  {
    return StatusCode::SUCCESS;
  }

  //----------------- Finalisation ------------------------------

  StatusCode eFEXFillEDM::finalize()
  {
    return StatusCode::SUCCESS;
  }
  
  void eFEXFillEDM::fillEmEDM(std::unique_ptr<xAOD::eFexEMRoIContainer> &container, uint8_t eFexNum, const std::unique_ptr<eFEXegTOB>& tobObject, bool xTOB) const
  {   
    // Create the object and fill it
    xAOD::eFexEMRoI* myEmEDM = new xAOD::eFexEMRoI();    
    container->push_back(myEmEDM);
    
    // Initialise either xTOB or TOB object as requested
    if (xTOB) 
      myEmEDM->initialize(tobObject->getxTobword0(), tobObject->getxTobword1()); 
    else {
      // For TOB we must translate eFEX index into Shelf+eFEX and add these when initialising
      uint8_t shelf = int(eFexNum/12);
      uint8_t eFEX  = eFexNum%12;
      myEmEDM->initialize(eFEX, shelf, tobObject->getTobword());       
    }

    // Supplementary information is the same either way
    myEmEDM->setRetaCore(tobObject->getRetaCore());
    myEmEDM->setRetaEnv(tobObject->getRetaEnv());
    myEmEDM->setRhadEM(tobObject->getRhadEM());
    myEmEDM->setRhadHad(tobObject->getRhadHad());
    myEmEDM->setWstotNumerator(tobObject->getWstotNum());
    myEmEDM->setWstotDenominator(tobObject->getWstotDen());

    ATH_MSG_DEBUG(" setting Type: " << myEmEDM->type() << " eFEX Number:  " << +myEmEDM->eFexNumber() << " shelf: " << +myEmEDM->shelfNumber() << " et: " << myEmEDM->et() << " MeV, " << myEmEDM->etTOB() << " TOB, " << myEmEDM->etXTOB() << " xTOB, eta: " << myEmEDM->eta() <<  " phi: " << myEmEDM->phi() << " input eFexNum: " << +eFexNum << " TOB word: " << tobObject->getTobword() << MSG::dec );

  }

  void eFEXFillEDM::fillTauEDM(std::unique_ptr<xAOD::eFexTauRoIContainer> &container, uint8_t eFexNum, const std::unique_ptr<eFEXtauTOB>& tobObject, bool xTOB) const
  {
    // Create the object and fill it:
    xAOD::eFexTauRoI* myTauEDM = new xAOD::eFexTauRoI();
    container->push_back(myTauEDM);

    // Initialise either xTOB or TOB object as requested
    if (xTOB) 
      myTauEDM->initialize(tobObject->getxTobword0(), tobObject->getxTobword1()); 
    else {
      // For TOB we must translate eFEX index into Shelf+eFEX and add these when initialising
      uint8_t shelf = int(eFexNum/12);
      uint8_t eFEX  = eFexNum%12;
      myTauEDM->initialize(eFEX, shelf, tobObject->getTobword());      
    }

    // There is some ambiguity in what 'numerator'/'denominator' mean in each of the tau isolation 
    // variables rCore and rHad below, since in the more 'physical' definition, we would consider core/(core+env),
    // whereas in the firmware we calculate core/env. Here, I store core->numerator, env->denominator. 
    // Provided we remember this when using them, we can then calculate either the 'physical' or the 'firmware' values.
    myTauEDM->setRCoreNumerator(tobObject->getRcoreCore());
    myTauEDM->setRCoreDenominator(tobObject->getRcoreEnv());
    myTauEDM->setRHadNumerator(tobObject->getRhadCore());
    myTauEDM->setRHadDenominator(tobObject->getRhadEnv());

    ATH_MSG_DEBUG("setting tau version " << myTauEDM->tobVersion() << " eFEX Number: " << +myTauEDM->eFexNumber() << " shelf: " << +myTauEDM->shelfNumber() << " et: " << myTauEDM->et() << " eta: " << myTauEDM->eta() << " phi: " << myTauEDM->phi() << " input eFexNum: " << +eFexNum << " TOB word: " << tobObject->getTobword() <<" xTOB word 1: "<< tobObject->getxTobword0() << " xTOB word 2: " << tobObject->getxTobword1() << " BDT score: " << tobObject->getBDTScore() << " BDT score from EDM " << myTauEDM->bdtScore() << MSG::dec);

  }

} // end of namespace bracket

