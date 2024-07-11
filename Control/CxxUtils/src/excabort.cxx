/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/src/excabort.cc
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2024
 * @brief Preload library to convert some exceptions to abort.
 *
 * The C++ library tends to report errors by raising exceptions.
 * This can however be annoying, since there is then generally no information
 * as to where the exception happened.  While we do have the exctrace code
 * as a partial workaround for this, this requires code changes at the
 * point where exceptions are caught, and it adds overhead to all
 * thrown exceptions.
 *
 * Here we try a different workaround.  Some exceptions never really make
 * any sense to catch, so we can just turn them into unconditional aborts.
 * If we preload this in athena, then we can get stack tracebacks
 * when these occur.
 */


#include <cstdlib>


namespace std {

void
__throw_bad_array_new_length()
{
  std::abort();
}


}
