
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "FPGAClusterConverter.h"

#include "AtlasDetDescr/AtlasDetectorID.h"
#include "InDetIdentifier/PixelID.h" 
#include "InDetIdentifier/SCT_ID.h"
#include "Identifier/IdentifierHash.h"

#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"

#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"

#include "TrkSpacePoint/SpacePointCollection.h"

FPGAClusterConverter::FPGAClusterConverter(const std::string& type, const std::string& name, const IInterface* parent):
  base_class(type, name, parent) {}

StatusCode FPGAClusterConverter::initialize() {

  ATH_MSG_DEBUG("Initializing FPGAClusterConverter...");

  // Get SCT & pixel Identifier helpers
  ATH_CHECK(detStore()->retrieve(m_pixelId, "PixelID"));
  ATH_CHECK(detStore()->retrieve(m_SCTId, "SCT_ID"));
  ATH_CHECK(detStore()->retrieve(m_pixelManager));
  ATH_CHECK(detStore()->retrieve(m_SCTManager));
  ATH_CHECK(m_lorentzAngleToolPixel.retrieve());
  ATH_CHECK(m_lorentzAngleToolStrip.retrieve());

  ATH_CHECK( m_FPGAClusterKey.initialize() );
  ATH_CHECK(m_beamSpotKey.initialize());

  return StatusCode::SUCCESS;

}

// Functions converting collections of FPGATrackSim Hits or Clusters into InDet or xAOD cluster collections / containers

StatusCode FPGAClusterConverter::convertHits(const std::vector<FPGATrackSimHit>& hits,
                                              InDet::PixelClusterCollection& pixelColl,
                                              InDet::SCT_ClusterCollection& SCTColl) const {
  ATH_MSG_DEBUG("Found " << hits.size() << " FPGATrackSimHits [InDet]");
  // reserve some memory
  pixelColl.reserve(hits.size());
  SCTColl.reserve(hits.size());
  for(const FPGATrackSimHit& h : hits) {

      std::vector<Identifier> rdoList;
      ATH_CHECK(getRdoList(rdoList, h));

      std::unique_ptr<InDet::PixelCluster> pixelCl{};
      std::unique_ptr<InDet::SCT_Cluster> SCTCl{};

      if (h.isPixel()) {
        ATH_CHECK(createPixelCluster(h, rdoList, pixelCl));
        if (pixelCl) pixelColl.push_back(std::move(pixelCl));
      }
      if (h.isStrip()) {
        ATH_CHECK(createSCTCluster(h, rdoList, SCTCl));
        if (SCTCl) SCTColl.push_back(std::move(SCTCl));
      }
    }
    
  ATH_MSG_DEBUG("pixelColl size: " << pixelColl.size() << " SCTColl size: " << SCTColl.size() );

  return StatusCode::SUCCESS;
}

// To be used in Track conversion
StatusCode FPGAClusterConverter::convertHits(const std::vector<const FPGATrackSimHit*>& hits,
                                              InDet::PixelClusterCollection& pixelColl,
                                              InDet::SCT_ClusterCollection& SCTColl) const {
  ATH_MSG_DEBUG("Found " << hits.size() << " FPGATrackSimHits [InDet]");
  
  // *** Match FPGATrackSimHit to FPGATrackSimCluster
  SG::ReadHandle<FPGATrackSimClusterCollection> FPGAClustersHandle (m_FPGAClusterKey);
  if (!FPGAClustersHandle.isValid()) {
    ATH_MSG_FATAL("Failed to retrieve FPGATrackSimClusterCollection");
    return StatusCode::FAILURE;
  }
  const FPGATrackSimClusterCollection *FPGAClusterColl = FPGAClustersHandle.cptr();

  for(const FPGATrackSimHit *h : hits){
    IdentifierHash hash = h->getIdentifierHash();
    FPGATrackSimCluster cl;
    for (const FPGATrackSimCluster& cluster: *FPGAClusterColl){
      FPGATrackSimHit clusterEq = cluster.getClusterEquiv();
      if (hash == clusterEq.getIdentifierHash()) {
        cl = cluster;
        break;
      }
    }
    FPGATrackSimHit clEq = cl.getClusterEquiv();

    // --- DEBUG
    ATH_MSG_DEBUG("Hit  identifier " << h->getIdentifierHash());
    ATH_MSG_DEBUG("Cluster identifier " << clEq.getIdentifierHash());
    // ---

    // *** FPGATrackSimCluster matched

    std::vector<Identifier> rdoList;
    ATH_CHECK(getRdoList(rdoList,cl));

    std::unique_ptr<InDet::PixelCluster> pixelCl{};
    std::unique_ptr<InDet::SCT_Cluster> SCTCl{};

    if (clEq.isPixel()) {
      ATH_CHECK(createPixelCluster(clEq, rdoList, pixelCl));
      if (pixelCl) pixelColl.push_back(std::move(pixelCl));
    }
    if (clEq.isStrip()) {
      ATH_CHECK(createSCTCluster(clEq, rdoList, SCTCl));
      if (SCTCl) SCTColl.push_back(std::move(SCTCl));
    }
      
  }

  ATH_MSG_DEBUG("pixelColl size: " << pixelColl.size() << " SCTColl size: " << SCTColl.size());

  return StatusCode::SUCCESS;

}


