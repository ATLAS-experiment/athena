/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 macro to assert an error condition
 ----------------------------------
 ATLAS Collaboration
 ***************************************************************************/


#ifndef TEST_SGASSERT_H
# define TEST_SGASSERT_H

#include <cassert>
#include <iostream>
#include <print>

#undef NDEBUG

#define SGASSERT( TRUEEXPR ) assert(TRUEEXPR)
#define SGASSERTERROR( FALSEEXPR )   \
    std::println (std::cerr, "Now we expect to see an error message:\n"   \
                  "----Error Message Starts--->>");                       \
    assert(!(FALSEEXPR));                                                 \
    std::println (std::cerr, "<<---Error Message Ends-------");


#endif // TEST_SGASSERT_H
