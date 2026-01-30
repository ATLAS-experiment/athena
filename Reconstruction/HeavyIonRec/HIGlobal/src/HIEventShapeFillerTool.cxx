/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "HIGlobal/HIEventShapeFillerTool.h"
#include <Navigation/NavigationToken.h>
#include <CaloEvent/CaloCell.h>
#include <CaloEvent/CaloCellContainer.h>
#include <xAODHIEvent/HIEventShape.h>
#include "HIEventUtils/HIEventDefs.h"
#include "HIEventUtils/HIEventShapeMapTool.h"
#include "LArElecCalib/LArProvenance.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "CaloDetDescr/CaloDetDescrElement.h"

#include <iostream>
#include <iomanip>

HIEventShapeFillerTool::HIEventShapeFillerTool(const std::string& myname) : asg::AsgTool(myname),
m_index(nullptr)
{
}

StatusCode HIEventShapeFillerTool::initialize()
{

  ATH_MSG_INFO("In HIEventShapeFillerTool::initialize()");

  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );
  ATH_CHECK(m_caloCellKey.initialize(SG::AllowEmpty));

  return StatusCode::SUCCESS;
}


StatusCode HIEventShapeFillerTool::initializeIndex()
{
  m_index = m_eventShapeMapTool->getIndex(HI::BinningScheme::COMPACT);
  return StatusCode::SUCCESS;
}


StatusCode HIEventShapeFillerTool::initializeEventShapeContainer(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape) const
{
  //use tool to initialize event shape object
  if (!m_useClusters) m_index->initializeEventShapeContainer(evtShape, m_numOrders);
  return StatusCode::SUCCESS;
}


StatusCode HIEventShapeFillerTool::fillCollectionFromTowers(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape, const SG::ReadHandleKey<xAOD::CaloClusterContainer>& tower_container_key, const SG::ReadHandleKey<INavigable4MomentumCollection>& navi_container_key, const EventContext& ctx) const
{
  //retrieve the tower container from store
  if (m_useClusters)
  {
    SG::ReadHandle<xAOD::CaloClusterContainer>  readHandleCaloClus(tower_container_key, ctx);
    ATH_CHECK(readHandleCaloClus.isValid());
    return fillCollectionFromClusterContainer(evtShape, readHandleCaloClus.cptr(),ctx);
  }
  SG::ReadHandle<INavigable4MomentumCollection>  readHandleINav(navi_container_key, ctx);
  ATH_CHECK(readHandleINav.isValid());
  return fillCollectionFromTowerContainer(evtShape, readHandleINav.cptr());
}


StatusCode HIEventShapeFillerTool::fillCollectionFromTowerContainer(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape, const INavigable4MomentumCollection* navInColl) const
{

  SG::ReadHandle<CaloCellContainer> cellContainer{m_caloCellKey};
        //loop on towers
  for (INavigable4MomentumCollection::const_iterator towerItr = navInColl->begin();
    towerItr != navInColl->end(); ++towerItr)
  {
    //navigate back to cells
    //Default is to sort the cells by either pointer values leading to irreproducible output
    //CaloCellIDFcn ensures cells are ordered by their IDs
    NavigationToken<CaloCell, double, CaloCellIDFcn> cellToken;
    (*towerItr)->fillToken(cellToken, double(1.));

    // Use eta/phi of tower in shape calculation
    float eta0 = (*towerItr)->eta();
    float phi0 = (*towerItr)->phi();

    if (cellToken.size() == 0) continue;
    for (NavigationToken<CaloCell, double, CaloCellIDFcn>::const_iterator cellItr = cellToken.begin();
      cellItr != cellToken.end(); ++cellItr) 
    {

     const CaloCell* cell=*cellItr;
     std::unique_ptr<const CaloCell> mirroredCell{};

     bool isDeadFEB = (!cell->caloDDE()->is_tile() && LArProv::test(cell->provenance(),LArProv::DEADFEB));

     if (isDeadFEB) {
        mirroredCell=getMirroredCell(cell,cellContainer.cptr());
        if (mirroredCell)
          cell=mirroredCell.get();
        else {
           ATH_MSG_WARNING("Failed to obtain mirrored cell for deadFEB cell with id" << std::hex << cell->ID().get_compact());
        }

     }//end if dead FEB

     updateShape(evtShape, m_index, cell, cellToken.getParameter(*cellItr), eta0, phi0);
    }
  }//end tower loop
  return StatusCode::SUCCESS;
}


