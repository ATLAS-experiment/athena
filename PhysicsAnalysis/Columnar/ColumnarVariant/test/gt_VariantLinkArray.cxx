/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <AsgTesting/UnitTest.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/ParticleDef.h>
#include <ColumnarJet/JetDef.h>
#include <ColumnarVariant/VariantLinkColumn.h>

#include <array>
#include <vector>

#include <gtest/gtest.h>

//
// unit test
//

namespace columnar
{
  template<ColumnarModeConcept CM>
  void checkKeyIndexVariantLink ()
  {
    using VariantCI = VariantContainerId<Particle1Def,Particle1Def,Particle2Def>;
    using Link = ObjectLink<VariantCI,CM>;
    using LinkAccessor = detail::ContainerFreeAccessor<Link,ColumnAccessMode::input,CM>;
    using LinkDecorator = detail::ContainerFreeAccessor<Link,ColumnAccessMode::output,CM>;
    ColumnarTool<CM> tool;
    ParticleAccessor<ObjectColumn,CM> objectAccessor {tool, "Particles"};
    Particle1Accessor<ObjectColumn,CM> targetAccessor1 {tool, "Targets1"};
    Particle2Accessor<ObjectColumn,CM> targetAccessor2 {tool, "Targets2"};
    LinkAccessor linkAccessor {tool, {}, {.offsetName = ParticleDef::idName, .baseName = std::string (ParticleDef::idName) + ".targetLink"}};
    LinkDecorator linkDecorator {tool, {}, {.offsetName = ParticleDef::idName, .baseName = std::string (ParticleDef::idName) + ".targetLink"}};
    tool.setColumnIndex ("Particles.targetLink.m_persKey", 1);
    tool.setColumnIndex ("Particles.targetLink.m_persIndex", 2);
    tool.setColumnIndex ("Particles.eventIndex", 3);
    tool.setColumnIndex ("Targets1", 4);
    tool.setColumnIndex ("Targets2", 5);
    const auto key1 = linkAccessor.targetKeys()[0];
    const auto key2 = linkAccessor.targetKeys()[1];
    ASSERT_NE (key1, key2);

    // all links are in the second event
    std::vector<typename CM::LinkKeyColumnType> keys {key1, key2, key1 + key2, CM::nullLinkKey, key1};
    std::vector<typename CM::LinkIndexColumnType> indices {1, 1, 0, CM::nullLinkIndex, 2};
    std::vector<ColumnarOffsetType> events (keys.size(), 1);
    std::vector<ColumnarOffsetType> targetOffsets1 {0, 2, 4};
    std::vector<ColumnarOffsetType> targetOffsets2 {0, 3, 5};
    std::array<void*,6> data {nullptr, keys.data(), indices.data(), events.data(), targetOffsets1.data(), targetOffsets2.data()};
    void **dataArea = data.data();

    EXPECT_TRUE (linkAccessor.isAvailable (dataArea));
    auto link1 = linkAccessor (dataArea, 0);
    EXPECT_TRUE (link1.has_value());
    EXPECT_TRUE (link1.template isContainer<Particle1Def>());
    EXPECT_FALSE (link1.template isContainer<Particle2Def>());
    EXPECT_EQ (link1.template tryGetVariant<Particle1Def>().value().getIndex(), 3u);
    EXPECT_FALSE (link1.template tryGetVariant<Particle2Def>().has_value());
    EXPECT_TRUE (link1 == (ObjectId<Particle1Def,CM> (dataArea, 3)));
    EXPECT_FALSE (link1 == (ObjectId<Particle1Def,CM> (dataArea, 2)));

    auto link2 = linkAccessor (dataArea, 1);
    EXPECT_TRUE (link2.template isContainer<Particle2Def>());
    EXPECT_EQ (link2.template tryGetVariant<Particle2Def>().value().getIndex(), 4u);
    EXPECT_FALSE (link1 == link2);

    // a key not matching any target
    auto linkUnknown = linkAccessor (dataArea, 2);
    EXPECT_TRUE (linkUnknown.has_value());
    EXPECT_FALSE (linkUnknown.template isContainer<Particle1Def>());
    EXPECT_FALSE (linkUnknown.template isContainer<Particle2Def>());
    EXPECT_FALSE (linkUnknown.template tryGetVariant<Particle1Def>().has_value());
    EXPECT_ANY_THROW ((void) linkUnknown.opt_value());

    auto linkNull = linkAccessor (dataArea, 3);
    EXPECT_FALSE (linkNull.has_value());
    EXPECT_FALSE (linkNull.opt_value().has_value());

    // the index is outside the event
    EXPECT_ANY_THROW ((void) linkAccessor (dataArea, 4).opt_value());

    auto range = linkAccessor (dataArea, 0, 2);
    ASSERT_EQ (range.size(), 2u);
    EXPECT_TRUE (range[1].template isContainer<Particle2Def>());

    // writing a link, which should then read back the same
    linkDecorator.set (dataArea, 3, ObjectId<Particle2Def,CM> (dataArea, 3));
    EXPECT_EQ (keys[3], key2);
    EXPECT_EQ (indices[3], 0u);
    EXPECT_EQ (linkAccessor (dataArea, 3).template tryGetVariant<Particle2Def>().value().getIndex(), 3u);

    // the target of a link has to be in the same event as the link
    EXPECT_ANY_THROW (linkDecorator.set (dataArea, 3, ObjectId<Particle2Def,CM> (dataArea, 2)));
    EXPECT_ANY_THROW (linkDecorator.set (dataArea, 3, ObjectId<Particle2Def,CM> (dataArea, 5)));
    EXPECT_EQ (keys[3], key2);
    EXPECT_EQ (indices[3], 0u);
  }

