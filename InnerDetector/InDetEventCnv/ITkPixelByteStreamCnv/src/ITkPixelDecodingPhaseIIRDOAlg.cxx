/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelDecodingPhaseIIRDOAlg.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "ITkPixelDecodingStreamInfo.h"
#include "ITkPixelPhaseIIRDOCallback.h"
#include <cstdint>
#include <vector>

#define ENABLE_TIMING = true;
#ifdef ENABLE_TIMING
#define SCOPED_TIMER(name, msg) ITkPixelDecodingPhaseIIRDO::ScopedTimer timer_##__LINE__(name, msg)
#else
#define SCOPED_TIMER(name, msg)
#endif

using namespace itksw::pix::endec;

StatusCode ITkPixelDecodingPhaseIIRDOAlg::initialize()
{
    ATH_CHECK(m_decodingTool.retrieve());

    ATH_CHECK(detStore()->retrieve(m_idHelper, "PixelID"));

    ATH_CHECK(m_pixelRDOKey.initialize());

    return StatusCode::SUCCESS;
}

StatusCode ITkPixelDecodingPhaseIIRDOAlg::execute(const EventContext& ctx) const
{
    //Timing
    SCOPED_TIMER("ITkPixelDecodingPhaseIIRDOAlg::execute", msg());

    // Instantiate the output (PhaseII).
    static const int CONTAINER_LIST_SIZE = 2;
    auto cont_coll = std::make_unique< PhaseIIPixelRawDataContainerMT>(
        m_idHelper->wafer_hash_max(), CONTAINER_LIST_SIZE);
    PhaseIIPixelRawDataContainerMT::ContainerPtr rdoCont = cont_coll->getNewContainerPtr();
    rdoCont->reserve(m_n_rdos_est);

    // Get the data to decode from ROBs
    std::vector<uint64_t> bytestream_data;
    std::vector<ITkPixelDecodingStreamInfo> streams_info;
    ATH_CHECK(m_decodingTool->aggregateData(ctx, bytestream_data, streams_info));

    // Decode data
    PixelCallbacksPhaseIIRDO::PhaseIIRDOCallback cb(cont_coll.get(), rdoCont, m_idHelper);
    ATH_CHECK(m_decodingTool->decodeData(bytestream_data, streams_info, cb));
    cb.registerLastModule();

    // Record output container to SG
    SG::WriteHandle<PhaseIIPixelRawDataContainer> pixelRDOContainerHandle(m_pixelRDOKey, ctx);
    ATH_CHECK(pixelRDOContainerHandle.record(std::move(cont_coll)));

    ATH_MSG_DEBUG("nb decoded RDOs: " << cb.getNbRDOs());

    return StatusCode::SUCCESS;
}
