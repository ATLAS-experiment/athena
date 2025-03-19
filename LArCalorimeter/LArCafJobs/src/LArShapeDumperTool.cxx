/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCafJobs/LArShapeDumperTool.h"
#include "LArRawEvent/LArDigit.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "LArElecCalib/ILArShape.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"

#include "LArRawConditions/LArPhysWave.h"
#include "LArRawConditions/LArShapeComplete.h"

#include "CaloDetDescr/CaloDetDescrElement.h"
#include "LArCafJobs/ShapeInfo.h"
#include "LArCafJobs/CellInfo.h"

#include <vector>
#include <iostream>
using std::endl;

using namespace LArSamples;


LArShapeDumperTool::LArShapeDumperTool(const std::string& type
				       , const std::string& name
				       , const IInterface* parent)
  : AthAlgTool(type, name, parent)
{
  declareInterface<ILArShapeDumperTool>(this);
  declareProperty("DoShape", m_doShape = true);
  declareProperty("DoAllShapes", m_doAllShapes = false);
  declareProperty("ShapeKey",m_shapeKey="LArShape17phases");
  declareProperty("IsSC", m_isSC = false);
}


LArShapeDumperTool::~LArShapeDumperTool() 
{
}


StatusCode LArShapeDumperTool::initialize() {

  const CaloCell_Base_ID* idHelper = nullptr;
  if(m_isSC)
     ATH_CHECK( detStore()->retrieve (idHelper, "CaloCell_SuperCell_ID") );
  else   
     ATH_CHECK( detStore()->retrieve (idHelper, "CaloCell_ID") );
  m_emId   = idHelper->em_idHelper();
  m_hecId  = idHelper->hec_idHelper();
  m_fcalId = idHelper->fcal_idHelper();

  if(m_isSC) {
     const LArOnline_SuperCellID *onlHSC;
     ATH_CHECK(detStore()->retrieve(onlHSC, "LArOnline_SuperCellID"));
     m_onlineHelper=onlHSC;
  } else {   
     const LArOnlineID *onlH;
     ATH_CHECK(detStore()->retrieve(onlH, "LArOnlineID"));
     m_onlineHelper=onlH;
  }
  
  return StatusCode::SUCCESS; 
}


StatusCode LArShapeDumperTool::finalize()
{
  return StatusCode::SUCCESS;
}


CellInfo* LArShapeDumperTool::makeCellInfo(const HWIdentifier& channelID, const Identifier& id, 
                                           const CaloDetDescrElement* caloDetElement) const
{
  CaloId calo = UNKNOWN_CALO;
  short iEta = -1, iPhi = -1, layer = -1;
  if (m_emId->is_lar_em(id)) {
    calo = (CaloId)m_emId->barrel_ec(id);
    iEta = m_emId->eta(id) + (m_emId->region(id) << 10);
    iPhi = m_emId->phi(id);
    layer = m_emId->sampling(id);
//      cout << "NJPB: " << hash << " " << m_emId->barrel_ec(channelID) << " " << m_emId->eta(id) << " " << m_emId->phi(id) << " " 
//           << m_emId->sampling(id) << " " << m_emId->region(id) << endl;
  }
  else if (m_hecId->is_lar_hec(id)) {
    calo = (CaloId)(abs(int(HEC_A))*(m_onlineHelper->pos_neg(channelID) ? 1 : -1));
    iEta = m_hecId->eta(id) + (m_hecId->region(id) << 10);
    iPhi = m_hecId->phi(id);
    layer = m_hecId->sampling(id);
  }
  else if (m_fcalId->is_lar_fcal(id)) {
    calo = (CaloId)(abs(int(FCAL_A))*(m_onlineHelper->pos_neg(channelID) ? 1 : -1));
    iEta = m_fcalId->eta(id);
    iPhi = m_fcalId->phi(id);
    layer = m_fcalId->module(id);
  }
  else {
    msg(MSG::WARNING) << "LArDigit Id "<< MSG::hex << id.get_identifier32().get_compact() << MSG::dec 
		      << " (FT: " << m_onlineHelper->feedthrough(channelID) << " FEBSlot: " 
		      << m_onlineHelper->slot(channelID) << " Chan: " << m_onlineHelper->channel(channelID)
		      << ") appears to be neither EM nor HEC nor FCAL." << endmsg;
    return nullptr;
  }

  ShapeInfo* shapeL = nullptr;
  ShapeInfo* shapeM = nullptr;
  ShapeInfo* shapeH = nullptr;
            
  if (m_doAllShapes) {
    shapeL = retrieveShape(channelID, CaloGain::LARLOWGAIN);
    shapeM = retrieveShape(channelID, CaloGain::LARMEDIUMGAIN);
    shapeH = retrieveShape(channelID, CaloGain::LARHIGHGAIN);
  }
 
  TVector3 position(0,0,1);
  ULong64_t onlid = (ULong64_t)channelID.get_identifier32().get_compact(); 
  if (caloDetElement) position = TVector3(caloDetElement->x(), caloDetElement->y(), caloDetElement->z());

  return new CellInfo(calo, layer, iEta, iPhi, 
                      m_onlineHelper->feedthrough(channelID), 
                      m_onlineHelper->slot(channelID), m_onlineHelper->channel(channelID), 
                      shapeL, shapeM, shapeH, position,  onlid);
}


ShapeInfo* LArShapeDumperTool::retrieveShape(const HWIdentifier& channelID, CaloGain::CaloGain gain) const
{
  ILArShape* ishape=nullptr;
  if (detStore()->retrieve(ishape,m_shapeKey)!=StatusCode::SUCCESS) {
    return nullptr;
  }
 
  const LArShapeComplete* shapeObj = dynamic_cast<const LArShapeComplete*>(ishape);
  if (!shapeObj) {
    msg(MSG::INFO) << "Shape object is not of type LArShapeComplete!" << endmsg;
    return nullptr;
  }
  //const std::vector< std::vector<float> >& fullShape = shapeObj->get(channelID, gain).m_vShape;
  //ILArShape::ShapeRef_t shapeder=shapeObj->Shape(chid,igain,iphase);

  ShapeInfo* shape = nullptr;
  const size_t nPhases=shapeObj->nTimeBins(channelID,gain);
  if (nPhases == 0) {
    msg(MSG::WARNING) << "Shape object for channel " << channelID << " and gain " << gain 
             << " has 0 phases !" << endmsg;
    return nullptr;
  }    

  for (unsigned int iPhase = 0; iPhase < nPhases; iPhase++) {
     ILArShape::ShapeRef_t shape_i=shapeObj->Shape(channelID,gain,iPhase);
     //const std::vector<float>& shape_i = fullShape[iPhase];
     if (!(shape_i.valid() && shape_i.size()>0)) {
       msg(MSG::WARNING) << "Shape object for channel " << channelID << " and gain " << gain 
			 << " has no data in phase " << iPhase << " !" << endmsg;
       delete shape;
       return nullptr;
     }
     for (unsigned int iSample = 0; iSample < shape_i.size(); iSample++) {
       if (iSample == 0 && iPhase == 0)
         shape = new ShapeInfo(shape_i.size(), 3, nPhases, 0);  // 0 was 12.5 
       shape->set(iSample, iPhase, shape_i[iSample]);
     }
  }
  return shape;
}