StatusCode FPGAClusterConverter::convertHits(const std::vector<FPGATrackSimHit>& hits,
                                              xAOD::PixelClusterContainer& pixelCont,
                                              xAOD::StripClusterContainer& SCTCont) const {
  ATH_MSG_DEBUG("Found " << hits.size() << " FPGATrackSimHits [xAOD]");
    // reserve some memory
  pixelCont.reserve(hits.size());
  SCTCont.reserve(hits.size());
  for(const FPGATrackSimHit& h : hits) {

      std::vector<Identifier> rdoList;
      ATH_CHECK(getRdoList(rdoList, h));

      if (h.isPixel()) {
        xAOD::PixelCluster *xaod_pcl = new xAOD::PixelCluster();
        pixelCont.push_back(xaod_pcl);
        ATH_CHECK(createPixelCluster(h, rdoList, *xaod_pcl));
      }
      if (h.isStrip()) {
        xAOD::StripCluster *xaod_scl = new xAOD::StripCluster();
        SCTCont.push_back(xaod_scl);
        ATH_CHECK(createSCTCluster(h, rdoList, *xaod_scl));
        if(!xaod_scl->rdoList().size())
          SCTCont.pop_back();
      }
    }
    
  ATH_MSG_DEBUG("xAOD pixelCont size: " << pixelCont.size() << " xAOD pixelCont size: " <<  SCTCont.size());

  return StatusCode::SUCCESS;
}



StatusCode FPGAClusterConverter::convertClusters(const std::vector<FPGATrackSimCluster>& clusters,
                                                  InDet::PixelClusterCollection& pixelColl,
                                                  InDet::SCT_ClusterCollection& SCTColl) const  {
  ATH_MSG_DEBUG("Found " << clusters.size() << " FPGATrackSimClusters [InDet]");
  // reserve some memory
  pixelColl.reserve(clusters.size());
  SCTColl.reserve(clusters.size());
  for(const FPGATrackSimCluster& cl : clusters) {

    FPGATrackSimHit clEq = cl.getClusterEquiv();
    std::vector<Identifier> rdoList;
    ATH_CHECK(getRdoList(rdoList, cl));

    std::unique_ptr<InDet::PixelCluster> pixelCl{};
    std::unique_ptr<InDet::SCT_Cluster> SCTCl{};

    if (clEq.isPixel()) {
      ATH_CHECK(createPixelCluster(clEq, rdoList, pixelCl));
      if (pixelCl) pixelColl.push_back(std::move(pixelCl));
    }
    if (clEq.isStrip()) {
      ATH_CHECK(createSCTCluster(clEq, rdoList, SCTCl));
      if (SCTCl) SCTColl.push_back(std::move(SCTCl));
    }
  }

  ATH_MSG_DEBUG("pixelColl size: " << pixelColl.size() << " SCTColl size: " << SCTColl.size());

  return StatusCode::SUCCESS;
}

StatusCode FPGAClusterConverter::convertClusters(const std::vector<FPGATrackSimCluster>& clusters,
                                                  xAOD::PixelClusterContainer& pixelCont,
                                                  xAOD::StripClusterContainer& SCTCont) const {
  ATH_MSG_DEBUG("Found " << clusters.size() << " FPGATrackSimClusters [xAOD]");
  // reserve some memory
  pixelCont.reserve(clusters.size());
  SCTCont.reserve(clusters.size());

  for(const FPGATrackSimCluster& cl : clusters) {

    FPGATrackSimHit clEq = cl.getClusterEquiv();

    std::vector<Identifier> rdoList;
    ATH_CHECK(getRdoList(rdoList, cl));

    if (clEq.isPixel()) {
      xAOD::PixelCluster *xaod_pcl = new xAOD::PixelCluster();
      pixelCont.push_back(xaod_pcl);
      ATH_CHECK(createPixelCluster(clEq, rdoList, *xaod_pcl));
    }
    if (clEq.isStrip()) {
      xAOD::StripCluster *xaod_scl = new xAOD::StripCluster();
      SCTCont.push_back(xaod_scl);
      ATH_CHECK(createSCTCluster(clEq, rdoList, *xaod_scl));
    }
  }

  ATH_MSG_DEBUG("xAOD pixelCont size: " << pixelCont.size() << " xAOD SCTCont size: " <<  SCTCont.size());

  return StatusCode::SUCCESS;
}

