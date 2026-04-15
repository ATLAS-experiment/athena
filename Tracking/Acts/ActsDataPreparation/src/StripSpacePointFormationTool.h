/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_STRIPSPACEPOINTFORMATIONTOOL_H
#define ACTSTRK_DATAPREPARATION_STRIPSPACEPOINTFORMATIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IStripSpacePointFormationTool.h"
#include "StripInformationHelper.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "SiSpacePointFormation/SiElementPropertiesTable.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include <string>

class SCT_ID;

namespace ActsTrk {
  
    /// @class ActsTrkStripSpacePointFormationTool
    /// Tool to produce strip space points.
    /// Strip space points are made by combining clusters from pairs of
    /// overlapping detectors. The access to overlapping detector elements is
    /// possible using the ContainerAccessor.
    /// The user can choose just to process the detector element and
    /// its opposite on the stereo layer, or also to consider overlaps with the
    /// four nearest neighbours of the opposite elements.
    ///
    /// Space points are then recorded to storegate as StripSP

    class StripSpacePointFormationTool: public extends<AthAlgTool, ActsTrk::IStripSpacePointFormationTool> {
    public:
        /// @name AthAlgTool methods
        //@{
        StripSpacePointFormationTool(const std::string& type,
                                     const std::string& name,
                                     const IInterface* parent);
        virtual ~StripSpacePointFormationTool() = default;
        virtual StatusCode initialize() override;
        //@}

        /// @name Production of space points
        //@{
        virtual StatusCode produceSpacePoints(const EventContext& ctx,
					    const xAOD::StripClusterContainer& clusterContainer,
					    const InDet::SiElementPropertiesTable& properties,
					    const InDetDD::SiDetectorElementCollection& elements,
					    const Amg::Vector3D& beamSpotVertex,
					    std::vector<StripSP>& spacePoints,
					    std::vector<StripSP>& overlapSpacePoints,
					    bool processOverlaps,
					    const std::vector<IdentifierHash>& hashesToProcess,
					    const ContainerAccessor<xAOD::StripCluster, IdentifierHash, 1>& stripAccessor ) const override;
        //@}

    private:

        /// @name Production of space points
        //@{
        StatusCode
	  fillStripSpacePoints(const std::array<const InDetDD::SiDetectorElement*,nNeighbours>& neighbourElements,
			       const std::array<std::vector<std::pair<const xAOD::StripCluster*, size_t>>,nNeighbours>& neighbourClusters,
			       const std::array<double, 14>& overlapExtents,
			       const Amg::Vector3D& beamSpotVertex,
			       std::vector<StripSP>& spacePoints,
			       std::vector<StripSP>& overlapSpacePoints ) const;
	
	StatusCode makeStripSpacePoint(std::vector<StripSP>&,
				       const StripInformationHelper& firstInfo,
				       const StripInformationHelper& secondInfo,
				       bool isEndcap,
				       double limit,
				       double slimit) const;

        void updateRange(const InDetDD::SiDetectorElement* element1,
                         const InDetDD::SiDetectorElement* element2,
                         double& stripLengthGapTolerance, double& min, double& max) const;

        double offset(const InDetDD::SiDetectorElement* element1,
                      const InDetDD::SiDetectorElement* element2,
                      double& stripLengthGapTolerance) const;

        void correctPolarRange(const InDetDD::SiDetectorElement* element,
                               double& min, double& max,
                               size_t& minStrip, size_t& maxStrip) const;

        std::pair<Amg::Vector3D, Amg::Vector3D > getStripEnds(const xAOD::StripCluster* cluster,
                                                              const InDetDD::SiDetectorElement* element,
                                                              size_t& stripIndex) const;
        //@}

        /// @name Id helpers
        //@{
        const SCT_ID* m_stripId{};
        //@}

        /// @name tool handles
        //@{
        /// @brief Using Lorentz angle tool
        ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool{this, "LorentzAngleTool", "",
	  "Tool to retreive Lorentz angle of SCT"};
        //@}

        /// @name Configuration flags
        //@{
        Gaudi::Property< bool > m_allClusters{this, "AllClusters", false, "Process all clusters without limits."};
        Gaudi::Property< bool > m_useTopSp{this, "useTopSp", false, "SP global position is for second strip module."};
        Gaudi::Property<bool> m_isITk {this, "isITk", true, "True if running in ITk"};
        //@}

        /// @name Cut parameters
        /// @brief The following are ranges within which clusters must lie to
        /// make a spacepoint. Opposite and eta neighbours
        /// clusters must lie within range of each other.
        /// Phi clusters must lie in region of each wafer separately.
        //@{
        Gaudi::Property< float > m_overlapLimitOpposite{this, "OverlapLimitOpposite", 2.8, "Overlap limit for opposite-neighbour."};
        Gaudi::Property< float > m_overlapLimitPhi{this, "OverlapLimitPhi", 5.64, "Overlap limit for phi-neighbours."};
        Gaudi::Property< float > m_overlapLimitEtaMin{this, "OverlapLimitEtaMin", 1.68, "Low overlap limit for eta-neighbours."};
        Gaudi::Property< float > m_overlapLimitEtaMax{this, "OverlapLimitEtaMax", 3.0, "High overlap limit for eta-neighbours."};
        /// @brief The following are parameters to build the space points.
        Gaudi::Property< float > m_stripLengthTolerance{this, "StripLengthTolerance", 0.01};
        Gaudi::Property< float > m_stripGapParameter{this, "StripGapParameter", 0.0015, "Recommend 0.001 - 0.0015 for ITK geometry"};
        Gaudi::Property< bool > m_useSCTLayerDep_OverlapCuts{this,"useSCTLayerDep_OverlapCuts", true};
        /// For applying geometric cuts on SPs using beamspot constraint
        Gaudi::Property< bool > m_useBeamSpotConstraint{this, "useBeamSpotConstraint", true, "Reject space points which are not compatible with a particle originating from the beamspot"};
      //@}

  };

}

#endif
