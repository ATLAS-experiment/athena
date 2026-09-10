// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s)
#include "RemoteGPUExampleAlg.h"

// System include(s)
#include <array>
#include <span>

namespace RemoteCall {

StatusCode RemoteGPUExampleAlg::initialize() {
  ATH_CHECK(m_remoteGPUSvc.retrieve());
  ATH_CHECK(m_remoteGPUSvc->registerFunction(test_fn, "test_fn"));
  ATH_CHECK(m_remoteGPUSvc->registerFunction(test_range, "test_range"));
  return StatusCode::SUCCESS;
}

StatusCode RemoteGPUExampleAlg::execute(const EventContext& ctx) const {
  if (m_remoteGPUSvc->isServer())
    return StatusCode::SUCCESS;

  int arg1 = 500;
  double arg2 = 15.3;
  auto [result] =
      m_remoteGPUSvc->call<double>(ctx, *this, "test_fn", &arg1, &arg2);
  if (*result != arg1 + arg2) {
    ATH_MSG_ERROR("Answer Incorrect! Scalar RPC result mismatch");
    return StatusCode::FAILURE;
  }

  // Vary the range length and values by event to check empty ranges and result
  // matching.
  std::array<double, 4> storage{double(ctx.evt()), 1.5, -2.0, 4.0};
  std::span<double> values(storage.data(), ctx.evt() % (storage.size() + 1));
  auto [doubled, count] = m_remoteGPUSvc->call<std::span<double>, std::size_t>(
      ctx, *this, "test_range", &values);
  if (doubled.size != values.size() || *count != values.size()) {
    ATH_MSG_ERROR("Answer Incorrect! Range RPC size mismatch");
    return StatusCode::FAILURE;
  }
  for (std::size_t i = 0; i < values.size(); ++i) {
    if (doubled.ptr[i] != 2 * values[i]) {
      ATH_MSG_ERROR("Answer Incorrect! Range RPC value mismatch");
      return StatusCode::FAILURE;
    }
  }
  ATH_MSG_INFO("Answer Correct!");
  return StatusCode::SUCCESS;
}

RPCRet<Host, double> RemoteGPUExampleAlg::test_fn(RPCArg<Host, int> arg1,
                                                  RPCArg<Host, double> arg2) {
  return {new double(*arg1.ptr + *arg2.ptr)};
}

std::tuple<RPCRet<Host, std::span<double>>, RPCRet<Host, std::size_t>>
RemoteGPUExampleAlg::test_range(RPCArg<Host, std::span<double>> values) {
  RPCRet<Host, std::span<double>> result(new double[values.size], values.size);
  for (std::size_t i = 0; i < values.size; ++i)
    result.ptr[i] = 2 * values.ptr[i];
  return {std::move(result),
          RPCRet<Host, std::size_t>(new std::size_t(values.size))};
}

}  // namespace RemoteCall