StatusCode FPGAClusterConverter::convertSpacePoints(const std::vector<FPGATrackSimCluster>& fpgaSPs,
                                                  xAOD::SpacePointContainer& SPStripCont,
                                                  xAOD::SpacePointContainer& SPPixelCont, 
                                                  xAOD::StripClusterContainer& stripClusterCont,
                                                  xAOD::PixelClusterContainer& pixelClusterCont) const {
  ATH_MSG_INFO("Converting Pixel SPs");
  SPPixelCont.reserve(pixelClusterCont.size());
  ATH_CHECK(createPixelSPs(SPPixelCont, pixelClusterCont));
  
  if (not m_skipStripSpacePointFormation) {
    ATH_MSG_INFO("Converting Strip SPs");
    SPStripCont.reserve(fpgaSPs.size());
    for (const FPGATrackSimCluster& cl : fpgaSPs) {
      xAOD::SpacePoint* xaod_sp = new xAOD::SpacePoint();
      SPStripCont.push_back(xaod_sp);
      ATH_CHECK(createSP(cl, *xaod_sp, stripClusterCont));
      if (!xaod_sp->elementIdList().size()) SPStripCont.pop_back();
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode FPGAClusterConverter::createPixelCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, std::unique_ptr<InDet::PixelCluster>& cl) const {
  ATH_MSG_DEBUG("\tCreate InDet::PixelCluster from FPGATrackSimHit");

  IdentifierHash hash = h.getIdentifierHash();
  
  float etaWidth = h.getEtaWidth();
  float phiWidth = h.getPhiWidth();
  int phiIndex = h.getPhiIndex();
  int etaIndex = h.getEtaIndex();

  const InDetDD::SiDetectorElement* pDE = m_pixelManager->getDetectorElement(hash);

  if( !pDE ) {
    ATH_MSG_ERROR("Detector Element doesn't exist " << hash);
    return StatusCode::FAILURE;
  }
 
  // *** Get cell from id
  Identifier wafer_id = m_pixelId->wafer_id(hash);
  Identifier hit_id = m_pixelId->pixel_id(wafer_id, phiIndex, etaIndex); 
  InDetDD::SiCellId cell =  pDE->cellIdFromIdentifier(hit_id);
  if(!cell.isValid()) {
    ATH_MSG_DEBUG("\t\tcell not valid");
    return StatusCode::FAILURE;
  }
  const InDetDD::PixelModuleDesign* design (dynamic_cast<const InDetDD::PixelModuleDesign*>(&pDE->design()));
  
  // **** Get InDet::SiWidth

  int colMin = static_cast<int>(etaIndex-0.5*etaWidth);
  int colMax = colMin+etaWidth;

  int rowMin = static_cast<int>(phiIndex-0.5*phiWidth);
  int rowMax = rowMin+phiWidth;

  double etaW = design->widthFromColumnRange(colMin, colMax-1); 
  double phiW = design->widthFromRowRange(rowMin, rowMax-1); 

  InDet::SiWidth siWidth(Amg::Vector2D(phiWidth,etaWidth),Amg::Vector2D(phiW,etaW));

  // **** Get SiLocalPosition from cell id and define Amg::Vector2D position
  InDetDD::SiLocalPosition silPos(pDE->rawLocalPositionOfCell(cell)); 
  Amg::Vector2D localPos(silPos);

  //TODO: understand if shift is needed
  if (m_doShift) {
    double shift =  m_lorentzAngleToolPixel->getLorentzShift(hash,Gaudi::Hive::currentContext());
    Amg::Vector2D localPosShift(localPos[Trk::locX]+shift,localPos[Trk::locY]); 
    localPos = localPosShift;
  }

  Amg::Vector3D globalPos = pDE->globalPosition(localPos);
  ATH_MSG_DEBUG("\t\tLocal position: x=" << localPos.x() << " y=" << localPos.y() ); 
  ATH_MSG_DEBUG("\t\tGlobal position: x=" << globalPos.x() << " y=" << globalPos.y()  << " z=" << globalPos.z() );

  Amg::MatrixX cov(2,2);
  cov.setZero();

  cov(0,0) = siWidth.phiR()*siWidth.phiR()/12; 
  cov(1,1) = siWidth.z()*siWidth.z()/12;
  float dummy_omegax = 0.5; 
  float dummy_omegay = 0.5;
  bool split = false;
  float splitProb1 = 0;
  float splitProb2 = 0;

  cl = std::make_unique<InDet::PixelCluster>(hit_id, localPos, std::vector<Identifier>(rdoList), siWidth, pDE, Amg::MatrixX(cov), dummy_omegax, dummy_omegay, split, splitProb1, splitProb2);

  return StatusCode::SUCCESS;
}

StatusCode FPGAClusterConverter::createPixelCluster(const FPGATrackSimHit& h,const  std::vector<Identifier>& rdoList, xAOD::PixelCluster &cl) const {
  ATH_MSG_DEBUG("\tCreate xAOD::PixelCluster from FPGATrackSimHit");

  IdentifierHash hash = h.getIdentifierHash();
  
  float etaWidth = h.getEtaWidth();
  float phiWidth = h.getPhiWidth();
  int phiIndex = h.getPhiIndex();
  int etaIndex = h.getEtaIndex();

  const InDetDD::SiDetectorElement* pDE = m_pixelManager->getDetectorElement(hash);

  if( !pDE ) {
    ATH_MSG_ERROR("Detector Element doesn't exist " << hash);
    return StatusCode::FAILURE;
  }
 
  // *** Get cell from id
  Identifier wafer_id = m_pixelId->wafer_id(hash);
  Identifier hit_id = m_pixelId->pixel_id(wafer_id, phiIndex, etaIndex); 
  InDetDD::SiCellId cell =  pDE->cellIdFromIdentifier(hit_id);
  if(!cell.isValid()) {
    ATH_MSG_DEBUG("\t\tcell not valid");
    return StatusCode::FAILURE;
  }
  const InDetDD::PixelModuleDesign* design (dynamic_cast<const InDetDD::PixelModuleDesign*>(&pDE->design()));

  // **** Get InDet::SiWidth

  int colMin = static_cast<int>(etaIndex-0.5*etaWidth);
  int colMax = colMin+etaWidth;

  int rowMin = static_cast<int>(phiIndex-0.5*phiWidth);
  int rowMax = rowMin+phiWidth;

  double etaW = design->widthFromColumnRange(colMin, colMax-1); 
  double phiW = design->widthFromRowRange(rowMin, rowMax-1); 

  InDet::SiWidth siWidth(Amg::Vector2D(phiWidth,etaWidth),Amg::Vector2D(phiW,etaW));

  // **** Get SiLocalPosition from cell id and define Amg::Vector2D position
  InDetDD::SiLocalPosition silPos(pDE->rawLocalPositionOfCell(cell)); 
  Amg::Vector2D localPos(silPos);

  //TODO: understand if shift is needed

  if (m_doShift) {
    double shift =  m_lorentzAngleToolPixel->getLorentzShift(hash,Gaudi::Hive::currentContext());
    Amg::Vector2D localPosShift(localPos[Trk::locX]+shift,localPos[Trk::locY]); 
    localPos = localPosShift;
  }

  ATH_MSG_DEBUG("\t\tLocal position: x=" << localPos.x() << " y=" << localPos.y() ); 

  Amg::MatrixX cov(2,2); 
  cov.setZero();

  cov(0,0) = siWidth.phiR()*siWidth.phiR()/12; 
  cov(1,1) = siWidth.z()*siWidth.z()/12; 

  float omegax = 0.5; 
  float omegay = 0.5;
  bool split = false;
  float splitProb1 = 0;
  float splitProb2 = 0;

  Eigen::Matrix<float,2,1> localPosition(localPos.x(), localPos.y()); 
  Eigen::Matrix<float,2,2> localCovariance;
  localCovariance.setZero();
  localCovariance(0, 0) = cov(0, 0);
  localCovariance(1, 1) = cov(1, 1);

  Eigen::Matrix<float,3,1> globalPosition(h.getX(),h.getY(),h.getZ());
  ATH_MSG_DEBUG("\t\tGlobal position: x=" << globalPosition.x() << " y=" << globalPosition.y()  << " z=" << globalPosition.z() );

  cl.setMeasurement<2>(hash, localPosition, localCovariance);
  cl.setIdentifier( rdoList.front().get_compact() );
  cl.setRDOlist(rdoList);
  cl.globalPosition() = globalPosition; 
  cl.setChannelsInPhiEta(siWidth.colRow()[0], siWidth.colRow()[1]);
  cl.setWidthInEta(static_cast<float>(siWidth.widthPhiRZ()[1]));
  cl.setOmegas(omegax, omegay);
  cl.setIsSplit(split);
  cl.setSplitProbabilities(splitProb1, splitProb2);
  ATH_MSG_DEBUG("\t\txaod width in eta " << cl.widthInEta());

  return StatusCode::SUCCESS;
}

StatusCode FPGAClusterConverter::createSCTCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, std::unique_ptr<InDet::SCT_Cluster>& cl) const {
  ATH_MSG_DEBUG("\t Create InDet::SCTCluster from FPGATrackSimHit ");


  IdentifierHash hash = h.getIdentifierHash();

  float phiWidth = h.getPhiWidth();
  int strip = static_cast<int>(h.getPhiIndex());
  ATH_CHECK(strip >= 0);
  const InDetDD::SiDetectorElement* pDE = m_SCTManager->getDetectorElement(hash);
  ATH_CHECK(pDE != nullptr);


  Identifier wafer_id = m_SCTId->wafer_id(hash);
  Identifier strip_id = m_SCTId->strip_id(wafer_id, strip);
  InDetDD::SiCellId cell =  pDE->cellIdFromIdentifier(strip_id);
  ATH_MSG_DEBUG("\t\tcell: " << cell);
  ATH_MSG_DEBUG("\t\tstrip_id " << strip_id);
  ATH_MSG_DEBUG("\t\tstrip: " << cell);
  ATH_MSG_DEBUG("\t\tStrip from idHelper: " << m_SCTId->strip(strip_id) );

  const InDetDD::SCT_ModuleSideDesign* design; 
  if (pDE->isBarrel()){ 
    design = (static_cast<const InDetDD::SCT_ModuleSideDesign*>(&pDE->design())); 
  } else{ 
    design = (static_cast<const InDetDD::StripStereoAnnulusDesign*>(&pDE->design())); 
  }  

  const int firstStrip = m_SCTId->strip(rdoList.front());
  const int lastStrip = m_SCTId->strip(rdoList.back());
  const int row = m_SCTId->row(rdoList.front());
  const int firstStrip1D = design->strip1Dim (firstStrip, row );
  const int lastStrip1D = design->strip1Dim( lastStrip, row );
  const InDetDD::SiCellId cell1(firstStrip1D);
  const InDetDD::SiCellId cell2(lastStrip1D);
  const InDetDD::SiLocalPosition firstStripPos( pDE->rawLocalPositionOfCell(cell1 ));
  const InDetDD::SiLocalPosition lastStripPos( pDE->rawLocalPositionOfCell(cell2) );
  const InDetDD::SiLocalPosition centre( (firstStripPos+lastStripPos) * 0.5 );
  const double width = design->stripPitch() * ( lastStrip - firstStrip + 1 );

  const std::pair<InDetDD::SiLocalPosition, InDetDD::SiLocalPosition> ends( design->endsOfStrip(centre) );
  const double stripLength( std::abs(ends.first.xEta() - ends.second.xEta()) );

  InDet::SiWidth siWidth(Amg::Vector2D(phiWidth,1), Amg::Vector2D(width,stripLength) ); //TODO: ok??
  Amg::Vector2D localPos(centre.xPhi(),  centre.xEta()); 

  ATH_MSG_DEBUG("\t\tcentre eta: " << centre.xEta() << " phi: " << centre.xPhi());
  ATH_MSG_DEBUG("\t\tStrip length: " << stripLength );
  ATH_MSG_DEBUG("\t\tlocal position before shift: " << localPos.x() << " phi: " << localPos.y());
  if (m_doShift) {
    double shift =  m_lorentzAngleToolStrip->getLorentzShift(hash,Gaudi::Hive::currentContext());
    Amg::Vector2D localPosShift(localPos[Trk::locX]+shift,localPos[Trk::locY]); 
    localPos = localPosShift;
  }

  Amg::Vector3D globalPos = pDE->globalPosition(localPos);
  ATH_MSG_DEBUG("\t\tLocal position: x=" << localPos.x() << " y=" << localPos.y() ); 
  ATH_MSG_DEBUG("\t\tGlobal position: x=" << globalPos.x() << " y=" << globalPos.y()  << " z=" << globalPos.z() );

  // Fill cov matrix. TODO: compare with https://gitlab.cern.ch/atlas/athena/-/blob/main/InnerDetector/InDetRecTools/SiClusterizationTool/src/ClusterMakerTool.cxx and xAOD function
  const double col_x = siWidth.colRow().x();
  const double col_y = siWidth.colRow().y();

  double scale_factor = 1.;
  if ( std::abs(col_x-1) < std::numeric_limits<double>::epsilon() )
    scale_factor = 1.05;
  else if ( std::abs(col_x-2) < std::numeric_limits<double>::epsilon() )
    scale_factor = 0.27;

  auto cov = Amg::MatrixX(2,2);
  cov.setIdentity();
  cov.fillSymmetric(0, 0, scale_factor * scale_factor * siWidth.phiR() * siWidth.phiR() * (1./12.));
  cov.fillSymmetric(1, 1, siWidth.z() * siWidth.z() / col_y / col_y * (1./12.));

  // rotation for endcap SCT 
  if(pDE->design().shape() == InDetDD::Trapezoid || pDE->design().shape() == InDetDD::Annulus) { 
    double sn      = pDE->sinStereoLocal(localPos);  
    double sn2     = sn*sn; 
    double cs2     = 1.-sn2; 
    double w       = pDE->phiPitch(localPos)/pDE->phiPitch();  
    double v0      = (cov)(0,0)*w*w; 
    double v1      = (cov)(1,1); 
    cov.fillSymmetric( 0, 0, cs2 * v0 + sn2 * v1 );
    cov.fillSymmetric( 0, 1, sn * std::sqrt(cs2) * (v0 - v1) );
    cov.fillSymmetric( 1, 1, sn2 * v0 + cs2 * v1 );
  }

  cl = std::make_unique<InDet::SCT_Cluster>(strip_id, localPos, std::vector<Identifier>(rdoList), siWidth, pDE, Amg::MatrixX(cov)); 

  return StatusCode::SUCCESS;
}

StatusCode FPGAClusterConverter::createSCTCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, xAOD::StripCluster& cl) const {
  ATH_MSG_DEBUG("\t Create xAOD::StripCluster from FPGATrackSimHit "); 
  //https://gitlab.cern.ch/atlas/athena/-/blob/main/InnerDetector/InDetRecTools/SiClusterizationTool/src/SCT_ClusteringTool.cxx

  IdentifierHash hash = h.getIdentifierHash();

  float phiWidth = h.getPhiWidth();
  int strip = static_cast<int>(h.getPhiIndex());
  ATH_CHECK(strip >= 0);
  const InDetDD::SiDetectorElement* pDE = m_SCTManager->getDetectorElement(hash);
  ATH_CHECK(pDE != nullptr);

  Identifier wafer_id = m_SCTId->wafer_id(hash);
  Identifier strip_id = m_SCTId->strip_id(wafer_id, strip);
  InDetDD::SiCellId cell =  pDE->cellIdFromIdentifier(strip_id);
  ATH_MSG_DEBUG("\t\tcell: " << cell);
  ATH_MSG_DEBUG("\t\tstrip_id " << strip_id);
  ATH_MSG_DEBUG("\t\tstrip: " << cell);
  ATH_MSG_DEBUG("\t\tStrip from idHelper: " << m_SCTId->strip(strip_id) );

  const InDetDD::SCT_ModuleSideDesign* design; 
  if (pDE->isBarrel()){ 
    design = (static_cast<const InDetDD::SCT_ModuleSideDesign*>(&pDE->design())); 
  } else{ 
    design = (static_cast<const InDetDD::StripStereoAnnulusDesign*>(&pDE->design())); 
  }  

  const int firstStrip = m_SCTId->strip(rdoList.front());
  const int lastStrip = m_SCTId->strip(rdoList.back());
  const int row = m_SCTId->row(rdoList.front());
  const int firstStrip1D = design->strip1Dim (firstStrip, row );
  const int lastStrip1D = design->strip1Dim( lastStrip, row );
  const InDetDD::SiCellId cell1(firstStrip1D);
  const InDetDD::SiCellId cell2(lastStrip1D);
  if (cell2 != design->cellIdInRange(cell2) || cell1 != design->cellIdInRange(cell1)) { // this seems to solve EFTRACK-743
    ATH_MSG_WARNING("Cell ID out of range. Skip making this Strip cluster");
    return StatusCode::SUCCESS;
  }
  const InDetDD::SiLocalPosition firstStripPos( pDE->rawLocalPositionOfCell(cell1 ));
  const InDetDD::SiLocalPosition lastStripPos( pDE->rawLocalPositionOfCell(cell2) );
  const InDetDD::SiLocalPosition centre( (firstStripPos+lastStripPos) * 0.5 );
  const double width = design->stripPitch() * ( lastStrip - firstStrip + 1 );

  const std::pair<InDetDD::SiLocalPosition, InDetDD::SiLocalPosition> ends( design->endsOfStrip(centre) );
  const double stripLength( std::abs(ends.first.xEta() - ends.second.xEta()) );

  InDet::SiWidth siWidth(Amg::Vector2D(phiWidth,1), Amg::Vector2D(width,stripLength) ); //TODO: ok??
  Amg::Vector2D localPos(centre.xPhi(),  centre.xEta()); 
  ATH_MSG_DEBUG("\t\tcentre eta: " << centre.xEta() << " phi: " << centre.xPhi());
  ATH_MSG_DEBUG("\t\tStrip length: " << stripLength );
  ATH_MSG_DEBUG("\t\tlocal position before shift: " << localPos.x() << " phi: " << localPos.y());

  if (m_doShift) {
    double shift =  m_lorentzAngleToolStrip->getLorentzShift(hash,Gaudi::Hive::currentContext());
    Amg::Vector2D localPosShift(localPos[Trk::locX]+shift,localPos[Trk::locY]); 
    localPos = localPosShift;
  }

  ATH_MSG_DEBUG("\t\tLocal position: x=" << localPos.x() << " y=" << localPos.y() ); 

  /* TODO */
  Eigen::Matrix<float,1,1> localPosition;
  Eigen::Matrix<float,1,1> localCovariance;
  localCovariance.setZero();

  if (pDE->isBarrel()) {
    localPosition(0, 0) = localPos.x();
    localCovariance(0, 0) = pDE->phiPitch() * pDE->phiPitch() * (1./12.);
  } else {
    InDetDD::SiCellId cellId = pDE->cellIdOfPosition(localPos);
    const InDetDD::StripStereoAnnulusDesign *designNew = dynamic_cast<const InDetDD::StripStereoAnnulusDesign *>(&pDE->design());
    if ( designNew == nullptr ) return StatusCode::FAILURE;
    InDetDD::SiLocalPosition localInPolar = designNew->localPositionOfCellPC(cellId);
    localPosition(0, 0) = localInPolar.xPhi();
    localCovariance(0, 0) = designNew->phiPitchPhi() * designNew->phiPitchPhi() * (1./12.);
  }

  Eigen::Matrix<float,3,1> globalPosition(h.getX(),h.getY(),h.getZ()); 
  ATH_MSG_DEBUG("\t\tGlobal position: x=" << globalPosition.x() << " y=" << globalPosition.y()  << " z=" << globalPosition.z() );

  cl.setMeasurement<1>(hash, localPosition, localCovariance);
  cl.setIdentifier( rdoList.front().get_compact() );
  cl.setRDOlist(rdoList);
  cl.globalPosition() = globalPosition;
  cl.setChannelsInPhi(siWidth.colRow()[0]);
  
  return StatusCode::SUCCESS;
}

