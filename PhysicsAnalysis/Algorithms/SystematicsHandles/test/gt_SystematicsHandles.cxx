/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <AsgMessaging/MessageCheck.h>
#include <AsgServices/AsgServiceConfig.h>
#include <AsgTesting/UnitTest.h>
#include <AsgTools/AsgTool.h>
#include <AsgTools/CurrentContext.h>
#include <xAODRootAccess/Init.h>
#include <xAODRootAccess/TEvent.h>
#include <xAODRootAccess/TStore.h>
#include <xAODJet/JetContainer.h>
#include <xAODJet/JetAuxContainer.h>
#include <SystematicsHandles/ISystematicsSvc.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <AsgServices/ServiceHandle.h>
#include <memory>

//
// method implementations
//

using namespace asg::msgUserCode;

namespace
{
  /// @brief record a jet container (with aux store) into the active transient
  /// store under @c name, mimicking an input/copy container in the CP sequence.
  xAOD::JetContainer *recordJets (xAOD::TStore& store, const std::string& name,
                                  std::size_t n = 3)
  {
    auto jets = std::make_unique<xAOD::JetContainer> ();
    auto aux = std::make_unique<xAOD::JetAuxContainer> ();
    jets->setStore (aux.get());
    for (std::size_t i = 0; i < n; ++i)
    {
      auto *jet = new xAOD::Jet ();
      jets->push_back (jet);
      jet->setJetP4 (xAOD::JetFourMom_t (40000.0, 0.5, 0.5, 1000.0));
    }
    auto *ptr = jets.get();
    EXPECT_SUCCESS (store.record (std::move (aux), name + "Aux."));
    EXPECT_SUCCESS (store.record (std::move (jets), name));
    return ptr;
  }

  /// @brief create a SystematicsSvc named "SystematicsSvc" (what the
  /// SysListHandle's ServiceHandle resolves by default).
  StatusCode makeSystematicsSvc (std::shared_ptr<CP::ISystematicsSvc>& svc,
                                 const std::vector<std::string>& systematicsList = {})
  {
    asg::AsgServiceConfig config ("CP::SystematicsSvc/SystematicsSvc");
    ANA_CHECK (config.setProperty ("systematicsList", systematicsList));
    ANA_CHECK (config.makeService (svc));
    return StatusCode::SUCCESS;
  }
}


// The core reproduction: the nominal copy `AnaJets_STEP1_NOSYS` (the analogue of
// the failing `AnaMuons_STEP1_NOSYS`) must be produced by the SysCopyHandle and
// then be retrievable by the next algorithm's SysReadHandle.
TEST (SystematicsHandlesTest, copyAndReadBackNominal)
{
  xAOD::TEvent event;
  xAOD::TStore store;

  std::shared_ptr<CP::ISystematicsSvc> svc;
  ASSERT_SUCCESS (makeSystematicsSvc (svc));

  auto tool = std::make_unique<asg::AsgTool> ("CopyTool");

  // the systematics handles, declared on the (blank) tool as a CP algorithm
  // would, but kept local to the test for flexibility
  CP::SysListHandle systematicsList {tool.get()};
  // the copy handle: reads `jets`, writes `jetsOut` (set as a property)
  CP::SysCopyHandle<xAOD::JetContainer> copyHandle {
    tool.get(), "jets", "AnaJets_%SYS%", "the jets to copy"};
  // the next algorithm reading the copy back
  CP::SysReadHandle<xAOD::JetContainer> readHandle {
    tool.get(), "readJets", "AnaJets_STEP1_%SYS%", "the copy to read back"};

  ASSERT_SUCCESS (tool->setProperty ("jetsOut", "AnaJets_STEP1_%SYS%"));
  ASSERT_SUCCESS (tool->initialize ());

  ASSERT_SUCCESS (copyHandle.initialize (systematicsList));
  ASSERT_SUCCESS (readHandle.initialize (systematicsList));
  ASSERT_SUCCESS (systematicsList.initialize ());

  // the nominal input the copy handle reads
  auto *input = recordJets (store, "AnaJets_NOSYS");

  const auto& sysList = systematicsList.systematicsVector ();
  ASSERT_EQ (1u, sysList.size ());          // just nominal
  const CP::SystematicSet& sys = sysList.front ();
  ASSERT_EQ ("", sys.name ());              // nominal == empty set

  // run the copy
  xAOD::JetContainer *copy = nullptr;
  ASSERT_SUCCESS (copyHandle.getCopy (copy, sys));
  ASSERT_NE (nullptr, copy);
  ASSERT_NE (input, copy);                  // a real (shallow) copy was made
  ASSERT_EQ (input->size (), copy->size ());

  // the copy must be recorded under exactly the name the reader expects
  ASSERT_EQ ("AnaJets_STEP1_NOSYS", readHandle.getName (sys));
  EXPECT_TRUE (store.contains<xAOD::JetContainer> ("AnaJets_STEP1_NOSYS"));

  // the next algorithm reads it back
  const xAOD::JetContainer *readBack = nullptr;
  ASSERT_SUCCESS (readHandle.retrieve (readBack, sys));
  ASSERT_EQ (copy, readBack);
}


