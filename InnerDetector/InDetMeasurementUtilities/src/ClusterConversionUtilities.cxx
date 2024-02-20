/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetMeasurementUtilities/ClusterConversionUtilities.h"

#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"

#include "xAODInDetMeasurement/Utilities.h"

#include "HGTD_PrepRawData/HGTD_Cluster.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterAuxContainer.h"

namespace TrackingUtilities {

  StatusCode convertInDetToXaodCluster(const HGTD_Cluster& indetCluster,
				       const InDetDD::HGTD_DetectorElement& element,
				       xAOD::HGTDCluster& xaodCluster)
  {
    IdentifierHash idHash = element.identifyHash();

    auto localPos = indetCluster.localPosition();
    auto localCov = indetCluster.localCovariance();

    const float time = indetCluster.time();
    const float timeResolution = indetCluster.timeResolution();
    
    const auto& RDOs = indetCluster.rdoList();
    const auto& ToTs = indetCluster.totList();

    // Time to fill the xaod cluster
    Eigen::Matrix<float,3,1> localPosition = Eigen::Matrix<float,3,1>::Zero();
    localPosition(0, 0) = localPos.x();
    localPosition(1, 0) = localPos.y();
    localPosition(2, 0)	= time;

    Eigen::Matrix<float,3,3> localCovariance = Eigen::Matrix<float,3,3>::Zero();
    localCovariance(0, 0) = localCov(0, 0);
    localCovariance(1, 1) = localCov(1, 1);
    localCovariance(2, 2) = timeResolution;

    xaodCluster.setMeasurement<3>(idHash, localPosition, localCovariance);
    xaodCluster.setRDOlist(RDOs);
    xaodCluster.setToTlist(ToTs);
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode convertInDetToXaodCluster(const InDet::PixelCluster& indetCluster,
				       const InDetDD::SiDetectorElement& element,
				       xAOD::PixelCluster& xaodCluster)
  {
    IdentifierHash idHash = element.identifyHash();

    auto localPos = indetCluster.localPosition();
    auto localCov = indetCluster.localCovariance();

    Eigen::Matrix<float,2,1> localPosition(localPos.x(), localPos.y());

    Eigen::Matrix<float,2,2> localCovariance;
    localCovariance.setZero();
    localCovariance(0, 0) = localCov(0, 0);
    localCovariance(1, 1) = localCov(1, 1);

    auto globalPos = indetCluster.globalPosition();
    Eigen::Matrix<float, 3, 1> globalPosition(globalPos.x(), globalPos.y(), globalPos.z());

    const auto& RDOs = indetCluster.rdoList();
    const auto& ToTs = indetCluster.totList();
    const auto& charges = indetCluster.chargeList();
    const auto& width = indetCluster.width();
    auto omegaX = indetCluster.omegax();
    auto omegaY = indetCluster.omegay();
    auto isSplit = indetCluster.isSplit();
    auto splitProbability1 = indetCluster.splitProbability1();
    auto splitProbability2 = indetCluster.splitProbability2();

    xaodCluster.setMeasurement<2>(idHash, localPosition, localCovariance);
    xaodCluster.setRDOlist(RDOs);
    xaodCluster.globalPosition() = globalPosition;
    xaodCluster.setToTlist(ToTs);
    xaodCluster.setTotalToT( xAOD::xAODInDetMeasurement::Utilities::computeTotalToT(ToTs) );
    xaodCluster.setChargelist(charges);
    xaodCluster.setTotalCharge( xAOD::xAODInDetMeasurement::Utilities::computeTotalCharge(charges) );
    xaodCluster.setLVL1A(indetCluster.LVL1A());
    xaodCluster.setChannelsInPhiEta(width.colRow()[0], width.colRow()[1]);
    xaodCluster.setWidthInEta(static_cast<float>(width.widthPhiRZ()[1]));
    xaodCluster.setOmegas(omegaX, omegaY);
    xaodCluster.setIsSplit(isSplit);
    xaodCluster.setSplitProbabilities(splitProbability1, splitProbability2);

    return StatusCode::SUCCESS;
  }


  StatusCode convertInDetToXaodCluster(const InDet::SCT_Cluster& indetCluster,
				       const InDetDD::SiDetectorElement& element,
				       xAOD::StripCluster& xaodCluster)
  {
    static const double one_over_twelve = 1. / 12.;
    IdentifierHash idHash = element.identifyHash();

    auto localPos = indetCluster.localPosition();
    
    Eigen::Matrix<float,1,1> localPosition;
    Eigen::Matrix<float,1,1> localCovariance;
    localCovariance.setZero();

    if (element.isBarrel()) {
      localPosition(0, 0) = localPos.x();
      localCovariance(0, 0) = element.phiPitch() * element.phiPitch() * one_over_twelve;
    } else {
      InDetDD::SiCellId cellId = element.cellIdOfPosition(localPos);
      const InDetDD::StripStereoAnnulusDesign *design = dynamic_cast<const InDetDD::StripStereoAnnulusDesign *>(&element.design());
      if ( design == nullptr ) {
	return StatusCode::FAILURE;
      }
      InDetDD::SiLocalPosition localInPolar = design->localPositionOfCellPC(cellId);
      localPosition(0, 0) = localInPolar.xPhi();
      localCovariance(0, 0) = design->phiPitchPhi() * design->phiPitchPhi() * one_over_twelve;
    }

    auto globalPos = indetCluster.globalPosition();
    Eigen::Matrix<float, 3, 1> globalPosition(globalPos.x(), globalPos.y(), globalPos.z());

    const auto& RDOs = indetCluster.rdoList();
    const auto& width = indetCluster.width();

    xaodCluster.setMeasurement<1>(idHash, localPosition, localCovariance);
    xaodCluster.setRDOlist(RDOs);
    xaodCluster.globalPosition() = globalPosition;
    xaodCluster.setChannelsInPhi(width.colRow()[0]);

    return StatusCode::SUCCESS;
  }

  StatusCode convertXaodToInDetCluster(const xAOD::PixelCluster& xaodCluster,
				       const InDetDD::SiDetectorElement& element,
				       const PixelID& pixelID,
				       InDet::PixelCluster*& indetCluster)
  {
    const InDetDD::PixelModuleDesign* design(dynamic_cast<const InDetDD::PixelModuleDesign*>(&element.design()));
    if (design == nullptr) {
      return StatusCode::FAILURE;
    }

    const auto& locPos = xaodCluster.localPosition<2>();
    Amg::Vector2D localPosition(locPos(0,0), locPos(1,0));

    InDetDD::SiLocalPosition centroid(localPosition);
    const Identifier id = element.identifierOfPosition(centroid);

    const auto& globalPos = xaodCluster.globalPosition();
    Amg::Vector3D globalPosition(globalPos(0, 0), globalPos(1, 0), globalPos(2, 0));

    auto errorMatrix = Amg::MatrixX(2,2);
    errorMatrix.setIdentity();
    errorMatrix.fillSymmetric(0, 0, xaodCluster.localCovariance<2>()(0, 0));
    errorMatrix.fillSymmetric(1, 1, xaodCluster.localCovariance<2>()(1, 1));

    int colmax = std::numeric_limits<int>::min();
    int rowmax = std::numeric_limits<int>::min();
    int colmin = std::numeric_limits<int>::max();
    int rowmin = std::numeric_limits<int>::max();

    const std::vector<Identifier>& rod_list_cluster = xaodCluster.rdoList();
    for (const auto& this_rdo : rod_list_cluster) {
      const int row = pixelID.phi_index(this_rdo);
      if (row > rowmax)
	rowmax = row;
      if (row < rowmin)
	rowmin = row;

      const int col = pixelID.eta_index(this_rdo);
      if (col > colmax)
	colmax = col;
      if (col < colmin)
	colmin = col;
    }

    double etaWidth = design->widthFromColumnRange(colmin, colmax);
    double phiWidth = design->widthFromRowRange(rowmin, rowmax);
    InDet::SiWidth width( Amg::Vector2D(xaodCluster.channelsInPhi(), xaodCluster.channelsInEta()),
			  Amg::Vector2D(phiWidth,etaWidth) );

    indetCluster = new InDet::PixelCluster(id,
					   localPosition,
					   globalPosition,
					   std::vector<Identifier>(xaodCluster.rdoList()),
					   xaodCluster.lvl1a(),
					   std::vector<int>(xaodCluster.totList()),
					   std::vector<float>(xaodCluster.chargeList()),
					   width,
					   &element,
					   std::move(errorMatrix),
					   xaodCluster.omegaX(),
					   xaodCluster.omegaY(),
					   xaodCluster.isSplit(),
					   xaodCluster.splitProbability1(),
					   xaodCluster.splitProbability2());

    return StatusCode::SUCCESS;
  }

  StatusCode convertXaodToInDetCluster(const xAOD::StripCluster& xaodCluster,
                                       const InDetDD::SiDetectorElement& element,
                                       const SCT_ID& stripID,
                                       InDet::SCT_Cluster*& indetCluster,
				       double shift)
  {
    static const double one_over_twelve = 1. / 12.;

    bool isBarrel = element.isBarrel();
    const InDetDD::SCT_ModuleSideDesign* design = nullptr;
    if (not isBarrel) {
      design = dynamic_cast<const InDetDD::StripStereoAnnulusDesign*>(&element.design());
    } else {
      design = dynamic_cast<const InDetDD::SCT_ModuleSideDesign*>(&element.design());
    }

    if (design == nullptr) {
      return StatusCode::FAILURE;
    }

    const auto designShape = design->shape();


    const auto& rdoList = xaodCluster.rdoList();
    Identifier id = rdoList.front();

    const auto& localPos = xaodCluster.localPosition<1>();

    double pos_x = localPos(0, 0);
    double pos_y = 0;
    if (not isBarrel) {
      const Identifier firstStripId = rdoList.front();
      int firstStrip = stripID.strip(firstStripId);
      int stripRow = stripID.row(firstStripId);
      int clusterSizeInStrips = xaodCluster.channelsInPhi();
      auto clusterPosition = design->localPositionOfCluster(design->strip1Dim(firstStrip, stripRow), clusterSizeInStrips);
      pos_x = clusterPosition.xPhi() + shift;
      pos_y = clusterPosition.xEta();
    }

    Amg::Vector2D locpos = Amg::Vector2D( pos_x, pos_y );

    // Most of the following is taken from what is done in ClusterMakerTool
    // Need to make this computation instead of using the local pos
    // with local pos instead some differences w.r.t. reference are observed
    const auto& firstStrip = stripID.strip(rdoList.front());
    const auto& lastStrip = stripID.strip(rdoList.back());
    const auto& row = stripID.row(rdoList.front());
    const int firstStrip1D = design->strip1Dim (firstStrip, row );
    const int lastStrip1D = design->strip1Dim( lastStrip, row );
    const InDetDD::SiCellId cell1(firstStrip1D);
    const InDetDD::SiCellId cell2(lastStrip1D);
    const InDetDD::SiLocalPosition firstStripPos( element.rawLocalPositionOfCell(cell1 ));
    const InDetDD::SiLocalPosition lastStripPos( element.rawLocalPositionOfCell(cell2) );
    const InDetDD::SiLocalPosition centre( (firstStripPos+lastStripPos) * 0.5 );
    const double clusterWidth = design->stripPitch() * ( lastStrip - firstStrip + 1 );

    const std::pair<InDetDD::SiLocalPosition, InDetDD::SiLocalPosition> ends( design->endsOfStrip(centre) );
    const double stripLength( std::abs(ends.first.xEta() - ends.second.xEta()) );

    InDet::SiWidth width( Amg::Vector2D(xaodCluster.channelsInPhi(), 1),
			  Amg::Vector2D(clusterWidth, stripLength) );

    const double col_x = width.colRow().x();
    const double col_y = width.colRow().y();

    double scale_factor = 1.;
    if ( col_x  == 1 )
      scale_factor = 1.05;
    else if ( col_x == 2 )
      scale_factor = 0.27;

    auto errorMatrix = Amg::MatrixX(2,2);
    errorMatrix.setIdentity();
    errorMatrix.fillSymmetric(0, 0, scale_factor * scale_factor * width.phiR() * width.phiR() * one_over_twelve);
    errorMatrix.fillSymmetric(1, 1, width.z() * width.z() / col_y / col_y * one_over_twelve);

    if( designShape == InDetDD::Trapezoid or
	designShape == InDetDD::Annulus) {
      // rotation for endcap SCT

      // The following is being computed with the local position,
      // without considering the lorentz shift
      // So we remove it from the local position
      Amg::Vector2D local(pos_x - shift, pos_y);
      double sn = element.sinStereoLocal(local);
      double sn2 = sn * sn;
      double cs2 = 1. - sn2;
      double w = element.phiPitch(local) / element.phiPitch();
      double v0 = errorMatrix(0,0) * w * w;
      double v1 = errorMatrix(1,1);
      errorMatrix.fillSymmetric( 0, 0, cs2 * v0 + sn2 * v1 );
      errorMatrix.fillSymmetric( 0, 1, sn * std::sqrt(cs2) * (v0 - v1) );
      errorMatrix.fillSymmetric( 1, 1, sn2 * v0 + cs2 * v1 );
    }

    indetCluster = new InDet::SCT_Cluster(id,
					  locpos,
					  std::vector<Identifier>(rdoList),
					  width,
					  &element,
					  std::move(errorMatrix));

    return StatusCode::SUCCESS;
  }

  StatusCode convertXaodToInDetCluster(const xAOD::HGTDCluster& xaodCluster,
                                       const InDetDD::HGTD_DetectorElement& element,
                                       ::HGTD_Cluster*& indetCluster) {

    const auto& locPos = xaodCluster.localPosition<3>(); 
    Amg::Vector2D localPosition(locPos(0,0), locPos(1,0));
    float time = locPos(2,0);

    InDetDD::SiLocalPosition centroid(localPosition);
    const Identifier id = element.identifierOfPosition(centroid);

    auto errorMatrix = Amg::MatrixX(2,2);
    errorMatrix.setIdentity();
    errorMatrix.fillSymmetric(0, 0, xaodCluster.localCovariance<3>()(0, 0));
    errorMatrix.fillSymmetric(1, 1, xaodCluster.localCovariance<3>()(1, 1));    
    float time_resolution = xaodCluster.localCovariance<3>()(2, 2);

    double etaWidth = 1.3;
    double phiWidth = 1.3;
    int channelsPhi = 1;
    int channelsEta = 1;
    InDet::SiWidth width( Amg::Vector2D(channelsPhi, channelsEta), Amg::Vector2D(phiWidth, etaWidth) );

    indetCluster = new ::HGTD_Cluster(id,
				      localPosition,
				      std::vector<Identifier>(xaodCluster.rdoList()),
				      width,
				      &element,
				      std::move(errorMatrix),
				      time,
				      time_resolution,
				      std::vector<int>(xaodCluster.totList()));				      

    return StatusCode::SUCCESS;
  }
  
} // Namespace


