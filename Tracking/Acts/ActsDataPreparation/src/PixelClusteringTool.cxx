/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelClusteringTool.h"

#include <Acts/Clusterization/Clusterization.hpp>

#include <PixelReadoutGeometry/PixelModuleDesign.h>
#include <xAODInDetMeasurement/PixelCluster.h>
#include <xAODInDetMeasurement/PixelClusterContainer.h>
#include <xAODInDetMeasurement/PixelClusterAuxContainer.h>
#include <xAODInDetMeasurement/Utilities.h>

#include <unordered_set>

using CLHEP::micrometer;

namespace ActsTrk {
static inline double square(const double x){
  return x*x;
}  
constexpr double ONE_TWELFTH = 1./12.;  
  
void clusterAddCell(PixelClusteringTool::Cluster& cl,
		    const PixelClusteringTool::Cell& cell)
{
  cl.ids.push_back(cell.ID);
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
  ATH_CHECK(m_offlineCalibDataKey.initialize(not m_offlineCalibDataKey.empty() and
					     m_errorStrategy == 2));
  
  ATH_MSG_DEBUG(name() << " successfully initialized");
  return StatusCode::SUCCESS;
}

PixelClusteringTool::PixelClusteringTool(
    const std::string& type, const std::string& name, const IInterface* parent)
    : base_class(type,name,parent)
{}

StatusCode
PixelClusteringTool::makeCluster(const EventContext& ctx,
				 const PixelClusteringTool::Cluster &cluster,
				 const PixelID& pixelID,
				 const InDetDD::SiDetectorElement* element,
				 xAOD::PixelCluster& xaodcluster) const
{ 
  const PixelChargeCalibCondData *calibData = nullptr;
  if (not m_chargeDataKey.empty()) {
    SG::ReadCondHandle<PixelChargeCalibCondData> calibDataHandle(m_chargeDataKey, ctx);
    calibData = calibDataHandle.cptr();
  }
  
  const PixelCalib::PixelOfflineCalibData *offlineCalibData = nullptr;
  if (not m_offlineCalibDataKey.empty() and m_errorStrategy == 2) {
    SG::ReadCondHandle<PixelCalib::PixelOfflineCalibData> offlineCalibDataHandle(m_offlineCalibDataKey, ctx);
    offlineCalibData = offlineCalibDataHandle.cptr();
  }

  const InDetDD::PixelModuleDesign& design = 
	dynamic_cast<const InDetDD::PixelModuleDesign&>(element->design());

  InDetDD::SiLocalPosition pos_acc(0,0);
  int tot_acc = 0;

  std::vector<float> chargeList;
  if (calibData) chargeList.reserve(cluster.ids.size());
  
  int colmax = std::numeric_limits<int>::min();
  int rowmax = std::numeric_limits<int>::min();
  int colmin = std::numeric_limits<int>::max();
  int rowmin = std::numeric_limits<int>::max();

  float qRowMin = 0.f;
  float qRowMax = 0.f;
  float qColMin = 0.f;
  float qColMax = 0.f;
  
  bool hasGanged = false;
  
  for (size_t i = 0; i < cluster.ids.size(); i++) {
    Identifier id = cluster.ids.at(i);
    hasGanged = hasGanged ||
      m_pixelRDOTool->isGanged(id, element).has_value();

    int tot = cluster.tots.at(i);
    float charge = tot;

    if (calibData) {
      Identifier moduleID = pixelID.wafer_id(id);
      IdentifierHash moduleHash = pixelID.wafer_hash(moduleID);
      charge = calibData->getCharge(m_pixelReadout->getDiodeType(id),
				    moduleHash,
				    m_pixelReadout->getFE(id, moduleID),
				    tot);
      // These numbers are taken from the Cluster Maker Tool
      if (moduleHash < 12 or moduleHash > 2035) {
        charge = tot/8.0*(8000.0-1200.0)+1200.0;
      }
      chargeList.push_back(charge);
    }
    
    const int row = pixelID.phi_index(id);
    if (row > rowmax) {
      rowmax = row;
      qRowMax =	charge;
    } else if (row == rowmax) {
      qRowMax += charge; 
    }
    
    if (row < rowmin) {  
      rowmin = row;
      qRowMin = charge;
    } else if (row == rowmin) {
      qRowMin += charge;
    } 
       
    const int col = pixelID.eta_index(id);
    if (col > colmax) {
      colmax = col;
      qColMax =	charge;
    } else if (col == colmax) {
      qColMax += charge;
    }     

    if (col < colmin) {
      colmin = col;
      qColMin = charge;
    } else if (col == colmin) {
      qColMin += charge;
    } 
    
    InDetDD::SiCellId si_cell = element->cellIdFromIdentifier(id);
    InDetDD::SiLocalPosition pos = design.localPositionOfCell(si_cell);
    pos_acc += tot * pos;
    tot_acc += tot;
  }
  
  if (tot_acc > 0)
    pos_acc /= tot_acc;

  // Compute omega for charge interpolation correction (if required)
  // Two pixels may have charge=0 (very rarely, hopefully)
  float omegax = -1.f;
  float omegay = -1.f;
  if(qRowMin + qRowMax > 0) omegax = qRowMax/(qRowMin + qRowMax);
  if(qColMin + qColMax > 0) omegay = qColMax/(qColMin + qColMax);

  
  const int colWidth = colmax - colmin + 1;
  const int rowWidth = rowmax - rowmin + 1;
  double etaWidth = design.widthFromColumnRange(colmin, colmax);
  double phiWidth = design.widthFromRowRange(rowmin, rowmax);
  InDet::SiWidth siWidth(Amg::Vector2D(rowWidth,colWidth), Amg::Vector2D(phiWidth,etaWidth));

  // ask for Lorentz correction, get global position
  double shift = m_pixelLorentzAngleTool->getLorentzShift(element->identifyHash());
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
  auto errorMatrix = Amg::MatrixX(2,2);
  errorMatrix.setIdentity();

  const double eta = std::abs(globalPos.eta());
  const Amg::Vector2D& colRow = siWidth.colRow();
  const double zPitch = siWidth.z()/colRow.y();
  const Identifier clusterID = Identifier(); // ???
  const int layer = pixelID.layer_disk(clusterID);
  const int phimod = pixelID.phi_module(clusterID);
  
  switch (m_errorStrategy) {
  case 0:
    errorMatrix.fillSymmetric(0,0, square(siWidth.phiR()) * ONE_TWELFTH);
    errorMatrix.fillSymmetric(1,1, square(siWidth.z()) * ONE_TWELFTH);
    break;
    
  case 2:
    // use parameterization only if the cluster does not
    // contain long pixels or ganged pixels
    // Also require calibration service is available....
    if (not hasGanged and
	zPitch > 399 * micrometer and
	zPitch < 401 * micrometer) {
      if (offlineCalibData) {
        if (element->isBarrel()) {
          int ibin = offlineCalibData->getPixelClusterErrorData()->getBarrelBin(eta,
										static_cast<int>(colRow.y()),
										static_cast<int>(colRow.x()));
          double phiError = offlineCalibData->getPixelClusterErrorData()->getPixelBarrelPhiError(ibin);
          double etaError = offlineCalibData->getPixelClusterErrorData()->getPixelBarrelEtaError(ibin);
          errorMatrix.fillSymmetric(0,0, pow(phiError,2));
          errorMatrix.fillSymmetric(1,1, pow(etaError,2));
        }
        else {
          int ibin = offlineCalibData->getPixelClusterErrorData()->getEndcapBin(static_cast<int>(colRow.y()),
										static_cast<int>(colRow.x()));
          double phiError = offlineCalibData->getPixelClusterErrorData()->getPixelEndcapPhiError(ibin);
          double etaError = offlineCalibData->getPixelClusterErrorData()->getPixelEndcapRError(ibin);
          errorMatrix.fillSymmetric(0,0, square(phiError));
          errorMatrix.fillSymmetric(1,1, square(etaError));
        }
      }
    } else {
      // cluster with ganged and/or long pixels
      errorMatrix.fillSymmetric(0,0, square(siWidth.phiR()/colRow.x()) * ONE_TWELFTH);
      errorMatrix.fillSymmetric(1,1, square(zPitch) * ONE_TWELFTH);
    }
    break;
   
  case 10:
    errorMatrix.fillSymmetric(0,0, square( getPixelCTBPhiError(layer, phimod, static_cast<int>(colRow.x())) ));
    errorMatrix.fillSymmetric(1,1, square(siWidth.z()/colRow.y()) * ONE_TWELFTH);
    break;
    
  case 1:
  default:
    errorMatrix.fillSymmetric(0,0, square(siWidth.phiR()/colRow.x()) * ONE_TWELFTH);
    errorMatrix.fillSymmetric(1,1, square(siWidth.z()/colRow.y()) * ONE_TWELFTH);
    break;
  }


  // Actually create the cluster (i.e. fill the values)
  IdentifierHash idHash = element->identifyHash();

  Eigen::Matrix<float,2,1> localPosition(locpos.x(), locpos.y());
  Eigen::Matrix<float,2,2> localCovariance = Eigen::Matrix<float,2,2>::Zero();
  localCovariance(0, 0) = errorMatrix(0, 0);
  localCovariance(1, 1) = errorMatrix(1, 1);
  
  xaodcluster.setMeasurement<2>(idHash, localPosition, localCovariance);
  xaodcluster.setRDOlist(cluster.ids);
  xaodcluster.globalPosition() = globalPos.cast<float>();
  xaodcluster.setToTlist(cluster.tots);
  xaodcluster.setTotalToT( xAOD::xAODInDetMeasurement::Utilities::computeTotalToT(cluster.tots) );
  xaodcluster.setChargelist(chargeList);
  xaodcluster.setTotalCharge( xAOD::xAODInDetMeasurement::Utilities::computeTotalCharge(chargeList) );
  xaodcluster.setLVL1A(cluster.lvl1min);
  xaodcluster.setChannelsInPhiEta(siWidth.colRow()[0],
				  siWidth.colRow()[1]);
  xaodcluster.setWidthInEta(static_cast<float>(siWidth.widthPhiRZ()[1]));
  xaodcluster.setOmegas(omegax, omegay);
  xaodcluster.setIsSplit(false);
  xaodcluster.setSplitProbabilities(0.0, 0.0);
    
  return StatusCode::SUCCESS;
}


StatusCode
PixelClusteringTool::clusterize(const RawDataCollection& RDOs,
				const PixelID& pixelID,
				const EventContext& ctx,
				ClusterContainer& container) const
{
    const InDetDD::SiDetectorElement* element = m_pixelRDOTool->checkCollection(RDOs, ctx);
    if (element == nullptr)
	return StatusCode::FAILURE;

    std::vector<InDet::UnpackedPixelRDO> cells =
	m_pixelRDOTool->getUnpackedPixelRDOs(RDOs, pixelID, element, ctx);

    ClusterCollection clusters =
      Acts::Ccl::createClusters<CellCollection, ClusterCollection, 2>
      (cells, Acts::Ccl::DefaultConnect<Cell, 2>(m_addCorners));

    std::size_t previousSizeContainer = container.size();
    // Fast insertion trick
    std::vector<xAOD::PixelCluster*> toAddCollection;
    toAddCollection.reserve(clusters.size());
    for (std::size_t i(0); i<clusters.size(); ++i)
      toAddCollection.push_back(new xAOD::PixelCluster());
    container.insert(container.end(), toAddCollection.begin(), toAddCollection.end());

    for (std::size_t i(0); i<clusters.size(); ++i) {
      const Cluster& cluster = clusters[i];
      ATH_CHECK(makeCluster(ctx, cluster, pixelID, element, *container[previousSizeContainer+i]));
    }

    return StatusCode::SUCCESS;
}

// CTB parameterization, B field off
double PixelClusteringTool::getPixelCTBPhiError(int layer, int phi,
						int phiClusterSize) const
{
  double sigmaL0Phi1[3] = { 8.2*micrometer,  9.7*micrometer, 14.6*micrometer};
  double sigmaL1Phi1[3] = {14.6*micrometer,  9.3*micrometer, 14.6*micrometer};
  double sigmaL2Phi1[3] = {14.6*micrometer,  8.6*micrometer, 14.6*micrometer};
  double sigmaL0Phi0[3] = {14.6*micrometer, 13.4*micrometer, 13.0*micrometer};
  double sigmaL1Phi0[3] = {14.6*micrometer,  8.5*micrometer, 11.0*micrometer};
  double sigmaL2Phi0[3] = {14.6*micrometer, 11.6*micrometer,  9.3*micrometer};
  
  if(phiClusterSize > 3) return 14.6*micrometer;
  
  if(layer == 0 && phi == 0) return sigmaL0Phi0[phiClusterSize-1];
  if(layer == 1 && phi == 0) return sigmaL1Phi0[phiClusterSize-1];
  if(layer == 2 && phi == 0) return sigmaL2Phi0[phiClusterSize-1];
  if(layer == 0 && phi == 1) return sigmaL0Phi1[phiClusterSize-1];
  if(layer == 1 && phi == 1) return sigmaL1Phi1[phiClusterSize-1];
  if(layer == 2 && phi == 1) return sigmaL2Phi1[phiClusterSize-1];
  
  // shouldn't really happen...
  ATH_MSG_WARNING("Unexpected layer and phi numbers: layer = "
		  << layer << " and phi = " << phi);
  return 14.6*micrometer;  
}
  
} // namespace ActsTrk
