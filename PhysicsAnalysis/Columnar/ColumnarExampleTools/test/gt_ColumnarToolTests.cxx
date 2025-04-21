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
#include <ColumnarTestFixtures/ColumnarPhysliteTest.h>

#include <ColumnarExampleTools/SimpleSelectorExampleTool.h>
#include <ColumnarExampleTools/OptionalColumnExampleTool.h>
#include <ColumnarExampleTools/ConfigurableColumnExampleTool.h>
#include <ColumnarExampleTools/ModularExampleTool.h>
#include <ColumnarExampleTools/StringExampleTool.h>

#include <xAODJet/JetContainer.h>
#include <xAODCore/ShallowCopy.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

//
// method implementations
//

// All the tests could instead be placed inside the `columnar`
// namespace, but for actual tools the tool may actually exist in a
// different namespace, so I usually use a `using` statement like this.
using columnar::ColumnarMemoryTest;
using columnar::ColumnarPhysLiteTest;


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


// this is a helper function that wraps the tool for XAOD usage for the
// PHYSLITE test below.  there is usually some amount of boilerplate
// code that test needs to run in XAOD mode, which is usually factored
// out into a separate function.
void callXAODSimpleSelectorExampleTool (const columnar::SimpleSelectorExampleTool& tool, bool isPrepCall, const std::string& name)
{
  using namespace asg::msgUserCode;
  const xAOD::JetContainer *jets = nullptr;
  ANA_CHECK_THROW (tool.evtStore()->retrieve (jets, name));
  // to allow for accurate performance measurements this function is
  // generally called twice, the first time is mostly to make sure that
  // all data is loaded into memory, and the second time is then used as
  // a performance measurement of the tool without i/o.
  if (isPrepCall)
  {
    // for the first preparatory call we make a copy of the object to
    // avoid the tool modifying the original object.
    auto [jetsCopy, auxCopy] = xAOD::shallowCopyContainer (*jets);
    tool.callSingleEvent (*jetsCopy);
    delete jetsCopy;
    delete auxCopy;
  } else
  {
    // for the second call we can skip the shallow copy, as this tool
    // just adds to the existing object.  for tools that modify the
    // object in place both paths would be identical, making shallow
    // copies and then recording them (the TStore is cleared between
    // both calls).
    tool.callSingleEvent (*jets);
  }
}


// this is a test that runs the tool on PHYSLITE.  this ensures that the
// tool works on actual data, not just synthetic one of the in-memory
// test.  it also allows for performance measurements of the tool in the
// different modes.
TEST_F (ColumnarPhysLiteTest, SimpleSelectorExampleTool)
{
  // check that we are in a project that supports this test
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::SimpleSelectorExampleTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->initialize ());

  // this will call the tool in either mode, and also performs some
  // performance measurements of the tool in either mode
  doCall (*tool, "SimpleSelectorExampleTool", "AnalysisJets", [&] (auto& args) {callXAODSimpleSelectorExampleTool (*tool, args.isPrepCall, args.inputContainer);}, {{"Particles", "AnalysisJets"}});
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



TEST_F (ColumnarMemoryTest, StringExampleTool)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::StringExampleTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->initialize ());

  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize ();

  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;
  std::cout << "recommended systematics size: " << toolHandle.getRecommendedSystematics().size() << std::endl;

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 1});

  columnMap.addColumn ("Met", {0, 2});
  columnMap.addColumn ("Met.name.offset", {0, 9, 14});
  columnMap.addColumn ("Met.name.data", {'I', 'n', 'v', 'i', 's', 'i', 'b', 'l', 'e', 'F', 'i', 'n', 'a', 'l'});
  columnMap.addColumn ("Met.selection", {0, 0});

  columnMap.setExpectation ("Met.selection", {0, 1});

  columnMap.connectColumnsToTool ();

  columnMap.call ();

  columnMap.checkExpectations ();
}

ATLAS_GOOGLE_TEST_MAIN
