/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "RegSelToolTester.h"
#include "RoiDescriptor/RoiDescriptor.h"

#include <array>
#include <string_view>

RegSelToolTester::RegSelToolTester(const std::string& name, ISvcLocator* pSvcLocator)
: AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode RegSelToolTester::initialize() {
  ATH_CHECK(m_regionSelectorTools.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode RegSelToolTester::execute(const EventContext& eventContext) const {
  constexpr static std::array<std::string_view,3> roiNames{"FullScan","Central","Forward"};
  const std::array<RoiDescriptor,3> rois{
    RoiDescriptor{RoiDescriptor::FULLSCAN},
    RoiDescriptor{-0.3, -0.1, 0.5, 0.8},
    RoiDescriptor{3.5, 2.5, 5.0, 0.0, -1.0, 1.0, 100, 50, 200}
  };
  std::vector<IdentifierHash> ids;
  std::vector<uint32_t> robs;

  for (const auto& tool : m_regionSelectorTools) {
    for (size_t iroi=0; iroi<3; ++iroi) {
      ids.clear();
      tool->lookup(eventContext)->HashIDList(rois[iroi], ids);
      ATH_MSG_INFO(roiNames[iroi] << " RoI " << rois[iroi] << " mapped to " << ids.size() << " hash IDs");
      robs.clear();
      tool->lookup(eventContext)->ROBIDList(rois[iroi], robs);
      ATH_MSG_INFO(roiNames[iroi] << " RoI " << rois[iroi] << " mapped to " << robs.size() << " ROB IDs");
    }
  }

  return StatusCode::SUCCESS;
}
