/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file test_eta.cxx
 * @brief test for EventPrimitivesHelpers
 */

#include <iostream>

#include "EventPrimitives/EventPrimitivesCovarianceHelpers.h"

int main() {

  std::cout << "Test Covariance Helpers " << '\n';
  std::cout << '\n' << "Testing Matrix A " << '\n';
  {
    AmgSymMatrix(5) A;
    A << 50977.455023, -3154.699348, 191.699597, -3.127933, 0.000037,
        -3154.699348, 50043.753058, -10.483807, -29.006012, -0.001595,
        191.699597, -10.483807, 0.724398, -0.012684, 0.000000, -3.127933,
        -29.006012, -0.012684, 0.021047, 0.000003, 0.000037, -0.001595,
        0.000000, 0.000003, 1e-12;

    std::cout << A << '\n';
    std::cout << "isPositiveSemiDefinite " << Amg::isPositiveSemiDefiniteSlow(A)
              << '\n';
    std::cout << "isPositiveDefinite " << Amg::isPositiveDefiniteSlow(A)
              << '\n';
    std::cout << "Cholesky isPositiveSemiDefinite "
              << Amg::isPositiveSemiDefinite(A) << '\n';
    std::cout << "Cholesky isPositiveDefinite " << Amg::isPositiveDefinite(A)
              << '\n';
    std::cout << "All diagonal Elements are >=0 "
              << Amg::hasPositiveOrZeroDiagElems(A) << '\n';
    std::cout << "All diagonal Elements are >0 " << Amg::hasPositiveDiagElems(A)
              << '\n';

    AmgVector(5) V;
    V << 50977.455023, -3154.699348, 191.699597, -3.127933, 0.000037;

    std::cout << V << '\n';
    std::cout << "ChiSquared is " << Amg::chi2(A, V)
              << '\n';
  }

  std::cout << '\n' << "Testing dynamic Matrix A" << '\n';
  {
    Amg::MatrixX A;
    A.resize(5, 5);
    A << 50977.455023, -3154.699348, 191.699597, -3.127933, 0.000037,
        -3154.699348, 50043.753058, -10.483807, -29.006012, -0.001595,
        191.699597, -10.483807, 0.724398, -0.012684, 0.000000, -3.127933,
        -29.006012, -0.012684, 0.021047, 0.000003, 0.000037, -0.001595,
        0.000000, 0.000003, 1e-12;
    std::cout << A << '\n';
    std::cout << "isPositiveSemiDefinite " << Amg::isPositiveSemiDefiniteSlow(A)
              << '\n';
    std::cout << "isPositiveDefinite " << Amg::isPositiveDefiniteSlow(A)
              << '\n';
    std::cout << "Cholesky isPositiveSemiDefinite "
              << Amg::isPositiveSemiDefinite(A) << '\n';
    std::cout << "Cholesky isPositiveDefinite " << Amg::isPositiveDefinite(A)
              << '\n';
    std::cout << "All diagonal Elements are >=0 "
              << Amg::hasPositiveOrZeroDiagElems(A) << '\n';
    std::cout << "All diagonal Elements are >0 " << Amg::hasPositiveDiagElems(A)
              << '\n';

    Amg::VectorX V;
    V.resize(5);
    V << 50977.455023, -3154.699348, 191.699597, -3.127933, 0.000037;

    std::cout << V << '\n';
    std::cout << "ChiSquared is " << Amg::chi2(A, V)
              << '\n';
  }

  std::cout << '\n' << "Testing Matrix B" << '\n';
  {
    AmgSymMatrix(5) B;
    B << 36.0000, 0.0000, -0.0222, -0.0001, 0.0000, 0.0000, 3600.0000, -1.5939,
        -0.0164, 0.0000, -0.0184, -1.2931, -6230751.9604, -290277.8826, -0.5211,
        0.0001, -0.0023, -290277.8826, -13523.4478, -0.0243, 0.0000, 0.0000,
        -0.5211, -0.0243, 0.0000;
    std::cout << B << '\n';
    std::cout << "isPositiveSemiDefinite " << Amg::isPositiveSemiDefiniteSlow(B)
              << '\n';
    std::cout << "isPositiveDefinite " << Amg::isPositiveDefiniteSlow(B)
              << '\n';
    std::cout << "Cholesky isPositiveSemiDefinite "
              << Amg::isPositiveSemiDefinite(B) << '\n';
    std::cout << "Cholesky isPositiveDefinite " << Amg::isPositiveDefinite(B)
              << '\n';
    std::cout << "All diagonal Elements are >=0 "
              << Amg::hasPositiveOrZeroDiagElems(B) << '\n';
    std::cout << "All diagonal Elements are >0 " << Amg::hasPositiveDiagElems(B)
              << '\n';

    AmgVector(5) V;
    V << 36.0000, 0.0000, -0.0222, -0.0001, 0.0000;

    std::cout << V << '\n';
    std::cout << "ChiSquared is " << Amg::chi2(B, V)
              << '\n';
  }

  std::cout << '\n' << "Testing Zero Matrix" << '\n';
  {
    AmgSymMatrix(5) zero;
    zero.setZero();
    std::cout << zero << '\n';
    std::cout << "isPositiveSemiDefinite "
              << Amg::isPositiveSemiDefiniteSlow(zero) << '\n';
    std::cout << "isPositiveDefinite " << Amg::isPositiveDefiniteSlow(zero)
              << '\n';
    std::cout << "Cholesky isPositiveSemiDefinite "
              << Amg::isPositiveSemiDefinite(zero) << '\n';
    std::cout << "Cholesky isPositiveDefinite " << Amg::isPositiveDefinite(zero)
              << '\n';
    std::cout << "All diagonal Elements are >=0 "
              << Amg::hasPositiveOrZeroDiagElems(zero) << '\n';
    std::cout << "All diagonal Elements are >0 "
              << Amg::hasPositiveDiagElems(zero) << '\n';

    AmgVector(5) zeroV;
    zeroV.setZero();

    std::cout << zeroV << '\n';
    std::cout << "ChiSquared is " << Amg::chi2(zero, zeroV)
              << '\n';
  }

  std::cout << '\n' << "Testing Dynamic Zero Matrix" << '\n';
  {
    Amg::MatrixX zero;
    zero.resize(5, 5);
    zero.setZero();
    std::cout << zero << '\n';
    std::cout << "isPositiveSemiDefinite "
              << Amg::isPositiveSemiDefiniteSlow(zero) << '\n';
    std::cout << "isPositiveDefinite " << Amg::isPositiveDefiniteSlow(zero)
              << '\n';
    std::cout << "Cholesky isPositiveSemiDefinite "
              << Amg::isPositiveSemiDefinite(zero) << '\n';
    std::cout << "Cholesky isPositiveDefinite " << Amg::isPositiveDefinite(zero)
              << '\n';
    std::cout << "All diagonal Elements are >=0 "
              << Amg::hasPositiveOrZeroDiagElems(zero) << '\n';
    std::cout << "All diagonal Elements are >0 "
              << Amg::hasPositiveDiagElems(zero) << '\n';

    Amg::VectorX zeroV;
    zeroV.resize(5);
    zeroV.setZero();

    std::cout << zeroV << '\n';
    std::cout << "ChiSquared is " << Amg::chi2(zero, zeroV)
              << '\n';
  }
}
