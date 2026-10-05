/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataWrite/src/AllocTestWriteWithoutAlloc.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2022
 * @brief Test writing AllocTest with a non-default allocator.
 */


#include "AllocTestWriteWithoutAlloc.h"
#include "DataModelTestDataWrite/AllocTestAuxContainer.h"


namespace DMTest {


/**
 * @brief Gaudi initialize method.
 */
StatusCode AllocTestWriteWithoutAlloc::initialize()
{
  ATH_CHECK( m_containerKey.initialize() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode AllocTestWriteWithoutAlloc::execute (const EventContext& ctx) const
{
  static const SG::Accessor<int> atInt3 ("atInt3");
  static const SG::Accessor<int> atInt4 ("atInt4");

  SG::WriteHandle<AllocTestContainer> cont (m_containerKey, ctx);
  ATH_CHECK( cont.record (std::make_unique<AllocTestContainer>(),
                          std::make_unique<AllocTestAuxContainer>()) );

  for (size_t i = 0; i < 10; i++) {
    cont->push_back (std::make_unique<AllocTest>());
    cont->back()->setAtInt1 (ctx.evt()*100 + i);
    cont->back()->setAtInt2 (ctx.evt()*100 + i + 10);
    atInt3(*cont->back()) = ctx.evt()*100 + i + 20;
    atInt4(*cont->back()) = ctx.evt()*100 + i + 30;
  }
  return StatusCode::SUCCESS;
}


} // namespace DMTest
