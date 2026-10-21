/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthExStoreGateExample/src/HandleTestTool3.h
 * @author Frank Winklmeier
 * @date July, 2022
 * @brief Test for DecorHandleKey depending on a regular handle key
 */

#include "HandleTestTool3.h"


namespace AthEx {

StatusCode HandleTestTool3::initialize()
{
  ATH_CHECK( m_rhKey.initialize() );
  ATH_CHECK( m_rdhKey.initialize() );
  ATH_CHECK( m_whKey.initialize() );
  ATH_CHECK( m_wdhKey.initialize() );
  ATH_CHECK( m_wdhKey2.initialize() );

  ATH_MSG_INFO( "RHKey    : {}", m_rhKey );
  ATH_MSG_INFO( "RDecorKey: {} {}", m_rdhKey, m_rdhKey.contHandleKey());
  ATH_MSG_INFO( "WHKey    : {}", m_whKey );
  ATH_MSG_INFO( "WDecorKey: {} {}", m_wdhKey, m_wdhKey.contHandleKey());
  ATH_MSG_INFO( "WDecorKey2: {} {}", m_wdhKey2, m_wdhKey2.contHandleKey());

  return StatusCode::SUCCESS;
}


} // namespace AthEx
