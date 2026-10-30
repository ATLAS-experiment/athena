/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "VP1RawDataSystems/VP1RawDataColl_TGC_Digit.h"
#include "VP1RawDataSystems/VP1RawDataHandle_TGC_Digit.h"
#include "VP1Utils/VP1SGContentsHelper.h"
#include "VP1Utils/VP1JobConfigInfo.h"
#include "VP1Utils/VP1DetInfo.h"
#include "VP1Base/VP1MaterialButton.h"
#include "MuonDigitContainer/TgcDigitContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadHandle.h"

QStringList VP1RawDataColl_TGC_Digit::availableCollections(IVP1System* sys)
{
  return VP1SGContentsHelper(sys).getKeys<TgcDigitContainer>();
}

VP1RawDataColl_TGC_Digit::VP1RawDataColl_TGC_Digit(
    VP1RawDataCommonData* common, const QString& key)
  : VP1RawDataCollBase(common, key)
{}

void VP1RawDataColl_TGC_Digit::assignDefaultMaterial(SoMaterial* material) const
{
  VP1MaterialButton::setMaterialParametersFromRGB(material, 80, 180, 255, 0.1);
}

bool VP1RawDataColl_TGC_Digit::load()
{
  const auto* helper = VP1DetInfo::tgcIDHelper();
  const auto* manager = VP1DetInfo::muonDetMgr();
  if (!VP1JobConfigInfo::hasMuonGeometry() || !helper || !manager) {
    message("TGC geometry or identifier helper unavailable");
    return false;
  }
  SG::ReadHandleKey<TgcDigitContainer> key{text().toStdString()};
  if (key.initialize().isFailure())
    return false;
  SG::ReadHandle<TgcDigitContainer> container(key, Gaudi::Hive::currentContext());
  if (!container.isValid()) {
    message("Could not retrieve TGC digit container " + text());
    return false;
  }
  unsigned skipped = 0;
  for (const auto* collection : *container) {
    if (!collection)
      continue;
    for (const TgcDigit* digit : *collection) {
      if (!digit || !helper->valid(digit->identify()) ||
          !manager->getTgcReadoutElement(digit->identify())) {
        ++skipped;
        continue;
      }
      addHandle(new VP1RawDataHandle_TGC_Digit(this, digit));
    }
  }
  if (skipped)
    message(QString("Skipped %1 TGC digits with invalid identifiers or missing geometry").arg(skipped));
  return true;
}

bool VP1RawDataColl_TGC_Digit::cut(VP1RawDataHandleBase*)
{
  // Eta/phi cuts are applied by VP1RawDataCollBase using each channel centre.
  return true;
}
