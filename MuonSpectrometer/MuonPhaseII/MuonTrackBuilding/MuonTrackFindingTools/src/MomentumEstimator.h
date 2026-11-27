/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTOOLS_MOMENTUMESTIMATOR_H
#define MUONTRACKFINDINGTOOLS_MOMENTUMESTIMATOR_H

#include "MuonTrackEvent/MsTrackSeed.h"

#include "Acts/MagneticField/MagneticFieldProvider.hpp"

#include <functional>
#include <algorithm>

namespace MuonR4 {

    class MomentumEstimator {
        public:
            /** @brief Function returning the projection direction of a muon segment */
            using WireDirection_t = std::function<Acts::Vector3(const Acts::GeometryContext& tgContext,
                                                                const xAOD::MuonSegment& segment)>;
            /** @brief Function returning the transform of a muon segment, where the covariance is defined */
            using SectorFrame_t = std::function<const Acts::Transform3&(const Acts::GeometryContext& tgContext,
                                                                        const xAOD::MuonSegment& segment)>;

            /** @brief Configuration of the estimator */
            struct Config {
                /** @brief Number of steps between two segments to integrate the magnetic field */
                unsigned nFieldSteps{30u};
                /** @brief Re-fit the bending plane when one segment with phi has been removed */
                bool refitBendingPlane{false};
                /** @brief Do not use the NSW segments for the bending plane if other segments have phi layers */
                bool skipNswInBendingPlane{false};
                /** @brief Sum of field integrals along the track below which the segments do not 
                           measure the bending and the beam spot is added to the fit */
                double minBendingPower{0.1 * Acts::UnitConstants::T * Acts::UnitConstants::m};
                /** @brief Recovery of a wrong segment in the q*p fit: fit chi2 / ndf above which it starts, 
                 *         and factor by which the dropped segment must reduce chi2 / ndf against any other 
                 *         removal and the fit. */
                double outlierChi2{10.};
                /** @brief Max number of dropped segments allowed during recovery */
                unsigned maxDroppedSegments{3u};
                /** @brief Prior sigmas of the shift of the bending plane fitted with q*p   
                 *         for barrel seeds (68% of the error of the plane on truth, PU0) */
                double planeShiftSigmaB{40. * Acts::UnitConstants::mm};
                /** @brief Prior sigmas of the shift of the bending plane fitted with q*p
                 *         for endcap seeds (68% of the error of the plane on truth, PU0) */
                double planeShiftSigmaE{7. * Acts::UnitConstants::mm};
                /** @brief Prior sigmas of the rotation of the bending plane (at the mean rho of the seed)
                 *         fitted with q*p for barrel seeds (68% of the error of the plane on truth, PU0) */
                double planeRotSigmaB{25. * Acts::UnitConstants::mrad};
                /** @brief Prior sigmas of the rotation of the bending plane (at the mean rho of the seed)
                 *         fitted with q*p for endcap seeds (68% of the error of the plane on truth, PU0) */
                double planeRotSigmaE{5. * Acts::UnitConstants::mrad};
                /** @brief Function returning the wire direction of a muon segment */
                WireDirection_t wireDir{};
                /** @brief Function returning the envelope of a muon segment */
                SectorFrame_t sectFrame{};
                /** @brief Pointer to the magnetic field provider */
                std::shared_ptr<const Acts::MagneticFieldProvider> bField{nullptr};
            };

            MomentumEstimator(Config&& config,
                              std::unique_ptr<const Acts::Logger> actsLogger = Acts::getDefaultLogger(
                                    "MomentumEstimator", Acts::Logging::Level::INFO));

            /** @brief Result of the momentum fit of a seed in the bending plane */
            struct Result {
                /** @brief The estimated q/p value, in [e/GeV] */
                double qOverP{0.};
                /** @brief The chi2 of the fit */
                double chi2{0.};    
                /** @brief The number of degrees of freedom of the fit */
                uint8_t ndf{0};  
                /** @brief chi2 / ndf of the fit */
                double redChi2() const { return chi2 / std::max(1., static_cast<double>(ndf)); }
            };
            /** @brief Abbreviation for the result of the momentum fit */
            using Result_t = std::optional<Result>;

            /** @brief Beamspot information used in the recovery */
            struct BeamSpot {
                /** Global Position */
                Acts::Vector3 position{};
                /** Longitudinal Size */
                double sigmaZ{0.};        
            };

