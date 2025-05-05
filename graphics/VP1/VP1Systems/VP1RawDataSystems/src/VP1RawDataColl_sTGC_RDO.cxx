/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
  // VP1Interval allowedToT;
  // bool requireHT = false;
  // bool useSpecialTRTHTMaterial = false;
};


//____________________________________________________________________
VP1RawDataColl_sTGC_RDO::VP1RawDataColl_sTGC_RDO(VP1RawDataCommonData*common,const QString& key)
  : VP1RawDataCollBase(common,key), m_d(new Imp)
{
  // m_d->requireHT = true;
  // m_d->useSpecialTRTHTMaterial = true;
  // m_d->allowedToT = VP1Interval();

  // connect(common->controller(),SIGNAL(useSpecialTRTHTMaterialChanged(bool)),
	//   this,SLOT(setUseSpecialTRTHTMaterial(bool)));
  // setUseSpecialTRTHTMaterial(common->controller()->useSpecialTRTHTMaterial());
}

//____________________________________________________________________
VP1RawDataColl_sTGC_RDO::~VP1RawDataColl_sTGC_RDO()
{
  delete m_d;
}

//____________________________________________________________________
void VP1RawDataColl_sTGC_RDO::assignDefaultMaterial(SoMaterial*m) const
{
  // VP1MaterialButton::setMaterialParametersFromRGB( m, 255, 136, 0, 0.1 );
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
    // return StatusCode::SUCCESS;
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

  // if (m_d->requireHT && !static_cast<VP1RawDataHandle_TRT_RDO*>(handle)->highThreshold())
  //   return false;
  // return m_d->allowedToT.contains(static_cast<VP1RawDataHandle_TRT_RDO*>(handle)->timeOverThreshold());
  VP1Msg::message("cut returning 'true'...");
  return true;
}