StatusCode HIEventShapeFillerTool::fillCollectionFromClusterContainer(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape, const xAOD::CaloClusterContainer* theClusters, const EventContext& ctx) const
{
  constexpr float area_slice = HI::TowerBins::getBinArea() * HI::TowerBins::numPhiBins();
  evtShape->reserve(HI::TowerBins::numEtaBins());
  for (unsigned int eb = 0; eb < HI::TowerBins::numEtaBins(); eb++)
  {
    xAOD::HIEventShape* e = new xAOD::HIEventShape();
    evtShape->push_back(e);
    e->setLayer(0);
    e->setEtaMin(HI::TowerBins::getBinLowEdgeEta(eb));
    e->setEtaMax(HI::TowerBins::getBinUpEdgeEta(eb));
    e->etCos().assign(m_numOrders, 0);
    e->etSin().assign(m_numOrders, 0);
    e->setArea(area_slice);
    e->setNCells(HI::TowerBins::numPhiBins());
  }


  static const SG::AuxElement::Decorator< float > decorator("HIEtaPhiWeight");
  static const SG::AuxElement::Decorator< float > cm_decorator("HIMag");
  static const SG::AuxElement::Accessor<float> acc_mcell_sumE("mcell_sumE");

  constexpr float area_cluster = HI::TowerBins::getBinArea();
  int runIndex = -1;
  if(!theClusters->empty() && m_towerWeightTool)
  {
    runIndex = m_towerWeightTool->getRunIndex(ctx);
  }

  for (auto cl : *theClusters) {

    double mcell_sumE = 0;
    if (acc_mcell_sumE.isAvailable(*cl)) {
      if(acc_mcell_sumE(*cl) > 1) {
        mcell_sumE = acc_mcell_sumE(*cl);
        ATH_MSG_DEBUG("Energy corrected from mirror cell to cluster: " << mcell_sumE);
      }
    }

    double ET = (cl->e()+mcell_sumE) / std::cosh(cl->eta0());
    double phi = cl->phi0();
    double eta = cl->eta0();
    unsigned int eb = HI::TowerBins::findBinEta(eta);
    xAOD::HIEventShape* slice = evtShape->at(eb);
    float weight = 1;
    if (m_towerWeightTool)
    {
      float recip = m_towerWeightTool->getEtaPhiResponse(eta, phi, runIndex);
      if (recip != 0.) weight = 1. / recip;
    }
    decorator(*cl) = weight;

    //HIMag back in rel 22 (removed by mistake in 21)
    float etot2 = 0;
    float er2 = 0;

    for (unsigned int sample = 0; sample < 24; sample++)
    {
      CaloSampling::CaloSample s = static_cast<CaloSampling::CaloSample>(sample);
      if (!cl->hasSampling(s)) continue;
      float esamp = std::abs(cl->eSample(s));
      float w1 = m_towerWeightTool->getWeight(eta, phi, s);
      float wr = m_towerWeightTool->getWeightMag(eta, phi, s);
      etot2 += esamp * w1;
      er2 += esamp * wr;

    }
    float cm = 0;
    if (etot2 != 0) cm = er2 / etot2;
    //float cm=er2/etot2;
    cm_decorator(*cl) = cm;

    //update members
    slice->setEt(slice->et() + weight * ET);
    slice->setRho(slice->rho() + weight * ET / area_cluster);

    for (unsigned int i = 0; i < static_cast<unsigned int>(m_numOrders); i++)
    {
      float nn = i + 1;
      float tmp_cos = slice->etCos().at(i);
      slice->etCos()[i] = tmp_cos + weight * ET * std::cos(nn * phi);
      float tmp_sin = slice->etSin().at(i);
      slice->etSin()[i] = tmp_sin + weight * ET * std::sin(nn * phi);
    }
  }
  ATH_MSG_DEBUG("DUMPING HIEVENTSHAPE");
  for (auto es : *evtShape)
  {
    ATH_MSG_DEBUG(std::setw(10) << es->etaMin()
      << std::setw(10) << es->etaMax()
      << std::setw(15) << es->et() * 1e-3);

    for (unsigned int i = 0; i < static_cast<unsigned int>(m_numOrders); i++)
    {
      ATH_MSG_DEBUG(std::setw(40) << i
        << std::setw(15) << es->etCos().at(i)
        << std::setw(15) << es->etSin().at(i));
    }

  }
  return StatusCode::SUCCESS;
}


