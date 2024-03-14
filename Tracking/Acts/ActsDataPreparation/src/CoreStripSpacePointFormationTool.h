/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_CORESTRIPSPACEPOINTFORMATIONTOOL_H
#define ACTSTRK_DATAPREPARATION_CORESTRIPSPACEPOINTFORMATIONTOOL_H

#include "ActsEventCnv/IActsToTrkConverterTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"

#include "ActsGeometry/ATLASSourceLink.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IStripSpacePointFormationTool.h"
#include "StripInformationHelper.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "SiSpacePointFormation/SiElementPropertiesTable.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "Acts/SpacePointFormation/SpacePointBuilder.hpp"
#include "ActsGeometry/ATLASSourceLink.h"
#include "InDetIdentifier/SCT_ID.h"

#include <string>

namespace ActsTrk {
    /// @class CoreStripSpacePointFormationTool
    /// Tool to produce strip space points using ACTS SP builder.
    /// Strip space points are made by combining clusters (obtained from SourceLinks)
    /// from pairs of overlapping detectors. The access to overlapping detector elements is
    /// possible using the ContainerAccessor.
    /// The user can choose just to process the detector element and
    /// its opposite on the stereo layer, or also to consider overlaps with the
    /// four nearest neighbours of the opposite elements.
    ///
    /// Space points are then recorded to storege as StripSP, and then
    /// stored as xAOD::SpacePoint in StripSpacePointFormationAlg

  class CoreStripSpacePointFormationTool: public extends<AthAlgTool, ActsTrk::IStripSpacePointFormationTool> {
  public:
    CoreStripSpacePointFormationTool(const std::string& type,
				     const std::string& name,
				     const IInterface* parent);

    virtual ~CoreStripSpacePointFormationTool() = default;

    virtual StatusCode initialize() override;

    virtual StatusCode produceSpacePoints( const EventContext& ctx,
					   const xAOD::StripClusterContainer& clusterContainer,
					   const InDet::SiElementPropertiesTable& properties,
					   const InDetDD::SiDetectorElementCollection& elements,
					   const Amg::Vector3D& beamSpotVertex,
					   std::vector<StripSP>& spacePoints,
					   std::vector<StripSP>& overlapSpacePoints,
					   bool processOverlaps ) const override;

  private:

    StatusCode fillSpacePoints(const EventContext& ctx,
			      std::shared_ptr<Acts::SpacePointBuilder<StripSP>> spBuilder,
			      std::array<const InDetDD::SiDetectorElement*,nNeighbours> neighbourElements,
			      std::array<std::vector<std::pair<ATLASUncalibSourceLink, size_t>>,nNeighbours> neighbourSourceLinks,
			      std::array<double, 14> overlapExtents,
			      const Amg::Vector3D& beamSpotVertex,
			      std::vector<StripSP>& spacePoints,
			      std::vector<StripSP>& overlapSpacePoints ) const;

    StatusCode makeSpacePoint(const EventContext& ctx,
			     std::vector<StripSP>& collection,
			     std::shared_ptr<Acts::SpacePointBuilder<StripSP>> spBuilder,
			     const ATLASUncalibSourceLink& currentSlink,
			     const ATLASUncalibSourceLink& anotherSlink,
			     const InDetDD::SiDetectorElement* currentElement,
			     const InDetDD::SiDetectorElement* anotherElement,
			     const double limit,
			     const double slimit,
			     const Acts::Vector3& vertex) const;

    void updateRange(const InDetDD::SiDetectorElement& element1,
		     const InDetDD::SiDetectorElement& element2,
		     double& stripLengthGapTolerance, double& min, double& max) const;

    double computeOffset(const InDetDD::SiDetectorElement& element1,
		  const InDetDD::SiDetectorElement& element2,
		  double& stripLengthGapTolerance) const;

    void correctPolarRange(const InDetDD::SiDetectorElement* element,
			   double& min, double& max,
			   size_t& minStrip, size_t& maxStrip) const;

    std::pair<Amg::Vector3D, Amg::Vector3D > getStripEnds(const xAOD::StripCluster* cluster,
							  const InDetDD::SiDetectorElement* element,
							  size_t& stripIndex) const;
    std::pair<Amg::Vector3D, Amg::Vector3D > getStripEnds(const ATLASUncalibSourceLink &sourceLink,
							  const InDetDD::SiDetectorElement* element,
							  size_t& stripIndex) const;
    const SCT_ID* m_stripId{};

    ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool{this, "LorentzAngleTool", "", "Tool to retreive Lorentz angle of SCT"};
    ToolHandle<IActsToTrkConverterTool> m_ATLASConverterTool{this, "ConverterTool", ""};
    ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};

    Gaudi::Property<bool> m_allClusters{this, "AllClusters", false, "Process all clusters without limits."};
    Gaudi::Property<float> m_overlapLimitOpposite{this, "OverlapLimitOpposite", 2.8, "Overlap limit for opposite-neighbour."};
    Gaudi::Property<float> m_overlapLimitPhi{this, "OverlapLimitPhi", 5.64, "Overlap limit for phi-neighbours."};
    Gaudi::Property<float> m_overlapLimitEtaMin{this, "OverlapLimitEtaMin", 1.68, "Low overlap limit for eta-neighbours."};
    Gaudi::Property<float> m_overlapLimitEtaMax{this, "OverlapLimitEtaMax", 3.0, "High overlap limit for eta-neighbours."};
    Gaudi::Property<float> m_stripLengthTolerance{this, "StripLengthTolerance", 0.01};
    Gaudi::Property<float> m_stripGapParameter{this, "StripGapParameter", 0.0015, "Recommend 0.001 - 0.0015 for ITK geometry"};
  };

}

#endif
