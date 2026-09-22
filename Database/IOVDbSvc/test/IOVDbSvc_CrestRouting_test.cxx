/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file IOVDbSvc/test/IOVDbSvc_CrestRouting_test.cxx
 * @brief Service-level tests of IOVDbSvc::setupFolders' CREST routing against
 *        local crest_fs fixtures
 *
 * Each scenario is a separately named service instance ("IOVDbSvc/<Scenario>"),
 * since a Gaudi service initializes only once and the fixture directories are
 * created at run time rather than fixed in a job-options file.
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
#include <chai/GlobalTag.h>
#include <chai/PayloadSpec.h>
#include <chai/Tag.h>
//
#include "Gaudi/Interfaces/IOptionsSvc.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SmartIF.h"
//
#include "AthenaKernel/ExtendedEventContext.h"
#include "AthenaKernel/IAddressProvider.h"
#include "AthenaKernel/IIOVDbSvc.h"
#include "AthenaKernel/IOVRange.h"
#include "AthenaKernel/IOVTime.h"
#include "SGTools/TransientAddress.h"
//
#include "GaudiKernelFixtureBase.h"
//
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

namespace {
  // chai::Tag::buildNodeDescription's documented (typeName, clid) pair for a
  // single-channel AthenaAttributeList folder
  constexpr uint32_t s_attrListClid = 40774348;
  constexpr const char* s_attrListTypeName = "AthenaAttributeList";
  constexpr const char* s_globalTag = "TEST-ROUTING";

  // Redirect stdout to a pipe for the scope's duration so MsgStream output,
  // which the real message service prints to stdout, can be inspected.
  struct StdoutCapture{
    int savedFd{-1};
    int pipeFds[2]{-1,-1};
    StdoutCapture(){
      fflush(stdout);
      savedFd=dup(fileno(stdout));
      if (savedFd < 0 || pipe(pipeFds) != 0 || dup2(pipeFds[1], fileno(stdout)) < 0){
        throw std::runtime_error(
          "IOVDbSvc_CrestRouting_test: could not redirect stdout for capture");
      }
    }
    StdoutCapture(const StdoutCapture&) = delete;
    StdoutCapture& operator=(const StdoutCapture&) = delete;
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

  size_t countOccurrences(const std::string& text, const std::string& needle){
    size_t count = 0;
    for (auto pos = text.find(needle); pos != std::string::npos;
         pos = text.find(needle, pos + needle.size())){
      ++count;
    }
    return count;
  }

  // Owns a temporary crest_fs directory and removes it again. Declared
  // before the chai::Database that reads out of it so it outlives that.
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

  std::string makeUniqueDir(const std::string& stem){
    std::string tmpl =
      (std::filesystem::temp_directory_path() /
       ("IOVDbSvc_CrestRouting_test_" + stem + "_XXXXXX")).string();
    std::vector<char> buffer(tmpl.begin(), tmpl.end());
    buffer.push_back('\0');
    if (mkdtemp(buffer.data()) == nullptr) {
      throw std::runtime_error(
        "IOVDbSvc_CrestRouting_test: could not create a temporary crest_fs directory");
    }
    return std::string(buffer.data());
  }

  // One crest_fs endpoint: a directory, a database on it, and the helpers
  // to populate it with single-channel run-lumi tags and one global tag.
  struct CrestFsEndpoint{
    TempDirGuard dir;
    chai::Database db;
    chai::GlobalTagPtr globalTag;

    explicit CrestFsEndpoint(const std::string& stem):
      dir(makeUniqueDir(stem)),
      db("crest_fs:" + dir.path),
      globalTag(db.createGlobalTag(s_globalTag, "IOVDbSvc CREST routing test", "test")){
    }

