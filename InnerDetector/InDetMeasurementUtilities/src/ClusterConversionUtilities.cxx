/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetMeasurementUtilities/ClusterConversionUtilities.h"

#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"

#include "HGTD_PrepRawData/HGTD_Cluster.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterAuxContainer.h"
#include "GeoModelKernel/throwExcept.h"
#include "xAODInDetMeasurement/Utilities.h"

constexpr static double one_over_twelve = 1. / 12.;

namespace TrackingUtilities {

  std::pair<xAOD::MeasVector<3>, xAOD::MeasMatrix<3>> convertHGTD_LocalPosCov(const HGTD_Cluster &cluster) {
    auto localPos = cluster.localPosition();
    auto localCov = cluster.localCovariance();

    const float time = cluster.time();
    const float timeResolution = cluster.timeResolution();

    Eigen::Matrix<float,3,1> localPosition = Eigen::Matrix<float,3,1>::Zero();
    localPosition(0, 0) = localPos.x();
    localPosition(1, 0) = localPos.y();
    localPosition(2, 0)  = time;

    Eigen::Matrix<float,3,3> localCovariance = Eigen::Matrix<float,3,3>::Zero();
    localCovariance(0, 0) = localCov(0, 0);
    localCovariance(1, 1) = localCov(1, 1);
    localCovariance(2, 2) = timeResolution * timeResolution;

    return {localPosition, localCovariance}; 
  }

  StatusCode convertInDetToXaodCluster(const HGTD_Cluster& indetCluster,
               const InDetDD::HGTD_DetectorElement& element,
               xAOD::HGTDCluster& xaodCluster)
  {
    IdentifierHash idHash = element.identifyHash();

    const auto [localPosition, localCovariance] = convertHGTD_LocalPosCov(indetCluster);
        
    const auto& RDOs = indetCluster.rdoList();
    const auto& ToTs = indetCluster.totList();

    xaodCluster.setMeasurement<3>(idHash, localPosition, localCovariance);
    xaodCluster.setIdentifier( indetCluster.identify().get_compact() );
    xaodCluster.setRDOlist(RDOs);
    xaodCluster.setToTlist(ToTs);
    
    return StatusCode::SUCCESS;
  }

  std::pair<xAOD::MeasVector<2>, xAOD::MeasMatrix<2>> convertPix_LocalPosCov(const InDet::PixelCluster &cluster) {
    auto localCov = cluster.localCovariance();

    xAOD::MeasVector<2> localPosition = xAOD::toStorage(cluster.localPosition());

    Eigen::Matrix<float,2,2> localCovariance;
    localCovariance.setZero();
    localCovariance(0, 0) = localCov(0, 0);
    localCovariance(1, 1) = localCov(1, 1);
    //cid 23274
    //coverity[UNINIT:FALSE]
    return {localPosition, localCovariance}; 
  }
  
  StatusCode convertInDetToXaodCluster(const InDet::PixelCluster& indetCluster,
               const InDetDD::SiDetectorElement& element,
               xAOD::PixelCluster& xaodCluster)
  {
    IdentifierHash idHash = element.identifyHash();
    
    const auto [localPosition, localCovariance] = convertPix_LocalPosCov(indetCluster);

    xAOD::MeasVector<3> globalPosition = xAOD::toStorage(indetCluster.globalPosition());
    
    const auto& RDOs = indetCluster.rdoList();
    const auto& ToTs = indetCluster.totList();
    const auto& charges = indetCluster.chargeList();
    const auto& width = indetCluster.width();

    xaodCluster.setMeasurement<2>(idHash, localPosition, localCovariance);
    xaodCluster.setIdentifier( indetCluster.identify().get_compact() );
    xaodCluster.setRDOlist(RDOs);
    xaodCluster.globalPosition() = globalPosition;
    xaodCluster.setToTlist(ToTs);
    xaodCluster.setChargelist(charges);
    xaodCluster.setTotalCharge( xAOD::xAODInDetMeasurement::Utilities::computeTotalCharge(xaodCluster) );
    xaodCluster.setLVL1A(indetCluster.LVL1A());
    xaodCluster.setChannelsInPhiEta(width.colRow()[0], width.colRow()[1]);
    xaodCluster.setWidthInEta(static_cast<float>(width.widthPhiRZ()[1]));

    return StatusCode::SUCCESS;
  }

