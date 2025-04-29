/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

// ********************************************************************
//
// NAME:     CaloGlobalRoIBuilder.cxx
// PACKAGE:  Trigger/TrigAlgorithms/TrigCaloRec
//
// AUTHOR:   D.O. Damazio
//           Algorithm to prepare RoIs based on the presence of cells
//	     above some noise threshold
//
// ********************************************************************
//

#include "CaloGlobalRoIBuilder.h"
#include "CxxUtils/phihelper.h"
#include "GaudiKernel/StatusCode.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "TrigT2CaloEgamma/RingerReFex.h"
#include "xAODTrigCalo/TrigEMCluster.h"
#include "xAODTrigCalo/TrigEMClusterContainer.h"
#include "xAODTrigCalo/TrigEMClusterAuxContainer.h"


// Constructor
CaloGlobalRoIBuilder::CaloGlobalRoIBuilder(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
};

// Initialize
StatusCode CaloGlobalRoIBuilder::initialize()
{
  ATH_MSG_DEBUG("in CaloGlobalRoIBuilder::initialize()" );

  ATH_CHECK( m_inputCellsKey.initialize() );
  ATH_CHECK( m_noiseCDOKey.initialize() );
  ATH_CHECK(m_clusterContainerKey.initialize());
  ATH_CHECK( m_ringerContainerKey.initialize() );
  m_emAlgTools.retrieve().ignore();

  return StatusCode::SUCCESS;

}

// Initialize
StatusCode CaloGlobalRoIBuilder::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("in CaloGlobalRoIBuilder::execute()" );
  auto cells = SG::makeHandle(m_inputCellsKey, ctx);
  SG::ReadCondHandle<CaloNoise> noiseHdl{m_noiseCDOKey, ctx};
  const CaloNoise *noisep = *noiseHdl;
  ATH_MSG_VERBOSE(" Input Cells : " << cells.name() <<" of size " <<cells->size() );
  SG::WriteHandle<xAOD::TrigEMClusterContainer> trigEmClusterCollection(m_clusterContainerKey, ctx);
  ATH_CHECK( trigEmClusterCollection.record(std::make_unique<xAOD::TrigEMClusterContainer>(),
                                          std::make_unique<xAOD::TrigEMClusterAuxContainer>()) );

  SG::WriteHandle<xAOD::TrigRingerRingsContainer> ringsCollection =
  SG::WriteHandle<xAOD::TrigRingerRingsContainer>( m_ringerContainerKey, ctx );

  ATH_CHECK( ringsCollection.record( std::make_unique<xAOD::TrigRingerRingsContainer>(),  
             std::make_unique<xAOD::TrigRingerRingsAuxContainer>() ) );

  unsigned int clustern=0; // for linking
  for (const auto cell : *cells ) {
    const CaloDetDescrElement* cdde = cell->caloDDE();
    if ( fabsf(cdde->eta()) > m_abseta_thr ) continue;
    if ( cdde->getSampling() > m_samp_thr ) continue;
    if (cdde->is_tile() ) continue;
    float thr=noisep->getNoise(cdde->identifyHash(), cell->gain());
    if ( cell->energy() > m_thr*thr ){
       // For each hot cell, let's make ringers
       xAOD::TrigEMCluster *ptrigEmCluster = new xAOD::TrigEMCluster();
       trigEmClusterCollection->push_back(ptrigEmCluster);
       ptrigEmCluster->setEnergy(cell->energy());
       ptrigEmCluster->setEnergy(cdde->getSampling(),cell->energy());
       ptrigEmCluster->setEt(cell->pt());
       ptrigEmCluster->setEta(cell->eta());
       ptrigEmCluster->setPhi(cell->phi());
       ptrigEmCluster->setE233(thr);
       double etaWidth(0.2);
       double phiWidth(0.2);
       double etamin = std::max(-2.5, cell->eta() - etaWidth);
       double etamax = std::min(2.5, static_cast<double>(cell->eta() + etaWidth));
       double phimin = CxxUtils::wrapToPi(cell->phi() - phiWidth);
       double phimax = CxxUtils::wrapToPi(static_cast<double>(cell->phi() + phiWidth));
       const TrigRoiDescriptor newroi(cell->eta(),etamin,etamax,cell->phi(),phimin,phimax);
       // use tool only to build rings
       std::vector<RingerReFex::RingSet> vec_rs;
       ATH_CHECK(m_emAlgTools->prepareRinger(vec_rs,*ptrigEmCluster,newroi,ctx));
       std::vector<float> ref_rings;
       for (std::vector<RingerReFex::RingSet>::iterator it=vec_rs.begin(); it!=vec_rs.end(); ++it)
       {
          auto rings = it->rings();
          ref_rings.insert(ref_rings.end(), rings.begin(), rings.end());
       }
       auto ptrigRingerRings= new xAOD::TrigRingerRings();
       ringsCollection->push_back( ptrigRingerRings );
       ptrigRingerRings->setRings(ref_rings);
       auto clusLink = ElementLink<xAOD::TrigEMClusterContainer>(m_clusterContainerKey.key(),clustern++,ctx);
       ptrigRingerRings->setEmClusterLink( clusLink  );

    }
  }

  return StatusCode::SUCCESS;
}