StatusCode HIEventShapeFillerTool::fillCollectionFromCells(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape, const SG::ReadHandleKey<CaloCellContainer>& cell_container_key, const EventContext& ctx) const
{
  //retrieve the cell container from store
  SG::ReadHandle<CaloCellContainer>  read_handle_caloCell(cell_container_key, ctx);
  const CaloCellContainer* CellContainer = read_handle_caloCell.get();
  return fillCollectionFromCellContainer(evtShape, CellContainer);
}


StatusCode HIEventShapeFillerTool::fillCollectionFromCellContainer(std::unique_ptr<xAOD::HIEventShapeContainer>& evtShape, const CaloCellContainer* CellContainer) const
{
  //loop on Cells
  for (const auto cellItr : *CellContainer) {
    const CaloCell* cell=cellItr; 
    std::unique_ptr<const CaloCell> mirroredCell{};
    bool isDeadFEB = (!cell->caloDDE()->is_tile() && LArProv::test(cell->provenance(),LArProv::DEADFEB));
    if (isDeadFEB) {
      mirroredCell=getMirroredCell(cell,CellContainer);
      if (mirroredCell)
        cell=mirroredCell.get();
      else {
        ATH_MSG_WARNING("Failed to obtain mirrored cell for deadFEB cell with id" << std::hex << cell->ID().get_compact());
      }
    }


   updateShape(evtShape, m_index, cell, 1., cellItr->eta(), cellItr->phi());
  } 

  return StatusCode::SUCCESS;
}


void HIEventShapeFillerTool::updateShape(std::unique_ptr<xAOD::HIEventShapeContainer>& shape, const HIEventShapeIndex* index, const CaloCell* theCell, float geoWeight, float eta0, float phi0, bool isNeg) const
{
  float sgn = (isNeg) ? -1 : 1;

  int layer = theCell->caloDDE()->getSampling();
  float cell_et = theCell->et();

  xAOD::HIEventShape* slice = index->getShape(eta0, layer, shape);
  //update members
  slice->setNCells(slice->nCells() + sgn);
  slice->setEt(slice->et() + sgn * cell_et * geoWeight);
  float deta = theCell->caloDDE()->deta();
  float dphi = theCell->caloDDE()->dphi();
  float area = std::abs(deta * dphi);
  float rho = 0;
  if (area != 0.) rho = cell_et / area;
  slice->setArea(slice->area() + sgn * area * geoWeight);
  slice->setRho(slice->rho() + sgn * rho);

  for (unsigned int ih = 0; ih < slice->etCos().size(); ih++)
  {
    float ih_f = ih + 1;
    float tmp_cos = slice->etCos().at(ih);
    slice->etCos()[ih] = tmp_cos + cell_et * cos(ih_f * phi0) * geoWeight;

    float tmp_sin = slice->etSin().at(ih);
    slice->etSin()[ih] = tmp_sin + cell_et * sin(ih_f * phi0) * geoWeight;
  }
}

std::unique_ptr<const CaloCell> HIEventShapeFillerTool::getMirroredCell(const CaloCell* pCell, const CaloCellContainer* ccc) const {

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
