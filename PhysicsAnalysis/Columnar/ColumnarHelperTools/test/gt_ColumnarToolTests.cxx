/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <AsgTesting/UnitTest.h>
#include <ColumnarCore/ColumnarDef.h>
#include <ColumnarInterfaces/KnownSgKeys.h>
#include <ColumnarTestFixtures/ColumnarMemoryTest.h>

#include <ColumnarHelperTools/ColumnarLinkTool.h>

#include <stdexcept>


//
// method implementations
//

using columnar::ColumnarMemoryTest;


// In-memory test that exercises the link-merging logic on a single
// event with two known target containers (`AnalysisJets`,
// `InDetTrackContainer`) and one empty link.
TEST_F (ColumnarMemoryTest, ColumnarLinkTool_known_keys)
{
  // The package is only built in ColumnarModeArray, but keep the
  // mode check for consistency with the example-tool tests.
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::ColumnarLinkTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("targetContainerNames", std::vector<std::string>{"AnalysisJets", "InDetTrackParticles"}));
  ASSERT_SUCCESS (tool->setProperty ("errorOnUnknownKey", false));
  ASSERT_SUCCESS (tool->initialize());

  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize();

  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;

  ColumnMapType columnMap {toolHandle};

    const auto jetKey = columnar::knownSgKeys.at ("AnalysisJets");
    const auto trkKey = columnar::knownSgKeys.at ("InDetTrackParticles");
    const auto eleKey = columnar::knownSgKeys.at ("AnalysisElectrons");

  // one event, three particles
  columnMap.addColumn ("EventInfo", {0, 2});
  columnMap.addColumn ("Container", {0, 3, 6});
  // per-event offset columns for the two registered targets
  columnMap.addColumn ("AnalysisJets", {0, 10, 18});
  columnMap.addColumn ("InDetTrackParticles", {0, 25, 45});
  columnMap.addColumn ("Container.linkIndex", {5, 10, 0, 7, 15, 8});
  columnMap.addColumn ("Container.linkSGKey", {jetKey, trkKey, SG::sgkey_t{0}, jetKey, trkKey, eleKey});
  columnMap.addColumn ("Container.outLink", {0, 0, 0, 0, 0, 0});

  using CM = columnar::ColumnarModeArray;
  columnMap.setExpectation ("Container.outLink",
                            {CM::mergeLinkKeyIndex (0, 5),
                             CM::mergeLinkKeyIndex (1, 10),
                             CM::LinkIndexType {CM::invalidLinkValue},
                             CM::mergeLinkKeyIndex (0, 17),
                             CM::mergeLinkKeyIndex (1, 40),
                             CM::mergeLinkKeyIndex (0xfe, 8)});

  columnMap.connectColumnsToTool ();
  columnMap.call ();
  columnMap.checkExpectations ();
}


// check we throw for an unknown key, as expected
TEST_F (ColumnarMemoryTest, ColumnarLinkTool_unknown_key_throws)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::ColumnarLinkTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty
    ("targetContainerNames", std::vector<std::string>{"AnalysisJets"}));
  ASSERT_SUCCESS (tool->initialize());

  const auto eleKey = columnar::knownSgKeys.at ("AnalysisElectrons");

  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize();

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 1});
  columnMap.addColumn ("Container", {0, 1});
  columnMap.addColumn ("AnalysisJets", {0, 10});
  columnMap.addColumn ("Container.linkIndex", {7});
  columnMap.addColumn ("Container.linkSGKey", {eleKey});
  columnMap.addColumn ("Container.outLink", {0});

  columnMap.connectColumnsToTool ();
  EXPECT_THROW (columnMap.call (), std::runtime_error);
}


// check we throw for an invalid index, as expected
TEST_F (ColumnarMemoryTest, ColumnarLinkTool_invalid_index_throws)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::ColumnarLinkTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty
    ("targetContainerNames", std::vector<std::string>{"AnalysisJets"}));
  ASSERT_SUCCESS (tool->initialize());

  const auto jetKey = columnar::knownSgKeys.at ("AnalysisJets");

  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize();

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 1});
  columnMap.addColumn ("Container", {0, 1});
  columnMap.addColumn ("AnalysisJets", {0, 10});
  columnMap.addColumn ("Container.linkIndex", {70});
  columnMap.addColumn ("Container.linkSGKey", {jetKey});
  columnMap.addColumn ("Container.outLink", {0});

  columnMap.connectColumnsToTool ();
  EXPECT_THROW (columnMap.call (), std::runtime_error);
}


