/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "StripClusteringTool.h"


#include <Acts/Clusterization/Clusterization.hpp>

#include <InDetRawData/SCT3_RawData.h>
#include <SCT_ReadoutGeometry/SCT_ModuleSideDesign.h>
#include <SCT_ReadoutGeometry/StripStereoAnnulusDesign.h>
#include <TrkSurfaces/Surface.h>

#include <algorithm>
#include <stdexcept>


namespace ActsTrk {
constexpr double ONE_TWELFTH = 1./12.;
  
// Required by ACTS clusterization
static
int getCellColumn(const StripClusteringTool::Cell& cell)
{
    return cell.index;
}

// Required by ACTS clusterization
static
int& getCellLabel(StripClusteringTool::Cell& cell)
{
    return cell.label;
}

// Required by ACTS clusterization
static
void clusterAddCell(StripClusteringTool::Cluster& cl, const StripClusteringTool::Cell& cell)
{
    cl.ids.push_back(cell.id);
    if (cl.ids.size() < (sizeof(cl.hitsInThirdTimeBin) * 8)) {
	cl.hitsInThirdTimeBin |= cell.timeBits.test(0) << cl.ids.size();
    }
}

StripClusteringTool::StripClusteringTool(
    const std::string& type, const std::string& name, const IInterface* parent)
    : base_class(type,name,parent)
{
}

StatusCode StripClusteringTool::initialize()
{
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_CHECK(m_conditionsTool.retrieve(DisableTool{!m_stripDetElStatus.empty()} ));
    ATH_CHECK(m_lorentzAngleTool.retrieve());
    ATH_CHECK(decodeTimeBins());    

    ATH_CHECK(m_stripDetElStatus.initialize(not m_stripDetElStatus.empty()));
    ATH_CHECK(m_stripDetEleCollKey.initialize());

    return StatusCode::SUCCESS;
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

const InDet::SiDetectorElementStatus *
StripClusteringTool::getStripDetElStatus(const EventContext& ctx) const
{
  if (m_stripDetElStatus.empty()) {
    return nullptr;
  }

  SG::ReadHandle<InDet::SiDetectorElementStatus> status = SG::makeHandle(m_stripDetElStatus, ctx);
  if (!status.isValid()) {
    std::stringstream msg;
    msg << "Failed to get " << m_stripDetElStatus.key() << " from StoreGate in " << name();
    throw std::runtime_error(msg.str());
  }

  return status.cptr();
}

StatusCode
StripClusteringTool::clusterize(const RawDataCollection& RDOs,
				const IDHelper& stripID,
				const EventContext& ctx,
				ClusterContainer& container) const
{
    IdentifierHash idHash = RDOs.identifyHash();

    const InDet::SiDetectorElementStatus *stripDetElStatus = getStripDetElStatus(ctx);

    bool goodModule = true;
    if (m_checkBadModules.value()) {
      if (stripDetElStatus != nullptr) {
          goodModule = stripDetElStatus->isGood(idHash);
      } else {
          goodModule = m_conditionsTool->isGood(idHash, ctx);
      }
    }

    VALIDATE_STATUS_ARRAY(
      m_checkBadModules.value() && !m_stripDetElStatus.empty(),
      stripDetElStatus->isGood(idHash), m_conditionsTool->isGood(idHash));

    if (!goodModule) {
      ATH_MSG_DEBUG("Strip module failed status check");
      return StatusCode::SUCCESS;
    }
    
    // If more than a certain number of RDOs set module to bad
    // in this case we skip clusterization
    if (m_maxFiredStrips != 0u) {
      unsigned int nFiredStrips = 0u;
      for (const SCT_RDORawData* rdo : RDOs) {
        nFiredStrips += rdo->getGroupSize();
      }
      if (nFiredStrips > m_maxFiredStrips)
	return StatusCode::SUCCESS;
    }
    
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleHandle(m_stripDetEleCollKey, ctx);
    ATH_CHECK( stripDetEleHandle.isValid() );
    const InDetDD::SiDetectorElementCollection* stripDetEle(*stripDetEleHandle);
    if (stripDetEle == nullptr) {
	ATH_MSG_FATAL("Invalid SiDetectorElementCollection");
	return StatusCode::FAILURE;
    }

    const InDetDD::SiDetectorElement* element =
	stripDetEle->getDetectorElement(idHash);

    const InDetDD::SiDetectorDesign& design = element->design();
    // get the pitch, this will be the local covariance for the cluster
    float pitch = element->isBarrel()
      ? design.phiPitch()
      : dynamic_cast<const InDetDD::StripStereoAnnulusDesign&>(element->design()).phiPitchPhi();
    Eigen::Matrix<float,1,1> localCov(pitch * pitch * ONE_TWELFTH);

    
    std::optional<std::pair<CellCollection,bool>> unpckd
	= unpackRDOs(RDOs, stripID, stripDetElStatus, ctx);
    if (not unpckd.has_value()) {
	ATH_MSG_FATAL("Error encountered while unpacking strip RDOs!");
	return StatusCode::FAILURE;
    }

    auto& [cells, badStripOnModule] = *unpckd;

    ClusterCollection clusters =
	Acts::Ccl::createClusters<CellCollection, ClusterCollection, 1>(cells);

    
    std::size_t previousSizeContainer = container.size();
    // Fast insertion trick
    std::vector<xAOD::StripCluster*> toAddCollection;
    toAddCollection.reserve(clusters.size());
    for (std::size_t i(0); i<clusters.size(); ++i)
      toAddCollection.push_back(new xAOD::StripCluster());
    container.insert(container.end(), toAddCollection.begin(), toAddCollection.end());

    double lorentzShift
	= m_lorentzAngleTool->getLorentzShift(idHash, ctx);

    for (std::size_t i(0); i<clusters.size(); ++i) {
      Cluster& cl = clusters[i];
	// Bad strips on a module invalidates the hitsInThirdTimeBin word.
	// Therefore set it to 0 if that's the case.
        // We are currently not using this, but keeping it here should we need it in the future
	// if (badStripOnModule) {
	//     cl.hitsInThirdTimeBin = 0;
	// }

	try {
	  ATH_CHECK(makeCluster(cl,
				lorentzShift,
				localCov,
				stripID,
				element,
				design,
				*container[previousSizeContainer+i]));
	} catch (const std::exception& e) {
	    ATH_MSG_FATAL("Exception thrown while creating xAOD::StripCluster:"
			  << e.what());
	    ATH_MSG_FATAL("Detector Element identifier: " << element->identify());
	    ATH_MSG_FATAL("Strip Identifiers in cluster:");
	    for (const Identifier& id : cl.ids)
		ATH_MSG_FATAL("  " << id);
	    return StatusCode::FAILURE;
	}
    }

    return StatusCode::SUCCESS;
}


static
std::pair<
    Eigen::Matrix<float,1,1>,
    Eigen::Matrix<float,3,1>>
computePosition(const StripClusteringTool::Cluster& cluster,
		double lorentzShift,
		const IStripClusteringTool::IDHelper& stripID,
		const InDetDD::SiDetectorElement* element,
		const InDetDD::SiDetectorDesign& design)
{
    std::size_t size = cluster.ids.size();
    InDetDD::SiCellId frontId = stripID.strip(cluster.ids.front());
    InDetDD::SiLocalPosition pos = design.localPositionOfCell(frontId);
    if (size > 1) {
	InDetDD::SiCellId backId = stripID.strip(cluster.ids.back());
	InDetDD::SiLocalPosition backPos =
	    design.localPositionOfCell(backId);
	pos = 0.5 * (pos + backPos);
    }

    // update the xPhi position
    pos.xPhi( pos.xPhi() + lorentzShift );

    if (!element->isBarrel()) {
	const InDetDD::StripStereoAnnulusDesign& annulusDesign =
	    dynamic_cast<const InDetDD::StripStereoAnnulusDesign&>
	    (design);
	pos = annulusDesign.localPositionOfCellPC(element->cellIdOfPosition(pos));
    }

    return std::make_pair(Eigen::Matrix<float,1,1>(pos.xPhi()),
			  Eigen::Matrix<float,3,1>(element->surface().localToGlobal(pos).cast<float>()));
}


// N.B. the cluster is added to the container
StatusCode
StripClusteringTool::makeCluster(const Cluster &cluster,
				 double lorentzShift,
				 Eigen::Matrix<float,1,1>& localCov,
				 const StripID& stripID,
				 const InDetDD::SiDetectorElement* element,
				 const InDetDD::SiDetectorDesign& design,
				 xAOD::StripCluster& cl) const
{
    auto [localPos, globalPos]
      = computePosition(cluster, lorentzShift, stripID, element, design);

    // For Strip Clusters the identifier is taken from the front rod list object
    // This is the same strategy used in Athena:
    // Since clusterId is arbitary (it only needs to be unique) just use ID of first strip
    // For strip Cluster it has been found that "identifierOfPosition" does not produces unique values
    cl.setMeasurement<1>(element->identifyHash(), localPos, localCov);
    cl.setIdentifier( cluster.ids.front().get_compact() );
    cl.globalPosition() = globalPos;
    cl.setRDOlist(cluster.ids);
    cl.setChannelsInPhi(cluster.ids.size());

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


bool StripClusteringTool::isBadStrip(const EventContext& ctx,
				     const InDet::SiDetectorElementStatus *stripDetElStatus,
				     const StripID& stripID,
				     IdentifierHash waferHash,
				     Identifier stripId) const
{
    if (stripDetElStatus) {
        const int strip_i{stripID.strip(stripId)};
	VALIDATE_STATUS_ARRAY(
	    stripDetElStatus,
	    stripDetElStatus->isCellGood(waferHash.value(), strip_i),
	    m_conditionsTool->isGood(stripId, InDetConditions::SCT_STRIP));
	return not stripDetElStatus->isCellGood(waferHash.value(), strip_i) ;
    }
    return not m_conditionsTool->isGood(stripId, InDetConditions::SCT_STRIP, ctx);
}


std::optional<std::pair<StripClusteringTool::CellCollection, bool>>
StripClusteringTool::unpackRDOs(const InDetRawDataCollection<StripRDORawData>& RDOs,
				const StripID& stripID,
				const InDet::SiDetectorElementStatus *stripDetElStatus,
				const EventContext& ctx) const
{
    CellCollection cells;
    // reserve memory. number evaluated on ttbar pu200
    cells.reserve(60);
    bool badStripOnModule{false};

    for (const StripRDORawData * raw : RDOs) {
	const SCT3_RawData* raw3 = dynamic_cast<const SCT3_RawData*>(raw);
	if (!raw3) {
	    ATH_MSG_ERROR("Casting into SCT3_RawData failed");
	    return {};
	}

	std::bitset<3> timePattern(raw3->getTimeBin());
	if (!passTiming(timePattern)) {
	    ATH_MSG_DEBUG("Strip failed timing check");
	    continue;
	}

	Identifier firstStripId = raw->identify();
	Identifier waferId = stripID.wafer_id(firstStripId);
	IdentifierHash waferHash = stripID.wafer_hash(waferId);
	size_t iFirstStrip = static_cast<size_t>(stripID.strip(firstStripId));
	size_t iMaxStrip = std::min(
	    iFirstStrip + raw->getGroupSize(),
	    static_cast<size_t>(stripID.strip_max(waferId)) + 1
        );

	for (size_t i = iFirstStrip; i < iMaxStrip; i++) {
	    Identifier stripIdent = stripID.strip_id(waferId, i);
	    if (isBadStrip(ctx, stripDetElStatus, stripID, waferHash, stripIdent)) {
		// Bad strip, throw it out to minimize useless work.
		ATH_MSG_DEBUG("Bad strip encountered:" << stripIdent
			      << ", wafer is: " << waferId);
		badStripOnModule = true;
	    } else {
		// Good strip!
		cells.emplace_back(i, stripIdent, std::move(timePattern));
	    }
	}
    }
    return std::make_pair(std::move(cells), badStripOnModule);
}

} // namespace ActsTrk
