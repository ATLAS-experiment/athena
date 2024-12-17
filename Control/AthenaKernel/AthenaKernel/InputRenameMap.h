// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthenaKernel/InputRenameMap.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2024
 * @brief Map keeping track of requested input renames.
 *
 * Broken out from IInputRename in order to reduce header dependencies.
 */


#ifndef ATHENAKERNEL_INPUTRENAMEMAP_H
#define ATHENAKERNEL_INPUTRENAMEMAP_H


#include "CxxUtils/sgkey_t.h"
#include <string>


namespace Athena {


/// Type of the input rename map: sgkey_t -> sgkey_t.
struct InputRenameEntry
{
  SG::sgkey_t m_sgkey;
  std::string m_key;
};
using InputRenameMap_t = SG::SGKeyMap<InputRenameEntry>;


} // namespace Athena


#endif // not ATHENAKERNEL_INPUTRENAMEMAP_H
