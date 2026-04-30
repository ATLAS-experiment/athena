/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <AsgTesting/UnitTest.h>
#include <AsgTools/AsgToolConfig.h>
#include <AsgTools/AsgTool.h>
#include <ColumnarInterfaces/IColumnarTool.h>
#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarToolWrapper/ToolColumnVectorMap.h>
#include <ColumnarToolWrapper/ColumnarToolHelpers.h>

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

  TEST (RunColumnarToolTest, baseUse)
  {
    // create the tool via the component library
    asg::AsgToolConfig config;
    config.setTypeAndName("columnar::SimpleSelectorExampleTool/" + makeUniqueName());
    ToolHandle<asg::AsgTool> toolHandle;
    std::shared_ptr<void> cleanup;
    ASSERT_SUCCESS(config.makeTool(toolHandle, cleanup));

    // cast to `IColumnarTool`. since `IColumnarTool` doesn't inherit
    // from `IAsgTool` we can't use it as a template parameter to
    // `ToolHandle`, but have to cast it afterwards.
    auto tool = dynamic_cast<IColumnarTool*> (&*toolHandle);
    ASSERT_NE(tool, nullptr);

    // a header that tracks the columns for all involved tools
    ColumnVectorHeader header;

    // a map of all columns used by a single tool
    ToolColumnVectorMap wrapper{header, *tool};


    // Below everything will be repeated for each batch of events. the
    // loop is just for illustrative purposes, in a real application
    // there would be many batches of events, but here we are just doing
    // one.
    for (std::size_t i = 0; i < 1; ++i)
    {
      // This contains pointers to all the columns for this batch of
      // events. Those are set below, and can then be passed into the
      // tool. For this test I simply look up the column index by name
      // on each call. In a real application you may just want to cache
      // the index instead of the name at the place where you read the
      // input data.
      ColumnVectorData columns {&header};

      // The offset map for the particles column, this is used to find
      // the object range for each event. It contains the index of the
      // first object in each event, followed by the total number of
      // objects at the end.
      std::vector<ColumnarOffsetType> offsets;
      offsets.push_back (0);
      offsets.push_back (1);
      columns.setColumn (header.getColumnIndex ("Particles"), offsets.size(), offsets.data());

      // The pt column for all particles for all events in this batch.
      std::vector<float> pt;
      pt.push_back (10e5);
      columns.setColumn (header.getColumnIndex ("Particles.pt"), pt.size(), pt.data());

      // The output selection column for all particles for all events in
      // this batch, this will be filled by the tool. We use a `char`
      // here to follow ATLAS conventions for selection columns, but in
      // the future this could also be a series of bits instead.
      std::vector<char> output;
      output.resize (pt.size(), 0.);
      columns.setColumn (header.getColumnIndex ("Particles.selection"), output.size(), output.data());

      // The event range column for this batch. This is always length
      // two: the index of the first event in the batch, followed by the
      // total number of events (i.e. 1 past the index of the last
      // event).
      std::vector<ColumnarOffsetType> numEvents;
      numEvents.push_back (0);
      numEvents.push_back (offsets.size()-1);
      columns.setColumn (header.getColumnIndex (eventRangeColumnName), numEvents.size(), numEvents.data());


      // Validate the input data. This will check that all required
      // columns are present, and that the sizes of the columns match
      // what is expected based on their offset maps.
      columns.checkData();

      // Call the tool with the assembled data. This will not perform
      // any checks on the data, as that has been done by the call
      // above.
      columns.callNoCheck (*tool);


      // check the output based on what we expect given the input
      EXPECT_FLOAT_EQ (1, output[0]);
    }
  }
}

ATLAS_GOOGLE_TEST_MAIN
