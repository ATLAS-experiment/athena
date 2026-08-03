/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "hltinterface/ContainerFactory.h"
#include "hltinterface/IInfoRegister.h"

#include "TrigExISPublishing.h"

#include <chrono>
#include <mutex>
#include <vector>

TrigExISPublishing::TrigExISPublishing(const std::string& name, ISvcLocator* svcLoc) :
  AthReentrantAlgorithm(name, svcLoc)
{}

StatusCode TrigExISPublishing::initialize()
{
  // construct the LAr noise burst container and register it
  auto cfact = hltinterface::ContainerFactory::getInstance();
  if (cfact) {
    try {
      const std::string ISname = "LArISInfo_NoiseBurstAlg";
      const std::string IStype = "LArNoiseBurstCandidates";
      m_IsObject = cfact->constructContainer(ISname, IStype);
      m_evntPos = cfact->addIntVector(m_IsObject, "Flag",
                                      hltinterface::GenericHLTContainer::LASTVALUE);
      m_timeTagPos = cfact->addIntVector(m_IsObject, "TimeStamp",
                                         hltinterface::GenericHLTContainer::LASTVALUE);
      m_timeTagPosns = cfact->addIntVector(m_IsObject, "TimeStamp_ns",
                                           hltinterface::GenericHLTContainer::LASTVALUE);
      ATH_MSG_DEBUG("Registering container in IS with name /HLTObjects/" << ISname);
      hltinterface::IInfoRegister::instance()->registerObject("/HLTObjects/", m_IsObject);
    }
    catch (std::exception& ex) {
      ATH_MSG_ERROR("Cannot publish to IS: " << ex.what());
    }
  }
  else {
    ATH_MSG_INFO("IS publishing not available");
  }

  return StatusCode::SUCCESS;
}

StatusCode TrigExISPublishing::execute(const EventContext& ctx) const
{
  auto* reg = hltinterface::IInfoRegister::instance();

  if (m_IsObject && reg) {
    // Time-varying values, to see at a glance that publication is alive.
    const long n = m_nEvents.fetch_add(1, std::memory_order_relaxed) + 1;
    const auto now  = std::chrono::system_clock::now().time_since_epoch();
    const long sec  = std::chrono::duration_cast<std::chrono::seconds>(now).count();
    const long nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count() % 1000000000L;

    boost::property_tree::ptree event_tree;
    event_tree.put("eventNumber", ctx.eventID().event_number());
    event_tree.put("LBNumber", ctx.eventID().lumi_block());

    // Take the publication mutex offered by the service.
    // endEvent() serialises every registered container, not just ours, so a private
    // lock would not stop another producer writing its container while our endEvent() reads it.
    // The lock must therefore span both the update and the endEvent() call.
    std::lock_guard<std::mutex> lock(reg->getPublicationMutex());
    try {
      reg->beginEvent(event_tree);

      m_IsObject->appendField(m_evntPos, std::vector<long>{n % 256});
      m_IsObject->appendField(m_timeTagPos, std::vector<long>{sec});
      m_IsObject->appendField(m_timeTagPosns, std::vector<long>{nsec});

      constexpr size_t maxEntries = 100;
      for (const size_t pos : {m_evntPos, m_timeTagPos, m_timeTagPosns}) {
        std::vector<long>& v = m_IsObject->getIntVecField(pos);
        if (v.size() > maxEntries) {
          v.erase(v.begin(), v.end() - maxEntries);
        }
      }

      reg->endEvent(event_tree);
    }
    catch (const std::exception& ex) {
      ATH_MSG_INFO("Caught exception during IS publication: " << ex.what());
    }
  }

  return StatusCode::SUCCESS;
}
