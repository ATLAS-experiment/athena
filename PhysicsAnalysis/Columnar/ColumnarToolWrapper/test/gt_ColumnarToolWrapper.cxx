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
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/ParticleDef.h>
#include <ColumnarCore/SelectionColumn.h>
#include <ColumnarToolWrapper/ColumnVectorWrapper.h>
#include <ColumnarToolWrapper/ToolColumnVectorMap.h>

//
// method implementations
//

namespace columnar
{
  static_assert (std::is_same_v<ColumnarMode,ColumnarModeArray>, "this test is only defined for array mode");


  // ==========================================================================
  // Single Tool Tests
  // ==========================================================================

  TEST (ColumnVectorWrapperTest, SingleTool_BasicColumns)
  {
    ColumnarTool<> tool;
    ParticleAccessor<ObjectColumn> particlesHandle{tool, "particles"};  // Creates offset column
    ParticleAccessor<float> ptAcc{tool, "pt"};
    ParticleAccessor<float> etaAcc{tool, "eta"};

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
    ColumnarTool<> tool;
    ParticleAccessor<ObjectColumn> particlesHandle{tool, "Particles"};
    ParticleAccessor<float> ptAcc{tool, "pt"};

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
    ColumnarTool<> tool1;
    ParticleAccessor<ObjectColumn> particlesHandle1{tool1, "particles"};  // Creates offset column
    ParticleAccessor<float> pt1{tool1, "pt"};

    ColumnarTool<> tool2;
    ParticleAccessor<ObjectColumn> particlesHandle2{tool2, "particles"};  // Same offset column
    ParticleAccessor<float> pt2{tool2, "pt"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper1{header, tool1};
    ToolColumnVectorMap wrapper2{header, tool2};

    // Same column name should get same index (deduplication)
    EXPECT_EQ(wrapper1.getColumnIndex("particles.pt"), wrapper2.getColumnIndex("particles.pt"));
    EXPECT_EQ(wrapper1.getColumnIndex("particles"), wrapper2.getColumnIndex("particles"));

    // The header should only have the column once
    std::size_t numColumns = header.numColumns();

    // Create a third tool with a different column
    ColumnarTool<> tool3;
    ParticleAccessor<ObjectColumn> particlesHandle3{tool3, "particles"};  // Same offset column
    ParticleAccessor<float> eta3{tool3, "eta"};
    ToolColumnVectorMap wrapper3{header, tool3};

    // Should have added one new column (particles.eta)
    EXPECT_EQ(header.numColumns(), numColumns + 1);
  }


  TEST (ColumnVectorWrapperTest, MultiTool_AccessModePromotion)
  {
    // Tool1 has input column, Tool2 has update for same column -> promotes to read-write
    ColumnarTool<> tool1;
    ParticleAccessor<ObjectColumn> particlesHandle1{tool1, "particles"};
    ParticleAccessor<float> ptInput{tool1, "pt"};

    ColumnarTool<> tool2;
    ParticleAccessor<ObjectColumn> particlesHandle2{tool2, "particles"};
    AccessorTemplate<ParticleDef,float,ColumnAccessMode::update> ptUpdate{tool2, "pt"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper1{header, tool1};

    // After first tool, column should be read-only
    std::size_t ptIndex = wrapper1.getColumnIndex("particles.pt");
    EXPECT_TRUE(header.getColumn(ptIndex).readOnly);

    // After second tool with update access, column should be read-write
    ToolColumnVectorMap wrapper2{header, tool2};
    EXPECT_FALSE(header.getColumn(ptIndex).readOnly);
  }


  TEST (ColumnVectorWrapperTest, MultiTool_OptionalThenRequired)
  {
    // Tool1 has the column as optional, Tool2 requires it -> required
    ColumnarTool<> tool1;
    ParticleAccessor<ObjectColumn> particlesHandle1{tool1, "particles"};
    ParticleAccessor<float> ptOptional{tool1, "pt", {.isOptional = true}};

    ColumnarTool<> tool2;
    ParticleAccessor<ObjectColumn> particlesHandle2{tool2, "particles"};
    ParticleAccessor<float> ptRequired{tool2, "pt"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper1{header, tool1};

    // After first tool, column should be optional
    std::size_t ptIndex = wrapper1.getColumnIndex("particles.pt");
    EXPECT_TRUE(header.getColumn(ptIndex).isOptional);

    // After second tool requiring it, column should be required
    ToolColumnVectorMap wrapper2{header, tool2};
    EXPECT_FALSE(header.getColumn(ptIndex).isOptional);
  }


  TEST (ColumnVectorWrapperTest, MultiTool_RequiredThenOptional)
  {
    // Tool1 requires the column, Tool2 has it as optional -> stays required
    ColumnarTool<> tool1;
    ParticleAccessor<ObjectColumn> particlesHandle1{tool1, "particles"};
    ParticleAccessor<float> ptRequired{tool1, "pt"};

    ColumnarTool<> tool2;
    ParticleAccessor<ObjectColumn> particlesHandle2{tool2, "particles"};
    ParticleAccessor<float> ptOptional{tool2, "pt", {.isOptional = true}};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper1{header, tool1};
    std::size_t ptIndex = wrapper1.getColumnIndex("particles.pt");
    EXPECT_FALSE(header.getColumn(ptIndex).isOptional);

    ToolColumnVectorMap wrapper2{header, tool2};
    EXPECT_FALSE(header.getColumn(ptIndex).isOptional);
  }


  TEST (ColumnVectorWrapperTest, MultiTool_OutputConflict)
  {
    // Tool1 has input, Tool2 has output for same column -> should throw error
    ColumnarTool<> tool1;
    ParticleAccessor<ObjectColumn> particlesHandle1{tool1, "particles"};
    ParticleAccessor<float> ptInput{tool1, "pt"};

    ColumnarTool<> tool2;
    ParticleAccessor<ObjectColumn> particlesHandle2{tool2, "particles"};
    ParticleDecorator<float> ptOutput{tool2, "pt"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper1{header, tool1};

    // Second tool with output access should throw
    EXPECT_THROW(ToolColumnVectorMap(header, tool2), std::runtime_error);
  }


  TEST (ColumnVectorWrapperTest, MultiTool_DisjointColumns)
  {
    // Two tools with completely different columns -> all columns created
    ColumnarTool<> tool1;
    ParticleAccessor<ObjectColumn> particlesHandle1{tool1, "particles"};
    ParticleAccessor<float> pt1{tool1, "pt"};

    ColumnarTool<> tool2;
    ParticleAccessor<ObjectColumn> particlesHandle2{tool2, "particles"};  // Same offset
    ParticleAccessor<float> eta2{tool2, "eta"};

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
    ColumnarTool<> tool1;
    ParticleAccessor<ObjectColumn> particlesHandle1{tool1, "particles"};
    ParticleDecorator<float> ptOutput{tool1, "pt"};

    ColumnarTool<> tool2;
    ParticleAccessor<ObjectColumn> particlesHandle2{tool2, "particles"};
    ParticleAccessor<float> ptInput{tool2, "pt"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper1{header, tool1};

    std::size_t ptIndex = wrapper1.getColumnIndex("particles.pt");
    EXPECT_FALSE(header.getColumn(ptIndex).readOnly); // output is read-write

    // Second tool with input access should work
    ToolColumnVectorMap wrapper2{header, tool2};
    EXPECT_FALSE(header.getColumn(ptIndex).readOnly); // should stay read-write
  }


  // ==========================================================================
  // Column Type Tests
  // ==========================================================================

  TEST (ColumnVectorWrapperTest, Type_CharIsSignAgnostic)
  {
    ColumnarTool<> tool;
    ParticleAccessor<ObjectColumn> particlesHandle{tool, "particles"};
    ParticleAccessor<char> flagAcc{tool, "flag"};
    ParticleAccessor<char> flag2Acc{tool, "flag2"};
    ParticleAccessor<char> flag3Acc{tool, "flag3"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper{header, tool};
    EXPECT_EQ(header.getColumn(wrapper.getColumnIndex("particles.flag")).typeCategory, ColumnTypeCategory::signAgnosticInteger);
    EXPECT_EQ(header.getColumn(wrapper.getColumnIndex("particles.flag")).sizeOfType, 1u);

    ColumnVectorData data{&header};
    const std::vector<char> charData (3, 0);
    const std::vector<unsigned char> ucharData (3, 0);
    const std::vector<signed char> scharData (3, 0);
    const std::vector<std::uint16_t> shortData (3, 0);
    const std::vector<float> floatData (3, 0);
    const bool boolData[3] = {false, false, false};

    // wrong size, non-integers and C++ bool are rejected
    EXPECT_THROW(wrapper.setColumn(data, "particles.flag", 3, shortData.data()), std::runtime_error);
    EXPECT_THROW(wrapper.setColumn(data, "particles.flag", 3, floatData.data()), std::runtime_error);
    EXPECT_THROW(wrapper.setColumn(data, "particles.flag", 3, boolData), std::runtime_error);

    // all the one-byte integer types are accepted
    EXPECT_NO_THROW(wrapper.setColumn(data, "particles.flag", 3, charData.data()));
    EXPECT_NO_THROW(wrapper.setColumn(data, "particles.flag2", 3, ucharData.data()));
    EXPECT_NO_THROW(wrapper.setColumn(data, "particles.flag3", 3, scharData.data()));

    // and the column can be retrieved as any of them as well
    EXPECT_EQ(wrapper.getColumn<const unsigned char>(data, "particles.flag").second, reinterpret_cast<const unsigned char*>(charData.data()));
    EXPECT_THROW((void) wrapper.getColumn<const std::uint16_t>(data, "particles.flag"), std::runtime_error);
  }


  TEST (ColumnVectorWrapperTest, Type_IntegerStrictByDefault)
  {
    ColumnarTool<> tool;
    ParticleAccessor<ObjectColumn> particlesHandle{tool, "particles"};
    ParticleAccessor<std::uint32_t> strictAcc{tool, "strict"};
    ParticleAccessor<std::uint32_t> looseAcc{tool, "loose", {.isSignAgnostic = true}};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper{header, tool};
    EXPECT_EQ(header.getColumn(wrapper.getColumnIndex("particles.strict")).typeCategory, ColumnTypeCategory::unsignedInteger);
    EXPECT_EQ(header.getColumn(wrapper.getColumnIndex("particles.loose")).typeCategory, ColumnTypeCategory::signAgnosticInteger);
    EXPECT_EQ(header.getColumn(wrapper.getColumnIndex("particles.loose")).sizeOfType, 4u);

    ColumnVectorData data{&header};
    const std::vector<std::int32_t> signedData (3, 0);
    const std::vector<std::uint32_t> unsignedData (3, 0);
    const std::vector<std::int64_t> longData (3, 0);
    const std::vector<float> floatData (3, 0);

    EXPECT_THROW(wrapper.setColumn(data, "particles.strict", 3, signedData.data()), std::runtime_error);
    EXPECT_NO_THROW(wrapper.setColumn(data, "particles.strict", 3, unsignedData.data()));

    EXPECT_THROW(wrapper.setColumn(data, "particles.loose", 3, longData.data()), std::runtime_error);
    EXPECT_THROW(wrapper.setColumn(data, "particles.loose", 3, floatData.data()), std::runtime_error);
    EXPECT_NO_THROW(wrapper.setColumn(data, "particles.loose", 3, signedData.data()));
  }


  TEST (ColumnVectorWrapperTest, Type_OffsetIsStrict)
  {
    ColumnarTool<> tool;
    ParticleAccessor<ObjectColumn> particlesHandle{tool, "particles"};
    ParticleAccessor<float> ptAcc{tool, "pt"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper{header, tool};
    EXPECT_EQ(header.getColumn(wrapper.getColumnIndex("particles")).typeCategory, ColumnTypeCategory::unsignedInteger);

    ColumnVectorData data{&header};
    const std::vector<std::make_signed_t<ColumnarOffsetType>> signedOffsets {0, 3};
    const std::vector<ColumnarOffsetType> offsets {0, 3};
    EXPECT_THROW(wrapper.setColumn(data, "particles", 2, signedOffsets.data()), std::runtime_error);
    EXPECT_NO_THROW(wrapper.setColumn(data, "particles", 2, offsets.data()));
  }


  TEST (ColumnVectorWrapperTest, Type_SignAgnosticNeedsInteger)
  {
    ColumnarTool<> tool;
    ParticleAccessor<ObjectColumn> particlesHandle{tool, "particles"};
    EXPECT_THROW((ParticleAccessor<float>{tool, "pt", {.isSignAgnostic = true}}), std::logic_error);
  }


  TEST (ColumnVectorWrapperTest, Type_SelectionIsBoolean)
  {
    ColumnarTool<> tool;
    ParticleAccessor<ObjectColumn> particlesHandle{tool, "particles"};
    ParticleAccessor<SelectionColumn> preselectionAcc{tool, "preselection"};
    ParticleDecorator<SelectionColumn> selectionDec{tool, "selection"};

    // a plain char accessor on the same column makes no promise on
    // the values, so that column is no longer boolean
    ParticleAccessor<SelectionColumn> mixedAcc{tool, "mixed"};
    ParticleAccessor<char> mixedCharAcc{tool, "mixed"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper{header, tool};
    for (const char *name : {"particles.preselection", "particles.selection"})
    {
      const auto& column = header.getColumn(wrapper.getColumnIndex(name));
      EXPECT_EQ(column.typeCategory, ColumnTypeCategory::boolean) << name;
      EXPECT_EQ(column.sizeOfType, 1u) << name;
    }
    const auto& mixed = header.getColumn(wrapper.getColumnIndex("particles.mixed"));
    EXPECT_EQ(mixed.typeCategory, ColumnTypeCategory::signAgnosticInteger);
    EXPECT_EQ(mixed.sizeOfType, 1u);
  }


  TEST (ColumnVectorWrapperTest, MultiTool_StrictTypeWins)
  {
    // if one tool wants the exact type the shared column wants it
    ColumnarTool<> tool1;
    ParticleAccessor<ObjectColumn> particlesHandle1{tool1, "particles"};
    ParticleAccessor<std::uint32_t> loose1{tool1, "value", {.isSignAgnostic = true}};
    ParticleAccessor<SelectionColumn> selection1{tool1, "flag"};

    ColumnarTool<> tool2;
    ParticleAccessor<ObjectColumn> particlesHandle2{tool2, "particles"};
    ParticleAccessor<std::uint32_t> strict2{tool2, "value"};
    ParticleAccessor<char> flag2{tool2, "flag"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper1{header, tool1};
    const std::size_t valueIndex = wrapper1.getColumnIndex("particles.value");
    const std::size_t flagIndex = wrapper1.getColumnIndex("particles.flag");
    EXPECT_EQ(header.getColumn(valueIndex).typeCategory, ColumnTypeCategory::signAgnosticInteger);
    EXPECT_EQ(header.getColumn(flagIndex).typeCategory, ColumnTypeCategory::boolean);

    ToolColumnVectorMap wrapper2{header, tool2};
    EXPECT_EQ(header.getColumn(valueIndex).typeCategory, ColumnTypeCategory::unsignedInteger);
    EXPECT_EQ(header.getColumn(flagIndex).typeCategory, ColumnTypeCategory::signAgnosticInteger);
  }

  TEST (ColumnVectorWrapperTest, Type_SameRepresentationAccepted)
  {
    // types with a different typeid but the same category and size
    // (e.g. `unsigned long` and `unsigned long long`) are accepted
    ColumnarTool<> tool;
    ParticleAccessor<ObjectColumn> particlesHandle{tool, "particles"};
    ParticleAccessor<long> longAcc{tool, "value"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper{header, tool};
    EXPECT_EQ(header.getColumn(wrapper.getColumnIndex("particles.value")).typeCategory, ColumnTypeCategory::signedInteger);

    ColumnVectorData data{&header};
    static_assert (sizeof (unsigned long long) == sizeof (ColumnarOffsetType));
    static_assert (sizeof (long long) == sizeof (long));
    const std::vector<unsigned long long> offsets {0, 3};
    const std::vector<long long> values (3, 0);
    const std::vector<unsigned long> unsignedValues (3, 0);
    EXPECT_NO_THROW(wrapper.setColumn(data, "particles", 2, offsets.data()));
    EXPECT_THROW(wrapper.setColumn(data, "particles.value", 3, unsignedValues.data()), std::runtime_error);
    EXPECT_NO_THROW(wrapper.setColumn(data, "particles.value", 3, values.data()));
  }


  TEST (ColumnVectorWrapperTest, Type_VoidWithoutType)
  {
    // the void interface can be used with only a category and size,
    // e.g. for data coming from python
    ColumnarTool<> tool;
    ParticleAccessor<ObjectColumn> particlesHandle{tool, "particles"};
    ParticleAccessor<float> ptAcc{tool, "pt"};
    ParticleAccessor<char> flagAcc{tool, "flag"};
    ParticleDecorator<char> outFlagDec{tool, "outFlag"};
    ParticleDecorator<SelectionColumn> selectionDec{tool, "selection"};

    ColumnVectorHeader header;
    ToolColumnVectorMap wrapper{header, tool};
    ColumnVectorData data{&header};
    const std::vector<std::uint8_t> bytes (3, 0);
    std::vector<std::uint8_t> outBytes (3, 0);
    std::vector<std::uint8_t> selection (3, 0);
    const std::vector<float> floats (3, 0);

    // float data needs a float column of the right size
    EXPECT_THROW(wrapper.setColumnVoid(data, "particles.pt", 3, floats.data(), nullptr, ColumnTypeCategory::floatingPoint, 8u, true), std::runtime_error);
    EXPECT_THROW(wrapper.setColumnVoid(data, "particles.pt", 3, floats.data(), nullptr, ColumnTypeCategory::signedInteger, 4u, true), std::runtime_error);
    EXPECT_NO_THROW(wrapper.setColumnVoid(data, "particles.pt", 3, floats.data(), nullptr, ColumnTypeCategory::floatingPoint, 4u, true));

    // boolean data can be read from any integer column, but only
    // written to boolean columns
    EXPECT_NO_THROW(wrapper.setColumnVoid(data, "particles.flag", 3, bytes.data(), nullptr, ColumnTypeCategory::boolean, 1u, true));
    EXPECT_THROW(wrapper.setColumnVoid(data, "particles.outFlag", 3, outBytes.data(), nullptr, ColumnTypeCategory::boolean, 1u, false), std::runtime_error);
    EXPECT_NO_THROW(wrapper.setColumnVoid(data, "particles.outFlag", 3, outBytes.data(), nullptr, ColumnTypeCategory::unsignedInteger, 1u, false));
    EXPECT_NO_THROW(wrapper.setColumnVoid(data, "particles.selection", 3, selection.data(), nullptr, ColumnTypeCategory::boolean, 1u, false));
  }
}

ATLAS_GOOGLE_TEST_MAIN