            /** @brief Momentum fit of the seed in the bending plane
             *  @param tgContext: GeometryContext for alignment
             *  @param mfContext: Magnetic field context to access the field
             *  @param seed: The track seed
             *  @param withRecovery: Apply the recovery (leave-one-out of a wrong segment, beam spot in place of the
             *                       inner station); without it, the plain fit of all the segments of the seed
             *  @param bs: Beamspot information, if available */
            Result_t fitQoverP(const Acts::GeometryContext& tgContext,
                               const Acts::MagneticFieldContext& mfContext,
                               const MsTrackSeed& seed,
                               const bool withRecovery,
                               const std::optional<BeamSpot>& bs = std::nullopt) const;

            /** @brief Support method to assess if the fit is reliable
             *  @param res: Fit result */
            bool isGoodFit(const Result_t& res) const {
                return res && res->redChi2() < m_cfg.outlierChi2;
            }
        private:
            /** @brief Components of the segment projection onto the bending plane */
            enum class iSegProj : std::uint8_t {
                eRho = 0,   // Radial position of the segment in the bending plane (along eR)
                eZ = 1,     // Z position of the segment in the bending plane (along beam line)
                eAlpha = 2, // Angle of the segment in the bending plane (from z towards eR)
                nProj = 3   // Number of projection components
            };
            static constexpr std::size_t s_nSegProj {Acts::toUnderlying(iSegProj::nProj)};

            /** @brief The easurements each segment provides in the bending plane */
            enum class iSegMeas : std::uint8_t {
                eU = 0,     // Offset of the segment across the bending plane (perpendicular to its direction)
                eAlpha = 1, // Angle of the segment in the bending plane
                nMeas = 2   // Number of measurements for a segment
            };
            static constexpr std::size_t s_nSegMeas {Acts::toUnderlying(iSegMeas::nMeas)};

            /** @brief The nuisance parameters in the fit */
            enum class iNuisance : std::uint8_t {
                ePlaneShift = 0, // Shift of the bending plane along its normal
                ePlaneRot = 1,   // Rotation of the bending plane around the z axis
                nPars = 2        // Number of nuisance parameters
            };
            static constexpr std::size_t s_nNuisance {Acts::toUnderlying(iNuisance::nPars)};

            /** @brief Magnetic field integrals */
            enum class iFieldInt : std::uint8_t {
                eS = 0,      // path length along the trajectory
                eB1 = 1,     // int_0^s b ds
                eB2 = 2,     // int_0^s (s - s') b ds'
                nInts = 3    // number of field integrals
            };
            static constexpr std::size_t s_nFieldInt {Acts::toUnderlying(iFieldInt::nInts)};

            /** @brief Enum for the fit parameters + nuisances */
            enum class iFitPars : std::uint8_t {
                eU0 = 0,          // Initial offset of the trajectory in the bending plane
                eAlpha0 = 1,      // Initial angle of the trajectory in the bending plane
                eK = 2,           // q/p [e/GeV]
                ePlaneShift = 3,  // Shift of the bending plane along its normal direction
                ePlaneRot = 4,    // Rotation of the bending plane about the z axis
                nPars = 5         // Number of fitted parameters   
            };
            static constexpr std::size_t s_nFitPars {Acts::toUnderlying(iFitPars::nPars)};

            /** @brief The common bending plane */
            using BendingPlane = std::pair<double, double>; // (phi, offset)
            /** @brief Field integrals */
            using FieldIntegrals = Acts::Vector<s_nFieldInt>;
            /** @brief Segment projection on the common bending plane */
            using SegProj = Acts::Vector<s_nSegProj>;
            /** @brief Two independent measurements each segment provides for the fit */
            using SegMeas = Acts::Vector<s_nSegMeas>;
            /** @brief Covariance of the segment measurements */
            using SegMeasCov = Acts::SquareMatrix<s_nSegMeas>;
            /** @brief Derivative of the segment projection with respect to the nuisance parameters */
            using ProjPlaneJac = Acts::Matrix<s_nSegProj, s_nNuisance>;
            /** @brief Design matrix: derivative of the predictions with respect to the fit parameters */
            using DesignMatrix = Acts::Matrix<s_nSegMeas, s_nFitPars>;

