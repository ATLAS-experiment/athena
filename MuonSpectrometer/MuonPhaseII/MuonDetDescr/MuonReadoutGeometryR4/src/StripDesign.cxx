/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonReadoutGeometryR4/StripDesign.h>
#include <GeoModelHelpers/TransformSorter.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <climits>
namespace {
    constexpr double tolerance = 0.001 * Gaudi::Units::mm;
}
/// Helper macro to facilliate the ordering
#define ORDER_PROP(PROP)                                            \
      {                                                             \
        if (std::abs(1.*PROP - 1.*other.PROP) > tolerance) {        \
            return PROP < other.PROP;                               \
        }                                                           \
      }

namespace MuonGMR4{
    StripDesign::StripDesign():
        AthMessaging{"MuonStripDesign"} {}
    bool operator<(const StripDesignPtr&a, const StripDesignPtr& b) {
        return (*a) < (*b);
    }
    bool StripDesign::operator<(const StripDesign& other) const {
        ORDER_PROP(firstStripNumber());
        ORDER_PROP(numStrips());
        ORDER_PROP(stripPitch());
        ORDER_PROP(stripWidth());
        ORDER_PROP(halfWidth());
        ORDER_PROP(longHalfHeight());
        ORDER_PROP(shortHalfHeight());
        ORDER_PROP(stereoAngle());
        ORDER_PROP(isFlipped());
        static const GeoTrf::TransformSorter trfSorter{};
        return trfSorter(m_firstStripPos, other.m_firstStripPos);
    }
    std::ostream& operator<<(std::ostream& ostr, const StripDesign& design) {
        design.print(ostr);
        return ostr;
    }
    void StripDesign::print(std::ostream& ostr) const {
        ostr<<"Strip -- number: "<<numStrips()<<", ";        
        ostr<<"pitch: "<<stripPitch()<<", ";
        ostr<<"width: "<<stripWidth()<<", ";
        ostr<<"Dimension  -- width x height [mm]: "<<halfWidth() * Gaudi::Units::mm<<" x ";
        ostr<<shortHalfHeight()<<"/"<<longHalfHeight()<<" [mm], ";
        if (hasStereoAngle()) ostr<<"stereo angle: "<<stereoAngle() / Gaudi::Units::deg<<", ";
        ostr<<"position first strip "<<Amg::toString(center(firstStripNumber()).value_or(Amg::Vector2D::Zero()),1);
        ostr<<" *** Trapezoid edges "<<Amg::toString(cornerBotLeft(),1)<<" - "<<Amg::toString(cornerBotRight(), 1)<<" --- ";
        ostr<<Amg::toString(cornerTopLeft(), 1)<<" - "<<Amg::toString(cornerTopRight(), 1);
    }
    void StripDesign::defineTrapezoid(double HalfShortY, double HalfLongY, double HalfHeight, double sAngle){
        defineTrapezoid(HalfShortY,HalfLongY, HalfHeight);
        setStereoAngle(sAngle);
    }

    void StripDesign::setStereoAngle(double sAngle) {
        if (std::abs(sAngle) < std::numeric_limits<float>::epsilon()) {
            return;
        }
        m_stereoAngle = sAngle;
        m_hasStereo = true;
        m_etaToStereo = Eigen::Rotation2D{-stereoAngle()};
        m_stereoToEta = Eigen::Rotation2D{stereoAngle()};
        m_stripDir = m_stereoToEta * m_stripDir;
        m_stripNormal = m_stereoToEta * m_stripNormal;
    }

    void StripDesign::defineTrapezoid(double HalfShortY, double HalfLongY, double HalfHeight) {
        m_bottomLeft = Amg::Vector2D{-HalfHeight, -HalfShortY};
        m_bottomRight = Amg::Vector2D{HalfHeight, -HalfLongY};
        m_topLeft = Amg::Vector2D{-HalfHeight, HalfShortY};
        m_topRight = Amg::Vector2D{HalfHeight, HalfLongY};

        m_shortHalfY = HalfShortY;
        m_longHalfY = HalfLongY;
        m_halfX = HalfHeight;  
        m_lenSlopEdge = std::hypot(2.*HalfHeight, HalfShortY - HalfLongY);
        resetDirCache();
        m_isFlipped = false;
    }
    void StripDesign::defineDiamond(double HalfShortY, double HalfLongY, 
                                    double HalfHeight, double yCutout) {
        /// define a trapezoid region to preserve the functionality of intersect functions in StripDesign class
        double HalfLongYuncut = HalfLongY + yCutout * (HalfLongY - HalfShortY)/(2*HalfHeight - yCutout);
        defineTrapezoid(HalfShortY, HalfLongYuncut, HalfHeight);
        m_yCutout = yCutout;
        m_longHalfY = HalfLongY;
        m_cutLongEdge = m_yCutout * (m_longHalfY - m_shortHalfY)/(2*m_halfX - m_yCutout);
    }
    void StripDesign::flipTrapezoid() {
        if (m_isFlipped) {
            ATH_MSG_WARNING("It's impossible to flip a trapezoid twice. Swap short and long lengths");
            return;
        }
        m_isFlipped = true;

        m_bottomLeft = Amg::Vector2D{-m_shortHalfY, -m_halfX};
        m_bottomRight = Amg::Vector2D{m_shortHalfY, -m_halfX};
        /// yCutOut defined for the sTGC detectors. Move the edges to the uncut trapezoid version
        /// Deal with the cutOut in the intersect       
        m_topLeft = Amg::Vector2D{-m_longHalfY - m_cutLongEdge, m_halfX};
        m_topRight = Amg::Vector2D{m_longHalfY + m_cutLongEdge, m_halfX};
        resetDirCache();
    }

    void StripDesign::defineStripLayout(Amg::Vector2D&& posFirst,
                                        const double stripPitch,
                                        const double stripWidth,
                                        const int numStrips,
                                        const int numFirst) {        
        m_channelShift = numFirst;
        m_numStrips = numStrips;
        m_stripPitch = stripPitch;
        m_stripWidth = stripWidth;
        m_firstStripPos = std::move(posFirst);
    }
    void StripDesign::resetDirCache() {
        m_dirTopEdge.release();
        m_dirBotEdge.release();
        m_dirLeftEdge.release();
        m_dirRightEdge.release();
    }

}
#undef ORDER_PROP 
