/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


// Gaudi includes
#include "GaudiKernel/IChronoStatSvc.h"

#include "AthenaKernel/errorcheck.h"

#include "CaloEvent/CaloTowerSeg.h"
#include "CaloTowerAlgorithm.h"

#include <string>
#include <vector>
#include <iomanip>

CaloTowerAlgorithm::CaloTowerAlgorithm(const std::string& name, 
				       ISvcLocator* pSvcLocator) 
  : AthReentrantAlgorithm(name,pSvcLocator)
  , m_chrono("ChronoStatSvc", name)
{
}

CaloTowerAlgorithm::~CaloTowerAlgorithm()
= default;

////////////////
// Initialize //
////////////////

StatusCode CaloTowerAlgorithm::initialize()
{
  // Retrieve ChronoStatSvc
  if (m_doChronoStat) {
    ATH_CHECK( m_chrono.retrieve() );
  }

  ATH_CHECK(m_towerContainerKey.initialize());
  ////////////////////
  // Allocate Tools //
  ////////////////////

  // check tool names
  if (m_ptools.empty()) {
    ATH_MSG_ERROR(" no tools given for this algorithm.");
    return StatusCode::FAILURE;
  }

  // find tools


  CaloTowerSeg theTowerSeg(m_nEtaTowers,m_nPhiTowers,m_minEta,m_maxEta);


  unsigned int toolCtr = 0;
  ATH_MSG_INFO(" ");
  ATH_MSG_INFO("List of tools in execution sequence:");
  ATH_MSG_INFO("------------------------------------");

  ATH_CHECK(m_ptools.retrieve());

  for (ToolHandle<ICaloTowerBuilderToolBase>& tool : m_ptools) {
    toolCtr++;

    ATH_MSG_INFO(std::setw(2) << toolCtr << ".) " << tool->type()
        << "::name() = \042" << tool->name() << "\042");

    ATH_MSG_INFO("------------------------------------");
    ATH_MSG_INFO(" ");

    ATH_MSG_DEBUG(" set correct tower seg for this tool "
        << tool->name());

    tool->setTowerSeg(theTowerSeg);

//    if (tool->initializeTool().isFailure()) {
//      ATH_MSG_WARNING(" Tool failed to initialize");
//    }

  } //close iteration over tools
  return StatusCode::SUCCESS;
}

/////////////
// Execute //
/////////////

StatusCode CaloTowerAlgorithm::execute (const EventContext& ctx) const
{

  /////////////////////
  // Tool Processing //
  /////////////////////

  CaloTowerSeg theTowerSeg(m_nEtaTowers,m_nPhiTowers,m_minEta,m_maxEta);

  SG::WriteHandle<CaloTowerContainer> theTowers(m_towerContainerKey, ctx);
  ATH_CHECK( theTowers.record(std::make_unique<CaloTowerContainer>(theTowerSeg)) );
  

  ToolHandleArray<ICaloTowerBuilderToolBase>::const_iterator firstITool  = m_ptools.begin();
  ToolHandleArray<ICaloTowerBuilderToolBase>::const_iterator lastITool   = m_ptools.end();
  StatusCode processStatus = StatusCode::SUCCESS;
  //
  // loop stops only when Failure indicated by one of the tools
  //

  ATH_MSG_DEBUG("In execute() ");
  
  while (!processStatus.isFailure() && firstITool != lastITool) {

    if (m_doChronoStat) {
      m_chrono->chronoStart((*firstITool)->name());
    }

    processStatus = (*firstITool)->execute(ctx, theTowers.ptr());

    if (m_doChronoStat) {
      m_chrono->chronoStop((*firstITool)->name());
    }
    if (!processStatus.isFailure()) {
      ATH_MSG_DEBUG((*firstITool)->name()
          << ": CaloTowerContainer::size() = " << theTowers->size());

      ++firstITool;
    } else {
      // some problem - but do not skip event loop!
      ATH_MSG_ERROR("problems while or after processing tool \042"
          << (*firstITool)->name()
          << "\042 - cross-check CaloTowerContainer::size() = "
          << theTowers->size());

      ++firstITool;
    }
  }

  return StatusCode::SUCCESS;
}

//////////////
// Finalize //
//////////////

StatusCode CaloTowerAlgorithm::finalize()
{
  return StatusCode::SUCCESS;
}
