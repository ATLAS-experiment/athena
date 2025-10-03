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
#include <ColumnarCore/VectorColumn.h>
#include <ColumnarCore/VectorVectorColumn.h>
#include <ColumnarEventInfo/EventInfoDef.h>
#include <ColumnarCore/ParticleDef.h>

//
// method implementations
//

namespace columnar
{
  using MyTool = ColumnarTool<ColumnarModeArray>;
  template<typename CT,ContainerIdConcept CI=ContainerId::particle> using MyAccessor = AccessorTemplate<CI,CT,ColumnAccessMode::input,ColumnarModeArray>;
  template<typename CT,ContainerIdConcept CI=ContainerId::particle> using MyDecorator = AccessorTemplate<CI,CT,ColumnAccessMode::output,ColumnarModeArray>;
  template<ContainerIdConcept CI=ContainerId::particle> using MyId = ObjectId<CI,ColumnarModeArray>;
  template<ContainerIdConcept CI=ContainerId::particle> using MyRange = ObjectRange<CI,ColumnarModeArray>;


  TEST (AccessorTest, defaultEventOffsets)
  {
    MyTool tool;
    {
      auto columns = tool.getColumnInfo();
      EXPECT_EQ (columns.size(), 1);
      auto& column = columns[0];
      EXPECT_EQ (column.name, numberOfEventsName);
      EXPECT_EQ (column.index, 0);
    }
    tool.setColumnIndex (numberOfEventsName, 1);
    {
      auto columns = tool.getColumnInfo();
      EXPECT_EQ (columns.size(), 1);
      auto& column = columns[0];
      EXPECT_EQ (column.name, numberOfEventsName);
      EXPECT_EQ (column.index, 1);
    }
  }


  TEST (AccessorTest, nativeEventAccessor)
  {
    MyTool tool;
    MyAccessor<uint32_t,ContainerId::eventInfo> eventAccessor {tool, "var1"};
    {
      auto columns = tool.getColumnInfo();
      EXPECT_EQ (columns.size(), 2);
      auto& column = columns[1];
      EXPECT_EQ (column.name, "EventInfo.var1");
      EXPECT_EQ (column.index, 0);
      EXPECT_EQ (column.type, &typeid (uint32_t));
      EXPECT_EQ (column.accessMode, ColumnAccessMode::input);
      EXPECT_EQ (column.offsetName, numberOfEventsName);
    }
    tool.setColumnIndex ("EventInfo.var1", 1);
    std::vector<void*> data (2, nullptr);
    std::vector<uint32_t> var1 = {0, 1, 2, 3, 4, 5};
    data[1] = var1.data();
    MyId<ContainerId::eventInfo> id1 {data.data(), 1};
    MyId<ContainerId::eventInfo> id2 {data.data(), 2};
    EXPECT_EQ (eventAccessor (id1), 1);
    EXPECT_EQ (eventAccessor (id2), 2);
    EXPECT_EQ (&eventAccessor(id1),&id1(eventAccessor));
    MyRange<ContainerId::eventInfo> range {data.data(), 1, 3};
    auto rangeView = eventAccessor (range);
    EXPECT_EQ (rangeView.size(), 2);
    EXPECT_EQ (rangeView.data(), var1.data() + 1);
    EXPECT_EQ (range(eventAccessor).data(), rangeView.data());
  }


