/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
  bool excludeMasked = false;
  VP1Interval mdtAllowedADC;
  VP1Interval mdtAllowedTDC;

};


//____________________________________________________________________
VP1RawDataColl_MDT_RDO::VP1RawDataColl_MDT_RDO(VP1RawDataCommonData*common,const QString& key)
  : VP1RawDataCollBase(common,key), m_d(new Imp)
{
  connect(common->controller(), SIGNAL(mdtAllowedADCChanged(const VP1Interval&)),
          this, SLOT(setMdtAllowedADC(const VP1Interval&)));
  setMdtAllowedADC(common->controller()->mdtAllowedADC());
  connect(common->controller(), SIGNAL(mdtAllowedTDCChanged(const VP1Interval&)),
          this, SLOT(setMdtAllowedTDC(const VP1Interval&)));
  setMdtAllowedTDC(common->controller()->mdtAllowedTDC());
  connect(common->controller(), SIGNAL(mdtExcludeMaskedChanged(bool)),
          this, SLOT(setMdtExcludeMasked(bool)));
  setMdtExcludeMasked(common->controller()->mdtExcludeMasked());
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
  }

  // Iterate on the collections
  for (const MdtDigitCollection* coll : *container) {
    /// Iterate on the digits of the collection
    for (const MdtDigit* digit : *coll) {
      if (!digit)[[unlikely]] continue;
      addHandle(new VP1RawDataHandle_MDT_RDO(this, digit));
    }
  }

  return true;
}

//____________________________________________________________________
bool VP1RawDataColl_MDT_RDO::cut(VP1RawDataHandleBase* handle)
{
  const auto* digit = static_cast<const VP1RawDataHandle_MDT_RDO*>(handle);
  return (!m_d->excludeMasked || !digit->isMasked()) &&
         m_d->mdtAllowedADC.contains(digit->amplitude()) &&
         m_d->mdtAllowedTDC.contains(digit->time());
}

void VP1RawDataColl_MDT_RDO::setMdtAllowedADC(const VP1Interval& range)
{
  if (m_d->mdtAllowedADC == range)
    return;
  m_d->mdtAllowedADC = range;
  recheckCutStatusOfAllHandles();
}

void VP1RawDataColl_MDT_RDO::setMdtAllowedTDC(const VP1Interval& range)
{
  if (m_d->mdtAllowedTDC == range)
    return;
  m_d->mdtAllowedTDC = range;
  recheckCutStatusOfAllHandles();
}

void VP1RawDataColl_MDT_RDO::setMdtExcludeMasked(bool exclude)
{
  if (m_d->excludeMasked == exclude)
    return;
  m_d->excludeMasked = exclude;
  if (exclude)
    recheckCutStatusOfAllVisibleHandles();
  else
    recheckCutStatusOfAllNotVisibleHandles();
}
