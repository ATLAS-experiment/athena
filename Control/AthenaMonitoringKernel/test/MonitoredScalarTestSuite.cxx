/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_MODULE MonitoredScalarTestSuite
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <iostream>

#include "AthenaMonitoringKernel/MonitoredScalar.h"

/// Custom type
class TestValue {
public:
  TestValue(double value) : m_value(value) {}
  operator double() const { return m_value; }
  bool operator==(const TestValue &other) const { return m_value == other.m_value; }
private:
  double m_value;
};

// Required for BOOST_TEST
std::ostream& operator<<(std::ostream& ost, const TestValue& v) {
  ost << static_cast<double>(v); return ost;
}


BOOST_AUTO_TEST_CASE( implicitType ) {
  auto phi = Monitored::Scalar("phi", 4.123456789012345);

  BOOST_TEST(phi == 4.123456789012345);
}

BOOST_AUTO_TEST_CASE( explicitType ) {
  auto phi = Monitored::Scalar<float>("phi", 4.1234567f);

  BOOST_TEST(phi == 4.1234567f);
}

BOOST_AUTO_TEST_CASE( customType ) {
  auto phi = Monitored::Scalar("phi", TestValue(4.123456789012345));

  BOOST_TEST(TestValue(4.123456789012345) == phi);
}

BOOST_AUTO_TEST_CASE( converter ) {
  auto phi = Monitored::Scalar<double>("phi", 4.123456789012345, [](double value) { return value * 1000; });

  BOOST_TEST(phi == 4.123456789012345);
  BOOST_TEST(phi.get(0) == 4123.456789012345);
}

BOOST_AUTO_TEST_CASE( changeValue ) {
  auto phi = Monitored::Scalar("phi", 4.2);
  phi = 5.5;

  BOOST_TEST(phi == 5.5);
}

BOOST_AUTO_TEST_CASE( elementAccess ) {
  auto phi = Monitored::Scalar("phi", 4.2);

  BOOST_TEST(phi.size() == 1);
  BOOST_TEST(phi.get(0) == 4.2);
}

BOOST_AUTO_TEST_CASE( valueOperator ) {
  auto name = Monitored::Scalar<std::string>("name", "foo");

  std::vector<std::string> v;
  v.push_back(name);
  BOOST_TEST(v[0] == "foo");
}

BOOST_AUTO_TEST_CASE( operator_int ) {
  auto i = Monitored::Scalar<int>("int", 0);
  i++;
  ++i;
  BOOST_TEST(i == 2);
  i--;
  --i;
  BOOST_TEST(i == 0);

  int j = i + 2;
  BOOST_TEST(j == 2);

  j = 2 + j;
  BOOST_TEST(j == 4);
}

BOOST_AUTO_TEST_CASE( operator_string ) {
  auto name = Monitored::Scalar<std::string>("name", "y");
  std::string s = std::string("x") + name;
  BOOST_TEST(s == "xy");

  s = name + std::string("z");
  BOOST_TEST(s == "yz");

  s = "x" + name;
  BOOST_TEST(s == "xy");

  s = name + "z";
  BOOST_TEST(s == "yz");
}
