/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE SimpleEncrypterTest

#include "BPhysTools/SimpleEncrypter.h"

#include <boost/test/unit_test.hpp>

#include <cmath>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace utf = boost::unit_test;

namespace {

  std::pair<std::string, std::string>
  makeKeyPair() {
    xAOD::SimpleEncrypter keyGenerator{"SimpleEncrypterKeyGenerator"};
    return keyGenerator.genKeyPair();
  }

}

BOOST_AUTO_TEST_SUITE(SimpleEncrypterTest)

BOOST_AUTO_TEST_CASE(generatedKeysAreNonEmpty) {
  const auto [privateKey, publicKey] = makeKeyPair();

  BOOST_TEST(!privateKey.empty());
  BOOST_TEST(!publicKey.empty());
}

BOOST_AUTO_TEST_CASE(generatedKeysCanBeSetAndReadBack, * utf::expected_failures(2)) {
  const auto [privateKey, publicKey] = makeKeyPair();

  xAOD::SimpleEncrypter encrypter{"SimpleEncrypterPublicKeyTest"};
  xAOD::SimpleEncrypter decrypter{"SimpleEncrypterPrivateKeyTest"};

  encrypter.setPubKey(publicKey);
  decrypter.setPrivKey(privateKey);

  BOOST_TEST(encrypter.getPubKey() == publicKey);
  BOOST_TEST(decrypter.getPrivKey() == privateKey);
}

BOOST_AUTO_TEST_CASE(integerValuesRoundTripWithGeneratedKeys) {
  const auto [privateKey, publicKey] = makeKeyPair();

  xAOD::SimpleEncrypter encrypter{"SimpleEncrypterIntegerEncrypt"};
  xAOD::SimpleEncrypter decrypter{"SimpleEncrypterIntegerDecrypt"};

  encrypter.setPubKey(publicKey);
  decrypter.setPrivKey(privateKey);

  const std::vector<xAOD::SimpleEncrypter::ULLI_t> values{
    0ULL,
    1ULL,
    2ULL,
    3ULL,
    10ULL,
    123ULL,
    1024ULL,
    65535ULL,
    1234567ULL
  };

  for (const xAOD::SimpleEncrypter::ULLI_t value : values) {
    const xAOD::SimpleEncrypter::ULLI_t encrypted = encrypter.encrypt(value);
    const xAOD::SimpleEncrypter::ULLI_t decrypted = decrypter.decrypt(encrypted);

    BOOST_TEST_CONTEXT("value = " << value) {
      BOOST_TEST(decrypted == value);
    }
  }
}

BOOST_AUTO_TEST_CASE(floatValuesRoundTripWithGeneratedKeys) {
  const auto [privateKey, publicKey] = makeKeyPair();

  xAOD::SimpleEncrypter encrypter{"SimpleEncrypterFloatEncrypt"};
  xAOD::SimpleEncrypter decrypter{"SimpleEncrypterFloatDecrypt"};

  encrypter.setPubKey(publicKey);
  decrypter.setPrivKey(privateKey);

  const std::vector<float> values{
    1.0F,
    1.5F,
    2.0F,
    3.25F,
    10.0F,
    123.456F,
    1000.0F,
    std::numeric_limits<float>::min(),
    std::numeric_limits<float>::epsilon()
  };

  for (const float value : values) {
    const float encrypted = encrypter.encrypt(value);
    const float decrypted = decrypter.decrypt(encrypted);

    BOOST_TEST_CONTEXT("value = " << value
                       << ", encrypted = " << encrypted
                       << ", decrypted = " << decrypted) {
      BOOST_TEST(std::isfinite(decrypted));
      BOOST_TEST(decrypted == value);
    }
  }
}

BOOST_AUTO_TEST_CASE(publicAndPrivateKeysCanBeUsedOnSeparateInstances) {
  const auto [privateKey, publicKey] = makeKeyPair();

  xAOD::SimpleEncrypter encrypter{"SimpleEncrypterSeparatePublic"};
  xAOD::SimpleEncrypter decrypter{"SimpleEncrypterSeparatePrivate"};

  encrypter.setPubKey(publicKey);
  decrypter.setPrivKey(privateKey);

  constexpr float original = 42.25F;

  const float encrypted = encrypter.encrypt(original);
  const float decrypted = decrypter.decrypt(encrypted);

  BOOST_TEST(decrypted == original);
}

BOOST_AUTO_TEST_SUITE_END()