  TEST (AccessorTest, objectAccessor)
  {
    MyTool tool;
    MyAccessor<ObjectColumn> objectAccessor {tool, "particles"};
    MyAccessor<uint32_t> varAccessor {tool, "var1"};
    {
      auto columns = tool.getColumnInfo();
      EXPECT_EQ (columns.size(), 3);
      {
        auto& column = columns[1];
        EXPECT_EQ (column.name, "particles");
        EXPECT_EQ (column.index, 0);
        EXPECT_EQ (column.type, &typeid (ColumnarOffsetType));
        EXPECT_EQ (column.accessMode, ColumnAccessMode::input);
        EXPECT_EQ (column.offsetName, numberOfEventsName);
      }
      {
        auto& column = columns[2];
        EXPECT_EQ (column.name, "particles.var1");
        EXPECT_EQ (column.index, 0);
        EXPECT_EQ (column.type, &typeid (uint32_t));
        EXPECT_EQ (column.accessMode, ColumnAccessMode::input);
        EXPECT_EQ (column.offsetName, "particles");
      }
    }
    tool.setColumnIndex ("particles", 1);
    tool.setColumnIndex ("particles.var1", 2);
    std::vector<void*> data (3, nullptr);
    std::vector<ColumnarOffsetType> particles = {0, 1, 3, 6};
    data[1] = particles.data();
    std::vector<uint32_t> var1 = {0, 1, 2, 3, 4, 5};
    data[2] = var1.data();

    {
      MyId<ContainerId::eventContext> eventId {data.data(), 1};
      auto objectRange = objectAccessor (eventId);
      EXPECT_EQ (objectRange.size(), 2);
      EXPECT_EQ (objectRange.beginIndex(), 1);
      EXPECT_EQ (objectRange.endIndex(), 3);
    }

    {
      MyRange<ContainerId::eventContext> eventRange {data.data(), 1, 3};
      auto objectRange = objectAccessor (eventRange);
      EXPECT_EQ (objectRange.size(), 5);
      EXPECT_EQ (objectRange.beginIndex(), 1);
      EXPECT_EQ (objectRange.endIndex(), 6);
    }
  }


  TEST (AccessorTest, vectorEventAccessor)
  {
    MyTool tool;
    MyAccessor<std::vector<uint32_t>,ContainerId::eventInfo> eventAccessor {tool, "var1"};
    MyAccessor<std::vector<RetypeColumn<uint64_t,uint32_t>>,ContainerId::eventInfo> eventRetypeAccessor {tool, "var1"};
    {
      auto columns = tool.getColumnInfo();
      EXPECT_EQ (columns.size(), 3);
      auto& columnOffset = columns[1];
      EXPECT_EQ (columnOffset.name, "EventInfo.var1.data");
      EXPECT_EQ (columnOffset.index, 0);
      EXPECT_EQ (columnOffset.type, &typeid (uint32_t));
      EXPECT_EQ (columnOffset.accessMode, ColumnAccessMode::input);
      EXPECT_EQ (columnOffset.offsetName, "EventInfo.var1.offset");
      auto& columnData = columns[2];
      EXPECT_EQ (columnData.name, "EventInfo.var1.offset");
      EXPECT_EQ (columnData.index, 0);
      EXPECT_EQ (columnData.type, &typeid (ColumnarOffsetType));
      EXPECT_EQ (columnData.accessMode, ColumnAccessMode::input);
      EXPECT_EQ (columnData.offsetName, numberOfEventsName);
    }
    tool.setColumnIndex ("EventInfo.var1.offset", 1);
    tool.setColumnIndex ("EventInfo.var1.data", 2);
    std::vector<void*> data (3, nullptr);
    std::vector<ColumnarOffsetType> var1Offsets = {0, 1, 3, 6, 7};
    std::vector<uint32_t> var1Data = {0, 1, 2, 3, 4, 5, 6};
    data[1] = var1Offsets.data();
    data[2] = var1Data.data();
    MyId<ContainerId::eventInfo> id1 {data.data(), 1};
    MyId<ContainerId::eventInfo> id2 {data.data(), 2};
    EXPECT_EQ (eventAccessor (id1).size(), 2);
    EXPECT_EQ (eventAccessor (id2).size(), 3);
    EXPECT_EQ (eventAccessor(id1)[0],1);
    EXPECT_EQ (eventAccessor(id1)[1],2);
    EXPECT_EQ (eventAccessor(id2)[0],3);
    EXPECT_EQ (eventAccessor(id2)[2],5);

    EXPECT_EQ (eventRetypeAccessor (id1).size(), 2);
    EXPECT_EQ (eventRetypeAccessor (id2).size(), 3);
    EXPECT_EQ (eventRetypeAccessor(id1)[0],1);
    EXPECT_EQ (eventRetypeAccessor(id1)[1],2);
    EXPECT_EQ (eventRetypeAccessor(id2)[0],3);
    EXPECT_EQ (eventRetypeAccessor(id2)[2],5);
  }