StatusCode FPGAClusterConverter::createPixelCluster(const FPGATrackSimCluster& cluster, std::unique_ptr<InDet::PixelCluster>& cl) const {
  ATH_MSG_DEBUG("\t Create InDet::PixelCluster from FPGATrackSimCluster");
  FPGATrackSimHit clEq = cluster.getClusterEquiv();  
  std::vector<Identifier> rdoList;  
  ATH_CHECK(getRdoList(rdoList, cluster));
  ATH_CHECK(createPixelCluster(clEq, rdoList, cl));
  return StatusCode::SUCCESS;
}

StatusCode FPGAClusterConverter::createPixelCluster(const FPGATrackSimCluster& cluster, xAOD::PixelCluster& cl) const {
  ATH_MSG_DEBUG("\t Create xAOD::PixelCluster from FPGATrackSimCluster");
  FPGATrackSimHit clEq = cluster.getClusterEquiv();  
  std::vector<Identifier> rdoList;  
  ATH_CHECK(getRdoList(rdoList, cluster));
  ATH_CHECK(createPixelCluster(clEq, rdoList, cl));
  return StatusCode::SUCCESS;
}

StatusCode FPGAClusterConverter::createSCTCluster(const FPGATrackSimCluster& cluster, std::unique_ptr<InDet::SCT_Cluster>& cl) const {
  ATH_MSG_DEBUG("\t Create InDet::SCT_Cluster from FPGATrackSimCluster");
  FPGATrackSimHit clEq = cluster.getClusterEquiv();  
  std::vector<Identifier> rdoList;  
  ATH_CHECK(getRdoList(rdoList, cluster));
  ATH_CHECK(createSCTCluster(clEq, rdoList,cl));
  return StatusCode::SUCCESS;
}

