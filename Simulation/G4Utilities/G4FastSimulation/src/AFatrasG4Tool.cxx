/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AFatrasG4Tool.h"
#include "AFatrasG4.h"


StatusCode AFatrasG4Tool::initializeFastSim()
{
    ATH_CHECK(FastSimulationBase::initializeFastSim());
    ATH_CHECK(m_ActsFatrasG4Tool.retrieve());
    ATH_CHECK(m_pixelHitsKey.initialize());
    ATH_CHECK(m_sctHitsKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode AFatrasG4Tool::BeginOfAthenaEvent(HitCollectionMap& hcm)
{
    ATH_MSG_DEBUG("AFatrasG4Tool:BeginOfAthenaEvent");
    ATH_CHECK(FastSimulationBase::BeginOfAthenaEvent(hcm));
    if (msgLvl(MSG::DEBUG)){
      const EventContext& ctx = Gaudi::Hive::currentContext();
      ATH_MSG_DEBUG("BeginOfAthenaEvent slot=" << ctx.slot());
    }

    return StatusCode::SUCCESS;
}


G4VFastSimulationModel* AFatrasG4Tool::makeFastSimModel()
{
  ATH_MSG_INFO("Initializing Fast Simulation Model AFatrasG4");
  // Create the AFatrasG4 fast simulation model
  return new AFatrasG4(name(), getRegion(), m_ActsFatrasG4Tool, this);
}

StatusCode AFatrasG4Tool::EndOfAthenaEvent(HitCollectionMap& hcm)
{
    ATH_MSG_DEBUG("AFatrasG4Tool:EndOfAthenaEvent");
    const EventContext& ctx = Gaudi::Hive::currentContext();
    ATH_MSG_DEBUG("EndOfAthenaEvent slot=" << ctx.slot()); //check if same slot as in BeginOfAthenaEvent

    // create the store record once at the end of each athena event
    SG::WriteHandle<SiHitCollection> pixelHandle(m_pixelHitsKey, ctx);
    if (!pixelHandle.isValid()) {
        ATH_MSG_DEBUG("Pixel container not valid — recording now");
        ATH_CHECK(pixelHandle.record(std::make_unique<SiHitCollection>(m_pixelHitsKey.key())));
    } else {
        ATH_MSG_DEBUG("Pixel container already valid");
    }

    SG::WriteHandle<SiHitCollection> sctHandle(m_sctHitsKey, ctx);
    if (!sctHandle.isValid()) {
        ATH_MSG_DEBUG("SCT container not valid — recording now");
        ATH_CHECK(sctHandle.record(std::make_unique<SiHitCollection>(m_sctHitsKey.key())));
    } else {
        ATH_MSG_DEBUG("SCT container already valid");
    }

    if (msgLvl(MSG::DEBUG)) {
      // check if record stores created
      ATH_MSG_DEBUG("---- HIT DEBUG ----");
      ATH_MSG_DEBUG("Slot              = " << ctx.slot());
      ATH_MSG_DEBUG("Pixel key         = " << m_pixelHitsKey.key());
      ATH_MSG_DEBUG("Pixel isPresent   = " << pixelHandle.isPresent());
      ATH_MSG_DEBUG("Pixel isValid     = " << pixelHandle.isValid());
      ATH_MSG_DEBUG("SCT key           = " << sctHandle.key());
      ATH_MSG_DEBUG("SCT isPresent     = " << sctHandle.isPresent());
      ATH_MSG_DEBUG("SCT isValid       = " << sctHandle.isValid());
      ATH_MSG_DEBUG("-------------------");    
    }

    if (!pixelHandle.isValid() || !sctHandle.isValid()) {
      ATH_MSG_ERROR("Hit containers missing at EndOfAthenaEvent!");
      return StatusCode::FAILURE;
    }

    // Get hits from ActsFatrasG4Tool cache
    const auto& pixelHits = m_ActsFatrasG4Tool->getPixelHitsCache(ctx);
    const auto& sctHits   = m_ActsFatrasG4Tool->getSCTHitsCache(ctx);
    ATH_MSG_DEBUG("Pixel cache size = " << pixelHits.size());
    ATH_MSG_DEBUG("SCT cache size   = " << sctHits.size());

    // Fill the hits to store handler from the caches
    for (const auto& hit : pixelHits) {pixelHandle->push_back(hit);}
    for (const auto& hit : sctHits) {sctHandle->push_back(hit);}
    ATH_MSG_DEBUG("After fill:");
    ATH_MSG_DEBUG("Pixel container size = " << pixelHandle->size());
    ATH_MSG_DEBUG("SCT container size   = " << sctHandle->size());

    // sanity printing
    if (msgLvl(MSG::VERBOSE)) {
      ATH_MSG_VERBOSE("==== FatrasG4Tool: Pixel Hits ====");
      for (const SiHit& hit : *pixelHandle) {
        const auto& start = hit.localStartPosition();
        const auto& end   = hit.localEndPosition();
        int barcode = hit.particleLink().barcode();

        ATH_MSG_VERBOSE("TrackID=" << barcode
                    << "  Edep(MeV)=" << hit.energyLoss()
                    << "  LocalStart=("
                    << start.x() << ", "
                    << start.y() << ", "
                    << start.z() << ")"
                    << "  LocalEnd=("
                    << end.x() << ", "
                    << end.y() << ", "
                    << end.z() << ")");
      }
      ATH_MSG_VERBOSE("==== FatrasG4Tool: SCT Hits ====");
      for (const SiHit& hit : *sctHandle) {
        const auto& start = hit.localStartPosition();
        const auto& end   = hit.localEndPosition();
        int barcode = hit.particleLink().barcode();

        ATH_MSG_VERBOSE("TrackID=" << barcode
                    << "  Edep(MeV)=" << hit.energyLoss()
                    << "  LocalStart=("
                    << start.x() << ", "
                    << start.y() << ", "
                    << start.z() << ")"
                    << "  LocalEnd=("
                    << end.x() << ", "
                    << end.y() << ", "
                    << end.z() << ")");
      }
    }

    // Clear cache for next event
    m_ActsFatrasG4Tool->clearCaches(ctx);

    return FastSimulationBase::EndOfAthenaEvent(hcm);
}
