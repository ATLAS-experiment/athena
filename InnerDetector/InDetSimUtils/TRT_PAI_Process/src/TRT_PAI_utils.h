/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_PAI_utils_h
#define TRT_PAI_utils_h

#include <vector>

/**
 * Utilities
 */
namespace TRT_PAI_utils {
  /**
   * Interpolation function. From tabulated values of y vs x, it returns
   * the interpolated y value corresponding to the input x value.
   * @return @a y value from interpolation
   * @param xval: @a x value for which we seek @a y value
   * @param xtabulated: tabulated @a x values
   * @param ytabulated: tabulated @a y values
   * @author Pavel Nevski (adapted to C++ by T.Kittelmann and Mogens Dam)
   */
  float Interpolate(float xval,
		    const std::vector<float>& xtabulated,
		    const std::vector<float>& ytabulated);
  inline float calculate_interpolation(float xval, float x1, float x2, float y1, float y2)
  {
    return y1+(y2-y1)*(xval-x1)/(x2-x1);
  }
}
#endif
