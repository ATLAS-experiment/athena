/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGAFPHYPO_AFPPROTONTRANSPORTPARAM_H
#define TRIGAFPHYPO_AFPPROTONTRANSPORTPARAM_H

#include "AFPProtonTransportPolynomial.h"
#include <memory>

/**
 * @brief Class for the parameterization used by the proton transport tool
 *
 * This class calculates the parameterization for a given polynomial. Using the given polynomial and the inital positon, slope and nominal beam energy it can calculate the parameterization.
 */

class AFPProtonTransportParam {
public:
  /// @brief Constructor which sets #m_energy and #m_polynomial
  /// @param beam_energy nominal beam energy
  /// @param poly_array array of pointers to 8 polynomials with transport parametrisation

  AFPProtonTransportParam (double beam_energy, std::unique_ptr<AFPProtonTransportPolynomial> poly_array[8]);
  /// Destructor of the class
  ~AFPProtonTransportParam() = default;

  /// This function evaluates value of the parameterization given a certain polynomial. It takes as arguments the inital position, slope and nominal energy and uses it along with the polynomial objects given to the constructor to calculate the value of the parameterization. 
  double evaluate(double x0, double y0, double z0, double sx, double sy, double E) const;

private:
  ///Nominal beam energy
  double m_energy;
  ///Object of the AFPProtonTransportPolynomial class used for the position calculations
  std::unique_ptr<AFPProtonTransportPolynomial> m_polynomial[8];
};

#endif
