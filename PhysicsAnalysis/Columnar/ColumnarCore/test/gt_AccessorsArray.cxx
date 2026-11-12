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
#include <algorithm>
#include <array>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnInfoHelpers.h>
#include <ColumnarCore/LinkColumn.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/VectorColumn.h>
#include <ColumnarJet/JetDef.h>
#include <ColumnarEventInfo/EventInfoDef.h>
#include <ColumnarCore/ParticleDef.h>

//
// method implementations
//

namespace columnar
{
  static_assert (std::is_same_v<ColumnarMode,ColumnarModeArray>, "this test is only defined for array mode");

  TEST (AccessorTest, defaultEventOffsets)
  {
    ColumnarTool<> tool;
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
    ColumnarTool<> tool;
    EventInfoAccessor<uint32_t> eventAccessor {tool, "var1"};
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
    EventInfoId id1 {data.data(), 1};
    EventInfoId id2 {data.data(), 2};
    EXPECT_EQ (eventAccessor (id1), 1);
    EXPECT_EQ (eventAccessor (id2), 2);
    EXPECT_EQ (&eventAccessor(id1),&id1(eventAccessor));
    EventInfoRange range {data.data(), 1, 3};
    auto rangeView = eventAccessor (range);
    EXPECT_EQ (rangeView.size(), 2);
    EXPECT_EQ (rangeView.data(), var1.data() + 1);
    EXPECT_EQ (range(eventAccessor).data(), rangeView.data());
  }


  TEST (AccessorTest, objectAccessor)
  {
    ColumnarTool<> tool;
    ParticleAccessor<ObjectColumn> objectAccessor {tool, "particles"};
    ParticleAccessor<uint32_t> varAccessor {tool, "var1"};
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
      EventContextId eventId {data.data(), 1};
      auto objectRange = objectAccessor (eventId);
      EXPECT_EQ (objectRange.size(), 2);
      EXPECT_EQ (objectRange.beginIndex(), 1);
      EXPECT_EQ (objectRange.endIndex(), 3);
    }

