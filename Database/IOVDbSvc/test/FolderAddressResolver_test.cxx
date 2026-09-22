/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file IOVDbSvc/test/FolderAddressResolver_test.cxx
 * @brief Tests for FolderAddressResolver, the address-field resolution both
 *        the COOL and CREST folders share
 *
 * The resolver needs only a MsgStream, a parsed description, and optionally an
 * IClassIDSvc, so every branch is reachable here without COOL, CHAI, or a
 * conditions database. The folder-level tests exercise it only along the happy
 * path, with an explicit clid in every fixture description.
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IOVDBSVC

#include <boost/test/unit_test.hpp>
//
#include <string>

#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ServiceHandle.h"

#include "../src/FolderAddressResolver.h"
#include "../src/IOVDbParser.h"

#include "GaudiKernelFixtureBase.h"

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

namespace {
  constexpr CLID s_attrListClid = 40774348;

  std::string descriptionWithClid(const std::string& extra = "") {
    return "<addrHeader><address_header service_type=\"71\" clid=\"" +
           std::to_string(s_attrListClid) + "\" /></addrHeader>"
           "<typeName>AthenaAttributeList</typeName>" + extra;
  }
}

struct MsgFixture: public GaudiKernelFixtureBase {
  ServiceHandle<IMessageSvc> msgSvc;
  // The real service, not a stub: IClassIDSvc derives from IService, so a
  // stub would be boilerplate and would not prove real type-name resolution.
  ServiceHandle<IClassIDSvc> clidSvc;
  MsgStream log;
  MsgFixture(): GaudiKernelFixtureBase(),
    msgSvc("msgSvc", "test"),
    clidSvc("ClassIDSvc", "test"),
    log(msgSvc.get(), "FolderAddressResolver_test") {
    log.setLevel(MSG::DEBUG);
  }
};

BOOST_FIXTURE_TEST_SUITE(FolderAddressResolverTest, MsgFixture)

  // An explicit clid in the description is used as-is. No IClassIDSvc is
  // consulted, so passing nullptr here is safe.
  BOOST_AUTO_TEST_CASE(resolve_explicitClid_noClassIDSvcNeeded) {
    IOVDbParser desc(descriptionWithClid(), log);
    IOVDbNamespace::FolderAddressSpec result;
    BOOST_TEST(IOVDbNamespace::resolveFolderAddress(
                 log, desc, "/test/folder", false, "currentKey", nullptr, result) == true);
    BOOST_TEST(result.clid == s_attrListClid);
    BOOST_TEST(result.typeName == "AthenaAttributeList");
    BOOST_TEST(result.named == false);
    BOOST_TEST(result.key == "currentKey");
    BOOST_TEST(!result.addrheader.empty());
  }

  // No clid in the description: the type name is resolved through IClassIDSvc.
  BOOST_AUTO_TEST_CASE(resolve_noClid_fallsBackToClassIDSvc) {
    BOOST_REQUIRE(clidSvc.retrieve().isSuccess());
    IOVDbParser desc("<typeName>AthenaAttributeList</typeName>", log);
    IOVDbNamespace::FolderAddressSpec result;
    BOOST_TEST(IOVDbNamespace::resolveFolderAddress(
                 log, desc, "/test/folder", false, "currentKey", clidSvc.get(), result) == true);
    BOOST_TEST(result.clid == s_attrListClid);
  }

  // No clid, and the service cannot resolve the type name either.
  BOOST_AUTO_TEST_CASE(resolve_unresolvableClid_fails) {
    BOOST_REQUIRE(clidSvc.retrieve().isSuccess());
    IOVDbParser desc("<typeName>NoSuchTypeNameExistsHere</typeName>", log);
    IOVDbNamespace::FolderAddressSpec result;
    BOOST_TEST(IOVDbNamespace::resolveFolderAddress(
                 log, desc, "/test/folder", false, "currentKey", clidSvc.get(), result) == false);
  }

  // No clid and no service to fall back on must fail without dereferencing the
  // null service handle.
  BOOST_AUTO_TEST_CASE(resolve_noClidAndNoClassIDSvc_failsWithoutDereferencing) {
    IOVDbParser desc("<typeName>AthenaAttributeList</typeName>", log);
    IOVDbNamespace::FolderAddressSpec result;
    BOOST_TEST(IOVDbNamespace::resolveFolderAddress(
                 log, desc, "/test/folder", false, "currentKey", nullptr, result) == false);
  }

  BOOST_AUTO_TEST_CASE(resolve_emptyTypeName_fails) {
    IOVDbParser desc("<addrHeader><address_header service_type=\"71\" clid=\"40774348\" /></addrHeader>", log);
    IOVDbNamespace::FolderAddressSpec result;
    BOOST_TEST(IOVDbNamespace::resolveFolderAddress(
                 log, desc, "/test/folder", false, "currentKey", nullptr, result) == false);
  }

  // A <key> in the description overrides the folder's current SG key ...
  BOOST_AUTO_TEST_CASE(resolve_descriptionKey_overridesCurrentKey) {
    IOVDbParser desc(descriptionWithClid("<key>fromDescription</key>"), log);
    IOVDbNamespace::FolderAddressSpec result;
    BOOST_REQUIRE(IOVDbNamespace::resolveFolderAddress(
                    log, desc, "/test/folder", false, "currentKey", nullptr, result));
    BOOST_TEST(result.key == "fromDescription");
  }

  // ... unless the key came from job options, which wins over the description.
  BOOST_AUTO_TEST_CASE(resolve_jobOptionKey_beatsDescriptionKey) {
    IOVDbParser desc(descriptionWithClid("<key>fromDescription</key>"), log);
    IOVDbNamespace::FolderAddressSpec result;
    BOOST_REQUIRE(IOVDbNamespace::resolveFolderAddress(
                    log, desc, "/test/folder", true, "fromJobOptions", nullptr, result));
    BOOST_TEST(result.key == "fromJobOptions");
  }

  BOOST_AUTO_TEST_CASE(resolve_namedChannels) {
    IOVDbParser desc(descriptionWithClid("<named/>"), log);
    IOVDbNamespace::FolderAddressSpec result;
    BOOST_REQUIRE(IOVDbNamespace::resolveFolderAddress(
                    log, desc, "/test/folder", false, "currentKey", nullptr, result));
    BOOST_TEST(result.named == true);
  }

BOOST_AUTO_TEST_SUITE_END()
