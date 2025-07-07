/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelClusteringTool.h"

#include <Acts/Clusterization/Clusterization.hpp>

#include <xAODInDetMeasurement/PixelCluster.h>
#include <xAODInDetMeasurement/PixelClusterContainer.h>
#include <xAODInDetMeasurement/PixelClusterAuxContainer.h>
#include <xAODInDetMeasurement/Utilities.h>

#include <unordered_set>
#include <stdexcept>

using CLHEP::micrometer;

// Helper functions for use with ACTS clusterization
// Put these in the InDet namespace so that ACTS can find them
// via ADL.
//
namespace InDet {
  static inline int getCellRow(const InDet::UnpackedPixelRDO& cell) { return cell.ROW; }
  static inline int getCellColumn(const InDet::UnpackedPixelRDO& cell) { return cell.COL; }
  static inline int& getCellLabel(InDet::UnpackedPixelRDO& cell) { return cell.NCL; }
}

namespace ActsTrk {
  
static inline void clusterAddCell(PixelClusteringTool::Cluster& cl,
		    const PixelClusteringTool::Cell& cell)
{
  cl.ids.push_back(cell.ID.get_compact());
  cl.tots.push_back(cell.TOT);
  if (cell.LVL1 < cl.lvl1min)
    cl.lvl1min = cell.LVL1;
}

StatusCode PixelClusteringTool::initialize()
{
  ATH_MSG_DEBUG("Initializing " << name() << " ...");
  ATH_CHECK(m_pixelRDOTool.retrieve());
  ATH_CHECK(m_pixelLorentzAngleTool.retrieve());
  if (not m_chargeDataKey.empty()) ATH_CHECK(m_pixelReadout.retrieve());
  
  ATH_CHECK(m_chargeDataKey.initialize(not m_chargeDataKey.empty()));

  ATH_CHECK( detStore()->retrieve(m_pixelID, "PixelID") );
  
  ATH_MSG_DEBUG(name() << " successfully initialized");
  return StatusCode::SUCCESS;
}

PixelClusteringTool::PixelClusteringTool(
    const std::string& type, const std::string& name, const IInterface* parent)
    : base_class(type,name,parent)
{}

StatusCode
PixelClusteringTool::makeCluster(const EventContext& ctx,
				 PixelClusteringTool::Cluster &cluster,
				 const PixelID& pixelID,
				 const InDetDD::SiDetectorElement* element,
				 const InDetDD::PixelModuleDesign& design,
				 const PixelChargeCalibCondData *calibData,
				 const PixelChargeCalibCondData::CalibrationStrategy calibStrategy,
				 xAOD::PixelCluster& xaodcluster) const
{ 

  InDetDD::SiLocalPosition pos_acc(0,0);
  int tot_acc = 0;

  std::vector<float> chargeList;
  if (calibData) chargeList.reserve(cluster.ids.size());
  
  int colmax = std::numeric_limits<int>::min();
  int rowmax = std::numeric_limits<int>::min();
  int colmin = std::numeric_limits<int>::max();
  int rowmin = std::numeric_limits<int>::max();

  // We temporary comment this since it is not used
  // bool hasGanged = false;
  
  Identifier moduleID = element->identify();
  IdentifierHash moduleHash = element->identifyHash();

  // This could be moved outside the cluster loop
  bool multiChip = design.numberOfCircuits() > 1;
    
  for (size_t i = 0; i < cluster.ids.size(); i++) {

    //Construct the identifier class
    Identifier id = Identifier(cluster.ids[i]);

    // We temporary comment this since it is not used
    // TODO: Check how the ganged info is used in legacy
    // if (multiChip)  {
    //   hasGanged = hasGanged ||
    // 	m_pixelRDOTool->isGanged(id, element).has_value();
    // }

    int tot = cluster.tots.at(i);
    float charge = tot;
        
    if (calibData) {

      if (design.getReadoutTechnology() != InDetDD::PixelReadoutTechnology::RD53) {
	ATH_MSG_ERROR("Chip type is not recognized!");
	return StatusCode::FAILURE;
      }
      
      // The calibration strategy is updated for each element 
      // Retrieving the calibration only depends on FE and not per cell (can be further optimized)
      // Single FE modules could have an optimized getCharge function where the calib constants are cached
      int feValue = multiChip ? m_pixelReadout->getFE(id, moduleID, element) : 0;
      charge = calibData->getCharge(m_pixelReadout->getDiodeType(id,element),
				    calibStrategy,
				    moduleHash,
				    feValue,
				    tot);
      chargeList.push_back(charge);
    }
    
    const int row = pixelID.phi_index(id);
    rowmax = std::max(rowmax, row);
    rowmin = std::min(rowmin, row);
           
    const int col = pixelID.eta_index(id);
    colmax = std::max(colmax, col);
    colmin = std::min(colmin, col);
    
    InDetDD::SiCellId si_cell = element->cellIdFromIdentifier(id);
    InDetDD::SiLocalPosition pos = design.localPositionOfCell(si_cell);

    // We compute the digital position as a sum of all RDO positions
    // all with the same weight of 1
    // We do not compute a charge-weighted center of gravity here (by default) since
    // we observe it to be worse than the digital position
    // ToT-weighted center of gravity must not be used
    if (m_useWeightedPos) {
      pos_acc += charge * pos;
      tot_acc += charge;
    } else {
      pos_acc += pos;
      tot_acc += 1;
    }
    
  }
  
  if (tot_acc > 0)
    pos_acc /= tot_acc;

  
  const int colWidth = colmax - colmin + 1;
  const int rowWidth = rowmax - rowmin + 1;
  double etaWidth = design.widthFromColumnRange(colmin, colmax);
  double phiWidth = design.widthFromRowRange(rowmin, rowmax);
  InDet::SiWidth siWidth(Amg::Vector2D(rowWidth,colWidth), Amg::Vector2D(phiWidth,etaWidth));

  // ask for Lorentz correction, get global position
  double shift = m_pixelLorentzAngleTool->getLorentzShift(moduleHash, ctx);
  const Amg::Vector2D localPos = pos_acc;
  Amg::Vector2D locpos(localPos[Trk::locX]+shift, localPos[Trk::locY]);
  // find global position of element
  const Amg::Transform3D& T = element->surface().transform();
  double Ax[3] = {T(0,0),T(1,0),T(2,0)};
  double Ay[3] = {T(0,1),T(1,1),T(2,1)};
  double R [3] = {T(0,3),T(1,3),T(2,3)};
  
  const Amg::Vector2D&    M = locpos;
  Amg::Vector3D globalPos(M[0]*Ax[0]+M[1]*Ay[0]+R[0],M[0]*Ax[1]+M[1]*Ay[1]+R[1],M[0]*Ax[2]+M[1]*Ay[2]+R[2]);

  // Compute error matrix
  float width0, width1;
  if (m_broadErrors) {
      // Use cluster width
      width0 = siWidth.phiR();
      width1 = siWidth.z();
  } else {
      // Use average pixel width
      width0 = siWidth.phiR() / siWidth.colRow().x();
      width1 = siWidth.z() / siWidth.colRow().y();
  }

  // Actually create the cluster (i.e. fill the values)
  
  Eigen::Matrix<float,2,1> localPosition(locpos.x(), locpos.y());
  Eigen::Matrix<float,2,2> localCovariance = Eigen::Matrix<float,2,2>::Zero();
  localCovariance(0, 0) = width0 * width0 / 12.0; 
  localCovariance(1, 1) = width1 * width1 / 12.0;
  
  xaodcluster.setMeasurement<2>(moduleHash, localPosition, localCovariance);
  xaodcluster.setIdentifier( element->identifierOfPosition(locpos).get_compact() );
  xaodcluster.setRDOlist(std::move(cluster.ids));
  xaodcluster.globalPosition() = globalPos.cast<float>();
  xaodcluster.setTotalToT( xAOD::xAODInDetMeasurement::Utilities::computeTotalToT(cluster.tots) );
  xaodcluster.setToTlist(std::move(cluster.tots));
  xaodcluster.setTotalCharge( xAOD::xAODInDetMeasurement::Utilities::computeTotalCharge(chargeList) );
  xaodcluster.setChargelist(std::move(chargeList));
  xaodcluster.setLVL1A(cluster.lvl1min);
  xaodcluster.setChannelsInPhiEta(siWidth.colRow()[0],
				  siWidth.colRow()[1]);
  xaodcluster.setWidthInEta(static_cast<float>(siWidth.widthPhiRZ()[1]));
  xaodcluster.setIsSplit(false);
  xaodcluster.setSplitProbabilities(0.0, 0.0);
    
  return StatusCode::SUCCESS;
}

StatusCode
PixelClusteringTool::clusterize(const EventContext& ctx,
				const RawDataCollection& RDOs,
				const InDet::SiDetectorElementStatus& pixelDetElStatus,
				const InDetDD::SiDetectorElement& element,
				std::vector<ClusterCollection>& collection) const
{
  IdentifierHash idHash = RDOs.identifyHash();
  if ( not pixelDetElStatus.isGood(idHash) ) {
    // the module being flagged as bad is not a failure
    return StatusCode::SUCCESS;
  }

  // Retrieve the cells from the detector element
  std::vector<InDet::UnpackedPixelRDO> cells =
    m_pixelRDOTool->getUnpackedPixelRDOs(RDOs, *m_pixelID, &element, ctx);
  
  ClusterCollection clusters =
    Acts::Ccl::createClusters<CellCollection, ClusterCollection, 2>
    (cells, Acts::Ccl::DefaultConnect<Cell, 2>(m_addCorners));
  collection.push_back( std::move(clusters) );
  
  return StatusCode::SUCCESS;
}


StatusCode
PixelClusteringTool::makeClusters(const EventContext& ctx,
				  typename IPixelClusteringTool::ClusterCollection& clusters,
				  const InDetDD::SiDetectorElement& element,
				  typename ClusterContainer::iterator itrContainer) const
{
  // We'd need a smarter move here!!!
  
  // Retrieve the calibration data
  const PixelChargeCalibCondData *calibData = nullptr;
  if (not m_chargeDataKey.empty()) {
    SG::ReadCondHandle<PixelChargeCalibCondData> calibDataHandle = SG::makeHandle( m_chargeDataKey, ctx );
    calibData = calibDataHandle.cptr();
    
    if (!calibData) {
      ATH_MSG_ERROR("PixelChargeCalibCondData requested but couldn't be retrieved from " << m_chargeDataKey.key());
      return StatusCode::FAILURE;
    }
  }
  
  // Get the element design
  const InDetDD::PixelModuleDesign& design = 
    static_cast<const InDetDD::PixelModuleDesign&>(element.design());
  
  // Get the calibration strategy for this module. 
  // Default to RD53 if the calibData is not available. That is fine because it won't be used anyway
  auto calibrationStrategy = calibData ? calibData->getCalibrationStrategy(element.identifyHash()) : PixelChargeCalibCondData::CalibrationStrategy::RD53;

  for (typename IPixelClusteringTool::Cluster& cl : clusters) {
    xAOD::PixelCluster* xaodCluster = *itrContainer;
    ATH_CHECK(makeCluster(ctx,
			  cl,
			  *m_pixelID,
			  &element,
			  design,
			  calibData,
			  calibrationStrategy,
			  *xaodCluster));
    ++itrContainer;
  }
  
  return StatusCode::SUCCESS;
}

} // namespace ActsTrk
