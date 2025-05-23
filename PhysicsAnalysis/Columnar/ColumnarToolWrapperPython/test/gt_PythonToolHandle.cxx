/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <AsgTesting/UnitTest.h>
#include <AsgTools/AsgToolConfig.h>
#include <AsgTools/AsgComponentFactories.h>
#include <ColumnarToolWrapperPython/PythonToolHandle.h>
#include <ColumnarExampleTools/SimpleSelectorExampleTool.h>
#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>
#include <mutex>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

//
// method implementations
//

namespace columnar
{
  namespace
  {
    /// \brief make a unique tool name to be used in unit tests
    std::string makeUniqueName ()
    {
      static std::atomic<unsigned> index = 0;
      return "unique" + std::to_string(++index);
    }
  }

  TEST (PythonToolHandleTest, baseUse)
  {
    // this test is only used in Array mode
    if (columnarAccessMode != 2)
      return;

    // at some point we need to have a better handling of tool factories
    // in AnalysisBase, but for now this does what I need.
    static std::once_flag onceFlag;
    std::call_once (onceFlag, []()
    {
      using namespace asg::msgUserCode;
      ANA_CHECK_THROW (asg::registerToolFactory<SimpleSelectorExampleTool> ("columnar::SimpleSelectorExampleTool"));
    });


    // create a basic python tool handle
    PythonToolHandle toolHandle;

    // set the type and name of the tool
    toolHandle.setTypeAndName ("columnar::SimpleSelectorExampleTool/" + makeUniqueName());

    // if we had properties we could set them here
    // toolHandle.setProperty ("Property", "Value");

    // initialize the tool with the set type, name and properties
    toolHandle.initialize();

    // set all the columns
    std::vector<ColumnarOffsetType> offsets;
    offsets.push_back (0);
    offsets.push_back (1);
    toolHandle.setColumn ("Particles", offsets.size(), offsets.data());
    std::vector<float> pt;
    std::vector<char> output;
    pt.push_back (10e5);
    output.resize (pt.size(), 0.);
    toolHandle.setColumn ("Particles.pt", pt.size(), pt.data());
    toolHandle.setColumn ("Particles.selection", output.size(), output.data());
    std::vector<ColumnarOffsetType> numEvents;
    numEvents.push_back (0);
    numEvents.push_back (offsets.size()-1);
    toolHandle.setColumn (numberOfEventsName, numEvents.size(), numEvents.data());

    // call the tool
    toolHandle.call();

    EXPECT_FLOAT_EQ (1, output[0]);
  }
}

ATLAS_GOOGLE_TEST_MAIN
