/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//tests for EventPrimitivesToStringConverter utility
//keeping with approximately the same style as pre-existing tests
// i.e. simple, no test frameework
#include "EventPrimitives/EventPrimitivesToStringConverter.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace Amg;

int 
checkEqual(const std::string& actual, const std::string& expected, const char* testName) {
  if (actual == expected) {
    return 0;
  }
  std::cerr << "FAILED: " << testName << '\n'
            << "Expected:\n"
            << expected << '\n'
            << "Actual:\n"
            << actual << '\n';

  return 1;
}

int 
checkEqual(double actual, double expected,const char* testName) {
  if (actual == expected) { //exact equality between doubles is ok here
    return 0;
  }
  std::cerr << "FAILED: " << testName << '\n'
            << "Expected: " << expected << '\n'
            << "Actual:   " << actual << '\n';

  return 1;
}

int main() {
  int failures{};
  {
  failures += checkEqual(roundWithPrecision(1.2345, 4),1.2345,"roundWithPrecision positive value");
  failures += checkEqual(roundWithPrecision(-1.2345, 4), -1.2345,"roundWithPrecision negative value above threshold");
  //this odd and possibly buggy result reflects the current behaviour:sign flip
  failures += checkEqual(roundWithPrecision(-0.00001, 4),0.00001,"roundWithPrecision small negative value");
  //sign flip?
  failures += checkEqual(roundWithPrecision(-0.00009, 4), 0.00009,"roundWithPrecision negative value below 1e-4");
  //sign flip?
  failures += checkEqual(roundWithPrecision(-0.0001, 4),-0.0001,"roundWithPrecision value exactly at threshold");
  //
  failures += checkEqual(roundWithPrecision(0.00001, 4),0.00001,"roundWithPrecision small positive value");
  failures += checkEqual(roundWithPrecision(0.0, 4),0.0,"roundWithPrecision zero");
  failures += checkEqual(roundWithPrecision(-0.4, 0),0.4,"roundWithPrecision precision zero small negative value");
  failures += checkEqual(roundWithPrecision(-1.0, 0),-1.0,"roundWithPrecision precision zero threshold");
  //current behaviour allows use of _negative_ precision
  failures += checkEqual(roundWithPrecision(-5.0, -1), 5.0,"roundWithPrecision negative precision");
  }
  {
    MatrixX matrix(3, 1);
    matrix << 1.0, 2.5, -3.25;
    const std::string result = toString(matrix);
    failures += checkEqual(result, "(1.0000, 2.5000, -3.2500)", "3D Vector");
  }

  {
    MatrixX matrix(2, 2);
    matrix << 1.0, 2.0,
              3.0, 4.0;
    const std::string result = toString(matrix);
    failures += checkEqual(result,"(1.0000, 2.0000)\n(3.0000, 4.0000)", "2x2 Matrix");
  }

  {
    MatrixX matrix(2, 2);
    matrix << 1.0, 2.0,
              3.0, 4.0;
    const std::string result = toString(matrix, 2, "  ");
    failures += checkEqual(result, "(1.00, 2.00)\n  (3.00, 4.00)", 
      "2x2  Matrix with offset and 2-point precision");
  }

  {
    MatrixX matrix(1, 3);
    matrix << 1.23456, 2.34567, 3.45678;
    const std::string result = toString(matrix, 3);
    failures += checkEqual(result, "(1.235, 2.346, 3.457)", "2x2 Matrix with 3-point precision");
  }

  {
    MatrixX matrix(2, 1);
    matrix << -0.00001, 0.00001;
    const std::string result = toString(matrix, 4);
    failures += checkEqual(result, "(0.0000, 0.0000)", "2D Vector with 4-point precision");
  }

  {
    MatrixX matrix(1, 1);
    matrix << 42.0;
    const std::string result = toString(matrix, 0);
    failures += checkEqual(result,"(42)", "1x1 Matrix");
  }
  if (failures!=0){
    const std::string number = (failures == 1) ? "test.\n" : "tests.\n";
    std::cerr<<"EventPrimitivesToStringConverter "<<failures<<" failed "<<number;
    return -1;
  }
  std::cout << "All EventPrimitivesToStringConverter tests passed.\n";
  return 0;
}