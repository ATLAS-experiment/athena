// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthenaKernel/DummyRCUSvc.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2019
 * @brief A no-op implementation of IRCUSvc, for use in unit tests.
 */


#ifndef ATHENAKERNEL_DUMMYRCUSVC_H
#define ATHENAKERNEL_DUMMYRCUSVC_H


#include "AthenaKernel/IRCUSvc.h"
#include <cstdlib>


namespace Athena_test {


class DummyRCUSvc
  : public implements<Athena::IRCUSvc>
{
public:
  virtual StatusCode remove (Athena::IRCUObject* /*obj*/) override
  {
    return StatusCode::SUCCESS;
  }
  virtual size_t getNumSlots() const override { return 1; }
  virtual void add (Athena::IRCUObject* /*obj*/) override
  { }
};


} // namespace Athena_test


#endif // not ATHENAKERNEL_DUMMYRCUSVC_H
