/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

class TH1;
class TAxis;

#include <iostream>

namespace RootHelpers
{
  /// Helper function for edge cases in 3D interpolation by interpolating in 2D and keeping the third axis axis (edge) fixed
  /// The input parameters "xAxis" and "yAxis" indicate which axes should be used for the 2D interpolation: x-axis: 1, y-axis: 2, z-axis: 3
  /// The input parameter "otherDimBin" is the bin on the third axis which will be kept constant in the interpolation
  double Interpolate2D(const TH1* histo, const double x, const double y, const int xAxis, const int yAxis, const int otherDimBin);

  /// Function for 3D interpolation to account for edge cases in TH3::Interpolate
  double Interpolate(const TH1* histo, const double x, const double y, const double z);

} // end RootHelpers namespace

