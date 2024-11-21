/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CoreStripSpacePointFormationTool.h"
#include "Acts/Utilities/SpacePointUtility.hpp"
#include "InDetIdentifier/SCT_ID.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "xAODInDetMeasurement/ContainerAccessor.h"
#include "Acts/SpacePointFormation/SpacePointBuilderConfig.hpp"
#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometry/ATLASSourceLinkSurfaceAccessor.h"

#include "StoreGate/WriteHandle.h"
namespace ActsTrk
{

  CoreStripSpacePointFormationTool::CoreStripSpacePointFormationTool(const std::string &type,
                                                                     const std::string &name,
                                                                     const IInterface *parent)
      : base_class(type, name, parent)
  {}

  StatusCode CoreStripSpacePointFormationTool::initialize(){

    ATH_CHECK(detStore()->retrieve(m_stripId, "SCT_ID"));
    ATH_CHECK(m_lorentzAngleTool.retrieve());
    ATH_CHECK(m_trackingGeometryTool.retrieve());
    ATH_CHECK(m_detectorElementToGeometryIdMapKey.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode CoreStripSpacePointFormationTool::produceSpacePoints(const EventContext &ctx,
								  const xAOD::StripClusterContainer &clusterContainer,
								  const InDet::SiElementPropertiesTable &properties,
								  const InDetDD::SiDetectorElementCollection &elements,
								  const Amg::Vector3D &beamSpotVertex,
								  std::vector<StripSP>& spacePoints,
								  std::vector<StripSP>& overlapSpacePoints,
								  bool processOverlaps,
								  const std::vector<IdentifierHash>& hashesToProcess,
								  const ContainerAccessor<xAOD::StripCluster, IdentifierHash, 1>& stripAccessor) const
  {
    /// Production of ActsTrk::SpacePoint from strip clusters
    /// Strip space points involves a more complex logic since
    /// they are made combining clusters from pairs of
    /// overlapping detectors.

    /// For each trigger element, first evaluate and collect the quantities you need to build the space points.
    /// The detector element array has capacity 6:
    /// [0] is the trigger element
    /// [1] is the opposite element
    /// [2]-[3] are the elements tested for eta overlaps
    /// [4]-[5] are the elements tested for phi overlaps
    /// For each element you save the corresponding cluster collections and the
    /// space point compatibility range as described below.
    ///
    /// For the opposite element and the ones tested for eta overlaps, you have to check
    /// if clusters are compatible with the local position of the trigger cluster
    /// requiring that the distance between the two clusters in phi is withing a specified range.
    /// - For the clusters on the opposite element: [-m_overlapLimitOpposite, m_overlapLimitOpposite]
    ///
    /// - For the eta overlapping clusters : you use m_overlapLimitEtaMin and m_overlapLimitEtaMax
    ///   in different combination depending if you are checking a stereo module or not
    ///
    /// For each element, the extremes of these ranges are saved in the overlapExtents array
    /// which is used later on
    ///   - overlapExtents[0], overlapExtents[1] are filled for the opposite element
    ///   - overlapExtents[2], overlapExtents[3], overlapExtents[4], overlapExtents[5] are filled for the eta overlapping elements
    ///
    /// For the elements tested for phi overlaps, you have to check
    /// if clusters are compatible with the local position of the trigger cluster.
    /// This needs that the trigger cluster is at the edge of the trigger module:
    /// e.g. -hwidth < locX_trigger < -hwidth+m_overlapLimitPhi (or hwidth-m_overlapLimitPhi < locX_trigger < hwidth)
    /// and that the other cluster is on the compatible edge of its module:
    /// e.g. hwidth-m_overlapLimitPhi < locX_other < hwidth (or -hwidth < locX_other < -hwidth+m_overlapLimitPhi)
    ///
    /// For each element, the extremes of these ranges are saved in the overlapExtents array
    /// which is used later on
    ///   - overlapExtents[6], overlapExtents[7], overlapExtents[10], overlapExtents[11]
    ///     overlapExtents[8], overlapExtents[9], overlapExtents[12], overlapExtents[13] are filled for the phi overlapping elements

    /// Access to the cluster from a given detector element is possible
    /// via the ContainerAccessor.

    auto spBuilderConfig = std::make_shared<Acts::SpacePointBuilderConfig>();
    const Acts::TrackingGeometry *acts_tracking_geometry=m_trackingGeometryTool->trackingGeometry().get();
    ATH_CHECK(acts_tracking_geometry != nullptr);
    SG::ReadCondHandle<ActsTrk::DetectorElementToActsGeometryIdMap>
       detectorElementToGeometryIdMap{m_detectorElementToGeometryIdMapKey, ctx};
    ATH_CHECK(detectorElementToGeometryIdMap.isValid());

    ATLASUncalibSourceLinkSurfaceAccessor surfaceAccessor{ *acts_tracking_geometry, **detectorElementToGeometryIdMap };

    spBuilderConfig->slSurfaceAccessor
      .connect<&ATLASUncalibSourceLinkSurfaceAccessor::operator()>(&surfaceAccessor);

    const std::shared_ptr<const Acts::TrackingGeometry> trkGeometry = m_trackingGeometryTool->trackingGeometry();
    spBuilderConfig->trackingGeometry = trkGeometry;

    
    auto spConstructor = [this, &clusterContainer, &elements](const Acts::Vector3 &pos,
                    std::optional<double> /*t*/,
							      const Acts::Vector2 &cov,
                    std::optional<double> /*varT*/,
							      const boost::container::static_vector<Acts::SourceLink, 2> &slinks)
      -> StripSP{
      std::vector<std::size_t> measIndices;
      std::array<StripInformationHelper, 2> stripInfos; 
      size_t idx = 0;
      for (const auto& slink : slinks){
	const auto& atlasSourceLink = slink.get<ATLASUncalibSourceLink>();
	const xAOD::UncalibratedMeasurement *hit = &getUncalibratedMeasurement(atlasSourceLink);

	// Check if the cluster is in the cluster container
	const auto it = std::find(clusterContainer.begin(), clusterContainer.end(), dynamic_cast<const xAOD::StripCluster*>(hit));
	if (it != clusterContainer.end()){
	  const auto cluster_index = it - clusterContainer.begin();
	  const auto id = hit->identifierHash();
	  const auto &element = elements.getDetectorElement(id);
	  size_t stripIndex = 0;
	  auto ends = this->getStripEnds(atlasSourceLink, element, stripIndex);
	  auto vertex = Amg::Vector3D(0,0,0);
	  StripInformationHelper stripInfo(id,ends.first, ends.second, vertex, ActsTrk::localXFromSourceLink(atlasSourceLink), cluster_index, stripIndex);
	  measIndices.push_back(cluster_index);
	  stripInfos[idx++] = std::move(stripInfo);
	}
      }
      const auto& [firstInfo, secondInfo] = stripInfos;
      const auto topHalfStripLength = 0.5*firstInfo.stripDirection().norm();
      Eigen::Matrix<double, 3, 1> topStripDirection = -firstInfo.stripDirection()/(2.*topHalfStripLength);
      Eigen::Matrix<double, 3, 1> topStripCenter = 0.5*firstInfo.trajDirection();

      const auto bottomHalfStripLength = 0.5*secondInfo.stripDirection().norm();
      Eigen::Matrix<double, 3, 1> bottomStripDirection = -secondInfo.stripDirection()/(2.*bottomHalfStripLength);
      Eigen::Matrix<double, 3, 1> stripCenterDistance = firstInfo.stripCenter()  - secondInfo.stripCenter();
      
      StripSP sp;
      sp.idHashes = {firstInfo.idHash(), secondInfo.idHash()};
      sp.globPos = pos.cast<float>();
      sp.cov_r = cov(0,0);
      sp.cov_z = cov(1,0);
      sp.measurementIndexes = measIndices;
      sp.topHalfStripLength = topHalfStripLength;
      sp.bottomHalfStripLength = bottomHalfStripLength;
      sp.topStripDirection = topStripDirection.cast<float>();
      sp.bottomStripDirection = bottomStripDirection.cast<float>();
      sp.stripCenterDistance = stripCenterDistance.cast<float>();
      sp.topStripCenter = topStripCenter.cast<float>();

      return sp;
    };

    auto spBuilder = std::make_shared<Acts::SpacePointBuilder<StripSP>>(*spBuilderConfig, spConstructor);

    const auto hashesProc = (hashesToProcess.size() > 0 ? hashesToProcess : stripAccessor.allIdentifiers());
    for (auto &idHash : hashesProc)
      {
	const InDetDD::SiDetectorElement *thisElement = elements.getDetectorElement(idHash);
	if (thisElement->isStereo())
	  continue;

	// Retrieve the neighbours of the detector element
	const std::vector<IdentifierHash>& others = *properties.neighbours(idHash);
	
	if ( others.empty()) continue;

	// This flag is use to trigger if the search should be performed.
	// In case there are no clusters on the neighbours of the selected
	// detector element, the flag stays false.
	bool search = false;
	size_t neighbour = 0;
	while (not search and neighbour < others.size()){
	  search = stripAccessor.isIdentifierPresent( others.at(neighbour) );
	  neighbour++;
	}
	if (not search) continue;

	// prepare clusters, indices and modules for space point formation
	std::array<std::vector<std::pair<const xAOD::StripCluster *, size_t>>, static_cast<size_t>(nNeighbours)> neighbourClusters{};
	std::array<std::vector<std::pair<ATLASUncalibSourceLink, size_t>>, static_cast<size_t>(nNeighbours)> neighbourSourceLinks{};
	std::array<const InDetDD::SiDetectorElement *, static_cast<size_t>(nNeighbours)> neighbourElements{};

	auto groupStart = clusterContainer.begin();
	// Get the detector element and range for the idHash
	neighbourElements[0] = thisElement;
	for (auto &this_range : stripAccessor.rangesForIdentifierDirect(idHash)){
	  for (auto start = this_range.first; start != this_range.second; ++start){
	    size_t position = std::distance(groupStart, start);
	    neighbourClusters[0].push_back(std::make_pair(*start, position));
            if ((*start)->identifierHash() != thisElement->identifyHash()) {
               throw std::logic_error("Identifier mismatch.");
            }
	    auto slink = makeATLASUncalibSourceLink(&clusterContainer, (*start)->index());
	    neighbourSourceLinks[0].emplace_back(std::make_pair(slink, position));
	  }
	}

	Identifier thisId = thisElement->identify();

	// define overlap extends before building space points
	std::array<double, 14> overlapExtents{};
	//   Default case: you test the opposite element and the overlapping in phi (total 3 elements)
	int Nmax = 4;

	// In the barrel, test the eta overlaps as well (total 5 elements)
	if (m_stripId->is_barrel(thisId))
	  Nmax = 6;

	// You can remove all the overlaps if requested.
	// Here you test only the opposite element
	if (not processOverlaps)
	  Nmax = 2;

	float hwidth(properties.halfWidth(idHash));
	int n = 0;

	// The order of the elements in others is such that you first get the opposite element,
	// the overlapping in phi and then the overlapping in eta
	// For this reason you need to re-order the indices, since the SiSpacePointMakerTool will process
	// first the eta overlaps and then the phi ones
	const std::array<size_t, nNeighbours> neigbourIndices{ThisOne, Opposite, EtaMinus, EtaPlus, PhiMinus, PhiPlus};

	for (const auto &otherHash : others){
	  if (++n == Nmax) break;

	  if (not stripAccessor.isIdentifierPresent(otherHash))
	    continue;

	  const InDetDD::SiDetectorElement *otherElement = elements.getDetectorElement(otherHash);

	  neighbourElements[neigbourIndices[n]] = otherElement;
	  for (auto &this_range : stripAccessor.rangesForIdentifierDirect(otherHash)){
	    for (auto start = this_range.first; start != this_range.second; ++start){
	      size_t position = std::distance(groupStart, start);
	      neighbourClusters[neigbourIndices[n]].push_back(std::make_pair(*start, position));
              if ((*start)->identifierHash() != otherElement->identifyHash()) {
                 throw std::logic_error("Identifier mismatch.");
              }
              auto slink = makeATLASUncalibSourceLink(&clusterContainer, (*start)->index());
	      neighbourSourceLinks[neigbourIndices[n]].emplace_back(std::make_pair(slink, position));
	    }
	  }

	  switch (n){
	  case Opposite:
	    {
	      overlapExtents[0] = -m_overlapLimitOpposite;
	      overlapExtents[1] = m_overlapLimitOpposite;
	      break;
	    }
	  case PhiMinus:
	    {
	      overlapExtents[6] = -hwidth;
	      overlapExtents[7] = -hwidth + m_overlapLimitPhi;
	      overlapExtents[8] = hwidth - m_overlapLimitPhi;
	      overlapExtents[9] = hwidth;
	      break;
	    }
	  case PhiPlus:
	    {
	      overlapExtents[10] = hwidth - m_overlapLimitPhi;
	      overlapExtents[11] = hwidth;
	      overlapExtents[12] = -hwidth;
	      overlapExtents[13] = -hwidth + m_overlapLimitPhi;
	      break;
	    }
	  case EtaMinus:
	    {
	      if ((m_stripId->layer_disk(thisId) & 1) == 0){
		overlapExtents[2] = m_overlapLimitEtaMin;
		overlapExtents[3] = m_overlapLimitEtaMax;
	      } else{
		overlapExtents[2] = -m_overlapLimitEtaMax;
		overlapExtents[3] = -m_overlapLimitEtaMin;
	      }
	      break;
	    }
	  default:
	    {
	      if ((m_stripId->layer_disk(thisId) & 1) == 0){
		overlapExtents[4] = -m_overlapLimitEtaMax;
		overlapExtents[5] = -m_overlapLimitEtaMin;
	      } else {
		overlapExtents[4] = m_overlapLimitEtaMin;
		overlapExtents[5] = m_overlapLimitEtaMax;
	      }
	      break;
	    }
	  }
	}

	ATH_CHECK( fillSpacePoints(ctx, spBuilder, neighbourElements, neighbourSourceLinks, overlapExtents, beamSpotVertex,
                                   spacePoints, overlapSpacePoints) );
      }

    return StatusCode::SUCCESS;
  }

  StatusCode CoreStripSpacePointFormationTool::fillSpacePoints(const EventContext &ctx,
							       std::shared_ptr<Acts::SpacePointBuilder<StripSP>> spBuilder,
							       std::array<const InDetDD::SiDetectorElement *,nNeighbours> elements,
							       std::array<std::vector<std::pair<ATLASUncalibSourceLink, size_t>>,nNeighbours> sourceLinks,
							       std::array<double, 14> overlapExtents,
							       const Amg::Vector3D &beamSpotVertex,
							       std::vector<StripSP>& spacePoints,
							       std::vector<StripSP>& overlapSpacePoints ) const
  {
    // This function is called once all the needed quantities are collected.
    // It is used to build space points checking the compatibility of clusters on pairs of detector elements.
    // Detector elements and cluster collections are elements and clusters, respectively.
    // [0] is the trigger element
    // [1] is the opposite element
    // [2]-[3] are the elements tested for eta overlaps
    // [4]-[5] are the elements tested for phi overlaps
    //
    // To build space points:
    // - For the opposite element and the ones tested for eta overlaps, you have to check
    //   if clusters are compatible with the local position of the trigger cluster
    //   requiring that the distance between the two clusters in phi is withing a specified range.
    //   - overlapExtents[0], overlapExtents[1] are filled for the opposite element
    //   - overlapExtents[2], overlapExtents[3], overlapExtents[4], overlapExtents[5] are filled for the eta overlapping elements
    // - For the elements tested for phi overlaps, you have to check
    //   if clusters are compatible with the local position of the trigger cluster.
    //   This needs that the trigger cluster is at the edge of the trigger module
    //   and that the other cluster is on the compatible edge of its module
    //   - overlapExtents[6], overlapExtents[7], overlapExtents[10], overlapExtents[11]
    //     overlapExtents[8], overlapExtents[9], overlapExtents[12], overlapExtents[13] are filled for the phi overlapping elements



    Acts::Vector3 vertex(beamSpotVertex.x(), beamSpotVertex.y(), beamSpotVertex.z());
    constexpr int otherSideIndex{1};
    constexpr int maxEtaIndex{3};
    std::array<int,nNeighbours - 1> elementIndex{};
    int nElements = 0;

    // For the nNeighbours sides, fill elementIndex with the indices of the existing elements.
    // Same the number of elements in nElements to loop on the later on
    for (int n = 1; n != nNeighbours; ++n) {
      if (elements[n]){
	elementIndex[nElements++] = n;
      }
    }
    // return if all detector elements are nullptr
    if(!nElements) return StatusCode::SUCCESS;

    const InDetDD::SiDetectorElement *triggerElement = elements[0];

    bool isEndcap = triggerElement->isEndcap();
    std::vector<StripInformationHelper> stripInfos;
    stripInfos.reserve(sourceLinks[0].size());

    std::vector<ATLASUncalibSourceLink> triggerSlinks;
    triggerSlinks.reserve(sourceLinks[0].size());

    // loop on all clusters on the trigger detector element and save the related information
    for (auto &sourceLink_index : sourceLinks[0]){
      triggerSlinks.emplace_back(sourceLink_index.first);
    }

    double limit = 1. + m_stripLengthTolerance;
    double slimit = 0.;

    if (not m_allClusters){
      // Start processing the opposite side and the eta overlapping elements
      int n = 0;
      for (; n < nElements; ++n){
	int currentIndex = elementIndex[n];
	if (currentIndex > maxEtaIndex)
	  break;

	// get the detector element and the IdentifierHash
	const InDetDD::SiDetectorElement *currentElement = elements[currentIndex];

	// retrieve the range
	double min = overlapExtents[currentIndex * 2 - 2];
	double max = overlapExtents[currentIndex * 2 - 1];

	size_t minStrip, maxStrip = 0;

	if (m_stripGapParameter != 0.){
	  updateRange(*triggerElement, *currentElement, slimit, min, max);
	  correctPolarRange(triggerElement, min, max, minStrip, maxStrip);
	}

	StripInformationHelper currentStripInfo;
	for (auto &sourceLink_index : sourceLinks[currentIndex]){
          float source_local_x = ActsTrk::localXFromSourceLink( sourceLink_index.first );
	  const auto currentSlink = sourceLink_index.first;
	  for (auto triggerSlink : triggerSlinks){

            double diff = source_local_x - ActsTrk::localXFromSourceLink( triggerSlink );
	    if (diff < min || diff > max)
	      continue;
	    if (currentIndex == otherSideIndex){
	      ATH_CHECK( makeSpacePoint(ctx, spacePoints, spBuilder, triggerSlink, currentSlink, triggerElement,
					     currentElement, limit, slimit, vertex));

	    } else {
	      ATH_CHECK(makeSpacePoint(ctx, overlapSpacePoints, spBuilder, triggerSlink, currentSlink, triggerElement,
					    currentElement, limit, slimit, vertex));
	    }
	  }
	}
      }
      // process the phi overlapping elements
      // if possible n starts from 4
      for (; n < nElements; ++n){
	int currentIndex = elementIndex[n];
	const InDetDD::SiDetectorElement *currentElement = elements[currentIndex];
	double min = overlapExtents[4 * currentIndex - 10];
	double max = overlapExtents[4 * currentIndex - 9];

	size_t minStrip, maxStrip = 0;

	if (m_stripGapParameter != 0.){
	  updateRange(*triggerElement, *currentElement, slimit, min, max);
	  correctPolarRange(triggerElement, min, max, minStrip, maxStrip);
	}

	std::vector<ATLASUncalibSourceLink> triggerPhiSlinks;
	triggerSlinks.reserve(triggerSlinks.size());
	for (auto triggerSlink : triggerSlinks){
	  auto centralValue = ActsTrk::localXFromSourceLink( triggerSlink );
	  auto minValue = min;
	  auto maxValue = max;
	  if (isEndcap){
	    size_t stripIndex = 0;
	    getStripEnds(triggerSlink, triggerElement, stripIndex);
	    centralValue = stripIndex;
	    minValue = minStrip;
	    maxValue = maxStrip;
	  }
	  if (minValue <= centralValue and centralValue <= maxValue){
	    triggerPhiSlinks.emplace_back(triggerSlink);
	  }
	}
	if (triggerPhiSlinks.empty())
	  continue;
	min = overlapExtents[4 * currentIndex - 8];
	max = overlapExtents[4 * currentIndex - 7];
	if (m_stripGapParameter != 0.){
	  updateRange(*triggerElement, *currentElement, slimit, min, max);
	  correctPolarRange(currentElement, min, max, minStrip, maxStrip);
	}

	for (auto &sourceLink_index : sourceLinks[currentIndex]){
	  const auto currentSlink = sourceLink_index.first;

	  size_t currentStripIndex = 0;
	  getStripEnds(currentSlink, currentElement, currentStripIndex);
	  auto centralValue = ActsTrk::localXFromSourceLink( sourceLink_index.first );
	  auto minValue = min;
	  auto maxValue = max;
	  if (isEndcap) {
	    centralValue = currentStripIndex;
	    minValue = minStrip;
	    maxValue = maxStrip;
	  }
	  if (centralValue < minValue or centralValue > maxValue)
	    continue;
	  for (auto &triggerSlink : triggerPhiSlinks) {
	    ATH_CHECK(makeSpacePoint(ctx, overlapSpacePoints, spBuilder, triggerSlink, currentSlink, triggerElement,
					  currentElement, limit, slimit, vertex));
	  }
	}
      }
      return StatusCode::SUCCESS;

    } // not m_allClusters

    for (int n = 0; n != nElements; ++n){

      int currentIndex = elementIndex[n];
      const InDetDD::SiDetectorElement *currentElement = elements[currentIndex];

      if (m_stripGapParameter != 0.){
	computeOffset(*triggerElement, *currentElement, slimit);
      }

      for (auto &sourceLink_index : sourceLinks[currentIndex]){
	size_t currentStripIndex = 0;
	getStripEnds(sourceLink_index.first, triggerElement, currentStripIndex);
	const auto currentSlink = sourceLink_index.first;
	  
	for (auto triggerSlink : triggerSlinks){
	  if (currentIndex == otherSideIndex){
	    ATH_CHECK(makeSpacePoint(ctx, spacePoints, spBuilder, triggerSlink, currentSlink, triggerElement,
					  currentElement, limit, slimit, vertex));
	  }else{
	    ATH_CHECK(makeSpacePoint(ctx, overlapSpacePoints, spBuilder, triggerSlink, currentSlink, triggerElement,
					  currentElement, limit, slimit, vertex));
	  }
	}
      }
    }
    return StatusCode::SUCCESS;
  }


  StatusCode CoreStripSpacePointFormationTool::makeSpacePoint(const EventContext &ctx,
							      std::vector<StripSP>& collection,
							      std::shared_ptr<Acts::SpacePointBuilder<StripSP>> spBuilder,
							      const ATLASUncalibSourceLink& currentSlink,
							      const ATLASUncalibSourceLink& anotherSlink,
							      const InDetDD::SiDetectorElement *currentElement,
							      const InDetDD::SiDetectorElement *anotherElement,
							      const double limit,
							      const double slimit,
							      const Acts::Vector3& vertex) const
  {

    auto tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();

    size_t stripIndex = 0;
    auto ends1 = getStripEnds(currentSlink, currentElement, stripIndex);
    auto ends2 = getStripEnds(anotherSlink, anotherElement, stripIndex);
    std::pair<Acts::Vector3, Acts::Vector3> ends1_acts;
    std::pair<Acts::Vector3, Acts::Vector3> ends2_acts;
    ends1_acts.first = ends1.first;
    ends1_acts.second = ends1.second;
    ends2_acts.first = ends2.first;
    ends2_acts.second = ends2.second;
    auto paramCovAccessor = [&](const Acts::SourceLink &slink) {
      const auto &atlasSLink = slink.get<ATLASUncalibSourceLink>();
      const xAOD::UncalibratedMeasurement *measurement = &getUncalibratedMeasurement(atlasSLink);
      Acts::BoundVector loc = Acts::BoundVector::Zero();
      Acts::BoundSquareMatrix cov = Acts::BoundMatrix::Zero();
      switch (measurement->type()) {
      case (xAOD::UncalibMeasType::StripClusterType):
         loc[Acts::eBoundLoc0] = measurement->localPosition<1>()[Trk::locX];
         cov.topLeftCorner<1, 1>() =
            measurement->localCovariance<1>().cast<double>();
         break;
      case (xAOD::UncalibMeasType::PixelClusterType):
         loc[Acts::eBoundLoc0] = measurement->localPosition<2>()[Trk::locX];
         loc[Acts::eBoundLoc1] = measurement->localPosition<2>()[Trk::locY];
         cov.topLeftCorner<2, 2>() =
            measurement->localCovariance<2>().cast<double>();
         break;
      default:
         throw std::domain_error(
                                 "Can only handle measurement type pixel or strip");
      }
      return std::make_pair(loc, cov);
    };
    std::vector<Acts::SourceLink> slinks;

    slinks.emplace_back(Acts::SourceLink{currentSlink});
    slinks.emplace_back(Acts::SourceLink{anotherSlink});

    Acts::SpacePointBuilderOptions spOpt{std::make_pair(ends1_acts, ends2_acts), paramCovAccessor};
    spOpt.vertex = vertex;
    spOpt.stripLengthTolerance = limit - 1;
    spOpt.stripLengthGapTolerance = slimit;


    spBuilder->buildSpacePoint(tgContext, slinks, spOpt,
                               std::back_inserter(collection));
    return StatusCode::SUCCESS;
  }

double CoreStripSpacePointFormationTool::computeOffset(const InDetDD::SiDetectorElement& element1,
						       const InDetDD::SiDetectorElement& element2,
						       double& stripLengthGapTolerance) const
{
    // Get transformation matrices and center positions of detector elements
    const Amg::Transform3D& t1 = element1.transform();
    const Amg::Transform3D& t2 = element2.transform();
    const Amg::Vector3D& c = element1.center();

    // Check if first element is an annulus
    bool isAnnulus = (element1.design().shape() == InDetDD::Annulus);

    // Compute x12 and radius of detector element
    double x12 = t1.linear().col(0).dot(t2.linear().col(0));
    double r = isAnnulus ? c.perp() : std::sqrt(t1(0, 3) * t1(0, 3) + t1(1, 3) * t1(1, 3));

    // Compute distance between detector elements in the direction of strips
    Amg::Vector3D dPos = t1.translation() - t2.translation();
    double s = dPos.dot(t1.linear().col(2));

    // Compute offset distance
    double dm = (m_stripGapParameter * r) * std::abs(s * x12);
    double d = isAnnulus ? dm / 0.04 : dm / std::sqrt((1. - x12) * (1. + x12));

    // Adjust offset distance for z-component of transformation matrix
    const double zComponentTolerance = 0.7;
    if (std::abs(t1(2, 2)) > zComponentTolerance)
        d *= (r / std::abs(t1(2, 3)));

    stripLengthGapTolerance = d;

    return dm;
}

  void CoreStripSpacePointFormationTool::updateRange(const InDetDD::SiDetectorElement &element1,
                                                     const InDetDD::SiDetectorElement &element2,
                                                     double &stripLengthGapTolerance,
                                                     double &min, double &max) const
  {
    double dm = computeOffset(element1, element2, stripLengthGapTolerance);
    min -= dm;
    max += dm;
  }

  void CoreStripSpacePointFormationTool::correctPolarRange(const InDetDD::SiDetectorElement *element,
                                                           double &min,
                                                           double &max,
                                                           size_t &minStrip,
                                                           size_t &maxStrip) const
  {
    if (element->isBarrel())
      return;

    // design for endcap modules
    const InDetDD::StripStereoAnnulusDesign *design = dynamic_cast<const InDetDD::StripStereoAnnulusDesign *>(&element->design());
    if (!design){
      ATH_MSG_FATAL("Invalid strip annulus design for module with identifier/identifierHash " << element->identify() << "/" << element->identifyHash());
      return;
    }

    // converting min and max from cartesian reference frame to polar frame
    auto firstPosition = (design->localPositionOfCell(design->strip1Dim(0, 0)) +
                          design->localPositionOfCell(design->strip1Dim(design->diodesInRow(0) - 1, 0))) *
                         0.5;

    double radius = firstPosition.xEta();

    InDetDD::SiCellId minCellId = element->cellIdOfPosition(InDetDD::SiLocalPosition(radius, min, 0.));
    InDetDD::SiCellId maxCellId = element->cellIdOfPosition(InDetDD::SiLocalPosition(radius, max, 0.));

    if (not minCellId.isValid()) 
      minCellId = InDetDD::SiCellId(0);

    if (not maxCellId.isValid())
      maxCellId = InDetDD::SiCellId(design->diodesInRow(0) - 1);

    minStrip = minCellId.strip();
    maxStrip = maxCellId.strip();

    // re-evaluate min and max in polar coordinate
    min = std::atan2(min, radius);
    max = std::atan2(max, radius);
  }

  std::pair<Amg::Vector3D, Amg::Vector3D>
  CoreStripSpacePointFormationTool::getStripEnds(const ATLASUncalibSourceLink &sourceLink,
						 const InDetDD::SiDetectorElement *element,
						 size_t &stripIndex) const
  {
    const xAOD::UncalibratedMeasurement &measurement = getUncalibratedMeasurement(sourceLink);
    auto cluster = dynamic_cast<const xAOD::StripCluster *>(&measurement);
    if(!cluster){
      ATH_MSG_FATAL("Could not cast UncalibratedMeasurement as StripCluster");
      return {};
    }
    float source_local_x = ActsTrk::localXFromSourceLink( sourceLink );
    if (element->isEndcap())  {
      // design for endcap modules
      const InDetDD::StripStereoAnnulusDesign *design = dynamic_cast<const InDetDD::StripStereoAnnulusDesign *>(&element->design());
      if (!design){
        ATH_MSG_FATAL("Invalid strip annulus design for module with identifier/identifierHash " << element->identify() << "/" << element->identifyHash());
        return {};
      }

      // calculate phi pitch for evaluating the strip index
      double phiPitchPhi = design->phiWidth()/design->diodesInRow(0);
      stripIndex = -std::floor(source_local_x / phiPitchPhi) + design->diodesInRow(0) *0.5 - 0.5;

      std::pair<Amg::Vector3D, Amg::Vector3D > ends = {
        element->globalPosition(design->stripPosAtR(stripIndex, 0, design->minR())),
        element->globalPosition(design->stripPosAtR(stripIndex, 0, design->maxR()))
      };
      return ends;
    }

    InDetDD::SiLocalPosition localPosition(0., source_local_x, 0.);
    std::pair<Amg::Vector3D, Amg::Vector3D> ends(element->endsOfStrip(localPosition));
    return ends;
  }

}
