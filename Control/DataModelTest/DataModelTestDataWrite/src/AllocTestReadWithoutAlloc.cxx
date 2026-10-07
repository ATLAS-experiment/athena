/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataWrite/src/AllocTestReadWithoutAlloc.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Test reading AllocTest with a non-default allocator.
 */


#include "AllocTestReadWithoutAlloc.h"
#include "DataModelTestDataWrite/AllocTestAuxContainer.h"
#include <print>
#include <sstream>


namespace DMTest {


/**
 * @brief Gaudi initialize method.
 */
StatusCode AllocTestReadWithoutAlloc::initialize()
{
  ATH_CHECK( m_containerKey.initialize() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode AllocTestReadWithoutAlloc::execute (const EventContext& ctx) const
{
  static const SG::Accessor<int> atInt3 ("atInt3");
  static const SG::Accessor<int> atInt4 ("atInt4");

  // Write to a sstream first, to avpod having the output broken up by
  // schema evolution messges.
  std::ostringstream ss;

  SG::ReadHandle<AllocTestContainer> cont (m_containerKey, ctx);
  std::print (ss, "{} ", m_containerKey.key());
  for (const AllocTest* at : *cont) {
    std::print (ss, "{} {} {} {} ",
                at->atInt1(), at->atInt2(), atInt3(*at), atInt4(*at));
  }
  std::println ("{}", ss.str());
  return StatusCode::SUCCESS;
}


} // namespace DMTest
