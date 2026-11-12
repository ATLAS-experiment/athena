/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "SeedValidationAlg.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"
#include "xAODInDetMeasurement/SpacePoint.h"

#include <GaudiKernel/StatusCode.h>

#include <algorithm>


namespace {

MsgStream & operator<<(MsgStream & ms, const ActsTrk::Seed & seed) {
  ms << "quality: " << seed.quality() << ", sps: [";
  for (const xAOD::SpacePoint* sp : seed.spacePoints()) {
    if (sp == nullptr) { ms << "(null) "; continue; }
    ms << "(" << sp->globalPosition().x() << ", " << sp->globalPosition().y()
       << ", " << sp->globalPosition().z() << ") ";
  }
  return ms << "]";
}

} // anonymous namespace

namespace ActsTrk {

StatusCode SeedValidationAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing");

  ATH_CHECK(m_referenceSeedsKey.initialize());
  ATH_CHECK(m_monitoredSeedsKey.initialize());

  if (!m_compareQuality) {
    ATH_MSG_INFO("Seed quality comparison is disabled");
  }

  return StatusCode::SUCCESS;
}

StatusCode SeedValidationAlg::execute(const EventContext& ctx) const
{
  auto ref_seeds = SG::makeHandle(m_referenceSeedsKey, ctx);
  auto mon_seeds = SG::makeHandle(m_monitoredSeedsKey, ctx);
  ATH_CHECK(ref_seeds.isValid());
  ATH_CHECK(mon_seeds.isValid());

  if (mon_seeds->size() != ref_seeds->size()) {
    ATH_MSG_ERROR("different sizes: seeds (ref: " << ref_seeds->size()
                  << ", mon: " << mon_seeds->size() << ")");
    return StatusCode::FAILURE;
  }

  unsigned int seed_mismatch = 0;
  for (std::size_t i = 0; i < mon_seeds->size(); ++i) {
    const auto index = static_cast<ActsTrk::SeedContainer::Index>(i);
    const ActsTrk::Seed mon = mon_seeds->at(index);
    const ActsTrk::Seed ref = ref_seeds->at(index);

    // Same spacepoints (by pointer), in the same order, by pointer and same quality if flag is on
        if (!std::ranges::equal(mon.spacePoints(), ref.spacePoints())
          || (m_compareQuality && mon.quality() != ref.quality()))
    {
      seed_mismatch += 1;
      ATH_MSG_DEBUG("seeds differ at " << i << ": ref=[" << ref << "], mon=[" << mon << ']');
    }
  }


  if (seed_mismatch != 0) {
    ATH_MSG_ERROR("seeds: " << seed_mismatch << " mismatches over " << mon_seeds->size() << " seeds");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("seeds: all " << mon_seeds->size() << " match");
  return StatusCode::SUCCESS;
}

} // namespace ActsTrk