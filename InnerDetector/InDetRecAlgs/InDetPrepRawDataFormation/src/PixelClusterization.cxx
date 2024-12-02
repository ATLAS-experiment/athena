/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file PixelClusterization.h
 * Implementation for PixelClusterization.
 * (modified s.roe, 18.12.2008)
 **/


// Silicon trackers includes
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetPrepRawDataFormation/PixelClusterization.h"
#include "InDetRawData/PixelRDORawData.h"

#include "Identifier/Identifier.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetPrepRawData/PixelClusterCollection.h"


#include "SiClusterizationTool/IPixelClusteringTool.h"
#include "SiClusterizationTool/PixelGangedAmbiguitiesFinder.h"

#include "StoreGate/WriteHandle.h"


namespace InDet{
  using namespace InDet;
  // Constructor with parameters:
  PixelClusterization::PixelClusterization(const std::string &name, ISvcLocator *pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator),
  m_clusteringTool("InDet::MergedPixelsTool", this), //made private
  m_gangedAmbiguitiesFinder("InDet::PixelGangedAmbiguitiesFinder", this), //made private
  m_rdoContainerKey(""),
  m_roiCollectionKey(""),
  m_roiSeeded(false),
  m_idHelper(nullptr),
  m_clusterContainerKey(""),
  m_clusterContainerLinkKey(""),
  m_ambiguitiesMapKey(""),
  m_clusterContainercacheKey("") {
    // Get parameter values from jobOptions file
    declareProperty("DataObjectName", m_rdoContainerKey = std::string("PixelRDOs"));
    declareProperty("clusteringTool", m_clusteringTool);
    declareProperty("gangedAmbiguitiesFinder", m_gangedAmbiguitiesFinder);
    declareProperty("ClustersName", 
                  m_clusterContainerKey = std::string("PixelClusters"),
                  "Pixel cluster container");
    declareProperty("ClustersLinkName_", 
                  m_clusterContainerLinkKey = std::string("PixelClusters"),
                  "Pixel cluster container link name (don't set this)");
    declareProperty("AmbiguitiesMap", 
                  m_ambiguitiesMapKey = std::string("PixelClusterAmbiguitiesMap"),
                  "Ambiguity Map container");
    declareProperty("RoIs", m_roiCollectionKey = std::string(""), "RoIs to read in");
    declareProperty("isRoI_Seeded", m_roiSeeded = false, "Use RoI");
    declareProperty("ClusterContainerCacheKey", m_clusterContainercacheKey, "Optional External Pixel cluster Cache");
  }
  
  //-----------------------------------------------------------------------------
  // Initialize method:
  StatusCode PixelClusterization::initialize(){
    // get the InDet::MergedPixelsTool
    ATH_CHECK(m_clusteringTool.retrieve());
    // get the InDet::PixelGangedAmbiguitiesFinder
    ATH_CHECK(m_gangedAmbiguitiesFinder.retrieve());
    // Get the Pixel helper
    ATH_CHECK(detStore()->retrieve(m_idHelper,"PixelID"));
    ATH_CHECK( m_rdoContainerKey.initialize() );
    if (m_roiSeeded) {
      ATH_CHECK( m_roiCollectionKey.initialize() );
      ATH_CHECK(m_regionSelector.retrieve());
    }else{
      m_regionSelector.disable();
      ATH_CHECK( m_roiCollectionKey.initialize(false) );
    }

    m_clusterContainerLinkKey = m_clusterContainerKey.key();

    ATH_CHECK( m_clusterContainerKey.initialize() );
    ATH_CHECK( m_clusterContainerLinkKey.initialize() );
    ATH_CHECK( m_ambiguitiesMapKey.initialize() );
    ATH_CHECK( m_clusterContainercacheKey.initialize(!m_clusterContainercacheKey.key().empty()) );

    if ( !m_monTool.empty() ) {
       ATH_CHECK(m_monTool.retrieve() );
    }

    return StatusCode::SUCCESS;
  }
  
