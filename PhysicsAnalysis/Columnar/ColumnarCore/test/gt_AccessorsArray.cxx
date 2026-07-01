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
#include <algorithm>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnInfoHelpers.h>
#include <ColumnarCore/LinkColumn.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/VectorColumn.h>
#include <ColumnarEventInfo/EventInfoDef.h>
#include <ColumnarCore/ParticleDef.h>

//
// method implementations
//

namespace columnar
{
  using MyTool = ColumnarTool<ColumnarModeArray>;
  template<typename CT,ContainerIdConcept CI=ParticleDef> using MyAccessor = AccessorTemplate<CI,CT,ColumnAccessMode::input,ColumnarModeArray>;
  template<typename CT,ContainerIdConcept CI=ParticleDef> using MyDecorator = AccessorTemplate<CI,CT,ColumnAccessMode::output,ColumnarModeArray>;
  template<ContainerIdConcept CI=ParticleDef> using MyId = ObjectId<CI,ColumnarModeArray>;
  template<ContainerIdConcept CI=ParticleDef> using MyRange = ObjectRange<CI,ColumnarModeArray>;


  TEST (AccessorTest, defaultEventOffsets)
  {
    MyTool tool;
    {
      auto columns = tool.getColumnInfo();
      EXPECT_EQ (columns.size(), 1);
      auto& column = columns[0];
      EXPECT_EQ (column.name, eventRangeColumnName);
      EXPECT_EQ (column.index, 0);
    }
    tool.setColumnIndex (eventRangeColumnName, 1);
    {
      auto columns = tool.getColumnInfo();
      EXPECT_EQ (columns.size(), 1);
      auto& column = columns[0];
      EXPECT_EQ (column.name, eventRangeColumnName);
      EXPECT_EQ (column.index, 1);
    }
  }


