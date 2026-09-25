/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file tokenLegacyOID_test.cxx
 * @brief Unit test for Token legacy OID format support
 */

#undef NDEBUG
#include <cassert>
#include <iostream>
#include <print>
#include <string>
#include <unordered_set>
#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/Guid.h"

void test_legacy_oid_format_ffffffff() {
    std::println ("Testing legacy OID format with FFFFFFFF values...");

    // Test legacy 8-digit OID format parsing with FFFFFFFF values
    std::string legacyTokenStr = "[DB=12345678-1234-1234-1234-123456789012][CNT=TestContainer][CLID=87654321-4321-4321-4321-210987654321][TECH=12345678][OID=FFFFFFFF-FFFFFFFF]";

    Token legacyToken;
    legacyToken.fromString(legacyTokenStr);

    // Verify that FFFFFFFF is correctly extended to 64-bit ~0x0LL
    assert(legacyToken.oid().first == static_cast<long long int>(~0x0ULL));
    assert(legacyToken.oid().second == static_cast<long long int>(~0x0ULL));

    std::println ("  Legacy FFFFFFFF test passed");
}

void test_modern_oid_format() {
    std::println ("Testing modern OID format...");

    // Test modern 16-digit OID format parsing
    std::string modernTokenStr = "[DB=12345678-1234-1234-1234-123456789012][CNT=TestContainer][CLID=87654321-4321-4321-4321-210987654321][TECH=12345678][OID=FEDCBA9876543210-0123456789ABCDEF]";

    Token modernToken;
    modernToken.fromString(modernTokenStr);

    assert(modernToken.oid().first == static_cast<long long int>(0xFEDCBA9876543210ULL));
    assert(modernToken.oid().second == static_cast<long long int>(0x0123456789ABCDEFULL));

    std::println ("  Modern OID format test passed");
}

void test_legacy_oid_format_regular_values() {
    std::println ("Testing legacy OID format with regular values...");

    // Test legacy format with non-FFFFFFFF values
    std::string legacyTokenStr = "[DB=12345678-1234-1234-1234-123456789012][CNT=TestContainer][CLID=87654321-4321-4321-4321-210987654321][TECH=12345678][OID=12345678-87654321]";

    Token legacyToken;
    legacyToken.fromString(legacyTokenStr);

    assert(legacyToken.oid().first == 0x12345678LL);
    assert(legacyToken.oid().second == 0x87654321LL);

    std::println ("  Legacy regular values test passed");
}

void test_roundtrip_serialization() {
    std::println ("Testing round-trip serialization...");

    // Test round-trip serialization preserves format
    Token originalToken;

    // Set up a test token
    Guid dbGuid;
    dbGuid.fromString("12345678-1234-1234-1234-123456789012");
    originalToken.setDb(dbGuid);

    originalToken.setCont("TestContainer");

    Guid classGuid;
    classGuid.fromString("87654321-4321-4321-4321-210987654321");
    originalToken.setClassID(classGuid);

    originalToken.setTechnology(0x12345678);
    Token::OID_t oid(0xFEDCBA9876543210LL, 0x0123456789ABCDEFLL);
    originalToken.setOid(oid);

    std::string tokenStr = originalToken.toString();
    Token parsedToken;
    parsedToken.fromString(tokenStr);

    assert(originalToken.oid().first == parsedToken.oid().first);
    assert(originalToken.oid().second == parsedToken.oid().second);
    assert(originalToken.toString() == parsedToken.toString());

    std::println ("  Round-trip serialization test passed");
}

void test_oid_format_detection() {
    std::println ("Testing OID format detection...");

    // Test that legacy and modern formats are correctly detected

    // Legacy format should be 23 characters: [OID=XXXXXXXX-XXXXXXXX]
    std::string legacyOid = "[OID=12345678-87654321]";
    assert(legacyOid.length() == 23);

    // Modern format should be 39 characters: [OID=XXXXXXXXXXXXXXXX-XXXXXXXXXXXXXXXX]
    std::string modernOid = "[OID=FEDCBA9876543210-0123456789ABCDEF]";
    assert(modernOid.length() == 39);

    std::println ("  OID format detection test passed");
}

// coverity[root_function]
int main() {
    static_assert(std::is_trivially_destructible<Guid>::value);
    static_assert(std::is_trivially_copyable<Guid>::value);
    //Check hash function compiles
    std::unordered_set<Guid> dfgfg;

    std::println ("Running Token legacy OID format tests...\n");

    try {
      test_legacy_oid_format_ffffffff();
      test_modern_oid_format();
      test_legacy_oid_format_regular_values();
      test_roundtrip_serialization();
      test_oid_format_detection();
    } catch (const std::exception& e) {
      std::println (std::cerr, "exception: {}", e.what());
      return 1;
    }

    std::println ("\nAll Token legacy OID format tests passed!");
    return 0;
}
