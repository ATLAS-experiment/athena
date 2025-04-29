/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class VP1RawDataHandle_sTGC_RDO          //
//                                                            //
//  Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch> 
//  Initial version: January 2025    
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1RawDataSystems/VP1RawDataHandle_sTGC_RDO.h"
#include "VP1RawDataSystems/VP1RawDataColl_sTGC_RDO.h"

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
VP1RawDataHandle_sTGC_RDO::VP1RawDataHandle_sTGC_RDO(VP1RawDataCollBase* coll,const sTgcDigit*data)
  : VP1RawDataHandleBase(coll), m_data(data)
{
}

//____________________________________________________________________
VP1RawDataHandle_sTGC_RDO::~VP1RawDataHandle_sTGC_RDO()
{
}

//____________________________________________________________________
QStringList VP1RawDataHandle_sTGC_RDO::clicked(bool verbose) const
{
  static const sTgcIdHelper * idhelper = VP1DetInfo::stgcIDHelper();
  const MuonGM::sTgcReadoutElement * elem = element();

  QStringList ll;
  ll << " ===> sTGC Digit data";
  // ll << "   is a valid digit: "+ m_data->is_valid(idhelper);
  ll << "   is_valid(idhelper): " + QString::number(m_data->is_valid(idhelper));
  ll << "   charge: " + QString::number(m_data->charge());
  ll << "   charge_6bit: " + QString::number(m_data->charge_6bit());
  ll << "   charge_10bit: " + QString::number(m_data->charge_10bit());
  ll << "   time: " + QString::number(m_data->time());
  ll << "   isDead: " + QString::number(m_data->isDead());
  ll << "   isPileup: " + QString::number(m_data->isPileup());
  if (verbose) {
    if (idhelper) {
      //Fixme: We should have common identify -> QStringList method in VP1DetInfo!!
      Identifier id(m_data->identify());

      // ll << "stripNumber: " << QString::number(elem->stripNumber());
         /// Returns whether the chamber is in the endcap
      // ll << "Is Endcap: " << QString::number(elem->endcap());


      Amg::Vector3D globalPos; 
         double length=0, angle=0;
         int channel = idhelper->channel(id);
         const MuonGM::MuonChannelDesign* design = elem->getDesign(id);
         elem->stripGlobalPosition(id, globalPos);
         length = design->channelLength(channel);
         angle = design->stereoAngle();

         
      // ll << "strip position X :" << elem->tubePos(id).position[Amg::x];
      // ll << "strip position X :" << globalPos[Amg::x];
      // ll << "strip length :" << length;
      // ll << "strip angle :" <<  angle;





      // std::cout << "tubePos x: " << elem->tubePos(id)[Amg::x] << std::endl;

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
SoNode * VP1RawDataHandle_sTGC_RDO::buildShape()
{
  const MuonGM::sTgcReadoutElement * elem = element();
  Identifier id(m_data->identify());

  static const sTgcIdHelper * idhelper = VP1DetInfo::stgcIDHelper();

Amg::Vector3D globalPos; 
         double length=0, angle=0;
         int channel = idhelper->channel(id);
         const MuonGM::MuonChannelDesign* design = elem->getDesign(id);
         elem->stripGlobalPosition(id, globalPos);
         length = design->channelLength(channel);


  // double tubeLength = elem->tubeLength(id);
  double tubeLength = length;
  // double strawlength = elem ? elem->strawLength() : 200.0;

  // SoNode * node = common()->nodeManager()->getShapeNode_DriftTube(tubeLength/2, elem->innerTubeRadius());
  SoNode * node = common()->nodeManager()->getShapeNode_Point();


  // SoNode * node = common()->nodeManager()->getShapeNode_Wire(0.5,0.0/*0 radius for line*/);
  // if (highThreshold() && static_cast<VP1RawDataColl_sTGC_RDO*>(coll())->useSpecialHTMat()) {
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
SoTransform * VP1RawDataHandle_sTGC_RDO::buildTransform()
{
  const MuonGM::sTgcReadoutElement * elem = element();
  if (!elem)
    return new SoTransform;//fixme
  Identifier id(m_data->identify());
  Amg::Vector3D globalPos; 
         elem->stripGlobalPosition(id, globalPos);

  // return VP1LinAlgUtils::toSoTransform(elem->localToGlobalTransf(id));
  // return VP1LinAlgUtils::toSoTransform(globalPos[Amg::x], globalPos[Amg::y], globalPos[Amg::z]);
  return VP1LinAlgUtils::toSoTransform(Amg::Transform3D(globalPos));
}

// //____________________________________________________________________
// int VP1RawDataHandle_sTGC_RDO::strawID() const
// {
//   const TRT_ID * idhelper = VP1DetInfo::trtIDHelper();
//   if (!idhelper)
//     return -1;
//   const int s = idhelper->straw(m_data->identify());
//   return (s <= 28 && s >= 0) ? s : -1;
// }

//____________________________________________________________________
const MuonGM::sTgcReadoutElement * VP1RawDataHandle_sTGC_RDO::element() const
{
  
  const MuonGM::MuonDetectorManager * detmgr = VP1DetInfo::muonDetMgr();

//   SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> detMgrKey{"MuonDetectorManager"};
// detMgrKey.initialize().ignore();
// SG::ReadCondHandle<MuonGM::MuonDetectorManager> detMgr{detMgrKey};


  if (!detmgr)
    return 0;
  return detmgr->getsTgcReadoutElement(m_data->identify());
}

// //____________________________________________________________________
// VP1RawDataFlags::InDetPartsFlags VP1RawDataHandle_sTGC_RDO::inInDetParts() const
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
// bool VP1RawDataHandle_sTGC_RDO::highThreshold() const
// {
//   return m_data->highLevel();
// }

// //____________________________________________________________________
// double VP1RawDataHandle_sTGC_RDO::timeOverThreshold() const
// {
//   return m_data->timeOverThreshold()*Gaudi::Units::nanosecond;//According to the comments, returned value is in Gaudi::Units::ns.
// }
