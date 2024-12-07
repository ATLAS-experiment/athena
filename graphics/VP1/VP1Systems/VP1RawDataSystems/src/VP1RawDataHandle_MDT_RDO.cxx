/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class VP1RawDataHandle_MDT_RDO          //
//                                                            //
//  Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch> 
//  Initial version: January 2024    
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1RawDataSystems/VP1RawDataHandle_MDT_RDO.h"
#include "VP1RawDataSystems/VP1RawDataColl_MDT_RDO.h"

#include "VP1RawDataSystems/VP1RawDataCommonData.h"
#include "VP1RawDataSystems/VP1RawDataSysController.h"

#include "VP1Utils/HitsSoNodeManager.h"
#include "VP1Utils/VP1DetInfo.h"
#include "VP1Utils/VP1LinAlgUtils.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"

#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoMaterial.h>

// #include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
// #include "TRT_ReadoutGeometry/TRT_BaseElement.h"
// #include "InDetIdentifier/TRT_ID.h"
// #include "InDetRawData/TRT_LoLumRawData.h"

#include "GaudiKernel/SystemOfUnits.h"

//____________________________________________________________________
VP1RawDataHandle_MDT_RDO::VP1RawDataHandle_MDT_RDO(VP1RawDataCollBase* coll,const MdtDigit*data)
  : VP1RawDataHandleBase(coll), m_data(data)
{
}

//____________________________________________________________________
VP1RawDataHandle_MDT_RDO::~VP1RawDataHandle_MDT_RDO()
{
}

//____________________________________________________________________
QStringList VP1RawDataHandle_MDT_RDO::clicked(bool verbose) const
{
  static const MdtIdHelper * idhelper = VP1DetInfo::mdtIDHelper();
  const MuonGM::MdtReadoutElement * elem = element();

  QStringList ll;
  ll << " ===> MDT Digit data";
  // ll << "   is a valid digit: "+ m_data->is_valid(idhelper);
  ll << "   adc: " + QString::number(m_data->adc());
  ll << "   tdc: " + QString::number(m_data->tdc());
  if (verbose) {
    if (idhelper) {
      //Fixme: We should have common identify -> QStringList method in VP1DetInfo!!
      Identifier id(m_data->identify());

      ll << "Is Barrel: " << QString::number(elem->barrel());
         /// Returns whether the chamber is in the endcap
      ll << "Is Endcap: " << QString::number(elem->endcap());
      // ll << "Tube position :" << elem->tubePos(id).position[Amg::x];
      std::cout << "tubePos x: " << elem->tubePos(id)[Amg::x] << std::endl;

      // int barrel_ec = idhelper->barrel_ec(id);
      // bool barrel(barrel_ec==1||barrel_ec==-1);
      // l << QString(barrel?"Barrel":"End Cap")+" "+QString(barrel_ec>0?"A":"C");
      // l << "   Phi module: "+QString::number(idhelper->phi_module(id));
      // l << "   "+QString(barrel?"Layer":"Wheel")+": "+QString::number(idhelper->layer_or_wheel(id));
      // l << "   Straw Layer: "+QString::number(idhelper->straw_layer(id));
      // l << "   Straw: "+QString::number(idhelper->straw(id));
    }
    // l << "  High Level: "+QString(m_data->highLevel()?"Yes":"No");
    // l << "  Time over Threshold (Gaudi::Units::ns): "+QString::number(m_data->timeOverThreshold());
    
  }
  return ll;
}

//____________________________________________________________________
SoNode * VP1RawDataHandle_MDT_RDO::buildShape()
{
  const MuonGM::MdtReadoutElement * elem = element();
  Identifier id(m_data->identify());
  double tubeLength = elem->tubeLength(id);
  // double strawlength = elem ? elem->strawLength() : 200.0;

  SoNode * node = common()->nodeManager()->getShapeNode_DriftTube(tubeLength/2, elem->innerTubeRadius());
  // SoNode * node = common()->nodeManager()->getShapeNode_Wire(0.5,0.0/*0 radius for line*/);
  // if (highThreshold() && static_cast<VP1RawDataColl_MDT_RDO*>(coll())->useSpecialHTMat()) {
  //   SoGroup * gr = new SoGroup;
  //   gr->addChild(coll()->common()->controller()->trtHTMaterial());
  //   gr->addChild(node);
  //   return gr;
  // } else {
  //   return node;
  // }
    return node;
}

//____________________________________________________________________
SoTransform * VP1RawDataHandle_MDT_RDO::buildTransform()
{
  const MuonGM::MdtReadoutElement * elem = element();
  if (!elem)
    return new SoTransform;//fixme
  Identifier id(m_data->identify());
  return VP1LinAlgUtils::toSoTransform(elem->localToGlobalTransf(id));
}

// //____________________________________________________________________
// int VP1RawDataHandle_MDT_RDO::strawID() const
// {
//   const TRT_ID * idhelper = VP1DetInfo::trtIDHelper();
//   if (!idhelper)
//     return -1;
//   const int s = idhelper->straw(m_data->identify());
//   return (s <= 28 && s >= 0) ? s : -1;
// }

//____________________________________________________________________
const MuonGM::MdtReadoutElement * VP1RawDataHandle_MDT_RDO::element() const
{
  
  const MuonGM::MuonDetectorManager * detmgr = VP1DetInfo::muonDetMgr();

//   SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> detMgrKey{"MuonDetectorManager"};
// detMgrKey.initialize().ignore();
// SG::ReadCondHandle<MuonGM::MuonDetectorManager> detMgr{detMgrKey};


  if (!detmgr)
    return 0;
  return detmgr->getMdtReadoutElement(m_data->identify());
}

// //____________________________________________________________________
// VP1RawDataFlags::InDetPartsFlags VP1RawDataHandle_MDT_RDO::inInDetParts() const
// {
//   const TRT_ID * idhelper = VP1DetInfo::trtIDHelper();
//   if (!idhelper)
//     return VP1RawDataFlags::All;
//   switch (idhelper->barrel_ec(m_data->identify())) {
//   case -2:
//     return VP1RawDataFlags::EndCapNegative;
//   case 2:
//     return VP1RawDataFlags::EndCapPositive;
//   case -1:
//     return VP1RawDataFlags::BarrelNegative;
//   case 1:
//     return VP1RawDataFlags::BarrelPositive;
//   default:
//     return VP1RawDataFlags::All;
//   }
// }

// //____________________________________________________________________
// bool VP1RawDataHandle_MDT_RDO::highThreshold() const
// {
//   return m_data->highLevel();
// }

// //____________________________________________________________________
// double VP1RawDataHandle_MDT_RDO::timeOverThreshold() const
// {
//   return m_data->timeOverThreshold()*Gaudi::Units::nanosecond;//According to the comments, returned value is in Gaudi::Units::ns.
// }
