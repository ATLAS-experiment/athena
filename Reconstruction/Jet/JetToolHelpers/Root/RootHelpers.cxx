/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "JetToolHelpers/RootHelpers.h"

#include "TH1.h"
#include "TMath.h"

namespace RootHelpers
{

  double Interpolate2D(const TH1* histo, const double x, const double y, const int xAxis, const int yAxis, const int otherDimBin)
  {
    // Copied from ROOT directly and trivially modified, all credit to ROOT authors of TH1, TH2, and TH3 Interpolate methods
    // Function was implemented to handle edge cases in the 3D interpolation by using 2D interpolation with the third dimension fixed

    // Determine the two axes to be used for the interpolation
    const TAxis* fXaxis = xAxis == 1 ? histo->GetXaxis() : xAxis == 2 ? histo->GetYaxis() : xAxis == 3 ? histo->GetZaxis() : nullptr;
    const TAxis* fYaxis = yAxis == 1 ? histo->GetXaxis() : yAxis == 2 ? histo->GetYaxis() : yAxis == 3 ? histo->GetZaxis() : nullptr;

    if (!fXaxis || !fYaxis)
    {
        histo->Error("Interpolate2D","Failed to parse axes from inputs");
        return 0;
    }

    // =========================================================
    // The code below is a copy of the TH2::Interpolate function
    // =========================================================
    Double_t f=0;
    Double_t x1=0,x2=0,y1=0,y2=0;
    Double_t dx,dy;
    Int_t bin_x = fXaxis->FindFixBin(x);
    Int_t bin_y = fYaxis->FindFixBin(y);
    if(bin_x<1 || bin_x>fXaxis->GetNbins() || bin_y<1 || bin_y>fYaxis->GetNbins()) {
       histo->Error("Interpolate","Cannot interpolate outside histogram domain. (x: %f vs [%f,%f], y: %f vs [%f,%f])",x,fXaxis->GetBinLowEdge(1),fXaxis->GetBinLowEdge(fXaxis->GetNbins()+1),y,fYaxis->GetBinLowEdge(1),fYaxis->GetBinLowEdge(fYaxis->GetNbins()+1));
       return 0;
    }
    Int_t quadrant = 0; // CCW from UR 1,2,3,4
    // which quadrant of the bin (bin_P) are we in?
    dx = fXaxis->GetBinUpEdge(bin_x)-x;
    dy = fYaxis->GetBinUpEdge(bin_y)-y;
    if (dx<=fXaxis->GetBinWidth(bin_x)/2 && dy<=fYaxis->GetBinWidth(bin_y)/2)
    quadrant = 1; // upper right
    if (dx>fXaxis->GetBinWidth(bin_x)/2 && dy<=fYaxis->GetBinWidth(bin_y)/2)
    quadrant = 2; // upper left
    if (dx>fXaxis->GetBinWidth(bin_x)/2 && dy>fYaxis->GetBinWidth(bin_y)/2)
    quadrant = 3; // lower left
    if (dx<=fXaxis->GetBinWidth(bin_x)/2 && dy>fYaxis->GetBinWidth(bin_y)/2)
    quadrant = 4; // lower right
    switch(quadrant) {
    case 1:
       x1 = fXaxis->GetBinCenter(bin_x);
       y1 = fYaxis->GetBinCenter(bin_y);
       x2 = fXaxis->GetBinCenter(bin_x+1);
       y2 = fYaxis->GetBinCenter(bin_y+1);
       break;
    case 2:
       x1 = fXaxis->GetBinCenter(bin_x-1);
       y1 = fYaxis->GetBinCenter(bin_y);
       x2 = fXaxis->GetBinCenter(bin_x);
       y2 = fYaxis->GetBinCenter(bin_y+1);
       break;
    case 3:
       x1 = fXaxis->GetBinCenter(bin_x-1);
       y1 = fYaxis->GetBinCenter(bin_y-1);
       x2 = fXaxis->GetBinCenter(bin_x);
       y2 = fYaxis->GetBinCenter(bin_y);
       break;
    case 4:
       x1 = fXaxis->GetBinCenter(bin_x);
       y1 = fYaxis->GetBinCenter(bin_y-1);
       x2 = fXaxis->GetBinCenter(bin_x+1);
       y2 = fYaxis->GetBinCenter(bin_y);
       break;
    }
    Int_t bin_x1 = fXaxis->FindFixBin(x1);
    if(bin_x1<1) bin_x1=1;
    Int_t bin_x2 = fXaxis->FindFixBin(x2);
    if(bin_x2>fXaxis->GetNbins()) bin_x2=fXaxis->GetNbins();
    Int_t bin_y1 = fYaxis->FindFixBin(y1);
    if(bin_y1<1) bin_y1=1;
    Int_t bin_y2 = fYaxis->FindFixBin(y2);
    if(bin_y2>fYaxis->GetNbins()) bin_y2=fYaxis->GetNbins();

    Double_t q11;
    Double_t q12;
    Double_t q21;
    Double_t q22;

    // ===========================================================================
    // The code to find q11, q12, q21 and q22 was adjusted w.r.t. TH2::Interpolate
    // to account for using 3D histograms with one fixed dimension
    // ===========================================================================

    // X,Y variable and Z fixed
    if (xAxis == 1 && yAxis == 2){
      q11 = histo->GetBinContent(histo->GetBin(bin_x1,bin_y1,otherDimBin));
      q12 = histo->GetBinContent(histo->GetBin(bin_x1,bin_y2,otherDimBin));
      q21 = histo->GetBinContent(histo->GetBin(bin_x2,bin_y1,otherDimBin));
      q22 = histo->GetBinContent(histo->GetBin(bin_x2,bin_y2,otherDimBin));
    }
    // X,Z variable and Y fixed
    else if (xAxis == 1 && yAxis == 3){
      q11 = histo->GetBinContent(histo->GetBin(bin_x1,otherDimBin,bin_y1));
      q12 = histo->GetBinContent(histo->GetBin(bin_x1,otherDimBin,bin_y2));
      q21 = histo->GetBinContent(histo->GetBin(bin_x2,otherDimBin,bin_y1));
      q22 = histo->GetBinContent(histo->GetBin(bin_x2,otherDimBin,bin_y2));
    }
    // Y,Z variable and X fixed
    else if (xAxis == 2 && yAxis == 3){
      q11 = histo->GetBinContent(histo->GetBin(otherDimBin,bin_x1,bin_y1));
      q12 = histo->GetBinContent(histo->GetBin(otherDimBin,bin_x1,bin_y2));
      q21 = histo->GetBinContent(histo->GetBin(otherDimBin,bin_x2,bin_y1));
      q22 = histo->GetBinContent(histo->GetBin(otherDimBin,bin_x2,bin_y2));
    }
    // Y,X variable and Z fixed
    else if (xAxis == 2 && yAxis == 1){
      q11 = histo->GetBinContent(histo->GetBin(bin_y1,bin_x1,otherDimBin));
      q12 = histo->GetBinContent(histo->GetBin(bin_y1,bin_x2,otherDimBin));
      q21 = histo->GetBinContent(histo->GetBin(bin_y2,bin_x1,otherDimBin));
      q22 = histo->GetBinContent(histo->GetBin(bin_y2,bin_x2,otherDimBin));
    }
    // Z,X variable and Y fixed
    else if (xAxis == 3 && yAxis == 1){
      q11 = histo->GetBinContent(histo->GetBin(bin_y1,otherDimBin,bin_x1));
      q12 = histo->GetBinContent(histo->GetBin(bin_y1,otherDimBin,bin_x2));
      q21 = histo->GetBinContent(histo->GetBin(bin_y2,otherDimBin,bin_x1));
      q22 = histo->GetBinContent(histo->GetBin(bin_y2,otherDimBin,bin_x2));
    }
    // Z,Y variable and X fixed
    else if (xAxis == 3 && yAxis == 2){
      q11 = histo->GetBinContent(histo->GetBin(otherDimBin,bin_y1,bin_x1));
      q12 = histo->GetBinContent(histo->GetBin(otherDimBin,bin_y1,bin_x2));
      q21 = histo->GetBinContent(histo->GetBin(otherDimBin,bin_y2,bin_x1));
      q22 = histo->GetBinContent(histo->GetBin(otherDimBin,bin_y2,bin_x2));
    }
    else{
      histo->Error("Interpolate2D","Unsupported axis combination: (x,y)=(%d,%d) with one bin fixed",xAxis,yAxis);
      return 0;
    }
    
    Double_t d = 1.0*(x2-x1)*(y2-y1);
    f = 1.0*q11/d*(x2-x)*(y2-y)+1.0*q21/d*(x-x1)*(y2-y)+1.0*q12/d*(x2-x)*(y-y1)+1.0*q22/d*(x-x1)*(y-y1);
    return f;
  }

