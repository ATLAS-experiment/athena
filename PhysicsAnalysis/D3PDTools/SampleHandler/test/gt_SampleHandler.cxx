/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/Global.h>

#include <RootCoreUtils/Assert.h>
#include <SampleHandler/DiskListLocal.h>
#include <SampleHandler/SampleHandler.h>
#include <SampleHandler/SampleMeta.h>
#include <SampleHandler/ScanDir.h>
#include <SampleHandler/ToolsDiscovery.h>
#include <TSystem.h>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <gtest/gtest.h>

//
// main program
//

using namespace SH;

TEST (SampleHandlerTest, add_single)
{
  SampleHandler sh;
  ASSERT_TRUE (sh.get ("sample") == nullptr);
  sh.add (std::make_unique<SampleMeta> ("sample"));
  ASSERT_TRUE (sh.get ("sample") != nullptr);
}

TEST (SampleHandlerTest, add_duplicate)
{
  SampleHandler sh;
  sh.add (std::make_unique<SampleMeta> ("sample"));
  ASSERT_ANY_THROW (sh.add (std::make_unique<SampleMeta> ("sample")));
}

TEST (SampleHandlerTest, addWithPrefix_single)
{
  SampleHandler target, source;
  source.add (std::make_unique<SampleMeta> ("sample"));
  target.addWithPrefix (source, "prefix_");
  ASSERT_TRUE (target.get ("prefix_sample") != nullptr);
}

TEST (SampleHandlerTest, addWithPrefix_duplicate)
{
  SampleHandler target, source;
  source.add (std::make_unique<SampleMeta> ("sample"));
  target.add (std::make_unique<SampleMeta> ("prefix_sample"));
  EXPECT_ANY_THROW (target.addWithPrefix (source, "prefix_"));
}

TEST (SampleHandlerTest, name_locked_after_add)
{
  SampleHandler sh;
  auto sample = std::make_shared<SampleMeta> ("sample");
  sh.add (sample);
  ASSERT_THROW (sample->name ("renamed"), std::logic_error);
  ASSERT_TRUE (sh.get ("sample") != nullptr);
  ASSERT_TRUE (sh.get ("renamed") == nullptr);
}

int main (int argc, char **argv)
{
  ::testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS();
}
