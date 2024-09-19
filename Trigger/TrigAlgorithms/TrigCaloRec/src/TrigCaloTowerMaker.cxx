/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     TrigCaloTowerMaker.cxx
// PACKAGE:  Trigger/TrigAlgorithms/TrigCaloRec
//
// AUTHOR:   P.A. Delsart
//           This is an Hlt algorithm that creates Calorimeter towers
//           It assumes CaloCellContainers are already produced and accessable
//           through the input TriggerElement. Based on TrigCaloRec
//
// ********************************************************************
//
//
#include <sstream>
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/TypeNameString.h"
#include "GaudiKernel/StatusCode.h"

#include "CxxUtils/phihelper.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include "CaloEvent/CaloCellContainer.h"
#include "EventKernel/INavigable4Momentum.h"
#include "NavFourMom/INavigable4MomentumCollection.h"

#include "CaloEvent/CaloTowerContainer.h"
#include "CaloEvent/CaloTowerSeg.h"
#include "CaloUtils/CaloTowerBuilderToolBase.h"

#include "TrigCaloTowerMaker.h"

class ISvcLocator;

TrigCaloTowerMaker::TrigCaloTowerMaker(const std::string& name,
                                       ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator),
      m_includeFcal(false) {

}

/////////////////////////////////////////////////////////////////////
// INITIALIZE:
// The initialize method will create all the required algorithm objects
// Note that it is NOT NECESSARY to run the initialize of individual
// sub-algorithms.  The framework takes care of it.
/////////////////////////////////////////////////////////////////////

StatusCode TrigCaloTowerMaker::initialize() {
  ATH_MSG_DEBUG("in initialize()");

  if (!m_monTool.empty()) {
    ATH_MSG_DEBUG("Retrieving monTool");
    CHECK(m_monTool.retrieve());
  } else {
    ATH_MSG_INFO("No monTool configured => NO MONITOING");
  }

  ATH_CHECK(m_towerMakerTools.retrieve());

  CaloTowerSeg theTowerSeg(m_nEtaTowers, m_nPhiTowers, m_minEta, m_maxEta);
  for (auto& tool: m_towerMakerTools) {
    tool->setTowerSeg(theTowerSeg);
  }
  // presence of tool with the name FC triggers use of FCal 
  for (auto& tool: m_towerMakerTools) {
    if ( tool->name().find("FC") != std::string::npos)  {
      m_includeFcal = true;
      ATH_MSG_INFO("Tool " << tool->name() << " is configured, will use FCal");
    }
  }

  m_caloTowerNav4LinkKey = m_outputTowerKey.key();

  ATH_CHECK(m_inputRoiKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_inputCellsKey.initialize());
  ATH_CHECK(m_outputTowerKey.initialize());
  ATH_CHECK(m_caloTowerNav4LinkKey.initialize());

  ATH_MSG_DEBUG("Initialization of TrigCaloTowerMaker completed successfully");

  return StatusCode::SUCCESS;
}

