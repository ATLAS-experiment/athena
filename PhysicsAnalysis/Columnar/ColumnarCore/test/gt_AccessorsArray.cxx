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
#include <ColumnarEventInfo/EventInfoDef.h>
#include <ColumnarCore/ParticleDef.h>

//
// method implementations
//

namespace columnar
{
  using MyTool = ColumnarTool<ColumnarModeArray>;
  template<typename CT,ContainerId CI=ContainerId::particle> using MyAccessor = AccessorTemplate<CI,CT,ColumnAccessMode::input,ColumnarModeArray>;
  template<typename CT,ContainerId CI=ContainerId::particle> using MyDecorator = AccessorTemplate<CI,CT,ColumnAccessMode::output,ColumnarModeArray>;
  template<ContainerId CI=ContainerId::particle> using MyId = ObjectId<CI,ColumnarModeArray>;
  template<ContainerId CI=ContainerId::particle> using MyRange = ObjectRange<CI,ColumnarModeArray>;


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
}

ATLAS_GOOGLE_TEST_MAIN
