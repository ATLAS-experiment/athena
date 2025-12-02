/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "HIClusterMaker.h"
#include "HIJetRec/HIJetRecDefs.h"
#include "NavFourMom/INavigable4MomentumCollection.h"
#include "CaloEvent/CaloCell.h"
#include "CaloEvent/CaloCellContainer.h"
#include "Navigation/NavigationToken.h"
#include "CaloUtils/CaloClusterStoreHelper.h"
#include "CxxUtils/prefetch.h"
#include <cmath>
#include <TVector2.h>

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "LArElecCalib/LArProvenance.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "CaloDetDescr/CaloDetDescrElement.h"

HIClusterMaker::HIClusterMaker(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name,pSvcLocator)
{
}

StatusCode HIClusterMaker::initialize()
{
  //First we initialize keys - after initialization they are frozen
  ATH_CHECK( m_towerContainerKey.initialize() );
  ATH_CHECK( m_cellContainerKey.initialize() );
  ATH_CHECK( m_outputKey.initialize() );

  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );

  return StatusCode::SUCCESS;
}

StatusCode HIClusterMaker::execute(const EventContext &ctx) const
{

  //retrieve the tower container from store
  const INavigable4MomentumCollection* navInColl = 0;
  SG::ReadHandle<INavigable4MomentumCollection>  readHandleTower ( m_towerContainerKey, ctx );
  ATH_CHECK( readHandleTower.isValid() );
  navInColl = readHandleTower.cptr();

  const CaloCellContainer * cellColl ;
  SG::ReadHandle<CaloCellContainer>  readHandleCell ( m_cellContainerKey, ctx );
  ATH_CHECK( readHandleCell.isValid() );
  cellColl = readHandleCell.cptr();

  //make the container
  //Tricky migration: here we don't have to migrate our methods but what we use from CaloClusterStoreHelper
  SG::WriteHandle<xAOD::CaloClusterContainer> writeHandleContainer ( m_outputKey, ctx );
  ATH_CHECK(CaloClusterStoreHelper::AddContainerWriteHandle(writeHandleContainer));

  //loop on towers
  for(const auto *towerItr : *navInColl)
  {
    //initialize variables
    float E_cl=0;
    float eta_cl=0;
    float phi_cl=0;
    float time_cl=0;
    float E2_cl=0;
    uint32_t samplingPattern=0;

    //navigate back to cells
    //Default is to sort the cells by either pointer values leading to irreproducible output
    //CaloCellIDFcn ensures cells are ordered by their IDs
    NavigationToken<CaloCell,double,CaloCellIDFcn> cellToken;
    towerItr->fillToken(cellToken,double(1.));

    // Make the cluster:
    std::unique_ptr<xAOD::CaloCluster> cl(CaloClusterStoreHelper::makeCluster(cellColl));

    float mirror_cell_sumE = 0;

    if ( cellToken.size() == 0 ) continue;
    for(NavigationToken<CaloCell,double,CaloCellIDFcn>::const_iterator cellItr = cellToken.begin(); cellItr != cellToken.end(); ++cellItr )
    {
      //Bad cell policy - to be kept
      //if(m_bad_cell_tool->SkipCell(*cellItr))
      //{
      //if(m_skipBadCells && (*cellItr)->badcell()) continue;
      //}

      double geoWeight = cellToken.getParameter(*cellItr);      

      const CaloCell* cell=*cellItr;
      std::unique_ptr<const CaloCell> mirroredCell{};

      bool isDeadFEB = (!cell->caloDDE()->is_tile() && LArProv::test(cell->provenance(),LArProv::DEADFEB));

      if (isDeadFEB) {
         mirroredCell=getMirroredCell(cell,readHandleCell.cptr());
         if (mirroredCell) {
           cell=mirroredCell.get();
           mirror_cell_sumE+= cell->energy()*geoWeight;
         }
         else {
            ATH_MSG_WARNING("Failed to obtain mirrored cell for deadFEB cell with id" << std::hex << cell->ID().get_compact());
         }

      }//end if dead FEB
     
      double cell_E_w=(*cellItr)->energy()*geoWeight;

      IdentifierHash hashid =(*cellItr)->caloDDE()->calo_hash();
      size_t iCell=cellColl->findIndex(hashid);
      cl->addCell(iCell,geoWeight);

      E_cl+=cell_E_w;
      eta_cl+=cell_E_w*(*cellItr)->eta();
      phi_cl+=cell_E_w*(*cellItr)->phi();
      E2_cl+=cell_E_w*cell_E_w;
      time_cl+=cell_E_w*cell_E_w*(*cellItr)->time();

      unsigned int sample = (CaloSampling::CaloSample) (*cellItr)->caloDDE()->getSampling();
      samplingPattern |= (0x1U<<sample);
    }//end cell loop

    ATH_MSG_VERBOSE("Energy Sum of mirror deadFEB: " << mirror_cell_sumE);
    //decorating cluster with sum of mirror cell energy
    static const SG::AuxElement::Decorator<float> Mcell_sumE("mcell_sumE");    
    Mcell_sumE(*cl) = mirror_cell_sumE; 

    float eta0=towerItr->eta();
    float phi0=towerItr->phi();

    if(E_cl < m_EminMoment)
    {
      eta_cl=eta0;
      phi_cl=phi0;
    }
    else
    {
      eta_cl/=E_cl;
      phi_cl/=E_cl;
    }
    //phi moment does not respect wrap-around
    phi_cl=TVector2::Phi_mpi_pi(phi_cl);
    if(!HIJetRec::inTowerBoundary(eta0,phi0,eta_cl,phi_cl))
    {
      eta_cl=eta0;
      phi_cl=phi0;
    }

    if(E2_cl < 1e-8) time_cl=0.;
    else time_cl/=E2_cl;

    //set initial tower position
    cl->setEta0(eta0);
    cl->setPhi0(phi0);

    //set initial kinematics to be the same for all signal states
    //update upstream
    cl->setRawE(E_cl);
    cl->setRawEta(eta_cl);
    cl->setRawPhi(phi_cl);
    cl->setRawM(0);

    cl->setAltE(E_cl);
    cl->setAltEta(eta_cl);
    cl->setAltPhi(phi_cl);
    cl->setAltM(0);

    cl->setCalE(E_cl);
    cl->setCalEta(eta_cl);
    cl->setCalPhi(phi_cl);
    cl->setCalM(0);

    //extra info
    cl->setTime(time_cl);
    cl->setSamplingPattern(samplingPattern);

    ATH_MSG_VERBOSE( std::setw(20) << "PUSHING CLUSTER"
                  << std::setw(15) << cl->e()
                  << std::setw(15) << cl->eta()
                  << std::setw(15) << cl->phi() );

    writeHandleContainer->push_back(std::move(cl));
  }//end tower loop
  return StatusCode::SUCCESS;
}

