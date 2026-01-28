/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <AsgMessaging/AsgMessaging.h>
#include <AsgTesting/UnitTest.h>
#include <AsgTools/AsgToolConfig.h>
#include <ColumnarTestFixtures/ColumnarMemoryTest.h>
#include <ColumnarTestFixtures/ColumnarPhysliteTest.h>

#include <METUtilities/ColumnarMETMaker.h>
#include <xAODMissingET/MissingETAuxContainer.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

//
// method implementations
//

using columnar::ColumnarMemoryTest;
using columnar::ColumnarPhysLiteTest;
using columnar::TestUtils::IXAODToolCaller;

TEST_F (ColumnarMemoryTest, METMaker_muon)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<met::ColumnarMETMaker> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("JetContainer", "dummyjets"));
  ASSERT_SUCCESS (tool->setProperty ("skipSystematicJetSelection", false));
  ASSERT_SUCCESS (tool->setProperty ("JetSelection", "Tight"));
  ASSERT_SUCCESS (tool->setProperty ("DoPFlow", true));
  ASSERT_SUCCESS (tool->setProperty ("DoSetMuonJetEMScale", true));
  ASSERT_SUCCESS (tool->setProperty ("JetConstitScaleMom", ""));
  ASSERT_SUCCESS (tool->setProperty ("columnarTermName", "Muons"));
  ASSERT_SUCCESS (tool->setProperty ("columnarParticleType", unsigned(xAOD::Type::Muon)));
  ASSERT_SUCCESS (tool->initialize ());
  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize ();

  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;
  std::cout << "recommended systematics size: " << toolHandle.getRecommendedSystematics().size() << std::endl;

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 1});
  columnMap.addColumn ("Photons", {0, 0});
  // columnMap.addColumn ("Particles1", {0, 0});

  columnMap.addColumn ("Particles", {0, 1});
  columnMap.addColumn ("Particles.pt", {10e5});
  columnMap.addColumn ("Particles.eta", {1});
  columnMap.addColumn ("Particles.phi", {1});
  columnMap.addColumn ("Particles.m", {0});
  columnMap.addColumn ("Particles.objectType", {unsigned(xAOD::Type::Muon)});

  columnMap.addColumn ("Jets", {0, 0});
  columnMap.addColumn ("Jets.pt", {});
  columnMap.addColumn ("Jets.eta", {});
  columnMap.addColumn ("Jets.phi", {});
  columnMap.addColumn ("Jets.m", {});
  columnMap.addColumn ("Jets.Width", {});
  columnMap.addColumn ("Jets.EnergyPerSampling.offset", {0});
  columnMap.addColumn ("Jets.EnergyPerSampling.data", {});
  columnMap.addColumn ("Jets.EMFrac", {});
  columnMap.addColumn ("Jets.PSFrac", {});
  columnMap.addColumn ("Jets.SumPtTrkPt500.offset", {0});
  columnMap.addColumn ("Jets.SumPtTrkPt500.data", {});
  columnMap.addColumn ("Jets.NumTrkPt500.offset", {0});
  columnMap.addColumn ("Jets.NumTrkPt500.data", {});
  // columnMap.addColumn ("Jets.constituentLinks.offset", {0});
  // columnMap.addColumn ("Jets.constituentLinks.data", {});

  columnMap.addColumn ("Muons", {0, 0});
  // columnMap.addColumn ("Muons.inDetTrackParticleLink", {});

  columnMap.addColumn ("Electrons", {0, 0});
  columnMap.addColumn ("Electrons.pt", {});

  // columnMap.addColumn ("InDetTrackParticles", {0, 0});
  // columnMap.addColumn ("InDetTrackParticles.pt", {});

  columnMap.addColumn ("MetAssoc", {0, 1});
  columnMap.addColumn ("MetAssoc.overlapIndices.outerOffset", {0, 1});
  columnMap.addColumn ("MetAssoc.overlapIndices.innerOffset", {0, 1});
  columnMap.addColumn ("MetAssoc.overlapIndices.data", {0});
  columnMap.addColumn ("MetAssoc.overlapTypes.outerOffset", {0, 1});
  columnMap.addColumn ("MetAssoc.overlapTypes.innerOffset", {0, 1});
  columnMap.addColumn ("MetAssoc.overlapTypes.data", {0});
  columnMap.addColumn ("MetAssoc.calkey.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.calkey.data", {0xff});
  columnMap.addColumn ("MetAssoc.calpx.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.calpx.data", {10e5});
  columnMap.addColumn ("MetAssoc.calpy.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.calpy.data", {10e5});
  columnMap.addColumn ("MetAssoc.calpz.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.calpz.data", {10e5});
  columnMap.addColumn ("MetAssoc.calsumpt.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.calsumpt.data", {10e5});
  columnMap.addColumn ("MetAssoc.cale.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.cale.data", {10e5});
  columnMap.addColumn ("MetAssoc.objectLinks.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.objectLinks.data", {0});
  columnMap.addColumn ("MetAssoc.objectLinks.keys", {0,1,2,3,4});
  columnMap.addColumn ("MetAssoc.trkkey.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.trkkey.data", {0xff});
  columnMap.addColumn ("MetAssoc.trkpx.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.trkpx.data", {10e5});
  columnMap.addColumn ("MetAssoc.trkpy.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.trkpy.data", {10e5});
  columnMap.addColumn ("MetAssoc.trkpz.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.trkpz.data", {10e5});
  columnMap.addColumn ("MetAssoc.trksumpt.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.trksumpt.data", {10e5});
  columnMap.addColumn ("MetAssoc.trke.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.trke.data", {10e5});
  columnMap.addColumn ("MetAssoc.jetLink", {columnar::invalidObjectIndex});
  columnMap.addColumn ("MetAssoc.jettrkpx", {10e4});
  columnMap.addColumn ("MetAssoc.jettrkpy", {10e4});
  columnMap.addColumn ("MetAssoc.jettrkpz", {10e4});
  columnMap.addColumn ("MetAssoc.jettrksumpt", {10e4});
  columnMap.addColumn ("MetAssoc.jettrke", {10e4});
  columnMap.addColumn ("MetAssoc.isMisc", {0});

  std::vector<std::string> inputMetTerms = {};
  std::vector<std::string> outputMetTerms = {"Muons", "MuonEloss"};

  columnMap.addColumn ("METCore", {0, inputMetTerms.size()});
  columnMap.addColumn ("METCore.source", {});
  columnMap.addColumn ("METCore.mpx", {});
  columnMap.addColumn ("METCore.mpy", {});
  columnMap.addColumn ("METCore.sumet", {});

  columnMap.addColumn ("OutputMET", {0, outputMetTerms.size()});

  columnMap.addColumn ("Particles.MetObjectWeight", {0});
  columnMap.addColumn ("MetAssoc.useObjectFlags", {0});
  {
    std::vector<columnar::ColumnarOffsetType> nameOffsets;
    nameOffsets.push_back (0);
    std::vector<char> nameData;
    std::vector<std::size_t> nameHash;
    for (auto& name : inputMetTerms)
    {
      nameData.insert (nameData.end(), name.begin(), name.end());
      nameOffsets.push_back (nameData.size());
      nameHash.push_back (std::hash<std::string>{}(name));
    }
    columnMap.addTypedColumn ("METCore.name.offset", std::move (nameOffsets));
    columnMap.addTypedColumn ("METCore.name.data", std::move (nameData));
    columnMap.addTypedColumn ("METCore.nameHash", std::move (nameHash));
  }

  {
    std::vector<columnar::ColumnarOffsetType> nameOffsets;
    nameOffsets.push_back (0);
    std::vector<char> nameData;
    std::vector<std::size_t> nameHash;
    for (auto& name : outputMetTerms)
    {
      nameData.insert (nameData.end(), name.begin(), name.end());
      nameOffsets.push_back (nameData.size());
      nameHash.push_back (std::hash<std::string>{}(name));
    }
    columnMap.addTypedColumn ("OutputMET.name.offset", std::move (nameOffsets));
    columnMap.addTypedColumn ("OutputMET.name.data", std::move (nameData));
    columnMap.addTypedColumn ("OutputMET.nameHash", std::move (nameHash));
  }
  columnMap.addTypedColumn ("OutputMET.source", std::vector<std::uint64_t> (outputMetTerms.size(), 0));
  columnMap.addTypedColumn ("OutputMET.mpx", std::vector<float> (outputMetTerms.size(), 0));
  columnMap.addTypedColumn ("OutputMET.mpy", std::vector<float> (outputMetTerms.size(), 0));
  columnMap.addTypedColumn ("OutputMET.sumet", std::vector<float> (outputMetTerms.size(), 0));

  columnMap.setExpectation ("OutputMET.source", {65544, 262152});
  columnMap.setExpectation ("OutputMET.mpx", {-540302.3125, 0});
  columnMap.setExpectation ("OutputMET.mpy", {-841471, 0});
  columnMap.setExpectation ("OutputMET.sumet", {1000000, 0});
  columnMap.setExpectation ("Particles.MetObjectWeight", {1});
  columnMap.setExpectation ("MetAssoc.useObjectFlags", {1});

  columnMap.connectColumnsToTool ();

  columnMap.call ();

  columnMap.checkExpectations ();
}


