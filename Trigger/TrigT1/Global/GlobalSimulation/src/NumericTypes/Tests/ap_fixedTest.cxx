/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "gtest/gtest.h"
#include "../ap_fixed.h"

#include <sstream>
#include <cmath>

// this function will be templated on the int type.
constexpr int pow(int base, int exp) noexcept {
  auto result{1};

  for(int i = 0; i < exp; ++i) {
    result *= base;
  }

  return result;
}

template<int W, int P>
constexpr double min() {
  static_assert(W >= P);
  static_assert(P >= 0);
  return -pow(2, W-1)/pow(2, P);
}

template<int W, int P>
constexpr double max () {
  static_assert(W >= P);
  static_assert(P >= 0);
  return static_cast<double>(pow(2, W-1)-1)/static_cast<double>(pow(2, P));
}

TEST (ap_fixedTester, minmax) {

  auto min_v = min<10, 5>();
  auto max_v = max<10, 5>();
  EXPECT_EQ(-16, min_v);
  EXPECT_EQ(15.96875, max_v);
}

TEST(ap_fixedTester, stablity) {


  constexpr int width{10};
  constexpr int prec{5};
  double delta = pow(2, -prec-1); // step by a value less then precision
  
  double val = min<width, prec>();
  double max_val = max<width, prec>();
  double cval = -val;

  std::size_t i{0};

  while (cval <= max_val) {
    
    auto ap_gs0 = GlobalSim::ap_fixed<10, 5>(cval);
    double d0 = static_cast<double>(ap_gs0);
    auto ap_gs1 = GlobalSim::ap_fixed<10, 5>(d0);
    double d1 = static_cast<double>(ap_gs1);
    auto ap_gs2 = GlobalSim::ap_fixed<10, 5>(d1);
    double d2 = static_cast<double>(ap_gs2);
   
    EXPECT_EQ (d2, d0);
    EXPECT_EQ (ap_gs0.m_value, ap_gs2.m_value);
    EXPECT_LT (std::abs(d0 -cval), 1./(2*2*2*2*2));
   

    cval =  -val + ((++i) * delta);
  }
}

TEST(ap_fixedTester, specialValue) {

  //Xylinx ap_fixed gets this wrong
  auto apf =  GlobalSim::ap_fixed<10, 5>(-0.327374935);
  EXPECT_EQ (-0.3125, static_cast<double>(apf));
  std::stringstream ss;
  ss << std::hex << apf.m_value;
  EXPECT_EQ ("fff6", ss.str());
}



TEST(ap_fixedTester, overflow_h) {
  constexpr int width{10};
  constexpr int prec{5};

  /// EXPECT does not take templates, provide a new typename
  using ap = typename GlobalSim::ap_fixed<width, prec>; 
  // out of range by 0.5*prec
  double outOfRange = max<width, prec>() + pow(2, -prec-1);

  EXPECT_THROW ((ap(outOfRange)), std::out_of_range);
}

TEST(ap_fixedTester, overflow_l) {
  constexpr int width{10};
  constexpr int prec{5};

  // out of range by 0.5*prec
  double outOfRange = min<width, prec>() - pow(2, -prec-1);
  using ap = GlobalSim::ap_fixed<width, prec>;

  EXPECT_THROW ((ap(outOfRange)), std::out_of_range);
}


TEST(ap_fixedTester, addition) {

  constexpr int width{10};
  constexpr int prec{5};
  constexpr double eps {std::pow(2.0, -prec)};
  using ap = GlobalSim::ap_fixed<width, prec>;

  ap ap_sum = ap(1) + ap(2);
  auto val = static_cast<double>(ap_sum);
  auto diff = std::abs(val - 3);
  
  EXPECT_EQ (3, val);
  EXPECT_LT (diff, eps);

  ap_sum = ap(1.5) + ap(2.5);
  val = static_cast<double>(ap_sum);
  diff = std::abs(val - 4);
  
  EXPECT_EQ (4, val);
  EXPECT_LT (diff, eps);
}

TEST(ap_fixedTester, addition1) {
  
  constexpr int width{10};
  constexpr int prec{5};
  constexpr double eps {std::pow(2.0, -prec)};
  using ap = GlobalSim::ap_fixed<width, prec>;

  ap ap_sum = ap(2) += 1;
  auto val = static_cast<double>(ap_sum);
  auto diff = std::abs(val - 3);
   
  EXPECT_EQ (3, val);
  EXPECT_LT (diff, eps);

  ap_sum = ap(1.5) += 2.5;
  val = static_cast<double>(ap_sum);
  diff = std::abs(val - 4);
  
  EXPECT_EQ (4, val);
  EXPECT_LT (diff, eps);
}