    // Builds a run-lumi tag: value over [since, until), then a second,
    // open-ended IOV from until on. CREST always reports a tag's last IOV as
    // open-ended, so the second payload is what pins the first one's until.
    void addScalarTag(const std::string& name, uint32_t value, uint64_t since, uint64_t until){
      using enum chai::Type;
      chai::PayloadSpec spec(chai::FieldSpec({{"value", UInt32}}), chai::ChannelSpec({{0, ""}}));
      auto tag = db.createTag(
        name, "routing test tag " + name, spec,
        {.iovType = chai::Tag::IovType::RunNumberLumiBlock,
         .nodeDescription = chai::Tag::buildNodeDescription(
             chai::Tag::IovType::RunNumberLumiBlock, s_attrListTypeName, s_attrListClid)});
      auto first = tag->buildContainer();
      first[0].push(value);
      tag->addPayload(first, since, until);
      auto second = tag->buildContainer();
      second[0].push(value + 100u);
      tag->addPayload(second, until);
    }
  };

  // Property values in the form the JobOptionsSvc parses
  std::string quoted(const std::string& value){
    return "\"" + value + "\"";
  }

  std::string stringList(const std::vector<std::string>& values){
    std::string out{"["};
    std::string sep;
    for (const auto& value : values){
      out += sep + quoted(value);
      sep = ", ";
    }
    return out + "]";
  }
}

struct GaudiKernelFixture:public GaudiKernelFixtureBase{
  // Retrieved once, right after Gaudi comes up and before any chai::Database is
  // constructed or destroyed: retrieving it later breaks Gaudi's plugin lookup
  // for EventPersistencySvc, for reasons not understood. Cannot live at
  // namespace scope either, since that constructs at static-init time, before
  // the ApplicationMgr exists.
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

////////////////////////////////////////////////////////////////////////////////
// Two crest_fs endpoints share one global tag name whose mappings deliberately
// disagree: each endpoint also maps the OTHER endpoint's folder, to a decoy tag
// with a different IOV. A folder resolved through the wrong endpoint's mapping
// fails or reports the decoy. Built once, after Gaudi is up.
////////////////////////////////////////////////////////////////////////////////
struct RoutingFixture:public GaudiKernelFixture{
  struct Endpoints{
    CrestFsEndpoint a{"A"};
    CrestFsEndpoint b{"B"};
    Endpoints(){
      a.addScalarTag("TagA", 1u, 100, 200);
      a.addScalarTag("TagC", 3u, 100, 200);          // reachable only via <ctag>, never mapped
      a.addScalarTag("DecoyForB_onA", 9u, 900, 1000);
      a.globalTag->addTag("/Crest/A", "TagA");
      a.globalTag->addTag("/Crest/B", "DecoyForB_onA");

      b.addScalarTag("TagB", 2u, 300, 400);
      b.addScalarTag("DecoyForA_onB", 9u, 900, 1000);
      b.globalTag->addTag("/Crest/B", "TagB");
      b.globalTag->addTag("/Crest/A", "DecoyForA_onB");
    }
  };

  static Endpoints& endpoints(){
    static Endpoints instance;
    return instance;
  }

  ServiceHandle<Gaudi::Interfaces::IOptionsSvc> jobOptions{"JobOptionsSvc", "test"};

  RoutingFixture():GaudiKernelFixture(){
    if (jobOptions.retrieve().isFailure()){
      throw std::runtime_error("JobOptionsSvc could not be retrieved in RoutingFixture");
    }
    endpoints();
  }

  // Configure one named IOVDbSvc instance for a CREST-sourced job. Folders
  // are given as the raw Folders-property entries.
  void configureCrestInstance(const std::string& instance, const std::string& dbConnection,
                              const std::vector<std::string>& folders, bool onlineMode = false){
    jobOptions->set(instance + ".Source", quoted("CREST"));
    jobOptions->set(instance + ".GlobalTag", quoted(s_globalTag));
    jobOptions->set(instance + ".dbConnection", quoted(dbConnection));
    jobOptions->set(instance + ".Folders", stringList(folders));
    jobOptions->set(instance + ".OnlineMode", onlineMode ? "True" : "False");
    // INFO output is needed to see the TagInfo bookkeeping lines
    jobOptions->set(instance + ".OutputLevel", "3");
  }

  static IOVTime runLumi(uint32_t run, uint32_t lumi){
    return IOVTime(run, lumi);
  }
};

BOOST_FIXTURE_TEST_SUITE(IOVDbSvcCrestRoutingTest, RoutingFixture)