TEST_F (ColumnarMemoryTest, METMaker_jet)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<met::ColumnarMETMaker> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("JetContainer", "dummyjets"));
  ASSERT_SUCCESS (tool->setProperty ("skipSystematicJetSelection", false));
  ASSERT_SUCCESS (tool->setProperty ("JetSelection", "Tight"));
  ASSERT_SUCCESS (tool->setProperty ("DoPFlow", true));
  ASSERT_SUCCESS (tool->setProperty ("DoSetMuonJetEMScale", true));
  ASSERT_SUCCESS (tool->setProperty ("columnarOperation", 1));
  ASSERT_SUCCESS (tool->setProperty ("columnarJetKey", "RefJet"));
  ASSERT_SUCCESS (tool->setProperty ("columnarSoftClusKey", "PVSoftTrk"));
  ASSERT_SUCCESS (tool->setProperty ("columnarTermName", "RefJet"));
  ASSERT_SUCCESS (tool->initialize ());
  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize ();

  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;
  std::cout << "recommended systematics size: " << toolHandle.getRecommendedSystematics().size() << std::endl;

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 1});
  columnMap.addColumn ("Photons", {0, 0});
  // columnMap.addColumn ("Particles1", {0, 0});

  columnMap.addColumn ("Particles", {0, 0});
  columnMap.addColumn ("Particles.pt", {});
  columnMap.addColumn ("Particles.eta", {});
  columnMap.addColumn ("Particles.phi", {});
  columnMap.addColumn ("Particles.m", {});
  columnMap.addColumn ("Particles.objectType", {});

  columnMap.addColumn ("Jets", {0, 1});
  columnMap.addColumn ("Jets.pt", {10e5});
  columnMap.addColumn ("Jets.eta", {1});
  columnMap.addColumn ("Jets.phi", {1});
  columnMap.addColumn ("Jets.m", {0});
  columnMap.addColumn ("Jets.Width", {0});
  columnMap.addColumn ("Jets.EnergyPerSampling.offset", {0, 0});
  columnMap.addColumn ("Jets.EnergyPerSampling.data", {});
  columnMap.addColumn ("Jets.EMFrac", {0});
  columnMap.addColumn ("Jets.PSFrac", {0});
  columnMap.addColumn ("Jets.SumPtTrkPt500.offset", {0, 1});
  columnMap.addColumn ("Jets.SumPtTrkPt500.data", {10e3});
  columnMap.addColumn ("Jets.NumTrkPt500.offset", {0, 1});
  columnMap.addColumn ("Jets.NumTrkPt500.data", {5});
  // columnMap.addColumn ("Jets.constituentLinks.offset", {0, 0});
  // columnMap.addColumn ("Jets.constituentLinks.data", {});
  columnMap.addColumn ("Jets.JetConstitScaleMomentum_pt", {10e5});
  columnMap.addColumn ("Jets.JetConstitScaleMomentum_eta", {1});
  columnMap.addColumn ("Jets.JetConstitScaleMomentum_phi", {1});
  columnMap.addColumn ("Jets.JetConstitScaleMomentum_m", {0});

  columnMap.addColumn ("Muons", {0, 0});
  // columnMap.addColumn ("Muons.inDetTrackParticleLink", {});

  columnMap.addColumn ("Electrons", {0, 0});
  columnMap.addColumn ("Electrons.pt", {});

  // columnMap.addColumn ("InDetTrackParticles", {0, 0});
  // columnMap.addColumn ("InDetTrackParticles.pt", {});

  columnMap.addColumn ("MetAssoc", {0, 1});
  columnMap.addColumn ("MetAssoc.overlapIndices.outerOffset", {0, 1});
  columnMap.addColumn ("MetAssoc.overlapIndices.innerOffset", {0, 1});
  columnMap.addColumn ("MetAssoc.overlapIndices.data", {0});
  columnMap.addColumn ("MetAssoc.overlapTypes.outerOffset", {0, 1});
  columnMap.addColumn ("MetAssoc.overlapTypes.innerOffset", {0, 1});
  columnMap.addColumn ("MetAssoc.overlapTypes.data", {0});
  columnMap.addColumn ("MetAssoc.calkey.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.calkey.data", {0xff});
  columnMap.addColumn ("MetAssoc.calpx.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.calpx.data", {10e5});
  columnMap.addColumn ("MetAssoc.calpy.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.calpy.data", {10e5});
  columnMap.addColumn ("MetAssoc.calpz.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.calpz.data", {10e5});
  columnMap.addColumn ("MetAssoc.calsumpt.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.calsumpt.data", {10e5});
  columnMap.addColumn ("MetAssoc.cale.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.cale.data", {10e5});
  columnMap.addColumn ("MetAssoc.objectLinks.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.objectLinks.data", {0});
  columnMap.addColumn ("MetAssoc.objectLinks.keys", {0,1,2,3,4});
  columnMap.addColumn ("MetAssoc.trkkey.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.trkkey.data", {0xff});
  columnMap.addColumn ("MetAssoc.trkpx.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.trkpx.data", {10e5});
  columnMap.addColumn ("MetAssoc.trkpy.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.trkpy.data", {10e5});
  columnMap.addColumn ("MetAssoc.trkpz.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.trkpz.data", {10e5});
  columnMap.addColumn ("MetAssoc.trksumpt.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.trksumpt.data", {10e5});
  columnMap.addColumn ("MetAssoc.trke.offset", {0, 1});
  columnMap.addColumn ("MetAssoc.trke.data", {10e5});
  columnMap.addColumn ("MetAssoc.jetLink", {0});
  columnMap.addColumn ("MetAssoc.jettrkpx", {10e4});
  columnMap.addColumn ("MetAssoc.jettrkpy", {10e4});
  columnMap.addColumn ("MetAssoc.jettrkpz", {10e4});
  columnMap.addColumn ("MetAssoc.jettrksumpt", {10e4});
  columnMap.addColumn ("MetAssoc.jettrke", {10e4});
  columnMap.addColumn ("MetAssoc.isMisc", {0});

  std::vector<std::string> inputMetTerms = {"PVSoftTrkCore"};
  std::vector<std::string> outputMetTerms = {"MuonEloss", "RefJet", "PVSoftTrk"};

  columnMap.addColumn ("METCore", {0, inputMetTerms.size()});
  columnMap.addColumn ("METCore.source", {0xff});
  columnMap.addColumn ("METCore.mpx", {5e4});
  columnMap.addColumn ("METCore.mpy", {5e4});
  columnMap.addColumn ("METCore.sumet", {5e4});

  columnMap.addColumn ("OutputMET", {0, outputMetTerms.size()});

  columnMap.addColumn ("Particles.MetObjectWeight", {});
  columnMap.addColumn ("Jets.MetObjectWeight", {0});
  columnMap.addColumn ("Jets.MetObjectWeightSoft", {0});
  columnMap.addColumn ("MetAssoc.useObjectFlags", {0});
  {
    std::vector<columnar::ColumnarOffsetType> nameOffsets;
    nameOffsets.push_back (0);
    std::vector<char> nameData;
    std::vector<std::size_t> nameHash;
    for (auto& name : inputMetTerms)
    {
      nameData.insert (nameData.end(), name.begin(), name.end());
      nameOffsets.push_back (nameData.size());
      nameHash.push_back (std::hash<std::string>{}(name));
    }
    columnMap.addTypedColumn ("METCore.name.offset", std::move (nameOffsets));
    columnMap.addTypedColumn ("METCore.name.data", std::move (nameData));
    columnMap.addTypedColumn ("METCore.nameHash", std::move (nameHash));
  }

  {
    std::vector<columnar::ColumnarOffsetType> nameOffsets;
    nameOffsets.push_back (0);
    std::vector<char> nameData;
    std::vector<std::size_t> nameHash;
    for (auto& name : outputMetTerms)
    {
      nameData.insert (nameData.end(), name.begin(), name.end());
      nameOffsets.push_back (nameData.size());
      nameHash.push_back (std::hash<std::string>{}(name));
    }
    columnMap.addTypedColumn ("OutputMET.name.offset", std::move (nameOffsets));
    columnMap.addTypedColumn ("OutputMET.name.data", std::move (nameData));
    columnMap.addTypedColumn ("OutputMET.nameHash", std::move (nameHash));
  }

  {
    std::vector<std::uint64_t> source;
    source.push_back (65544);
    source.resize (outputMetTerms.size(), 0);
    columnMap.addTypedColumn ("OutputMET.source", std::move (source));
  }
  columnMap.addTypedColumn ("OutputMET.mpx", std::vector<float> (outputMetTerms.size(), 0));
  columnMap.addTypedColumn ("OutputMET.mpy", std::vector<float> (outputMetTerms.size(), 0));
  columnMap.addTypedColumn ("OutputMET.sumet", std::vector<float> (outputMetTerms.size(), 0));

  columnMap.setExpectation ("OutputMET.source", {65544, 65552, 255});
  columnMap.setExpectation ("OutputMET.mpx", {0, -540302.31, 50000});
  columnMap.setExpectation ("OutputMET.mpy", {0, -841471, 50000});
  columnMap.setExpectation ("OutputMET.sumet", {0, 1000000, 50000});
  columnMap.setExpectation ("Particles.MetObjectWeight", {});
  columnMap.setExpectation ("Jets.MetObjectWeight", {1});
  columnMap.setExpectation ("Jets.MetObjectWeightSoft", {0});
  columnMap.setExpectation ("MetAssoc.useObjectFlags", {0});

  columnMap.connectColumnsToTool ();

  columnMap.call ();

  columnMap.checkExpectations ();
}