 std::pair<xAOD::MeasVector<1>, xAOD::MeasMatrix<1>> convertSCT_LocalPosCov(const InDet::SCT_Cluster &cluster, bool isITk) {
    const InDetDD::SiDetectorElement& element{*cluster.detectorElement()};
    auto localPos = cluster.localPosition();

    float localPosition = 0.f, localCovariance = 0.f;
    if (element.isBarrel() or (not isITk)) {
      localPosition = localPos.x();
      localCovariance = element.phiPitch() * element.phiPitch() * one_over_twelve;
    } else {
      InDetDD::SiCellId cellId = element.cellIdOfPosition(localPos);
      const auto* design = dynamic_cast<const InDetDD::StripStereoAnnulusDesign *>(&element.design());
      if ( design == nullptr ) {
        THROW_EXCEPTION("Invalid bounds from "<<cluster);
      }
      InDetDD::SiLocalPosition localInPolar = design->localPositionOfCellPC(cellId);
      localPosition = localInPolar.xPhi();
      localCovariance = design->phiPitchPhi() * design->phiPitchPhi() * one_over_twelve;
    }

    return std::make_pair(xAOD::MeasVector<1>{localPosition}, 
                          xAOD::MeasMatrix<1>{localCovariance});
  }

  StatusCode convertInDetToXaodCluster(const InDet::SCT_Cluster& indetCluster,
               const InDetDD::SiDetectorElement& element,
               xAOD::StripCluster& xaodCluster,
               bool isITk)
  {
    IdentifierHash idHash = element.identifyHash();

    const auto [localPosition, localCovariance] = convertSCT_LocalPosCov(indetCluster, isITk);
   
    auto globalPos = indetCluster.globalPosition();
    Eigen::Matrix<float, 3, 1> globalPosition(globalPos.x(), globalPos.y(), globalPos.z());

    const auto& RDOs = indetCluster.rdoList();
    const auto& width = indetCluster.width();

    xaodCluster.setMeasurement<1>(idHash, localPosition, localCovariance);
    xaodCluster.setIdentifier( indetCluster.identify().get_compact() );
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

    Amg::Vector2D localPosition = xAOD::toEigen(xaodCluster.localPosition<2>());
    
    InDetDD::SiLocalPosition centroid(localPosition);
    const Identifier id = element.identifierOfPosition(centroid);

    Amg::Vector3D globalPosition = xAOD::toEigen(xaodCluster.globalPosition());
    auto errorMatrix = Amg::MatrixX(2,2);
    errorMatrix.setIdentity();
    errorMatrix.fillSymmetric(0, 0, xaodCluster.localCovariance<2>()(0, 0));
    errorMatrix.fillSymmetric(1, 1, xaodCluster.localCovariance<2>()(1, 1));

    int colmax = std::numeric_limits<int>::min();
    int rowmax = std::numeric_limits<int>::min();
    int colmin = std::numeric_limits<int>::max();
    int rowmin = std::numeric_limits<int>::max();

    float qRowMin = 0.f;
    float qRowMax = 0.f;
    float qColMin = 0.f;
    float qColMax = 0.f;
    
    SG::ConstAccessor<SG::JaggedVecElt<Identifier::value_type> >::element_type
       rdo_list_cluster = xaodCluster.rdoList();
    SG::ConstAccessor<SG::JaggedVecElt<float> >::element_type
       charge_list_cluster = xaodCluster.chargeList();
    std::vector<Identifier> rdo_list_new;
    
    auto tot_list = xaodCluster.totList();

    rdo_list_new.reserve(rdo_list_cluster.size());

    if (rdo_list_cluster.size() == charge_list_cluster.size()) {
      for (std::size_t i(0); i<rdo_list_cluster.size(); ++i) {
        Identifier this_rdo(rdo_list_cluster[i]);
        rdo_list_new.push_back(this_rdo);
        const float this_charge=charge_list_cluster[i];
        const int row = pixelID.phi_index(this_rdo);
        if (row > rowmax) {
          rowmax = row;
          qRowMax = this_charge;
        } else if (row == rowmax) {
          qRowMax += this_charge; 
        }
        if (row < rowmin) {  
          rowmin = row;
          qRowMin = this_charge;
        } else if (row == rowmin) {
          qRowMin += this_charge;
        } 
        
        const int col = pixelID.eta_index(this_rdo);
        if (col > colmax) {
          colmax = col;
          qColMax = this_charge;
        } else if (col == colmax) {
          qColMax += this_charge;
        }     
        
        if (col < colmin) {
          colmin = col;
          qColMin = this_charge;
        } else if (col == colmin) {
          qColMin += this_charge;
        }
        
      }//loop on rdo list
    } // check that rdo list has the same size of charge list
     else {
        std::ranges::transform(rdo_list_cluster, std::back_inserter(rdo_list_new),
                               [](const Identifier::value_type& rdo) { return Identifier{rdo}; });  
     }
    // Compute omega for charge interpolation correction (if required)
    // Two pixels may have charge=0 (very rarely, hopefully)
    float omegax = -1.f;
    float omegay = -1.f;
    if(qRowMin + qRowMax > 0) omegax = qRowMax/(qRowMin + qRowMax);
    if(qColMin + qColMax > 0) omegay = qColMax/(qColMin + qColMax);
        
    double etaWidth = design->widthFromColumnRange(colmin, colmax);
    double phiWidth = design->widthFromRowRange(rowmin, rowmax);
    InDet::SiWidth width( Amg::Vector2D(xaodCluster.channelsInPhi(), xaodCluster.channelsInEta()),
                          Amg::Vector2D(phiWidth,etaWidth) );
    indetCluster = new InDet::PixelCluster(id,
                                           localPosition,
                                           globalPosition,
                                           std::move(rdo_list_new),
                                           xaodCluster.lvl1a(),
                                           std::vector<int>(tot_list.begin(), tot_list.end()),
                                           std::vector<float>(charge_list_cluster.begin(),charge_list_cluster.end()),
                                           width,
                                           &element,
                                           std::move(errorMatrix),
                                           omegax, omegay,
                                           false, 0, 0);

    return StatusCode::SUCCESS;
  }