// In-memory test that exercises depth-1 (vector of links per
// particle) input.  One event with three particles; particle 0 has
// two links, particle 1 has zero (empty inner vector edge case),
// particle 2 has three links.
TEST_F (ColumnarMemoryTest, ColumnarLinkTool_vector_of_links_depth1)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::ColumnarLinkTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("targetContainerNames", std::vector<std::string>{"AnalysisJets", "InDetTrackParticles"}));
  ASSERT_SUCCESS (tool->setProperty ("nestingDepth", 1u));
  ASSERT_SUCCESS (tool->initialize());

  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize();

  ColumnMapType columnMap {toolHandle};

  const auto jetKey = columnar::knownSgKeys.at ("AnalysisJets");
  const auto trkKey = columnar::knownSgKeys.at ("InDetTrackParticles");

  // one event, three particles
  columnMap.addColumn ("EventInfo", {0, 1});
  columnMap.addColumn ("Container", {0, 3});
  columnMap.addColumn ("AnalysisJets", {0, 10});
  columnMap.addColumn ("InDetTrackParticles", {0, 25});
  // per-particle inner-link counts: 2, 0, 3 -> total 5 flat links
  columnMap.addColumn ("Container.linkIndex.offset", {0, 2, 2, 5});
  columnMap.addColumn ("Container.linkIndex.data", {5, 7, 4, 9, 0});
  columnMap.addColumn ("Container.linkSGKey.data",
                       {jetKey, trkKey, jetKey, trkKey, SG::sgkey_t{0}});
  columnMap.addColumn ("Container.outLink.data", {0, 0, 0, 0, 0});

  using CM = columnar::ColumnarModeArray;
  columnMap.setExpectation ("Container.outLink.data",
                            {CM::mergeLinkKeyIndex (0, 5),
                             CM::mergeLinkKeyIndex (1, 7),
                             CM::mergeLinkKeyIndex (0, 4),
                             CM::mergeLinkKeyIndex (1, 9),
                             CM::LinkIndexType {CM::invalidLinkValue}});

  columnMap.connectColumnsToTool ();
  columnMap.call ();
  columnMap.checkExpectations ();
}


// In-memory test that exercises depth-2 (vector of vector of links
// per particle) input.  One event with two particles; the inner
// structure includes one empty inner vector.
TEST_F (ColumnarMemoryTest, ColumnarLinkTool_vector_of_vector_of_links_depth2)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<columnar::ColumnarLinkTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("targetContainerNames", std::vector<std::string>{"AnalysisJets", "InDetTrackParticles"}));
  ASSERT_SUCCESS (tool->setProperty ("nestingDepth", 2u));
  ASSERT_SUCCESS (tool->initialize());

  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize();

  ColumnMapType columnMap {toolHandle};

  const auto jetKey = columnar::knownSgKeys.at ("AnalysisJets");
  const auto trkKey = columnar::knownSgKeys.at ("InDetTrackParticles");

  // one event, two particles
  columnMap.addColumn ("EventInfo", {0, 1});
  columnMap.addColumn ("Container", {0, 2});
  columnMap.addColumn ("AnalysisJets", {0, 10});
  columnMap.addColumn ("InDetTrackParticles", {0, 25});
  // particle 0 has 2 inner vectors, particle 1 has 2 inner vectors
  columnMap.addColumn ("Container.linkIndex.outerOffset", {0, 2, 4});
  // inner vector sizes: 1, 0 (empty), 2, 2 -> total 5 flat links
  columnMap.addColumn ("Container.linkIndex.innerOffset", {0, 1, 1, 3, 5});
  columnMap.addColumn ("Container.linkIndex.data", {3, 6, 8, 0, 11});
  columnMap.addColumn ("Container.linkSGKey.data",
                       {jetKey, trkKey, jetKey, SG::sgkey_t{0}, trkKey});
  columnMap.addColumn ("Container.outLink.data", {0, 0, 0, 0, 0});

  using CM = columnar::ColumnarModeArray;
  columnMap.setExpectation ("Container.outLink.data",
                            {CM::mergeLinkKeyIndex (0, 3),
                             CM::mergeLinkKeyIndex (1, 6),
                             CM::mergeLinkKeyIndex (0, 8),
                             CM::LinkIndexType {CM::invalidLinkValue},
                             CM::mergeLinkKeyIndex (1, 11)});

  columnMap.connectColumnsToTool ();
  columnMap.call ();
  columnMap.checkExpectations ();
}


ATLAS_GOOGLE_TEST_MAIN
