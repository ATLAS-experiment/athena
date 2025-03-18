/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonReadoutGeometry/MuonPadDesign.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/GlobalUtilities.h"
#include <ext/alloc_traits.h>
#include <stdexcept>
using MuonGM::MuonPadDesign;

MuonPadDesign::MuonPadDesign(): AthMessaging{"MuonPadDesign"}{}
bool MuonPadDesign::withinSensitiveArea(const Amg::Vector2D& pos) const {
    double top_H1 = maxSensitiveY();
    double bot_H2 = minSensitiveY();
    double max_x = maxAbsSensitiveX(pos.y());
    bool y_in_range = (pos.y() <= top_H1 and pos.y() >= bot_H2);
    bool x_in_range = std::abs(pos.x()) <= max_x + 0.01;
    return y_in_range and x_in_range;
}
//----------------------------------------------------------
double MuonPadDesign::minSensitiveY() const { return yCutout ? -(Size - yCutout) : -0.5 * Size; }
//----------------------------------------------------------
double MuonPadDesign::maxSensitiveY() const { return yCutout ? yCutout : 0.5 * Size; }
//----------------------------------------------------------
double MuonPadDesign::maxAbsSensitiveX(const double& y) const {
    double half_openingAngle = sectorOpeningAngle / 2.0;
    if (isLargeSector && yCutout) {  // if QL3
        if (y > 0)                   // In cutout region
            return 0.5 * lPadWidth;
        else
            return y * std::tan(half_openingAngle * CLHEP::degree) + 0.5 * lPadWidth;
    } else
        return (y - Size * 0.5) * std::tan(half_openingAngle * CLHEP::degree) + 0.5 * lPadWidth;

    return -1;
}
//----------------------------------------------------------
std::pair<int, int> MuonPadDesign::channelNumber(const Amg::Vector2D& pos) const {
    /* Changes in this package are due to new geometry implementations
     * Correct active area position and inclusion of proper QL3 shape.
     * coordinates (0,0) now point to the center of the active region, not gas volume
     * for QL3, where ycutout !=0, (0,0) is at start of ycutout */

    // perform check of the sensitive area
    std::pair<int, int> result(-1, -1);

    // padEta
    double y1 = yCutout ? Size - yCutout + pos.y() : 0.5 * Size + pos.y();  // distance from small edge to hit
    double padEtadouble;
    int padEta = 0;
    // padPhi
    // To obtain the pad number of a given position, its easier to apply the
    // pad staggering to the position instead of the pad: apply -PadPhiShift.
    // Pad corners and positions however fully account for their staggering
    double locPhi = std::atan(-1.0 * pos.x() / (radialDistance + pos.y())) / CLHEP::degree;
    double maxlocPhi = std::atan(maxAbsSensitiveX(pos.y()) / (radialDistance + pos.y())) / CLHEP::degree;
    double fuzziedX = pos.x() - 1. * PadPhiShift * std::cos(locPhi * CLHEP::degree);
    double fuzziedlocPhi = std::atan(-1.0 * fuzziedX / (radialDistance + pos.y())) / CLHEP::degree;

    bool below_half_length = (y1 < 0);
    bool outside_phi_range = (std::abs(locPhi) > maxlocPhi) || (std::abs(fuzziedlocPhi) > maxlocPhi);

    if (withinSensitiveArea(pos) && !below_half_length) {
      if (y1 > firstRowPos) {
        //+1 for firstRow, +1 because a remainder means another row (3.1=4)
        padEtadouble = ((y1 - firstRowPos) / inputRowPitch) + 1 + 1;
        padEta = padEtadouble;
      } else if (y1 >= 0) {
        padEta = 1;
      }
      double padPhidouble;
      // These are separated as the hits on the pads closest to the side edges are not fuzzied
      // We must do a correction in order to stay consistent with indexing
      if (outside_phi_range)
        padPhidouble = (locPhi - firstPhiPos) / inputPhiPitch;
      else  // Look for the index of the fuzzied hit
        padPhidouble = (fuzziedlocPhi - firstPhiPos) / inputPhiPitch;
      int padPhi = padPhidouble + 2;  //(+1 because remainder means next column e.g. 1.1=2, +1 so rightmostcolumn=1)

      // adjust indices if necessary
      if (padEta == nPadH + 1) { padEta -= 1; }                 // the top row can be bigger, therefore it is really in the nPadH row.
      if (padPhi == 0) { padPhi = 1; }                          // adjust rightmost
      if (padPhi == nPadColumns + 1) { padPhi = nPadColumns; }  // adjust lefmost

      // final check on range
      bool ieta_out_of_range = (padEta > nPadH + 1);
      bool iphi_out_of_range = (padPhi < 0 || padPhi > nPadColumns + 1);
      bool index_out_of_range = ieta_out_of_range or iphi_out_of_range;
      if (index_out_of_range) {
          std::stringstream sstr{};
          if (ieta_out_of_range){
            sstr<<__FILE__<<":"<<__LINE__<<" "<<__func__<<"() eta out of range "
                <<Amg::toString(pos, 2)<<" (ieta, iphi) = ("<<padEta<<","<<padPhi<<").";
          } else {
            sstr<<__FILE__<<":"<<__LINE__<<" "<<__func__<<"() phi out of range "
                <<Amg::toString(pos, 2)<<" (ieta, iphi) = ("<<padEta<<","<<padPhi<<").";
          }
          throw std::runtime_error(sstr.str());
      
      } else {
          result = std::make_pair(padEta, padPhi);
      }
    }
    return result;
}

