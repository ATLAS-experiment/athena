/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgTesting/UnitTest.h>
#include <AsgTools/AsgToolConfig.h>
#include <ColumnarTestFixtures/ColumnarMemoryTest.h>

#include <ColumnarExampleTools/SimpleSelectorExampleTool.h>
#include <ColumnarExampleTools/OptionalColumnExampleTool.h>
#include <ColumnarExampleTools/ConfigurableColumnExampleTool.h>
#include <ColumnarExampleTools/ModularExampleTool.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

//
// method implementations
//

// All the tests could instead be placed inside the `columnar`
// namespace, but for actual tools the tool may actually exist in a
// different namespace, so I usually use a `using` statement like this.
using columnar::ColumnarMemoryTest;


// this is a test that manually loads data into memory, and then runs
// the columnar tool on it.  this is useful for ensuring that the tool
// actually works and the relevant code paths have all been converted.
//
// this is about as close as we can get to a unit test for the tool as a
// whole, as it checks exactly defined inputs and outputs.  however,
// full CP tools are usually so complex that it is often not feasible to
// do full unit testing this way.  at times it is even hard to find a
// set of inputs that is even valid at all.
TEST_F (ColumnarMemoryTest, SimpleSelectorExampleTool)
{
  // check that we are in array mode
  if (!checkMode())
    return;

  // set up the tool
  auto tool = std::make_unique<columnar::SimpleSelectorExampleTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->initialize ());

  // this is a wrapper around the tool for this test
  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize ();

  // print out some information about the tool
  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;
  std::cout << "recommended systematics size: " << toolHandle.getRecommendedSystematics().size() << std::endl;

  // the in-memory data frame we are filling
  ColumnMapType columnMap {toolHandle};

  // the various columns we are loading into memory
  columnMap.addColumn ("EventInfo", {0, 2});
  columnMap.addColumn ("Particles", {0, 1, 3});
  columnMap.addColumn ("Particles.pt", {10e5, 10e5, 1e3});
  columnMap.addColumn ("Particles.selection", {0, 0, 0});

  // the expected output of the tool
  columnMap.setExpectation ("Particles.selection", {1, 1, 0});

  // connect the columns to the tool
  columnMap.connectColumnsToTool ();

  // run the tool
  columnMap.call ();

  // check the output
  columnMap.checkExpectations ();
}


TEST_F (ColumnarMemoryTest, OptionalColumnExampleTool_present)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::OptionalColumnExampleTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->initialize ());

  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize ();

  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;
  std::cout << "recommended systematics size: " << toolHandle.getRecommendedSystematics().size() << std::endl;

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 2});

  columnMap.addColumn ("Particles", {0, 1, 3});
  columnMap.addColumn ("Particles.pt", {10e5, 10e5, 1e3});
  columnMap.addColumn ("Particles.ptCorr", {10e5, 1e3, 10e5});
  columnMap.addColumn ("Particles.selection", {0, 0, 0});

  columnMap.setExpectation ("Particles.selection", {1, 0, 1});

  columnMap.connectColumnsToTool ();

  columnMap.call ();

  columnMap.checkExpectations ();
}


TEST_F (ColumnarMemoryTest, OptionalColumnExampleTool_absent)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::OptionalColumnExampleTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->initialize ());

  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize ();

  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;
  std::cout << "recommended systematics size: " << toolHandle.getRecommendedSystematics().size() << std::endl;

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 2});

  columnMap.addColumn ("Particles", {0, 1, 3});
  columnMap.addColumn ("Particles.pt", {10e5, 10e5, 1e3});
  columnMap.addColumn ("Particles.selection", {0, 0, 0});

  columnMap.setExpectation ("Particles.selection", {1, 1, 0});

  columnMap.connectColumnsToTool ();

  columnMap.call ();

  columnMap.checkExpectations ();
}


TEST_F (ColumnarMemoryTest, ConfigurableColumnExampleTool)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::ConfigurableColumnExampleTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("ptVar", "ptCorr"));
  ASSERT_SUCCESS (tool->initialize ());

  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize ();

  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;
  std::cout << "recommended systematics size: " << toolHandle.getRecommendedSystematics().size() << std::endl;

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 2});

  columnMap.addColumn ("Particles", {0, 1, 3});
  columnMap.addColumn ("Particles.ptCorr", {10e5, 10e5, 1e3});
  columnMap.addColumn ("Particles.selection", {0, 0, 0});

  columnMap.setExpectation ("Particles.selection", {1, 1, 0});

  columnMap.connectColumnsToTool ();

  columnMap.call ();

  columnMap.checkExpectations ();
}


TEST_F (ColumnarMemoryTest, ModularExampleTool)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::ModularExampleTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->initialize ());

  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize ();

  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;
  std::cout << "recommended systematics size: " << toolHandle.getRecommendedSystematics().size() << std::endl;

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 2});

  columnMap.addColumn ("Particles", {0, 1, 3});
  columnMap.addColumn ("Particles.pt", {10e5, 10e5, 1e3});
  columnMap.addColumn ("Particles.eta", {0, 3, 0});
  columnMap.addColumn ("Particles.selection", {0, 0, 0});

  columnMap.setExpectation ("Particles.selection", {1, 0, 0});

  columnMap.connectColumnsToTool ();

  columnMap.call ();

  columnMap.checkExpectations ();
}

ATLAS_GOOGLE_TEST_MAIN
