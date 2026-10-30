/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "VP1RawDataSystems/VP1RawDataColl_RPC_Digit.h"
#include "VP1RawDataSystems/VP1RawDataCommonData.h"
#include "VP1RawDataSystems/VP1RawDataSysController.h"
#include "RpcDigitCuts.h"
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
  : VP1RawDataCollBase(common, key), m_cuts(std::make_unique<RpcDigitCuts>())
{
  connect(common->controller(), &VP1RawDataSysController::rpcShowEtaChanged,
          this, &VP1RawDataColl_RPC_Digit::setRpcShowEta);
  setRpcShowEta(common->controller()->rpcShowEta());
  connect(common->controller(), &VP1RawDataSysController::rpcShowPhiChanged,
          this, &VP1RawDataColl_RPC_Digit::setRpcShowPhi);
  setRpcShowPhi(common->controller()->rpcShowPhi());
  connect(common->controller(), &VP1RawDataSysController::rpcShowPrimarySideChanged,
          this, &VP1RawDataColl_RPC_Digit::setRpcShowPrimarySide);
  setRpcShowPrimarySide(common->controller()->rpcShowPrimarySide());
  connect(common->controller(), &VP1RawDataSysController::rpcShowOppositeSideChanged,
          this, &VP1RawDataColl_RPC_Digit::setRpcShowOppositeSide);
  setRpcShowOppositeSide(common->controller()->rpcShowOppositeSide());
  connect(common->controller(), &VP1RawDataSysController::rpcKeepMissingToTChanged,
          this, &VP1RawDataColl_RPC_Digit::setRpcKeepMissingToT);
  setRpcKeepMissingToT(common->controller()->rpcKeepMissingToT());
  connect(common->controller(), &VP1RawDataSysController::rpcAllowedTimeChanged,
          this, &VP1RawDataColl_RPC_Digit::setRpcAllowedTime);
  setRpcAllowedTime(common->controller()->rpcAllowedTime());
  connect(common->controller(), &VP1RawDataSysController::rpcAllowedToTChanged,
          this, &VP1RawDataColl_RPC_Digit::setRpcAllowedToT);
  setRpcAllowedToT(common->controller()->rpcAllowedToT());
}

VP1RawDataColl_RPC_Digit::~VP1RawDataColl_RPC_Digit() = default;

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

bool VP1RawDataColl_RPC_Digit::cut(VP1RawDataHandleBase* handle)
{
  const auto* digit = static_cast<const VP1RawDataHandle_RPC_Digit*>(handle);
  return m_cuts->accepts(digit->measuresPhi(), digit->time(), digit->timeOverThreshold(), digit->oppositeSide());
}

void VP1RawDataColl_RPC_Digit::setRpcShowEta(bool enabled)
{
  if (m_cuts->showEta == enabled)
    return;
  m_cuts->showEta = enabled;
  if (enabled)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

void VP1RawDataColl_RPC_Digit::setRpcShowPhi(bool enabled)
{
  if (m_cuts->showPhi == enabled)
    return;
  m_cuts->showPhi = enabled;
  if (enabled)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

void VP1RawDataColl_RPC_Digit::setRpcShowPrimarySide(bool enabled)
{
  if (m_cuts->showPrimarySide == enabled)
    return;
  m_cuts->showPrimarySide = enabled;
  if (enabled)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

void VP1RawDataColl_RPC_Digit::setRpcShowOppositeSide(bool enabled)
{
  if (m_cuts->showOppositeSide == enabled)
    return;
  m_cuts->showOppositeSide = enabled;
  if (enabled)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

void VP1RawDataColl_RPC_Digit::setRpcKeepMissingToT(bool enabled)
{
  if (m_cuts->keepMissingToT == enabled)
    return;
  m_cuts->keepMissingToT = enabled;
  if (enabled)
    recheckCutStatusOfAllNotVisibleHandles();
  else
    recheckCutStatusOfAllVisibleHandles();
}

void VP1RawDataColl_RPC_Digit::setRpcAllowedTime(const VP1Interval& range)
{
  if (m_cuts->timeRange == range)
    return;
  m_cuts->timeRange = range;
  recheckCutStatusOfAllHandles();
}

void VP1RawDataColl_RPC_Digit::setRpcAllowedToT(const VP1Interval& range)
{
  if (m_cuts->totRange == range)
    return;
  m_cuts->totRange = range;
  recheckCutStatusOfAllHandles();
}
