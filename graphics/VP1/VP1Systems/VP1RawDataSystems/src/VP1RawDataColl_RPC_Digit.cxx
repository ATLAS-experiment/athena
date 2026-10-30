/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "VP1RawDataSystems/VP1RawDataColl_RPC_Digit.h"
#include "VP1RawDataSystems/VP1RawDataHandle_RPC_Digit.h"
#include "VP1Utils/VP1SGContentsHelper.h"
#include "VP1Utils/VP1JobConfigInfo.h"
#include "VP1Utils/VP1DetInfo.h"
#include "VP1Base/VP1MaterialButton.h"
#include "MuonDigitContainer/RpcDigitContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadHandle.h"

QStringList VP1RawDataColl_RPC_Digit::availableCollections(IVP1System* sys)
{
  return VP1SGContentsHelper(sys).getKeys<RpcDigitContainer>();
}

VP1RawDataColl_RPC_Digit::VP1RawDataColl_RPC_Digit(
    VP1RawDataCommonData* common, const QString& key)
  : VP1RawDataCollBase(common, key)
{}

void VP1RawDataColl_RPC_Digit::assignDefaultMaterial(SoMaterial* material) const
{
  VP1MaterialButton::setMaterialParametersFromRGB(material, 255, 190, 80, 0.1);
}

bool VP1RawDataColl_RPC_Digit::load()
{
  const auto* helper = VP1DetInfo::rpcIDHelper();
  const auto* manager = VP1DetInfo::muonDetMgr();
  if (!VP1JobConfigInfo::hasMuonGeometry() || !helper || !manager) {
    message("RPC geometry or identifier helper unavailable");
    return false;
  }
  SG::ReadHandleKey<RpcDigitContainer> key{text().toStdString()};
  if (key.initialize().isFailure())
    return false;
  SG::ReadHandle<RpcDigitContainer> container(key, Gaudi::Hive::currentContext());
  if (!container.isValid()) {
    message("Could not retrieve RPC digit container " + text());
    return false;
  }
  unsigned skipped = 0;
  for (const auto* collection : *container) {
    if (!collection)
      continue;
    for (const RpcDigit* digit : *collection) {
      if (!digit || !helper->valid(digit->identify()) ||
          !manager->getRpcReadoutElement(digit->identify())) {
        ++skipped;
        continue;
      }
      addHandle(new VP1RawDataHandle_RPC_Digit(this, digit));
    }
  }
  if (skipped)
    message(QString("Skipped %1 RPC digits with invalid identifiers or missing geometry").arg(skipped));
  return true;
}

bool VP1RawDataColl_RPC_Digit::cut(VP1RawDataHandleBase*)
{
  // Shared eta/phi cuts are applied by the base class at the strip centre.
  return true;
}