//----------------------------------------------------------
bool MuonPadDesign::channelPosition(const int channel, Amg::Vector2D& pos) const {
    return channelPosition(etaPhiId(channel), pos);
}
bool MuonPadDesign::channelPosition(const std::pair<int, int>& pad, Amg::Vector2D& pos) const {
    CornerArray corners{make_array<Amg::Vector2D, 4>(Amg::Vector2D::Zero())};
    channelCorners(pad, corners);
    pos = 0.25 * (corners[botLeft] + corners[botRight] + corners[topLeft] + corners[topRight]); 
    return true;
}
Amg::Vector2D MuonPadDesign::distanceToPad(const Amg::Vector2D& pos, int channel) const {
    CornerArray corners{make_array<Amg::Vector2D, 4>(Amg::Vector2D::Zero())};
    channelCorners(channel, corners);

    Amg::Vector2D leftEdge = corners[topLeft] - corners[botLeft];
    const double lenLeft = std::hypot(leftEdge.x(), leftEdge.y());
    leftEdge /= lenLeft;
    const double leftIsect = Amg::intersect<2>(pos, Amg::Vector2D::UnitX(),
                                                 corners[botLeft], leftEdge).value_or(1.e9);

    const Amg::Vector2D leftPad = corners[botLeft] + leftIsect * leftEdge;
    const Amg::Vector2D rightPad = corners[botRight] + leftIsect * (corners[topRight] - corners[botRight]).unit();
    const double deltaX = pos.x() - leftPad.x();
    const double lenX = rightPad.x() - leftPad.x();
       
    /// In terms of y the hit could be in the pad
    if (leftIsect >= 0.  && leftIsect <= lenLeft) {
       /// Hit is inside the pad
       if (deltaX >= 0. && deltaX < lenX) {
            return Amg::Vector2D::Zero();
       } else if (deltaX < 0.) {
            return deltaX * Amg::Vector2D::UnitX();
       }
       return (deltaX - lenX) * Amg::Vector2D::UnitX();
    }
    if (deltaX > 0.  && deltaX < lenX) {
        return (leftIsect < 0 ? corners[botRight].y() - pos.y() 
                              : pos.y() - corners[topRight].y())* Amg::Vector2D::UnitY();
    }
    return (leftIsect < 0 ? corners[botRight].y() - pos.y() 
                              : pos.y() - corners[topRight].y())* Amg::Vector2D::UnitY() +
           (deltaX < 0. ? deltaX  : (deltaX - lenX) )* Amg::Vector2D::UnitX();

}

