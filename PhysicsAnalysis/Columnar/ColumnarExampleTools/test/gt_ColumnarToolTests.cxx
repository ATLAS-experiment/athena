/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgMessaging/AsgMessaging.h>
#include <AsgTesting/UnitTest.h>
#include <AsgTools/AsgToolConfig.h>
#include <ColumnarTestFixtures/ColumnarMemoryTest.h>
#include <ColumnarTestFixtures/ColumnarPhysliteTest.h>

#include <ColumnarExampleTools/SimpleSelectorExampleTool.h>
#include <ColumnarExampleTools/OptionalColumnExampleTool.h>
#include <ColumnarExampleTools/ConfigurableColumnExampleTool.h>
#include <ColumnarExampleTools/ModularExampleTool.h>
#include <ColumnarExampleTools/MomentumAccessorExampleTool.h>
#include <ColumnarExampleTools/StringExampleTool.h>
#include <ColumnarExampleTools/VariantExampleTool.h>

#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>
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
using columnar::TestUtils::IXAODToolCaller;


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
class XAODSimpleSelectorExampleToolCaller final : public IXAODToolCaller, public asg::AsgMessaging
{
public:
  XAODSimpleSelectorExampleToolCaller (const columnar::SimpleSelectorExampleTool& tool, const std::string& name)
    : AsgMessaging ("XAODSimpleSelectorExampleToolCaller"), m_tool (tool), m_name (name)
  {}

  virtual StatusCode retrieve (EventStoreType& evtStore) override
  {
    ANA_CHECK (evtStore.retrieve (m_jets, m_name));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode copyRecord (EventStoreType& evtStore, const std::string& postfix) override
  {
    auto [jetsCopy, auxCopy] = xAOD::shallowCopyContainer (*m_jets);
    m_jets = jetsCopy;
    ANA_CHECK (evtStore.record (jetsCopy, m_name + postfix));
    ANA_CHECK (evtStore.record (auxCopy, m_name + postfix + "Aux."));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode call () override
  {
    m_tool.callSingleEvent (*m_jets);
    return StatusCode::SUCCESS;
  }

private:
  const columnar::SimpleSelectorExampleTool& m_tool;
  std::string m_name;

  const xAOD::JetContainer *m_jets = nullptr;
};


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

  XAODSimpleSelectorExampleToolCaller xAODToolCaller (*tool, "AnalysisJets");

  // this will call the tool in either mode, and also performs some
  // performance measurements of the tool in either mode
  doCall (*tool, "SimpleSelectorExampleTool", "AnalysisJets", xAODToolCaller, {{"Particles", "AnalysisJets"}});
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


// in-memory test for the momentum accessor example tool
TEST_F (ColumnarMemoryTest, MomentumAccessorExampleTool)
{
  // check that we are in array mode
  if (!checkMode())
    return;

  // set up the tool
  auto tool = std::make_unique<columnar::MomentumAccessorExampleTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("ObjectType", static_cast<unsigned>(xAODType::ObjectType::Jet)));
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
  columnMap.addColumn ("Particles.eta", {0, 3, 0});
  columnMap.addColumn ("Particles.phi", {0, 0, 0});
  columnMap.addColumn ("Particles.m", {0, 0, 0});
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
template<typename ContainerType>
class XAODMomentumAccessorExampleToolCaller final : public IXAODToolCaller, public asg::AsgMessaging
{
public:
  XAODMomentumAccessorExampleToolCaller (const columnar::MomentumAccessorExampleTool& tool, const std::string& name)
    : AsgMessaging ("XAODMomentumAccessorExampleToolCaller"), m_tool (tool), m_name (name)
  {}

  virtual StatusCode retrieve (EventStoreType& evtStore) override
  {
    ATH_CHECK (evtStore.retrieve (m_jets, m_name));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode copyRecord (EventStoreType& evtStore, const std::string& postfix) override
  {
    auto [jetsCopy, auxCopy] = xAOD::shallowCopyContainer (*m_jets);
    m_jets = jetsCopy;
    ATH_CHECK (evtStore.record (jetsCopy, m_name + postfix));
    ATH_CHECK (evtStore.record (auxCopy, m_name + postfix + "Aux."));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode call () override
  {
    m_tool.callSingleEvent (*m_jets);
    return StatusCode::SUCCESS;
  }

private:
  const columnar::MomentumAccessorExampleTool& m_tool;
  std::string m_name;

  const ContainerType *m_jets = nullptr;
};


// this is a test that runs the momentum accessor example tool on
// PHYSLITE
TEST_F (ColumnarPhysLiteTest, MomentumAccessorExampleTool)
{
  // check that we are in a project that supports this test
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::MomentumAccessorExampleTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("ObjectType", static_cast<unsigned>(xAODType::ObjectType::Jet)));
  ASSERT_SUCCESS (tool->initialize ());

  XAODMomentumAccessorExampleToolCaller<xAOD::JetContainer> xAODToolCaller (*tool, "AnalysisJets");

  // this will call the tool in either mode, and also performs some
  // performance measurements of the tool in either mode
  doCall (*tool, "MomentumAccessorExampleTool", "AnalysisJets", xAODToolCaller, {{"Particles", "AnalysisJets"}});
}


// another test for the momentum accessor example tool on PHYSLITE, but
// this time for photons. this is mostly here to see the speed
// difference for massless particles (which simplifies some momentum
// calculations).
TEST_F (ColumnarPhysLiteTest, MomentumAccessorExampleTool_photons)
{
  // check that we are in a project that supports this test
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::MomentumAccessorExampleTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("ObjectType", static_cast<unsigned>(xAODType::ObjectType::Photon)));
  ASSERT_SUCCESS (tool->initialize ());

  XAODMomentumAccessorExampleToolCaller<xAOD::PhotonContainer> xAODToolCaller (*tool, "AnalysisPhotons");

  // this will call the tool in either mode, and also performs some
  // performance measurements of the tool in either mode
  doCall (*tool, "MomentumAccessorExampleTool", "AnalysisPhotons", xAODToolCaller, {{"Particles", "AnalysisPhotons"}});
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

TEST_F (ColumnarMemoryTest, VariantExampleTool)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::VariantExampleTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->initialize ());

  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize ();

  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;
  std::cout << "recommended systematics size: " << toolHandle.getRecommendedSystematics().size() << std::endl;

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 1});

  columnMap.addColumn ("AnalysisElectrons", {0, 2});
  columnMap.addColumn ("AnalysisElectrons.pt", {10e3, 50e3});
  columnMap.addColumn ("AnalysisElectrons.eta", {0, -1});
  columnMap.addColumn ("AnalysisElectrons.ptRank", {0, 0});
  columnMap.addColumn ("AnalysisElectrons.etaRank", {0, 0});

  columnMap.setExpectation ("AnalysisElectrons.ptRank", {2, 0});
  columnMap.setExpectation ("AnalysisElectrons.etaRank", {0, 2});

  columnMap.addColumn ("AnalysisMuons", {0, 1});
  columnMap.addColumn ("AnalysisMuons.pt", {30e3});
  columnMap.addColumn ("AnalysisMuons.eta", {0.5});
  columnMap.addColumn ("AnalysisMuons.ptRank", {0});

  columnMap.setExpectation ("AnalysisMuons.ptRank", {1});

  columnMap.connectColumnsToTool ();

  columnMap.call ();

  columnMap.checkExpectations ();
}

