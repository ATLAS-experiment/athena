/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class VP1RawDataColl_sTGC_RDO            //
//                                                            //
//  Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi>
//  Initial version: December 2024                             //
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1RawDataSystems/VP1RawDataCommonData.h"
#include "VP1RawDataSystems/VP1RawDataSysController.h"
#include "VP1RawDataSystems/VP1RawDataColl_sTGC_RDO.h"
#include "VP1RawDataSystems/VP1RawDataHandle_sTGC_RDO.h"

#include "VP1Utils/VP1JobConfigInfo.h"
#include "VP1Base/VP1MaterialButton.h"
#include "VP1Utils/VP1SGContentsHelper.h"
#include "VP1Utils/VP1SGAccessHelper.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"

#include "StoreGate/ReadCondHandle.h"

//____________________________________________________________________
QStringList VP1RawDataColl_sTGC_RDO::availableCollections(IVP1System*sys)
{
  return VP1SGContentsHelper(sys).getKeys<sTgcDigitContainer>();
}

//____________________________________________________________________
class VP1RawDataColl_sTGC_RDO::Imp {
public:
  bool stgcShowPads = true;
  bool stgcShowStrips = true;
  bool stgcShowWires = true;
  bool stgcExcludeDead = false;
  bool stgcExcludePileup = false;

  VP1Interval stgcAllowedCharge;
  VP1Interval stgcAllowedTime;

};


//____________________________________________________________________
VP1RawDataColl_sTGC_RDO::VP1RawDataColl_sTGC_RDO(VP1RawDataCommonData*common,const QString& key)
  : VP1RawDataCollBase(common,key), m_d(new Imp)
{
  connect(common->controller(), SIGNAL(stgcAllowedChargeChanged(const VP1Interval&)),
          this, SLOT(setStgcAllowedCharge(const VP1Interval&)));
  setStgcAllowedCharge(common->controller()->stgcAllowedCharge());
  connect(common->controller(), SIGNAL(stgcAllowedTimeChanged(const VP1Interval&)),
          this, SLOT(setStgcAllowedTime(const VP1Interval&)));
  setStgcAllowedTime(common->controller()->stgcAllowedTime());
  connect(common->controller(), SIGNAL(stgcShowPadsChanged(bool)),
          this, SLOT(setStgcShowPads(bool)));
  setStgcShowPads(common->controller()->stgcShowPads());
  connect(common->controller(), SIGNAL(stgcShowStripsChanged(bool)),
          this, SLOT(setStgcShowStrips(bool)));
  setStgcShowStrips(common->controller()->stgcShowStrips());
  connect(common->controller(), SIGNAL(stgcShowWiresChanged(bool)),
          this, SLOT(setStgcShowWires(bool)));
  setStgcShowWires(common->controller()->stgcShowWires());
  connect(common->controller(), SIGNAL(stgcExcludeDeadChanged(bool)),
          this, SLOT(setStgcExcludeDead(bool)));
  setStgcExcludeDead(common->controller()->stgcExcludeDead());
  connect(common->controller(), SIGNAL(stgcExcludePileupChanged(bool)),
          this, SLOT(setStgcExcludePileup(bool)));
  setStgcExcludePileup(common->controller()->stgcExcludePileup());

}

//____________________________________________________________________
VP1RawDataColl_sTGC_RDO::~VP1RawDataColl_sTGC_RDO()
{
  delete m_d;
}

//____________________________________________________________________
void VP1RawDataColl_sTGC_RDO::assignDefaultMaterial(SoMaterial*m) const
{
  VP1MaterialButton::setMaterialParameters( m, 0.42, 0.96, 0.16, 0.1 );
}

