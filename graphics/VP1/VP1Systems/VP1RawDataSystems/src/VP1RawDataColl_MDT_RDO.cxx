/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class VP1RawDataColl_MDT_RDO            //
//                                                            //
//  Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi>
//  Initial version: January 2024                             //
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1RawDataSystems/VP1RawDataCommonData.h"
#include "VP1RawDataSystems/VP1RawDataSysController.h"
#include "VP1RawDataSystems/VP1RawDataColl_MDT_RDO.h"
#include "VP1RawDataSystems/VP1RawDataHandle_MDT_RDO.h"

#include "VP1Utils/VP1JobConfigInfo.h"
#include "VP1Base/VP1MaterialButton.h"
#include "VP1Utils/VP1SGContentsHelper.h"
#include "VP1Utils/VP1SGAccessHelper.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"

#include "StoreGate/ReadCondHandle.h"

//____________________________________________________________________
QStringList VP1RawDataColl_MDT_RDO::availableCollections(IVP1System*sys)
{
  return VP1SGContentsHelper(sys).getKeys<MdtDigitContainer>();
}

//____________________________________________________________________
class VP1RawDataColl_MDT_RDO::Imp {
public:
  // VP1Interval allowedToT;
  // bool requireHT = false;
  // bool useSpecialTRTHTMaterial = false;
};


//____________________________________________________________________
VP1RawDataColl_MDT_RDO::VP1RawDataColl_MDT_RDO(VP1RawDataCommonData*common,const QString& key)
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
VP1RawDataColl_MDT_RDO::~VP1RawDataColl_MDT_RDO()
{
  delete m_d;
}

//____________________________________________________________________
void VP1RawDataColl_MDT_RDO::assignDefaultMaterial(SoMaterial*m) const
{
  VP1MaterialButton::setMaterialParametersFromRGB( m, 255, 136, 0, 0.1 );
}

//____________________________________________________________________
bool VP1RawDataColl_MDT_RDO::load() {
  if (!VP1JobConfigInfo::hasMuonGeometry()) {
    message("Muon geometry not configured in job");
    return false;
  }

  //   // From:
  //   https://acode-browser1.usatlas.bnl.gov/lxr/source/athena/MuonSpectrometer/MuonCnv/MuonByteStreamCnvTest/src/ReadMdtDigit.cxx#0055
  // std::string key = "MDT_DIGITS";
  //      SG::ReadHandle<Muon::MdtDigitContainer> hndl(key);
  //      const MdtDigitContainer* mdt_container = hndl.get();
  //      ATH_CHECK(mdt_container != nullptr);

  // SG::ReadCondHandle<MuonGM::MuonDetectorManager>
  // detMgr{"MuonDetectorManager"};

  // get the detMgr
  // TODO: we should probably simplify by moving to:
  //   const MuonGM::MuonDetectorManager * detmgr = VP1DetInfo::muonDetMgr();
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> detMgrKey{
      "MuonDetectorManager"};
  detMgrKey.initialize().ignore();
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detMgr{detMgrKey};

  SG::ReadHandleKey<MdtDigitContainer> digitContainerKey{"MDT_DIGITS"};
  digitContainerKey.initialize().ignore();

  const EventContext& ctx = Gaudi::Hive::currentContext();

  SG::ReadHandle<MdtDigitContainer> container(digitContainerKey, ctx);
  if (!container.isValid()) {
    std::cout << "Could not find MdtDigitContainer called " << container.name()
              << " in store " << container.store() << std::endl;
    // return StatusCode::SUCCESS;
  }
  //  ATH_MSG_DEBUG("Found MdtDigitContainer called " << container.name() << "
  //  in store " << container.store());

  // ATH_MSG_DEBUG("****** mdt->size() : " << mdt_container->size());

  // MuonDetectorManager from the conditions store
  //  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> detMgr{this,
  //  "DetectorManagerKey", "MuonDetectorManager", "Key of input
  //  MuonDetectorManager condition data"};

  //  // MuonDetectorManager from the conditions store
  //          SG::ReadCondHandleKey<MuonGM::MuonDetectorManager>
  //          m_DetectorManagerKey{this, "DetectorManagerKey",
  //          "MuonDetectorManager",
  //                                                                                  "Key of input MuonDetectorManager condition data"};
  //          SG::ReadCondHandle<MuonGM::MuonDetectorManager>
  //          DetectorManagerHandle{m_DetectorManagerKey};

  // Iterate on the collections
  for (const MdtDigitCollection* coll : *container) {
    /// Iterate on the digits of the collection
    for (const MdtDigit* digit : *coll) {
      const Identifier digitId{digit->identify()};

      // example from Johannes
      /*
      mdtReadoutElement = detMgr->getMdtReadoutElement(digitId);
      const Amg::Vector3D tubePos = mdtReadoutElement->tubePos(digitId);
      */

      if (digit)
        addHandle(new VP1RawDataHandle_MDT_RDO(this, digit));
    }
  }

  //   recheckCutStatusOfAllHandles();
  return true;
}

//____________________________________________________________________
bool VP1RawDataColl_MDT_RDO::cut(VP1RawDataHandleBase* handle)
{

  // if (m_d->requireHT && !static_cast<VP1RawDataHandle_TRT_RDO*>(handle)->highThreshold())
  //   return false;
  // return m_d->allowedToT.contains(static_cast<VP1RawDataHandle_TRT_RDO*>(handle)->timeOverThreshold());
  VP1Msg::message("cut returning 'true'...");
  return true;
}
