/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EventPrimitivesToStringConverter.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef EVENTPRIMITIVESTOSTRINGCONVERTER_H_
#define EVENTPRIMITIVESTOSTRINGCONVERTER_H_
#include "EventPrimitives/EventPrimitives.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <cmath>



namespace Amg {

/** EventPrimitvesToStringConverter

    inline methods for conversion of EventPrimitives (Matrix)
    to std::string.

    This is to enhance formatted screen ouput and for ASCII based
    testing.

    The offset can be used to offset the lines (starting from line 2) wrt to the
    zero position for formatting reasons.

    @author Niels.Van.Eldik@cern.ch

    */
    
inline double
roundWithPrecision(double val, int precision)
{
  const double scale = std::pow(10.0, precision);
  const double rounded = std::round(val * scale) / scale;
  return rounded == 0.0 ? 0.0 : val;
}

inline std::string
toString(const MatrixX& matrix, int precision = 4,
         std::string_view offset = "")
{
  std::ostringstream sout;
  sout << std::fixed << std::setprecision(precision);

  const double scale = std::pow(10.0, precision);

  for (int i = 0; i < matrix.rows(); ++i) {
    sout << '(';

    for (int j = 0; j < matrix.cols(); ++j) {
      const double val = matrix(i, j);

      sout << (std::round(val * scale) == 0.0 ? 0.0 : val);

      if (j + 1 != matrix.cols()) {
        sout << ", ";
      }
    }

    sout << ')';

    if (i + 1 != matrix.rows()) {
      sout << '\n' << offset;
    }
  }

  return sout.str();
}

}  // namespace Amg

#endif /* EVENTPRIMITIVESTOSTRINGCONVERTER_H_ */
