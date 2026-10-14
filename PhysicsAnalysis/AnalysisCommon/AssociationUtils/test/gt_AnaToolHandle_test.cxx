/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/** Unit tests for AssociationUtils tools in StandaloneToolHandle
 */

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "AsgTesting/UnitTest.h"
#include "AsgMessaging/MessageCheck.h"
#include "gtest/gtest.h"
#ifdef ROOTCORE
#include "xAODRootAccess/Init.h"
#endif

#include "AsgTools/StandaloneToolHandle.h"

#include "AssociationUtils/IOverlapTool.h"
#include "AssociationUtils/IOverlapRemovalTool.h"

namespace
{
  using namespace ADD_CMODE(ORUtils);
  using namespace asg::msgUserCode;

  TEST(StandaloneToolHandleTest, OverlapTools)
  {
    auto toolStrings = {
      columnar::add_cmode("ORUtils")+"::DeltaROverlapTool/DrORT1",
      columnar::add_cmode("ORUtils")+"::ObjLinkOverlapTool/LinkORT1",
      columnar::add_cmode("ORUtils")+"::EleEleOverlapTool/EleEleORT1",
      columnar::add_cmode("ORUtils")+"::EleJetOverlapTool/EleJetORT1",
      columnar::add_cmode("ORUtils")+"::EleMuSharedTrkOverlapTool/EleMuORT1",
      columnar::add_cmode("ORUtils")+"::MuJetOverlapTool/MuJetORT1",
      columnar::add_cmode("ORUtils")+"::AltMuJetOverlapTool/AltMuJetORT1",
      columnar::add_cmode("ORUtils")+"::TauLooseEleOverlapTool/TauEleORT1",
      columnar::add_cmode("ORUtils")+"::TauLooseMuOverlapTool/TauMuORT1",
      columnar::add_cmode("ORUtils")+"::TauJetOverlapTool/TauJetORT1",
      columnar::add_cmode("ORUtils")+"::TauAntiTauJetOverlapTool/AntiTauJetORT1",
    };

    // Try to create and initialize each tool with the StandaloneToolHandle
    for(const auto& toolStr : toolStrings) {
      asg::StandaloneToolHandle<IOverlapTool> handle(toolStr);
      EXPECT_SUCCESS( handle.initialize() );
    }
  }

  TEST(StandaloneToolHandleTest, MasterTools)
  {
    auto toolStrings = {
      columnar::add_cmode("ORUtils")+"::OverlapRemovalTool/ORTool1",
    };
    // Try to create and initialize each tool with the StandaloneToolHandle
    for(const auto& toolStr : toolStrings) {
      asg::StandaloneToolHandle<IOverlapRemovalTool> handle(toolStr);
      EXPECT_SUCCESS( handle.initialize() );
    }
  }

  // Try to assign an overlap tool to a master tool with StandaloneToolHandle
  TEST(StandaloneToolHandleTest, ToolAssignment)
  {
    // Specify the tool types and names
    const auto masterToolName = columnar::add_cmode("ORUtils")+"::OverlapRemovalTool/ORTool2";
    const auto overlapToolType = columnar::add_cmode("ORUtils")+"::DeltaROverlapTool";
    const auto key = "EleJetORT";
    // Create the StandaloneToolHandles
    asg::StandaloneToolHandle<IOverlapRemovalTool> masterHandle(masterToolName);
    // Hook them up and initialize
    ASSERT_SUCCESS( masterHandle.createPrivateTool(key, overlapToolType) );
    ASSERT_SUCCESS( masterHandle.initialize() );
    ASSERT_TRUE( masterHandle.get() != nullptr );
  }

  // Try to assign an overlap tool to a master tool with StandaloneToolHandle.
  // Same as above except that we initialize the overlap handle differently.
  // This test is designed to mimic what happens in the ToolBox initialization.
  TEST(StandaloneToolHandleTest, ToolAssignment2)
  {
    // Specify the tool types and names
    const auto masterToolName = columnar::add_cmode("ORUtils")+"::OverlapRemovalTool/ORTool3";
    const auto overlapToolType = columnar::add_cmode("ORUtils")+"::DeltaROverlapTool";
    const auto key = "EleJetORT";
    // Create the StandaloneToolHandles
    asg::StandaloneToolHandle<IOverlapRemovalTool> masterHandle(masterToolName);
    // Hook up the sub-tool
    ASSERT_SUCCESS( masterHandle.createPrivateTool(key, overlapToolType) );
    ASSERT_SUCCESS( masterHandle.initialize() );
    ASSERT_TRUE( masterHandle.get() != nullptr );
  }

}

int main(int argc, char** argv)
{
#ifdef ROOTCORE
  StatusCode::enableFailure();
  ANA_CHECK( xAOD::Init() );
#endif
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
