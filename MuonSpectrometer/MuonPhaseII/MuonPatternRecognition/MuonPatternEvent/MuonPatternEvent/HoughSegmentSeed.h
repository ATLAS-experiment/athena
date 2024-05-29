/// copyright am arsch

#ifndef MUONR4__HOUGHSEGMENTSEED__H
#define MUONR4__HOUGHSEGMENTSEED__H

#include "MuonPatternEvent/HoughMaximum.h"

namespace MuonR4 {
/// @brief Representation of a segment seed (a fully processed hough maximum) produced
/// by the hough transform. 
/// @tparam HitType: Data type encoding the hits used in the transform
template <class HitType>
class HoughSegmentSeed_impl : public HoughMaximum_impl<HitType> {
   public:
    /// @brief Constructor to write a segment seed from an eta maximum and a valid
    /// phi extension. 
    /// @param tanTheta: tan(theta) from the eta-transform
    /// @param interceptY: y axis intercept from the eta-transform
    /// @param tanPhi: tan(phi) from the phi-extension
    /// @param interceptX: x axis intercept from the phi-extension
    /// @param counts: (weighted) counts for the given hough maximum
    /// @param hits: Measurements on this maximum
    HoughSegmentSeed_impl(double tanTheta, double interceptY, double tanPhi,
                     double interceptX, double counts,
                     std::vector<HitType>&& hits)
        : HoughMaximum_impl<HitType>(tanTheta, interceptY, counts, std::move(hits)),
          m_tanPhi(tanPhi),
          m_interceptX(interceptX) {
        m_hasPhiExt = true;
    }

    /// @brief Constructor to write a segment seed from an eta maximum without 
    /// a valid phi extension
    /// @param toCopy: Eta maximum 
    HoughSegmentSeed_impl(const HoughMaximum_impl<HitType>& toCopy) : HoughMaximum_impl<HitType>(toCopy) {
        m_hasPhiExt = false;
    }

    /// @brief getter
    /// @return  the angle from the phi extension
    double tanPhi() const { return m_tanPhi; }

    /// @brief getter
    /// @return  the intercept from the phi extension
    double interceptX() const { return m_interceptX; }

    /// @brief check whether the segment seed includes a 
    /// valid phi extension
    /// @return true if an extension exists, false if 
    /// we are dealing with a pure eta maximum
    bool hasPhiExtension() const { return m_hasPhiExt; }

   private:
    double m_tanPhi{0.};      // angle from phi extension
    double m_interceptX{0.};  // intercept from phi extension
    bool m_hasPhiExt{false};    // flag indicating presence of phi extension
};

}  // namespace MuonR4

#endif