TEST(ap_fixedTester, subtraction) {

  constexpr int width{10};
  constexpr int prec{5};
  constexpr double eps {std::pow(2.0, -prec)};
  using ap = GlobalSim::ap_fixed<width, prec>;

  ap ap_diff = ap(1) - ap(2);
  auto val = static_cast<double>(ap_diff);
  auto diff = std::abs(val + 1); 

  EXPECT_EQ (-1, val);
  EXPECT_LT (diff, eps);

  ap_diff = ap(1.5) - ap(2.5);
  val = static_cast<double>(ap_diff);
  diff = std::abs(val + 1); 

  EXPECT_EQ (-1, val);
  EXPECT_LT (diff, eps);
}

TEST(ap_fixedTester, subtraction1) {

  constexpr int width{10};
  constexpr int prec{5};
  constexpr double eps {std::pow(2.0, -prec)};
  using ap = GlobalSim::ap_fixed<width, prec>;

  ap ap_diff = ap(1) -= 2;
  auto val = static_cast<double>(ap_diff);
  auto diff = std::abs(val + 1);

  EXPECT_EQ (-1, val);
  EXPECT_LT (diff, eps);

  ap_diff = ap(1.5) -= 2.5;
  val = static_cast<double>(ap_diff);
  diff = std::abs(val + 1); 

  EXPECT_EQ (-1, val);
  EXPECT_LT (diff, eps);
}



TEST(ap_fixedTester, multiplication) {

  constexpr int width{10};
  constexpr int prec{5};
  constexpr double eps {std::pow(2.0, -prec)};
  using ap = GlobalSim::ap_fixed<width, prec>;

  ap ap_prod = ap(1) * ap(2);
  auto val = static_cast<double>(ap_prod);
  auto diff = std::abs(val - 2);

  EXPECT_EQ (2, val);
  EXPECT_LT (diff, eps);

  ap_prod = ap(-1.5)*ap(2);
  val = static_cast<double>(ap_prod);
  diff = std::abs(val +3);

  EXPECT_EQ (-3, val);
  EXPECT_LT (diff, eps);
}

TEST(ap_fixedTester, multiplication1) {
  constexpr int width{10};
  constexpr int prec{5};
  constexpr double eps {std::pow(2.0, -prec)};
  using ap = GlobalSim::ap_fixed<width, prec>;

  ap ap_prod = ap(1) *= ap(2);
  auto val = static_cast<double>(ap_prod);
  auto diff = std::abs(val - 2);

  EXPECT_EQ (2, val);
  EXPECT_LT (diff, eps);

  ap_prod = ap(-1.5) *= ap(2.);
  val = static_cast<double>(ap_prod);
  diff = std::abs(val +3);
  
  EXPECT_EQ (-3, val);
  EXPECT_LT (diff, eps);
}


TEST(ap_fixedTester, division) {

  constexpr int width{10};
  constexpr int prec{5};
  constexpr double eps {std::pow(2.0, -prec)};
  using ap = GlobalSim::ap_fixed<width, prec>;

  ap ap_div = ap(1)/ap(2);
  auto val = static_cast<double>(ap_div);
  auto diff = std::abs(val - 0.5);

  EXPECT_EQ (0.5, val);
  EXPECT_LT (diff, eps);

  ap_div = ap(-1.5)/ap(2);
  val = static_cast<double>(ap_div);
  diff = std::abs(val + 0.75);

  EXPECT_EQ (-0.75, val);
  EXPECT_LT (diff, eps);
}


TEST(ap_fixedTester, division1) {

  constexpr int width{10};
  constexpr int prec{5};
  constexpr double eps {std::pow(2.0, -prec)};
  using ap = GlobalSim::ap_fixed<width, prec>;

  ap ap_div = ap(1)/=ap(2);
  auto val = static_cast<double>(ap_div);
  auto diff = std::abs(val - 0.5);

  EXPECT_EQ (0.5, val);
  EXPECT_LT (diff, eps);

  ap_div = ap(-1.5)/ap(2);
  val = static_cast<double>(ap_div);
  diff = std::abs(val + 0.75);

  EXPECT_EQ (-0.75, val);
  EXPECT_LT (diff, eps);
}


TEST(ap_fixedTester, negation) {

  constexpr int width{10};
  constexpr int prec{5};

  using ap = GlobalSim::ap_fixed<width, prec>;

  auto ap_p = ap(1);
  auto ap_n = -ap_p;
  auto ap_pp = -ap_n;
  
  auto val_p = static_cast<double>(ap_p);
  auto val_n = static_cast<double>(ap_n);
  auto val_pp = static_cast<double>(ap_pp);

  EXPECT_EQ (val_p, -val_n);
  EXPECT_EQ (val_p, val_pp);
}

TEST(ap_fixedTester, doubleMult) {

  constexpr int width{10};
  constexpr int prec{5};

  using ap = GlobalSim::ap_fixed<width, prec>;
  ap ap_mul = 9.99 * ap(1);

  EXPECT_EQ (10, static_cast<double>(ap_mul));
}