// Same cycle, but with an actual systematic variation affecting the input, so
// the per-systematic name expansion (%SYS% -> NOSYS / -> the variation) is
// exercised on both the write and the read side.
TEST (SystematicsHandlesTest, copyAndReadBackWithSystematic)
{
  xAOD::TEvent event;
  xAOD::TStore store;

  const CP::SystematicVariation var ("JET_Fake__1up");
  CP::SystematicSet affecting;
  affecting.insert (var);

  std::shared_ptr<CP::ISystematicsSvc> svc;
  ASSERT_SUCCESS (makeSystematicsSvc (svc, {"", "JET_Fake__1up"}));
  // register the variation as affecting, and mark the input as affected by it
  ASSERT_SUCCESS (svc->addSystematics (affecting, affecting));
  ASSERT_SUCCESS (svc->setObjectSystematics ("AnaJets_%SYS%", affecting));

  auto tool = std::make_unique<asg::AsgTool> ("CopyTool2");

  CP::SysListHandle systematicsList {tool.get()};
  CP::SysCopyHandle<xAOD::JetContainer> copyHandle {
    tool.get(), "jets", "AnaJets_%SYS%", "the jets to copy"};
  CP::SysReadHandle<xAOD::JetContainer> readHandle {
    tool.get(), "readJets", "AnaJets_STEP1_%SYS%", "the copy to read back"};

  ASSERT_SUCCESS (tool->setProperty ("jetsOut", "AnaJets_STEP1_%SYS%"));
  ASSERT_SUCCESS (tool->initialize ());

  ASSERT_SUCCESS (copyHandle.initialize (systematicsList));
  ASSERT_SUCCESS (readHandle.initialize (systematicsList));
  ASSERT_SUCCESS (systematicsList.initialize ());

  // both nominal and varied inputs exist in the store
  recordJets (store, "AnaJets_NOSYS");
  recordJets (store, "AnaJets_JET_Fake__1up");

  const auto& sysList = systematicsList.systematicsVector ();
  ASSERT_EQ (2u, sysList.size ());          // nominal + the variation

  for (const CP::SystematicSet& sys : sysList)
  {
    xAOD::JetContainer *copy = nullptr;
    ASSERT_SUCCESS (copyHandle.getCopy (copy, sys));
    ASSERT_NE (nullptr, copy);

    const std::string expected = sys.name ().empty ()
      ? "AnaJets_STEP1_NOSYS" : "AnaJets_STEP1_JET_Fake__1up";
    ASSERT_EQ (expected, readHandle.getName (sys));
    EXPECT_TRUE (store.contains<xAOD::JetContainer> (expected));

    const xAOD::JetContainer *readBack = nullptr;
    ASSERT_SUCCESS (readHandle.retrieve (readBack, sys));
    ASSERT_EQ (copy, readBack);
  }
}

ATLAS_GOOGLE_TEST_MAIN
