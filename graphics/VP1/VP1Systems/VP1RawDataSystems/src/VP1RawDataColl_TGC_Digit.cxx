/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "VP1RawDataSystems/VP1RawDataColl_TGC_Digit.h"
#include "VP1RawDataSystems/VP1RawDataSysController.h"
#include "VP1RawDataSystems/VP1RawDataCommonData.h"
#include "TgcDigitCuts.h"
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
  : VP1RawDataCollBase(common, key), m_cuts(std::make_unique<TgcDigitCuts>())
{
  connect(common->controller(), &VP1RawDataSysController::tgcShowStripsChanged,
          this, &VP1RawDataColl_TGC_Digit::setTgcShowStrips);
  setTgcShowStrips(common->controller()->tgcShowStrips());
  connect(common->controller(), &VP1RawDataSysController::tgcShowWiresChanged,
          this, &VP1RawDataColl_TGC_Digit::setTgcShowWires);
  setTgcShowWires(common->controller()->tgcShowWires());
  connect(common->controller(), &VP1RawDataSysController::tgcShowPreviousChanged,
          this, &VP1RawDataColl_TGC_Digit::setTgcShowPrevious);
  setTgcShowPrevious(common->controller()->tgcShowPrevious());
  connect(common->controller(), &VP1RawDataSysController::tgcShowCurrentChanged,
          this, &VP1RawDataColl_TGC_Digit::setTgcShowCurrent);
  setTgcShowCurrent(common->controller()->tgcShowCurrent());
  connect(common->controller(), &VP1RawDataSysController::tgcShowNextChanged,
          this, &VP1RawDataColl_TGC_Digit::setTgcShowNext);
  setTgcShowNext(common->controller()->tgcShowNext());
  connect(common->controller(), &VP1RawDataSysController::tgcShowNextNextChanged,
          this, &VP1RawDataColl_TGC_Digit::setTgcShowNextNext);
  setTgcShowNextNext(common->controller()->tgcShowNextNext());
  connect(common->controller(), &VP1RawDataSysController::tgcShowUndefinedChanged,
          this, &VP1RawDataColl_TGC_Digit::setTgcShowUndefined);
  setTgcShowUndefined(common->controller()->tgcShowUndefined());
  connect(common->controller(), &VP1RawDataSysController::tgcShowUnknownChanged,
          this, &VP1RawDataColl_TGC_Digit::setTgcShowUnknown);
  setTgcShowUnknown(common->controller()->tgcShowUnknown());
}

VP1RawDataColl_TGC_Digit::~VP1RawDataColl_TGC_Digit() = default;

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

bool VP1RawDataColl_TGC_Digit::cut(VP1RawDataHandleBase* handle)
{
  const auto* digit = static_cast<const VP1RawDataHandle_TGC_Digit*>(handle);
  return m_cuts->accepts(digit->isStrip(), digit->bcTag());
}

void VP1RawDataColl_TGC_Digit::setTgcShowStrips(bool enabled)
{
  if (m_cuts->showStrips == enabled)
    return;
  m_cuts->showStrips = enabled;
  if (enabled)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

void VP1RawDataColl_TGC_Digit::setTgcShowWires(bool enabled)
{
  if (m_cuts->showWires == enabled)
    return;
  m_cuts->showWires = enabled;
  if (enabled)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

void VP1RawDataColl_TGC_Digit::setTgcShowPrevious(bool enabled)
{
  if (m_cuts->showPrevious == enabled)
    return;
  m_cuts->showPrevious = enabled;
  if (enabled)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

void VP1RawDataColl_TGC_Digit::setTgcShowCurrent(bool enabled)
{
  if (m_cuts->showCurrent == enabled)
    return;
  m_cuts->showCurrent = enabled;
  if (enabled)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

void VP1RawDataColl_TGC_Digit::setTgcShowNext(bool enabled)
{
  if (m_cuts->showNext == enabled)
    return;
  m_cuts->showNext = enabled;
  if (enabled)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

void VP1RawDataColl_TGC_Digit::setTgcShowNextNext(bool enabled)
{
  if (m_cuts->showNextNext == enabled)
    return;
  m_cuts->showNextNext = enabled;
  if (enabled)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

void VP1RawDataColl_TGC_Digit::setTgcShowUndefined(bool enabled)
{
  if (m_cuts->showUndefined == enabled)
    return;
  m_cuts->showUndefined = enabled;
  if (enabled)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

void VP1RawDataColl_TGC_Digit::setTgcShowUnknown(bool enabled)
{
  if (m_cuts->showUnknown == enabled)
    return;
  m_cuts->showUnknown = enabled;
  if (enabled)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}
