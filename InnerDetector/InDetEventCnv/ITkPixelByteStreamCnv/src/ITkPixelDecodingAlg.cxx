/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelDecodingAlg.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "StoreGate/UpdateHandle.h"
#include "ITkPixelDecodingStreamInfo.h"
#include <cstdint>
#include <vector>

#define ENABLE_TIMING = true;
#ifdef ENABLE_TIMING
#define SCOPED_TIMER(name, msg) ITkPixelDecoding::ScopedTimer timer_##__LINE__(name, msg)
#else
#define SCOPED_TIMER(name, msg)
#endif


using namespace itksw::pix::endec;

StatusCode ITkPixelDecodingAlg::initialize()
{
    ATH_CHECK(m_decodingTool.retrieve());

    ATH_CHECK(detStore()->retrieve(m_idHelper, "PixelID"));

    ATH_CHECK(m_pixelRDOKey.initialize());

    ATH_CHECK(m_pixelRDOCacheKey.initialize(!m_pixelRDOCacheKey.key().empty()));

    return StatusCode::SUCCESS;
}

StatusCode ITkPixelDecodingAlg::execute(const EventContext& ctx) const
{
    //Timing
    SCOPED_TIMER("ITkPixelDecodingAlg::execute", msg());

    //Instantiate the output container, either standalone or backed by an external
    //cache shared between the various RoI views that may run this algorithm within
    //the same event (e.g. in the trigger).
    bool externalCache = !m_pixelRDOCacheKey.key().empty();
    std::unique_ptr<PixelRDO_Container> rdoCont;
    if (!externalCache) {
        rdoCont = std::make_unique<PixelRDO_Container>(m_idHelper->wafer_hash_max());
    }
    else {
        SG::UpdateHandle<PixelRDO_Cache> rdoCache(m_pixelRDOCacheKey, ctx);
        ATH_CHECK(rdoCache.isValid());
        rdoCont = std::make_unique<PixelRDO_Container>(rdoCache.ptr());
        ATH_MSG_DEBUG("Created container using external cache " << m_pixelRDOCacheKey.key());
    }

    // Get the data to decode from ROBs
    std::vector<uint64_t> bytestream_data;
    std::vector<ITkPixelDecodingStreamInfo> streams_info;
    ATH_CHECK(m_decodingTool->aggregateData(ctx, bytestream_data, streams_info));

    // Decode data
    PixelCallbacks::RDOCallback cb(rdoCont.get(), m_idHelper, msg());
    ATH_CHECK(m_decodingTool->decodeData(bytestream_data, streams_info, cb));

    // Record output container to SG
    SG::WriteHandle<PixelRDO_Container> pixelRDOContainerHandle(m_pixelRDOKey, ctx);
    ATH_CHECK(pixelRDOContainerHandle.record(std::move(rdoCont)));

    ATH_MSG_DEBUG("nb decoded RDOs: " << cb.getNbRDOs());

    return StatusCode::SUCCESS;
}

StatusCode ITkPixelDecodingAlg::finalize(){
    return StatusCode::SUCCESS;
}