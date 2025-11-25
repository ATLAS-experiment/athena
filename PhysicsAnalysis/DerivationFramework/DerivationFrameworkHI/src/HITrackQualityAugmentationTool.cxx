/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkHI/HITrackQualityAugmentationTool.h"

namespace DerivationFramework {

StatusCode HITrackQualityAugmentationTool::initialize()
{
  ATH_CHECK(m_trackParticlesName.initialize());
  ATH_CHECK(m_vertexContainerName.initialize());
  ATH_CHECK(m_eventInfoKey.initialize());
  
  m_decorator = m_trackParticlesName.key() + "." + m_decorator.key();
  ATH_CHECK(m_decorator.initialize());
  
  m_chi2Decorator = m_trackParticlesName.key() + "." + m_chi2Decorator.key();
  ATH_CHECK(m_chi2Decorator.initialize());
  
  m_vertexIndexDecorator = m_trackParticlesName.key() + "." + m_vertexIndexDecorator.key();
  ATH_CHECK(m_vertexIndexDecorator.initialize());
  
  m_covD0Decorator = m_trackParticlesName.key() + "." + m_covD0Decorator.key();
  ATH_CHECK(m_covD0Decorator.initialize());
  
  m_covZ0Decorator = m_trackParticlesName.key() + "." + m_covZ0Decorator.key();
  ATH_CHECK(m_covZ0Decorator.initialize());
  
  m_covThetaDecorator = m_trackParticlesName.key() + "." + m_covThetaDecorator.key();
  ATH_CHECK(m_covThetaDecorator.initialize());

  CHECK(m_trkSelTool_pp.retrieve());
  CHECK(m_trkSelTool_hi_loose.retrieve());
  CHECK(m_trkSelTool_hi_tight.retrieve());
  
  if (!m_trkToLeptonPVTool.empty()) {
    CHECK(m_trkToLeptonPVTool.retrieve());
  }

  return StatusCode::SUCCESS;
}

 
StatusCode HITrackQualityAugmentationTool::addBranches(const EventContext& ctx) const{

      // Get Primary vertex
      SG::ReadHandle<xAOD::VertexContainer> vertices{m_vertexContainerName, ctx};
      if(!vertices.isValid()) {
        ATH_MSG_ERROR ("Couldn't retrieve VertexContainer with key " << m_vertexContainerName.key());
        return StatusCode::FAILURE;
      }
      const xAOD::Vertex* pv(0);
      for (const xAOD::Vertex* vx : *vertices) {
        if (vx->vertexType() == xAOD::VxType::PriVtx) {
          pv = vx;
          break;
        }
      }
 
      // Get EventInfo
      SG::ReadHandle<xAOD::EventInfo> eventInfo{m_eventInfoKey, ctx};
      if(!eventInfo.isValid()) {
        ATH_MSG_ERROR ("Couldn't retrieve EventInfo with key " << m_eventInfoKey.key());
        return StatusCode::FAILURE;
      }
 
      // Get the track container
      SG::ReadHandle<xAOD::TrackParticleContainer> tracks{m_trackParticlesName, ctx};
      if(!tracks.isValid()) {
        ATH_MSG_ERROR ("Couldn't retrieve TrackParticleContainer with key " << m_trackParticlesName.key());
        return StatusCode::FAILURE;
      }
     
      // Decorators
      SG::WriteDecorHandle<xAOD::TrackParticleContainer, unsigned short> decorator{m_decorator, ctx };
      SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> chi2Decorator{m_chi2Decorator, ctx };
      SG::WriteDecorHandle<xAOD::TrackParticleContainer, int> vertexIndexDecorator{m_vertexIndexDecorator, ctx };
      SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> covD0Decorator{m_covD0Decorator, ctx };
      SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> covZ0Decorator{m_covZ0Decorator, ctx };
      SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> covThetaDecorator{m_covThetaDecorator, ctx };
 
      // Get track quality and chi2 to PV
      for(const auto* track:*tracks) {
        if(pv) {
          decorator(*track) = GetTrackQualityNew(track,pv);
          
          // Check if track is associated with the primary vertex
          bool isFromPV = false;
          int vertexIndex = -1;
          
          // Find which vertex this track is associated with
          for (size_t ivx = 0; ivx < vertices->size(); ++ivx) {
            const xAOD::Vertex* vx = (*vertices)[ivx];
            const std::vector<ElementLink<xAOD::TrackParticleContainer>>& trkLinks = vx->trackParticleLinks();
            
            for (const auto& trkLink : trkLinks) {
              if (trkLink.isValid() && *trkLink == track) {
                vertexIndex = static_cast<int>(ivx);
                if (vx->vertexType() == xAOD::VxType::PriVtx) {
                  isFromPV = true;
                }
                break;
              }
            }
            if (vertexIndex >= 0) break;
          }
          
          vertexIndexDecorator(*track) = vertexIndex;
          
          // Calculate chi2 to PV only for non-PV tracks
          float chi2ToPV = -999.0;  // Default for PV tracks
          if (!isFromPV && !m_trkToLeptonPVTool.empty()) {
            std::unique_ptr<xAOD::Vertex> fittedVertex = m_trkToLeptonPVTool->matchTrkToPV(track, pv, eventInfo.cptr());
            if (fittedVertex) {
              chi2ToPV = fittedVertex->chiSquared();
            } else {
              chi2ToPV = -1.0;  // Fit failed for non-PV track
            }
          }
          chi2Decorator(*track) = chi2ToPV;
          
          // Extract covariance matrix diagonal elements
          float covD0 = -999.0;
          float covZ0 = -999.0;
          float covTheta = -999.0;
          try {
            auto covMatrix = track->definingParametersCovMatrix();
            covD0 = covMatrix(0, 0);      // d0 variance
            covZ0 = covMatrix(1, 1);      // z0 variance
            covTheta = covMatrix(3, 3);   // theta variance
          } catch (...) {
            // Covariance matrix not available - keep default -999.0
          }
          covD0Decorator(*track) = covD0;
          covZ0Decorator(*track) = covZ0;
          covThetaDecorator(*track) = covTheta;
        }
        else {
          decorator(*track) = 0;
          chi2Decorator(*track) = -1.0;
          vertexIndexDecorator(*track) = -1;
          covD0Decorator(*track) = -999.0;
          covZ0Decorator(*track) = -999.0;
          covThetaDecorator(*track) = -999.0;
        }
      }
 
      return StatusCode::SUCCESS;
}


unsigned short HITrackQualityAugmentationTool::GetTrackQualityNew(const xAOD::TrackParticle* track,const xAOD::Vertex* pv) const {
  
  static const SG::AuxElement::ConstAccessor<unsigned char> acc_n_sct_hits("numberOfSCTHits");
  int n_sct_hits = acc_n_sct_hits(*track);
    
  
  float d0      = track->d0();
  float z0_wrtPV= track->z0()+track->vz()-pv->z();
  float theta   = track->theta();

  //-------------------------------------------------------------------------------------------------
  bool pass_min_bias=false;
  if (m_trkSelTool_pp->accept(*track, pv)) pass_min_bias=true;
  //-------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------
  bool pass_hi_loose=false;
  if (m_trkSelTool_hi_loose->accept(*track, pv)) pass_hi_loose=true;
  //-------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------
  bool pass_hi_loose_additional_SCT_hit=true;
  if(!pass_hi_loose || n_sct_hits<7) pass_hi_loose_additional_SCT_hit=false;
  //-------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------
  bool pass_hi_loose_tight_d0_z0=true;
  if(!pass_hi_loose || fabs(d0)>1.0 || fabs(z0_wrtPV*sin(theta))>1.0) pass_hi_loose_tight_d0_z0=false;
  //-------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------
  bool pass_hi_loose_tighter_d0_z0=true;
  if(!pass_hi_loose || fabs(d0)>0.5 || fabs(z0_wrtPV*sin(theta))>0.5) pass_hi_loose_tighter_d0_z0=false;
  //-------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------
  bool pass_hi_tight=false;
  if (m_trkSelTool_hi_tight->accept(*track, pv)) pass_hi_tight=true;
  //-------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------
  bool pass_hi_tight_loose_d0_z0=true;
  if(pass_hi_tight==false){
    const auto& taccept = m_trkSelTool_hi_tight->getAcceptInfo();
    asg::AcceptData acceptData(&taccept);
    static const auto d0Index = taccept.getCutPosition("D0");
    static const auto z0Index = taccept.getCutPosition("Z0SinTheta");
    static const auto nCuts = taccept.getNCuts();
    auto cutBitset = acceptData.getCutResultBitSet();
    cutBitset |= (1 << d0Index) | (1 << z0Index);
    if(cutBitset.count() != nCuts                   ) pass_hi_tight_loose_d0_z0=false;
    if(fabs(d0)>1.5 || fabs(z0_wrtPV*sin(theta))>1.5) pass_hi_tight_loose_d0_z0=false;
  }
  //-------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------
  bool pass_hi_tight_tighter_d0_z0=true;
  if(!pass_hi_tight || fabs(d0)>0.5 || fabs(z0_wrtPV*sin(theta))>0.5) pass_hi_tight_tighter_d0_z0=false;
  //-------------------------------------------------------------------------------------------------

  unsigned short    quality =0;
  if(pass_min_bias                   ) quality+=PP_MIN_BIAS;
  if(pass_hi_loose                   ) quality+=HI_LOOSE;
  if(pass_hi_loose_additional_SCT_hit) quality+=HI_LOOSE_7SCT_HITS;
  if(pass_hi_loose_tight_d0_z0       ) quality+=HI_LOOSE_TIGHT_D0_Z0;
  if(pass_hi_loose_tighter_d0_z0     ) quality+=HI_LOOSE_TIGHTER_D0_Z0;
  if(pass_hi_tight_loose_d0_z0       ) quality+=HI_TIGHT_LOOSE_D0_Z0;
  if(pass_hi_tight                   ) quality+=HI_TIGHT;
  if(pass_hi_tight_tighter_d0_z0     ) quality+=HI_TIGHT_TIGHTER_D0_Z0;
  return quality;
}



unsigned short HITrackQualityAugmentationTool::GetTrackQuality(const xAOD::TrackParticle* track,float z_vtx) const {
          //-------------------------------------------------------------------------------------------------
          float pt      = track->pt();
          float eta     = track->eta();
          //float phi     = track->phi();
          
    
          static const SG::AuxElement::ConstAccessor<unsigned char> acc_n_Ipix_hits("numberOfInnermostPixelLayerHits");
          static const SG::AuxElement::ConstAccessor<unsigned char> acc_n_Ipix_expected("expectInnermostPixelLayerHit");
          static const SG::AuxElement::ConstAccessor<unsigned char> acc_n_NIpix_hits("numberOfNextToInnermostPixelLayerHits");
          static const SG::AuxElement::ConstAccessor<unsigned char> acc_n_NIpix_expected("expectNextToInnermostPixelLayerHit");
          static const SG::AuxElement::ConstAccessor<unsigned char> acc_n_sct_hits("numberOfSCTHits");
          static const SG::AuxElement::ConstAccessor<unsigned char> acc_n_pix_hits("numberOfPixelHits");
          static const SG::AuxElement::ConstAccessor<unsigned char> acc_n_sct_holes("numberOfSCTHoles");
          static const SG::AuxElement::ConstAccessor<unsigned char> acc_n_sct_dead("numberOfSCTDeadSensors");
          static const SG::AuxElement::ConstAccessor<unsigned char> acc_n_pix_dead("numberOfPixelDeadSensors");

          int n_Ipix_hits = acc_n_Ipix_hits(*track);
          int n_Ipix_expected = acc_n_Ipix_expected(*track);
          int n_NIpix_hits = acc_n_NIpix_hits(*track);
          int n_NIpix_expected = acc_n_NIpix_expected(*track);
          int n_sct_hits = acc_n_sct_hits(*track);
          int n_pix_hits = acc_n_pix_hits(*track);
          int n_sct_holes = acc_n_sct_holes(*track);
          int n_sct_dead = acc_n_sct_dead(*track);
          int n_pix_dead = acc_n_pix_dead(*track);  

    
          float chi2=track->chiSquared();
          float ndof=track->numberDoF();
    
          float d0      = track->d0();
          float z0_wrtPV= track->z0()+track->vz()-z_vtx;
          float theta   = track->theta();
          //-------------------------------------------------------------------------------------------------


          //-------------------------------------------------------------------------------------------------
          bool pass_min_bias=true;
          {
            if(fabs(eta)>2.5) pass_min_bias=false;
            if(n_Ipix_expected>0){
              if (n_Ipix_hits==0) pass_min_bias=false;
            }
            else{
              if(n_NIpix_expected>0 && n_NIpix_hits==0) pass_min_bias=false;
            }
      
            int n_sct=n_sct_hits+n_sct_dead;
            if     (pt<=300) {if (n_sct <2)  pass_min_bias=false;}
            else if(pt<=400) {if (n_sct <4)  pass_min_bias=false;}
            else if(pt> 400) {if (n_sct <6)  pass_min_bias=false;}
      
            int n_pix=n_pix_hits+n_pix_dead;
            if(n_pix<=0) pass_min_bias=false;
      
            if(fabs(d0)>1.5) pass_min_bias=false;
            if(fabs(z0_wrtPV*sin(theta))>1.5) pass_min_bias=false;
      
            if(pt>10000 && TMath::Prob(chi2,ndof)<=0.01) pass_min_bias=false;
            //if(n_sct_holes>1 || n_pix_holes>0) continue;
            //if(n_pix_hits<3 || n_sct_hits<8) continue;
          }
          //-------------------------------------------------------------------------------------------------


 
          //-------------------------------------------------------------------------------------------------
          bool pass_hi_loose=true;
          {
            if(fabs(eta)>2.5) pass_hi_loose=false;
            if(n_Ipix_expected>0){
              if (n_Ipix_hits==0) pass_hi_loose=false;
            }
            else{
              if(n_NIpix_expected>0 && n_NIpix_hits==0) pass_hi_loose=false;
            }
      
            if(n_pix_hits==0) pass_hi_loose=false;
            if(n_sct_hits< 6) pass_hi_loose=false;
            if(pt>10000 && TMath::Prob(chi2,ndof)<=0.01) pass_hi_loose=false;
            if(fabs(d0) >1.5) pass_hi_loose=false;
            if(fabs(z0_wrtPV*sin(theta))>1.5) pass_hi_loose=false;
          }
          //-------------------------------------------------------------------------------------------------



          //-------------------------------------------------------------------------------------------------
          bool pass_hi_loose_additional_SCT_hit=true;
          if(!pass_hi_loose) pass_hi_loose_additional_SCT_hit=false;
          else{
            if(n_sct_hits<7) pass_hi_loose_additional_SCT_hit=false;
          }
          //-------------------------------------------------------------------------------------------------



          //-------------------------------------------------------------------------------------------------
          bool pass_hi_loose_tight_d0_z0=true;
          if(!pass_hi_loose || fabs(d0)>1.0 || fabs(z0_wrtPV*sin(theta))>1.0) pass_hi_loose_tight_d0_z0=false;
          //-------------------------------------------------------------------------------------------------



          //-------------------------------------------------------------------------------------------------
          bool pass_hi_loose_tighter_d0_z0=true;
          if(!pass_hi_loose || fabs(d0)>0.5 || fabs(z0_wrtPV*sin(theta))>0.5) pass_hi_loose_tighter_d0_z0=false;
          //-------------------------------------------------------------------------------------------------



         //-------------------------------------------------------------------------------------------------
         bool pass_hi_tight_loose_d0_z0=true;
         if(!pass_hi_loose) pass_hi_tight_loose_d0_z0=false;
         else{
           if(n_pix_hits <2  ) pass_hi_tight_loose_d0_z0=false;
           if(n_sct_hits <8  ) pass_hi_tight_loose_d0_z0=false;
           if(n_sct_holes>1  ) pass_hi_tight_loose_d0_z0=false;
           if(ndof==0) pass_hi_tight_loose_d0_z0=false;
           else if(chi2/ndof>6) pass_hi_tight_loose_d0_z0=false;
         }
         //-------------------------------------------------------------------------------------------------



          //-------------------------------------------------------------------------------------------------
          bool pass_hi_tight=true;
          if(!pass_hi_loose) pass_hi_tight=false;
          else{
            if(n_pix_hits <2  ) pass_hi_tight=false;
            if(n_sct_hits <8  ) pass_hi_tight=false;
            if(n_sct_holes>1  ) pass_hi_tight=false;
            if(fabs(d0)   >1.0) pass_hi_tight=false;
            if(fabs(z0_wrtPV*sin(theta))>1.0) pass_hi_tight=false;
            if(ndof==0) pass_hi_tight=false;
            else if(chi2/ndof>6) pass_hi_tight=false;
          }
          //-------------------------------------------------------------------------------------------------



          //-------------------------------------------------------------------------------------------------
          bool pass_hi_tight_tighter_d0_z0=true;
          if(!pass_hi_tight) pass_hi_tight_tighter_d0_z0=false;
          else{
            if(fabs(d0)>0.5 || fabs(z0_wrtPV*sin(theta))>0.5) pass_hi_tight_tighter_d0_z0=false;
          }
          //-------------------------------------------------------------------------------------------------





          unsigned short    quality =0;
          if(pass_min_bias                   ) quality+=PP_MIN_BIAS;
          if(pass_hi_loose                   ) quality+=HI_LOOSE;
          if(pass_hi_loose_additional_SCT_hit) quality+=HI_LOOSE_7SCT_HITS;
          if(pass_hi_loose_tight_d0_z0       ) quality+=HI_LOOSE_TIGHT_D0_Z0;
          if(pass_hi_loose_tighter_d0_z0     ) quality+=HI_LOOSE_TIGHTER_D0_Z0;
          if(pass_hi_tight_loose_d0_z0       ) quality+=HI_TIGHT_LOOSE_D0_Z0;
          if(pass_hi_tight                   ) quality+=HI_TIGHT;
          if(pass_hi_tight_tighter_d0_z0     ) quality+=HI_TIGHT_TIGHTER_D0_Z0;
          return quality;
}

}