            /** @brief Structure to hold the data for each segment after projection */
            struct SegmentData {
                // Pointer to the segment
                const xAOD::MuonSegment* segment{nullptr};  
                // Projection of the segment onto the bending plane            
                SegProj proj{SegProj::Zero()};       
                // Covariance of the segment measurement in the bending plane       
                SegMeasCov cov{SegMeasCov::Zero()};
                // Derivative of the segment projection with respect to nuisance parameters          
                ProjPlaneJac planeJac{ProjPlaneJac::Zero()};
            };
            
            /** @brief Method performing the q*p fit
             *  @param mfCache: Magnetic field cache
             *  @param bendingPlane: The common bending plane
             *  @param segmentData: The data for each segment
             *  @param bs: Beamspot: used if provided.
             *  @return: The result of the fit, or std::nullopt if the fit fails */
            Result_t fitQoverP(Acts::MagneticFieldProvider::Cache& mfCache,
                               const BendingPlane& bendingPlane,
                               const std::span<const SegmentData> segmentData,
                               const std::optional<BeamSpot>& bs = std::nullopt) const;
            /** @brief Common bending plane of the seed, parallel to the z axis, from its segments with phi 
             *         layers: two or more, the line through their positions in the transverse plane; one, its line;
             *         none, the plane of the expanded sector through the origin.
             *  @param seed: The track seed
             *  @return: The plane parameters */
            BendingPlane fitBendingPlane(const MsTrackSeed& seed) const;
            /** @brief Compute the common bending plane given a set of segments, if enough segments with phi
                       layers are provided.
             *  @param segs: The segments used to compute the bending plane
             *  @return: The plane parameters */
            std::optional<BendingPlane> 
                fitBendingPlane(std::span<const xAOD::MuonSegment* const> segs) const;
            /** @brief Projects the segment onto the common bending plane (parallel to the z axis): its position and its
             *         direction are moved along the wire into the plane.
             *  @param tgContext: Geometry context
             *  @param bendingPlane: The common bending plane
             *  @param segment: The segment to project
             *  @return: Projection of a segment onto the common bending plane, covariance of the two measured 
             *          quantities (offset and angle in the plane), and derivatives of projection with respect 
             *          to the nuisance parameters. */
            SegmentData projectSegment(const Acts::GeometryContext& tgContext,
                                       const BendingPlane& bendingPlane,
                                       const xAOD::MuonSegment& segment) const;
            /** @brief Projects multiple segments onto the common bending plane.
             *  @param tgContext: Geometry context
             *  @param bendingPlane: The common bending plane
             *  @param segments: The segments to project
             *  @return: Projections of the segments onto the common bending plane, each with its own covariance and derivatives. */
            std::vector<SegmentData> projectSegments(const Acts::GeometryContext& tgContext,
                                                     const BendingPlane& bendingPlane,
                                                     const std::span<const xAOD::MuonSegment* const> segments) const;
            /** @brief Field integrals along the path through the given points (straight lines between consecutive ones,
             *         the field sampled at the steps of nFieldSteps): for each point j its path length s_j from the first
             *         point and the integrals B1_j = int_0^{s_j} b ds and B2_j = int_0^{s_j} (s_j - s) b ds of the bending
             *         field b = B.n (n the normal of the bending plane).
             *  @param points: Positions in the bending plane, ordered along the track
             *  @param planeNorm: Normal of the bending plane
             *  @param mfCache: The initialized magnetic field cache
             *  @return: (s_j, B1_j, B2_j) for each point [mm, T m, T m mm], Acts units */
            std::vector<FieldIntegrals> forceIntegration(const std::vector<Acts::Vector3>& points,
                                                         const Acts::Vector3& planeNorm,
                                                         Acts::MagneticFieldProvider::Cache& mfCache) const;
        
            /** @brief Configuration object */
            Config m_cfg{};    
            /** @brief Logger for the acts logger */
            std::unique_ptr<const Acts::Logger> m_logger{};
            /** @brief Getter for the logger */
            const Acts::Logger& logger() const {
                return *m_logger;
            }
            /** @brief Maximum number of segments of a seed to be considered for recovery */
            static constexpr std::size_t s_maxSegments {10u};                                    
            /** @brief The list of field steps in the force field integration */
            std::vector<double> m_fieldExtpSteps{};
    };
}



#endif