    {
      EventContextRange eventRange {data.data(), 1, 3};
      auto objectRange = objectAccessor (eventRange);
      EXPECT_EQ (objectRange.size(), 5);
      EXPECT_EQ (objectRange.beginIndex(), 1);
      EXPECT_EQ (objectRange.endIndex(), 6);
    }
  }


  TEST (AccessorTest, vectorEventAccessor)
  {
    ColumnarTool<> tool;
    EventInfoAccessor<std::vector<uint32_t>> eventAccessor {tool, "var1"};
    EventInfoAccessor<std::vector<RetypeColumn<uint64_t,uint32_t>>> eventRetypeAccessor {tool, "var1"};
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
    EventInfoId id1 {data.data(), 1};
    EventInfoId id2 {data.data(), 2};
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
    ColumnarTool<> tool;
    EventInfoAccessor<std::vector<std::vector<uint32_t>>> eventAccessor {tool, "var1"};
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
    EventInfoId id0 {data.data(), 0};
    EventInfoId id1 {data.data(), 1};
    EventInfoId id2 {data.data(), 2};
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
    ColumnarTool<> tool;
    ParticleAccessor<uint32_t> particleAccessor {tool, "var1"};
    ParticleAccessor<ObjectColumn> objectAccessor {tool, "Particles"};
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
    ColumnarTool<> tool;
    EventInfoAccessor<uint32_t> eventAccessor {tool, "var1"};
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
    ColumnarTool<> tool;
    ParticleAccessor<ObjectColumn> objectAccessor {tool, "Particles"};
    Particle1Accessor<ObjectColumn> targetAccessor {tool, "Targets"};
    ParticleAccessor<OptParticle1Id> linkAccessor {tool, "targetLink"};
    ASSERT_SUCCESS (tool.initializeColumns());

    auto columns = tool.getColumnInfo();
    auto hasColumn = [&] (const std::string& name) {return std::any_of (columns.begin(), columns.end(), [&] (auto& column) {return column.name == name;});};
    // the key and index columns have no link specific meta-data,
    // the link just requests the offset map of its target
    EXPECT_FALSE (hasColumn ("Particles.targetLink"));
    EXPECT_TRUE (hasColumn ("Particles.targetLink.m_persKey"));
    EXPECT_TRUE (hasColumn ("Particles.targetLink.m_persIndex"));
    EXPECT_TRUE (hasColumn ("Particles.eventIndex"));
    EXPECT_TRUE (hasColumn ("Targets"));
  }


  template<ColumnarModeConcept CM>
  void checkTargetKey ()
  {
    using LinkAccessor = detail::ContainerFreeAccessor<OptObjectId<Particle1Def,CM>,ColumnAccessMode::input,CM>;
    ColumnarTool<CM> tool;
    ParticleAccessor<ObjectColumn,CM> objectAccessor {tool, "Particles"};
    Particle1Accessor<ObjectColumn,CM> targetAccessor {tool, "Targets"};
    LinkAccessor linkAccessor {tool, {}, {.offsetName = ParticleDef::idName, .baseName = std::string (ParticleDef::idName) + ".targetLink"}};
    const auto expectedKey = computeSgKey ("Targets", ClassID_traits<xAOD::IParticleContainer>::ID());
    EXPECT_EQ (linkAccessor.targetKey(), expectedKey);
    // a link with just the target type accepts exactly one key
    static_assert (LinkCastColumn<Particle1Def>::numLinkTypes == 1u);
    EXPECT_EQ (linkAccessor.allTargetKeys().size(), 1u);

    LinkAccessor movedAccessor {std::move (linkAccessor)};
    EXPECT_EQ (movedAccessor.targetKey(), expectedKey);
    LinkAccessor assignedAccessor;
    assignedAccessor = std::move (movedAccessor);
    EXPECT_EQ (assignedAccessor.targetKey(), expectedKey);
  }

  template<ColumnarModeConcept CM>
  void checkTargetKeyLateRegistration ()
  {
    // the link is declared before the ObjectColumn of its target
    using LinkAccessor = detail::ContainerFreeAccessor<OptObjectId<Particle1Def,CM>,ColumnAccessMode::input,CM>;
    ColumnarTool<CM> tool;
    ParticleAccessor<ObjectColumn,CM> objectAccessor {tool, "Particles"};
    LinkAccessor linkAccessor {tool, {}, {.offsetName = ParticleDef::idName, .baseName = std::string (ParticleDef::idName) + ".targetLink"}};
    Particle1Accessor<ObjectColumn,CM> targetAccessor {tool, "Targets"};
    EXPECT_EQ (linkAccessor.targetKey(), computeSgKey ("Targets", ClassID_traits<xAOD::IParticleContainer>::ID()));
    EXPECT_NO_THROW (tool.getColumnInfo());
  }

  template<ColumnarModeConcept CM>
  void checkTargetKeyMissingName ()
  {
    // the link target has no ObjectColumn, so its name and with it
    // the target key are unknown
    using LinkAccessor = detail::ContainerFreeAccessor<OptObjectId<Particle1Def,CM>,ColumnAccessMode::input,CM>;
    ColumnarTool<CM> tool;
    ParticleAccessor<ObjectColumn,CM> objectAccessor {tool, "Particles"};
    LinkAccessor linkAccessor {tool, {}, {.offsetName = ParticleDef::idName, .baseName = std::string (ParticleDef::idName) + ".targetLink"}};
    EXPECT_ANY_THROW (tool.getColumnInfo());
  }

  /// @brief the name reported for links in the given column
  std::string nameForLinks (const IColumnarTool& tool, const std::string& column)
  {
    for (const auto& info : tool.getColumnInfo())
    {
      if (info.name == column)
        return info.nameForLinks;
    }
    throw std::runtime_error ("column not found: " + column);
  }

  template<ColumnarModeConcept CM>
  void checkTargetKeyRename ()
  {
    using LinkAccessor = detail::ContainerFreeAccessor<OptObjectId<Particle1Def,CM>,ColumnAccessMode::input,CM>;
    ColumnarTool<CM> tool;
    ParticleAccessor<ObjectColumn,CM> objectAccessor {tool, "Particles"};
    Particle1Accessor<ObjectColumn,CM> targetAccessor {tool, "Targets"};
    LinkAccessor linkAccessor {tool, {}, {.offsetName = ParticleDef::idName, .baseName = std::string (ParticleDef::idName) + ".targetLink"}};
    // renaming the column doesn't change the key
    tool.renameColumn ("Targets", "Other");
    EXPECT_EQ (linkAccessor.targetKey(), computeSgKey ("Targets", ClassID_traits<xAOD::IParticleContainer>::ID()));
    EXPECT_EQ (nameForLinks (tool, "Other"), "Targets");
    // setting the link target name does, using the current column name
    tool.setLinkTargetName ("Other", "Link1");
    EXPECT_EQ (linkAccessor.targetKey(), computeSgKey ("Link1", ClassID_traits<xAOD::IParticleContainer>::ID()));
    EXPECT_EQ (nameForLinks (tool, "Other"), "Link1");
    // the link target name is not the name of the column
    EXPECT_ANY_THROW (tool.setLinkTargetName ("Link1", "Link2"));
    tool.setLinkTargetName ("Other", "Link2");
    EXPECT_EQ (nameForLinks (tool, "Other"), "Link2");
    EXPECT_ANY_THROW (tool.setLinkTargetName ("Targets", "Link3"));
    EXPECT_ANY_THROW (tool.setLinkTargetName ("Unknown", "Link3"));
    EXPECT_ANY_THROW (tool.setLinkTargetName ("Particles.targetLink.m_persKey", "Link3"));
    // a later rename keeps the name, and needs the new column name
    tool.renameColumn ("Other", "Another");
    EXPECT_EQ (nameForLinks (tool, "Another"), "Link2");
    EXPECT_ANY_THROW (tool.setLinkTargetName ("Other", "Link3"));
    tool.setLinkTargetName ("Another", "Link3");
    EXPECT_EQ (linkAccessor.targetKey(), computeSgKey ("Link3", ClassID_traits<xAOD::IParticleContainer>::ID()));
    // containers nothing links to don't report a name
    EXPECT_EQ (nameForLinks (tool, "Particles"), "");
  }

  template<ColumnarModeConcept CM>
  void checkTargetKeyMerge ()
  {
    using LinkAccessor1 = detail::ContainerFreeAccessor<OptObjectId<Particle1Def,CM>,ColumnAccessMode::input,CM>;
    using LinkAccessor2 = detail::ContainerFreeAccessor<OptObjectId<Particle2Def,CM>,ColumnAccessMode::input,CM>;
    ColumnarTool<CM> tool;
    ParticleAccessor<ObjectColumn,CM> objectAccessor {tool, "Particles"};
    Particle1Accessor<ObjectColumn,CM> targetAccessor1 {tool, "Targets1"};
    Particle2Accessor<ObjectColumn,CM> targetAccessor2 {tool, "Targets2"};
    LinkAccessor1 linkAccessor1 {tool, {}, {.offsetName = ParticleDef::idName, .baseName = std::string (ParticleDef::idName) + ".targetLink1"}};
    LinkAccessor2 linkAccessor2 {tool, {}, {.offsetName = ParticleDef::idName, .baseName = std::string (ParticleDef::idName) + ".targetLink2"}};
    // a merged column has a single name, so the two defaults conflict
    tool.renameColumn ("Targets1", "Merged");
    tool.renameColumn ("Targets2", "Merged");
    EXPECT_ANY_THROW (tool.getColumnInfo());
    // one call sets the name for both containers
    tool.setLinkTargetName ("Merged", "Link");
    EXPECT_EQ (nameForLinks (tool, "Merged"), "Link");
    EXPECT_EQ (linkAccessor1.targetKey(), computeSgKey ("Link", ClassID_traits<xAOD::IParticleContainer>::ID()));
    EXPECT_EQ (linkAccessor2.targetKey(), computeSgKey ("Link", ClassID_traits<xAOD::IParticleContainer>::ID()));
  }

  template<ColumnarModeConcept CM>
  void checkTargetKeySubtool ()
  {
    // the link is in a subtool, the ObjectColumn of its target in the
    // parent tool, and they only get joined afterwards
    using LinkAccessor = detail::ContainerFreeAccessor<OptObjectId<Particle1Def,CM>,ColumnAccessMode::input,CM>;
    ColumnarTool<CM> subtool;
    ParticleAccessor<ObjectColumn,CM> subObjectAccessor {subtool, "Particles"};
    LinkAccessor linkAccessor {subtool, {}, {.offsetName = ParticleDef::idName, .baseName = std::string (ParticleDef::idName) + ".targetLink"}};

    ColumnarTool<CM> tool;
    ParticleAccessor<ObjectColumn,CM> objectAccessor {tool, "Particles"};
    Particle1Accessor<ObjectColumn,CM> targetAccessor {tool, "Targets"};
    tool.addSubtool (subtool);
    EXPECT_EQ (linkAccessor.targetKey(), computeSgKey ("Targets", ClassID_traits<xAOD::IParticleContainer>::ID()));
    EXPECT_NO_THROW (tool.getColumnInfo());
  }

  template<ColumnarModeConcept CM>
  void checkKeyIndexLinkResolve ()
  {
    using LinkAccessor = detail::ContainerFreeAccessor<OptObjectId<Particle1Def,CM>,ColumnAccessMode::input,CM>;
    ColumnarTool<CM> tool;
    ParticleAccessor<ObjectColumn,CM> objectAccessor {tool, "Particles"};
    Particle1Accessor<ObjectColumn,CM> targetAccessor {tool, "Targets"};
    LinkAccessor linkAccessor {tool, {}, {.offsetName = ParticleDef::idName, .baseName = std::string (ParticleDef::idName) + ".targetLink"}};
    const auto key = linkAccessor.targetKey();
    tool.setColumnIndex ("Particles.targetLink.m_persKey", 1);
    tool.setColumnIndex ("Particles.targetLink.m_persIndex", 2);
    tool.setColumnIndex ("Particles.eventIndex", 3);
    tool.setColumnIndex ("Targets", 4);

    // two events, with two and three target objects
    const std::vector<typename CM::LinkKeyColumnType> keys {key, key, key, 0, key, 0, key, key + 1};
    const std::vector<typename CM::LinkIndexColumnType> indices {0, 1, 2, 5, CM::nullLinkIndex, 0, 3, 0};
    const std::vector<ColumnarOffsetType> events {0, 0, 1, 1, 1, 1, 1, 1};
    const std::vector<ColumnarOffsetType> targetOffsets {0, 2, 5};
    std::array<void*,5> data {nullptr, const_cast<void*>(static_cast<const void*>(keys.data())), const_cast<void*>(static_cast<const void*>(indices.data())), const_cast<void*>(static_cast<const void*>(events.data())), const_cast<void*>(static_cast<const void*>(targetOffsets.data()))};

    EXPECT_TRUE (linkAccessor.isAvailable (data.data()));
    EXPECT_EQ (linkAccessor (data.data(), 0).value().getIndex(), 0u);
    EXPECT_EQ (linkAccessor (data.data(), 1).value().getIndex(), 1u);
    // the index is relative to the event
    EXPECT_EQ (linkAccessor (data.data(), 2).value().getIndex(), 4u);
    // a null link if either the key or the index is null
    EXPECT_FALSE (linkAccessor (data.data(), 3).has_value());
    EXPECT_FALSE (linkAccessor (data.data(), 4).has_value());
    EXPECT_FALSE (linkAccessor (data.data(), 5).has_value());
    // the index is outside the event, even though it is inside the column
    EXPECT_ANY_THROW ((void) linkAccessor (data.data(), 6));
    // the key doesn't match the target
    EXPECT_ANY_THROW ((void) linkAccessor (data.data(), 7));

    auto range = linkAccessor (data.data(), 1, 3);
    ASSERT_EQ (range.size(), 2u);
    EXPECT_EQ (range[1].value().getIndex(), 4u);

    data[4] = nullptr;
    EXPECT_FALSE (linkAccessor.isAvailable (data.data()));
  }

  template<ColumnarModeConcept CM>
  void checkKeyIndexLinkExtraTypes ()
  {
    // a generic target also accepts the key for the listed extra type,
    // i.e. here links that were written as links to jets
    using LinkAccessor = detail::ContainerFreeAccessor<LinkCastColumn<Particle1Def,ParticleDef,JetDef>,ColumnAccessMode::input,CM>;
    ColumnarTool<CM> tool;
    ParticleAccessor<ObjectColumn,CM> objectAccessor {tool, "Particles"};
    Particle1Accessor<ObjectColumn,CM> targetAccessor {tool, "Targets"};
    LinkAccessor linkAccessor {tool, {}, {.offsetName = ParticleDef::idName, .baseName = std::string (ParticleDef::idName) + ".targetLink"}};
    const auto baseKey = computeSgKey ("Targets", ClassID_traits<xAOD::IParticleContainer>::ID());
    const auto extraKey = computeSgKey ("Targets", ClassID_traits<xAOD::JetContainer>::ID());
    EXPECT_EQ (linkAccessor.targetKey(), baseKey);
    EXPECT_EQ (linkAccessor.allTargetKeys().size(), 2u);
    // the static count has to match the keys the accessor really has
    using LinkColumn = LinkCastColumn<Particle1Def,ParticleDef,JetDef>;
    static_assert (LinkColumn::numLinkTypes == 2u);
    EXPECT_EQ (linkAccessor.allTargetKeys().size(), LinkColumn::numLinkTypes);
    tool.setColumnIndex ("Particles.targetLink.m_persKey", 1);
    tool.setColumnIndex ("Particles.targetLink.m_persIndex", 2);
    tool.setColumnIndex ("Particles.eventIndex", 3);
    tool.setColumnIndex ("Targets", 4);

    std::vector<typename CM::LinkKeyColumnType> keys {baseKey, extraKey, baseKey + extraKey};
    std::vector<typename CM::LinkIndexColumnType> indices {0, 1, 0};
    std::vector<ColumnarOffsetType> events {0, 0, 0};
    std::vector<ColumnarOffsetType> targetOffsets {0, 2};
    std::array<void*,5> data {nullptr, keys.data(), indices.data(), events.data(), targetOffsets.data()};
    EXPECT_EQ (linkAccessor (data.data(), 0).value().getIndex(), 0u);
    EXPECT_EQ (linkAccessor (data.data(), 1).value().getIndex(), 1u);
    EXPECT_ANY_THROW ((void) linkAccessor (data.data(), 2));
  }

  TEST (LinkAccessorTest, keyIndexLinkExtraTypes)
  {
    checkKeyIndexLinkExtraTypes<ColumnarMode> ();
  }

  TEST (LinkAccessorTest, keyIndexLinkResolve)
  {
    checkKeyIndexLinkResolve<ColumnarMode> ();
  }

  TEST (LinkAccessorTest, targetKey)
  {
    checkTargetKey<ColumnarMode> ();
  }

  TEST (LinkAccessorTest, targetKeyLateRegistration)
  {
    checkTargetKeyLateRegistration<ColumnarMode> ();
  }

  TEST (LinkAccessorTest, targetKeyMissingName)
  {
    checkTargetKeyMissingName<ColumnarMode> ();
  }

  TEST (LinkAccessorTest, targetKeyRename)
  {
    checkTargetKeyRename<ColumnarMode> ();
    checkTargetKeyMerge<ColumnarMode> ();
  }

  TEST (LinkAccessorTest, targetKeySubtool)
  {
    checkTargetKeySubtool<ColumnarMode> ();
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
