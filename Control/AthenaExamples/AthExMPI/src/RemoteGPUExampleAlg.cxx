// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s)
#include "RemoteGPUExampleAlg.h"

// System include(s)
#include <memory>

namespace RemoteCall {

StatusCode RemoteGPUExampleAlg::initialize() {
  ATH_CHECK(m_remoteGPUSvc.retrieve());
  ATH_CHECK(m_remoteGPUSvc->registerFunction(test_fn, "test_fn"));
  return StatusCode::SUCCESS;
}

StatusCode RemoteGPUExampleAlg::execute(const EventContext& ctx) const {
  if (m_remoteGPUSvc->isServer()) {
    return StatusCode::SUCCESS;
  }

  int arg1 = 500;
  double arg2 = 15.3;
  auto [result] =
      m_remoteGPUSvc->call<double>(ctx, *this, "test_fn", &arg1, &arg2);

  if (*result == arg1 + arg2) {
    ATH_MSG_INFO("Answer Correct!");
  } else {
    ATH_MSG_WARNING("Answer Incorrect!");
  }
  return StatusCode::SUCCESS;
}

HostPtr<double> RemoteGPUExampleAlg::test_fn(HostPtr<int> arg1,
                                             HostPtr<double> arg2) {
  auto* ans = new double;
  *ans = *arg1.ptr + *arg2.ptr;
  return {ans};
}
}  // namespace RemoteCall
