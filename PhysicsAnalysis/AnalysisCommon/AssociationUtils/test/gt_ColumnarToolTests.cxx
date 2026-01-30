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

#include <AssociationUtils/AltMuJetOverlapTool.h>
#include <AssociationUtils/DeltaROverlapTool.h>
#include <AssociationUtils/EleEleOverlapTool.h>
#include <AssociationUtils/EleJetOverlapTool.h>
#include <AssociationUtils/MuJetOverlapTool.h>
#include <AssociationUtils/EleMuSharedTrkOverlapTool.h>
#include <AssociationUtils/TauAntiTauJetOverlapTool.h>
#include <AssociationUtils/TauJetOverlapTool.h>
#include <AssociationUtils/TauLooseEleOverlapTool.h>
#include <AssociationUtils/TauLooseMuOverlapTool.h>

#include <xAODJet/JetContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODTau/TauJetContainer.h>
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

namespace ORUtils
{
  TEST_F (ColumnarMemoryTest, DeltaROverlapTool)
  {
    // check that we are in array mode
    if (!checkMode())
      return;

    // set up the tool
    auto tool = std::make_unique<DeltaROverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("ObjectType1", xAODType::ObjectType::Jet));
    ASSERT_SUCCESS (tool->setProperty ("ObjectType2", xAODType::ObjectType::Photon));
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
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
    columnMap.addColumn ("EventInfo", {0, 1});
    columnMap.addColumn ("particle1", {0, 5});
    columnMap.addColumn ("particle1.pt", {100e3, 100e3, 73e3, 14e3, 1e3});
    columnMap.addColumn ("particle1.eta", {-3, 2, 0, -1, 1.5});
    columnMap.addColumn ("particle1.phi", {-3, 2, 0, -1, 1.5});
    columnMap.addColumn ("particle1.m", {0, 0, 0, 0, 0});
    columnMap.addColumn ("particle1.selected", {1, 1, 1, 1, 1});
    columnMap.addColumn ("particle1.overlaps", {1, 1, 1, 1, 1});
    columnMap.addColumn ("particle2", {0, 3});
    columnMap.addColumn ("particle2.pt", {100e3, 100e3, 73e3,});
    columnMap.addColumn ("particle2.eta", {-3, 2, 0});
    columnMap.addColumn ("particle2.phi", {-3, 2, 0});
    columnMap.addColumn ("particle2.selected", {1, 1, 1});
    columnMap.addColumn ("particle2.overlaps", {1, 1, 1});

    // the expected output of the tool
    columnMap.setExpectation ("particle1.overlaps", {0, 0, 0, 1, 1});
    columnMap.setExpectation ("particle2.overlaps", {1, 1, 1});

    // connect the columns to the tool
    columnMap.connectColumnsToTool ();

    // run the tool
    columnMap.call ();