  TEST (AccessorTest, nativeEventAccessor)
  {
    MyTool tool;
    MyAccessor<uint32_t,EventInfoDef> eventAccessor {tool, "var1"};
    ASSERT_SUCCESS (tool.initializeColumns());
    {
      auto columns = tool.getColumnInfo();
      EXPECT_EQ (columns.size(), 2);
      auto& column = columns[1];
      EXPECT_EQ (column.name, "EventInfo.var1");
      EXPECT_EQ (column.index, 0);
      EXPECT_EQ (column.type, &typeid (uint32_t));
      EXPECT_EQ (column.accessMode, ColumnAccessMode::input);
      EXPECT_EQ (column.offsetName, eventRangeColumnName);
    }
    tool.setColumnIndex ("EventInfo.var1", 1);
    std::vector<void*> data (2, nullptr);
    std::vector<uint32_t> var1 = {0, 1, 2, 3, 4, 5};
    data[1] = var1.data();
    MyId<EventInfoDef> id1 {data.data(), 1};
    MyId<EventInfoDef> id2 {data.data(), 2};
    EXPECT_EQ (eventAccessor (id1), 1);
    EXPECT_EQ (eventAccessor (id2), 2);
    EXPECT_EQ (&eventAccessor(id1),&id1(eventAccessor));
    MyRange<EventInfoDef> range {data.data(), 1, 3};
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
    ASSERT_SUCCESS (tool.initializeColumns());
    {
      auto columns = tool.getColumnInfo();
      EXPECT_EQ (columns.size(), 3);
      {
        auto& column = columns[1];
        EXPECT_EQ (column.name, "particles");
        EXPECT_EQ (column.index, 0);
        EXPECT_EQ (column.type, &typeid (ColumnarOffsetType));
        EXPECT_EQ (column.accessMode, ColumnAccessMode::input);
        EXPECT_EQ (column.offsetName, eventRangeColumnName);
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
      MyId<EventContextDef> eventId {data.data(), 1};
      auto objectRange = objectAccessor (eventId);
      EXPECT_EQ (objectRange.size(), 2);
      EXPECT_EQ (objectRange.beginIndex(), 1);
      EXPECT_EQ (objectRange.endIndex(), 3);
    }

    {
      MyRange<EventContextDef> eventRange {data.data(), 1, 3};
      auto objectRange = objectAccessor (eventRange);
      EXPECT_EQ (objectRange.size(), 5);
      EXPECT_EQ (objectRange.beginIndex(), 1);
      EXPECT_EQ (objectRange.endIndex(), 6);
    }
  }


  TEST (AccessorTest, vectorEventAccessor)
  {
    MyTool tool;
    MyAccessor<std::vector<uint32_t>,EventInfoDef> eventAccessor {tool, "var1"};
    MyAccessor<std::vector<RetypeColumn<uint64_t,uint32_t>>,EventInfoDef> eventRetypeAccessor {tool, "var1"};
    ASSERT_SUCCESS (tool.initializeColumns());
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
      EXPECT_EQ (columnData.offsetName, eventRangeColumnName);
    }
    tool.setColumnIndex ("EventInfo.var1.offset", 1);
    tool.setColumnIndex ("EventInfo.var1.data", 2);
    std::vector<void*> data (3, nullptr);
    std::vector<ColumnarOffsetType> var1Offsets = {0, 1, 3, 6, 7};
    std::vector<uint32_t> var1Data = {0, 1, 2, 3, 4, 5, 6};
    data[1] = var1Offsets.data();
    data[2] = var1Data.data();
    MyId<EventInfoDef> id1 {data.data(), 1};
    MyId<EventInfoDef> id2 {data.data(), 2};
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
    MyAccessor<std::vector<std::vector<uint32_t>>,EventInfoDef> eventAccessor {tool, "var1"};
    ASSERT_SUCCESS (tool.initializeColumns());
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
      EXPECT_EQ (columns[3].offsetName, eventRangeColumnName);
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
    MyId<EventInfoDef> id0 {data.data(), 0};
    MyId<EventInfoDef> id1 {data.data(), 1};
    MyId<EventInfoDef> id2 {data.data(), 2};
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



  // Tests for renameColumn

  TEST (RenameColumnTest, namedParticles)
  {
    MyTool tool;
    MyAccessor<uint32_t,ParticleDef> particleAccessor {tool, "var1"};
    MyAccessor<ObjectColumn,ParticleDef> objectAccessor {tool, "Particles"};
    ASSERT_SUCCESS (tool.initializeColumns());

    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 3);
      EXPECT_EQ (columns[0].name, "EventInfo");
      EXPECT_EQ (columns[0].index, 0);
      EXPECT_EQ (columns[1].name, "Particles");
      EXPECT_EQ (columns[1].index, 0);
      EXPECT_EQ (columns[2].name, "Particles.var1");
      EXPECT_EQ (columns[2].index, 0);
    }
    tool.setColumnIndex ("Particles", 10);
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 3);
      EXPECT_EQ (columns[1].name, "Particles");
      EXPECT_EQ (columns[1].index, 10);
    }
    tool.setColumnIndex ("Particles.var1", 1);
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 3);
      EXPECT_EQ (columns[2].name, "Particles.var1");
      EXPECT_EQ (columns[2].index, 1);
      EXPECT_EQ (columns[2].offsetName, "Particles");
    }
    tool.renameColumn ("Particles.var1", "XParticles.var1");
    EXPECT_ANY_THROW (tool.renameColumn ("Particles.var1", "XParticles.var1"));
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 3);
      EXPECT_EQ (columns[2].name, "XParticles.var1");
      EXPECT_EQ (columns[2].index, 1);
    }
    tool.setColumnIndex ("XParticles.var1", 2);
    EXPECT_ANY_THROW (tool.setColumnIndex ("Particles.var1", 3));
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 3);
      EXPECT_EQ (columns[2].name, "XParticles.var1");
      EXPECT_EQ (columns[2].index, 2);
    }
    tool.renameColumn ("Particles", "XParticles");
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 3);
      EXPECT_EQ (columns[1].name, "XParticles");
      EXPECT_EQ (columns[1].index, 10);
      EXPECT_EQ (columns[2].name, "XParticles.var1");
      EXPECT_EQ (columns[2].offsetName, "XParticles");
    }
    tool.setColumnIndex ("XParticles", 20);
    EXPECT_ANY_THROW (tool.setColumnIndex ("Particles", 30));
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 3);
      EXPECT_EQ (columns[1].name, "XParticles");
      EXPECT_EQ (columns[1].index, 20);
    }
    tool.renameColumn ("XParticles.var1", "YParticles.var1");
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 3);
      EXPECT_EQ (columns[2].name, "YParticles.var1");
      EXPECT_EQ (columns[2].index, 2);
    }
    tool.setColumnIndex ("YParticles.var1", 4);
    EXPECT_ANY_THROW (tool.setColumnIndex ("Particles.var1", 5));
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 3);
      EXPECT_EQ (columns[2].name, "YParticles.var1");
      EXPECT_EQ (columns[2].index, 4);
    }
  }

  TEST (RenameColumnTest, basicEventInfo)
  {
    MyTool tool;
    MyAccessor<uint32_t,EventInfoDef> eventAccessor {tool, "var1"};
    ASSERT_SUCCESS (tool.initializeColumns());

    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 2);
      EXPECT_EQ (columns[0].name, "EventInfo");
      EXPECT_EQ (columns[0].index, 0);
      EXPECT_EQ (columns[1].name, "EventInfo.var1");
      EXPECT_EQ (columns[1].index, 0);
    }
    tool.setColumnIndex ("EventInfo", 10);
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 2);
      EXPECT_EQ (columns[0].name, "EventInfo");
      EXPECT_EQ (columns[0].index, 10);
    }
    tool.setColumnIndex ("EventInfo.var1", 1);
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 2);
      EXPECT_EQ (columns[1].name, "EventInfo.var1");
      EXPECT_EQ (columns[1].index, 1);
      EXPECT_EQ (columns[1].offsetName, eventRangeColumnName);
    }
    tool.renameColumn ("EventInfo.var1", "MyEventInfo.var1");
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 2);
      EXPECT_EQ (columns[1].name, "MyEventInfo.var1");
      EXPECT_EQ (columns[1].index, 1);
    }
    tool.setColumnIndex ("MyEventInfo.var1", 2);
    EXPECT_ANY_THROW (tool.setColumnIndex ("EventInfo.var1", 3));
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 2);
      EXPECT_EQ (columns[1].name, "MyEventInfo.var1");
      EXPECT_EQ (columns[1].index, 2);
    }
    tool.renameColumn ("EventInfo", "MyEventInfo");
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 2);
      EXPECT_EQ (columns[0].name, "MyEventInfo");
      EXPECT_EQ (columns[0].index, 10);
      EXPECT_EQ (columns[1].name, "MyEventInfo.var1");
      EXPECT_EQ (columns[1].offsetName, "MyEventInfo");
    }
    tool.setColumnIndex ("MyEventInfo", 20);
    EXPECT_ANY_THROW (tool.setColumnIndex ("EventInfo", 30));
    {
      auto columns = tool.getColumnInfo();
      ASSERT_EQ (columns.size(), 2);
      EXPECT_EQ (columns[0].name, "MyEventInfo");
      EXPECT_EQ (columns[0].index, 20);
    }
  }



  // Tests for link accessors and StoreGate key computation

  TEST (LinkAccessorTest, linkColumnInfo)
  {
    MyTool tool;
    MyAccessor<ObjectColumn> objectAccessor {tool, "Particles"};
    MyAccessor<ObjectColumn,Particle1Def> targetAccessor {tool, "Targets"};
    MyAccessor<OptObjectId<Particle1Def,ColumnarModeArray>> linkAccessor {tool, "targetLink"};
    ASSERT_SUCCESS (tool.initializeColumns());

    auto columns = tool.getColumnInfo();
    auto linkColumn = std::find_if (columns.begin(), columns.end(), [] (auto& column) {return column.name == "Particles.targetLink";});
    ASSERT_NE (linkColumn, columns.end());
    EXPECT_EQ (linkColumn->soleLinkTargetName, "Targets");
    EXPECT_EQ (linkColumn->soleLinkTargetClid, ClassID_traits<xAOD::IParticleContainer>::ID());
  }


  TEST (ComputeSgKeyTest, knownKeys)
  {
    // expected values read from a PHYSLITE input file (see also the
    // knownKeys table in ColumnarTestFixtures), with the CLIDs from
    // the CLASS_DEF macros of the corresponding container types
    EXPECT_EQ (computeSgKey ("AnalysisMuons", 1178459224), 0x3a6b126fu);
    EXPECT_EQ (computeSgKey ("InDetTrackParticles", 1287425431), 0x1d3890dbu);
    EXPECT_EQ (computeSgKey ("egammaClusters", 1219821989), 0x15788d1fu);
    // without the CLID the key differs
    EXPECT_NE (computeSgKey ("InDetTrackParticles", 0), 0x1d3890dbu);
  }
}

ATLAS_GOOGLE_TEST_MAIN
