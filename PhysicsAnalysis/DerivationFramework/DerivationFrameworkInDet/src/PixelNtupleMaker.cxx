/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkInDet/PixelNtupleMaker.h"

#include "xAODCore/ShallowCopy.h"
#include "xAODTracking/TrackParticle.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkEventPrimitives/TrackStateDefs.h"
#include "AthContainers/ConstAccessor.h"

#include <vector>
#include <string>

#include "TLorentzVector.h"

DerivationFramework::PixelNtupleMaker::PixelNtupleMaker(const std::string& t, const std::string& n, const IInterface* p) : 
  base_class(t,n,p)
{
}

DerivationFramework::PixelNtupleMaker::~PixelNtupleMaker() = default;

StatusCode DerivationFramework::PixelNtupleMaker::initialize() {
  ATH_CHECK(m_selector.retrieve());
  ATH_CHECK(m_containerKey.initialize());
  ATH_CHECK(m_measurementContainerKey.initialize());
  ATH_CHECK(m_monitoringTracks.initialize());
  return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::PixelNtupleMaker::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::PixelNtupleMaker::addBranches() const {
  const EventContext& ctx = Gaudi::Hive::currentContext();

  SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_containerKey,ctx);
  if (!tracks.isValid()) { return StatusCode::SUCCESS; }

  // Check the event contains tracks
  SG::WriteHandle<xAOD::TrackParticleContainer> PixelMonitoringTrack(m_monitoringTracks,ctx);
  if (PixelMonitoringTrack.record(std::make_unique<xAOD::TrackParticleContainer>(),
                                  std::make_unique<xAOD::TrackParticleAuxContainer>()).isFailure())  {
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> pixClusters(m_measurementContainerKey,ctx);
  if (!pixClusters.isValid()) { return StatusCode::SUCCESS; }

  // monitoring variables.
  const int nbin_charge =  60;
  const int nbin_tot    = 120;
  const int nbin_eta    =  30;
  const int nbin_dedx   = 300;
  const int nbin_size   =  10;
  const int nbin_reso   =  80;
  const int nbin_occ    =  80;

  std::vector<std::vector<int>> clusCharge(11,std::vector<int>(nbin_charge,0));
  std::vector<std::vector<int>> clusToT(11,std::vector<int>(nbin_tot,0));
  std::vector<std::vector<int>> clusEta(11,std::vector<int>(nbin_eta,0));
  std::vector<std::vector<int>> clusHitEta(11,std::vector<int>(nbin_eta,0));
  std::vector<std::vector<int>> clusdEdx(11,std::vector<int>(nbin_dedx,0));
  std::vector<std::vector<int>> clusSizeX(11,std::vector<int>(nbin_size,0));
  std::vector<std::vector<int>> clusSizeZ(11,std::vector<int>(nbin_size,0));
  std::vector<std::vector<int>> clusResidualX(11,std::vector<int>(nbin_reso,0));
  std::vector<std::vector<int>> clusResidualY(11,std::vector<int>(nbin_reso,0));
  std::vector<std::vector<int>> clusHole(11,std::vector<int>(nbin_eta,0));
  std::vector<std::vector<int>> clusOcc(11,std::vector<int>(nbin_occ,0));
 
  std::vector<int> trkEta(nbin_eta,0);
  std::vector<int> trkHole(nbin_eta,0);
  std::vector<int> trkdEdx(nbin_dedx,0);

  float maxPt = 0.0;
  const xAOD::TrackParticle* trk_maxpt = nullptr;

  std::vector<float> tmpCov(15,0.);

  // StoreGateSvc+InDetTrackParticles.msosLink
  static const SG::ConstAccessor<MeasurementsOnTrack>  acc_MeasurementsOnTrack("msosLink");

  for (const xAOD::TrackParticle* trk : *tracks) {
    uint8_t nPixHits = 0;             trk->summaryValue(nPixHits,xAOD::numberOfPixelHits); 
    uint8_t nSCTHits = 0;             trk->summaryValue(nSCTHits,xAOD::numberOfSCTHits); 
    if (trk->pt()<1000.0 && nPixHits<4) { continue; }
    if (trk->pt()<1000.0 && nSCTHits<1) { continue; }

    std::vector<uint64_t> holeIndex;
    std::vector<int> clusterLayer;
    std::vector<int> clusterBEC;
    std::vector<int> clusterModulePhi;
    std::vector<int> clusterModuleEta;
    std::vector<float> clusterCharge;
    std::vector<int> clusterToT;
    std::vector<int> clusterL1A;
    std::vector<int> clusterIsSplit;
    std::vector<int> clusterSize;
    std::vector<int> clusterSizePhi;
    std::vector<int> clusterSizeZ;
    std::vector<bool> isEdge;
    std::vector<bool> isOverflow;
    std::vector<float> trackPhi;
    std::vector<float> trackTheta;
    std::vector<float> trackX;
    std::vector<float> trackY;
    std::vector<float> localX;
    std::vector<float> localY;
    std::vector<float> globalX;
    std::vector<float> globalY;
    std::vector<float> globalZ;
    std::vector<float> unbiasedResidualX;
    std::vector<float> unbiasedResidualY;
    std::vector<int> clusterIsolation10x2;
    std::vector<int> clusterIsolation20x4;
    std::vector<int> numTotalClustersPerModule;
    std::vector<int> numTotalPixelsPerModule;
    std::vector<float> moduleLorentzShift;

    std::vector<std::vector<int>> rdoToT;
    std::vector<std::vector<float>> rdoCharge;
    std::vector<std::vector<int>> rdoPhi;
    std::vector<std::vector<int>> rdoEta;

    const MeasurementsOnTrack& measurementsOnTrack = acc_MeasurementsOnTrack(*trk);
    for (const auto & msos_iter : measurementsOnTrack) {  
      if (!msos_iter.isValid()) { continue; }
      const xAOD::TrackStateValidation* msos = *msos_iter; 
      if (msos->detType()!=Trk::TrackState::Pixel) { continue; } // its a pixel 
      if (msos->type()==Trk::TrackStateOnSurface::Hole) {   // hole
        holeIndex.push_back(msos->detElementId());
        continue; 
      }
      if (!msos->trackMeasurementValidationLink().isValid()) { continue; }
      if (!(*(msos->trackMeasurementValidationLink())))      { continue; }

      const xAOD::TrackMeasurementValidation* msosClus =  *(msos->trackMeasurementValidationLink());        

      static const SG::ConstAccessor<float> chargeAcc("charge");
      static const SG::ConstAccessor<int> layerAcc("layer");
      static const SG::ConstAccessor<int> becAcc("bec");
      static const SG::ConstAccessor<int> phi_moduleAcc("phi_module");
      static const SG::ConstAccessor<int> eta_moduleAcc("eta_module");
      static const SG::ConstAccessor<int> ToTAcc("ToT");
      static const SG::ConstAccessor<int> LVL1AAcc("LVL1A");
      static const SG::ConstAccessor<char> isSplitAcc("isSplit");
      static const SG::ConstAccessor<int> nRDOAcc("nRDO");
      static const SG::ConstAccessor<int> sizePhiAcc("sizePhi");
      static const SG::ConstAccessor<int> sizeZAcc("sizeZ");
      static const SG::ConstAccessor<float> LorentzShiftAcc("LorentzShift");
      static const SG::ConstAccessor<std::vector<int> > rdo_totAcc("rdo_tot");
      static const SG::ConstAccessor<std::vector<float> > rdo_chargeAcc("rdo_charge");

      for (const auto *clus_itr : *pixClusters) {
        if (clus_itr->identifier()!=(msosClus)->identifier()) { continue; }
        if (chargeAcc(*clus_itr)!=chargeAcc(*msosClus)) { continue; }

        clusterLayer.push_back(layerAcc(*clus_itr));
        clusterBEC.push_back(becAcc(*clus_itr));
        clusterModulePhi.push_back(phi_moduleAcc(*clus_itr));
        clusterModuleEta.push_back(eta_moduleAcc(*clus_itr));
        clusterCharge.push_back(chargeAcc(*clus_itr));
        clusterToT.push_back(ToTAcc(*clus_itr));
        clusterL1A.push_back(LVL1AAcc(*clus_itr));
        clusterIsSplit.push_back(isSplitAcc(*clus_itr));
        clusterSize.push_back(nRDOAcc(*clus_itr));
        clusterSizePhi.push_back(sizePhiAcc(*clus_itr));
        clusterSizeZ.push_back(sizeZAcc(*clus_itr));

        trackPhi.push_back(msos->localPhi());
        trackTheta.push_back(msos->localTheta());
        trackX.push_back(msos->localX());
        trackY.push_back(msos->localY());
        localX.push_back(clus_itr->localX());
        localY.push_back(clus_itr->localY());
        globalX.push_back(clus_itr->globalX());
        globalY.push_back(clus_itr->globalY());
        globalZ.push_back(clus_itr->globalZ());
        unbiasedResidualX.push_back(msos->unbiasedResidualX());
        unbiasedResidualY.push_back(msos->unbiasedResidualY());
        moduleLorentzShift.push_back(LorentzShiftAcc(*clus_itr));

        // cluster isolation   IBL:50x250um, PIXEL:50x400um
        //    - isolation region 10x2 = 500x500um for IBL,  500x800um for PIXEL
        int numNeighborCluster10x2 = 0;
        int numNeighborCluster20x4 = 0;
        int nTotalClustersPerModule = 0;
        int nTotalPixelsPerModule = 0;
        for (const auto *clus_neighbor : *pixClusters) {
          if (layerAcc(*clus_neighbor)==layerAcc(*clus_itr)
              && becAcc(*clus_neighbor)==becAcc(*clus_itr)
              && phi_moduleAcc(*clus_neighbor)==phi_moduleAcc(*clus_itr)
              && eta_moduleAcc(*clus_neighbor)==eta_moduleAcc(*clus_itr)) {
            float deltaX = std::abs(clus_neighbor->localX()-clus_itr->localX());
            float deltaY = std::abs(clus_neighbor->localY()-clus_itr->localY());
            nTotalClustersPerModule++;
            nTotalPixelsPerModule += nRDOAcc(*clus_neighbor);
            if (deltaX>0.0 && deltaY>0.0) {
              if (layerAcc(*clus_itr)==0 && becAcc(*clus_itr)==0) {  // IBL
                if (deltaX<0.500 && deltaY<0.500) { numNeighborCluster10x2++; }
                if (deltaX<1.000 && deltaY<1.000) { numNeighborCluster20x4++; }
              }
              else {
                if (deltaX<0.500 && deltaY<0.800) { numNeighborCluster10x2++; }
                if (deltaX<1.000 && deltaY<1.600) { numNeighborCluster20x4++; }
              }
            }
          }
        }
        clusterIsolation10x2.push_back(numNeighborCluster10x2);
        clusterIsolation20x4.push_back(numNeighborCluster20x4);
        numTotalClustersPerModule.push_back(nTotalClustersPerModule);
        numTotalPixelsPerModule.push_back(nTotalPixelsPerModule);

        // is edge pixel?
        // contain overlflow hit?
        bool checkEdge = false;
        bool checkOverflow = false;

        // rdo information 
        std::vector<int> tmpToT;
        std::vector<float> tmpCharge;
        std::vector<int> tmpPhi;
        std::vector<int> tmpEta;

        static const SG::ConstAccessor<std::vector<int> > rdo_phi_pixel_indexAcc("rdo_phi_pixel_index");
        int nrdo = rdo_phi_pixel_indexAcc.isAvailable(*clus_itr) ? rdo_phi_pixel_indexAcc(*clus_itr).size() : -1;
        for (int i=0; i<nrdo; i++) {

          int phi = rdo_phi_pixel_indexAcc(*clus_itr)[i];
          if (phi<5)   { checkEdge=true; }
          if (phi>320) { checkEdge=true; }

          int eta = rdo_phi_pixel_indexAcc(*clus_itr)[i];
          if (layerAcc(*clus_itr)==0 && becAcc(*clus_itr)==0) {  // IBL
            if (eta_moduleAcc(*clus_itr)>-7 && eta_moduleAcc(*clus_itr)<6) { // IBL Planar
              if (eta<5)   { checkEdge=true; }
              if (eta>154) { checkEdge=true; }
            }
            else {  // IBL 3D
              if (eta<5)  { checkEdge=true; }
              if (eta>74) { checkEdge=true; }
            }
          }
          else {
            if (eta<5)   { checkEdge=true; }
            if (eta>154) { checkEdge=true; }
          }

          int tot = rdo_totAcc(*clus_itr)[i];
          if (layerAcc(*clus_itr)==0 && becAcc(*clus_itr)==0) {  // IBL
            if (tot==16) { checkOverflow=true; }
          }
          else if (layerAcc(*clus_itr)==1 && becAcc(*clus_itr)==0) {  // b-layer
            if (tot==150) { checkOverflow=true; }
          }
          else {
            if (tot==255) { checkOverflow=true; }
          }

          float charge = rdo_chargeAcc(*clus_itr)[i];
          if (trk->pt()>2000.0) {
            tmpToT.push_back(tot);
            tmpCharge.push_back(charge);
            tmpPhi.push_back(phi);
            tmpEta.push_back(eta);
          }
        }
        isEdge.push_back(checkEdge);
        isOverflow.push_back(checkOverflow);

        rdoToT.push_back(tmpToT);
        rdoCharge.push_back(tmpCharge);
        rdoPhi.push_back(tmpPhi);
        rdoEta.push_back(tmpEta);
        break;
      }
    }

    if (m_storeMode==1) {
      static const SG::Decorator<float> d0err("d0err");
      static const SG::Decorator<float> z0err("z0err");
      static const SG::Decorator<std::vector<uint64_t>>   HoleIndex("HoleIndex");
      static const SG::Decorator<std::vector<int>>   ClusterLayer("ClusterLayer");
      static const SG::Decorator<std::vector<int>>   ClusterBEC("ClusterBEC");
      static const SG::Decorator<std::vector<int>>   ClusterModulePhi("ClusterModulePhi");
      static const SG::Decorator<std::vector<int>>   ClusterModuleEta("ClusterModuleEta");
      static const SG::Decorator<std::vector<float>> ClusterCharge("ClusterCharge");
      static const SG::Decorator<std::vector<int>>   ClusterToT("ClusterToT");
      static const SG::Decorator<std::vector<int>>   ClusterL1A("ClusterL1A");
      static const SG::Decorator<std::vector<int>>   ClusterIsSplit("ClusterIsSplit");
      static const SG::Decorator<std::vector<int>>   ClusterSize("ClusterSize");
      static const SG::Decorator<std::vector<int>>   ClusterSizePhi("ClusterSizePhi");
      static const SG::Decorator<std::vector<int>>   ClusterSizeZ("ClusterSizeZ");
      static const SG::Decorator<std::vector<bool>>  ClusterIsEdge("ClusterIsEdge");
      static const SG::Decorator<std::vector<bool>>  ClusterIsOverflow("ClusterIsOverflow");
      static const SG::Decorator<std::vector<float>> TrackLocalPhi("TrackLocalPhi");
      static const SG::Decorator<std::vector<float>> TrackLocalTheta("TrackLocalTheta");
      static const SG::Decorator<std::vector<float>> TrackLocalX("TrackLocalX");
      static const SG::Decorator<std::vector<float>> TrackLocalY("TrackLocalY");
      static const SG::Decorator<std::vector<float>> ClusterLocalX("ClusterLocalX");
      static const SG::Decorator<std::vector<float>> ClusterLocalY("ClusterLocalY");
      static const SG::Decorator<std::vector<float>> ClusterGlobalX("ClusterGlobalX");
      static const SG::Decorator<std::vector<float>> ClusterGlobalY("ClusterGlobalY");
      static const SG::Decorator<std::vector<float>> ClusterGlobalZ("ClusterGlobalZ");
      static const SG::Decorator<std::vector<float>> UnbiasedResidualX("UnbiasedResidualX");
      static const SG::Decorator<std::vector<float>> UnbiasedResidualY("UnbiasedResidualY");
      static const SG::Decorator<std::vector<int>>   ClusterIsolation10x2("ClusterIsolation10x2");
      static const SG::Decorator<std::vector<int>>   ClusterIsolation20x4("ClusterIsolation20x4");
      static const SG::Decorator<std::vector<int>>   NumTotalClustersPerModule("NumTotalClustersPerModule");
      static const SG::Decorator<std::vector<int>>   NumTotalPixelsPerModule("NumTotalPixelsPerModule");
      static const SG::Decorator<std::vector<float>> ModuleLorentzShift("ModuleLorentzShift");
      static const SG::Decorator<std::vector<std::vector<int>>>   RdoToT("RdoToT");
      static const SG::Decorator<std::vector<std::vector<float>>> RdoCharge("RdoCharge");
      static const SG::Decorator<std::vector<std::vector<int>>>   RdoPhi("RdoPhi");
      static const SG::Decorator<std::vector<std::vector<int>>>   RdoEta("RdoEta");

      d0err(*trk)                = trk->definingParametersCovMatrixVec().at(0);
      z0err(*trk)                = trk->definingParametersCovMatrixVec().at(2);
      HoleIndex(*trk)            = std::move(holeIndex);
      ClusterLayer(*trk)         = std::move(clusterLayer);
      ClusterBEC(*trk)           = std::move(clusterBEC);
      ClusterModulePhi(*trk)     = std::move(clusterModulePhi);
      ClusterModuleEta(*trk)     = std::move(clusterModuleEta);
      ClusterCharge(*trk)        = std::move(clusterCharge);
      ClusterToT(*trk)           = std::move(clusterToT);
      ClusterL1A(*trk)           = std::move(clusterL1A);
      ClusterIsSplit(*trk)       = std::move(clusterIsSplit);
      ClusterSize(*trk)          = std::move(clusterSize);
      ClusterSizePhi(*trk)       = std::move(clusterSizePhi);
      ClusterSizeZ(*trk)         = std::move(clusterSizeZ);
      ClusterIsEdge(*trk)        = std::move(isEdge);
      ClusterIsOverflow(*trk)    = std::move(isOverflow);
      TrackLocalPhi(*trk)        = std::move(trackPhi);
      TrackLocalTheta(*trk)      = std::move(trackTheta);
      TrackLocalX(*trk)          = std::move(trackX);
      TrackLocalY(*trk)          = std::move(trackY);
      ClusterLocalX(*trk)        = std::move(localX);
      ClusterLocalY(*trk)        = std::move(localY);
      ClusterGlobalX(*trk)       = std::move(globalX);
      ClusterGlobalY(*trk)       = std::move(globalY);
      ClusterGlobalZ(*trk)       = std::move(globalZ);
      UnbiasedResidualX(*trk)    = std::move(unbiasedResidualX);
      UnbiasedResidualY(*trk)    = std::move(unbiasedResidualY);
      ClusterIsolation10x2(*trk) = std::move(clusterIsolation10x2);
      ClusterIsolation20x4(*trk) = std::move(clusterIsolation20x4);
      NumTotalClustersPerModule(*trk) = std::move(numTotalClustersPerModule);
      NumTotalPixelsPerModule(*trk)   = std::move(numTotalPixelsPerModule);
      ModuleLorentzShift(*trk)   = std::move(moduleLorentzShift);
      RdoToT(*trk)    = std::move(rdoToT);
      RdoCharge(*trk) = std::move(rdoCharge);
      RdoPhi(*trk)    = std::move(rdoPhi);
      RdoEta(*trk)    = std::move(rdoEta);
    }

    // further track selection for slimmed track variables
    uint8_t nPixelDeadSensor = 0;     trk->summaryValue(nPixelDeadSensor,xAOD::numberOfPixelDeadSensors);
    uint8_t numberOfPixelHits= 0;     trk->summaryValue(numberOfPixelHits,xAOD::numberOfPixelHits);
    uint8_t numberOfPixelHoles= 0;    trk->summaryValue(numberOfPixelHoles,xAOD::numberOfPixelHoles);
    uint8_t numberOfPixelOutliers= 0; trk->summaryValue(numberOfPixelOutliers,xAOD::numberOfPixelOutliers);
    uint8_t nSCTHoles = 0;            trk->summaryValue(nSCTHoles,xAOD::numberOfSCTHoles); 
    uint8_t nPixSharedHits = 0;       trk->summaryValue(nPixSharedHits,xAOD::numberOfPixelSharedHits); 
    uint8_t nSCTSharedHits = 0;       trk->summaryValue(nSCTSharedHits,xAOD::numberOfSCTSharedHits); 

    // loose track selection
    bool passLooseCut = static_cast<bool>(m_selector->accept(trk));
    if (!passLooseCut) { continue; }

    //==================
    // Efficiency check
    //==================
    int checkIBL(0),check3D(0),checkBLY(0),checkLY1(0),checkLY2(0),checkEA1(0),checkEA2(0),checkEA3(0),checkEC1(0),checkEC2(0),checkEC3(0);
    for (int i=0; i<(int)clusterLayer.size(); i++) {
      if (clusterBEC.at(i)== 0 && clusterLayer.at(i)==0) {
        if (clusterModuleEta.at(i)>-7 && clusterModuleEta.at(i)<6) { checkIBL++; }
        else                                                       { check3D++; }
      }
      else if (clusterBEC.at(i)== 0 && clusterLayer.at(i)==1) { checkBLY++; }
      else if (clusterBEC.at(i)== 0 && clusterLayer.at(i)==2) { checkLY1++; }
      else if (clusterBEC.at(i)== 0 && clusterLayer.at(i)==3) { checkLY2++; }
      else if (clusterBEC.at(i)== 2 && clusterLayer.at(i)==0) { checkEA1++; }
      else if (clusterBEC.at(i)== 2 && clusterLayer.at(i)==1) { checkEA2++; }
      else if (clusterBEC.at(i)== 2 && clusterLayer.at(i)==2) { checkEA3++; }
      else if (clusterBEC.at(i)==-2 && clusterLayer.at(i)==0) { checkEC1++; }
      else if (clusterBEC.at(i)==-2 && clusterLayer.at(i)==1) { checkEC2++; }
      else if (clusterBEC.at(i)==-2 && clusterLayer.at(i)==2) { checkEC3++; }
    }

    if (trk->pt()>2000.0 && nPixelDeadSensor==0) {
      int ietabin = trunc((trk->eta()+3.0)/0.2);

      std::for_each((trkEta.begin()+ietabin),(trkEta.begin()+ietabin+1),[](int &n){ n++; }); 
      if (numberOfPixelHoles>0) { std::for_each((trkHole.begin()+ietabin),(trkHole.begin()+ietabin+1),[](int &n){ n++; }); }

      if (checkBLY>0 && checkLY1>0 && checkLY2>0) { std::for_each((clusEta[0].begin()+ietabin),(clusEta[0].begin()+ietabin+1),[](int &n){ n++; });   if (checkIBL>0) { std::for_each((clusHitEta[0].begin()+ietabin),(clusHitEta[0].begin()+ietabin+1),[](int &n){ n++; }); }}
      if (checkBLY>0 && checkEC2>0 && checkEC3>0) { std::for_each((clusEta[1].begin()+ietabin),(clusEta[1].begin()+ietabin+1),[](int &n){ n++; });   if (check3D>0)  { std::for_each((clusHitEta[1].begin()+ietabin),(clusHitEta[1].begin()+ietabin+1),[](int &n){ n++; }); }}
      if (checkIBL>0 && checkLY1>0 && checkLY2>0) { std::for_each((clusEta[2].begin()+ietabin),(clusEta[2].begin()+ietabin+1),[](int &n){ n++; });   if (checkBLY>0) { std::for_each((clusHitEta[2].begin()+ietabin),(clusHitEta[2].begin()+ietabin+1),[](int &n){ n++; }); }}
      if (checkIBL>0 && checkBLY>0 && checkLY2>0) { std::for_each((clusEta[3].begin()+ietabin),(clusEta[3].begin()+ietabin+1),[](int &n){ n++; });   if (checkLY1>0) { std::for_each((clusHitEta[3].begin()+ietabin),(clusHitEta[3].begin()+ietabin+1),[](int &n){ n++; }); }}
      if (checkIBL>0 && checkBLY>0 && checkLY1>0) { std::for_each((clusEta[4].begin()+ietabin),(clusEta[4].begin()+ietabin+1),[](int &n){ n++; });   if (checkLY2>0) { std::for_each((clusHitEta[4].begin()+ietabin),(clusHitEta[4].begin()+ietabin+1),[](int &n){ n++; }); }}
      if (checkIBL>0 && checkEA2>0 && checkEA3>0) { std::for_each((clusEta[5].begin()+ietabin),(clusEta[5].begin()+ietabin+1),[](int &n){ n++; });   if (checkEA1>0) { std::for_each((clusHitEta[5].begin()+ietabin),(clusHitEta[5].begin()+ietabin+1),[](int &n){ n++; }); }}
      if (checkIBL>0 && checkEA1>0 && checkEA3>0) { std::for_each((clusEta[6].begin()+ietabin),(clusEta[6].begin()+ietabin+1),[](int &n){ n++; });   if (checkEA2>0) { std::for_each((clusHitEta[6].begin()+ietabin),(clusHitEta[6].begin()+ietabin+1),[](int &n){ n++; }); }}
      if (checkIBL>0 && checkEA1>0 && checkEA2>0) { std::for_each((clusEta[7].begin()+ietabin),(clusEta[7].begin()+ietabin+1),[](int &n){ n++; });   if (checkEA3>0) { std::for_each((clusHitEta[7].begin()+ietabin),(clusHitEta[7].begin()+ietabin+1),[](int &n){ n++; }); }}
      if (checkIBL>0 && checkEC2>0 && checkEC3>0) { std::for_each((clusEta[8].begin()+ietabin),(clusEta[8].begin()+ietabin+1),[](int &n){ n++; });   if (checkEC1>0) { std::for_each((clusHitEta[8].begin()+ietabin),(clusHitEta[8].begin()+ietabin+1),[](int &n){ n++; }); }}
      if (checkIBL>0 && checkEC1>0 && checkEC3>0) { std::for_each((clusEta[9].begin()+ietabin),(clusEta[9].begin()+ietabin+1),[](int &n){ n++; });   if (checkEC2>0) { std::for_each((clusHitEta[9].begin()+ietabin),(clusHitEta[9].begin()+ietabin+1),[](int &n){ n++; }); }}
      if (checkIBL>0 && checkEC1>0 && checkEC2>0) { std::for_each((clusEta[10].begin()+ietabin),(clusEta[10].begin()+ietabin+1),[](int &n){ n++; }); if (checkEC3>0) { std::for_each((clusHitEta[10].begin()+ietabin),(clusHitEta[10].begin()+ietabin+1),[](int &n){ n++; }); }}

      for (int i=0; i<(int)holeIndex.size(); i++) {
        int becH, layerH, etaH, phiH;
        GetLayerEtaPhiFromId(holeIndex.at(i),&becH,&layerH,&etaH,&phiH);
        if (becH== 0 && layerH==0) {
          if (etaH>-7 && etaH<6) { std::for_each((clusHole[0].begin()+ietabin),(clusHole[0].begin()+ietabin+1),[](int &n){ n++; }); }
          else                   { std::for_each((clusHole[1].begin()+ietabin),(clusHole[1].begin()+ietabin+1),[](int &n){ n++; }); }
        }
        if (becH== 0 && layerH==1) { std::for_each((clusHole[2].begin()+ietabin),(clusHole[2].begin()+ietabin+1),[](int &n){ n++; }); }
        if (becH== 0 && layerH==2) { std::for_each((clusHole[3].begin()+ietabin),(clusHole[3].begin()+ietabin+1),[](int &n){ n++; }); }
        if (becH== 0 && layerH==3) { std::for_each((clusHole[4].begin()+ietabin),(clusHole[4].begin()+ietabin+1),[](int &n){ n++; }); }
        if (becH== 2 && layerH==0) { std::for_each((clusHole[5].begin()+ietabin),(clusHole[5].begin()+ietabin+1),[](int &n){ n++; }); }
        if (becH== 2 && layerH==1) { std::for_each((clusHole[6].begin()+ietabin),(clusHole[6].begin()+ietabin+1),[](int &n){ n++; }); }
        if (becH== 2 && layerH==2) { std::for_each((clusHole[7].begin()+ietabin),(clusHole[7].begin()+ietabin+1),[](int &n){ n++; }); }
        if (becH==-2 && layerH==0) { std::for_each((clusHole[8].begin()+ietabin),(clusHole[8].begin()+ietabin+1),[](int &n){ n++; }); }
        if (becH==-2 && layerH==1) { std::for_each((clusHole[9].begin()+ietabin),(clusHole[9].begin()+ietabin+1),[](int &n){ n++; }); }
        if (becH==-2 && layerH==2) { std::for_each((clusHole[10].begin()+ietabin),(clusHole[10].begin()+ietabin+1),[](int &n){ n++; }); }
      }
    }

    // Cluster should not contain edge pixels
    int clusterEdge = 0;
    for (int i=0; i<(int)clusterLayer.size(); i++) { clusterEdge+=isEdge.at(i); }

    // Cluster should not contain overflow pixels
    int clusterOverflow = 0;
    for (int i=0; i<(int)clusterLayer.size(); i++) { clusterOverflow+=isOverflow.at(i); }

    // Cluster should not contain split state
    int isSplit = 0;
    for (int i=0; i<(int)clusterLayer.size(); i++) { isSplit+=clusterIsSplit.at(i); }

    // Strong isolation
    int iso20x4 = 0;
    for (int i=0; i<(int)clusterLayer.size(); i++) { iso20x4+=clusterIsolation20x4.at(i); }

    // Good tracks must be required for the dE/dx and Lorentz angle measurements
    bool passCut = false;
    if (numberOfPixelHits>3 && numberOfPixelHoles==0 && nPixelDeadSensor==0 && numberOfPixelOutliers==0 && clusterEdge==0 && clusterOverflow==0 && isSplit==0 && iso20x4==0) {
      if (checkIBL>0 && checkBLY>0 && checkLY1>0 && checkLY2>0) { // Barrel
        passCut = true;
      }
      else if (checkIBL>0 && checkEA1>0 && checkEA2>0 && checkEA3>0) { // Endcap
        passCut = true;
      }
      else if (checkIBL>0 && checkEC1>0 && checkEC2>0 && checkEC3>0) { // Endcap
        passCut = true;
      }
      else if (check3D>0 && checkEC2>0 && checkEC3>0) { // 3D
        passCut = true;
      }
    }
    if (!passCut) { continue; }

    // Cut on angle alpha
    bool isAlphaCut = false;
    for (int i=0; i<(int)clusterLayer.size(); i++) {
      float alpha = std::atan(std::hypot(std::tan(trackTheta[i]),std::tan(trackPhi[i])));
      if (std::cos(alpha)<0.16) { isAlphaCut=true; break; }
    }
    if (isAlphaCut) { continue; }

    if (trk->pt()>maxPt) {
      maxPt = trk->pt();
      trk_maxpt = trk;
    }

    // Cluster study
    float energyPair   = 3.62e-6;   // Electron-hole pair creation energy  e-h/[MeV]
    float siDensity    = 2.329;     // Silicon density [g/cm^3]
    float thicknessIBL = 0.0200;    // thickness for IBL planar [cm]
    float thickness3D  = 0.0230;    // thickness for IBL 3D [cm]
    float thicknessPIX = 0.0250;    // thickness for PIXEL [cm]
    float sumE = 0.0;
    float sumX = 0.0;
    for (int i=0; i<(int)clusterLayer.size(); i++) {

      float alpha = std::atan(std::hypot(std::tan(trackTheta[i]),std::tan(trackPhi[i])));
      float thickness = thicknessPIX;
      if (clusterBEC.at(i)==0 && clusterLayer.at(i)==0) {
        thickness = (clusterModuleEta[i]<6 && clusterModuleEta[i]>-7) ? thicknessIBL : thickness3D;
      }
      float recodEdx = clusterCharge[i]*energyPair/thickness/siDensity*std::cos(alpha);
      sumE += clusterCharge[i]*energyPair;
      sumX += thickness*siDensity/std::cos(alpha);

      int ibc = (clusterCharge[i]>0.0 && clusterCharge[i]<120000.0) ? trunc(clusterCharge[i]/2000.0) : nbin_charge-1; 
      int ibt = (clusterToT[i]>0 && clusterToT[i]<120) ? trunc(clusterToT[i]) : nbin_tot-1;
      int ibe = (recodEdx>0.0 && recodEdx<3.0) ? trunc(recodEdx*100) : nbin_dedx-1;
      int ibx = (clusterSizePhi[i]>-0.5 && clusterSizePhi[i]<9.5) ? trunc(clusterSizePhi[i]) : nbin_size-1;
      int ibz = (clusterSizeZ[i]>-0.5 && clusterSizeZ[i]<9.5) ? trunc(clusterSizeZ[i]) : nbin_size-1;
      int irx = (unbiasedResidualX[i]>-0.4 && unbiasedResidualX[i]<0.4) ? trunc((unbiasedResidualX[i]+0.4)/0.01) : nbin_reso-1;
      int iry = (unbiasedResidualY[i]>-0.4 && unbiasedResidualY[i]<0.4) ? trunc((unbiasedResidualY[i]+0.4)/0.01) : nbin_reso-1;
      int ibo = (numTotalPixelsPerModule[i]>-0.1 && numTotalPixelsPerModule[i]<160) ? trunc(numTotalPixelsPerModule[i]/2.0) : nbin_occ-1;

      int idx = 0;
      if (clusterBEC.at(i)==0 && clusterLayer.at(i)==0) {
        if (clusterModuleEta[i]<6 && clusterModuleEta[i]>-7)  { idx=0; } // IBL-planar
        else                                                  { idx=1; } // 3D sensor
      }
      else if (clusterBEC.at(i)==0 && clusterLayer.at(i)==1)  { idx=2; } // B-layer
      else if (clusterBEC.at(i)==0 && clusterLayer.at(i)==2)  { idx=3; } // Layer-1
      else if (clusterBEC.at(i)==0 && clusterLayer.at(i)==3)  { idx=4; } // Layer-2
      else if (clusterBEC.at(i)==2 && clusterLayer.at(i)==0)  { idx=5; } // Endcap-A1
      else if (clusterBEC.at(i)==2 && clusterLayer.at(i)==1)  { idx=6; } // Endcap-A2
      else if (clusterBEC.at(i)==2 && clusterLayer.at(i)==2)  { idx=7; } // Endcap-A3
      else if (clusterBEC.at(i)==-2 && clusterLayer.at(i)==0) { idx=8; } // Endcap-C1
      else if (clusterBEC.at(i)==-2 && clusterLayer.at(i)==1) { idx=9; } // Endcap-C2
      else if (clusterBEC.at(i)==-2 && clusterLayer.at(i)==2) { idx=10; } // Endcap-C3

      std::for_each((clusCharge[idx].begin()+ibc),(clusCharge[idx].begin()+ibc+1),[](int &n){ n++; });
      std::for_each((clusToT[idx].begin()+ibt),(clusToT[idx].begin()+ibt+1),[](int &n){ n++; });
      std::for_each((clusdEdx[idx].begin()+ibe),(clusdEdx[idx].begin()+ibe+1),[](int &n){ n++; });
      std::for_each((clusSizeX[idx].begin()+ibx),(clusSizeX[idx].begin()+ibx+1),[](int &n){ n++; });
      std::for_each((clusSizeZ[idx].begin()+ibz),(clusSizeZ[idx].begin()+ibz+1),[](int &n){ n++; });
      std::for_each((clusResidualX[idx].begin()+irx),(clusResidualX[idx].begin()+irx+1),[](int &n){ n++; });
      std::for_each((clusResidualY[idx].begin()+iry),(clusResidualY[idx].begin()+iry+1),[](int &n){ n++; });
      std::for_each((clusOcc[idx].begin()+ibo),(clusOcc[idx].begin()+ibo+1),[](int &n){ n++; });
    }

    if (sumX>0.0) {
      int iby = (sumE/sumX>0.0 && sumE/sumX<3.0) ? trunc(sumE/sumX*100) : 299;
      std::for_each((trkdEdx.begin()+iby),(trkdEdx.begin()+iby+1),[](int &n){ n++; });
    }
  }

  if (m_storeMode==2) {
    if (maxPt>0.0) {
      xAOD::TrackParticle* tp = new xAOD::TrackParticle();
      tp->makePrivateStore(*trk_maxpt);
      tp->setDefiningParametersCovMatrixVec(tmpCov);

      static const SG::Decorator<std::vector<int>> TrackEtaIBL("TrackEtaIBL");
      static const SG::Decorator<std::vector<int>> TrackEta3D("TrackEta3D");
      static const SG::Decorator<std::vector<int>> TrackEtaBL("TrackEtaBL");
      static const SG::Decorator<std::vector<int>> TrackEtaL1("TrackEtaL1");
      static const SG::Decorator<std::vector<int>> TrackEtaL2("TrackEtaL2");
      static const SG::Decorator<std::vector<int>> TrackEtaEA1("TrackEtaEA1");
      static const SG::Decorator<std::vector<int>> TrackEtaEA2("TrackEtaEA2");
      static const SG::Decorator<std::vector<int>> TrackEtaEA3("TrackEtaEA3");
      static const SG::Decorator<std::vector<int>> TrackEtaEC1("TrackEtaEC1");
      static const SG::Decorator<std::vector<int>> TrackEtaEC2("TrackEtaEC2");
      static const SG::Decorator<std::vector<int>> TrackEtaEC3("TrackEtaEC3");
      static const SG::Decorator<std::vector<int>> TrackHitEtaIBL("TrackHitEtaIBL");
      static const SG::Decorator<std::vector<int>> TrackHitEta3D("TrackHitEta3D");
      static const SG::Decorator<std::vector<int>> TrackHitEtaBL("TrackHitEtaBL");
      static const SG::Decorator<std::vector<int>> TrackHitEtaL1("TrackHitEtaL1");
      static const SG::Decorator<std::vector<int>> TrackHitEtaL2("TrackHitEtaL2");
      static const SG::Decorator<std::vector<int>> TrackHitEtaEA1("TrackHitEtaEA1");
      static const SG::Decorator<std::vector<int>> TrackHitEtaEA2("TrackHitEtaEA2");
      static const SG::Decorator<std::vector<int>> TrackHitEtaEA3("TrackHitEtaEA3");
      static const SG::Decorator<std::vector<int>> TrackHitEtaEC1("TrackHitEtaEC1");
      static const SG::Decorator<std::vector<int>> TrackHitEtaEC2("TrackHitEtaEC2");
      static const SG::Decorator<std::vector<int>> TrackHitEtaEC3("TrackHitEtaEC3");
      static const SG::Decorator<std::vector<int>> ClusterChargeIBL("ClusterChargeIBL");
      static const SG::Decorator<std::vector<int>> ClusterCharge3D("ClusterCharge3D");
      static const SG::Decorator<std::vector<int>> ClusterChargeBL("ClusterChargeBL");
      static const SG::Decorator<std::vector<int>> ClusterChargeL1("ClusterChargeL1");
      static const SG::Decorator<std::vector<int>> ClusterChargeL2("ClusterChargeL2");
      static const SG::Decorator<std::vector<int>> ClusterChargeEA1("ClusterChargeEA1");
      static const SG::Decorator<std::vector<int>> ClusterChargeEA2("ClusterChargeEA2");
      static const SG::Decorator<std::vector<int>> ClusterChargeEA3("ClusterChargeEA3");
      static const SG::Decorator<std::vector<int>> ClusterChargeEC1("ClusterChargeEC1");
      static const SG::Decorator<std::vector<int>> ClusterChargeEC2("ClusterChargeEC2");
      static const SG::Decorator<std::vector<int>> ClusterChargeEC3("ClusterChargeEC3");
      static const SG::Decorator<std::vector<int>> ClusterToTIBL("ClusterToTIBL");
      static const SG::Decorator<std::vector<int>> ClusterToT3D("ClusterToT3D");
      static const SG::Decorator<std::vector<int>> ClusterToTBL("ClusterToTBL");
      static const SG::Decorator<std::vector<int>> ClusterToTL1("ClusterToTL1");
      static const SG::Decorator<std::vector<int>> ClusterToTL2("ClusterToTL2");
      static const SG::Decorator<std::vector<int>> ClusterToTEA1("ClusterToTEA1");
      static const SG::Decorator<std::vector<int>> ClusterToTEA2("ClusterToTEA2");
      static const SG::Decorator<std::vector<int>> ClusterToTEA3("ClusterToTEA3");
      static const SG::Decorator<std::vector<int>> ClusterToTEC1("ClusterToTEC1");
      static const SG::Decorator<std::vector<int>> ClusterToTEC2("ClusterToTEC2");
      static const SG::Decorator<std::vector<int>> ClusterToTEC3("ClusterToTEC3");
      static const SG::Decorator<std::vector<int>> ClusterdEdxIBL("ClusterdEdxIBL");
      static const SG::Decorator<std::vector<int>> ClusterdEdx3D("ClusterdEdx3D");
      static const SG::Decorator<std::vector<int>> ClusterdEdxBL("ClusterdEdxBL");
      static const SG::Decorator<std::vector<int>> ClusterdEdxL1("ClusterdEdxL1");
      static const SG::Decorator<std::vector<int>> ClusterdEdxL2("ClusterdEdxL2");
      static const SG::Decorator<std::vector<int>> ClusterdEdxEA1("ClusterdEdxEA1");
      static const SG::Decorator<std::vector<int>> ClusterdEdxEA2("ClusterdEdxEA2");
      static const SG::Decorator<std::vector<int>> ClusterdEdxEA3("ClusterdEdxEA3");
      static const SG::Decorator<std::vector<int>> ClusterdEdxEC1("ClusterdEdxEC1");
      static const SG::Decorator<std::vector<int>> ClusterdEdxEC2("ClusterdEdxEC2");
      static const SG::Decorator<std::vector<int>> ClusterdEdxEC3("ClusterdEdxEC3");
      static const SG::Decorator<std::vector<int>> ClusterSizeXIBL("ClusterSizeXIBL");
      static const SG::Decorator<std::vector<int>> ClusterSizeX3D("ClusterSizeX3D");
      static const SG::Decorator<std::vector<int>> ClusterSizeXBL("ClusterSizeXBL");
      static const SG::Decorator<std::vector<int>> ClusterSizeXL1("ClusterSizeXL1");
      static const SG::Decorator<std::vector<int>> ClusterSizeXL2("ClusterSizeXL2");
      static const SG::Decorator<std::vector<int>> ClusterSizeXEA1("ClusterSizeXEA1");
      static const SG::Decorator<std::vector<int>> ClusterSizeXEA2("ClusterSizeXEA2");
      static const SG::Decorator<std::vector<int>> ClusterSizeXEA3("ClusterSizeXEA3");
      static const SG::Decorator<std::vector<int>> ClusterSizeXEC1("ClusterSizeXEC1");
      static const SG::Decorator<std::vector<int>> ClusterSizeXEC2("ClusterSizeXEC2");
      static const SG::Decorator<std::vector<int>> ClusterSizeXEC3("ClusterSizeXEC3");
      static const SG::Decorator<std::vector<int>> ClusterSizeZIBL("ClusterSizeZIBL");
      static const SG::Decorator<std::vector<int>> ClusterSizeZ3D("ClusterSizeZ3D");
      static const SG::Decorator<std::vector<int>> ClusterSizeZBL("ClusterSizeZBL");
      static const SG::Decorator<std::vector<int>> ClusterSizeZL1("ClusterSizeZL1");
      static const SG::Decorator<std::vector<int>> ClusterSizeZL2("ClusterSizeZL2");
      static const SG::Decorator<std::vector<int>> ClusterSizeZEA1("ClusterSizeZEA1");
      static const SG::Decorator<std::vector<int>> ClusterSizeZEA2("ClusterSizeZEA2");
      static const SG::Decorator<std::vector<int>> ClusterSizeZEA3("ClusterSizeZEA3");
      static const SG::Decorator<std::vector<int>> ClusterSizeZEC1("ClusterSizeZEC1");
      static const SG::Decorator<std::vector<int>> ClusterSizeZEC2("ClusterSizeZEC2");
      static const SG::Decorator<std::vector<int>> ClusterSizeZEC3("ClusterSizeZEC3");
      static const SG::Decorator<std::vector<int>> ClusterResidualXIBL("ClusterResidualXIBL");
      static const SG::Decorator<std::vector<int>> ClusterResidualX3D("ClusterResidualX3D");
      static const SG::Decorator<std::vector<int>> ClusterResidualXBL("ClusterResidualXBL");
      static const SG::Decorator<std::vector<int>> ClusterResidualXL1("ClusterResidualXL1");
      static const SG::Decorator<std::vector<int>> ClusterResidualXL2("ClusterResidualXL2");
      static const SG::Decorator<std::vector<int>> ClusterResidualXEA1("ClusterResidualXEA1");
      static const SG::Decorator<std::vector<int>> ClusterResidualXEA2("ClusterResidualXEA2");
      static const SG::Decorator<std::vector<int>> ClusterResidualXEA3("ClusterResidualXEA3");
      static const SG::Decorator<std::vector<int>> ClusterResidualXEC1("ClusterResidualXEC1");
      static const SG::Decorator<std::vector<int>> ClusterResidualXEC2("ClusterResidualXEC2");
      static const SG::Decorator<std::vector<int>> ClusterResidualXEC3("ClusterResidualXEC3");
      static const SG::Decorator<std::vector<int>> ClusterResidualYIBL("ClusterResidualYIBL");
      static const SG::Decorator<std::vector<int>> ClusterResidualY3D("ClusterResidualY3D");
      static const SG::Decorator<std::vector<int>> ClusterResidualYBL("ClusterResidualYBL");
      static const SG::Decorator<std::vector<int>> ClusterResidualYL1("ClusterResidualYL1");
      static const SG::Decorator<std::vector<int>> ClusterResidualYL2("ClusterResidualYL2");
      static const SG::Decorator<std::vector<int>> ClusterResidualYEA1("ClusterResidualYEA1");
      static const SG::Decorator<std::vector<int>> ClusterResidualYEA2("ClusterResidualYEA2");
      static const SG::Decorator<std::vector<int>> ClusterResidualYEA3("ClusterResidualYEA3");
      static const SG::Decorator<std::vector<int>> ClusterResidualYEC1("ClusterResidualYEC1");
      static const SG::Decorator<std::vector<int>> ClusterResidualYEC2("ClusterResidualYEC2");
      static const SG::Decorator<std::vector<int>> ClusterResidualYEC3("ClusterResidualYEC3");
      static const SG::Decorator<std::vector<int>> ClusterHoleIBL("ClusterHoleIBL");
      static const SG::Decorator<std::vector<int>> ClusterHole3D("ClusterHole3D");
      static const SG::Decorator<std::vector<int>> ClusterHoleBL("ClusterHoleBL");
      static const SG::Decorator<std::vector<int>> ClusterHoleL1("ClusterHoleL1");
      static const SG::Decorator<std::vector<int>> ClusterHoleL2("ClusterHoleL2");
      static const SG::Decorator<std::vector<int>> ClusterHoleEA1("ClusterHoleEA1");
      static const SG::Decorator<std::vector<int>> ClusterHoleEA2("ClusterHoleEA2");
      static const SG::Decorator<std::vector<int>> ClusterHoleEA3("ClusterHoleEA3");
      static const SG::Decorator<std::vector<int>> ClusterHoleEC1("ClusterHoleEC1");
      static const SG::Decorator<std::vector<int>> ClusterHoleEC2("ClusterHoleEC2");
      static const SG::Decorator<std::vector<int>> ClusterHoleEC3("ClusterHoleEC3");
      static const SG::Decorator<std::vector<int>> ClusterOccIBL("ClusterOccIBL");
      static const SG::Decorator<std::vector<int>> ClusterOcc3D("ClusterOcc3D");
      static const SG::Decorator<std::vector<int>> ClusterOccBL("ClusterOccBL");
      static const SG::Decorator<std::vector<int>> ClusterOccL1("ClusterOccL1");
      static const SG::Decorator<std::vector<int>> ClusterOccL2("ClusterOccL2");
      static const SG::Decorator<std::vector<int>> ClusterOccEA1("ClusterOccEA1");
      static const SG::Decorator<std::vector<int>> ClusterOccEA2("ClusterOccEA2");
      static const SG::Decorator<std::vector<int>> ClusterOccEA3("ClusterOccEA3");
      static const SG::Decorator<std::vector<int>> ClusterOccEC1("ClusterOccEC1");
      static const SG::Decorator<std::vector<int>> ClusterOccEC2("ClusterOccEC2");
      static const SG::Decorator<std::vector<int>> ClusterOccEC3("ClusterOccEC3");
      static const SG::Decorator<std::vector<int>> TrackALL("TrackALL");
      static const SG::Decorator<std::vector<int>> TrackHOLE("TrackHOLE");
      static const SG::Decorator<std::vector<int>> TrackdEdx("TrackdEdx");

      TrackEtaIBL(*tp)      = std::move(clusEta[0]);
      TrackEta3D(*tp)       = std::move(clusEta[1]);
      TrackEtaBL(*tp)       = std::move(clusEta[2]);
      TrackEtaL1(*tp)       = std::move(clusEta[3]);
      TrackEtaL2(*tp)       = std::move(clusEta[4]);
      TrackEtaEA1(*tp)      = std::move(clusEta[5]);
      TrackEtaEA2(*tp)      = std::move(clusEta[6]);
      TrackEtaEA3(*tp)      = std::move(clusEta[7]);
      TrackEtaEC1(*tp)      = std::move(clusEta[8]);
      TrackEtaEC2(*tp)      = std::move(clusEta[9]);
      TrackEtaEC3(*tp)      = std::move(clusEta[10]);
      TrackHitEtaIBL(*tp)   = std::move(clusHitEta[0]);
      TrackHitEta3D(*tp)    = std::move(clusHitEta[1]);
      TrackHitEtaBL(*tp)    = std::move(clusHitEta[2]);
      TrackHitEtaL1(*tp)    = std::move(clusHitEta[3]);
      TrackHitEtaL2(*tp)    = std::move(clusHitEta[4]);
      TrackHitEtaEA1(*tp)   = std::move(clusHitEta[5]);
      TrackHitEtaEA2(*tp)   = std::move(clusHitEta[6]);
      TrackHitEtaEA3(*tp)   = std::move(clusHitEta[7]);
      TrackHitEtaEC1(*tp)   = std::move(clusHitEta[8]);
      TrackHitEtaEC2(*tp)   = std::move(clusHitEta[9]);
      TrackHitEtaEC3(*tp)   = std::move(clusHitEta[10]);
      ClusterChargeIBL(*tp) = std::move(clusCharge[0]);
      ClusterCharge3D(*tp)  = std::move(clusCharge[1]);
      ClusterChargeBL(*tp)  = std::move(clusCharge[2]);
      ClusterChargeL1(*tp)  = std::move(clusCharge[3]);
      ClusterChargeL2(*tp)  = std::move(clusCharge[4]);
      ClusterChargeEA1(*tp) = std::move(clusCharge[5]);
      ClusterChargeEA2(*tp) = std::move(clusCharge[6]);
      ClusterChargeEA3(*tp) = std::move(clusCharge[7]);
      ClusterChargeEC1(*tp) = std::move(clusCharge[8]);
      ClusterChargeEC2(*tp) = std::move(clusCharge[9]);
      ClusterChargeEC3(*tp) = std::move(clusCharge[10]);
      ClusterToTIBL(*tp)    = std::move(clusToT[0]);
      ClusterToT3D(*tp)     = std::move(clusToT[1]);
      ClusterToTBL(*tp)     = std::move(clusToT[2]);
      ClusterToTL1(*tp)     = std::move(clusToT[3]);
      ClusterToTL2(*tp)     = std::move(clusToT[4]);
      ClusterToTEA1(*tp)    = std::move(clusToT[5]);
      ClusterToTEA2(*tp)    = std::move(clusToT[6]);
      ClusterToTEA3(*tp)    = std::move(clusToT[7]);
      ClusterToTEC1(*tp)    = std::move(clusToT[8]);
      ClusterToTEC2(*tp)    = std::move(clusToT[9]);
      ClusterToTEC3(*tp)    = std::move(clusToT[10]);
      ClusterdEdxIBL(*tp)   = std::move(clusdEdx[0]);
      ClusterdEdx3D(*tp)    = std::move(clusdEdx[1]);
      ClusterdEdxBL(*tp)    = std::move(clusdEdx[2]);
      ClusterdEdxL1(*tp)    = std::move(clusdEdx[3]);
      ClusterdEdxL2(*tp)    = std::move(clusdEdx[4]);
      ClusterdEdxEA1(*tp)   = std::move(clusdEdx[5]);
      ClusterdEdxEA2(*tp)   = std::move(clusdEdx[6]);
      ClusterdEdxEA3(*tp)   = std::move(clusdEdx[7]);
      ClusterdEdxEC1(*tp)   = std::move(clusdEdx[8]);
      ClusterdEdxEC2(*tp)   = std::move(clusdEdx[9]);
      ClusterdEdxEC3(*tp)   = std::move(clusdEdx[10]);
      ClusterSizeXIBL(*tp)  = std::move(clusSizeX[0]);
      ClusterSizeX3D(*tp)   = std::move(clusSizeX[1]);
      ClusterSizeXBL(*tp)   = std::move(clusSizeX[2]);
      ClusterSizeXL1(*tp)   = std::move(clusSizeX[3]);
      ClusterSizeXL2(*tp)   = std::move(clusSizeX[4]);
      ClusterSizeXEA1(*tp)  = std::move(clusSizeX[5]);
      ClusterSizeXEA2(*tp)  = std::move(clusSizeX[6]);
      ClusterSizeXEA3(*tp)  = std::move(clusSizeX[7]);
      ClusterSizeXEC1(*tp)  = std::move(clusSizeX[8]);
      ClusterSizeXEC2(*tp)  = std::move(clusSizeX[9]);
      ClusterSizeXEC3(*tp)  = std::move(clusSizeX[10]);
      ClusterSizeZIBL(*tp)  = std::move(clusSizeZ[0]);
      ClusterSizeZ3D(*tp)   = std::move(clusSizeZ[1]);
      ClusterSizeZBL(*tp)   = std::move(clusSizeZ[2]);
      ClusterSizeZL1(*tp)   = std::move(clusSizeZ[3]);
      ClusterSizeZL2(*tp)   = std::move(clusSizeZ[4]);
      ClusterSizeZEA1(*tp)  = std::move(clusSizeZ[5]);
      ClusterSizeZEA2(*tp)  = std::move(clusSizeZ[6]);
      ClusterSizeZEA3(*tp)  = std::move(clusSizeZ[7]);
      ClusterSizeZEC1(*tp)  = std::move(clusSizeZ[8]);
      ClusterSizeZEC2(*tp)  = std::move(clusSizeZ[9]);
      ClusterSizeZEC3(*tp)  = std::move(clusSizeZ[10]);
      ClusterResidualXIBL(*tp) = std::move(clusResidualX[0]);
      ClusterResidualX3D(*tp)  = std::move(clusResidualX[1]);
      ClusterResidualXBL(*tp)  = std::move(clusResidualX[2]);
      ClusterResidualXL1(*tp)  = std::move(clusResidualX[3]);
      ClusterResidualXL2(*tp)  = std::move(clusResidualX[4]);
      ClusterResidualXEA1(*tp) = std::move(clusResidualX[5]);
      ClusterResidualXEA2(*tp) = std::move(clusResidualX[6]);
      ClusterResidualXEA3(*tp) = std::move(clusResidualX[7]);
      ClusterResidualXEC1(*tp) = std::move(clusResidualX[8]);
      ClusterResidualXEC2(*tp) = std::move(clusResidualX[9]);
      ClusterResidualXEC3(*tp) = std::move(clusResidualX[10]);
      ClusterResidualYIBL(*tp) = std::move(clusResidualY[0]);
      ClusterResidualY3D(*tp)  = std::move(clusResidualY[1]);
      ClusterResidualYBL(*tp)  = std::move(clusResidualY[2]);
      ClusterResidualYL1(*tp)  = std::move(clusResidualY[3]);
      ClusterResidualYL2(*tp)  = std::move(clusResidualY[4]);
      ClusterResidualYEA1(*tp) = std::move(clusResidualY[5]);
      ClusterResidualYEA2(*tp) = std::move(clusResidualY[6]);
      ClusterResidualYEA3(*tp) = std::move(clusResidualY[7]);
      ClusterResidualYEC1(*tp) = std::move(clusResidualY[8]);
      ClusterResidualYEC2(*tp) = std::move(clusResidualY[9]);
      ClusterResidualYEC3(*tp) = std::move(clusResidualY[10]);
      ClusterHoleIBL(*tp) = std::move(clusHole[0]);
      ClusterHole3D(*tp)  = std::move(clusHole[1]);
      ClusterHoleBL(*tp)  = std::move(clusHole[2]);
      ClusterHoleL1(*tp)  = std::move(clusHole[3]);
      ClusterHoleL2(*tp)  = std::move(clusHole[4]);
      ClusterHoleEA1(*tp) = std::move(clusHole[5]);
      ClusterHoleEA2(*tp) = std::move(clusHole[6]);
      ClusterHoleEA3(*tp) = std::move(clusHole[7]);
      ClusterHoleEC1(*tp) = std::move(clusHole[8]);
      ClusterHoleEC2(*tp) = std::move(clusHole[9]);
      ClusterHoleEC3(*tp) = std::move(clusHole[10]);
      ClusterOccIBL(*tp)  = std::move(clusOcc[0]);
      ClusterOcc3D(*tp)   = std::move(clusOcc[1]);
      ClusterOccBL(*tp)   = std::move(clusOcc[2]);
      ClusterOccL1(*tp)   = std::move(clusOcc[3]);
      ClusterOccL2(*tp)   = std::move(clusOcc[4]);
      ClusterOccEA1(*tp)  = std::move(clusOcc[5]);
      ClusterOccEA2(*tp)  = std::move(clusOcc[6]);
      ClusterOccEA3(*tp)  = std::move(clusOcc[7]);
      ClusterOccEC1(*tp)  = std::move(clusOcc[8]);
      ClusterOccEC2(*tp)  = std::move(clusOcc[9]);
      ClusterOccEC3(*tp)  = std::move(clusOcc[10]);
      TrackALL(*tp)       = std::move(trkEta);
      TrackHOLE(*tp)      = std::move(trkHole);
      TrackdEdx(*tp)      = std::move(trkdEdx);

      PixelMonitoringTrack->push_back(tp);
    }
  }
  return StatusCode::SUCCESS;
}  

void DerivationFramework::PixelNtupleMaker::GetLayerEtaPhiFromId(uint64_t id,int *barrelEC, int *layer, int *eta, int *phi) {
  *eta =(id>>43) % 0x20 - 10;
  *phi =(id>>43) % 0x800 / 0x20;
  int layer_index = ((id>>43) / 0x800) & 0xF;

  //A possibility is to use a bit by bit AND: layer_index & 0xF
  switch (layer_index) {
    case 4:
      *barrelEC=-2;
      *layer = 0;
      break;
    case 5:
      *barrelEC=-2;
      *layer = 1;
      break;
    case 6:
      *barrelEC=-2;
      *layer = 2;
      break;
    case 8:
      *barrelEC=0;
      *layer=0;
      break;
    case 9:
      *layer=1;
      *barrelEC=0;
      break;
    case 10:
      *layer=2;
      *barrelEC=0;
      break;
    case 11:
      *barrelEC=0;
      *layer =3;
      break;
    case 12:
      *barrelEC=2;
      *layer=0;
      break;
    case 13:
      *layer =1;
      *barrelEC=2;
      break;
    case 14:
      *layer=2;
      *barrelEC=2;
      break;
  }
}

