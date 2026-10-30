/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "VP1RawDataSystems/VP1RawDataHandle_RPC_Digit.h"
#include "VP1RawDataSystems/VP1RawDataCommonData.h"
#include "VP1Utils/VP1DetInfo.h"
#include "VP1Utils/VP1LinAlgUtils.h"
#include "VP1Utils/HitsSoNodeManager.h"
#include "MuonDigitContainer/RpcDigit.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "GaudiKernel/SystemOfUnits.h"
#include <Inventor/nodes/SoTransform.h>

VP1RawDataHandle_RPC_Digit::VP1RawDataHandle_RPC_Digit(
    VP1RawDataCollBase* collection, const RpcDigit* data)
  : VP1RawDataHandleBase(collection), m_data(data)
{}

const MuonGM::RpcReadoutElement* VP1RawDataHandle_RPC_Digit::element() const
{
  const auto* helper = VP1DetInfo::rpcIDHelper();
  const auto* manager = VP1DetInfo::muonDetMgr();
  return manager && helper && helper->valid(m_data->identify())
      ? manager->getRpcReadoutElement(m_data->identify()) : nullptr;
}

QStringList VP1RawDataHandle_RPC_Digit::clicked(bool verbose) const
{
  const Identifier id = m_data->identify();
  const auto* helper = VP1DetInfo::rpcIDHelper();
  const bool valid = helper && helper->valid(id);
  QStringList lines;
  lines << " ===> RPC Digit data";
  lines << "   Identifier: " + QString::fromStdString(id.getString());
  lines << "   Strip view: " + (valid ? QString(helper->measuresPhi(id) ? "Phi" : "Eta")
                                       : QString("unavailable"));
  lines << "   Time [ns]: " + QString::number(m_data->time() / Gaudi::Units::ns);
  lines << "   Time over threshold [ns]: " + QString::number(m_data->ToT() / Gaudi::Units::ns)
           + (m_data->ToT() < 0 ? " (not available)" : "");
  lines << "   stripSide: " + QString(m_data->stripSide() ? "true" : "false");
  if (verbose) {
    if (valid) {
      lines << "   Station name: " + QString::fromStdString(helper->stationNameString(id));
      lines << "   Station eta index: " + QString::number(helper->stationEta(id));
      lines << "   Station phi index: " + QString::number(helper->stationPhi(id));
      lines << "   Doublet R: " + QString::number(helper->doubletR(id));
      lines << "   Doublet Z: " + QString::number(helper->doubletZ(id));
      lines << "   Doublet phi: " + QString::number(helper->doubletPhi(id));
      lines << "   Gas gap: " + QString::number(helper->gasGap(id));
      lines << "   Strip: " + QString::number(helper->strip(id));
    } else {
      lines << "   Decoded identifier unavailable";
    }
    const auto* elem = element();
    if (elem) {
      const Amg::Vector3D position = elem->stripPos(id);
      lines << QString("   Global position [mm] (x, y, z): (%1, %2, %3)")
                   .arg(position.x() / Gaudi::Units::mm, 0, 'g', 10)
                   .arg(position.y() / Gaudi::Units::mm, 0, 'g', 10)
                   .arg(position.z() / Gaudi::Units::mm, 0, 'g', 10);
      const bool phi = helper->measuresPhi(id);
      lines << "   Strip length [mm]: " + QString::number(elem->StripLength(phi) / Gaudi::Units::mm);
      lines << "   Strip width [mm]: " + QString::number(elem->StripWidth(phi) / Gaudi::Units::mm);
    } else {
      lines << "   Global position unavailable";
    }
  }
  return lines;
}

SoNode* VP1RawDataHandle_RPC_Digit::buildShape()
{
  const auto* elem = element();
  if (!elem)
    return common()->nodeManager()->getShapeNode_Point();
  const bool phi = VP1DetInfo::rpcIDHelper()->measuresPhi(m_data->identify());
  // Strip footprint from geometry; depth is a display thickness in mm.
  return common()->nodeManager()->getShapeNode_Strip(
      elem->StripLength(phi), elem->StripWidth(phi), 0.1 * Gaudi::Units::mm);
}

SoTransform* VP1RawDataHandle_RPC_Digit::buildTransform()
{
  const auto* elem = element();
  if (!elem)
    return new SoTransform;
  const Identifier id = m_data->identify();
  // The tracking frame has X across the strip and Y along it, matching VP1.
  // Do not use localToGlobalTransf(), which follows AMDB conventions instead.
  Amg::Transform3D transform = elem->transform(id);
  transform.translation() = elem->stripPos(id);
  return VP1LinAlgUtils::toSoTransform(transform);
}