StatusCode HIClusterMaker::finalize()
{
  return StatusCode::SUCCESS;
}


StatusCode HIClusterMaker::dumpClusters(xAOD::CaloClusterContainer* clusColl)
{
  ATH_MSG_INFO("Dumping PseudoJets");
  for(xAOD::CaloClusterContainer::iterator clusCollIter= clusColl->begin();
      clusCollIter!= clusColl->end(); ++clusCollIter)
  {
    xAOD::CaloCluster* cl = (*clusCollIter);

    float E_cl=0;
    float eta_cl=0;
    float phi_cl=0;

    CaloClusterCellLink* cellLinks=cl->getOwnCellLinks();

    if (!cellLinks)
    {
      ATH_MSG_ERROR("Can't get valid links to CaloCells (CaloClusterCellLink)!");
      return StatusCode::FAILURE;
    }

    unsigned int ncells=0;
    float sumw=0;
    // CaloClusterCellLink::iterator cellIterEnd=cellLinks->end();
    // for(CaloClusterCellLink::iterator cellIter=cellLinks->end();
    //   cellIter != cellIterEnd; cellIter++, ncells++ )
    // {
    xAOD::CaloCluster::cell_iterator cellIterEnd = cl->cell_end();
    for(xAOD::CaloCluster::cell_iterator cellIter= cl->cell_begin(); cellIter != cellIterEnd; ++cellIter )
    {
      CxxUtils::prefetchNext (cellIter, cellIterEnd);
      const CaloCell* pCell=(*cellIter);
      //double geoWeight =cellIter.weight(); //weird synatx, "." on iterator
      double cell_E_w=pCell->energy();//*geoWeight;

      E_cl+=cell_E_w;
      eta_cl+=cell_E_w*pCell->eta();
      phi_cl+=cell_E_w*pCell->phi();
      //sumw+=geoWeight;
    }
    if(E_cl!=0.)
    {
      eta_cl/=E_cl;
      phi_cl/=E_cl;
    }

    ATH_MSG_INFO( std::setw(10) << "DUMPING CLUSTER"
               << std::setw(15) << cl->e()
               << std::setw(15) << cl->eta()
               << std::setw(15) << cl->phi()
               << std::setw(15) << E_cl
               << std::setw(15) << eta_cl
               << std::setw(15) << phi_cl
               << std::setw(15) << ncells
               << std::setw(15) << sumw );
  }
  return StatusCode::SUCCESS;
}

std::unique_ptr<const CaloCell> HIClusterMaker::getMirroredCell(const CaloCell* pCell, const CaloCellContainer* ccc) const {

  const Identifier id = pCell->ID();
  const int subCalo = m_calo_id->sub_calo(id);
  const int pos_neg = m_calo_id->pos_neg(id);
  const int sampling = m_calo_id->sampling(id);
  const int region = m_calo_id->region(id);
  const int eta = m_calo_id->eta(id);
  const int phi = m_calo_id->phi(id);

  ATH_MSG_VERBOSE("DeadFEB cell parameter: (" << subCalo << "," << pos_neg << "," << sampling << "," << region << "," << eta << "," << phi << ")");

  const Identifier mirroredID = m_calo_id->cell_id(subCalo,
                                                   -pos_neg,  // flip to get cell in oposite eta
                                                   sampling, region, eta, phi);

  const CaloCell* mirroredCell = ccc->findCell(m_calo_id->calo_cell_hash(mirroredID));
  if (!mirroredCell) {
    return nullptr;
  }
  ATH_MSG_VERBOSE("DeadFEB cell (et,layer,deta,dphi): (" << pCell->et() << "," << pCell->caloDDE()->getSampling() << "," << pCell->caloDDE()->eta() << ","
                                                      << pCell->caloDDE()->phi() << ")");

  ATH_MSG_VERBOSE("Mirror cell (et,layer,deta,dphi): " << mirroredCell->et() << "," << mirroredCell->caloDDE()->getSampling() << ","
                                                    << mirroredCell->caloDDE()->eta() << "," << mirroredCell->caloDDE()->phi() << ")");

  // Build a fake-cell with the DDE of the cell we are replacing and
  // energy,time,etc from the eta-mirrored cell
  return std::make_unique<const CaloCell>(pCell->caloDDE(), mirroredCell->energy(), mirroredCell->time(), mirroredCell->quality(), mirroredCell->provenance(),
                                          mirroredCell->gain());
}
