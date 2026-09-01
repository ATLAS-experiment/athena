/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class VP1RawDataHandle_MM_RDO          //
//                                                            //
//  Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch> 
//  Initial version: January 2024    
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1RawDataSystems/VP1RawDataHandle_MM_RDO.h"
#include "VP1RawDataSystems/VP1RawDataColl_MM_RDO.h"

#include "VP1RawDataSystems/VP1RawDataCommonData.h"
#include "VP1RawDataSystems/VP1RawDataSysController.h"

#include "VP1Utils/HitsSoNodeManager.h"
#include "VP1Utils/VP1DetInfo.h"
#include "VP1Utils/VP1LinAlgUtils.h"

#include "VP1Base/VP1Msg.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"

#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoMaterial.h>

#include "GaudiKernel/SystemOfUnits.h"

//____________________________________________________________________
VP1RawDataHandle_MM_RDO::VP1RawDataHandle_MM_RDO(VP1RawDataCollBase* coll,const MmDigit*data)
  : VP1RawDataHandleBase(coll), m_data(data)
{
}

//____________________________________________________________________
VP1RawDataHandle_MM_RDO::~VP1RawDataHandle_MM_RDO()
{
}

//____________________________________________________________________
QStringList VP1RawDataHandle_MM_RDO::clicked(bool verbose) const
{
  static const MmIdHelper * idhelper = VP1DetInfo::mmIDHelper();
  const MuonGM::MMReadoutElement * elem = element();

  QStringList ll;
  ll << " ===> MM Digit data";
  // ll << "   is a valid digit: "+ m_data->is_valid(idhelper);
  ll << "   stripResponseTime: " + QString::number(m_data->stripResponseTime());
  ll << "   stripResponseCharge: " + QString::number(m_data->stripResponseCharge());
  if (verbose)
  {
    if (idhelper)
    {
      // Fixme: We should have common identify -> QStringList method in VP1DetInfo!!
      Identifier id(m_data->identify());

      Amg::Vector3D globalPos;
      double length = 0, angle = 0;
      int channel = idhelper->channel(id);
      const MuonGM::MuonChannelDesign *design = elem->getDesign(id);
      elem->stripGlobalPosition(id, globalPos);
      length = design->channelLength(channel);
      angle = design->stereoAngle();
    }
  }
  return ll;
}

//____________________________________________________________________
SoNode * VP1RawDataHandle_MM_RDO::buildShape()
{
  const MuonGM::MMReadoutElement* elem = element();
  Identifier id(m_data->identify());

  static const MmIdHelper* idhelper = VP1DetInfo::mmIDHelper();
  if (!idhelper) {
    VP1Msg::messageDebug(
        "MM idhelper is null. Returning without building the shape for MM "
        "digits...");
    SoNode* node = common()->nodeManager()->getShapeNode_Point();
    return node;
  }

  VP1Msg::messageDebug("Building MM digit strip...");
  const MuonGM::MuonChannelDesign* design = elem->getDesign(id);

  double striplength{0.}, stripWidth{0.};
  int channel = idhelper->channel(id);

  Amg::Vector2D locPos;
  elem->stripPosition(id, locPos);

  striplength = design->channelLength(channel);
  stripWidth = design->inputWidth;

  SoNode* node = common()->nodeManager()->getShapeNode_Strip(
      striplength, std::min(10.0, stripWidth), 0.01);

  return node;
}

//____________________________________________________________________
SoTransform * VP1RawDataHandle_MM_RDO::buildTransform()
{
  const MuonGM::MMReadoutElement * elem = element();
  if (!elem)
    return new SoTransform;//fixme
  Identifier id(m_data->identify());
  
  Amg::Vector3D globalPos; 
  elem->stripGlobalPosition(id, globalPos);

  return VP1LinAlgUtils::toSoTransform(Amg::Transform3D(globalPos));
}

//____________________________________________________________________
const MuonGM::MMReadoutElement * VP1RawDataHandle_MM_RDO::element() const
{
  
  const MuonGM::MuonDetectorManager * detmgr = VP1DetInfo::muonDetMgr();

  if (!detmgr)
    return 0;
  return detmgr->getMMReadoutElement(m_data->identify());
}
