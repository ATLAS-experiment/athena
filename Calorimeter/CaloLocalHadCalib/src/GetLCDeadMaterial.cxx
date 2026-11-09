/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------
// File and Version Information:
// $Id: GetLCDeadMaterial.cxx,v 1.1 2009-03-03 17:30:23 pospelov Exp $
//
// Description: see GetLCDeadMaterial.h
// 
// Environment:
//      Software developed for the ATLAS Detector at CERN LHC
//
// Author List:
//      Gennady Pospelov
//
//-----------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "CaloLocalHadCalib/GetLCDeadMaterial.h"

//---------------
// C++ Headers --
//---------------
#include "PathResolver/PathResolver.h"
#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/StatusCode.h"

#include "CaloConditions/CaloLocalHadCoeff.h"
#include "CaloLocalHadCalib/CaloLocalHadCoeffHelper.h"
#include "CaloLocalHadCalib/CaloHadDMCoeffData.h"
#include "CaloLocalHadCalib/CaloHadDMCoeffFit.h"
#include "CaloLocalHadCalib/CaloHadDMCoeffMinim.h"
#include "CaloLocalHadCalib/CaloHadDMCoeffCheck.h"

#include "TTree.h"
#include <memory>


//###############################################################################
GetLCDeadMaterial::GetLCDeadMaterial(const std::string& name, 
                           ISvcLocator* pSvcLocator) 
  : AthAlgorithm(name, pSvcLocator)
{
}



/* ****************************************************************************

***************************************************************************** */
GetLCDeadMaterial::~GetLCDeadMaterial()
{

}



/* ****************************************************************************

***************************************************************************** */
StatusCode GetLCDeadMaterial::initialize()
{
  //std::cout << std::endl;
  //std::cout << std::endl;
  //std::cout << "--- GetLCDeadMaterial::initialize() --- " << std::endl;

  /* ********************************************
  default coefficients
  ******************************************** */
  CaloLocalHadCoeffHelper dmHelper;
  std::string fileName = PathResolver::find_file (m_HadDMCoeffInputFile, "DATAPATH");
  std::unique_ptr<CaloLocalHadCoeff> initHadDMCoeff (dmHelper.InitDataFromFile(fileName.c_str()));
  if( !initHadDMCoeff ) {
    ATH_MSG_FATAL( " Error while initializing default dead material coefficients " );
    return StatusCode::FAILURE;
  }
  //dmHelper.PrintData(initHadDMCoeff, std::cout);
  ATH_MSG_INFO( "  Number of dead material areas defined:" << initHadDMCoeff->getSizeAreaSet()
      << ", total number of correction coefficients:" << initHadDMCoeff->getSizeCoeffSet() );

  /* ********************************************
  input chain
  ******************************************** */
  TChain *pChain = new TChain("DeadMaterialTree");
  if( m_inputRootFiles.value().empty() ) {
    ATH_MSG_FATAL( " Empty vector of input root files! "  );
    return StatusCode::FAILURE;
  }
  for (const std::string& fname : m_inputRootFiles.value()) {
    ATH_MSG_INFO( " Adding root file '" <<fname << "'"  );
    pChain->Add( fname.c_str() );
  }

  // pointer to the chain and data in it
  std::unique_ptr<CaloHadDMCoeffData> dmData (new CaloHadDMCoeffData(pChain));
  if(m_ClassificationType == "particleid") {
    ATH_MSG_INFO( "Particle ID em fraction will be used to classify clusters:"  );
    m_isSingleParticle = false;
    dmData->SetClassificationType(CaloHadDMCoeffData::kCLASSIFY_USE_PARTICLEID);
  }
  dmData->SetMaxEventsPerFile(m_MaxEventsPerFile);

  std::unique_ptr<CaloLocalHadCoeff> newHadDMCoeffFit;
  std::unique_ptr<CaloLocalHadCoeff> newHadDMCoeffMinim;

  // TProfile approach (presamplers, material between EMB&TILE, EMEC&HEC)
  // and lookup approach (leakages, unclassified DM energy)
  if(m_doFit) {
    CaloHadDMCoeffFit *dmFit = new CaloHadDMCoeffFit();
    dmFit->SetNormalizationType(m_NormalizationTypeForFit.value());
    newHadDMCoeffFit = std::unique_ptr<CaloLocalHadCoeff>(dmFit->process(dmData.get(), initHadDMCoeff.get(), m_isSingleParticle, m_isTestbeam));
    if( !newHadDMCoeffFit ) {
      ATH_MSG_FATAL( "Failed in CaloHadDMCoeffFit::process()"  );
      delete dmFit;
      return StatusCode::FAILURE;
    }
    if(!m_reportProfiles.value().empty()) dmFit->make_report(m_reportProfiles.value());
    delete dmFit;
  }

  // minimization approach (material before FCAL)
  if(m_doMinimization) {
    CaloHadDMCoeffMinim *dmMinim = new CaloHadDMCoeffMinim();
    dmMinim->SetNormalizationType(m_NormalizationTypeForMinim.value());
    if(newHadDMCoeffFit) {
      // to use previous coefficients as initial values
      newHadDMCoeffMinim = std::unique_ptr<CaloLocalHadCoeff>(dmMinim->process(dmData.get(), newHadDMCoeffFit.get(), m_isSingleParticle, m_isTestbeam));
    }else{
      newHadDMCoeffMinim = std::unique_ptr<CaloLocalHadCoeff>(dmMinim->process(dmData.get(), initHadDMCoeff.get(), m_isSingleParticle, m_isTestbeam));
    }
    if( !newHadDMCoeffMinim ) {
      ATH_MSG_FATAL( "Failed in CaloHadDMCoeffMinim::process()"  );
      return StatusCode::FAILURE;
    }
    if(!m_reportMinimization.value().empty()) dmMinim->make_report(m_reportMinimization.value());
    delete dmMinim;
  }

  CaloLocalHadCoeff *result = nullptr;
  if(newHadDMCoeffMinim) {
    result = new CaloLocalHadCoeff(*newHadDMCoeffMinim);
  }else if(newHadDMCoeffFit) {
    result = new CaloLocalHadCoeff(*newHadDMCoeffFit);
  }else{
    result = new CaloLocalHadCoeff(*initHadDMCoeff);
  }
  // prints coefficients to ASCII file
  if (result) {
    std::ofstream fout;
    fout.open(m_HadDMCoeffOutputFile.value().c_str());
    dmHelper.PrintData(result, fout);
    fout.close();
  }

  // writing data to the pool
  if(m_doPool) {
    ATH_CHECK( detStore()->record(result,m_key.value()) );
    ATH_CHECK( detStore()->setConst(result) );
  }

  // running toy reconstruction for validation of new dead material constants
  if(m_doCheck) {
    CaloHadDMCoeffCheck *dmCheck = new CaloHadDMCoeffCheck();
    dmCheck->process(dmData.get(), result, m_isSingleParticle, m_isTestbeam);
    dmCheck->make_report(m_reportCheck.value());
    delete dmCheck;
  }

  // end of the game
  delete pChain;

  if(result && !m_doPool) delete result;

  return StatusCode::SUCCESS;
}



/* ****************************************************************************

***************************************************************************** */
StatusCode GetLCDeadMaterial::finalize()
{
  ATH_MSG_INFO( " Nothing to be done in finalize() method "  );
  return StatusCode::SUCCESS;
}



/* ****************************************************************************

***************************************************************************** */
StatusCode GetLCDeadMaterial::execute(const EventContext& /*ctx*/)
{
  ATH_MSG_INFO( " Nothing to be done in execute() method "  );
  return StatusCode::SUCCESS;
}

