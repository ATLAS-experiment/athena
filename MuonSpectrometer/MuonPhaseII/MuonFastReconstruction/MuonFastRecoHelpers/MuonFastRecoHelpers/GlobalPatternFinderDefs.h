/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_FASTRECONSTRUCTIONALGS_PATTERNFINDERUTILITIES__H
#define MUONR4_FASTRECONSTRUCTIONALGS_PATTERNFINDERUTILITIES__H

#include "MuonFastRecoHelpers/GlobalPatternFinder.h"

#include "MuonRecToolInterfacesR4/IFastRecoVisualizationTool.h"
#include "MuonSpacePoint/SpacePoint.h"
#include "MuonTrackEvent/ExpandedSector.h"
#include "xAODMuonPrepData/UtilFunctions.h"

namespace MuonR4::FastReco{
    using StIndex = GlobalPatternFinder::StIndex;

    static const SpacePointPerLayerSorter s_spSorter{};

    /** @brief Base class for hit struct containing hit information. */
    struct GlobalPatternFinder::HitPayload{
        /** @brief Constructor with parameters
         *  @param sp The space point
         *  @param bucket The space point bucket
         *  @param localToGlobal The transformation from local to global coordinates
         *  @param locLayer The layer number in the sector frame
         *  @param station The station index */
        explicit HitPayload(const Acts::GeometryContext& gctx,
                            const SpacePoint* sp,
                            const SpacePointBucket* bucket,
                            const Amg::Transform3D& localToGlobal);
        /** @brief Sensor direction */
        Amg::Vector3D sensorDir(const Acts::GeometryContext& gctx) const;

        /** @brief Hit contribution contribution to the residual variance 
                   due to its intrinsic position uncertainty.
         *  @param contractionVector The contraction vector to compute the residual variance
         *  @param isProjected Whether the hit has been projected
         *  @return Residual variance contribution */
        double residualVariance(const Acts::GeometryContext& gctx, 
                                const Amg::Vector3D& contractionVector, 
                                const bool isProjected) const;

        /** @brief Global position */
        Amg::Vector3D position{Amg::Vector3D::Zero()};
        /** @brief Pointer to the underlying hit */
        const SpacePoint* sp{nullptr};
        /** @brief Pointer to the parent bucket */
        const SpacePointBucket* bucket{nullptr};
        /** @brief Associated detector surface */
        const Acts::Surface& surface{xAOD::muonSurface(sp->primaryMeasurement())};
        /** @brief Cached angular covariance [rad^2] of the hit in the phi angle */
        double phiCov{0.};
        /** @brief Strip angle when the strips are non-orthogonal */
        double stripAngle{0.};
        /** @brief Station index */
        StIndex station{toStationIndex(sp->msSector()->chamberIndex())};
        /** @brief Layer number in the sector frame */
        uint8_t locLayer{static_cast<uint8_t>(s_spSorter.sectorLayerNum(*sp))};
        /** @brief Is precision hit */
        bool isPrecision{isPrecisionHit(*sp)};
        /** @brief Are the strips non-orthogonal */
        bool nonOrthogonalStrips{false};
        /** @brief Equal operator: it compares the underlying hit */
        bool operator==(const HitPayload& other) const;
        /** @brief Arrow operator: it allows to access the underlying hit */
        const SpacePoint* operator->() const { return sp; }
        /** @brief Dereference operator: it allows to access the underlying hit */
        const SpacePoint& operator*() const { return *sp; }
    };

    /** @brief Structure to hold the search tree data */
    struct GlobalPatternFinder::SearchTreeData {
        /** @brief Vector of strip hits */
        std::vector<HitPayload> stripPayloads;
        /** @brief The search tree */
        SearchTree_t tree;
    };

    /** @brief Small wrapper for candidate hits used to build patterns. This is needed
     *         because the global layer number cannot be defined globally, but it can be
     *         computed given a set of hits. We store locally most frequently accessed data 
     *         to avoid frequentpointer indirection (memory anyhow used for padding). */
    struct GlobalPatternFinder::CandidateHit {
        /** @brief Pointer to the underlying hit */
        const HitPayload* hit{nullptr};
        /** @brief Station index */
        StIndex station{};
        /** @brief Global measurement layer number */
        uint8_t globLayer{0u};
        // Forward commonly used accessors for convenience
        const HitPayload* operator->() const { return hit; }
        const HitPayload& operator*() const { return *hit; }
        const SpacePoint* sp() const { return hit->sp; }
        bool operator==(const CandidateHit& other) const { return *hit == *other.hit; }
        bool operator==(const HitPayload& other) const { return *hit == other; }
        // Print and stream operator
        friend std::ostream& operator<<(std::ostream& ostr, const CandidateHit& c) {
            c.print(ostr);
            return ostr;
        }
        void print(std::ostream& ostr) const;
    };