  template<ColumnarModeConcept CM>
  void checkKeyIndexVariantLinkCompare ()
  {
    // the extra type gives each target a second key it accepts
    using VariantCI = VariantContainerId<Particle1Def,Particle1Def,Particle2Def>;
    using LinkAccessor = detail::ContainerFreeAccessor<LinkCastColumn<VariantCI,ParticleDef,JetDef>,ColumnAccessMode::input,CM>;
    ColumnarTool<CM> tool;
    ParticleAccessor<ObjectColumn,CM> objectAccessor {tool, "Particles"};
    Particle1Accessor<ObjectColumn,CM> targetAccessor1 {tool, "Targets1"};
    Particle2Accessor<ObjectColumn,CM> targetAccessor2 {tool, "Targets2"};
    LinkAccessor linkAccessor {tool, {}, {.offsetName = ParticleDef::idName, .baseName = std::string (ParticleDef::idName) + ".targetLink"}};
    tool.setColumnIndex ("Particles.targetLink.m_persKey", 1);
    tool.setColumnIndex ("Particles.targetLink.m_persIndex", 2);
    tool.setColumnIndex ("Particles.eventIndex", 3);
    tool.setColumnIndex ("Targets1", 4);
    tool.setColumnIndex ("Targets2", 5);
    const auto& allKeys = linkAccessor.allTargetKeys();
    ASSERT_EQ (allKeys[0].size(), 2u);
    // the static count has to match the keys the accessor really has
    using VariantColumn = LinkCastColumn<VariantCI,ParticleDef,JetDef>;
    static_assert (VariantColumn::numLinkTypes == 2u);
    ASSERT_EQ (allKeys[0].size(), VariantColumn::numLinkTypes);
    ASSERT_EQ (allKeys[1].size(), VariantColumn::numLinkTypes);
    const auto key1 = allKeys[0][0];
    const auto key1Other = allKeys[0][1];
    const auto key2 = allKeys[1][0];
    const auto keyUnknown = key1 + key1Other + key2;
    ASSERT_NE (key1, key1Other);

    std::vector<typename CM::LinkKeyColumnType> keys {key1, key1Other, key1, key2, CM::nullLinkKey, keyUnknown, keyUnknown, key1, key1 + key2};
    std::vector<typename CM::LinkIndexColumnType> indices {1, 1, 1, 1, CM::nullLinkIndex, 1, 1, 99, 1};
    std::vector<ColumnarOffsetType> events {1, 1, 0, 1, 1, 1, 1, 1, 1};
    std::vector<ColumnarOffsetType> targetOffsets1 {0, 2, 4};
    std::vector<ColumnarOffsetType> targetOffsets2 {0, 3, 5};
    std::array<void*,6> data {nullptr, keys.data(), indices.data(), events.data(), targetOffsets1.data(), targetOffsets2.data()};
    void **dataArea = data.data();
    auto link = [&] (std::size_t index) {return linkAccessor.link (dataArea, index);};

    // the same object through two different keys
    EXPECT_TRUE (link(0) == link(1));
    // the same key and index, but in a different event
    EXPECT_FALSE (link(0) == link(2));
    // the same index, but in a different container
    EXPECT_FALSE (link(0) == link(3));
    // null links
    EXPECT_TRUE (link(4) == link(4));
    EXPECT_FALSE (link(0) == link(4));
    EXPECT_FALSE (link(4) == link(0));
    // links to containers not in the variant
    EXPECT_TRUE (link(5) == link(6));
    EXPECT_FALSE (link(0) == link(5));
    EXPECT_FALSE (link(5) == link(8));
    // comparing doesn't check the index against the target
    EXPECT_NO_THROW (EXPECT_FALSE (link(0) == link(7)));
    EXPECT_NO_THROW (EXPECT_TRUE (link(7) == link(7)));
  }