  // Exercises every routing path setupFolders supports: the job endpoint, a
  // second endpoint via <db>, a <ctag> bypass, and metadata-only folders.
  BOOST_AUTO_TEST_CASE(setupFolders_resolvesEachFolderOnItsOwnEndpoint){
    const auto& ep = endpoints();
    configureCrestInstance("CrestRouting", ep.a.dir.path, {
      "/TagInfo<metaOnly/>",
      "/Crest/A",
      "/Crest/B<db>crest_fs:" + ep.b.dir.path + "</db>",
      "/Crest/C<ctag>TagC</ctag>",
      "/Crest/MetaOnly<metaOnly/>",
    });
    ServiceHandle<IIOVDbSvc> iovdbsvc("IOVDbSvc/CrestRouting", "test");
    BOOST_REQUIRE(iovdbsvc.retrieve().isSuccess());

    // Preloading resolves every CHAI tag, drops the metadata-only folders
    // (no input file carries them here), and fills TagInfo
    SmartIF<IAddressProvider> provider(iovdbsvc.get());
    BOOST_REQUIRE(provider.isValid());
    IAddressProvider::tadList tlist;
    StdoutCapture capture;
    const bool preloaded = provider->preLoadAddresses(StoreID::DETECTOR_STORE, tlist).isSuccess();
    const std::string captured = capture.finish();
    BOOST_REQUIRE(preloaded);
    BOOST_TEST(tlist.size() == 3u);
    // With a GlobalTag set, every folder's stale input-file tag is removed
    // from the propagated TagInfo, CREST folders included.
    BOOST_REQUIRE(!captured.empty());
    BOOST_TEST(captured.find("Added taginfo remove for /Crest/A") != std::string::npos);
    BOOST_TEST(captured.find("Added taginfo remove for /Crest/B") != std::string::npos);
    BOOST_TEST(captured.find("Added taginfo remove for /Crest/C") != std::string::npos);
    for (auto* tad : tlist){
      delete tad;
    }

    const auto keys = iovdbsvc->getKeyList();
    BOOST_TEST(keys.size() == 3u);

    IIOVDbSvc::KeyInfo info;
    BOOST_REQUIRE(iovdbsvc->getKeyInfo("/Crest/A", info));
    BOOST_TEST(info.tag == "TagA");
    BOOST_REQUIRE(iovdbsvc->getKeyInfo("/Crest/B", info));
    BOOST_TEST(info.tag == "TagB");   // endpoint B's mapping, not A's "TagOnlyInMappingA"
    BOOST_REQUIRE(iovdbsvc->getKeyInfo("/Crest/C", info));
    BOOST_TEST(info.tag == "TagC");   // the <ctag> override, never looked up

    // And the routed folders actually deliver their own endpoint's data
    IOVRange range;
    std::string tag;
    std::unique_ptr<IOpaqueAddress> addr;
    BOOST_TEST(iovdbsvc->getRange(s_attrListClid, "/Crest/A", runLumi(0, 150), range, tag, addr).isSuccess());
    BOOST_TEST(range.start().re_time() == 100ull);
    BOOST_TEST(range.stop().re_time() == 200ull);
    BOOST_TEST(iovdbsvc->getRange(s_attrListClid, "/Crest/B", runLumi(0, 350), range, tag, addr).isSuccess());
    BOOST_TEST(range.start().re_time() == 300ull);
    BOOST_TEST(range.stop().re_time() == 400ull);
  }

  // A COOL job with one folder overridden to CREST via <db> and no <ctag>
  // fetches the mapping from that folder's endpoint, not the job's
  // (COOL) dbConnection.
  BOOST_AUTO_TEST_CASE(setupFolders_coolJobWithCrestDbOverride_usesOverrideEndpointMapping){
    const auto& ep = endpoints();
    const std::string instance{"CoolWithCrestOverride"};
    jobOptions->set(instance + ".GlobalTag", quoted(s_globalTag));
    jobOptions->set(instance + ".Folders", stringList({
      "/Crest/B<db>crest_fs:" + ep.b.dir.path + "</db><key>/Crest/B_fromCoolJob</key>",
    }));
    ServiceHandle<IIOVDbSvc> iovdbsvc("IOVDbSvc/" + instance, "test");
    BOOST_REQUIRE(iovdbsvc.retrieve().isSuccess());

    SmartIF<IAddressProvider> provider(iovdbsvc.get());
    BOOST_REQUIRE(provider.isValid());
    IAddressProvider::tadList tlist;
    BOOST_REQUIRE(provider->preLoadAddresses(StoreID::DETECTOR_STORE, tlist).isSuccess());
    for (auto* tad : tlist){
      delete tad;
    }

    IIOVDbSvc::KeyInfo info;
    BOOST_REQUIRE(iovdbsvc->getKeyInfo("/Crest/B_fromCoolJob", info));
    BOOST_TEST(info.folderName == "/Crest/B");
    BOOST_TEST(info.tag == "TagB");
  }