  //----------------------------------------------------------------------------
  // Execute method:
  StatusCode PixelClusterization::execute(const EventContext& ctx) const {
    //Monitoring Tool Configuration
    auto mnt_timer_Total                 = Monitored::Timer<std::chrono::milliseconds>("TIME_Total");
    
  

    SG::WriteHandle<PixelClusterContainer> clusterContainer(m_clusterContainerKey, ctx);
    if(m_clusterContainercacheKey.key().empty()){
      ATH_CHECK( clusterContainer.record (std::make_unique<PixelClusterContainer>(m_idHelper->wafer_hash_max(), EventContainers::Mode::OfflineFast)) );
    }else{
      SG::UpdateHandle<PixelClusterContainerCache> clusterContainercache(m_clusterContainercacheKey, ctx);
      ATH_CHECK(clusterContainercache.isValid());
      ATH_CHECK( clusterContainer.record (std::make_unique<PixelClusterContainer>(clusterContainercache.ptr() )));
    }

    ATH_CHECK(clusterContainer.isValid());
    ATH_MSG_DEBUG( "Container '" << clusterContainer->name() << "' initialised" );

    ATH_CHECK( clusterContainer.symLink (m_clusterContainerLinkKey) );
    ATH_MSG_DEBUG( "Pixel clusters '" << clusterContainer.name() << "' symlinked in StoreGate");

    ATH_MSG_DEBUG( "Creating the ganged ambiguities map");
    SG::WriteHandle<PixelGangedClusterAmbiguities> ambiguitiesMap(m_ambiguitiesMapKey, ctx);
    ambiguitiesMap = std::make_unique<PixelGangedClusterAmbiguities>();

    SG::ReadHandle<PixelRDO_Container> rdoContainer(m_rdoContainerKey, ctx);

    ATH_CHECK(rdoContainer.isValid());

    ATH_MSG_DEBUG( "Data object " << rdoContainer.name() << " found" );

    std::unique_ptr<DataPool<PixelCluster>> dataItemsPool = nullptr;
    const bool hasExternalCache = rdoContainer->hasExternalCache();
    if (!hasExternalCache) {
      dataItemsPool = std::make_unique<DataPool<PixelCluster>>(ctx);
      dataItemsPool->reserve(20000);  // Some large default size
    }else if (m_useDataPoolWithCache){
      dataItemsPool = std::make_unique<DataPool<PixelCluster>>(ctx);
      //Default size for now 1024 let it expand on its own
    }


    if (!m_roiSeeded) {//Full-scan mode
      PixelRDO_Container::const_iterator rdoCollections      = rdoContainer->begin();
      PixelRDO_Container::const_iterator rdoCollectionsEnd   = rdoContainer->end();

      for(; rdoCollections!=rdoCollectionsEnd; ++rdoCollections){
        const COLLECTION* RDO_Collection(*rdoCollections);
        if (!RDO_Collection || RDO_Collection->empty()) continue;
        PixelClusterContainer::IDC_WriteHandle lock = clusterContainer->getWriteHandle(rdoCollections.hashId());
        if( lock.OnlineAndPresentInAnotherView() ) continue;

        // Use one of the specific clustering AlgTools to make clusters
        std::unique_ptr<PixelClusterCollection> clusterCollection(
            m_clusteringTool->clusterize(*RDO_Collection, *m_idHelper,
                                         dataItemsPool.get(), ctx));

        if (clusterCollection && !clusterCollection->empty()){

          m_gangedAmbiguitiesFinder->execute(clusterCollection.get(),*ambiguitiesMap);
          ATH_CHECK(lock.addOrDelete( std::move(clusterCollection) ));
        }else{
          ATH_MSG_DEBUG("No PixelClusterCollection to write");
        }
      }
    }
    else {//enter RoI-seeded mode
      
      SG::ReadHandle<TrigRoiDescriptorCollection> roiCollection(m_roiCollectionKey, ctx);
      ATH_CHECK(roiCollection.isValid());

      TrigRoiDescriptorCollection::const_iterator roi = roiCollection->begin();
      TrigRoiDescriptorCollection::const_iterator roiE = roiCollection->end();

      std::vector<IdentifierHash> listOfPixIds;
      for (; roi!=roiE; ++roi) {
        
        listOfPixIds.clear();//Prevents needless memory reallocations
        m_regionSelector->lookup(ctx)->HashIDList( **roi, listOfPixIds );

	ATH_MSG_VERBOSE(**roi);
        ATH_MSG_VERBOSE( "REGTEST: Pixel : Roi contains " 
		     << listOfPixIds.size() << " det. Elements" );
        for (auto & listOfPixId : listOfPixIds) {
          const InDetRawDataCollection<PixelRDORawData>* RDO_Collection (rdoContainer->indexFindPtr(listOfPixId));

          if (!RDO_Collection) continue;
          PixelClusterContainer::IDC_WriteHandle lock = clusterContainer->getWriteHandle(listOfPixId);
          if( lock.OnlineAndPresentInAnotherView() ) continue;

          // Use one of the specific clustering AlgTools to make clusters
          std::unique_ptr<PixelClusterCollection> clusterCollection(
              m_clusteringTool->clusterize(*RDO_Collection, *m_idHelper,
                                           dataItemsPool.get(), ctx));
          if (clusterCollection && !clusterCollection->empty()) {
            ATH_MSG_VERBOSE( "REGTEST: Pixel : clusterCollection contains " 
                << clusterCollection->size() << " clusters" );
            m_gangedAmbiguitiesFinder->execute(clusterCollection.get(),*ambiguitiesMap);
            ATH_CHECK(lock.addOrDelete( std::move(clusterCollection) ));

          } else {
            ATH_MSG_DEBUG("No PixelClusterCollection to write");
          }
        }
      }
    }
    ATH_CHECK(clusterContainer.setConst());
    ATH_MSG_DEBUG("clusterContainer->numberOfCollections() " <<  clusterContainer->numberOfCollections());
    ATH_CHECK(ambiguitiesMap.isValid());
    ATH_MSG_DEBUG( "PixelClusterAmbiguitiesMap recorded in StoreGate");
    auto monTime = Monitored::Group(m_monTool, mnt_timer_Total);
    return StatusCode::SUCCESS;
  }

}// namespace closure


