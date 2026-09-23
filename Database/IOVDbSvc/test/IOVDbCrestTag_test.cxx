/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file IOVDbSvc/test/IOVDbCrestTag_test.cxx
 * @brief Tests for IOVDbCrestTag against a local crest_fs CHAI database, in the
 *        Boost framework
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IOVDBSVC

#include <boost/test/unit_test.hpp>
//
#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unistd.h>
#include <vector>
//
#include <chai/Container.h>
#include <chai/Database.h>
#include <chai/PayloadSpec.h>
#include <chai/Tag.h>
#include <chai/VectorContainer.h>
//
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/IOpaqueAddress.h"
//
#include "../src/FolderTypes.h"
#include "../src/IOVDbParser.h"
#include "../src/IOVDbCrestTag.h"
#include "../src/IOVDbJsonStringFunctions.h"
//
#include "AthenaPoolUtilities/CondAttrListCollAddress.h"
//
#include "GaudiKernelFixtureBase.h"
//
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

namespace {
  // chai::Tag::buildNodeDescription's documented (typeName, clid) pairs
  constexpr uint32_t s_attrListClid = 40774348;
  constexpr const char* s_attrListTypeName = "AthenaAttributeList";
  constexpr uint32_t s_attrListCollClid = 1238547719;
  constexpr uint32_t s_attrListVecClid = 55403898;
  // A CREST bound above cool::ValidityKeyMax (2^63 - 1), which IOVTime rejects
  constexpr uint64_t s_beyondValidityKeyMax = static_cast<uint64_t>(cool::ValidityKeyMax) + 5;

  // Redirects stdout to a pipe for the scope's lifetime, so DEBUG-level
  // MsgStream output (printed to stdout) can be captured and checked.
  struct StdoutCapture{
    int savedFd{-1};
    int pipeFds[2]{-1,-1};
    StdoutCapture(){
      fflush(stdout);
      savedFd=dup(fileno(stdout));
      if (savedFd < 0 || pipe(pipeFds) != 0 || dup2(pipeFds[1], fileno(stdout)) < 0){
        throw std::runtime_error("IOVDbCrestTag_test: could not redirect stdout for capture");
      }
    }
    StdoutCapture(const StdoutCapture&) = delete;
    StdoutCapture& operator=(const StdoutCapture&) = delete;

    // Restoring here too matters: if the captured call throws, stdout stays
    // pointed at a pipe nobody drains, losing further Boost.Test output once
    // the pipe buffer fills.
    ~StdoutCapture(){
      restore();
    }

    std::string
    finish(){
      fflush(stdout);
      restore();
      std::string captured;
      std::array<char,4096> buf{};
      ssize_t n{};
      while ((n=read(pipeFds[0], buf.data(), buf.size()))>0){
        captured.append(buf.data(), static_cast<size_t>(n));
      }
      if (pipeFds[0] >= 0){
        close(pipeFds[0]);
        pipeFds[0] = -1;
      }
      return captured;
    }

  private:
    // Idempotent, so finish() and the destructor can both call it.
    void restore(){
      if (savedFd >= 0){
        dup2(savedFd, fileno(stdout));
        close(savedFd);
        savedFd = -1;
      }
      if (pipeFds[1] >= 0){
        close(pipeFds[1]);
        pipeFds[1] = -1;
      }
    }
  };
}

struct GaudiKernelFixture:public GaudiKernelFixtureBase{
  // Retrieved right after Gaudi starts, before any chai::Database exists:
  // retrieving it later makes Gaudi's plugin lookup for EventPersistencySvc
  // fail, for reasons not understood. Kept out of namespace scope since that
  // would construct it at static-init time, before the ApplicationMgr exists.
  static ServiceHandle<IAddressCreator> persistencySvc;
  GaudiKernelFixture():GaudiKernelFixtureBase(){
    static const bool retrieved = [](){
      if (persistencySvc.retrieve().isFailure()) {
        throw std::runtime_error(
          "EventPersistencySvc could not be retrieved in GaudiKernelFixture");
      }
      return true;
    }();
    (void)retrieved;
  }
};
ServiceHandle<IAddressCreator> GaudiKernelFixture::persistencySvc("EventPersistencySvc", "test");

struct MsgFixture{
  ServiceHandle<IMessageSvc> msgSvc;
  MsgStream log;
  MsgFixture():msgSvc("msgSvc","test"),
   log(msgSvc.get(), "IOVDbCrestTag_test"){
    log.setLevel(MSG::DEBUG);
  }
};

