/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <memory>
#include "TRTRawDataProvider.h"

using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

// --------------------------------------------------------------------
// Constructor

TRTRawDataProvider::TRTRawDataProvider(const std::string& name,
				       ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm      ( name, pSvcLocator ),
  m_robDataProvider ( "ROBDataProviderSvc", name ),
  m_rawDataTool     ( "TRTRawDataProviderTool",this ),
  m_CablingSvc      ( "TRT_CablingSvc", name ),
  m_trt_id          ( nullptr ),
  m_rdoContainerKey("")
{
  declareProperty("RoIs", m_roiCollectionKey = std::string(""), "RoIs to read in");
  declareProperty("isRoI_Seeded", m_roiSeeded = false, "Use RoI");
  declareProperty("RDOKey", m_rdoContainerKey = std::string("TRT_RDOs"));
  declareProperty("BSErrkey",m_bsErrContKey = "TRT_ByteStreamErrs");
  declareProperty("RDOCacheKey", m_rdoCacheKey);
  declareProperty ("ProviderTool", m_rawDataTool );
}

// --------------------------------------------------------------------
// Initialize

StatusCode TRTRawDataProvider::initialize() {

  ATH_CHECK(m_robDataProvider.retrieve());

  ATH_CHECK(m_rawDataTool.retrieve());

  // Get the TRT Helper
  ATH_CHECK(detStore()->retrieve(m_trt_id, "TRT_ID"));


  if (m_roiSeeded) {
    ATH_CHECK( m_roiCollectionKey.initialize() );
    ATH_CHECK(m_regionSelector.retrieve());
  }
  else {//Only need cabling if not using RoIs
    // Retrieve id mapping
    ATH_CHECK(m_CablingSvc.retrieve());
    ATH_CHECK( m_roiCollectionKey.initialize(false) ); //Clear if unneeded
    m_regionSelector.disable();
  }

  ATH_CHECK( m_rdoContainerKey.initialize() );

  ATH_CHECK( m_bsErrContKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_rdoCacheKey.initialize(SG::AllowEmpty) );

  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------
// Execute

StatusCode TRTRawDataProvider::execute(const EventContext& ctx) const
{
  SG::WriteHandle<TRT_RDO_Container> rdoContainer(m_rdoContainerKey, ctx);
  if( m_rdoCacheKey.empty() ) {
    rdoContainer = std::make_unique<TRT_RDO_Container>(m_trt_id->straw_hash_max(), EventContainers::Mode::OfflineFast);

  }
  else{
    SG::UpdateHandle<TRT_RDO_Cache> updateh(m_rdoCacheKey, ctx);
    rdoContainer = std::make_unique<TRT_RDO_Container>(updateh.ptr());
    ATH_MSG_DEBUG("Created container " << m_rdoContainerKey.key() << " using external cache " << m_rdoCacheKey.key());
  }
  ATH_CHECK(rdoContainer.isValid());

  std::unique_ptr<TRT_BSErrContainer> bsErrCont=std::make_unique<TRT_BSErrContainer>();

  std::vector<uint32_t> listOfRobs;
  if (!m_roiSeeded) {
    listOfRobs = m_CablingSvc->getAllRods();
  }
  else {//Enter RoI-seeded mode
      SG::ReadHandle<TrigRoiDescriptorCollection> roiCollection(m_roiCollectionKey, ctx);
      ATH_CHECK(roiCollection.isValid());

      TrigRoiDescriptorCollection::const_iterator roi = roiCollection->begin();
      TrigRoiDescriptorCollection::const_iterator roiE = roiCollection->end();
      TrigRoiDescriptor superRoI;//add all RoIs to a super-RoI
      superRoI.setComposite(true);
      superRoI.manageConstituents(false);
      for (; roi!=roiE; ++roi) {
        superRoI.push_back(*roi);
      }

      m_regionSelector->lookup(ctx)->ROBIDList( superRoI, listOfRobs );

  }
  std::vector<const ROBFragment*> listOfRobf;
  m_robDataProvider->getROBData( ctx, listOfRobs, listOfRobf);

  ATH_MSG_DEBUG( "Number of ROB fragments " << listOfRobf.size() );

  // ask TRTRawDataProviderTool to decode it and to fill the IDC
  const bool hasExternalCache = rdoContainer->hasExternalCache();
  std::unique_ptr<DataPool<TRT_LoLumRawData>> dataItemsPool = nullptr;
  if (!hasExternalCache) {
      dataItemsPool = std::make_unique<DataPool<TRT_LoLumRawData>>(ctx);
      dataItemsPool->reserve(100000);  // Some large default size 
  }else if(m_useDataPoolWithCache){
    dataItemsPool = std::make_unique<DataPool<TRT_LoLumRawData>>(ctx);
    //this is  per view so let it expand on its own in blocks
  }

  if (m_rawDataTool->convert(listOfRobf,&(*rdoContainer),bsErrCont.get(), dataItemsPool.get(), ctx).isFailure()){
    ATH_MSG_WARNING( "BS conversion into RDOs failed" );
  }
  ATH_MSG_DEBUG( "Number of Collections in IDC " << rdoContainer->numberOfCollections() );


  if (!m_bsErrContKey.empty()) {
    ATH_MSG_DEBUG("Recording BS error container");
    SG::WriteHandle<TRT_BSErrContainer> bsErrContHdl{m_bsErrContKey, ctx};
    ATH_CHECK(bsErrContHdl.record(std::move(bsErrCont)));
  }


  return StatusCode::SUCCESS;
}

