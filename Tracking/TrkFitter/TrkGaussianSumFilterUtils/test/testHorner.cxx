
/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// since we are testing an "internal" /implementation method of the .cxx
#include "src/ElectronCombinedMaterialEffects.cxx"
#include <cmath>
#include <iostream>

int
main()
{

  // Let's use the expansion of the exponential function
  // and approximate e-1 = 1/e
  constexpr std::array<double, 1> expPoly0{ 1. };
  constexpr std::array<double, 2> expPoly1{ 1., 1. };
  constexpr std::array<double, 3> expPoly2{ 1 / 2., 1., 1. };
  constexpr std::array<double, 4> expPoly3{ 1 / 6., 1 / 2., 1., 1. };
  constexpr std::array<double, 5> expPoly4{ 1. / 24., 1 / 6., 1 / 2., 1., 1. };
  constexpr std::array<double, 6> expPoly5{ 1. / 120., 1. / 24., 1 / 6., 1 / 2., 1., 1.};
  constexpr std::array<double, 7> expPoly6{ 1. / 720., 1. / 120., 1. / 24., 1 / 6., 1 / 2., 1., 1.};

  std::cout << "Approximating e^-1 = "<< 1 / M_E << '\n';
  std::cout <<"0th "<< hornerEvaluate(expPoly0, -1) << " vs " << 1. << '\n';
  std::cout <<"1st "<< hornerEvaluate(expPoly1, -1) << " vs " << 0. << '\n';
  std::cout <<"2nd "<< hornerEvaluate(expPoly2, -1) << " vs " << 1. / 2. << '\n';
  std::cout <<"3rd "<< hornerEvaluate(expPoly3, -1) << " vs " << 1. / 3. << '\n';
  std::cout <<"4th "<< hornerEvaluate(expPoly4, -1) << " vs " << 3. / 8. << '\n';
  std::cout <<"5th "<< hornerEvaluate(expPoly5, -1) << " vs " << 11. / 30. << '\n';
  std::cout <<"6th "<< hornerEvaluate(expPoly6, -1) << " vs " << 53. / 144. << '\n';
  return 0;
}

