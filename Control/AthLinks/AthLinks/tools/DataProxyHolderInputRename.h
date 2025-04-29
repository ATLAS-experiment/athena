// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthLinks/DataProxyHolderInputRename.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2024
 * @brief Set map used for performing input renaming in toTransient.
 *
 * Broken out from DataProxyHolder.h to reduce header dependencies.
 */


#ifndef ATHLINKS_DATAPROXYHOLDERINPUTRENAME_H
#define ATHLINKS_DATAPROXYHOLDERINPUTRENAME_H


#include "AthenaKernel/IInputRename.h"
#include "CxxUtils/checker_macros.h"


namespace SG {


/**
 * @brief Set map used for performing input renaming in toTransient.
 * @param map The new map, or nullptr for no renmaing.
 */
void setDataProxyHolderInputRenameMap ATLAS_NOT_THREAD_SAFE
  (const Athena::IInputRename::InputRenameRCU_t* map);


} // namespace SG


#endif // not ATHLINKS_DATAPROXYHOLDERINPUTRENAME_H
