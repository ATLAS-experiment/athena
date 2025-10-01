/// copyright am arsch

#ifndef MUONR4_MUONPATTERNEVENT_HOUGHSEGMENTSEED__H
#define MUONR4_MUONPATTERNEVENT_HOUGHSEGMENTSEED__H

#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"


namespace MuonR4 {
/// @brief Representation of a segment seed (a fully processed hough maximum) produced
/// by the hough transform. 

class SegmentSeed {
   public:
    using HitType = HoughHitType;
    using Parameters = SegmentFit::Parameters;
    
    /// @brief Constructor to write a segment seed from an eta maximum and a valid
    /// phi extension. 
    /// @param tanTheta: tan(theta) from the eta-transform
    /// @param interceptY: y axis intercept from the eta-transform
    /// @param tanPhi: tan(phi) from the phi-extension
    /// @param interceptX: x axis intercept from the phi-extension
    /// @param counts: (weighted) counts for the given hough maximum
    /// @param hits: Measurements on this maximum
    /// @param bucket: Space point bucket out of which the seed is built
    SegmentSeed(double tanTheta, double interceptY, double tanPhi,
                double interceptX, double counts,
                std::vector<HitType>&& hits,
                const SpacePointBucket* bucket);
                  

    /// @brief Constructor to write a segment seed from an eta maximum without 
    /// a valid phi extension
    /// @param toCopy: Eta maximum 
    SegmentSeed(const HoughMaximum& toCopy);

    /// @brief Returns the angle from the phi extension
    double tanPhi() const;
    /// @brief Returns the intercept from the phi extension
    double interceptX() const;  
    /// @brief Returns the angular coordinate of the eta transform
    double tanTheta() const;
    /// @brief Returns the intercept coordinate of the eta transform
    double interceptY() const;
    /// @brief Returns the parameter array
    const Parameters& parameters() const;

    /// @return Returns the number of counts
    double getCounts() const;
    /// @brief Returns the list of assigned hits
    const std::vector<HitType>& getHitsInMax() const;
    
    /// @brief Returns the bucket out of which the seed was formed
    const SpacePointBucket* parentBucket() const;
    /// @brief Returns the associated chamber
    const MuonGMR4::SpectrometerSector* msSector() const;

    /// @brief check whether the segment seed includes a 
    /// valid phi extension
    /// @return true if an extension exists, false if 
    /// we are dealing with a pure eta maximum
    bool hasPhiExtension() const;

    /** @brief Returns the position of the seed in the sector frame */
    Amg::Vector3D localPosition() const;
    /** @brief Returns the direction of the seed in the sector frame */
    Amg::Vector3D localDirection() const; 
   private:
        /** @brief Set of defining parameters */
        Parameters m_pars{Parameters::Zero()};
        /** @brief Pointer to the parent */
        const SpacePointBucket* m_parent{nullptr};
        /** @brief List of associated hits */
        std::vector<HitType> m_hits{};
        /** @brief Does the sed have a phi extension */
        bool m_hasPhiExt{false};
        /** @brief Effective countsfrom the hough seed */
        double m_counts{0.};
};

}  // namespace MuonR4

#endif
