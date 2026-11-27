/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackParticleClusterAssociationAlg.h"

#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include "FourMomUtils/P4Helpers.h"

#include "AthContainers/AuxElement.h"


StatusCode TrackParticleClusterAssociationAlg::initialize()
{

  m_doDetEta = !m_detectorEtaDecor.key().empty();

  ATH_CHECK( m_caloEntryTool.retrieve() );
  ATH_CHECK( m_trackParticleCollectionHandle.initialize() );
  ATH_CHECK( m_caloClusters.initialize() );
  ATH_CHECK( m_detectorEtaDecor.initialize(m_doDetEta) );
  ATH_CHECK( m_assocClustersDecor.initialize() );
  ATH_CHECK( m_sigmaWidthKey.initialize() );
  ATH_CHECK(m_trkLinkKey.initialize());

  ATH_CHECK(m_caloEntryParsDecor.initialize( !m_caloEntryParsDecor.empty() ) );
  ATH_CHECK(m_caloExtKey.initialize( !m_caloEntryParsDecor.empty() ) );

  ATH_CHECK( m_vertexContHandle.initialize( !m_vertexContHandle.empty() ) );
  if(!m_vertexContHandle.empty() ) {
    ATH_CHECK(m_trackvertexassoTool.retrieve() );
  }

  ATH_MSG_DEBUG(" cluster decoration = "<< m_assocClustersDecor.key() );
  return StatusCode::SUCCESS;
}

StatusCode TrackParticleClusterAssociationAlg::execute(const EventContext& ctx) const
{

  ATH_MSG_DEBUG("excute()");
  // get track particles
  SG::ReadHandle<xAOD::TrackParticleContainer> trackParticles(m_trackParticleCollectionHandle, ctx);
  ATH_MSG_DEBUG("retrieved "<< m_trackParticleCollectionHandle.key()<< " size ="<< trackParticles->size() );
  
  // pre-calculate a width of clusters, set it as dynamica attribute so we don't have to recalculate it
  SG::ReadHandle<xAOD::CaloClusterContainer> clusterContainer(m_caloClusters, ctx);
  ATH_MSG_DEBUG("retrieved "<< m_caloClusters.key() << " size = "<< clusterContainer->size() );
  SG::WriteDecorHandle<xAOD::CaloClusterContainer, float> sig_dec (m_sigmaWidthKey, ctx);
  //for(const xAOD::CaloCluster *cl : *clusterContainer){
  for(const xAOD::CaloCluster *cl : *clusterContainer){
    double rad;
    cl->retrieveMoment(xAOD::CaloCluster::SECOND_R,rad);
    double cent;
    cl->retrieveMoment(xAOD::CaloCluster::CENTER_MAG,cent);
    
    float cl_eta {99};
    if (m_doDetEta)
    {
        SG::ReadDecorHandle<xAOD::CaloClusterContainer, float> detEta(m_detectorEtaDecor, ctx);
        cl_eta = detEta(*cl);
    }
    else
        cl_eta = cl->eta();

    double sigmaWidth = 0.0;
    if(cent > 0) sigmaWidth = atan(sqrt(rad)/cent)*cosh(cl_eta);
    sig_dec(*cl) = sigmaWidth;
  }


  const xAOD::Vertex * pv0 = nullptr;
  if(!m_vertexContHandle.empty()){
    SG::ReadHandle<xAOD::VertexContainer> vxCont(m_vertexContHandle, ctx);
    if(!vxCont->empty()) pv0=(*vxCont)[0]; // Hard code HS vertex as PV0
  }

  SG::WriteDecorHandle<xAOD::TrackParticleContainer,
		       std::vector<ElementLink<xAOD::CaloClusterContainer>> > assoClustDecor(m_assocClustersDecor, ctx);
    
  ATH_MSG_DEBUG("will decorate with "<<assoClustDecor.key()<< " and adding trkParam : "<< m_caloEntryParsDecor.key()  );

  // ******************************************
  // main loop over tracks 
  unsigned int ntracks = 0;
  for( const xAOD::TrackParticle* tp : *trackParticles){

    // retrieve the vector of links to cluster (and creating it )
    std::vector< ElementLink< xAOD::CaloClusterContainer > > & caloClusterLinks = assoClustDecor(*tp);

    if( tp->pt() < m_ptCut ) continue;

    if( pv0 != nullptr) if(! m_trackvertexassoTool->isCompatible(*tp, *pv0 )) continue;

    ATH_MSG_DEBUG(" Selected track " << tp->index() << "  pt " << tp->pt() << " eta " << tp->eta() << " phi " << tp->phi() );

    // extrapolate the track to the calo entry
    std::optional<TrackCaloEntryPoint> caloEntry = m_caloEntryTool->caloEntryPoint(ctx, tp);
    if (!caloEntry) {
      ATH_MSG_DEBUG(" Selected track "<< tp->index() << " has no calo entry point ");
      continue;
    }

    // build the associated clusters
    std::vector<const xAOD::CaloCluster*> assoClusters = associatedClusters( *caloEntry, *clusterContainer, ctx);

    // translate vector of links to cluster in ElementLink

    caloClusterLinks.reserve( assoClusters.size() );
    for(const xAOD::CaloCluster* cluster : assoClusters) caloClusterLinks.emplace_back( *clusterContainer,cluster->index() );
    ntracks++;
  }// end loop over tracks

  // 2nd loop over track, only to decorate with Track parameter if requested.
  if (! m_caloEntryParsDecor.empty() ){
    // we can not do this in the above loop because declaring a WriteDecorHandle requires a non empty key
    //  (otherwise : run-time error).
    // IMPORTANT : this assumes a correspondance between the TrackParticleContainer and the CaloExtensionCollection !
    SG::ReadHandle<CaloExtensionCollection> caloExts( m_caloExtKey, ctx );
    SG::WriteDecorHandle<xAOD::TrackParticleContainer,  const Trk::TrackParameters*> trkParamDecor( m_caloEntryParsDecor, ctx );
    for( const xAOD::TrackParticle* tp : *trackParticles){
      const Trk::CaloExtension * caloExtension = (*caloExts)[tp->index() ] ;
      if (caloExtension == nullptr ) trkParamDecor( *tp ) =  nullptr ;
      else trkParamDecor( *tp ) = caloExtension->caloEntryLayerIntersection();      
    }
  }
  
  ATH_MSG_DEBUG(" Total number of selected tracks: " << ntracks );

  return StatusCode::SUCCESS;
}