//____________________________________________________________________
bool VP1RawDataColl_sTGC_RDO::load() {
  if (!VP1JobConfigInfo::hasMuonGeometry()) {
    message("Muon geometry not configured in job");
    return false;
  }

  // get the detMgr
  // TODO: we should probably simplify by moving to:
  //   const MuonGM::MuonDetectorManager * detmgr = VP1DetInfo::muonDetMgr();
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> detMgrKey{
      "MuonDetectorManager"};
  detMgrKey.initialize().ignore();
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detMgr{detMgrKey};

  SG::ReadHandleKey<sTgcDigitContainer> digitContainerKey{"sTGC_DIGITS"};
  digitContainerKey.initialize().ignore();

  const EventContext& ctx = Gaudi::Hive::currentContext();

  SG::ReadHandle<sTgcDigitContainer> container(digitContainerKey, ctx);
  if (!container.isValid()) {
    std::cout << "Could not find sTgcDigitContainer called " << container.name()
              << " in store " << container.store() << std::endl;
  }

  // Iterate on the collections
  for (const sTgcDigitCollection* coll : *container) {
    /// Iterate on the digits of the collection
    for (const sTgcDigit* digit : *coll) {
      const Identifier digitId{digit->identify()};

      // example from Johannes
      /*
      mdtReadoutElement = detMgr->getMdtReadoutElement(digitId);
      const Amg::Vector3D tubePos = mdtReadoutElement->tubePos(digitId);
      */

      if (digit)
        addHandle(new VP1RawDataHandle_sTGC_RDO(this, digit));
    }
  }

  //   recheckCutStatusOfAllHandles();
  return true;
}

//____________________________________________________________________
bool VP1RawDataColl_sTGC_RDO::cut(VP1RawDataHandleBase* handle)
{
  const auto* digit = static_cast<const VP1RawDataHandle_sTGC_RDO*>(handle);
  if ((m_d->stgcExcludeDead && digit->isDead()) ||
      (m_d->stgcExcludePileup && digit->isPileup()))
    return false;
  if (!m_d->stgcShowPads && !m_d->stgcShowStrips && !m_d->stgcShowWires)
    return false;
  if (!(m_d->stgcShowPads && m_d->stgcShowStrips && m_d->stgcShowWires)) {
    const int type = digit->channelType();
    if (!((type == sTgcIdHelper::Pad && m_d->stgcShowPads) ||
          (type == sTgcIdHelper::Strip && m_d->stgcShowStrips) ||
          (type == sTgcIdHelper::Wire && m_d->stgcShowWires)))
      return false;
  }
  return m_d->stgcAllowedCharge.contains(digit->amplitude()) &&
         m_d->stgcAllowedTime.contains(digit->time());
}

void VP1RawDataColl_sTGC_RDO::setStgcAllowedCharge(const VP1Interval& range)
{
  if (m_d->stgcAllowedCharge == range)
    return;
  m_d->stgcAllowedCharge = range;
  recheckCutStatusOfAllHandles();
}

void VP1RawDataColl_sTGC_RDO::setStgcAllowedTime(const VP1Interval& range)
{
  if (m_d->stgcAllowedTime == range)
    return;
  m_d->stgcAllowedTime = range;
  recheckCutStatusOfAllHandles();
}

void VP1RawDataColl_sTGC_RDO::setStgcShowPads(bool enabled)
{
  if (m_d->stgcShowPads == enabled)
    return;
  m_d->stgcShowPads = enabled;
  recheckCutStatusOfAllHandles();
}

void VP1RawDataColl_sTGC_RDO::setStgcShowStrips(bool enabled)
{
  if (m_d->stgcShowStrips == enabled)
    return;
  m_d->stgcShowStrips = enabled;
  recheckCutStatusOfAllHandles();
}

void VP1RawDataColl_sTGC_RDO::setStgcShowWires(bool enabled)
{
  if (m_d->stgcShowWires == enabled)
    return;
  m_d->stgcShowWires = enabled;
  recheckCutStatusOfAllHandles();
}

void VP1RawDataColl_sTGC_RDO::setStgcExcludeDead(bool enabled)
{
  if (m_d->stgcExcludeDead == enabled)
    return;
  m_d->stgcExcludeDead = enabled;
  recheckCutStatusOfAllHandles();
}

void VP1RawDataColl_sTGC_RDO::setStgcExcludePileup(bool enabled)
{
  if (m_d->stgcExcludePileup == enabled)
    return;
  m_d->stgcExcludePileup = enabled;
  recheckCutStatusOfAllHandles();
}