class XAODToolCallerMuon final : public IXAODToolCaller, public asg::AsgMessaging
{
public:
  XAODToolCallerMuon (const met::ColumnarMETMaker& tool, std::string inputContainer)
    : AsgMessaging ("XAODToolCallerMuon"), m_tool (tool), m_inputContainer (std::move (inputContainer)) {}

  virtual StatusCode retrieve (EventStoreType& evtStore) override
  {
    ANA_CHECK (evtStore.retrieve (m_particles, m_inputContainer));
    ANA_CHECK (evtStore.retrieve (m_metAssoc, "METAssoc_AnalysisMET"));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode copyRecord (EventStoreType& evtStore, const std::string& postfix) override
  {
    auto met = std::make_unique<xAOD::MissingETContainer>();
    auto metAux = std::make_unique<xAOD::MissingETAuxContainer>();
    met->setStore (metAux.get());
    m_met = met.get();
    ANA_CHECK (evtStore.record (std::move (met), "AnaMET" + postfix));
    ANA_CHECK (evtStore.record (std::move (metAux), "AnaMET" + postfix + "Aux."));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode call () override
  {
    xAOD::MissingETAssociationHelper helper (m_metAssoc);
    ANA_CHECK_THROW (m_tool.rebuildMET ("Muons", xAOD::Type::Muon, m_met, m_particles, helper, MissingETBase::UsageHandler::PhysicsObject));
    return StatusCode::SUCCESS;
  }

private:
  const met::ColumnarMETMaker& m_tool;
  std::string m_inputContainer;

  const xAOD::IParticleContainer *m_particles = nullptr;
  xAOD::MissingETContainer *m_met = nullptr;
  const xAOD::MissingETAssociationMap *m_metAssoc = nullptr;
};



TEST_F (ColumnarPhysLiteTest, METMaker_muon)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<met::ColumnarMETMaker> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("JetContainer", "dummyjets"));
  ASSERT_SUCCESS (tool->setProperty ("skipSystematicJetSelection", false));
  ASSERT_SUCCESS (tool->setProperty ("JetSelection", "Tight"));
  ASSERT_SUCCESS (tool->setProperty ("DoPFlow", true));
  ASSERT_SUCCESS (tool->setProperty ("DoSetMuonJetEMScale", true));
  ASSERT_SUCCESS (tool->setProperty ("JetConstitScaleMom", ""));
  ASSERT_SUCCESS (tool->setProperty ("columnarTermName", "Muons"));
  ASSERT_SUCCESS (tool->setProperty ("columnarParticleType", unsigned(xAOD::Type::Muon)));
  columnar::PhotonAccessor<float> photonPtAcc (*tool, "pt"); // this works around a limitation in the test fixture
  ASSERT_SUCCESS (tool->initialize ());

