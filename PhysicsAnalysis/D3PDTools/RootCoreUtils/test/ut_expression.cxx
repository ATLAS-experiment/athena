/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/StringUtil.h>

//
// main program
//

using namespace RCU;

int main ()
{
/// STL version
  RCU_ASSERT_SOFT(match_expr(std::regex(R"(.*\.root.*)"), "test.root_4"));
  RCU_ASSERT_SOFT(!match_expr(std::regex(R"(.*\.root.*)"), "test.asdroot_4"));
  RCU_ASSERT_SOFT(!match_expr(std::regex(R"(.*\.root)"), "test.root_4"));
  RCU_ASSERT_SOFT(match_expr(std::regex(glob_to_regexp("*.root*")), "test.root_4"));
  RCU_ASSERT_SOFT(!match_expr(std::regex(glob_to_regexp("*.root*")), "test.asdroot_4"));
  const std::string someNonsenseName{"thingy-majjig.suffix.suffix"};
  const std::string result = RCU::substitute(RCU::substitute(someNonsenseName, ".", "p"), "-", "_");
  RCU_ASSERT_SOFT(result == "thingy_majjigpsuffixpsuffix");
}
