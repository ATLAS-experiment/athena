/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <AsgTesting/UnitTest.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/ParticleDef.h>
#include <ColumnarToolWrapper/ColumnVectorWrapper.h>
#include <ColumnarToolWrapper/ToolColumnVectorMap.h>

//
// method implementations
//

namespace columnar
{
  // Type aliases for ColumnarModeArray (hardcoded for this test)
  using MyTool = ColumnarTool<ColumnarModeArray>;
  template<typename CT> using MyAccessor = AccessorTemplate<ContainerId::particle,CT,ColumnAccessMode::input,ColumnarModeArray>;
  template<typename CT> using MyDecorator = AccessorTemplate<ContainerId::particle,CT,ColumnAccessMode::output,ColumnarModeArray>;
  template<typename CT> using MyUpdater = AccessorTemplate<ContainerId::particle,CT,ColumnAccessMode::update,ColumnarModeArray>;


  // ==========================================================================
  // Single Tool Tests
  // ==========================================================================

  TEST (ColumnVectorWrapperTest, SingleTool_BasicColumns)
  {
    MyTool tool;
    MyAccessor<ObjectColumn> particlesHandle{tool, "particles"};  // Creates offset column
    MyAccessor<float> ptAcc{tool, "pt"};
    MyAccessor<float> etaAcc{tool, "eta"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper{header, tool};

    // Check that columns were registered:
    // - null column (index 0)
    // - size column (index 1)
    // - numberOfEvents offset (from tool default)
    // - particles offset column
    // - particles.pt
    // - particles.eta
    EXPECT_GE(header.numColumns(), 5u);

    // Verify we can retrieve the column indices
    auto columnNames = wrapper.getColumnNames();
    EXPECT_TRUE(std::find(columnNames.begin(), columnNames.end(), "particles") != columnNames.end());
    EXPECT_TRUE(std::find(columnNames.begin(), columnNames.end(), "particles.pt") != columnNames.end());
    EXPECT_TRUE(std::find(columnNames.begin(), columnNames.end(), "particles.eta") != columnNames.end());
  }


  TEST (ColumnVectorWrapperTest, SingleTool_OffsetColumns)
  {
    MyTool tool;
    MyAccessor<ObjectColumn> particlesHandle{tool, "Particles"};
    MyAccessor<float> ptAcc{tool, "pt"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper{header, tool};

    // Verify header passes self-check (validates offset relationships)
    header.checkSelf();

    // Verify the offset column was created
    auto columnNames = wrapper.getColumnNames();
    EXPECT_TRUE(std::find(columnNames.begin(), columnNames.end(), "Particles") != columnNames.end());
    EXPECT_TRUE(std::find(columnNames.begin(), columnNames.end(), "Particles.pt") != columnNames.end());
  }


  // ==========================================================================
  // Multi-Tool Tests
  // ==========================================================================

  TEST (ColumnVectorWrapperTest, MultiTool_SharedColumn)
  {
    // Two tools with the same column name should get the same index
    MyTool tool1;
    MyAccessor<ObjectColumn> particlesHandle1{tool1, "particles"};  // Creates offset column
    MyAccessor<float> pt1{tool1, "pt"};

    MyTool tool2;
    MyAccessor<ObjectColumn> particlesHandle2{tool2, "particles"};  // Same offset column
    MyAccessor<float> pt2{tool2, "pt"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper1{header, tool1};
    ToolColumnVectorMap wrapper2{header, tool2};

    // Same column name should get same index (deduplication)
    EXPECT_EQ(wrapper1.getColumnIndex("particles.pt"), wrapper2.getColumnIndex("particles.pt"));
    EXPECT_EQ(wrapper1.getColumnIndex("particles"), wrapper2.getColumnIndex("particles"));

    // The header should only have the column once
    std::size_t numColumns = header.numColumns();

    // Create a third tool with a different column
    MyTool tool3;
    MyAccessor<ObjectColumn> particlesHandle3{tool3, "particles"};  // Same offset column
    MyAccessor<float> eta3{tool3, "eta"};
    ToolColumnVectorMap wrapper3{header, tool3};

    // Should have added one new column (particles.eta)
    EXPECT_EQ(header.numColumns(), numColumns + 1);
  }


  TEST (ColumnVectorWrapperTest, MultiTool_AccessModePromotion)
  {
    // Tool1 has input column, Tool2 has update for same column -> promotes to read-write
    MyTool tool1;
    MyAccessor<ObjectColumn> particlesHandle1{tool1, "particles"};
    MyAccessor<float> ptInput{tool1, "pt"};

    MyTool tool2;
    MyAccessor<ObjectColumn> particlesHandle2{tool2, "particles"};
    MyUpdater<float> ptUpdate{tool2, "pt"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper1{header, tool1};

    // After first tool, column should be read-only
    std::size_t ptIndex = wrapper1.getColumnIndex("particles.pt");
    EXPECT_TRUE(header.getColumn(ptIndex).readOnly);

    // After second tool with update access, column should be read-write
    ToolColumnVectorMap wrapper2{header, tool2};
    EXPECT_FALSE(header.getColumn(ptIndex).readOnly);
  }


  TEST (ColumnVectorWrapperTest, MultiTool_OutputConflict)
  {
    // Tool1 has input, Tool2 has output for same column -> should throw error
    MyTool tool1;
    MyAccessor<ObjectColumn> particlesHandle1{tool1, "particles"};
    MyAccessor<float> ptInput{tool1, "pt"};

    MyTool tool2;
    MyAccessor<ObjectColumn> particlesHandle2{tool2, "particles"};
    MyDecorator<float> ptOutput{tool2, "pt"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper1{header, tool1};

    // Second tool with output access should throw
    EXPECT_THROW(ToolColumnVectorMap(header, tool2), std::runtime_error);
  }


  TEST (ColumnVectorWrapperTest, MultiTool_DisjointColumns)
  {
    // Two tools with completely different columns -> all columns created
    MyTool tool1;
    MyAccessor<ObjectColumn> particlesHandle1{tool1, "particles"};
    MyAccessor<float> pt1{tool1, "pt"};

    MyTool tool2;
    MyAccessor<ObjectColumn> particlesHandle2{tool2, "particles"};  // Same offset
    MyAccessor<float> eta2{tool2, "eta"};

    ColumnVectorHeader header;
    std::size_t initialColumns = header.numColumns();

    ToolColumnVectorMap wrapper1{header, tool1};
    std::size_t afterTool1 = header.numColumns();
    EXPECT_GT(afterTool1, initialColumns);

    ToolColumnVectorMap wrapper2{header, tool2};
    std::size_t afterTool2 = header.numColumns();
    // Should have added particles.eta (but not particles again due to deduplication)
    EXPECT_GT(afterTool2, afterTool1);

    // Both columns should exist
    auto names1 = wrapper1.getColumnNames();
    auto names2 = wrapper2.getColumnNames();
    EXPECT_TRUE(std::find(names1.begin(), names1.end(), "particles.pt") != names1.end());
    EXPECT_TRUE(std::find(names2.begin(), names2.end(), "particles.eta") != names2.end());
  }


  TEST (ColumnVectorWrapperTest, MultiTool_OutputThenInput)
  {
    // Tool1 has output, Tool2 has input for same column -> should work (output stays output)
    MyTool tool1;
    MyAccessor<ObjectColumn> particlesHandle1{tool1, "particles"};
    MyDecorator<float> ptOutput{tool1, "pt"};

    MyTool tool2;
    MyAccessor<ObjectColumn> particlesHandle2{tool2, "particles"};
    MyAccessor<float> ptInput{tool2, "pt"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper1{header, tool1};

    std::size_t ptIndex = wrapper1.getColumnIndex("particles.pt");
    EXPECT_FALSE(header.getColumn(ptIndex).readOnly); // output is read-write

    // Second tool with input access should work
    ToolColumnVectorMap wrapper2{header, tool2};
    EXPECT_FALSE(header.getColumn(ptIndex).readOnly); // should stay read-write
  }
}

ATLAS_GOOGLE_TEST_MAIN
