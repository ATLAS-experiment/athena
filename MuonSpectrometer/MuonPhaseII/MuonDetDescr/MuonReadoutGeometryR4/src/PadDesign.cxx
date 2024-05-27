/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/PadDesign.h>
#include <GaudiKernel/SystemOfUnits.h>

namespace MuonGMR4{
    void PadDesign::print(std::ostream& ostr) const {
        ostr<<"Dimension  -- width x height [mm]: "<<halfWidth() * Gaudi::Units::mm<<" x ";
        ostr<<shortHalfHeight()<<"/"<<longHalfHeight()<<" [mm], ";
        if (hasStereoAngle()) ostr<<"stereo angle: "<<stereoAngle() / Gaudi::Units::deg<<", ";
        ostr<<"position first pad "<<Amg::toString(stripPosition(firstStripNumber()),1);
        ostr<<" *** Trapezoid edges "<<Amg::toString(cornerBotLeft(),1)<<" - "<<Amg::toString(cornerBotRight(), 1)<<" --- ";
        ostr<<Amg::toString(cornerTopLeft(), 1)<<" - "<<Amg::toString(cornerTopRight(), 1);
        ostr<<" -- numPadEta: "<<numPadEta()<<", numPadPhi: "<<numPadPhi()<<", pad height: "<<padHeight();
        ostr<<" -- firstPadPhiDiv: "<<firstPadPhiDiv()<<", anglePadPhi: "<<anglePadPhi()<<", padPhiShift: "<<padPhiShift();
        ostr<<" -- firstPadHeight: "<<firstPadHeight()<<", beamlineRadius: "<<beamlineRadius();
    }

    bool PadDesign::operator<(const PadDesign& other) const {
        return static_cast<const StripDesign&>(*this) < other;
    }

    void PadDesign::definePadRow(const double firstPadPhiDiv,
                                const int numPadPhi,
                                const double anglePadPhi,
                                const int padPhiShift) {

        m_firstPadPhiDiv = firstPadPhiDiv;
        m_numPadPhi = numPadPhi;
        m_anglePadPhi = anglePadPhi;
        m_padPhiShift = static_cast<double>(padPhiShift);
    };

    void PadDesign::definePadColumn(const double firstPadHeight,
                                const int numPadEta,
                                const double padHeight,
                                const int maxPadEta) {

        m_firstPadHeight = firstPadHeight;
        m_numPadEta = numPadEta;
        m_padHeight = padHeight;
        m_maxPadEta = maxPadEta;
    };

    void PadDesign::defineBeamlineRadius(const double radius) {
        m_radius = radius;
    };

    localCornerArray PadDesign::padCorners(const std::pair<int, int>& padEtaPhi) const {
        int padEta = padEtaPhi.first;
        int padPhi = padEtaPhi.second;
        localCornerArray padCorners = {make_array<Amg::Vector2D, 4>(Amg::Vector2D::Zero())};
        /// Variables to store the distance of the two sides of the pad w.r.t. the chamber origin
        double botEdge{0.};
        double topEdge{0.};
        /// Variables to store the distance of the four points of the pad in x direction w.r.t. the chamber origin
        double botLeftPoint{0.};
        double botRightPoint{0.};
        double topLeftPoint{0.};
        double topRightPoint{0.};
        /// Defining the top and the bottom edge of the active area depending on whether its a diamond or a trapezoid
        double maxBottom = /*yCutout()? (-2. * halfWidth() + yCutout()) :*/ -halfWidth();
        double maxTop = /*yCutout()? yCutout :*/ halfWidth();
        /// Calculating the distance of top and bottom sides of the pad w.r.t. the chamber origin
        if(padEta == 1) {
            botEdge = maxBottom;
            topEdge = botEdge + firstPadHeight();
        }
        else if(padEta > 1 && padEta <= numPadEta()) {
            botEdge = maxBottom + firstPadHeight() + (padEta - 2) * padHeight();
            topEdge = botEdge + padHeight();
            if(padEta == numPadEta()) topEdge = maxTop;
        }
        else {
            ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" The given pad Eta " << padEta << " is out of range. Maximum range is " << numPadEta());
            return padCorners;
        }