    /** @brief: Enum for possible outcomes of pattern line compatibility test */        
    enum class LineTestDecision : std::int8_t{
        /** @brief Test successfull, add hit to pattern */
        eAddHit,
        /** @brief Test successfull with multiple pattern hits on same layer, branch the pattern */
        eBranchPattern,
        /** @brief Test failed, discard the hit */
        eRejectHit
    };
    /** @brief : Small struct to encapsulate the result of the line compatibility test */
    struct LineTestRes  {
        double residual{0.};
        double sigma{0.};
        LineTestDecision result {LineTestDecision::eRejectHit};
    };
    /** @brief Pattern state object storing pattern information during construction */
    struct GlobalPatternFinder::PatternState {
        /** @brief Constructor taking the seed information 
            *  @param seed: seed hit
            *  @param expSector: **expanded** sector coordinate
            *  @param cfg: pointer to configuration object
            *  @param logger: pointer to messaging object */
        explicit PatternState(const CandidateHit& seed,
                              const std::int8_t expSector,
                              const Config* cfg,
                              const AthMessaging* logger);
        /** @brief Move constructor
            *  @param other: other pattern state to move from */
        PatternState(PatternState&& other) noexcept = default;
        /** @brief Move assignment operator
            *  @param other: other pattern state to move from */
        PatternState& operator=(PatternState&& other) noexcept = default;
        /** @brief Copy constructor
            *  @param other: other pattern state to copy from */
        PatternState(const PatternState& other) = default;
        /** @brief Copy assignment operator
            *  @param other: other pattern state to copy from */
        PatternState& operator=(const PatternState& other) = default;
        /** @brief Destructor */
        ~PatternState() =default;
        /** @brief Add a hit to the pattern and update the internal state
            *  @param hit: hit to be added
            *  @param residual: residual of the hit
            *  @param resSigma: residual uncertainty of the hit */
        void addHit(const CandidateHit& hit,
                    const double residual,
                    const double resSigma);
        /** @brief Overwrite the hits on the last layer with the new one
            *  @param newHit: new hit to replace with
            *  @param newResidual: residual of the new hit 
            *  @param newResSigma: residual uncertainty of the new hit
            *  @param beamSpot: needed to update line parameters */
        void overWriteHit(const CandidateHit& newHit,
                            const double newResidual,
                            const double newResSigma);
        /** @brief Method checking line compatibility of a test hit against the pattern
            *  @param testHit: test hit information
            *  @param beamSpot: Beam spot position, needed to update the pattern line
            *  @return: result of the test, including the computed line residual and acceptance window */
        LineTestRes checkLineComp(const Acts::GeometryContext& gctx,
                                  const CandidateHit& testHit,
                                  const Amg::Vector3D& beamSpot);
        /** @brief Method to compute the residual of a test hit against the pattern line
            *  @param testHit: test hit information
            *  @return: Test result holding the residual and acceptance window. The decision is set later. */
        LineTestRes computeLineResidual(const Acts::GeometryContext& gctx,
                                        const CandidateHit& testHit) const;
        /** @brief Project a certain hit position onto the bending plane where the pattern is defined. 
            *         The hit is moved along the sensor direction if it does not measure phi, 
            *         or is rotated around the Z axis if it does.
            *  @param hit: hit whose position is to be projected
            *  @return: projected position */
        Amg::Vector3D projToPhiPlane(const Acts::GeometryContext& gctx,
                                     const HitPayload& hit) const;
        /** @brief Method to check the phi compatibility of a test hit with a given pattern
            *  @param hit: hit to be checked
            *  @return: true if the test hit is phi compatible with the pattern, false otherwise */
        bool isPhiCompatible(const HitPayload& hit) const;
        /** @brief Check wheter a hit is present in the pattern
            *  @param hit: hit to be checked
            *  @return: boolean indicating if the hit is in the pattern */
        bool isInPattern(const HitPayload& hit) const;
        /** @brief Move the line anchor hit given a reference hit. The anchor is defined
                    as the closest hit in the closest station to the referece hit, */
        void moveLineAnchorHit(const CandidateHit& refHit);
        /** @brief Update the line parameters based on the current hits
            *  @param beamSpot: position of the beam spot, needed when there are not enough hits */
        void updateLineParameters(const Acts::GeometryContext& gctx,
                                  const Amg::Vector3D& beamSpot);
        /** @brief Helper method to update the pattern phi and bending plane normal */
        void updatePatternPhi();
        /** @brief Return the mean normalized residual squared */
        double getMeanResidual2() const;
        /** @brief Return the number of layers in bending coordinate */
        uint8_t nBendingLayers() const;
        /** @brief Method returning the number of stations
            *  @param onlyGoodStations: flag to indicate if only good stations should be counted,
            *         i.e. having a minimum number of hits */
        uint8_t nStations(const bool onlyGoodStations) const;
        /** @brief Get the buckets associated with the pattern */
        std::vector<const SpacePointBucket*> getParentBuckets() const;