  StatusCode convertXaodToInDetCluster(const xAOD::StripCluster& xaodCluster,
                                       const InDetDD::SiDetectorElement& element,
                                       const SCT_ID& stripID,
                                       InDet::SCT_Cluster*& indetCluster,
                                       double shift)
  {
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


    SG::ConstAccessor<SG::JaggedVecElt<Identifier::value_type> >::element_type
       rdo_list_cluster = xaodCluster.rdoList();
    Identifier id(rdo_list_cluster.front());

    const auto& localPos = xaodCluster.localPosition<1>();

    double pos_x = localPos(0, 0);
    double pos_y = 0;
    if (not isBarrel) {
      const Identifier firstStripId(id);
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
    const auto& firstStrip = stripID.strip(Identifier(rdo_list_cluster.front()));
    const auto& lastStrip = stripID.strip(Identifier(rdo_list_cluster.back()));
    const auto& row = stripID.row(Identifier(rdo_list_cluster.front()));
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

    if( designShape == InDetDD::Trapezoid or designShape == InDetDD::Annulus) {
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
    std::vector<Identifier> rdo_list_new;
    for(Identifier::value_type rdo_id_value : rdo_list_cluster) {
       rdo_list_new.emplace_back(rdo_id_value);
    }

    indetCluster = new InDet::SCT_Cluster(id,
                                          locpos,
                                          std::move(rdo_list_new),
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
    float time = xAOD::HGTDCluster::time(locPos);

    InDetDD::SiLocalPosition centroid(localPosition);
    const Identifier id = element.identifierOfPosition(centroid);

    xAOD::ConstMatrixMap<3> local_covariance(xaodCluster.localCovariance<3>());
    auto errorMatrix = Amg::MatrixX(2,2);
    errorMatrix.setIdentity();
    errorMatrix.fillSymmetric(0, 0, local_covariance(0, 0));
    errorMatrix.fillSymmetric(1, 1, local_covariance(1, 1));
    float time_resolution = std::sqrt(xAOD::HGTDCluster::timeCovariance(local_covariance));

    double etaWidth = 1.3;
    double phiWidth = 1.3;
    int channelsPhi = 1;
    int channelsEta = 1;
    InDet::SiWidth width( Amg::Vector2D(channelsPhi, channelsEta), Amg::Vector2D(phiWidth, etaWidth) );
    std::vector<Identifier> rdo_list;
    rdo_list.reserve(xaodCluster.rdoList().size());
    for (const Identifier::value_type rdo_id_value : xaodCluster.rdoList()) {
       rdo_list.emplace_back(rdo_id_value);
    }

    indetCluster = new ::HGTD_Cluster(id,
              localPosition,
              std::move(rdo_list),
              width,
              &element,
              std::move(errorMatrix),
              time,
              time_resolution,
              std::vector<int>(xaodCluster.totList()));              

    return StatusCode::SUCCESS;
  }
  
} // Namespace