////////////////////////////////////////////////////////////////////////////////
// A local crest_fs database, wiped and repopulated fresh for every test case,
// with tags covering the timebase-correction matrix plus a two-IOV scalar
// tag for loadAt/isResident/getAddress coverage.
////////////////////////////////////////////////////////////////////////////////
// Derives from GaudiKernelFixture instead of relying on the outer suite's
// fixture: Boost.Test never constructs a suite's own fixture when the suite has
// no test cases of its own, and every test case here lives in this fixture's suite.
struct CrestFsFixture:public GaudiKernelFixture{
  // Owns the fixture's crest_fs directory and removes it in the destructor.
  // Declared before db so it is destroyed after it: the directory must outlive
  // the database reading out of it.
  struct TempDirGuard{
    std::string path;
    explicit TempDirGuard(std::string p):path(std::move(p)){}
    TempDirGuard(const TempDirGuard&) = delete;
    TempDirGuard& operator=(const TempDirGuard&) = delete;
    ~TempDirGuard(){
      std::error_code ec;
      std::filesystem::remove_all(path, ec);
    }
  };

  TempDirGuard dir{makeUniqueDir()};
  chai::Database db;

  CrestFsFixture():GaudiKernelFixture(),db("crest_fs:" + dir.path){
    createTags();
  }

  // mkdtemp keeps this fixture's directory from colliding with
  // another build's on the same node.
  static std::string makeUniqueDir(){
    std::string tmpl =
      (std::filesystem::temp_directory_path() / "IOVDbCrestTag_test_crest_fs_XXXXXX").string();
    std::vector<char> buffer(tmpl.begin(), tmpl.end());
    buffer.push_back('\0');
    if (mkdtemp(buffer.data()) == nullptr) {
      throw std::runtime_error(
        "IOVDbCrestTag_test: could not create a temporary crest_fs directory");
    }
    return std::string(buffer.data());
  }

