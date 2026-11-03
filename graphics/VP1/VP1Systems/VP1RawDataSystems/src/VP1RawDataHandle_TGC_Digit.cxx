/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "VP1RawDataSystems/VP1RawDataHandle_TGC_Digit.h"
#include "VP1RawDataSystems/VP1RawDataCommonData.h"
#include "VP1Utils/VP1DetInfo.h"
#include "VP1Utils/VP1LinAlgUtils.h"
#include "VP1Utils/HitsSoNodeManager.h"
#include "MuonDigitContainer/TgcDigit.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include <Inventor/nodes/SoTransform.h>

VP1RawDataHandle_TGC_Digit::VP1RawDataHandle_TGC_Digit(
    VP1RawDataCollBase* collection, const TgcDigit* data)
  : VP1RawDataHandleBase(collection), m_data(data)
{}

const MuonGM::TgcReadoutElement* VP1RawDataHandle_TGC_Digit::element() const
{
  const auto* helper = VP1DetInfo::tgcIDHelper();
  const auto* manager = VP1DetInfo::muonDetMgr();
  return manager && helper && helper->valid(m_data->identify())
      ? manager->getTgcReadoutElement(m_data->identify()) : nullptr;
}

QStringList VP1RawDataHandle_TGC_Digit::clicked(bool verbose) const
{
  const Identifier id = m_data->identify();
  const auto* helper = VP1DetInfo::tgcIDHelper();
  const bool valid = helper && helper->valid(id);
  QStringList lines;
  lines << " ===> TGC Digit data";
  lines << "   Identifier: " + QString::fromStdString(id.getString());
  lines << "   Channel type: " + (valid ? QString(helper->isStrip(id) ? "Strip" : "Wire gang")
                                         : QString("unavailable"));
  QString bc;
  switch (m_data->bcTag()) {
  case TgcDigit::BC_UNDEFINED: bc = "undefined"; break;
  case TgcDigit::BC_PREVIOUS: bc = "previous"; break;
  case TgcDigit::BC_CURRENT: bc = "current"; break;
  case TgcDigit::BC_NEXT: bc = "next"; break;
  case TgcDigit::BC_NEXTNEXT: bc = "next-next"; break;
  default: bc = "unknown"; break;
  }
  lines << QString("   BC tag: %1 (%2)").arg(m_data->bcTag()).arg(bc);
  if (verbose) {
    if (valid) {
      lines << "   Station name: " + QString::fromStdString(helper->stationNameString(id));
      lines << "   Station eta index: " + QString::number(helper->stationEta(id));
      lines << "   Station phi index: " + QString::number(helper->stationPhi(id));
      lines << "   Gas gap: " + QString::number(helper->gasGap(id));
      lines << "   Channel: " + QString::number(helper->channel(id));
    } else {
      lines << "   Decoded identifier unavailable";
    }
    const auto* elem = element();
    if (elem) {
      const Amg::Vector3D position = elem->channelPos(id);
      lines << QString("   Global position [mm] (x, y, z): (%1, %2, %3)")
                   .arg(position.x(), 0, 'g', 10).arg(position.y(), 0, 'g', 10)
                   .arg(position.z(), 0, 'g', 10);
    } else {
      lines << "   Global position unavailable";
    }
  }
  return lines;
}

SoNode* VP1RawDataHandle_TGC_Digit::buildShape()
{
  const auto* elem = element();
  if (!elem)
    return common()->nodeManager()->getShapeNode_Point();
  const auto* helper = VP1DetInfo::tgcIDHelper();
  const Identifier id = m_data->identify();
  const int gap = helper->gasGap(id);
  const int channel = helper->channel(id);
  const bool strip = helper->isStrip(id);
  const double length = strip ? elem->stripLength() : elem->gangCentralWidth(gap, channel);
  const double width = strip ? elem->stripWidth(gap, channel) : elem->gangRadialLength(gap, channel);
  // The box represents the channel footprint; depth is a display thickness in mm.
  return common()->nodeManager()->getShapeNode_Strip(length, width, 0.1);
}

SoTransform* VP1RawDataHandle_TGC_Digit::buildTransform()
{
  const auto* elem = element();
  if (!elem)
    return new SoTransform;
  const Identifier id = m_data->identify();
  Amg::Transform3D transform = elem->localToGlobalTransf(id);
  // VP1 strip shapes follow local Y. Use the actual strip direction, including
  // its stereo angle; wire gangs follow the wire-plane X axis.
  const bool strip = VP1DetInfo::tgcIDHelper()->isStrip(id);
  const Amg::Vector3D along = strip ? elem->stripDir(id).normalized()
                                   : Amg::Vector3D(transform.linear().col(0));
  const Amg::Vector3D normal = transform.linear().col(2);
  transform.linear().col(0) = along.cross(normal).normalized();
  transform.linear().col(1) = along;
  transform.linear().col(2) = normal;
  transform.translation() = elem->channelPos(id);
  return VP1LinAlgUtils::toSoTransform(transform);
}

bool VP1RawDataHandle_TGC_Digit::isStrip() const
{
  const auto* helper = VP1DetInfo::tgcIDHelper();
  return helper && helper->valid(m_data->identify()) && helper->isStrip(m_data->identify());
}

unsigned VP1RawDataHandle_TGC_Digit::bcTag() const
{
  return m_data->bcTag();
}