  TEST (AccessorTest, vectorVectorEventAccessor)
  {
    MyTool tool;
    MyAccessor<std::vector<std::vector<uint32_t>>,ContainerId::eventInfo> eventAccessor {tool, "var1"};
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 4);
      EXPECT_EQ (columns[1].name, "EventInfo.var1.data");
      EXPECT_EQ (columns[1].index, 0);
      EXPECT_EQ (columns[1].type, &typeid (uint32_t));
      EXPECT_EQ (columns[1].accessMode, ColumnAccessMode::input);
      EXPECT_EQ (columns[1].offsetName, "EventInfo.var1.innerOffset");
      EXPECT_EQ (columns[2].name, "EventInfo.var1.innerOffset");
      EXPECT_EQ (columns[2].index, 0);
      EXPECT_EQ (columns[2].type, &typeid (ColumnarOffsetType));
      EXPECT_EQ (columns[2].accessMode, ColumnAccessMode::input);
      EXPECT_EQ (columns[2].offsetName, "EventInfo.var1.outerOffset");
      EXPECT_EQ (columns[3].name, "EventInfo.var1.outerOffset");
      EXPECT_EQ (columns[3].index, 0);
      EXPECT_EQ (columns[3].type, &typeid (ColumnarOffsetType));
      EXPECT_EQ (columns[3].accessMode, ColumnAccessMode::input);
      EXPECT_EQ (columns[3].offsetName, numberOfEventsName);
    }
    tool.setColumnIndex ("EventInfo.var1.outerOffset", 1);
    tool.setColumnIndex ("EventInfo.var1.innerOffset", 2);
    tool.setColumnIndex ("EventInfo.var1.data", 3);
    std::vector<void*> data (4, nullptr);
    std::vector<ColumnarOffsetType> var1OuterOffsets = {0, 0, 2, 5};
    std::vector<ColumnarOffsetType> var1InnerOffsets = {0, 1, 3, 6, 7, 10};
    ASSERT_EQ (var1OuterOffsets.back(), var1InnerOffsets.size()-1);
    std::vector<uint32_t> var1Data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ASSERT_EQ (var1Data.size(), var1InnerOffsets.back());
    data[1] = var1OuterOffsets.data();
    data[2] = var1InnerOffsets.data();
    data[3] = var1Data.data();
    MyId<ContainerId::eventInfo> id0 {data.data(), 0};
    MyId<ContainerId::eventInfo> id1 {data.data(), 1};
    MyId<ContainerId::eventInfo> id2 {data.data(), 2};
    EXPECT_EQ (eventAccessor (id0).size(), 0);
    EXPECT_EQ (eventAccessor (id1).size(), 2);
    EXPECT_EQ (eventAccessor (id2).size(), 3);
    EXPECT_ANY_THROW ((void) eventAccessor (id0)[0]);
    EXPECT_EQ (eventAccessor (id1)[0].size(), 1);
    EXPECT_EQ (eventAccessor (id1)[1].size(), 2);
    EXPECT_ANY_THROW ((void) eventAccessor (id1)[2]);
    EXPECT_EQ (eventAccessor (id2)[0].size(), 3);
    EXPECT_EQ (eventAccessor (id2)[1].size(), 1);
    EXPECT_EQ (eventAccessor (id2)[2].size(), 3);
    EXPECT_ANY_THROW ((void) eventAccessor (id2)[3]);
    EXPECT_EQ (eventAccessor(id1)[0][0], 0);
    EXPECT_EQ (eventAccessor(id1)[1][0], 1);
    EXPECT_EQ (eventAccessor(id1)[1][1], 2);
    EXPECT_EQ (eventAccessor(id2)[0][0], 3);
    EXPECT_EQ (eventAccessor(id2)[0][1], 4);
    EXPECT_EQ (eventAccessor(id2)[0][2], 5);
    EXPECT_EQ (eventAccessor(id2)[1][0], 6);
    EXPECT_EQ (eventAccessor(id2)[2][0], 7);
    EXPECT_EQ (eventAccessor(id2)[2][1], 8);
    EXPECT_EQ (eventAccessor(id2)[2][2], 9);
  }
}

ATLAS_GOOGLE_TEST_MAIN
