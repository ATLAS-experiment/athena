/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JetTrackFilteringAlg.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

namespace InDet {

StatusCode JetTrackFilteringAlg::initialize()
{
  if (!m_systematicVariation.value().empty())
    m_systSet = CP::SystematicSet(
        {CP::SystematicVariation(m_systematicVariation)});

  if (!m_stdFilterTool.empty())
    ATH_CHECK(m_stdFilterTool.retrieve());
  if (!m_lrtFilterTool.empty())
    ATH_CHECK(m_lrtFilterTool.retrieve());
  if (!m_jetFilterTool.empty())
    ATH_CHECK(m_jetFilterTool.retrieve());

  // Validate that at least one configured tool owns the systematic.
  if (!m_systSet.empty()) {
    const CP::SystematicVariation& var = *m_systSet.begin();
    bool anyAffected = false;
    if (!m_stdFilterTool.empty())
      anyAffected |= m_stdFilterTool->isAffectedBySystematic(var);
    if (!m_lrtFilterTool.empty())
      anyAffected |= m_lrtFilterTool->isAffectedBySystematic(var);
    if (!m_jetFilterTool.empty())
      anyAffected |= m_jetFilterTool->isAffectedBySystematic(var);
    if (!anyAffected) {
      ATH_MSG_ERROR("Systematic '" << m_systematicVariation.value()
          << "' is not known to any configured filter tool");
      return StatusCode::FAILURE;
    }
  }

  ATH_CHECK(m_jetKey.initialize());
  ATH_CHECK(m_inGhostKey.initialize());
  ATH_CHECK(m_outGhostKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode JetTrackFilteringAlg::execute(const EventContext& ctx) const
{
  SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey, ctx);
  ATH_CHECK(jets.isValid());
  const xAOD::JetContainer* jetCont = jets.ptr();

  SG::ReadDecorHandle<IPC, IPLV> ghostIn(m_inGhostKey, ctx);
  SG::WriteDecorHandle<IPC, IPLV> ghostOut(m_outGhostKey, ctx);

  for (const auto* jet : *jetCont) {
    IPLV surviving;
    for (const auto& gl : ghostIn(*jet)) {
      const auto* trk =
          static_cast<const xAOD::TrackParticle*>(*gl);
      const auto patternReco = trk->patternRecoInfo();
      bool passFilter;
      if (patternReco.test(xAOD::SiSpacePointsSeedMaker_LargeD0)) {
        // LRT track: use LRT tool if configured, otherwise pass
        passFilter = m_lrtFilterTool.empty()
                  || m_lrtFilterTool->accept(trk, m_systSet);
      } else {
        // STD track: STD tool then Jet tool, both must pass
        passFilter = (m_stdFilterTool.empty()
                      || m_stdFilterTool->accept(trk, m_systSet))
                  && (m_jetFilterTool.empty()
                      || m_jetFilterTool->accept(trk, jetCont, m_systSet));
      }
      if (!passFilter) continue;
      surviving.push_back(gl);
    }
    ghostOut(*jet) = surviving;
  }

  return StatusCode::SUCCESS;
}

} // namespace InDet
