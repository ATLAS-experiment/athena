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
#include "VP1RawDataSystems/VP1RawDataHandle_TRT_RDO.h"
#include "VP1Utils/VP1JobConfigInfo.h"
#include "VP1Base/VP1MaterialButton.h"
#include "VP1Utils/VP1SGContentsHelper.h"
#include "VP1Utils/VP1SGAccessHelper.h"
// #include "InDetRawData/TRT_RDO_Container.h"
// #include "InDetRawData/TRT_LoLumRawData.h"

//____________________________________________________________________
QStringList VP1RawDataColl_MDT_RDO::availableCollections(IVP1System*sys)
{
  return VP1SGContentsHelper(sys).getKeys<MdtCsmContainer>();
}

//____________________________________________________________________
class VP1RawDataColl_MDT_RDO::Imp {
public:
  VP1Interval allowedToT;
  bool requireHT = false;
  bool useSpecialTRTHTMaterial = false;
};


//____________________________________________________________________
VP1RawDataColl_MDT_RDO::VP1RawDataColl_MDT_RDO(VP1RawDataCommonData*common,const QString& key)
  : VP1RawDataCollBase(common,key), m_d(new Imp)
{
  m_d->requireHT = true;
  m_d->useSpecialTRTHTMaterial = true;
  m_d->allowedToT = VP1Interval();

  connect(common->controller(),SIGNAL(trtAllowedToTChanged(const VP1Interval&)),
	  this,SLOT(setAllowedToT(const VP1Interval&)));
  setAllowedToT(common->controller()->trtAllowedToT());

  connect(common->controller(),SIGNAL(trtRequireHTChanged(bool)),
	  this,SLOT(setRequireHT(bool)));
  setRequireHT(common->controller()->trtRequireHT());

  connect(common->controller(),SIGNAL(useSpecialTRTHTMaterialChanged(bool)),
	  this,SLOT(setUseSpecialTRTHTMaterial(bool)));
  setUseSpecialTRTHTMaterial(common->controller()->useSpecialTRTHTMaterial());
}

//____________________________________________________________________
VP1RawDataColl_MDT_RDO::~VP1RawDataColl_MDT_RDO()
{
  delete m_d;
}

//____________________________________________________________________
void VP1RawDataColl_MDT_RDO::assignDefaultMaterial(SoMaterial*m) const
{
  VP1MaterialButton::setMaterialParameters( m, 0.85, 0.85, 0.85, 0.1 );
}

//____________________________________________________________________
bool VP1RawDataColl_MDT_RDO::load()
{
  if (!VP1JobConfigInfo::hasMuonGeometry()) {
    message("TRT geometry not configured in job");
    return false;
  }
  
//   // From: https://acode-browser1.usatlas.bnl.gov/lxr/source/athena/MuonSpectrometer/MuonCnv/MuonByteStreamCnvTest/src/ReadMdtDigit.cxx#0055
// std::string key = "MDT_DIGITS";
//      SG::ReadHandle<Muon::MdtDigitContainer> hndl(key);
//      const MdtDigitContainer* mdt_container = hndl.get();
//      ATH_CHECK(mdt_container != nullptr);


    SG::ReadConHandle<MuonGM::MuonDetectorManager> detMgr{"MuonDetectorManager"};
     ATH_MSG_DEBUG("****** mdt->size() : " << mdt_container->size());

// Access by Collection
     for (const MdtCollection* coll : *mdt_container) {
          for (const MdtDigit* digit : *coll) {
              const Identifier digitId{digit->identify()};
              mdtReadoutElement = detMgr->getMdtReadoutElement(digitId);
              const Amg::Vector3D tubePos = mdtReadoutElement->tubePos(digitId);
      if (MdtDigit)
	      addHandle(new VP1RawDataHandle_MDT_RDO(this,mdtReadoutElement));
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
  return;
}

// //____________________________________________________________________
// void VP1RawDataColl_MDT_RDO::setAllowedToT(const VP1Interval& i)
// {
//   if (m_d->allowedToT==i)
//     return;
//   bool relaxed(i.contains(m_d->allowedToT));
//   bool tightened(m_d->allowedToT.contains(i));
//   m_d->allowedToT=i;
//   if (relaxed)
//     recheckCutStatusOfAllNotVisibleHandles();
//   else if (tightened)
//     recheckCutStatusOfAllVisibleHandles();
//   else
//     recheckCutStatusOfAllHandles();
// }

// //____________________________________________________________________
// void VP1RawDataColl_MDT_RDO::setRequireHT(bool b)
// {
//   if (m_d->requireHT==b)
//     return;
//   m_d->requireHT=b;
//   if (b)
//     recheckCutStatusOfAllVisibleHandles();
//   else
//     recheckCutStatusOfAllNotVisibleHandles();
// }

// //____________________________________________________________________
// void VP1RawDataColl_MDT_RDO::setUseSpecialTRTHTMaterial(bool b)
// {
//   if (m_d->useSpecialTRTHTMaterial==b)
//     return;
//   m_d->useSpecialTRTHTMaterial=b;

//   std::vector<VP1RawDataHandleBase*>::iterator it(getHandles().begin()),itE(getHandles().end());
//   for (;it!=itE;++it) {
//     if (static_cast<VP1RawDataHandle_TRT_RDO*>(*it)->highThreshold())
//       (*it)->update3DObjects();
//   }

// }

// //____________________________________________________________________
// bool VP1RawDataColl_MDT_RDO::useSpecialHTMat()
// {
//   return m_d->useSpecialTRTHTMaterial;
// }
