/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/MsgStream.h"
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "TRTAlignCondAlg.h"


TRTAlignCondAlg::TRTAlignCondAlg(const std::string& name
				 , ISvcLocator* pSvcLocator )
  : ::AthAlgorithm(name,pSvcLocator)
  , m_detManager(nullptr)
{
}

TRTAlignCondAlg::~TRTAlignCondAlg()
= default;

StatusCode TRTAlignCondAlg::initialize()
{
  ATH_MSG_DEBUG("initialize " << name());

  // Read Condition Handles initialize
  ATH_CHECK( m_readKeyRegular.initialize(!m_useDynamicFolders.value()) );
  ATH_CHECK( m_readKeyDynamicGlobal.initialize(m_useDynamicFolders.value()) );
  ATH_CHECK( m_readKeyDynamicRegular.initialize(m_useDynamicFolders.value()) );

  // Write condition handles initialize
  ATH_CHECK( m_writeKeyAlignStore.initialize(!m_writeKeyAlignStore.empty()) );
  ATH_CHECK( m_writeKeyDetElCont.initialize() );

  ATH_CHECK(detStore()->retrieve(m_detManager,"TRT"));

  return StatusCode::SUCCESS;
}

StatusCode TRTAlignCondAlg::execute()
{
  ATH_MSG_DEBUG("execute " << name());

  const EventContext& ctx = Gaudi::Hive::currentContext();
  // ____________ Construct Write Cond Handles and check their validity ____________
  SG::WriteCondHandle<InDetDD::TRT_DetElementContainer> writeHandleDetElCont{m_writeKeyDetElCont, ctx};
  if (writeHandleDetElCont.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandleDetElCont.fullKey() << " is already valid."
                  << ". In theory this should not be called, but may happen"
                  << " if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }

  std::unique_ptr<SG::WriteCondHandle<GeoAlignmentStore>> writeHandleAlignStore{};
  if (!m_writeKeyAlignStore.empty()) {
    writeHandleAlignStore = std::make_unique<SG::WriteCondHandle<GeoAlignmentStore>>(m_writeKeyAlignStore, ctx);
    if (writeHandleAlignStore->isValid()) {
      ATH_MSG_DEBUG("CondHandle " << writeHandleAlignStore->fullKey() << " is already valid."
                    << ". In theory this should not be called, but may happen"
                    << " if multiple concurrent events are being processed out of order.");
      return StatusCode::SUCCESS;
    }
  }

  // ____________ Construct new Write Cond Object and its range ____________
  std::unique_ptr<GeoAlignmentStore> writeCdoAlignStore{std::make_unique<GeoAlignmentStore>()};

  // ____________ Get Read Cond Objects ____________
  // Container for passing read CDO-s over to the Detector Manager
  InDetDD::RawAlignmentObjects readCdoContainer;

  const InDetDD::TRT_DetElementCollection* unAlignedColl{m_detManager->getDetectorElementCollection()};
  if (unAlignedColl==nullptr) {
    ATH_MSG_FATAL("Null pointer is returned by getDetectorElementCollection()");
    return StatusCode::FAILURE;
  }

  if(m_useDynamicFolders) {
    // 1. Dynamic folders

    // ** Global
    SG::ReadCondHandle<CondAttrListCollection> readHandleDynamicGlobal{m_readKeyDynamicGlobal,ctx};
    // Get CDO and store it into container
    const CondAttrListCollection* readCdoDynamicGlobal{*readHandleDynamicGlobal};
    if(readCdoDynamicGlobal==nullptr) {
      ATH_MSG_ERROR("Null pointer to the read conditions object: Dynamic Global");
      return StatusCode::FAILURE;
    }
    readCdoContainer.emplace(m_readKeyDynamicGlobal.key(),readCdoDynamicGlobal);
    // Add depdency for IOV-intersection
    if (writeHandleAlignStore) writeHandleAlignStore->addDependency(readHandleDynamicGlobal);
    writeHandleDetElCont.addDependency(readHandleDynamicGlobal);

    // ** Regular
    SG::ReadCondHandle<AlignableTransformContainer> readHandleDynamicRegular{m_readKeyDynamicRegular,ctx};
    // Get CDO and store it into container
    const AlignableTransformContainer* readCdoDynamicRegular{*readHandleDynamicRegular};
    if(readCdoDynamicRegular==nullptr) {
      ATH_MSG_ERROR("Null pointer to the read conditions object: Dynamic Regular");
      return StatusCode::FAILURE;
    }
    readCdoContainer.emplace(m_readKeyDynamicRegular.key(),readCdoDynamicRegular);
    // Add depdency for IOV-intersection
    if (writeHandleAlignStore) writeHandleAlignStore->addDependency(readHandleDynamicRegular);
    writeHandleDetElCont.addDependency(readHandleDynamicRegular);
  }
  else {
    // 2. Regular folder
    SG::ReadCondHandle<AlignableTransformContainer> readHandleRegular{m_readKeyRegular,ctx};
    // Get CDO and store it into container
    const AlignableTransformContainer* readCdoRegular{*readHandleRegular};
    if(readCdoRegular==nullptr) {
      ATH_MSG_ERROR("Null pointer to the read conditions object: Regular");
      return StatusCode::FAILURE;
    }
    readCdoContainer.emplace(m_readKeyRegular.key(),readCdoRegular);
    //Add dependency for IOV-intersection
    if (writeHandleAlignStore) writeHandleAlignStore->addDependency(readHandleRegular);
    writeHandleDetElCont.addDependency(readHandleRegular);
  }

  // ____________ Apply alignments to TRT GeoModel ____________
  if(m_detManager->align(readCdoContainer, writeCdoAlignStore.get()).isFailure()) {
    ATH_MSG_ERROR("Failed to apply alignments to TRT");
    return StatusCode::FAILURE;
  }

  const InDetDD::TRT_DetElementCollection* alignedColl{m_detManager->getDetectorElementCollection()};
  if (alignedColl==nullptr) {
    ATH_MSG_FATAL("Null pointer is returned by getDetectorElementCollection()");
    return StatusCode::FAILURE;
  }

  // ____________ Construct new Write Cond Object ____________
  std::unique_ptr<InDetDD::TRT_DetElementContainer> writeCdoDetElCont{std::make_unique<InDetDD::TRT_DetElementContainer>()};

  // ____________ Update writeCdo using readCdo ____________
  std::map<const InDetDD::TRT_EndcapElement*, InDetDD::TRT_EndcapElement*> oldToNewECMap;
  std::map<const InDetDD::TRT_BarrelElement*, InDetDD::TRT_BarrelElement*> oldToNewBAMap;

  oldToNewECMap[nullptr] = nullptr;
  oldToNewBAMap[nullptr] = nullptr;

  //Create new aligned detector elements
  for (const InDetDD::TRT_BaseElement* oldEl : *alignedColl) {
    InDetDD::TRT_BaseElement::Type type = oldEl->type();

    if(type == InDetDD::TRT_BaseElement::ENDCAP) {
      const InDetDD::TRT_EndcapElement* oldEl_Endcap = static_cast<const InDetDD::TRT_EndcapElement*>(oldEl);
      //New encap element with new alignment created based on old element
      InDetDD::TRT_EndcapElement* newEl = new InDetDD::TRT_EndcapElement(*oldEl_Endcap);
      oldToNewECMap[oldEl_Endcap] = newEl;
      writeCdoDetElCont->addEndcapElement(newEl);
    } else if (type == InDetDD::TRT_BaseElement::BARREL) {
      const InDetDD::TRT_BarrelElement* oldEl_Barrel = static_cast<const InDetDD::TRT_BarrelElement*>(oldEl);
      //New barrel element with new alignment created based on old element
      InDetDD::TRT_BarrelElement* newEl = new InDetDD::TRT_BarrelElement(*oldEl_Barrel);
      oldToNewBAMap[oldEl_Barrel] = newEl;
      writeCdoDetElCont->addBarrelElement(newEl);
    } else {
      ATH_MSG_FATAL("Unknown TRT detector element found");
      return StatusCode::FAILURE;
    }
  }

  //Set detector elements links
  for (auto pairOfEl : oldToNewECMap) {
    if (!pairOfEl.first) continue; // skip nullptr
    pairOfEl.second->setNextInZ(oldToNewECMap[pairOfEl.first->nextInZ()]);
    pairOfEl.second->setPreviousInZ(oldToNewECMap[pairOfEl.first->previousInZ()]);
  }
  for (auto pairOfEl : oldToNewBAMap) {
    if (!pairOfEl.first) continue; // skip nullptr
    pairOfEl.second->setNextInR(oldToNewBAMap[pairOfEl.first->nextInR()]);
    pairOfEl.second->setPreviousInR(oldToNewBAMap[pairOfEl.first->previousInR()]);
    pairOfEl.second->setNextInPhi(oldToNewBAMap[pairOfEl.first->nextInPhi()]);
    pairOfEl.second->setPreviousInPhi(oldToNewBAMap[pairOfEl.first->previousInPhi()]);
  }

  // Update all detector elements caches
  for (InDetDD::TRT_BaseElement* newEl : *(writeCdoDetElCont->getElements())) {
    newEl->updateAllCaches(writeCdoAlignStore.get());
  }

  // Record WriteCondHandle
  const std::size_t writeHandleDetElContSize{writeCdoDetElCont->getElements()->size()};

  writeCdoDetElCont->setNumerology(m_detManager->getNumerology());

  // Record the resulting CDO
  if (writeHandleAlignStore != nullptr) {
    if(writeHandleAlignStore->record(std::move(writeCdoAlignStore)).isFailure()) {
      ATH_MSG_ERROR("Could not record GeoAlignmentStore " << writeHandleAlignStore->key()
        << " with EventRange " << writeHandleAlignStore->getRange()
        << " into Conditions Store");
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("recorded new CDO " << writeHandleAlignStore->key() << " with range " 
        << writeHandleAlignStore->getRange() << " into Conditions Store");
  }

  if (writeHandleDetElCont.record(std::move(writeCdoDetElCont)).isFailure()) {
    ATH_MSG_FATAL("Could not record " << writeHandleDetElCont.key()
                  << " with EventRange " << writeHandleDetElCont.getRange()
                  << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new CDO " << writeHandleDetElCont.key() << " with range " 
        << writeHandleDetElCont.getRange() << " with size of " << writeHandleDetElContSize << " into Conditions Store");

  return StatusCode::SUCCESS;
}
