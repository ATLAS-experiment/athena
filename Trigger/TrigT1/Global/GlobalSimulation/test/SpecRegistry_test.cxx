/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

/*
  Checks that every spec listed in selection.xml can actually be built and used.

  Nothing about a spec is verified at compile time any more: the pack and unpack
  functions are made from the type name through the interpreter, so a spec that
  is listed but cannot be compiled against fails when a job starts rather than
  when the package is built. This test is what moves that back to build time.
*/

#include "SpecRegistry.h"

#include "AthContainers/AuxElement.h"

#include "gtest/gtest.h"

#include <string>
#include <vector>

namespace {

  /// The names in the dictionary, split out of knownSpecNames().
  std::vector<std::string> specNames() {
    std::vector<std::string> names;
    const std::string all = GlobalSim::knownSpecNames();
    std::size_t begin = 0;
    while (begin < all.size()) {
      const std::size_t comma = all.find(", ", begin);
      names.push_back(all.substr(begin, comma - begin));
      if (comma == std::string::npos) { break; }
      begin = comma + 2;
    }
    return names;
  }

  /// A token that exercises the whole width without being a run of one digit,
  /// so a field landing at the wrong offset shows up.
  std::string token(std::size_t nChars) {
    static const std::string digits = "0123456789ABCDEF";
    std::string out;
    for (std::size_t i = 0; i < nChars; ++i) { out += digits[(i * 7) % 16]; }
    return out;
  }

}

TEST(SpecRegistry, dictionaryIsNotEmpty) {
  // A failure here means no specs were found in the rootmap, and every other
  // test below would be skipped rather than failed.
  EXPECT_FALSE(specNames().empty()) << GlobalSim::knownSpecNames();
}

TEST(SpecRegistry, unknownNameIsRejected) {
  std::string error;
  EXPECT_EQ(GlobalSim::specEntry("no_such_spec", error), nullptr);
  EXPECT_FALSE(error.empty());
}

TEST(SpecRegistry, everyListedSpecBuilds) {
  for (const std::string& name : specNames()) {
    std::string error;
    const GlobalSim::SpecEntry* entry = GlobalSim::specEntry(name, error);
    ASSERT_NE(entry, nullptr) << name << ": " << error;
    EXPECT_GT(entry->width, 0u) << name;
    EXPECT_EQ(entry->width % 4, 0u) << name << " is not a whole number of hex "
                                    << "characters wide";
    EXPECT_TRUE(static_cast<bool>(entry->pack)) << name;
    EXPECT_TRUE(static_cast<bool>(entry->unpack)) << name;
  }
}

TEST(SpecRegistry, everyListedSpecRoundTrips) {
  for (const std::string& name : specNames()) {
    std::string error;
    const GlobalSim::SpecEntry* entry = GlobalSim::specEntry(name, error);
    ASSERT_NE(entry, nullptr) << name << ": " << error;

    // A spec need not declare a field for every bit -- topoc_pu_type leaves the
    // top 28 clear -- so an arbitrary token is not expected to come back
    // unchanged. What must hold is that one pass through the object settles it:
    // whatever the spec keeps of a token has to survive being written and read
    // again unchanged, otherwise a file this package writes cannot be read back.
    SG::AuxElement first;
    first.makePrivateStore();
    entry->unpack(first, token(entry->width / 4));
    const std::string once = entry->pack(first);

    SG::AuxElement second;
    second.makePrivateStore();
    entry->unpack(second, once);
    EXPECT_EQ(entry->pack(second), once) << name;

    EXPECT_EQ(once.size(), entry->width / 4) << name;
  }
}

TEST(SpecRegistry, entryIsCached) {
  const std::vector<std::string> names = specNames();
  ASSERT_FALSE(names.empty());

  std::string error;
  const GlobalSim::SpecEntry* first = GlobalSim::specEntry(names.front(), error);
  const GlobalSim::SpecEntry* again = GlobalSim::specEntry(names.front(), error);
  ASSERT_NE(first, nullptr) << error;
  EXPECT_EQ(first, again) << "the entry was built a second time";
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