StatusCode TrigCaloTowerMaker::execute(const EventContext& ctx) const {
  // Monitoring initialization...
  auto timer = Monitored::Timer("TIME_execute");
  auto time_tools = Monitored::Timer("TIME_tools");
  auto mon_towerContainerSize = Monitored::Scalar("towerContainerSize", 0.);

  auto monitorIt =
      Monitored::Group(m_monTool, timer, time_tools, mon_towerContainerSize);

  ATH_MSG_DEBUG("in execute()");


  // Get RoiDescriptor
  const bool seedLess = m_inputRoiKey.empty();
  TrigRoiDescriptor roiDescriptor;
  if (!seedLess) {
    SG::ReadHandle<TrigRoiDescriptorCollection> roiCollection =
        SG::makeHandle(m_inputRoiKey, ctx);
    if ( !roiCollection.isValid()) {
      ATH_MSG_ERROR("Cell maker did not get a valid RoIs collection");
      return StatusCode::FAILURE;
    }
    if (roiCollection->size() == 0) {
      ATH_MSG_DEBUG(" RoI collection size = 0");
      return StatusCode::SUCCESS;
    }
    if (roiCollection->size() > 1)
      ATH_MSG_WARNING("Misconfiguration - Called with "
                      << roiCollection->size()
                      << " ROI, but it should be called with 1 RoI - Will only "
                         "process the first RoI");

    roiDescriptor = roiCollection->front();
    ATH_MSG_DEBUG("Operating on " << roiCollection->size() << "RoI(s)");
  } else {
    roiDescriptor = TrigRoiDescriptor(true);
  }
  ATH_MSG_DEBUG(" RoI id " << roiDescriptor.roiId()
                           << " located at   phi = " << roiDescriptor.phi()
                           << ", eta = " << roiDescriptor.eta());

  auto caloTowerContainer = SG::makeHandle(m_outputTowerKey, ctx);

  /// ho hum, this needs a flag for using own wdiths rather than those from the
  /// roiDescriptor in addition, this will *not* work properly for composite
  /// RoIs

  if (roiDescriptor.composite()) {
    ATH_MSG_WARNING(
        " Attempting to use composite RoI as a normal RoI - this is probably "
        "*not* what you want to do "
        << roiDescriptor);
  }

  double eta0 = roiDescriptor.eta();
  double phi0 = roiDescriptor.phi();

  phi0 = CxxUtils::wrapToPi(phi0);
  double etamin = eta0 - m_deta / 2.;
  double etamax = eta0 + m_deta / 2.;
  // What if phimin < 0 or phimax > 2Pi ??  question
  double phimin = phi0 - m_dphi / 2.;
  double phimax = phi0 + m_dphi / 2.;

  ATH_MSG_DEBUG(" eta0 = " << eta0 << " phi0 = " << phi0
                           << " etamin = " << etamin << " etamax = " << etamax
                           << " phimin = " << phimin << " phimax = " << phimax);

  CaloTowerSeg myseg(m_nEtaTowers, m_nPhiTowers, m_minEta, m_maxEta);
  CaloTowerSeg::SubSeg subseg =
      myseg.subseg(roiDescriptor.eta(), m_deta, roiDescriptor.phi(), m_dphi);
  if (m_includeFcal) {
    ATH_CHECK(
        caloTowerContainer.record(std::make_unique<CaloTowerContainer>(myseg)));
  } else {
    ATH_CHECK(caloTowerContainer.record(
        std::make_unique<CaloTowerContainer>(subseg.segmentation())));
  }
  CaloTowerContainer* pCaloTowerContainer = caloTowerContainer.ptr();

  ATH_CHECK(caloTowerContainer.symLink(m_caloTowerNav4LinkKey));
  ATH_MSG_DEBUG("CaloTowerContainer"
                << caloTowerContainer.name()
                << " symlinked to  INavigable4MomentumCollection in StoreGate");

  auto caloCellContainer = SG::makeHandle(m_inputCellsKey, ctx);

  // Get the last container in the vector. Should be th one produced by the
  // previous TrigCaloCellMaker.
  const CaloCellContainer* theCellCont = caloCellContainer.ptr();

  ATH_MSG_DEBUG(
      " REGTEST: Retrieved a Cell Container of Size= " << theCellCont->size());

  // Now Build the towers
  // -----------------------------------------------------------------
  time_tools.start();

  for ( auto& tool: m_towerMakerTools ) {
    if (m_includeFcal) {
      ATH_CHECK(tool->execute(ctx, pCaloTowerContainer, theCellCont));
      ATH_MSG_DEBUG("Executed tool " << tool.name());
    } else {
      ATH_CHECK(tool->execute(ctx, pCaloTowerContainer, theCellCont, &subseg));
      ATH_MSG_DEBUG("Executed tool " << tool.name());
    }
  }
  time_tools.stop();

  ATH_MSG_DEBUG(" REGTEST: Produced a Tower Container "
                  << caloTowerContainer.name() << " at "
                  << caloTowerContainer.cptr()
                  << " of Size= " << pCaloTowerContainer->size());
  ATH_MSG_DEBUG(" handle info:  " << caloTowerContainer);
  mon_towerContainerSize = static_cast<float>(pCaloTowerContainer->size());

  return StatusCode::SUCCESS;
}
