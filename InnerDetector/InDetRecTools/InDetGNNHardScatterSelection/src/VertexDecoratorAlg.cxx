/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "VertexDecoratorAlg.h"
#include "InDetTruthVertexValidation/InDetVertexTruthMatchUtils.h"
#include "xAODEgamma/ElectronxAODHelpers.h"
#include "xAODEgamma/EgammaContainer.h"
#include "AthContainers/ConstDataVector.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "PhotonVertexSelection/PhotonVertexHelpers.h"


namespace InDetGNNHardScatterSelection
{
  VertexDecoratorAlg ::VertexDecoratorAlg(const std::string &name,
                                          ISvcLocator *pSvcLocator)
      : AthHistogramAlgorithm (name, pSvcLocator)
  {
  }

  StatusCode VertexDecoratorAlg ::initialize()
  {
    ATH_CHECK(m_vertexInKey.initialize());
    ATH_CHECK(m_eventInKey.initialize());
    ATH_CHECK(m_photonsInKey.initialize());
    ATH_CHECK(m_electronsInKey.initialize());
    ATH_CHECK(m_muonsInKey.initialize());
    ATH_CHECK(m_jetsInKey.initialize());

    const std::string baseName = m_vertexInKey.key();

    // WriteHandleKeys
    m_photonLinksKey = baseName + ".photonLinks";
    m_jetLinksKey = baseName + ".jetLinks";
    m_electronLinksKey = baseName + ".electronLinks";
    m_muonLinksKey = baseName + ".muonLinks";
    m_mDecor_ntrk = baseName + "." + m_mDecor_ntrk.key();
    m_mDecor_sumPt = baseName + "." + m_mDecor_sumPt.key();
    m_mDecor_chi2Over_ndf = baseName + "." + m_mDecor_chi2Over_ndf.key();
    m_mDecor_z_asym = baseName + "." + m_mDecor_z_asym.key();
    m_mDecor_weighted_z_asym = baseName + "." + m_mDecor_weighted_z_asym.key();
    m_mDecor_weighted_z_kurt = baseName + "." + m_mDecor_weighted_z_kurt.key();
    m_mDecor_z_skew = baseName + "." + m_mDecor_z_skew.key();
    m_mDecor_photon_deltaz = baseName + "." + m_mDecor_photon_deltaz.key();
    m_mDecor_photon_deltaPhi = baseName + "." + m_mDecor_photon_deltaPhi.key();
    m_mDecor_actualInterPerXing = baseName + "." + m_mDecor_actualInterPerXing.key();

    ATH_CHECK(m_photonLinksKey.initialize());
    ATH_CHECK(m_jetLinksKey.initialize());
    ATH_CHECK(m_electronLinksKey.initialize());
    ATH_CHECK(m_muonLinksKey.initialize());
    ATH_CHECK(m_mDecor_ntrk.initialize());
    ATH_CHECK(m_mDecor_sumPt.initialize());
    ATH_CHECK(m_mDecor_chi2Over_ndf.initialize());
    ATH_CHECK(m_mDecor_z_asym.initialize());
    ATH_CHECK(m_mDecor_weighted_z_asym.initialize());
    ATH_CHECK(m_mDecor_weighted_z_kurt.initialize());
    ATH_CHECK(m_mDecor_z_skew.initialize());
    ATH_CHECK(m_mDecor_photon_deltaz.initialize());
    ATH_CHECK(m_mDecor_photon_deltaPhi.initialize());
    ATH_CHECK(m_mDecor_actualInterPerXing.initialize());

    // ReadHandleKeys
    m_deltaZKey = baseName + ".deltaZ";
    m_deltaPhiKey = baseName + ".deltaPhi";
    ATH_CHECK(m_deltaZKey.initialize());
    ATH_CHECK(m_deltaPhiKey.initialize());

    // additional ReadHandleKeys to declare dependencies to the scheduler
    std::string photonBaseName = m_photonsInKey.key();

    m_caloPointingZKey = photonBaseName + ".caloPointingZ";
    m_zCommonKey = photonBaseName + ".zCommon";
    m_zCommonErrorKey = photonBaseName + ".zCommonError";

    ATH_CHECK(m_caloPointingZKey.initialize());
    ATH_CHECK(m_zCommonKey.initialize());
    ATH_CHECK(m_zCommonErrorKey.initialize());

    // Tools
    ATH_CHECK(m_gnnTool.retrieve());
    ATH_CHECK(m_trkVtxAssociationTool->setProperty("WorkingPoint","Prompt_MaxWeight"));
    ATH_CHECK(m_trkVtxAssociationTool->setProperty("AMVFVerticesDeco","TTVA_AMVFVertices_forReco"));
    ATH_CHECK(m_trkVtxAssociationTool->setProperty("AMVFWeightsDeco","TTVA_AMVFWeights_forReco"));
    ATH_CHECK(m_trkVtxAssociationTool.retrieve());
    ATH_CHECK(m_trkVtxAssociationTool->initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode VertexDecoratorAlg ::execute() 
  {
    const EventContext &ctx = Gaudi::Hive::currentContext();
    SG::ReadHandle<xAOD::VertexContainer> vertices(m_vertexInKey, ctx);
    ATH_CHECK(vertices.isValid());

    SG::ReadHandle<xAOD::PhotonContainer> photonsIn(m_photonsInKey, ctx);
    ATH_CHECK(photonsIn.isValid());
    SG::ReadHandle<xAOD::ElectronContainer> electronsIn(m_electronsInKey, ctx);
    ATH_CHECK(electronsIn.isValid());
    SG::ReadHandle<xAOD::MuonContainer> muonsIn(m_muonsInKey, ctx);
    ATH_CHECK(muonsIn.isValid());
    SG::ReadHandle<xAOD::JetContainer> jetsIn(m_jetsInKey, ctx);
    ATH_CHECK(jetsIn.isValid());
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInKey,ctx);
    ATH_CHECK(eventInfo.isValid());

    SG::WriteDecorHandle<xAOD::VertexContainer, std::vector<ElementLink<xAOD::PhotonContainer>>> dec_photonLinks(m_photonLinksKey, ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, std::vector<ElementLink<xAOD::JetContainer>>> dec_jetLinks(m_jetLinksKey, ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, std::vector<ElementLink<xAOD::ElectronContainer>>> dec_electronLinks(m_electronLinksKey, ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, std::vector<ElementLink<xAOD::MuonContainer>>> dec_muonLinks(m_muonLinksKey, ctx);

    SG::ReadDecorHandle<xAOD::VertexContainer, float> acc_deltaZ(m_deltaZKey, ctx);
    SG::ReadDecorHandle<xAOD::VertexContainer, float> acc_deltaPhi(m_deltaPhiKey, ctx);

    // Decorations needed by the GNNTool
    SG::WriteDecorHandle<xAOD::VertexContainer, int> dec_ntrk(m_mDecor_ntrk, ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, float> dec_sumPt(m_mDecor_sumPt,ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, float> dec_chi2Over_ndf(m_mDecor_chi2Over_ndf,ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, float> dec_z_asym(m_mDecor_z_asym,ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, float> dec_weighted_z_asym(m_mDecor_weighted_z_asym,ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, float> dec_z_kurt(m_mDecor_weighted_z_kurt,ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, float> dec_z_skew(m_mDecor_z_skew,ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, float> dec_photon_deltaz(m_mDecor_photon_deltaz,ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, float> dec_photon_deltaPhi(m_mDecor_photon_deltaPhi,ctx);
    SG::WriteDecorHandle<xAOD::VertexContainer, float> dec_actualInterPerXing(m_mDecor_actualInterPerXing,ctx);

    std::map< const xAOD::Vertex*, std::vector<const xAOD::Jet*> > jetsInVertex;
    std::map< const xAOD::Jet*, std::map< const xAOD::Vertex*, int> > jetVertexPt;

    for (const xAOD::Vertex *vertex : *vertices)
    {
      if (vertex->vertexType() == xAOD::VxType::NoVtx)
        continue;

      jetsInVertex[vertex] = {};
      for(const xAOD::Jet *jet : *jetsIn){
        jetVertexPt[jet][vertex] = 0;
      }

      dec_actualInterPerXing(*vertex) = eventInfo->actualInteractionsPerCrossing();

      // variables for calculation of delta Z asymmetry and delta d asymmetry
      float z_asym = 0;
      float sumDZ = 0;
      float deltaZ = 0;
      float modsumDZ = 0;
      float weighted_sumDZ = 0;
      float weighted_deltaZ = 0;
      float weighted_modsumDZ = 0;
      float weighted_z_asym = 0;

      // make vector
      std::vector<float> track_deltaZ;

      for (size_t i = 0; i < vertex->nTrackParticles(); i++) {

        const xAOD::TrackParticle *trackTmp = vertex->trackParticle(i);

        if(!trackTmp) continue;

        deltaZ = trackTmp->z0() + trackTmp->vz() - vertex->z();
        track_deltaZ.push_back(deltaZ);
        // get the track weight for each track to get the deltaZ/trk_weight
        float trk_weight = vertex->trackWeight(i);
        weighted_deltaZ = deltaZ * trk_weight;
        // sum of delta z
        sumDZ += deltaZ;
        modsumDZ += std::abs(deltaZ);
        weighted_sumDZ += weighted_deltaZ;
        weighted_modsumDZ += std::abs(weighted_deltaZ);
      } // end loop over tracks

      if (modsumDZ > 0) {
        z_asym = sumDZ / modsumDZ;
      }
      if (weighted_modsumDZ > 0) {
        weighted_z_asym = weighted_sumDZ / weighted_modsumDZ;
      }

      float mean_Dz = sumDZ / track_deltaZ.size(); // calculate average
      float number_tracks = track_deltaZ.size(); // get number of tracks

      float z_skew = 0; // skewness of DeltaZ asymmetry
      float z_kurt = 0; // Kurtosis of DeltaZ asymmetry
      float z_var = 0;  // variance of DeltaZ

      for (auto i : track_deltaZ)
      {
        float z_zbar = (i - mean_Dz);
        z_var += std::pow(z_zbar, 2);
        z_skew += std::pow(z_zbar, 3);
        z_kurt += std::pow(z_zbar, 4);
      }
      if (number_tracks > 1 && z_var > 0) {
        z_var /= (number_tracks - 1);
        float z_sd = std::sqrt(z_var);
        z_skew /= (number_tracks - 1) * std::pow(z_sd, 3);
        z_kurt /= (number_tracks - 1) * std::pow(z_sd, 4);
      }
      else
      {
        ATH_MSG_WARNING("z momenta are NaN: setting to zero");
        z_skew = 0.;
        z_kurt = 0.;
      }

      dec_ntrk(*vertex) = number_tracks;

      if(!dec_sumPt.isAvailable()){
        dec_sumPt(*vertex) = xAOD::PVHelpers::getVertexSumPt(vertex, 1, false);
      }
      dec_chi2Over_ndf(*vertex) = vertex->chiSquared() / vertex->numberDoF();
      dec_z_asym(*vertex) = z_asym;
      dec_weighted_z_asym(*vertex) = weighted_z_asym;
      dec_z_kurt(*vertex) = z_kurt;
      dec_z_skew(*vertex) = z_skew;

      if (acc_deltaZ.isAvailable()) {
        //protect against rare NaNs before assigning decorator: setting to 0 (-999 cause NaNs)
        if (std::isnan(acc_deltaZ(*vertex))) {
          ATH_MSG_WARNING("photon deltaPhi is NaN: setting to 0!");
          dec_photon_deltaz(*vertex) = 0;
        }
        else{
        dec_photon_deltaz(*vertex) = acc_deltaZ(*vertex);
        }
      }
      else{
       dec_photon_deltaz(*vertex) = 0;
      }
      if (acc_deltaPhi.isAvailable()) {
        if (std::isnan(acc_deltaPhi(*vertex))) {
          ATH_MSG_WARNING("photon deltaPhi is NaN: setting to 0!");
          dec_photon_deltaPhi(*vertex) = 0;
        }
        else{
        dec_photon_deltaPhi(*vertex) = acc_deltaPhi(*vertex);
        }
      }
      else{
       dec_photon_deltaPhi(*vertex) = 0;
      }

      // associate objects to vertices
      std::vector<ElementLink<xAOD::ElectronContainer>> electronLinks;
      for(const xAOD::Electron* electron : *electronsIn){
        const auto *id_trk = xAOD::EgammaHelpers::getOriginalTrackParticle(electron);
        if(!id_trk) continue;
        auto eleVertex = m_trkVtxAssociationTool->getUniqueMatchVertexLink(*id_trk, *vertices);
        if(!eleVertex) continue;
        ElementLink<xAOD::ElectronContainer> elLink;
        if(*eleVertex == vertex){
            elLink.setElement(electron);
            elLink.setStorableObject(*electronsIn.ptr(), true);
            electronLinks.push_back(elLink);
        }

      }
      dec_electronLinks(*vertex) = electronLinks;

      std::vector<ElementLink<xAOD::PhotonContainer>> photonLinks;
      for(const xAOD::Photon* photon : *photonsIn){
        ElementLink<xAOD::PhotonContainer> phLink;
        phLink.setElement(photon);
        phLink.setStorableObject(*photonsIn.ptr(), true);
        photonLinks.push_back(phLink);
      }
      dec_photonLinks(*vertex) = photonLinks;

      float maxPtFrac = -1;
      const xAOD::Jet* uniqueJetAddress = nullptr;

      std::vector<ElementLink<xAOD::JetContainer>> jetLinks;
      for(const xAOD::Jet* jet : *jetsIn){
      
        std::vector<const xAOD::TrackParticle*> ghostTracks = jet->getAssociatedObjects<xAOD::TrackParticle >(xAOD::JetAttribute::GhostTrack);

        for(const xAOD::TrackParticle* jtrk : ghostTracks){
          if( !jtrk ) continue;
          auto jetTrackVertex = m_trkVtxAssociationTool->getUniqueMatchVertexLink(*jtrk, *vertices);
          if(jetTrackVertex) jetVertexPt[jet][*jetTrackVertex] += jtrk->pt();
        }
        if(jetVertexPt[jet][vertex] > maxPtFrac){
          maxPtFrac = jetVertexPt[jet][vertex];
          uniqueJetAddress = jet;
        }
      }

      for(const xAOD::Jet* jet : *jetsIn){
        if(uniqueJetAddress == jet){
          ElementLink<xAOD::JetContainer> jetLink;
          jetLink.setElement(jet);
          jetLink.setStorableObject(*jetsIn.ptr(), true);
          jetLinks.push_back(jetLink);
          break;
        }
      }
      dec_jetLinks(*vertex) = jetLinks;

      std::vector<ElementLink<xAOD::MuonContainer>> muonLinks;
      for(const xAOD::Muon* muon : *muonsIn){
        auto tp = muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
        if(!tp) continue;
        try{
          auto muonVertex = m_trkVtxAssociationTool->getUniqueMatchVertexLink(*tp, *vertices);
          if(!muonVertex) continue;
          ElementLink<xAOD::MuonContainer> muonLink;
          if(*muonVertex == vertex){
            muonLink.setElement(muon);
            muonLink.setStorableObject(*muonsIn.ptr(), true);
            muonLinks.push_back(muonLink);
          }
        }catch(...) {
          ATH_MSG_DEBUG("Skipping muon as the track is not associated to any PV ");
          ATH_MSG_DEBUG("Muon pT, eta = " << muon->pt() << " " << muon->eta());
        }

      }
      dec_muonLinks(*vertex) = muonLinks;

      // Finally, decorate the vertices with the GNN score
      m_gnnTool->decorate(*vertex);
    }

    return StatusCode::SUCCESS;
  }

} // namespace InDetGNNHardScatterSelection