StatusCode FPGAClusterConverter::createSCTCluster(const FPGATrackSimCluster& cluster, xAOD::StripCluster& cl) const {
  ATH_MSG_DEBUG("\t Create xAOD::StripCluster from FPGATrackSimCluster");
  FPGATrackSimHit clEq = cluster.getClusterEquiv();  
  std::vector<Identifier> rdoList;  
  ATH_CHECK(getRdoList(rdoList, cluster));
  ATH_CHECK(createSCTCluster(clEq, rdoList, cl));
  return StatusCode::SUCCESS;
}


StatusCode FPGAClusterConverter::createPixelSPs(xAOD::SpacePointContainer& pixelSPs, xAOD::PixelClusterContainer& clustersCont) const {

  for (const xAOD::PixelCluster* p_cl : clustersCont)
  {
    pixelSPs.emplace_back(new xAOD::SpacePoint);

    // Global position 
    Eigen::Matrix<float, 3, 1> globalPos(p_cl->globalPosition().x(), p_cl->globalPosition().y(), p_cl->globalPosition().z());

    // Covariance
    // TODO: check if we need to scale covariance based on rotation matrix like in PixelSpacePointFormationTool.cxx
    const float & cov_r = p_cl->localCovariance<2>()(0,0);
    const float & cov_z = p_cl->localCovariance<2>()(1,0);

    // measurement list
    std::vector< const xAOD::UncalibratedMeasurement* > measurementLinks({ p_cl });

    pixelSPs.back()->setSpacePoint(
      p_cl->identifierHash(),
      globalPos,
      cov_r,
      cov_z,
      measurementLinks
    );
  }

  return StatusCode::SUCCESS;
}