std::vector<const xAOD::CaloCluster* >
TrackParticleClusterAssociationAlg::associatedClusters(const TrackCaloEntryPoint & caloEntry, const xAOD::CaloClusterContainer & allClusters,
                                                       const EventContext& ctx) const
{
  std::vector<const xAOD::CaloCluster* > clusters;

  const float eta = caloEntry.eta;
  const float phi = caloEntry.phi;
  const double uncertEta = caloEntry.uncertEta;
  const double uncertPhi = caloEntry.uncertPhi;
  const double uncertExtrp = caloEntry.uncertExtrp();

  // --- covariance of the extrapolated (eta, phi) at the calo entry (used when Chi2Match is on) ---
  // computed by the calo entry extrapolation tool from the (theta, phi) covariance
  const double covEtaEta = m_chi2Match ? caloEntry.covEtaEta : 0.;
  const double covPhiPhi = m_chi2Match ? caloEntry.covPhiPhi : 0.;
  const double covEtaPhi = m_chi2Match ? caloEntry.covEtaPhi : 0.;
  // largest eigenvalue of the extrapolation covariance (rotation invariant, includes correlations)
  const double lambdaExtrp = 0.5 * (covEtaEta + covPhiPhi) + std::sqrt(0.25 * (covEtaEta - covPhiPhi) * (covEtaEta - covPhiPhi) + covEtaPhi * covEtaPhi);
  const float dr2Cut0 = m_dr * m_dr;
  // to access the pre-calculated width :
  static const SG::ConstAccessor<float> sig_acc("sigmaWidth");

  for(const xAOD::CaloCluster * cl : allClusters){

    const float dPhi = P4Helpers::deltaPhi(cl->phi(), phi);

    float cl_eta {99};
    if (m_doDetEta) {
      SG::ReadDecorHandle<xAOD::CaloClusterContainer, float> detEta(m_detectorEtaDecor, ctx);
      cl_eta = detEta(*cl);
    } else {
      cl_eta = cl->eta();
    }

    const float dEta = cl_eta - eta;
    const float dr2  = dPhi * dPhi + dEta * dEta;

    if (m_useCovariance) {
      const double sigmaWidth = sig_acc(*cl);
      if (m_chi2Match) {
        // Mahalanobis distance using the full 2x2 covariance
        // (extrapolation + isotropic cluster width): chi2 = d^T (S_extrp + sigmaWidth^2 * 1)^-1 d
        const double s2 = sigmaWidth * sigmaWidth;
        // correlation-aware analogue of "extrapolation uncertainty larger than cluster width":
        // skip if the extrapolation error ellipse extends beyond the cluster width in any direction
        if (lambdaExtrp > s2) {
          ATH_MSG_DEBUG("Extrapolation uncertainty larger than cluster width! Skipping cluster.");
          continue;
        }
        const double a = covEtaEta + s2;
        const double c = covPhiPhi + s2;
        const double b = covEtaPhi;
        const double det = a * c - b * b;
        if (det <= 0.) continue; // degenerate covariance, skip this cluster
        if ( (c * dEta * dEta - 2. * b * dEta * dPhi + a * dPhi * dPhi) / det >= m_chi2Cut) continue;
      } else {
      const double uncertClus = 2. * sigmaWidth * sigmaWidth;
      if (uncertExtrp > uncertClus) {
        ATH_MSG_DEBUG("Extrapolation uncertainty larger than cluster width! Skipping cluster.");
        continue;
      }
      if (dr2 >= (sigmaWidth + uncertEta) * (sigmaWidth + uncertEta) + (sigmaWidth + uncertPhi) * (sigmaWidth + uncertPhi)) continue;
      }

    } else {
      if (dr2 >= dr2Cut0) continue;
    }
    clusters.push_back(cl);
  }

  return clusters;
}