        /// Caculating the angular positions of the left and the right edges
        double phiRight = firstPadPhiDiv() + (padPhi - 2) * anglePadPhi();
        double phiLeft =  firstPadPhiDiv() + (padPhi - 1) * anglePadPhi();
        /// Defining tangent of the phi angles calculated above
        double tanRight = std::tan(phiRight * Gaudi::Units::deg);
        double tanLeft = std::tan(phiLeft * Gaudi::Units::deg);
        /// Calculating distance from the beamline to the bottom and top edges of the pad
        double botBase = botEdge + beamlineRadius();
        double topBase = topEdge + beamlineRadius();
        /// Calculating the distance of the four corners of the pad in x direction w.r.t. the chamber origin
        botLeftPoint = -botBase * tanLeft;
        botRightPoint = -botBase * tanRight;
        topLeftPoint = -topBase * tanLeft;
        topRightPoint = -topBase * tanRight;
        /// Calculating the cosine of the angular positions to calculate the staggering in x direction w.r.t. the chamber origin
        double cosRight = botBase / std::hypot(botRightPoint, botBase);
        double cosLeft = botBase / std::hypot(botLeftPoint, botBase);
        /// Adjusting the four corners of the pads for staggering in x direction w.r.t. the chamber origin
        botLeftPoint += padPhiShift() * cosLeft;
        botRightPoint += padPhiShift() * cosRight;
        topLeftPoint += padPhiShift() * cosLeft;
        topRightPoint += padPhiShift() * cosRight;
        /// Outer edges of the trapezoid do not undergo staggering. Hence the if conditions.
        double adjHeight = /*yCutout()? 2 * halfWidth() - yCutout() :*/ 2 * halfWidth();
        if(padPhi == 1) {
            botRightPoint = shortHalfHeight() + ((longHalfHeight() - shortHalfHeight()) * (botEdge - maxBottom) / adjHeight);
            topRightPoint = shortHalfHeight() + ((longHalfHeight() - shortHalfHeight()) * (topEdge - maxBottom) / adjHeight);
        }
        if(padPhi == numPadPhi()) {
            botLeftPoint = -shortHalfHeight() - ((longHalfHeight() - shortHalfHeight()) * (botEdge - maxBottom) / adjHeight);
            topLeftPoint = -shortHalfHeight() - ((longHalfHeight() - shortHalfHeight()) * (topEdge - maxBottom) / adjHeight);
        }
        /// Adjusting the outer edges of the pads in diamond chambers (QL3)
        /// There are pads in the outer columns that may contain five vertices when the topEdge of the pad is above the
        /// gasGap origin and the bottom edge is below. This is only the case when yCutout is nonzero i.e. L3 sector.
        /// In our logic, we are keeping only four corners for all the pads and essentially ignoring the fifth vertex 
        /// for these specific cases because the technical drawings suggest that the active ignored is very small and
        /// the effect on reconstruction and digitization will be negligible.
/*
        if (yCutout && topEdge > 0) {
            if (padPhi == 1) {
                topRightPoint = longHalfHeight();
                if (botEdge > 0) botRightPoint = longHalfHeight();
            }
            if (padPhi == numPadPhi()) {
                topLeftPoint = -longHalfHeight();
                if (botEdge > 0) botLeftPoint = -longHalfHeight();
            }
        }
*/
        /// Swapping the edges and the points in a mirror fashion if our initial assumptions about the left/Right 
        /// and top/Bottom are false.
        if (botEdge > topEdge) {
        ATH_MSG_VERBOSE("Swap top and bottom side "<<padEtaPhi.first<<"/"<<padEtaPhi.second);
        std::swap(botEdge, topEdge);
        }
        if (botLeftPoint > botRightPoint) {
            ATH_MSG_VERBOSE("Swap bottom left and right points "<<padEtaPhi.first<<"/"<<padEtaPhi.second);
            std::swap(botLeftPoint, botRightPoint);
        }
        if (topLeftPoint > topRightPoint) {
            ATH_MSG_VERBOSE("Swap top left and right points "<<padEtaPhi.first<<"/"<<padEtaPhi.second);
            std::swap(topLeftPoint, topRightPoint);
        }
        padCorners[botLeft] = Amg::Vector2D(botLeftPoint, botEdge);
        padCorners[botRight] = Amg::Vector2D(botRightPoint, botEdge);
        padCorners[topLeft] = Amg::Vector2D(topLeftPoint, topEdge);
        padCorners[topRight] = Amg::Vector2D(topRightPoint, topEdge);
        return padCorners; 
    };
}