  XAODToolCallerMuon callXAODMuon (*tool, "AnalysisMuons");

  doCall ({.tool = tool.get(), .name = "ColumnarMETMaker", .xAODToolCaller = &callXAODMuon, .containerRenames = {{"Particles", "AnalysisMuons"}, {"Photons", "AnalysisPhotons"}, {"Electrons", "AnalysisElectrons"}, {"Muons", "AnalysisMuons"}, {"MetAssoc", "METAssoc_AnalysisMET"}, {"Jets", "AnalysisJets"}, {"METCore", "MET_Core_AnalysisMET"}}, .metTermNames = {"Muons", "MuonEloss"}});
}

class XAODToolCallerJet final : public IXAODToolCaller, public asg::AsgMessaging
{
public:
  XAODToolCallerJet (const met::ColumnarMETMaker& tool, std::string inputContainer)
    : AsgMessaging ("XAODToolCallerJet"), m_tool (tool), m_inputContainer (std::move (inputContainer)) {}

  virtual StatusCode retrieve (EventStoreType& evtStore) override
  {
    ANA_CHECK (evtStore.retrieve (m_particles, m_inputContainer));
    ANA_CHECK (evtStore.retrieve (m_metCore, "MET_Core_AnalysisMET"));
    ANA_CHECK (evtStore.retrieve (m_metAssoc, "METAssoc_AnalysisMET"));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode copyRecord (EventStoreType& evtStore, const std::string& postfix) override
  {
    auto met = std::make_unique<xAOD::MissingETContainer>();
    auto metAux = std::make_unique<xAOD::MissingETAuxContainer>();
    met->setStore (metAux.get());
    m_met = met.get();
    ANA_CHECK (evtStore.record (std::move (met), "AnaMET" + postfix));
    ANA_CHECK (evtStore.record (std::move (metAux), "AnaMET" + postfix + "Aux."));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode call () override
  {
    xAOD::MissingETAssociationHelper helper (m_metAssoc);
    ANA_CHECK_THROW (m_tool.rebuildJetMET ("Jets", "SoftClus", "PVSoftTrk", m_met, m_particles, m_metCore, helper, false));
    return StatusCode::SUCCESS;
  }

private:
  const met::ColumnarMETMaker& m_tool;
  std::string m_inputContainer;

  const xAOD::JetContainer *m_particles = nullptr;
  xAOD::MissingETContainer *m_met = nullptr;
  const xAOD::MissingETContainer *m_metCore = nullptr;
  const xAOD::MissingETAssociationMap *m_metAssoc = nullptr;
};

TEST_F (ColumnarPhysLiteTest, METMaker_jet)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<met::ColumnarMETMaker> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("JetContainer", "dummyjets"));
  ASSERT_SUCCESS (tool->setProperty ("skipSystematicJetSelection", false));
  ASSERT_SUCCESS (tool->setProperty ("JetSelection", "Tight"));
  ASSERT_SUCCESS (tool->setProperty ("DoPFlow", true));
  ASSERT_SUCCESS (tool->setProperty ("DoSetMuonJetEMScale", false)); // default is true, but we set it to false for this test, as the muon term has not been calculated
  ASSERT_SUCCESS (tool->setProperty ("columnarOperation", 1));
  ASSERT_SUCCESS (tool->setProperty ("columnarJetKey", "RefJet"));
  ASSERT_SUCCESS (tool->setProperty ("columnarSoftClusKey", "PVSoftTrk"));
  ASSERT_SUCCESS (tool->setProperty ("columnarTermName", "RefJet"));
  columnar::MuonAccessor<float> muonPtAcc (*tool, "pt"); // this works around a limitation in the test fixture
  columnar::PhotonAccessor<float> photonPtAcc (*tool, "pt"); // this works around a limitation in the test fixture
  ASSERT_SUCCESS (tool->initialize ());

  XAODToolCallerJet callXAODJet (*tool, "AnalysisJets");

  doCall ({.tool = tool.get(), .name = "ColumnarMETMaker", .xAODToolCaller = &callXAODJet, .containerRenames = {{"Particles", "AnalysisJets"}, {"Photons", "AnalysisPhotons"}, {"Electrons", "AnalysisElectrons"}, {"Muons", "AnalysisMuons"}, {"MetAssoc", "METAssoc_AnalysisMET"}, {"Jets", "AnalysisJets"}, {"METCore", "MET_Core_AnalysisMET"}}, .metTermNames = {"RefJet", "MuonEloss", "PVSoftTrk"}});
}

ATLAS_GOOGLE_TEST_MAIN