StatusCode FPGAClusterConverter::createSP(const FPGATrackSimCluster& cl, xAOD::SpacePoint& sp , xAOD::StripClusterContainer& clustersCont ) const {

  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey };
  const InDet::BeamSpotData* beamSpot = *beamSpotHandle;
  Amg::Vector3D vertex = beamSpot->beamVtx().position();

  const FPGATrackSimHit& clEq = cl.getClusterEquiv();

  const IdentifierHash& hash = clEq.getIdentifierHash();

  // Global position and covariance 
    
  Eigen::Matrix<float,3,1> globalPos(clEq.getX(),clEq.getY(),clEq.getZ());

  // Covariance
  // TODO: update to ITk? Can it be done as for pixel? (L728-729)?
  // Lines taken from SCT_SpacePoint::setupLocalCovarianceSCT()
  float deltaY = 0.0004; // roughly pitch of SCT (80 mu) / sqrt(12)
  float covTerm = 1600.*deltaY;
  Eigen::Matrix<float, 2, 1> variance(0.1, 8.*covTerm);
  const InDetDD::SiDetectorElement* element =  m_SCTManager->getDetectorElement(hash);
  // Swap r/z covariance terms for endcap clusters
  if ( element->isEndcap() )
      std::swap( variance(0, 0), variance(1, 0) );
  float cov_r = variance(0,0);
  float cov_z = variance(1,0);

  // ***** Get Strips related infos *****

  // idHashes and measurements
  std::vector<unsigned int> idHashList;
  std::vector<const xAOD::UncalibratedMeasurement_v1*> measurements;

  //Get measurements

  float topHalfStripLength, bottomHalfStripLength;
  Amg::Vector3D topStripDirection;
  Amg::Vector3D bottomStripDirection;
  Amg::Vector3D stripCenter1;
  Amg::Vector3D stripCenter2;
  int index = 0;
  for (const FPGATrackSimHit& h : cl.getHitList()) {
    idHashList.push_back(h.getIdentifierHash());
    for (auto orig_cl : clustersCont) {
      if (h.getIdentifierHash()==orig_cl->identifierHash()) {
        if (index == 0) {ATH_CHECK(getStripsInfo(*orig_cl, topHalfStripLength, topStripDirection, stripCenter1));}
        if (index == 1) {ATH_CHECK(getStripsInfo(*orig_cl, bottomHalfStripLength, bottomStripDirection, stripCenter2));}
        measurements.push_back(orig_cl);
        index++;
      }
    }
  }

  Amg::Vector3D topTrajDir = 2. * ( stripCenter1 - vertex);
  Amg::Vector3D topStripCenter = 0.5 * topTrajDir;
  Amg::Vector3D stripCenterDistance = stripCenter1 - stripCenter2;

  ATH_MSG_DEBUG("topHalfStripLength = " << topHalfStripLength << " bottomHalfStripLength = " << bottomHalfStripLength);
  ATH_MSG_DEBUG("topStripDirection = (" << topStripDirection.x() <<", " << topStripDirection.y() <<", " << topStripDirection.z() <<") " << "bottomStripDirection = (" << bottomStripDirection.x() <<", " << bottomStripDirection.y() <<", " << bottomStripDirection.z() <<") " );
  ATH_MSG_DEBUG("stripCenterDistance = (" << stripCenterDistance.x() <<", " << stripCenterDistance.y() <<", " << stripCenterDistance.z() << ")" );
  ATH_MSG_DEBUG("topStripCenter = (" << topStripCenter.x() <<", " << topStripCenter.y() <<", " << topStripCenter.z() << ")" );

  // Fill xAOD::SpacePoint
  sp.setSpacePoint(
    idHashList, 
    globalPos, 
    cov_r, 
    cov_z, 
    measurements,
    topHalfStripLength,
    bottomHalfStripLength,
    topStripDirection.cast<float>(),
    bottomStripDirection.cast<float>(),
    stripCenterDistance.cast<float>(),
    topStripCenter.cast<float>()
  );

  return StatusCode::SUCCESS;
}