  void createTags(){
    using enum chai::Type;
    chai::PayloadSpec spec(chai::FieldSpec({{"value", UInt32}}), chai::ChannelSpec({{0, ""}}));

    // Two IOVs, description's <timeStamp> already matches the tag's IovType
    // (the Unchanged case). Also used for loadAt/isResident/getAddress coverage.
    auto agreeing = db.createTag(
      "AttrListTag_Agreeing", "test tag, timebase agrees", spec,
      {.iovType = chai::Tag::IovType::RunNumberLumiBlock,
       .nodeDescription = chai::Tag::buildNodeDescription(
           chai::Tag::IovType::RunNumberLumiBlock, s_attrListTypeName, s_attrListClid)});
    auto c1 = agreeing->buildContainer();
    c1[0].push(1u);
    agreeing->addPayload(c1, 100, 200);
    auto c2 = agreeing->buildContainer();
    c2[0].push(2u);
    agreeing->addPayload(c2, 200);

    // IovType is RunNumberLumiBlock but the description's <timeStamp> says
    // "time": the Corrected case (the coolr-migration bug fingerprint)
    auto disagreeing = db.createTag(
      "AttrListTag_Disagreeing", "test tag, timebase disagrees", spec,
      {.iovType = chai::Tag::IovType::RunNumberLumiBlock,
       .nodeDescription = chai::Tag::buildNodeDescription(
           chai::Tag::IovType::Time, s_attrListTypeName, s_attrListClid)});
    auto c3 = disagreeing->buildContainer();
    c3[0].push(3u);
    disagreeing->addPayload(c3, 100, 200);

    // IovType this code doesn't understand at all: the same wrong-key
    // fingerprint as an explicit disagreement, refused the same way.
    auto unsupportedIovType = db.createTag(
      "AttrListTag_RunNumberIovType", "test tag, IovType this code can't map", spec,
      {.iovType = chai::Tag::IovType::RunNumber,
       .nodeDescription = chai::Tag::buildNodeDescription(
           chai::Tag::IovType::RunNumberLumiBlock, s_attrListTypeName, s_attrListClid)});
    auto c3b = unsupportedIovType->buildContainer();
    c3b[0].push(30u);
    unsupportedIovType->addPayload(c3b, 100, 200);

    // An IOV whose until exceeds cool::ValidityKeyMax: CREST bounds are 64-bit,
    // COOL keys are 63-bit. Two IOVs, so the first one's until is the oversized
    // value rather than open-ended.
    auto untilBeyondMax = db.createTag(
      "AttrListTag_UntilBeyondMax", "test tag, until above ValidityKeyMax", spec,
      {.iovType = chai::Tag::IovType::RunNumberLumiBlock,
       .nodeDescription = chai::Tag::buildNodeDescription(
           chai::Tag::IovType::RunNumberLumiBlock, s_attrListTypeName, s_attrListClid)});
    auto cBeyond = untilBeyondMax->buildContainer();
    cBeyond[0].push(7u);
    untilBeyondMax->addPayload(cBeyond, 100, s_beyondValidityKeyMax);
    auto cBeyond2 = untilBeyondMax->buildContainer();
    cBeyond2[0].push(8u);
    untilBeyondMax->addPayload(cBeyond2, s_beyondValidityKeyMax);

    // A timestamp-indexed tag, description in agreement. IOVs are in
    // nanoseconds, as CREST stores this axis.
    auto timeIndexed = db.createTag(
      "AttrListTag_TimeIndexed", "test tag, timestamp-indexed", spec,
      {.iovType = chai::Tag::IovType::Time,
       .nodeDescription = chai::Tag::buildNodeDescription(
           chai::Tag::IovType::Time, s_attrListTypeName, s_attrListClid)});
    // Two IOVs, as for the run-lumi fixture: with only one, the last (and
    // only) IOV is open-ended, so nothing would pin the upper bound.
    auto cTime = timeIndexed->buildContainer();
    cTime[0].push(5u);
    timeIndexed->addPayload(cTime, 1'600'000'000'000'000'000, 1'700'000'000'000'000'000);
    auto cTime2 = timeIndexed->buildContainer();
    cTime2[0].push(6u);
    timeIndexed->addPayload(cTime2, 1'700'000'000'000'000'000);

    // No <timeStamp> element at all: the Inserted case.
    auto noTimestamp = db.createTag(
      "AttrListTag_NoTimeStamp", "test tag, timebase absent", spec,
      {.iovType = chai::Tag::IovType::RunNumberLumiBlock,
       .nodeDescription = "<addrHeader><address_header service_type=\"71\" clid=\"" +
                           std::to_string(s_attrListClid) + "\" /></addrHeader><typeName>" +
                           s_attrListTypeName + "</typeName>"});
    auto c4 = noTimestamp->buildContainer();
    c4[0].push(4u);
    noTimestamp->addPayload(c4, 100, 200);

    // Two named channels: exercises getAddress's AttrListColl path plus the
    // m_named channel attachment (buildNodeDescription() has no <named/> param).
    chai::PayloadSpec collSpec(chai::FieldSpec({{"value", UInt32}}),
                                chai::ChannelSpec({{0, "chanA"}, {1, "chanB"}}));
    auto coll = db.createTag(
      "AttrListCollTag_Named", "test tag, named multi-channel collection", collSpec,
      {.iovType = chai::Tag::IovType::RunNumberLumiBlock,
       .nodeDescription = chai::Tag::buildNodeDescription(
                             chai::Tag::IovType::RunNumberLumiBlock, "CondAttrListCollection",
                             s_attrListCollClid)
                           + "<named/>"});
    // CREST only honors addPayload()'s until as the tag's final end time: a
    // single-IOV tag comes back open-ended regardless of it, so a second, later
    // payload is what actually bounds the first at 400.
    auto cc1 = coll->buildContainer();
    cc1[0].push(10u);
    cc1[1].push(20u);
    coll->addPayload(cc1, 300, 400);
    auto cc2 = coll->buildContainer();
    cc2[0].push(11u);
    cc2[1].push(21u);
    coll->addPayload(cc2, 400);

    // Single channel, field "PoolRef" of type String.
    chai::PayloadSpec poolRefSpec(chai::FieldSpec({{"PoolRef", String}}), chai::ChannelSpec({{0, ""}}));
    auto poolRef = db.createTag(
      "PoolRefTag", "test tag, single-channel PoolRef", poolRefSpec,
      {.iovType = chai::Tag::IovType::RunNumberLumiBlock,
       .nodeDescription = chai::Tag::buildNodeDescription(
                             chai::Tag::IovType::RunNumberLumiBlock, s_attrListTypeName,
                             s_attrListClid)});
    auto pr1 = poolRef->buildContainer();
    pr1[0].push(std::string("[DB=test][CNT=test][CLID=abc][TECH=00000202][OID=00000000-00000000]"));
    poolRef->addPayload(pr1, 500, 600);

    // Same PoolRef field as above, on two channels: PoolRefColl instead of PoolRef.
    chai::PayloadSpec poolRefCollSpec(chai::FieldSpec({{"PoolRef", String}}),
                                       chai::ChannelSpec({{0, ""}, {1, ""}}));
    auto poolRefColl = db.createTag(
      "PoolRefCollTag", "test tag, multi-channel PoolRef collection", poolRefCollSpec,
      {.iovType = chai::Tag::IovType::RunNumberLumiBlock,
       .nodeDescription = chai::Tag::buildNodeDescription(
                             chai::Tag::IovType::RunNumberLumiBlock, s_attrListTypeName,
                             s_attrListClid)});
    auto prc1 = poolRefColl->buildContainer();
    prc1[0].push(
      std::string("[DB=test][CNT=chan0][CLID=abc][TECH=00000202][OID=00000000-00000000]"));
    prc1[1].push(
      std::string("[DB=test][CNT=chan1][CLID=abc][TECH=00000202][OID=00000000-00000000]"));
    poolRefColl->addPayload(prc1, 500, 600);

    // Vector payload, three rows on one channel: exercises getAddress's
    // CoolVector path and the typeName/isVectorPayload() cross-check in preload().
    chai::PayloadSpec vecSpec(chai::FieldSpec({{"value", UInt32}}), chai::ChannelSpec({{0, ""}}));
    auto vec = db.createTag(
      "CoolVectorTag", "test tag, vector payload multi-row", vecSpec,
      {.iovType = chai::Tag::IovType::RunNumberLumiBlock,
       .nodeDescription = chai::Tag::buildNodeDescription(
                             chai::Tag::IovType::RunNumberLumiBlock, "CondAttrListVec",
                             s_attrListVecClid, 256)});
    auto vc = vec->buildVectorContainer();
    std::vector<chai::ValuesPtr> rows;
    for (uint32_t v : {10u, 20u, 30u}) {
      auto row = std::make_shared<chai::Values>(vecSpec.fields());
      row->push(v);
      rows.push_back(std::move(row));
    }
    vc.setRows(0, rows);
    vec->addPayload(vc, 700, 800);
    // A second, later payload bounds the first at 800: addPayload()'s until
    // applies to the whole tag, not per IOV (see AttrListCollTag_Named above).
    auto vc2 = vec->buildVectorContainer();
    std::vector<chai::ValuesPtr> rows2;
    for (uint32_t v : {40u, 50u}) {
      auto row = std::make_shared<chai::Values>(vecSpec.fields());
      row->push(v);
      rows2.push_back(std::move(row));
    }
    vc2.setRows(0, rows2);
    vec->addPayload(vc2, 800);
  }
};

