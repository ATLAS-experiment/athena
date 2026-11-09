/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ByteStreamToRawEventAlg.h"
#include <mutex>

StatusCode ByteStreamToRawEventAlg::initialize() {
  ATH_CHECK(m_robsSvc.retrieve());
  ATH_CHECK(m_byteStreamCnvSvc.retrieve());

  m_eventsCache = SG::SlotSpecificObj<Cache>(SG::getNSlots());

  return StatusCode::SUCCESS;
}

StatusCode ByteStreamToRawEventAlg::execute(const EventContext& ctx) const {
  RawEventWrite rew;

  FullEventAssembler<SrcIdMap>* fea = 0;
  for (std::string const & raw_container: m_RawContainers) {
    ATH_CHECK(m_byteStreamCnvSvc->getFullEventAssembler(fea, raw_container));
    fea->fill(&rew, msg());
  }

  const uint32_t rawSize = rew.size_word();
	ATH_MSG_DEBUG("rawSize after fill: " << rew.size_word() << " words");

  Cache* event_cache = m_eventsCache.get(ctx);
  { // lock scope
    std::lock_guard<std::mutex> event_cache_lock(event_cache->m_mutex);

    std::vector<uint32_t>& buffer = m_eventsCache.get(ctx)->m_data;
    buffer.resize(rawSize);
    const uint32_t count =
        eformat::write::copy(*(rew.bind()), buffer.data(), rawSize);
    if (count != rawSize) {
      ATH_MSG_ERROR("Memcopy failed");
      return StatusCode::FAILURE;
    }

    RawEvent* fullEvent = &(m_eventsCache.get(ctx)->m_event);
    fullEvent->assign(buffer.data());
    ATH_CHECK(fullEvent->check_tree_noex());
    m_robsSvc->setNextEvent(ctx, fullEvent);
  } // lock scope

  // Clean for next event
  for (std::string const & raw_container: m_RawContainers) {
    ATH_CHECK(m_byteStreamCnvSvc->getFullEventAssembler(fea, raw_container));
    fea->clear();
  }

  return StatusCode::SUCCESS;
}
