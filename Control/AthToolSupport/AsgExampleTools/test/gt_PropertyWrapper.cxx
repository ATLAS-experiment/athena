/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <AsgTesting/UnitTest.h>
#include <cmath>
#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

//
// method implementations
//

namespace asg
{
  TEST (PropertyWrapperTest, Property_int)
  {
    AsgTool tool ("tool");
    Gaudi::Property<int> property (&tool, "property", 7, "description");
    ASSERT_EQ (7, property.value());
    ASSERT_SUCCESS (tool.setProperty ("property", 8));
    ASSERT_EQ (8, property.value());
    std::ostringstream os;
    os << property;
    ASSERT_EQ ("'property': 8", os.str());
  }
}

ATLAS_GOOGLE_TEST_MAIN
