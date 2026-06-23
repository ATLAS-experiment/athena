/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "StripClusteringTool.h"

#include <InDetRawData/SCT3_RawData.h>
#include <SCT_ReadoutGeometry/SCT_ModuleSideDesign.h>
#include <SCT_ReadoutGeometry/StripStereoAnnulusDesign.h>
#include <TrkSurfaces/Surface.h>
#include "xAODCore/VariableStruct.h"
#include "xAODInDetMeasurement/JaggedVecEltCache.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>


namespace ActsTrk {
constexpr double ONE_TWELFTH = 1./12.;
constexpr float oneStripSF = 1.1025;
constexpr float twoStripSF = 0.0729; 

StripClusteringTool::StripClusteringTool(
    const std::string& type, const std::string& name, const IInterface* parent)
    : base_class(type,name,parent)
{
}

StatusCode StripClusteringTool::initialize()
{
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_MSG_DEBUG(m_stripDetElStatus);
    ATH_MSG_DEBUG(m_checkBadModules);
    ATH_MSG_DEBUG(m_maxFiredStrips);
    ATH_MSG_DEBUG(m_stripDetEleCollKey);
    ATH_MSG_DEBUG(m_isITk);
    ATH_MSG_DEBUG(m_errorStrategy);

    ATH_CHECK(m_lorentzAngleTool.retrieve());
    ATH_CHECK(decodeTimeBins());    

    ATH_CHECK(m_stripDetElStatus.initialize(not m_stripDetElStatus.empty()));
    ATH_CHECK(m_stripDetEleCollKey.initialize());

    ATH_CHECK( detStore()->retrieve(m_stripID, "SCT_ID") );
    
    return StatusCode::SUCCESS;
}

std::pair<unsigned int, unsigned int>
StripClusteringTool::countCells(const RDOContainer& rdoContainer,
                                const std::vector<IdentifierHash> &listOfIds,
                                const InDetDD::SiDetectorElementCollection &detector_elements) const {
   auto getNHits =[](const InDetRawDataCollection<SCT_RDORawData> &RDOs,
                     [[maybe_unused]] const InDetDD::SiDetectorElementCollection &detector_elements )
      -> unsigned int
   {
      unsigned int n_hits = 0u;
      for (const SCT_RDORawData* rdo : RDOs) {
        n_hits += rdo->getGroupSize();
      }
      return n_hits;
   };
   unsigned int n_hits=0;
   if (listOfIds.empty()) {
      for (const InDetRawDataCollection<SCT_RDORawData> *RDOs : rdoContainer) {
         assert( RDOs);
         n_hits += getNHits(*RDOs, detector_elements);
      }
   }
   else {
      for (const IdentifierHash& id : listOfIds) {
         if (not id.is_valid()) continue;
         const InDetRawDataCollection<SCT_RDORawData> *RDOs = rdoContainer.indexFindPtr(id);
         if (RDOs) { // necessary ?
            n_hits += getNHits(*RDOs, detector_elements);
         }
      }
   }
   return {n_hits,n_hits};
}

struct StripAuxDataCache : xAOD::VariableStruct {
   StripAuxDataCache(SG::AuxVectorData& cont, unsigned int n_cluster_rdos)
      : xAOD::VariableStruct(cont),
        rdoList(cont, xAOD::StripCluster::rdoListAcc(), n_cluster_rdos)
   {}
   xAOD::xAODInDetMeasurement::Utilities::JaggedVecEltCache<Identifier::value_type> rdoList;
};

std::any StripClusteringTool::createEventDataCache(xAOD::StripClusterContainer& cont,
                                                   std::size_t nClusterRDOs) const
{
   return std::any (StripAuxDataCache (cont, nClusterRDOs));
}

StatusCode StripClusteringTool::decodeTimeBins()
{
    for (size_t i = 0; i < m_timeBinStr.size(); i++) {
	if (i >= 3) {
	    ATH_MSG_WARNING("Time bin string has excess characters");
	    break;
	}
	switch (std::toupper(m_timeBinStr[i])) {
	case 'X': m_timeBinBits[i] = -1; break;
	case '0': m_timeBinBits[i] =  0; break;
	case '1': m_timeBinBits[i] =  1; break;
	default:
	    ATH_MSG_FATAL("Invalid time bin string: " << m_timeBinStr);
	    return StatusCode::FAILURE;
	}
    }
    return StatusCode::SUCCESS;
}

StatusCode
StripClusteringTool::clusterize([[maybe_unused]] const EventContext& ctx,
                                const RawDataCollection& RDOs,
                                const InDet::SiDetectorElementStatus& stripDetElStatus,
                                const InDetDD::SiDetectorElement& element,
                                IStripClusteringTool::CellContainer &cellContainer) const
{
    IdentifierHash idHash = RDOs.identifyHash();
    IStripClusteringTool::CellContainer::ModuleRangeGuard rangeGuard(cellContainer.startNewModule(idHash));

    // At least an empty cluster collection needs to be always added because the
    // assumption is that there is one element per element RawDataCollection.
    bool goodModule = true;
    if (m_checkBadModules.value()) {
      goodModule = stripDetElStatus.isGood(idHash);
    }

    // If more than a certain number of RDOs set module to bad
    // in this case we skip clusterization
    if (!goodModule && m_maxFiredStrips != 0u) {
      unsigned int nFiredStrips = 0u;
      for (const SCT_RDORawData* rdo : RDOs) {
        nFiredStrips += rdo->getGroupSize();
      }
      goodModule |= (nFiredStrips <m_maxFiredStrips);
    }

    if (goodModule) {
       std::span<IStripClusteringTool::CellContainer::Cell>
          cellRange = unpackRDOs(RDOs, stripDetElStatus, element, cellContainer);

       static constexpr unsigned int SORT_BY_LOCAL_X=0u;
       namespace CL=Acts::InPlaceClusterization;
       CL::clusterize<SORT_BY_LOCAL_X, std::uint16_t>(cellRange,
                                                      CL::defaultConnectionHelper<CL::EConnectionType::CommonEdgeOrCorner>(cellRange));

       // set the cell range per cluster
       Acts::InPlaceClusterization::for_each_cluster(cellRange,
                                                     [&cellContainer](std::span<IStripClusteringTool::CellContainer::Cell> &,
                                                                      unsigned int idx_begin,
                                                                      unsigned int idx_end) {
          cellContainer.registerNewCluster(idx_begin,idx_end);
       });
    }
    // must add a range for every call otherwise the cell container and
    // the list of processed modules get out of sync.
    cellContainer.registerClustersForNewModule(rangeGuard.range());
    return StatusCode::SUCCESS;
}

  
StatusCode
StripClusteringTool::makeClusters(const EventContext& ctx,
                                  [[maybe_unused]] const RDOContainer &rdoContainer,
                                  const IStripClusteringTool::CellContainer& cellContainer,
                                  unsigned int imodule,
                                  const InDetDD::SiDetectorElement& element,
                                  unsigned int icluster,
                                  xAOD::StripClusterContainer& cont,
                                  std::any& cache) const
{
    const IdentifierHash idHash = element.identifyHash();
    double lorentzShift = m_lorentzAngleTool->getLorentzShift(idHash, ctx);

    const InDetDD::SiDetectorDesign& design = element.design();
    // get the pitch, this will be the local covariance for the cluster

    assert((not m_isITk) || element.isBarrel() ||  dynamic_cast<const InDetDD::StripStereoAnnulusDesign*>(&element.design()) !=nullptr);
    float pitch = (element.isBarrel() or (not m_isITk))
      ? design.phiPitch()
      : static_cast<const InDetDD::StripStereoAnnulusDesign&>(element.design()).phiPitchPhi();
    Eigen::Matrix<float,1,1> localCov(pitch * pitch * ONE_TWELFTH);

    StripAuxDataCache* auxDataCache = std::any_cast<StripAuxDataCache> (&cache);
    if (!auxDataCache) throw std::bad_any_cast();

    using CellContainerProxy = InPlaceClusterization::CellContainerProxy<const IStripClusteringTool::CellContainer>;
    using ModuleProxy = InPlaceClusterization::ModuleProxy<const IStripClusteringTool::CellContainer>;
    using ClusterProxy = InPlaceClusterization::ClusterProxy<const IStripClusteringTool::CellContainer>;
    CellContainerProxy cellContainerProxy(&cellContainer);
    ModuleProxy moduleProxy(cellContainerProxy[imodule]);

    for (ClusterProxy clusterProxy: moduleProxy) {
       assert( icluster< cont.size() && cont[icluster]);
       xAOD::StripCluster &stripCluster= *cont[icluster];
       ATH_CHECK(makeCluster(icluster++,
                             stripCluster,
                             clusterProxy,
                             element,
                             design,
                             lorentzShift,
                             localCov,
                             *auxDataCache));
    }
    return StatusCode::SUCCESS;
}

static
std::pair<
    Eigen::Matrix<float,1,1>,
    Eigen::Matrix<float,3,1>>
computePosition(const InPlaceClusterization::ClusterProxy<const IStripClusteringTool::CellContainer>& cluster,
		const InDetDD::SiDetectorElement& element,
		const InDetDD::SiDetectorDesign& design,
		double lorentzShift,
		bool isITk )
{


    InDetDD::SiCellId frontId = cluster.front().coordinates()[0];
    InDetDD::SiLocalPosition pos = design.localPositionOfCell(frontId);
    if (cluster.size()>1) {
      InDetDD::SiCellId backId = cluster.back().coordinates()[0];
      InDetDD::SiLocalPosition backPos =
         design.localPositionOfCell(backId);
	pos = 0.5 * (pos + backPos);
    }

    // update the xPhi position
    pos.xPhi( pos.xPhi() + lorentzShift );
    // @TODO use local to global of Acts surface instead
    Eigen::Matrix<float,3,1> posG(element.surface().localToGlobal(pos).cast<float>());
    
    if ((not element.isBarrel()) and isITk) {
    	assert(dynamic_cast<const InDetDD::StripStereoAnnulusDesign*>(&design) != nullptr);
	const InDetDD::StripStereoAnnulusDesign& annulusDesign =
	    static_cast<const InDetDD::StripStereoAnnulusDesign&>
	    (design);
	pos = annulusDesign.localPositionOfCellPC(element.cellIdOfPosition(pos));
    }

    return std::make_pair(Eigen::Matrix<float,1,1>(pos.xPhi()),
			  std::move(posG));
}

static
InDetDD::SiLocalPosition
computeCentrePosition(const InPlaceClusterization::ClusterProxy<const IStripClusteringTool::CellContainer>& cluster,
                      const InDetDD::SiDetectorDesign& design)
{   // For Inner Detector SCT, compute the local position of the cluster center using the strip positions.
    InDetDD::SiCellId frontId = cluster.front().coordinates()[0];
    InDetDD::SiLocalPosition pos = design.localPositionOfCell(frontId);
    if (cluster.size() > 1) {
      InDetDD::SiCellId backId = cluster.back().coordinates()[0];
      InDetDD::SiLocalPosition backPos = design.localPositionOfCell(backId);
      pos = 0.5 * (pos + backPos);
    }
    return pos;
}

static
float computeRotatedLocalCov(const InPlaceClusterization::ClusterProxy<const IStripClusteringTool::CellContainer> &cluster,
                             float localPos,
                             float localCov,
                             const InDetDD::SiDetectorElement& element,
                             const InDetDD::SiDetectorDesign& design)
{   // For Inner Detector SCT, in the case of endcap modules, rotate the local covariance to account for the stereo angle.
    const bool rotate = (design.shape() == InDetDD::Trapezoid || design.shape() == InDetDD::Annulus);
    if (!rotate) {
      return localCov;
    }

    const auto* sctDesign = dynamic_cast<const InDetDD::SCT_ModuleSideDesign*>(&design);
    if (sctDesign == nullptr) {
      return localCov;
    }

    const InDetDD::SiLocalPosition centrePos = computeCentrePosition(cluster, design);
    const auto ends = sctDesign->endsOfStrip(centrePos);
    const double stripL = std::abs(ends.first.xEta() - ends.second.xEta());
    const double iphipitch = 1. / element.phiPitch();
    const Amg::Vector2D localPos2D{localPos, 0.0};
    const double w = element.phiPitch(localPos2D) * iphipitch;

    const double sn = element.sinStereoLocal(localPos2D);
    const double sn2 = sn * sn;
    const double cs2 = 1. - sn2;
    const double v0 = localCov * w * w;
    const double v1 = stripL * stripL * ONE_TWELFTH;

    const float rotatedCov = cs2 * v0 + sn2 * v1;
    // copied from InDet::SCT_ClusteringTool, but in ACTS-based tracking, SCT cov is 1-dimensional, just keep the rotatedCov(0,0) for the moment
    // rotatedCov(0,1) = rotatedCov = sn * sqrt(cs2) * (v0 - v1);
    // rotatedCov(1,1) = sn2 * v0 + cs2 * v1;
    return rotatedCov;
}

StatusCode
StripClusteringTool::makeCluster(size_t icluster,
                                 xAOD::StripCluster& cl,
                                 const StripClusteringTool::ClusterProxy &clusterProxy,
                                 const InDetDD::SiDetectorElement& element,
                                 const InDetDD::SiDetectorDesign& design,
                                 const double lorentzShift,
                                 Eigen::Matrix<float,1,1>& localCov,
                                 StripAuxDataCache &auxDataCache) const
{
    auto [localPos, globalPos]
      = computePosition(clusterProxy, element, design, lorentzShift, m_isITk);

    // For Strip Clusters the identifier is taken from the front rod list object
    // This is the same strategy used in Athena:
    // Since clusterId is arbitary (it only needs to be unique) just use ID of first strip
    // For strip Cluster it has been found that "identifierOfPosition" does not produces unique values


    // If requiring broad errors, use the cluster size as error -
    // TODO use SiWidth to get the right cluster size as I'm assuming equal strip pitch
    std::size_t size = clusterProxy.size();
    if (m_errorStrategy == 1)   {// use width
      
      localCov *= size*size;
      
    } else if (m_errorStrategy == 2) { //use tuned error as function of size
      
      if (size == 1)
        localCov *= oneStripSF;
      else if (size == 2)
        localCov *= twoStripSF*size*size;
      else
        localCov *= size*size;
      
    }

    if (not m_isITk) {
      localCov(0,0) = computeRotatedLocalCov(clusterProxy, localPos(0,0), localCov(0,0), element, design);
    }

    cl.setMeasurement<1>(element.identifyHash(), localPos, localCov);
    Identifier frontRDOId = m_stripID->strip_id(element.identify(), clusterProxy.front().coordinates()[0]);
    cl.setIdentifier( frontRDOId.get_compact() );

    // Do I really need the global position in fast tracking?
    cl.globalPosition() = globalPos;     
    
    cl.setChannelsInPhi(size);

    unsigned int n_rdos = auxDataCache.rdoList.getBeginIndex(icluster);
    Identifier waferId=element.identify();
    using CellProxy = InPlaceClusterization::CellProxy<const StripClusteringTool::CellContainer>;
    for (CellProxy cellProxy : clusterProxy) {
       Identifier rdoId = m_stripID->strip_id(waferId, cellProxy.coordinates()[0]);
       auxDataCache.rdoList.setValue(n_rdos,rdoId.get_compact());
       ++n_rdos;
    }
    auxDataCache.rdoList.updateEndIndex(icluster,n_rdos);
    
    return StatusCode::SUCCESS;
}


bool StripClusteringTool::passTiming(const std::bitset<3>& timePattern) const {
    // Convert the given timebin to a bit set and test each bit
    // if bit is -1 (i.e. X) it always passes, other wise require exact match of 0/1
    // N.B bitset has opposite order to the bit pattern we define
    if (m_timeBinBits[0] != -1 and timePattern.test(2) != static_cast<bool>(m_timeBinBits[0])) return false;
    if (m_timeBinBits[1] != -1 and timePattern.test(1) != static_cast<bool>(m_timeBinBits[1])) return false;
    if (m_timeBinBits[2] != -1 and timePattern.test(0) != static_cast<bool>(m_timeBinBits[2])) return false;
    return true;
}


bool StripClusteringTool::isBadStrip(const InDet::SiDetectorElementStatus *stripDetElStatus,
				     IdentifierHash waferHash,
				     std::int16_t strip)
{
    if (stripDetElStatus) {
	return not stripDetElStatus->isCellGood(waferHash.value(), strip) ;
    }
    return false;
}


std::span<IStripClusteringTool::CellContainer::Cell>
StripClusteringTool::unpackRDOs(const RawDataCollection& RDOs,
				const InDet::SiDetectorElementStatus& stripDetElStatus,
				const InDetDD::SiDetectorElement& element,
                                IStripClusteringTool::CellContainer &cellContainer) const
{
    const InDetDD::SiDetectorDesign& design = element.design();

    //Check type in debug build otherwise assume it is correct
    assert(dynamic_cast<const InDetDD::SCT_ModuleSideDesign*>(&design)!=nullptr);
    std::size_t ncells = static_cast<size_t>(static_cast<const InDetDD::SCT_ModuleSideDesign&>(design).cells());
     
    IStripClusteringTool::CellContainer::ModuleRangeGuard rangeGuard(cellContainer, RDOs.identifyHash() );
    // Simple single-entry cache
    IdentifierHash waferHash=RDOs.identifyHash();

    for (unsigned int rdo_i=0; rdo_i<RDOs.size(); ++rdo_i) {
        const StripRDORawData *raw=RDOs[rdo_i];

    	//Check type in debug build otherwise assume it is correct
    	assert(dynamic_cast<const SCT3_RawData*>(raw)!=nullptr);
	const SCT3_RawData* raw3 = static_cast<const SCT3_RawData*>(raw);

	std::bitset<3> timePattern(raw3->getTimeBin());
	if (!passTiming(timePattern)) {
	    ATH_MSG_DEBUG("Strip failed timing check");
	    continue;
	}

	Identifier firstStripId = raw->identify();
	Identifier waferId = m_stripID->wafer_id(firstStripId);
	
        assert( waferHash == m_stripID->wafer_hash(waferId) );

        std::int16_t iFirstStrip = static_cast<std::int16_t>(m_stripID->strip(firstStripId));
        assert(m_stripID->strip(firstStripId)==iFirstStrip);

        assert( ncells == static_cast<std::uint16_t>(ncells) );
	std::int16_t iMaxStrip = std::min(
            static_cast<std::int16_t>(iFirstStrip + raw->getGroupSize()),
            static_cast<std::int16_t>(ncells));
	
	for (std::int16_t strip_i = iFirstStrip; strip_i < iMaxStrip; strip_i++) {
	    if (isBadStrip(&stripDetElStatus, waferHash, strip_i)) {
		// Bad strip, throw it out to minimize useless work.
               ATH_MSG_DEBUG("Bad strip encountered on module " << waferHash << " : "
                              << m_stripID->strip_id(waferId, strip_i)
			      << ", wafer is: " << waferId << " strip is "<< strip_i
                              << " hash " << m_stripID->wafer_hash(waferId));
	    } else {
		// Good strip!

               std::array<std::int16_t,1> coordinates{strip_i};
               cellContainer.emplace_back_cell(coordinates, rdo_i);
	    }
	}
    }

    return rangeGuard.moduleCellSpan();
}

} // namespace ActsTrk