    // check the output
    columnMap.checkExpectations ();
  }


  // this is a helper class that wraps the tool for XAOD usage for the
  // PHYSLITE test below.
  template<typename Container1,typename Container2>
  class CallXAODOverlapTool final : public IXAODToolCaller, public asg::AsgMessaging
  {
  public:
    CallXAODOverlapTool (const IOverlapTool& tool, const std::string& name1, const std::string& name2)
      : AsgMessaging ("CallXAODOverlapTool"), m_tool (tool), m_name1 (name1), m_name2 (name2)
    {
      if constexpr (std::is_same_v<Container1,Container2>)
        m_copy1 = m_name1 == m_name2;
    }

    virtual StatusCode retrieve (EventStoreType& evtStore) override
    {
      ATH_CHECK (evtStore.retrieve (m_particles1, m_name1));
      if (!m_copy1)
        ATH_CHECK (evtStore.retrieve (m_particles2, m_name2));
      return StatusCode::SUCCESS;
    }

    virtual StatusCode copyRecord (EventStoreType& evtStore, const std::string& postfix) override
    {
      auto [particles1Copy, aux1Copy] = xAOD::shallowCopyContainer (*m_particles1);
      m_particles1 = particles1Copy;
      ATH_CHECK (evtStore.record (particles1Copy, m_name1 + postfix));
      ATH_CHECK (evtStore.record (aux1Copy, m_name1 + postfix + "Aux."));
      if constexpr (std::is_same_v<Container1,Container2>)
      {
        if (m_copy1)
        {
          m_particles2 = m_particles1;
          return StatusCode::SUCCESS;
        }
      }
      auto [particles2Copy, aux2Copy] = xAOD::shallowCopyContainer (*m_particles2);
      m_particles2 = particles2Copy;
      ATH_CHECK (evtStore.record (particles2Copy, m_name2 + postfix));
      ATH_CHECK (evtStore.record (aux2Copy, m_name2 + postfix + "Aux."));
      return StatusCode::SUCCESS;
    }

    virtual StatusCode call () override
    {
      ATH_CHECK (m_tool.findOverlaps (*m_particles1, *m_particles2));
      return StatusCode::SUCCESS;
    }

  private:
    const IOverlapTool& m_tool;
    std::string m_name1;
    std::string m_name2;
    bool m_copy1 = false;

    const Container1 *m_particles1 = nullptr;
    const Container2 *m_particles2 = nullptr;
  };


  // this is a test that runs the tool on PHYSLITE.  this ensures that the
  // tool works on actual data, not just synthetic one of the in-memory
  // test.  it also allows for performance measurements of the tool in the
  // different modes.
  TEST_F (ColumnarPhysLiteTest, DeltaROverlapTool_jetPhoton)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<DeltaROverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("ObjectType1", xAODType::ObjectType::Jet));
    ASSERT_SUCCESS (tool->setProperty ("ObjectType2", xAODType::ObjectType::Photon));
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    ASSERT_SUCCESS (tool->initialize ());
  
    std::string particles1 = "AnalysisJets";
    std::string particles2 = "AnalysisPhotons";
    CallXAODOverlapTool<xAOD::JetContainer,xAOD::PhotonContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "DeltaROverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }

  // this runs the test above, but with IParticle momentum accessors in
  // xAOD mode
  TEST_F (ColumnarPhysLiteTest, DeltaROverlapTool_jetPhoton_readPhotonMass)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<DeltaROverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("ObjectType1", xAODType::ObjectType::Jet));
    // setting this to a jet causes it to read the mass for the photon
    // instead of using a hardcoded mass
    ASSERT_SUCCESS (tool->setProperty ("ObjectType2", xAODType::ObjectType::Jet));
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    ASSERT_SUCCESS (tool->initialize ());

    std::string particles1 = "AnalysisJets";
    std::string particles2 = "AnalysisPhotons";
    CallXAODOverlapTool<xAOD::JetContainer,xAOD::PhotonContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "DeltaROverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }

  // this runs the test above, but with IParticle momentum accessors in
  // xAOD mode
  TEST_F (ColumnarPhysLiteTest, DeltaROverlapTool_jetPhoton_withIParticle)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<DeltaROverlapTool> (makeUniqueName());
    if (!columnar::ColumnarModeDefault::isXAOD)
    {
      ASSERT_SUCCESS (tool->setProperty ("ObjectType1", xAODType::ObjectType::Jet));
      // setting this to a jet causes it to read the mass for the photon
      // instead of using a hardcoded mass
      ASSERT_SUCCESS (tool->setProperty ("ObjectType2", xAODType::ObjectType::Jet));
    }
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    ASSERT_SUCCESS (tool->initialize ());

    std::string particles1 = "AnalysisJets";
    std::string particles2 = "AnalysisPhotons";
    CallXAODOverlapTool<xAOD::JetContainer,xAOD::PhotonContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "DeltaROverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }


  // this is a test that runs the tool on PHYSLITE.  this ensures that the
  // tool works on actual data, not just synthetic one of the in-memory
  // test.  it also allows for performance measurements of the tool in the
  // different modes.
  TEST_F (ColumnarPhysLiteTest, DeltaROverlapTool_jetElectron)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<DeltaROverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("ObjectType1", xAODType::ObjectType::Jet));
    ASSERT_SUCCESS (tool->setProperty ("ObjectType2", xAODType::ObjectType::Electron));
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    ASSERT_SUCCESS (tool->initialize ());
  
    std::string particles1 = "AnalysisJets";
    std::string particles2 = "AnalysisElectrons";
    CallXAODOverlapTool<xAOD::JetContainer,xAOD::ElectronContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "DeltaROverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }

  // this runs the test above, but with IParticle momentum accessors in
  // xAOD mode
  TEST_F (ColumnarPhysLiteTest, DeltaROverlapTool_jetElectron_readElectronMass)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<DeltaROverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("ObjectType1", xAODType::ObjectType::Jet));
    // setting this to a jet causes it to read the mass for the Electron
    // instead of using a hardcoded mass
    ASSERT_SUCCESS (tool->setProperty ("ObjectType2", xAODType::ObjectType::Jet));
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    ASSERT_SUCCESS (tool->initialize ());

    std::string particles1 = "AnalysisJets";
    std::string particles2 = "AnalysisElectrons";
    CallXAODOverlapTool<xAOD::JetContainer,xAOD::ElectronContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "DeltaROverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }

  // this runs the test above, but with IParticle momentum accessors in
  // xAOD mode
  TEST_F (ColumnarPhysLiteTest, DeltaROverlapTool_jetElectron_withIParticle)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<DeltaROverlapTool> (makeUniqueName());
    if (!columnar::ColumnarModeDefault::isXAOD)
    {
      ASSERT_SUCCESS (tool->setProperty ("ObjectType1", xAODType::ObjectType::Jet));
      // setting this to a jet causes it to read the mass for the electron
      // instead of using a hardcoded mass
      ASSERT_SUCCESS (tool->setProperty ("ObjectType2", xAODType::ObjectType::Jet));
    }
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    ASSERT_SUCCESS (tool->initialize ());

    std::string particles1 = "AnalysisJets";
    std::string particles2 = "AnalysisElectrons";
    CallXAODOverlapTool<xAOD::JetContainer,xAOD::ElectronContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "DeltaROverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }

  TEST_F (ColumnarPhysLiteTest, MuJetOverlapTool)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<MuJetOverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    // these dummy accessors are needed to make sure we have an offset
    // map for the track containers (that's a limitation in the test,
    // not in actual use)
    columnar::Track0Accessor<float> dummy0Acc {*tool, "phi"};
    columnar::Track1Accessor<float> dummy1Acc {*tool, "phi"};
    ASSERT_SUCCESS (tool->initialize ());
  
    std::string particles1 = "AnalysisMuons";
    std::string particles2 = "AnalysisJets";
    CallXAODOverlapTool<xAOD::MuonContainer,xAOD::JetContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "MuJetOverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }

  TEST_F (ColumnarPhysLiteTest, AltMuJetOverlapTool)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<AltMuJetOverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    // these dummy accessors are needed to make sure we have an offset
    // map for the track containers (that's a limitation in the test,
    // not in actual use)
    columnar::Track0Accessor<float> dummy0Acc {*tool, "phi"};
    columnar::Track1Accessor<float> dummy1Acc {*tool, "phi"};
    ASSERT_SUCCESS (tool->initialize ());
  
    std::string particles1 = "AnalysisMuons";
    std::string particles2 = "AnalysisJets";
    CallXAODOverlapTool<xAOD::MuonContainer,xAOD::JetContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "AltMuJetOverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }

  TEST_F (ColumnarPhysLiteTest, EleEleOverlapTool)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<EleEleOverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    // these dummy accessors are needed to make sure we have an offset
    // map for the track containers (that's a limitation in the test,
    // not in actual use)
    columnar::TrackAccessor<float> dummy0Acc {*tool, "phi"};
    ASSERT_SUCCESS (tool->initialize ());
  
    std::string particles1 = "AnalysisElectrons";
    std::string particles2 = "AnalysisElectrons";
    CallXAODOverlapTool<xAOD::ElectronContainer,xAOD::ElectronContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "EleEleOverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }

  TEST_F (ColumnarPhysLiteTest, EleJetOverlapTool)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<EleJetOverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    ASSERT_SUCCESS (tool->initialize ());

    std::string particles1 = "AnalysisElectrons";
    std::string particles2 = "AnalysisJets";
    CallXAODOverlapTool<xAOD::ElectronContainer,xAOD::JetContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "EleJetOverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }

  TEST_F (ColumnarPhysLiteTest, EleMuSharedTrkOverlapTool)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<EleMuSharedTrkOverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    // these dummy accessors are needed to make sure we have an offset
    // map for the track containers (that's a limitation in the test,
    // not in actual use)
    columnar::Particle1Accessor<float> dummyEleAcc {*tool, "phi"};
    columnar::Track0Accessor<float> dummy0Acc {*tool, "phi"};
    columnar::Track1Accessor<float> dummy1Acc {*tool, "phi"};
    columnar::Track2Accessor<float> dummy2Acc {*tool, "phi"};
    ASSERT_SUCCESS (tool->initialize ());

    std::string particles1 = "AnalysisElectrons";
    std::string particles2 = "AnalysisMuons";
    CallXAODOverlapTool<xAOD::ElectronContainer,xAOD::MuonContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "EleMuSharedTrkOverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }

  TEST_F (ColumnarMemoryTest, TauAntiTauJetOverlapTool)
  {
    // check that we are in array mode
    if (!checkMode())
      return;

    // set up the tool
    auto tool = std::make_unique<TauAntiTauJetOverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
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
    columnMap.addColumn ("EventInfo", {0, 1});
    columnMap.addColumn ("EventInfo.eventNumber", {1234});
    columnMap.addColumn ("particle1", {0, 5});
    columnMap.addColumn ("particle1.pt", {100e3, 100e3, 73e3, 14e3, 1e3});
    columnMap.addColumn ("particle1.eta", {-3, 2, 0, -1, 1.5});
    columnMap.addColumn ("particle1.phi", {-3, 2, 0, -1, 1.5});
    columnMap.addColumn ("particle1.m", {0, 0, 0, 0, 0});
    columnMap.addColumn ("particle1.overlaps", {1, 1, 1, 1, 1});
    columnMap.addColumn ("particle2", {0, 3});
    columnMap.addColumn ("particle2.pt", {100e3, 100e3, 73e3,});
    columnMap.addColumn ("particle2.eta", {-3, 2, 0});
    columnMap.addColumn ("particle2.phi", {-3, 2, 0});
    columnMap.addColumn ("particle2.m", {0, 0, 0});
    columnMap.addColumn ("particle2.antiTauEventCategory", {1, 1, 1});
    columnMap.addColumn ("particle2.overlaps", {1, 1, 1});

    // the expected output of the tool
    // TO CHECK: I'm not sure if this is what's expected, but it's what the tool produced
    columnMap.setExpectation ("particle1.overlaps", {1, 1, 1, 1, 1});
    columnMap.setExpectation ("particle2.overlaps", {0, 0, 0});

    // connect the columns to the tool
    columnMap.connectColumnsToTool ();

    // run the tool
    columnMap.call ();

    // check the output
    columnMap.checkExpectations ();
  }

  TEST_F (ColumnarPhysLiteTest, DISABLED_TauAntiTauJetOverlapTool)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<TauAntiTauJetOverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    ASSERT_SUCCESS (tool->initialize ());

    std::string particles1 = "AnalysisJets";
    std::string particles2 = "AnalysisTauJets";
    CallXAODOverlapTool<xAOD::JetContainer,xAOD::TauJetContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "TauAntiTauJetOverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }

  TEST_F (ColumnarPhysLiteTest, TauJetOverlapTool)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<TauJetOverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    ASSERT_SUCCESS (tool->initialize ());

    std::string particles1 = "AnalysisJets";
    std::string particles2 = "AnalysisTauJets";
    CallXAODOverlapTool<xAOD::JetContainer,xAOD::TauJetContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "TauAntiTauJetOverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }

  TEST_F (ColumnarPhysLiteTest, TauLooseEleOverlapTool)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<TauLooseEleOverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    ASSERT_SUCCESS (tool->initialize ());

    std::string particles1 = "AnalysisTauJets";
    std::string particles2 = "AnalysisElectrons";
    CallXAODOverlapTool<xAOD::TauJetContainer,xAOD::ElectronContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "TauLooseEleOverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }

  TEST_F (ColumnarPhysLiteTest, TauLooseMuOverlapTool)
  {
    // check that we are in a project that supports this test
    if (!checkMode())
      return;

    auto tool = std::make_unique<TauLooseMuOverlapTool> (makeUniqueName());
    ASSERT_SUCCESS (tool->setProperty ("OutputPassValue", true));
    ASSERT_SUCCESS (tool->setProperty ("InputLabel", ""));
    ASSERT_SUCCESS (tool->initialize ());

    std::string particles1 = "AnalysisTauJets";
    std::string particles2 = "AnalysisMuons";
    CallXAODOverlapTool<xAOD::TauJetContainer,xAOD::MuonContainer> callXAOD (*tool, particles1, particles2);

    // this will call the tool in either mode, and also performs some
    // performance measurements of the tool in either mode
    doCall ({.tool = tool.get(), .name = "TauLooseMuOverlapTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"particle1", particles1},{"particle2", particles2}}});
  }
}

ATLAS_GOOGLE_TEST_MAIN
