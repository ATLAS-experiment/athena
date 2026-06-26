/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

#include "VP1Base/VP1Msg.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"

#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoMaterial.h>

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

  QStringList ll;
  ll << " ===> sTGC Digit data";
  ll << "   type: "+ QString::fromStdString(m_channelTypeStr);
  ll << "   is_valid(idhelper): " + QString::number(m_data->is_valid(idhelper));
  ll << "   charge: " + QString::number(m_data->charge());
  ll << "   charge_6bit: " + QString::number(m_data->charge_6bit());
  ll << "   charge_10bit: " + QString::number(m_data->charge_10bit());
  ll << "   time: " + QString::number(m_data->time());
  ll << "   isDead: " + QString::number(m_data->isDead());
  ll << "   isPileup: " + QString::number(m_data->isPileup());
  if (verbose)
  {
    if (idhelper)
    {
      // Fixme: We should have common identify -> QStringList method in VP1DetInfo!!
      Identifier id(m_data->identify());
    }
  }
  return ll;
}

//____________________________________________________________________
SoNode * VP1RawDataHandle_sTGC_RDO::buildShape()
{

  SoNode * node{nullptr};


  const MuonGM::sTgcReadoutElement * elem = element();
  Identifier id(m_data->identify());

  static const sTgcIdHelper * idhelper = VP1DetInfo::stgcIDHelper();
  const int channel = idhelper->channel(id);
  m_channelType = idhelper->channelType(id);

  // element's local and global positions
  Amg::Vector3D globalPos{Amg::Vector3D::Zero()}; 
  Amg::Vector2D pos{Amg::Vector2D::Zero()}; 
  elem->stripGlobalPosition(id, globalPos);
  elem->stripPosition(id, pos);
  
  // sTGC element's geometry
  double shortWidth=0, longWidth=0, length=0;
  const Trk::PlaneSurface surface = elem->surface(id);

  // PAD geometry
  if (m_channelType == 0) {
    m_channelTypeStr = "PAD";
    length = elem->channelPitch(id);  // Height of a pad
    std::array<Amg::Vector2D, 4> corners{
        make_array<Amg::Vector2D, 4>(Amg::Vector2D::Zero())};
    elem->padCorners(id, corners);  // BotLeft, BotRight, TopLeft, TopRight
    shortWidth = (corners.at(1) - corners.at(0)).norm();
    longWidth = (corners.at(3) - corners.at(2)).norm();

    node = common()->nodeManager()->getShapeNode_Pad(length,shortWidth,longWidth,10*Gaudi::Units::mm/*Dummy depth*/); 
  }
  // STRIP geometry
  else if (m_channelType == 1) {
    m_channelTypeStr = "STRIP";
    const MuonGM::MuonChannelDesign* design = elem->getDesign(id);
    length = design->channelLength(channel);
    shortWidth = elem->channelPitch(id);  // Full pitch of strips
    longWidth = shortWidth;

    node = common()->nodeManager()->getShapeNode_Strip(length,longWidth,10*Gaudi::Units::mm/*Dummy depth*/); 
  }
  // WIRE geometry
  else {
    m_channelTypeStr = "WIRE";
    const MuonGM::MuonChannelDesign* design = elem->getDesign(id);
    if (!design) {
      // VP1Msg::messageVerbose("No wire design for hit " + id.getString());
      std::cout << "No wire design for hit " << id << std::endl;;
      return node;
    }

    // recalculate length and globalPos for wires, because
    // design->channelLength(channel) doesn't look sensible
    double fulllength = design->xSize();
    double locY = design->firstPos() +
                  (channel - 1) * design->inputPitch * design->groupWidth;
    if (std::abs(locY) > 0.5 * design->minYSize()) {  // triangle region
      double dY = 0.5 * (design->maxYSize() - design->minYSize());
      length = (0.5 * design->maxYSize() - std::abs(locY)) / dY * fulllength;
      if (std::abs(locY) > 873) {  // trapezoid region in the outter most part
                                   // of the large sector
        length += 0.5 * fulllength;
      }
      elem->surface(id).localToGlobal(
          Amg::Vector2D(pos.x(), pos.y() + 0.5 * (fulllength - length)),
          Amg::Vector3D::Zero(), globalPos);
    } else {  // rectangular region
      length = fulllength;
    }

    shortWidth = elem->channelPitch(id);  // Width of a full wire group
    longWidth = shortWidth;

    node = common()->nodeManager()->getShapeNode_Wire(length,longWidth,10*Gaudi::Units::mm/*Dummy depth*/);
  }
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

  return VP1LinAlgUtils::toSoTransform(Amg::Transform3D(globalPos));
}

//____________________________________________________________________
const MuonGM::sTgcReadoutElement * VP1RawDataHandle_sTGC_RDO::element() const
{
  
  const MuonGM::MuonDetectorManager * detmgr = VP1DetInfo::muonDetMgr();

  if (!detmgr)
    return 0;
  return detmgr->getsTgcReadoutElement(m_data->identify());
}