BOOST_FIXTURE_TEST_SUITE(IOVDbCrestTagTest, GaudiKernelFixture)
  MsgFixture msgFixture;

  BOOST_FIXTURE_TEST_SUITE(IOVDbCrestTagMethods, CrestFsFixture)

    // preload() must FATAL and return nullptr (not throw or touch CHAI) when
    // the folder has no CREST tag name and isn't served from file metadata.
    BOOST_AUTO_TEST_CASE(preload_emptyTagFatal){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_TEST(addr == nullptr);
    }

    // preload() must FATAL (and return nullptr) when the named CREST tag
    // cannot be resolved, rather than letting the exception propagate
    BOOST_AUTO_TEST_CASE(preload_unknownTagFatal){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "NoSuchTagExists");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_TEST(addr == nullptr);
    }

    // joTag() plays no part in resolving the CREST tag. It only records an
    // explicit <tag>/<ctag> so fillTagInfo() can write it into /TagInfo.
    BOOST_AUTO_TEST_CASE(joTag_empty_whenNeitherTagNorCtagGiven){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Agreeing");
      BOOST_TEST(tag.joTag().empty());
    }

    BOOST_AUTO_TEST_CASE(joTag_fromExplicitTagElement){
      IOVDbParser folderprop("/test/folder<tag>MyExplicitTag</tag>", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Agreeing");
      BOOST_TEST(tag.joTag() == "MyExplicitTag");
    }

    BOOST_AUTO_TEST_CASE(joTag_fromCtagElement_whenNoTagElement){
      IOVDbParser folderprop("/test/folder<ctag>MyCrestOnlyTag</ctag>", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Agreeing");
      BOOST_TEST(tag.joTag() == "MyCrestOnlyTag");
    }

    // <tag> takes precedence, matching how the constructor already treats
    // <tag> as the one to warn about (see the INFO log) when both are given.
    BOOST_AUTO_TEST_CASE(joTag_prefersTagElement_whenBothGiven){
      IOVDbParser folderprop("/test/folder<tag>FromTag</tag><ctag>FromCtag</ctag>", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Agreeing");
      BOOST_TEST(tag.joTag() == "FromTag");
    }

    BOOST_AUTO_TEST_CASE(preload_timebaseAgrees_noCorrection){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Agreeing");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.timeStamp() == false);  // Run-lumi
      BOOST_TEST(tag.folderType() == IOVDbNamespace::AttrList);
      BOOST_TEST(tag.clid() == static_cast<CLID>(s_attrListClid));
    }

    // The disagreeing tag's timeStamp mismatches its IovType (the
    // coolr-migration mis-key fingerprint): preload() must FATAL and return
    // nullptr rather than correct it.
    BOOST_AUTO_TEST_CASE(preload_timebaseDisagrees_fatals){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Disagreeing");
      StdoutCapture capture;
      auto addr = tag.preload(nullptr, 0, 0);
      const std::string captured = capture.finish();
      BOOST_TEST(addr == nullptr);
      // Pin the reason: a nullptr alone could also come from any other
      // preload() failure on the same tag.
      BOOST_REQUIRE(!captured.empty());
      BOOST_TEST(captured.find("FATAL") != std::string::npos);
      BOOST_TEST(captured.find("disagrees with the tag's IovType") != std::string::npos);
    }

    // An IovType this code has no mapping for (RunNumber, Unspecified) must
    // also FATAL rather than keying the tag off the description's own timebase.
    BOOST_AUTO_TEST_CASE(preload_timebaseUnsupportedIovType_fatals){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db,
                        "AttrListTag_RunNumberIovType");
      StdoutCapture capture;
      auto addr = tag.preload(nullptr, 0, 0);
      const std::string captured = capture.finish();
      BOOST_TEST(addr == nullptr);
      BOOST_REQUIRE(!captured.empty());
      BOOST_TEST(captured.find("FATAL") != std::string::npos);
      BOOST_TEST(captured.find("has unsupported IovType") != std::string::npos);
    }

    // Positive counterpart to the run-lumi cases: a timestamp-indexed tag
    // keys on the timestamp axis and resolves payloads with nanosecond keys.
    BOOST_AUTO_TEST_CASE(preload_timebaseTimeIndexed_keysOnTimestamp){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_TimeIndexed");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.timeStamp() == true);
      BOOST_TEST(tag.folderType() == IOVDbNamespace::AttrList);

      BOOST_TEST(tag.loadAt(1'650'000'000'000'000'000) == true);
      BOOST_TEST(tag.isResident(1'650'000'000'000'000'000) == true);
      // The resident IOV is half-open on the timestamp axis just as it is on
      // run-lumi: its own start counts as covered, the next IOV's start does
      // not.
      BOOST_TEST(tag.isResident(1'600'000'000'000'000'000) == true);
      BOOST_TEST(tag.isResident(1'700'000'000'000'000'000) == false);
    }

    // A missing <timeStamp> element gets one inserted from the tag's IovType
    BOOST_AUTO_TEST_CASE(preload_timebaseAbsent_insertedFromIovType){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_NoTimeStamp");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.timeStamp() == false);
    }

    // isResident() treats since as inclusive and until as exclusive: reftime
    // on the IOV's own start counts as resident.
    BOOST_AUTO_TEST_CASE(isResident_boundary){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Agreeing");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.isResident(150) == false);  // Cache empty before any loadAt
      BOOST_TEST(tag.loadAt(150) == true);
      BOOST_TEST(tag.isResident(100) == true);   // == cacheStart: resident
      BOOST_TEST(tag.isResident(199) == true);
      BOOST_TEST(tag.isResident(200) == false);  // == cacheStop: open interval
      BOOST_TEST(tag.isResident(99) == false);
    }

    // A CREST until above cool::ValidityKeyMax is clamped to it (with a
    // WARNING) instead of becoming an invalid IOVTime. The resident then runs
    // to the last valid key, half-open as always.
    BOOST_AUTO_TEST_CASE(loadAt_untilBeyondValidityKeyMax_clampedWithWarning){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db,
                        "AttrListTag_UntilBeyondMax");
      BOOST_REQUIRE(tag.preload(nullptr, 0, 0) != nullptr);
      StdoutCapture capture;
      const bool loaded = tag.loadAt(150);
      const std::string captured = capture.finish();
      BOOST_REQUIRE(loaded);
      BOOST_REQUIRE(!captured.empty());
      BOOST_TEST(captured.find("exceeds cool::ValidityKeyMax; clamped") != std::string::npos);
      BOOST_TEST(tag.isResident(100) == true);
      BOOST_TEST(tag.isResident(cool::ValidityKeyMax - 1) == true);
      BOOST_TEST(tag.isResident(cool::ValidityKeyMax) == false);
      std::unique_ptr<IOpaqueAddress> returnedAddress;
      IOVRange range;
      bool poolPayloadRequested = false;
      BOOST_REQUIRE(tag.getAddress(150, &(*GaudiKernelFixture::persistencySvc), 0,
                                   returnedAddress, range, poolPayloadRequested));
      BOOST_TEST(range.start().re_time() == 100ull);
      BOOST_TEST(range.stop().re_time() == static_cast<unsigned long long>(cool::ValidityKeyMax));
      BOOST_TEST(range.stop().isValid());
    }

    // A vkey before the tag's first IOV must leave loadAt() returning false
    // with the cache reset, not throw or leave stale state behind
    BOOST_AUTO_TEST_CASE(loadAt_beforeFirstIov_returnsFalseAndResets){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Agreeing");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.loadAt(150) == true);
      BOOST_TEST(tag.isResident(150) == true);
      BOOST_TEST(tag.loadAt(0) == false);
      BOOST_TEST(tag.isResident(150) == false);
    }

    BOOST_AUTO_TEST_CASE(getAddress_scalarHappyPath){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Agreeing");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.loadAt(150) == true);
      std::unique_ptr<IOpaqueAddress> returnedAddress;
      IOVRange range;
      bool poolPayloadRequested = false;
      BOOST_TEST(tag.getAddress(150, &(*GaudiKernelFixture::persistencySvc), 0,
                                returnedAddress, range, poolPayloadRequested));
      BOOST_TEST(tag.retrieved() == true);
      BOOST_TEST(range.start().re_time() == 100ull);
      BOOST_TEST(range.stop().re_time() == 200ull);
    }

    // getAddress() must return false, not throw, when called with no resident
    // payload: the state loadAt() leaves behind after a failed fetch.
    BOOST_AUTO_TEST_CASE(getAddress_noResident_fails){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Agreeing");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.loadAt(0) == false);  // Before the tag's first IOV
      std::unique_ptr<IOpaqueAddress> returnedAddress;
      IOVRange range;
      bool poolPayloadRequested = false;
      BOOST_TEST(!tag.getAddress(0, &(*GaudiKernelFixture::persistencySvc), 0,
                                 returnedAddress, range, poolPayloadRequested));
    }

    // Two getAddress() calls on the same resident each build an independent
    // object. Only the loadAt() that fetched it counts bytes.
    BOOST_AUTO_TEST_CASE(getAddress_calledTwice_bytesCountedOnce){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Agreeing");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.loadAt(150) == true);
      const auto bytesAfterLoad = tag.bytesRead();
      BOOST_TEST(bytesAfterLoad > 0u);

      std::unique_ptr<IOpaqueAddress> firstAddress;
      std::unique_ptr<IOpaqueAddress> secondAddress;
      IOVRange range;
      bool poolPayloadRequested = false;
      BOOST_TEST(tag.getAddress(150, &(*GaudiKernelFixture::persistencySvc), 0,
                                firstAddress, range, poolPayloadRequested));
      BOOST_TEST(tag.getAddress(150, &(*GaudiKernelFixture::persistencySvc), 0,
                                secondAddress, range, poolPayloadRequested));
      BOOST_TEST(tag.bytesRead() == bytesAfterLoad);
      // cppcheck-suppress mismatchingContainers
      // Two unrelated unique_ptrs, not iterators of one container. cppcheck
      // mistakes .get() != .get() for the iterator-comparison bug pattern.
      const bool independentObjects = (firstAddress.get() != secondAddress.get());
      BOOST_TEST(independentObjects);
    }

    // The "Retrieved object" line is a parse anchor for
    // python/getProblemFolderFromLogs.py and must not be reworded
    BOOST_AUTO_TEST_CASE(getAddress_parseAnchorText){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Agreeing");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.loadAt(150) == true);
      std::unique_ptr<IOpaqueAddress> returnedAddress;
      IOVRange range;
      bool poolPayloadRequested = false;
      StdoutCapture capture;
      const bool ok = tag.getAddress(150, &(*GaudiKernelFixture::persistencySvc), 0,
                                     returnedAddress, range, poolPayloadRequested);
      const std::string captured = capture.finish();
      BOOST_TEST(ok);
      BOOST_TEST(captured.find(
        "Retrieved object: folder /test/folder at IOV 150 channels 1 has range") !=
                 std::string::npos);
    }

    // Named multi-channel collection: getAddress must assemble a
    // CondAttrListCollection over both channels without error
    BOOST_AUTO_TEST_CASE(getAddress_namedCollection){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListCollTag_Named");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.folderType() == IOVDbNamespace::AttrListColl);
      BOOST_TEST(tag.loadAt(350) == true);
      std::unique_ptr<IOpaqueAddress> returnedAddress;
      IOVRange range;
      bool poolPayloadRequested = false;
      BOOST_TEST(tag.getAddress(350, &(*GaudiKernelFixture::persistencySvc), 0,
                                returnedAddress, range, poolPayloadRequested));
      BOOST_TEST(tag.retrieved() == true);
      BOOST_TEST(range.start().re_time() == 300ull);
      BOOST_TEST(range.stop().re_time() == 400ull);
    }

    // channelSelection restricts getAddress()'s output to the selected
    // channels. It is a job-option modifier embedded in the folder path here.
    BOOST_AUTO_TEST_CASE(getAddress_channelSelection_filtersToSelectedChannel){
      IOVDbParser folderprop("/test/folder<channelSelection>0:0</channelSelection>", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListCollTag_Named");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.loadAt(350) == true);
      std::unique_ptr<IOpaqueAddress> returnedAddress;
      IOVRange range;
      bool poolPayloadRequested = false;
      BOOST_TEST(tag.getAddress(350, &(*GaudiKernelFixture::persistencySvc), 0,
                                returnedAddress, range, poolPayloadRequested));
      auto* collAddr = dynamic_cast<CondAttrListCollAddress*>(returnedAddress.get());
      BOOST_REQUIRE(collAddr != nullptr);
      CondAttrListCollection* coll = collAddr->attrListColl();
      BOOST_REQUIRE(coll != nullptr);
      // Fixture has two channels (0 and 1). Only channel 0 is selected.
      BOOST_TEST(coll->size() == 1u);
      const bool hasChannelZero = (coll->chanAttrListPair(0) != coll->end());
      BOOST_TEST(hasChannelZero);
    }

    // A vkey before a COLLECTION-type folder's first IOV returns true with an
    // empty but resident cache. The message is a WARNING, not an ERROR, so
    // Reco_tf.py's log-scanner does not treat it as fatal.
    BOOST_AUTO_TEST_CASE(loadAt_beforeFirstIov_collectionType_degradesGracefully){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListCollTag_Named");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.folderType() == IOVDbNamespace::AttrListColl);
      StdoutCapture capture;
      const bool loaded = tag.loadAt(100);  // Tag's first IOV starts at 300
      const std::string captured = capture.finish();
      BOOST_TEST(loaded == true);
      BOOST_TEST(tag.isResident(100) == true);
      BOOST_TEST(tag.isResident(299) == true);
      BOOST_TEST(tag.isResident(300) == false);  // Real coverage starts here
      // Require some output first: an empty capture would satisfy the
      // negative check below without proving anything.
      BOOST_REQUIRE(!captured.empty());
      BOOST_TEST(captured.find("WARNING") != std::string::npos);
      BOOST_TEST(captured.find("ERROR") == std::string::npos);
    }

    // getAddress() on the empty resident above must build an empty
    // CondAttrListCollection (0 channels) rather than fail.
    BOOST_AUTO_TEST_CASE(getAddress_emptyResident_collectionType_buildsEmptyCollection){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListCollTag_Named");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.loadAt(100) == true);
      std::unique_ptr<IOpaqueAddress> returnedAddress;
      IOVRange range;
      bool poolPayloadRequested = false;
      BOOST_TEST(tag.getAddress(100, &(*GaudiKernelFixture::persistencySvc), 0,
                                returnedAddress, range, poolPayloadRequested));
      BOOST_TEST(tag.retrieved() == true);
      BOOST_TEST(range.start().re_time() == 0ull);    // cool::ValidityKeyMin
      BOOST_TEST(range.stop().re_time() == 300ull);   // Tag's real first IOV
      BOOST_REQUIRE(returnedAddress != nullptr);
    }

    // Single-channel PoolRef: determineFolderType()'s field-based inference and
    // getAddress()'s PoolRef branch.
    BOOST_AUTO_TEST_CASE(getAddress_poolRef){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "PoolRefTag");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.folderType() == IOVDbNamespace::PoolRef);
      BOOST_TEST(tag.loadAt(550) == true);
      std::unique_ptr<IOpaqueAddress> returnedAddress;
      IOVRange range;
      bool poolPayloadRequested = false;
      BOOST_TEST(tag.getAddress(550, &(*GaudiKernelFixture::persistencySvc), 0,
                                returnedAddress, range, poolPayloadRequested));
      BOOST_TEST(poolPayloadRequested == true);
    }

    // Multi-channel counterpart to getAddress_poolRef: folderType comes out
    // PoolRefColl instead of PoolRef.
    BOOST_AUTO_TEST_CASE(getAddress_poolRefColl){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "PoolRefCollTag");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.folderType() == IOVDbNamespace::PoolRefColl);
      BOOST_TEST(tag.loadAt(550) == true);
      std::unique_ptr<IOpaqueAddress> returnedAddress;
      IOVRange range;
      bool poolPayloadRequested = false;
      BOOST_TEST(tag.getAddress(550, &(*GaudiKernelFixture::persistencySvc), 0,
                                returnedAddress, range, poolPayloadRequested));
      BOOST_TEST(poolPayloadRequested == true);
    }

    // Vector payload, three rows on one channel: getAddress()'s CoolVector
    // branch against the crest_fs vector fixture
    BOOST_AUTO_TEST_CASE(getAddress_vectorMultiRow){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "CoolVectorTag");
      auto addr = tag.preload(nullptr, 0, 0);
      BOOST_REQUIRE(addr != nullptr);
      BOOST_TEST(tag.folderType() == IOVDbNamespace::CoolVector);
      BOOST_TEST(tag.loadAt(750) == true);
      std::unique_ptr<IOpaqueAddress> returnedAddress;
      IOVRange range;
      bool poolPayloadRequested = false;
      BOOST_TEST(tag.getAddress(750, &(*GaudiKernelFixture::persistencySvc), 0,
                                returnedAddress, range, poolPayloadRequested));
      BOOST_TEST(tag.retrieved() == true);
      BOOST_TEST(range.start().re_time() == 700ull);
      BOOST_TEST(range.stop().re_time() == 800ull);
    }

    // dumpChannelsAsJson feeds cool_crest_compare (not itself a ctest), and
    // stripIovBounds is written against its output shape.
    BOOST_AUTO_TEST_CASE(dumpChannelsAsJson_scalarEmitsBoundsAndPayload){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Agreeing");
      BOOST_REQUIRE(tag.preload(nullptr, 0, 0) != nullptr);
      BOOST_REQUIRE(tag.loadAt(150));
      const std::string dumped = tag.dumpChannelsAsJson(150);
      BOOST_TEST(dumped.find("\"since\" : 100") != std::string::npos);
      BOOST_TEST(dumped.find("\"until\" : 200") != std::string::npos);
      BOOST_TEST(dumped.find("\"payload\" : ") != std::string::npos);
      // The bound-stripping the compare tool applies has to bite on this
      BOOST_TEST(IOVDbNamespace::stripIovBounds(dumped).find("\"since\"") == std::string::npos);
      BOOST_TEST(IOVDbNamespace::stripIovBounds(dumped).find("\"payload\"") != std::string::npos);
    }

    BOOST_AUTO_TEST_CASE(dumpChannelsAsJson_vectorEmitsEveryChannel){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "CoolVectorTag");
      BOOST_REQUIRE(tag.preload(nullptr, 0, 0) != nullptr);
      BOOST_REQUIRE(tag.loadAt(750));
      const std::string dumped = tag.dumpChannelsAsJson(750);
      BOOST_TEST(dumped.find("\"payload\" : ") != std::string::npos);
      BOOST_TEST(IOVDbNamespace::stripIovBounds(dumped).find("\"until\"") == std::string::npos);
    }

    // With nothing resident at the requested time the dump is the empty
    // array, not a stale payload left over from an earlier load.
    BOOST_AUTO_TEST_CASE(dumpChannelsAsJson_notResidentIsEmpty){
      IOVDbParser folderprop("/test/folder", msgFixture.log);
      IOVDbCrestTag tag(nullptr, folderprop, msgFixture.log, nullptr, nullptr, db, "AttrListTag_Agreeing");
      BOOST_REQUIRE(tag.preload(nullptr, 0, 0) != nullptr);
      BOOST_TEST(tag.dumpChannelsAsJson(150) == "[]");
    }

  BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