        /** @brief Pointer to cfg option */
        const Config* cfg{nullptr};
        /** @brief Logger */
        const AthMessaging* logger{nullptr};
        /** @brief Pointer to Visual Information for pattern visualization */
        Acts::CloneablePtr<PatHitVisual> visualInfo{nullptr};
        /** @brief Last inserted hit. Needed to speed-up lookup */
        CandidateHit lastInsertedHit{};
        /** @brief Last hit in the second-to-last layer */
        CandidateHit prevLayerHit{};
        /** @brief Line anchor hit */
        CandidateHit lineAnchorHit{};
        /** @brief Normal vector to the bending plane where the pattern lies */
        Amg::Vector3D bendPlaneNorm{Amg::Vector3D::Zero()};
        /** @brief Position and direction of the pattern line. Both are constructed to be
            *         within the bending plane of the pattern. We store them as vectors to
            *         facilitate vector operations and avoid constructing them repeatedly. */
        Amg::Vector3D linePos{Amg::Vector3D::Zero()};
        Amg::Vector3D lineDir{Amg::Vector3D::Zero()};
        /** @brief Distance between the two points defining the pattern line */
        double leverArm{0.};
        /** @brief Mean over eta hits of the square of their residual divided by residual uncertainty */
        double meanNormResidual2{0.};
        /** @brief Residual & residual uncertainty of the last inserted hit (needed when replacing a hit) */
        double lastResidual{0.};
        double lastResSigma{0.};
        /** @brief Pattern phi, which is the phi of the bending plane where the pattern lies */
        double patPhi{0.};
        /** @brief Pattern theta, which is the value of the seed hit */
        double patTheta{0.};
        /** @brief Covariance of the pattern phi */
        double patPhiCov{0.};
        /** @brief **expanded** MS sector */
        ExpandedSector expSect{static_cast<int8_t>(0)};
        /** @brief Counts of precision / non-precision / phi layers  */
        uint8_t nPrecisionLayers{0u};
        uint8_t nTriggerLayers{0u};
        uint8_t nPhiLayers{0u};
        /** @brief Flag to indicate if the pattern has been finalized */
        bool isFinalized{false};
        /** @brief Flag to indicate if the pattern is overlapping with another one, used during overlap removal */
        bool isOverlap{false};
        /** @brief Whether we used the beamspot to compute the line parameters */
        bool useBeamspot{false};
        /** @brief Whether we need to update the pattern line the next time we find a hit in a new layer */
        bool needLineUpdate{false};
        
        /** @brief Map collection of hits per station. A pattern is determined by the hits belonging to it. */
        std::array<std::vector<CandidateHit>, s_nStations> hitsPerStation{};
        /** @brief Array holding phi-only hits */
        std::vector<HitPayload> phiOnlyHits{};

        /** @brief Patterns are considered identical if they have the same hit content. However, map comparison is very expensive */
        bool operator==(const PatternState& other) const = delete;
        /** @brief Print the pattern candidate */
        void print(std::ostream& ostr, bool detailed) const;
    };

    /** @brief A view of the pattern state for printing purposes */
    struct GlobalPatternFinder::PatternPrintView {
        /** @brief The pattern state to be printed */
        const PatternState& pat;
        /** @brief Whether to print detailed information */
        bool detailed = false;

        /** @brief Print the pattern print view */
        friend std::ostream& operator<<(std::ostream& os, const PatternPrintView& v) {
            v.pat.print(os, v.detailed);
            return os;
        }
    };
    
}

#endif