StatusCode FPGAClusterConverter::getRdoList(std::vector<Identifier> &rdoList, const FPGATrackSimCluster& cluster) const {

  std::vector<FPGATrackSimHit> hits = cluster.getHitList();

  for (const FPGATrackSimHit& h : hits) {
    IdentifierHash hash = h.getIdentifierHash();
    int phiIndex = h.getPhiIndex();
    int etaIndex = h.getEtaIndex();

    if (h.isPixel()) {
      Identifier wafer_id_hit = m_pixelId->wafer_id(hash);
      Identifier hit_id = m_pixelId->pixel_id(wafer_id_hit, phiIndex, etaIndex); 
      rdoList.push_back(hit_id);
    }
    if (h.isStrip()) {
      Identifier wafer_id_hit = m_SCTId->wafer_id(hash);
      Identifier hit_id = m_SCTId->strip_id(wafer_id_hit, int(phiIndex)); 
      rdoList.push_back(hit_id);
    }
  }

  return StatusCode::SUCCESS;

}


StatusCode FPGAClusterConverter::getRdoList(std::vector<Identifier> &rdoList, const FPGATrackSimHit& h) const {

  IdentifierHash hash = h.getIdentifierHash();
  int phiIndex = h.getPhiIndex();
  int etaIndex = h.getEtaIndex();

  if (h.isPixel()) {
    Identifier wafer_id_hit = m_pixelId->wafer_id(hash);
    Identifier hit_id = m_pixelId->pixel_id(wafer_id_hit, phiIndex, etaIndex); 
    rdoList.push_back(hit_id);
  }
  if (h.isStrip()) {
    Identifier wafer_id_hit = m_SCTId->wafer_id(hash);
    Identifier hit_id = m_SCTId->strip_id(wafer_id_hit, int(phiIndex)); 
    rdoList.push_back(hit_id);
  }

  return StatusCode::SUCCESS;
}

StatusCode FPGAClusterConverter::getStripsInfo(const xAOD::StripCluster& cl, float& halfStripLength, Amg::Vector3D& stripDirection, Amg::Vector3D& stripCenter) const {

  const int &strip = m_SCTId->strip(cl.rdoList().front());
  const IdentifierHash &hash = cl.identifierHash();

  const InDetDD::SiDetectorElement* pDE = m_SCTManager->getDetectorElement(hash);

  const Identifier &wafer_id = m_SCTId->wafer_id(hash);
  const Identifier &strip_id = m_SCTId->strip_id(wafer_id, strip);
  const InDetDD::SiCellId & cell =  pDE->cellIdFromIdentifier(strip_id);

  const InDetDD::SiLocalPosition localPos( pDE->rawLocalPositionOfCell(cell ));
  std::pair<Amg::Vector3D, Amg::Vector3D> end = (pDE->endsOfStrip(localPos));
  stripCenter = 0.5 * (end.first + end.second);
  Amg::Vector3D stripDir = end.first - end.second;
  
  halfStripLength = 0.5 * stripDir.norm();
  stripDirection = stripDir / (2. * (halfStripLength));

  return StatusCode::SUCCESS;
}

