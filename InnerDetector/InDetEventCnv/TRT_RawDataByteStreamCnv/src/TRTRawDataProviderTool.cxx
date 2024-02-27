/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "TRTRawDataProviderTool.h"
#include "GaudiKernel/IToolSvc.h"
#include "InDetRawData/TRT_RDORawData.h"
#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/StoreClearedIncident.h"
#include "StoreGate/WriteHandle.h"


using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;


// -------------------------------------------------------
//



// -------------------------------------------------------
// default contructor

TRTRawDataProviderTool::TRTRawDataProviderTool
( const std::string& type, const std::string& name,const IInterface* parent )
  :  base_class( type, name, parent ),
     m_decoder   ("TRT_RodDecoder",this)
{
  declareProperty ("Decoder", m_decoder);
}

// -------------------------------------------------------
// initialize

StatusCode TRTRawDataProviderTool::initialize()
{

  ATH_CHECK( AlgTool::initialize() );

  ATH_CHECK( m_decoder.retrieve() );
  ATH_MSG_INFO( "Retrieved tool " << m_decoder );

  //initialize write handles
  ATH_CHECK(m_lvl1idkey.initialize(m_storeInDetTimeColls));
  ATH_CHECK(m_bcidkey.initialize(m_storeInDetTimeColls));

  return StatusCode::SUCCESS;
}

// -------------------------------------------------------
// convert method

StatusCode TRTRawDataProviderTool::convert(const std::vector<const ROBFragment*>& vecRobs,
					   TRT_RDO_Container* rdoIdc,
					   TRT_BSErrContainer* bserr,
             DataPool<TRT_LoLumRawData>* dataItemsPool,
             const EventContext& ctx) const
{

  static std::atomic_int DecodeErrCount = 0;

  if(vecRobs.empty())
     return StatusCode::SUCCESS;


  std::unique_ptr<InDetTimeCollection> LVL1Collection;
  std::unique_ptr<InDetTimeCollection> BCCollection;
  std::vector<const ROBFragment*>::const_iterator rob_it = vecRobs.begin();


  if ( m_storeInDetTimeColls ) {
    // Create Collections for per ROD vectors on L1ID and BCID
    LVL1Collection = std::make_unique<InDetTimeCollection>();
    LVL1Collection->reserve(vecRobs.size());

    BCCollection = std::make_unique<InDetTimeCollection>();
    BCCollection->reserve(vecRobs.size());
  }

  // loop over the ROB fragments
  for(; rob_it!=vecRobs.end(); ++rob_it)
  {

    uint32_t robid = (*rob_it)->rod_source_id();

    if (m_storeInDetTimeColls) {
      /*
       * Add to vector containing pairs of (ROD ID, [L1ID,BCID])
       */

      unsigned int lvl1id = (*rob_it)->rod_lvl1_id();
      LVL1Collection->emplace_back(robid, lvl1id);

      unsigned int bcid = (*rob_it)->rod_bc_id();
      BCCollection->emplace_back(robid, bcid);

#ifdef TRT_BSC_DEBUG
	 ATH_MSG_DEBUG( "Stored LVL1ID " << lvl1id << " and BCID " << bcid << " in InDetTimeCollections" );
#endif
    }
      StatusCode sc = m_decoder->fillCollection( &**rob_it, rdoIdc, bserr, dataItemsPool);
      if ( sc == StatusCode::FAILURE )
      {
         if (DecodeErrCount < 100) {
           ATH_MSG_INFO("Problem with TRT ByteStream Decoding!");
           DecodeErrCount++;
         } else if (100 == DecodeErrCount) {
           ATH_MSG_INFO(
               "Too many Problem with TRT Decoding messages.  Turning message "
               "off.");
           DecodeErrCount++;
         }
        // return sc;  Don't return on single ROD failure
      }
  }

  /*
   * record per ROD L1ID and BCID collections
   */
  if ( m_storeInDetTimeColls ) {

    SG::WriteHandle<InDetTimeCollection> lvl1id(m_lvl1idkey,ctx);
    ATH_CHECK(lvl1id.record(std::move(LVL1Collection)));

    SG::WriteHandle<InDetTimeCollection> bcid(m_bcidkey,ctx);
    ATH_CHECK(bcid.record(std::move(BCCollection)));
  }

  return StatusCode::SUCCESS;
}