  // A folder missing from the GlobalTag mapping fails initialize
  // instead of being skipped
  BOOST_AUTO_TEST_CASE(setupFolders_folderMissingFromGlobalTag_failsInitialize){
    const auto& ep = endpoints();
    configureCrestInstance("CrestMissingFolder", ep.a.dir.path, {"/Crest/NotMapped"});
    ServiceHandle<IIOVDbSvc> iovdbsvc("IOVDbSvc/CrestMissingFolder", "test");

    StdoutCapture capture;
    const bool initialized = iovdbsvc.retrieve().isSuccess();
    const std::string captured = capture.finish();
    BOOST_TEST(!initialized);
    BOOST_REQUIRE(!captured.empty());
    BOOST_TEST(captured.find("does not contain folder /Crest/NotMapped") != std::string::npos);
  }

  // OnlineMode (the HLT's flag) combined with CREST (the HLT's backend)
  // initializes. The between-run reload is COOL-only and never opens a
  // CREST-only connection as COOL.
  BOOST_AUTO_TEST_CASE(onlineMode_withCrestFolder_initializesAndSurvivesRunChange){
    const auto& ep = endpoints();
    configureCrestInstance("CrestOnline", ep.a.dir.path, {"/Crest/A"}, /*onlineMode=*/true);
    ServiceHandle<IIOVDbSvc> iovdbsvc("IOVDbSvc/CrestOnline", "test");

    StdoutCapture initCapture;
    const bool initialized = iovdbsvc.retrieve().isSuccess();
    const std::string initOutput = initCapture.finish();
    BOOST_REQUIRE(initialized);
    BOOST_TEST(initOutput.find("between-run reload applies to COOL folders only") != std::string::npos);

    // The first BeginRun of a job has no previous run to compare with, so
    // only the second call, on a different run, reaches the reload loop
    EventContext ctx;
    Atlas::setExtendedEventContext(ctx, Atlas::ExtendedEventContext(nullptr));
    BOOST_REQUIRE(iovdbsvc->signalBeginRun(runLumi(1, 0), ctx).isSuccess());
    StdoutCapture reloadCapture;
    const bool reloaded = iovdbsvc->signalBeginRun(runLumi(2, 0), ctx).isSuccess();
    const std::string reloadOutput = reloadCapture.finish();
    BOOST_TEST(reloaded);
    BOOST_TEST(reloadOutput.find("Opening COOL connection") == std::string::npos);
    BOOST_TEST(reloadOutput.find("cannot be opened") == std::string::npos);
  }

  // An endpoint whose mapping cannot be fetched fails the job once, not
  // once per folder that would have needed the mapping
  BOOST_AUTO_TEST_CASE(setupFolders_unreachableEndpoint_failsInitializeOnce){
    const auto& ep = endpoints();
    configureCrestInstance("CrestBadEndpoint", ep.a.dir.path + "/does-not-exist", {"/Crest/A", "/Crest/B"});
    ServiceHandle<IIOVDbSvc> iovdbsvc("IOVDbSvc/CrestBadEndpoint", "test");

    StdoutCapture capture;
    const bool initialized = iovdbsvc.retrieve().isSuccess();
    const std::string captured = capture.finish();
    BOOST_TEST(!initialized);
    BOOST_REQUIRE(!captured.empty());
    BOOST_TEST(countOccurrences(captured, "Failed to fetch the CHAI GlobalTag mapping") == 1u);
  }

BOOST_AUTO_TEST_SUITE_END()