  double Interpolate(const TH1* histo, const double x, const double y, const double z)
  {

    // =======================================================================================
    // The ROOT Interpolate function throws an error when the provided value is:
    //   - below the bin center of the first bin on a given axis
    //   - above the bin center of the last bin on a given axis
    // We want to support these edge cases in our calibrations
    // =======================================================================================
    // If no edge cases are identified, we use TH3::Interpolate() from ROOT
    // If an edge case is found, we use 2D interpolation with the third dimension fixed:
    //   - Edge cases were previously tried with weights set including the under/overflow bins
    //     (to follow what TH2::Interpolate() does for boundaries)
    //   - In some cases, it performed quite poorly
    // =======================================================================================

    const TAxis* fXaxis = histo->GetXaxis();
    const TAxis* fYaxis = histo->GetYaxis();
    const TAxis* fZaxis = histo->GetZaxis();

    // Find the bin by bin edges
    Int_t ubx = fXaxis->FindFixBin(x);
    Int_t uby = fYaxis->FindFixBin(y);
    Int_t ubz = fZaxis->FindFixBin(z);
    
    // Check if the value(s) are outside of the bin range(s)
    if ( ubx < 1 || ubx > histo->GetNbinsX() || uby < 1 || uby > histo->GetNbinsY() || ubz < 1 || ubz > histo->GetNbinsZ() )
    {
       histo->Error("Interpolate","Cannot interpolate outside histogram domain. (x: %f vs [%f,%f], y: %f vs [%f,%f], z: %f vs [%f,%f])",x,fXaxis->GetBinLowEdge(1),fXaxis->GetBinLowEdge(histo->GetNbinsX()+1),y,fYaxis->GetBinLowEdge(1),fYaxis->GetBinLowEdge(histo->GetNbinsY()+1),z,fZaxis->GetBinLowEdge(1),fZaxis->GetBinLowEdge(histo->GetNbinsZ()+1));
       return 0;
    }

    // Check for edge cases:
    if( (ubx == 1 && x < fXaxis->GetBinCenter(ubx)) || (ubx == fXaxis->GetNbins() && x > fXaxis->GetBinCenter(ubx)))
      return RootHelpers::Interpolate2D(histo,y,z,2,3,ubx);
    else if( (uby == 1 && y < fYaxis->GetBinCenter(uby)) || (uby == fYaxis->GetNbins() && y > fYaxis->GetBinCenter(uby)))
      return RootHelpers::Interpolate2D(histo,x,z,1,3,uby);
    else if( (ubz == 1 && z < fZaxis->GetBinCenter(ubz)) || (ubz == fZaxis->GetNbins() && z > fZaxis->GetBinCenter(ubz)))
      return RootHelpers::Interpolate2D(histo,x,y,1,2,ubz);

    // If no edge cases identified, use ROOT TH3::Interpolate function
    return histo->Interpolate(x,y,z);
  }

} // end RootHelpers namespace
