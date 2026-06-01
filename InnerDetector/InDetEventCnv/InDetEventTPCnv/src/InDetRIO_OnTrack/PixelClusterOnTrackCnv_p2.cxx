/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetEventTPCnv/InDetRIO_OnTrack/PixelClusterOnTrackCnv_p2.h"
#include "TrkEventTPCnv/helpers/EigenHelpers.h"
#include "AthenaKernel/errorcheck.h"
#include "TrkEventTPCnv/TrkEventPrimitives/ErrorMatrixCnv_p1.h"
#include "TrkEventTPCnv/TrkEventPrimitives/LocalParametersCnv_p1.h"
#include "InDetIdentifier/PixelID.h"

void PixelClusterOnTrackCnv_p2::persToTrans( const InDet::PixelClusterOnTrack_p2 *persObj,InDet::PixelClusterOnTrack *transObj, MsgStream &log ){
    //std::cout<<"READING PixelClusterOnTrackCnv_p2"<<std::endl;

    if(!m_isInitialized) {
        if (this->initialize(log) != StatusCode::SUCCESS) {
            log << MSG::FATAL << "Could not initialize PixelClusterOnTrackCnv_p2 " << endmsg;
        }
    }

    ElementLinkToIDCPixelClusterContainer rio;
    m_elCnv.persToTrans(&persObj->m_prdLink,&rio,log);

    Trk::LocalParameters localParams;
    fillTransFromPStore( &m_localParCnv, persObj->m_localParams, &localParams, log );

    Trk::ErrorMatrix dummy;
    Amg::MatrixX localCovariance;
    fillTransFromPStore( &m_errorMxCnv, persObj->m_localErrMat, &dummy, log );
    EigenHelpers::vectorToEigenMatrix(dummy.values, localCovariance, "PixelClusterOnTrackCnv_p2");

    *transObj = InDet::PixelClusterOnTrack(rio,
                                           localParams,
                                           localCovariance,
                                           persObj->m_idDE,
                                           m_pixId->pixel_id(persObj->m_id),
                                           persObj->m_energyLoss,
                                           persObj->m_isFake,
                                           persObj->m_hasClusterAmbiguity,
                                           persObj->m_isbroad
                                           );

    // Attempt to call supertool to fill in detElements
    m_eventCnvTool->recreateRIO_OnTrack(transObj);
    if (transObj->detectorElement()==nullptr) 
        log << MSG::WARNING<<"Unable to reset DetEl for this RIO_OnTrack, "
            << "probably because of a problem with the Identifier/IdentifierHash : ("
            << transObj->identify()<<"/"<<transObj->idDE()<<endmsg;

}


void PixelClusterOnTrackCnv_p2::transToPers( const InDet::PixelClusterOnTrack *transObj, InDet::PixelClusterOnTrack_p2 *persObj, MsgStream &log ) {
  if (transObj==nullptr or persObj==nullptr) return;

  persObj->m_id = transObj->identify().get_compact();
  persObj->m_localParams = toPersistent( &m_localParCnv, &transObj->localParameters(), log );
  Trk::ErrorMatrix pMat;
  EigenHelpers::eigenMatrixToVector(pMat.values, transObj->localCovariance(), "PixelClusterOnTrackCnv_p2");
  persObj->m_localErrMat = toPersistent( &m_errorMxCnv, &pMat, log );
  persObj->m_idDE = transObj->idDE();
  persObj->m_isbroad = transObj->isBroadCluster();
  persObj->m_hasClusterAmbiguity = transObj->hasClusterAmbiguity();
  persObj->m_isFake              = transObj->isFake();
  persObj->m_energyLoss          = transObj->energyLoss();
 
  using PixelContainer = InDet::PixelClusterContainer;

  static const SG::InitializedReadHandleKey<PixelContainer> pixelClusters{
      "PixelClusters"};
  static const SG::InitializedReadHandleKey<PixelContainer> bkgPixelClusters{
      "Bkg_PixelClusters"};
  static const SG::InitializedReadHandleKey<PixelContainer> itkPixelClusters{
      "ITkPixelClusters"};
  static const SG::InitializedReadHandleKey<PixelContainer> bkgItkPixelClusters{
      "Bkg_ITkPixelClusters"};

  struct ContainerCandidate {
    const SG::ReadHandleKey<PixelContainer>& key;
    const char* overlayOutputName;
  };
  static const ContainerCandidate candidates[] = {
      {pixelClusters, "Bkg_PixelClusters"},
      {itkPixelClusters, "Bkg_ITkPixelClusters"},
      {bkgPixelClusters, "Bkg_PixelClusters"},
      {bkgItkPixelClusters, "Bkg_ITkPixelClusters"},
  };

  ElementLink<PixelContainer>::index_type hashAndIndex{0};

  persObj->m_prdLink.m_contName.clear();

  const bool doOverlay = m_eventCnvTool->doTrackOverlay();

  for (const ContainerCandidate& candidate : candidates) {
    const bool found =
        m_eventCnvTool
            ->getHashAndIndex<PixelContainer, InDet::PixelClusterOnTrack>(
                transObj, candidate.key, hashAndIndex);

    if (!found) continue;

    persObj->m_prdLink.m_contName =
        doOverlay ? candidate.overlayOutputName : candidate.key.key();

    break;
  }

  persObj->m_prdLink.m_elementIndex = hashAndIndex;
}

StatusCode PixelClusterOnTrackCnv_p2::initialize(MsgStream &/*log*/) {
    // Do not initialize again:
    m_isInitialized=true;

    // Get Storegate, ID helpers, and so on
    ISvcLocator* svcLocator = Gaudi::svcLocator();
    SmartIF<StoreGateSvc> detStore{svcLocator->service("DetectorStore")};
    CHECK( detStore.isValid() );
    CHECK( detStore->retrieve(m_pixId, "PixelID") );

    return StatusCode::SUCCESS;
}