  template<ColumnarModeConcept CM>
  void checkKeyIndexVariantLinkMerged ()
  {
    // both targets are merged into a single column, so they accept the
    // same keys
    using VariantCI = VariantContainerId<Particle1Def,Particle1Def,Particle2Def>;
    using LinkAccessor = detail::ContainerFreeAccessor<LinkCastColumn<VariantCI,ParticleDef,JetDef>,ColumnAccessMode::input,CM>;
    ColumnarTool<CM> tool;
    ParticleAccessor<ObjectColumn,CM> objectAccessor {tool, "Particles"};
    Particle1Accessor<ObjectColumn,CM> targetAccessor1 {tool, "Targets1"};
    Particle2Accessor<ObjectColumn,CM> targetAccessor2 {tool, "Targets2"};
    LinkAccessor linkAccessor {tool, {}, {.offsetName = ParticleDef::idName, .baseName = std::string (ParticleDef::idName) + ".targetLink"}};
    tool.renameColumn ("Targets1", "Merged");
    tool.renameColumn ("Targets2", "Merged");
    tool.setLinkTargetName ("Merged", "Link");
    tool.setColumnIndex ("Particles.targetLink.m_persKey", 1);
    tool.setColumnIndex ("Particles.targetLink.m_persIndex", 2);
    tool.setColumnIndex ("Particles.eventIndex", 3);
    tool.setColumnIndex ("Merged", 4);
    const auto& allKeys = linkAccessor.allTargetKeys();
    ASSERT_EQ (allKeys[0].size(), 2u);
    EXPECT_EQ (allKeys[0], allKeys[1]);

    std::vector<typename CM::LinkKeyColumnType> keys {allKeys[0][0], allKeys[0][1]};
    std::vector<typename CM::LinkIndexColumnType> indices {1, 1};
    std::vector<ColumnarOffsetType> events {0, 0};
    std::vector<ColumnarOffsetType> targetOffsets {0, 2};
    std::array<void*,5> data {nullptr, keys.data(), indices.data(), events.data(), targetOffsets.data()};
    auto link = linkAccessor.link (data.data(), 0);
    EXPECT_TRUE (link.template isContainer<Particle1Def>());
    EXPECT_TRUE (link.template isContainer<Particle2Def>());
    EXPECT_TRUE (link == linkAccessor.link (data.data(), 1));
  }

  TEST (LinkAccessorTest, keyIndexVariantLink)
  {
    checkKeyIndexVariantLink<ColumnarMode> ();
  }

  TEST (LinkAccessorTest, keyIndexVariantLinkCompare)
  {
    checkKeyIndexVariantLinkCompare<ColumnarMode> ();
  }

  TEST (LinkAccessorTest, keyIndexVariantLinkMerged)
  {
    checkKeyIndexVariantLinkMerged<ColumnarMode> ();
  }
}

ATLAS_GOOGLE_TEST_MAIN