//----------------------------------------------------------
bool MuonPadDesign::channelCorners(const int channel, CornerArray& corners) const {
    return channelCorners(etaPhiId(channel), corners);
}
bool MuonPadDesign::channelCorners(const std::pair<int, int>& pad, CornerArray& corners) const {
    // DG-2015-11-30: todo check whether the offset subtraction is still needed
    int iEta = pad.first;   // -1 + padEtaMin;
    int iPhi = pad.second;  //  -1 + padPhiMin;
    // bool invalid_indices = iEta<1 || iPhi<1; // DG-2015-11-30 do we still need to check this?
    // if(invalid_indices) return false;
    // double yBot = -0.5*Length + firstRowPos + ysFrame + iEta*inputRowPitch;
    // double yTop = -0.5*Length + firstRowPos + ysFrame + (iEta+1)*inputRowPitch;

    ////// ASM-2015-12-07 : New Implementation
    double yBot = 0., yTop = 0.;
    if (iEta == 1) {
        yBot = yCutout ? -(Size - yCutout) : -0.5 * Size;
        yTop = yBot + firstRowPos;
    } else if (iEta > 1) {
        yBot = yCutout ? -(Size - yCutout) + firstRowPos + (iEta - 2) * inputRowPitch 
                       : -0.5 * Size + firstRowPos + (iEta - 2) * inputRowPitch;
        yTop = yBot + inputRowPitch;
        if (iEta == nPadH) yTop = maxSensitiveY();
    } else {  // Unkwown ieta
        return false;
    }
    ////// ASM-2015-12-07 : New Implementation

    // restrict y to the module sensitive area
    double minY = minSensitiveY();
    double maxY = maxSensitiveY();
    if (yBot < minY) yBot = minY;
    if (yTop > maxY) yTop = maxY;

    // here L/R are defined as if you were looking from the IP to the
    // detector (same a clockwise/counterclockwise phi but shorter)
    double phiRight = firstPhiPos + (iPhi - 2) * inputPhiPitch;
    double phiLeft = firstPhiPos + (iPhi - 1) * inputPhiPitch;

    const double tanRight = std::tan(phiRight *CLHEP::degree);
    const double tanLeft = std::tan(phiLeft *CLHEP::degree);
    double xBotRight = -(yBot + radialDistance) * tanRight;
    double xBotLeft = -(yBot + radialDistance) * tanLeft;
    double xTopRight = -(yTop + radialDistance) * tanRight;
    double xTopLeft = -(yTop + radialDistance) * tanLeft;
  
    const double cosRight = (radialDistance + yBot) / std::hypot(xBotRight, (radialDistance + yBot));
    const double cosLeft = (radialDistance + yBot) / std::hypot(xBotLeft, (radialDistance + yBot));
    
    xBotRight += 1.*PadPhiShift*cosRight;
    xBotLeft += 1.*PadPhiShift*cosLeft;
    xTopRight += 1.*PadPhiShift*cosRight;
    xTopLeft += 1.*PadPhiShift*cosLeft;

    // For the R3 geometry converted from Phase II, we are shifting the gasgap center
    // to the center of the gap for L3, originally defined at the cutout base.
    // Hence, we are defining an offset, yCutoutOffset to be used in the pad corner calculations.
    double yCutoutOffset{0.};
    if(isConvertedFromPhaseII) {
        yCutoutOffset = 24.74;
    }
    // Adjust outer columns
    // No staggering from fuziness in the outer edges
    if (iPhi == 1) {
        double yLength = yCutout ? Size - yCutout + yCutoutOffset: Size;
        xBotRight = 0.5 * (sPadWidth + (lPadWidth - sPadWidth) * (yBot - minY) / yLength);
        xTopRight = 0.5 * (sPadWidth + (lPadWidth - sPadWidth) * (yTop - minY) / yLength);
    }
    if (iPhi == nPadColumns) {
        double yLength = yCutout ? Size - yCutout + yCutoutOffset: Size;
        xBotLeft = -0.5 * (sPadWidth + (lPadWidth - sPadWidth) * (yBot - minY) / yLength);
        xTopLeft = -0.5 * (sPadWidth + (lPadWidth - sPadWidth) * (yTop - minY) / yLength);
    }

    // Adjust for cutout region
    if (yCutout && (yTop - yCutoutOffset) > 0) {
        float cutoutXpos = 0.5 * lPadWidth;
        if (iPhi == 1) {
            xTopRight = cutoutXpos;
            if (yBot - yCutoutOffset > 0) xBotRight = cutoutXpos;
        } else if (iPhi == nPadColumns) {
            xTopLeft = -1.0 * cutoutXpos;
            if (yBot - yCutoutOffset > 0) xBotLeft = -1.0 * cutoutXpos;
        }
    }
    if (yBot > yTop) {
        ATH_MSG_VERBOSE("Swap top and bottom side "<<pad.first<<"/"<<pad.second);
        std::swap(yBot, yTop);
    }
    if (xBotLeft > xBotRight) {
        ATH_MSG_VERBOSE("Swap bottom left and right points "<<pad.first<<"/"<<pad.second);
        std::swap(xBotLeft, xBotRight);
    }
    if (xTopLeft > xTopRight) {
        ATH_MSG_VERBOSE("Swap top left and right points "<<pad.first<<"/"<<pad.second);
        std::swap(xTopLeft, xTopRight);
    }
    corners[botLeft] = Amg::Vector2D(xBotLeft, yBot);
    corners[botRight] = Amg::Vector2D(xBotRight, yBot);
    corners[topLeft] = Amg::Vector2D(xTopLeft, yTop);
    corners[topRight] = Amg::Vector2D(xTopRight, yTop);
    return true;
}
//----------------------------------------------------------