// this is a helper function for the PHYSLITE test below.  there is
// usually some amount of boilerplate code that test needs to run in
// XAOD mode, which is usually factored out into a separate function.
class XAODVariantExampleToolCaller final : public IXAODToolCaller, public asg::AsgMessaging
{
public:
  XAODVariantExampleToolCaller (const columnar::VariantExampleTool& tool, const std::string& electronName, const std::string& muonName)
    : AsgMessaging("XAODVariantExampleToolCaller"), m_tool (tool), m_electronName (electronName), m_muonName (muonName)
  {}

  virtual StatusCode retrieve (EventStoreType& evtStore) override
  {
    ANA_CHECK (evtStore.retrieve (m_electrons, m_electronName));
    ANA_CHECK (evtStore.retrieve (m_muons, m_muonName));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode copyRecord (EventStoreType& evtStore, const std::string& postfix) override
  {
    auto [electronsCopy, electronsAuxCopy] = xAOD::shallowCopyContainer (*m_electrons);
    m_electrons = electronsCopy;
    ANA_CHECK (evtStore.record (electronsCopy, m_electronName + postfix));
    ANA_CHECK (evtStore.record (electronsAuxCopy, m_electronName + postfix + "Aux."));
    auto [muonsCopy, muonsAuxCopy] = xAOD::shallowCopyContainer (*m_muons);
    m_muons = muonsCopy;
    ANA_CHECK (evtStore.record (muonsCopy, m_muonName + postfix));
    ANA_CHECK (evtStore.record (muonsAuxCopy, m_muonName + postfix + "Aux."));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode call () override
  {
    m_tool.callSingleEvent (*m_electrons, *m_muons);
    return StatusCode::SUCCESS;
  }

private:
  const columnar::VariantExampleTool& m_tool;
  std::string m_electronName;
  std::string m_muonName;

  const xAOD::ElectronContainer *m_electrons = nullptr;
  const xAOD::MuonContainer *m_muons = nullptr;
};

// this is a test that runs the tool on PHYSLITE.  this ensures that the
// tool works on actual data, not just synthetic one of the in-memory
// test.  it also allows for performance measurements of the tool in the
// different modes.
TEST_F (ColumnarPhysLiteTest, VariantExampleTool)
{
  // check that we are in a project that supports this test
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::VariantExampleTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->initialize ());

  XAODVariantExampleToolCaller xAODToolCaller (*tool, "AnalysisElectrons", "AnalysisMuons");

  // this will call the tool in either mode, and also performs some
  // performance measurements of the tool in either mode
  doCall (*tool, "VariantExampleTool", "AnalysisElectrons", xAODToolCaller, {{}});
}

ATLAS_GOOGLE_TEST_MAIN
