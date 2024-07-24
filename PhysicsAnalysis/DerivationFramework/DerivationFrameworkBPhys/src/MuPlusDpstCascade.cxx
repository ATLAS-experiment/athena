/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/////////////////////////////////////////////////////////////////
// MuPlusDpstCascade.cxx, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
#include "DerivationFrameworkBPhys/MuPlusDpstCascade.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "DerivationFrameworkBPhys/CascadeTools.h"
#include "DerivationFrameworkBPhys/BPhysPVCascadeTools.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTracking/Vertex.h"
#include "InDetBeamSpotService/IBeamCondSvc.h"
#include "xAODBPhys/BPhysHypoHelper.h"
#include <algorithm>
#include "xAODTracking/VertexContainer.h"
#include "DerivationFrameworkBPhys/LocalVector.h"
#include "HepPDT/ParticleDataTable.hh"
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"

namespace DerivationFramework {
    typedef ElementLink<xAOD::VertexContainer> VertexLink;
    typedef std::vector<VertexLink> VertexLinkVector;
    typedef std::vector<const xAOD::TrackParticle*> TrackBag;
    typedef ElementLink<xAOD::TrackParticleContainer> TrackLink;
    typedef std::vector<TrackLink> TrackLinkVector;


    StatusCode MuPlusDpstCascade::initialize() {
        // retrieving vertex Fitter
        ATH_CHECK( m_iVertexFitter.retrieve());
        ATH_CHECK( m_iVertexFitter2.retrieve());

        // retrieving the V0 tools
        ATH_CHECK( m_V0Tools.retrieve());

        // retrieving the Cascade tools
        ATH_CHECK( m_CascadeTools.retrieve());
        ATH_CHECK( m_CascadeToolsAdd.retrieve());

        // Get the beam spot service
        ATH_CHECK( m_beamSpotSvc.retrieve() );

        IPartPropSvc* partPropSvc = nullptr;
        ATH_CHECK( service("PartPropSvc", partPropSvc, true) );
        auto pdt = partPropSvc->PDT();

        // retrieve particle masses
        if(m_vtx0MassHypo < 0.) m_vtx0MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::B_c_plus);
        if(m_vtx1MassHypo < 0.) m_vtx1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::D0);

        if(m_vtx0Daug1MassHypo < 0.) m_vtx0Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::mu_minus);
        if(m_vtx0Daug2MassHypo < 0.) m_vtx0Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
        if(m_vtx1Daug1MassHypo < 0.) m_vtx1Daug1MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::pi_plus);
        if(m_vtx1Daug2MassHypo < 0.) m_vtx1Daug2MassHypo = BPhysPVCascadeTools::getParticleMass(pdt, PDG::K_plus);

        //======================== inDetTrack selection tool ==================
        m_trackSelectionTools = std::make_unique<InDet::InDetTrackSelectionTool>("TrackSelector");
        ANA_CHECK(m_trackSelectionTools->setProperty("CutLevel", "LoosePrimary"));
        ANA_CHECK(m_trackSelectionTools->initialize() );
        //=====================================================================

        return StatusCode::SUCCESS;
    }


    StatusCode MuPlusDpstCascade::addBranches() const
    {

    std::vector<Trk::VxCascadeInfo*> cascadeinfoContainer;
        std::vector<std::vector<Trk::VxCascadeInfo*>> cascadeinfoContainer2; //several additional cascade fits with tracks for each B candidate
        std::vector<bool> chiPR;
      constexpr int topoN = 2;
      std::array<xAOD::VertexContainer*, topoN> Vtxwritehandles;
      std::array<xAOD::VertexAuxContainer*, topoN> Vtxwritehandlesaux;
      if(m_cascadeOutputsKeys.size() !=topoN)  { ATH_MSG_ERROR("Incorrect number of VtxContainers"); return StatusCode::FAILURE; }
        
        std::array<xAOD::VertexContainer*, topoN> VtxwritehandlesAdd;
        std::array<xAOD::VertexAuxContainer*, topoN> VtxwritehandlesauxAdd;

        if(m_cascadeOutputsKeysAdd.size() !=topoN)  { ATH_MSG_ERROR("Incorrect number of VtxContainers for additional cascade"); return StatusCode::FAILURE; }

      for(int i =0; i<topoN;i++){
         Vtxwritehandles[i] = new xAOD::VertexContainer();
         Vtxwritehandlesaux[i] = new xAOD::VertexAuxContainer();
         Vtxwritehandles[i]->setStore(Vtxwritehandlesaux[i]);
         ATH_CHECK(evtStore()->record(Vtxwritehandles[i]   , m_cascadeOutputsKeys[i]       ));
         ATH_CHECK(evtStore()->record(Vtxwritehandlesaux[i], m_cascadeOutputsKeys[i] + "Aux."));
          
          VtxwritehandlesAdd[i] = new xAOD::VertexContainer();
          VtxwritehandlesauxAdd[i] = new xAOD::VertexAuxContainer();
          VtxwritehandlesAdd[i]->setStore(VtxwritehandlesauxAdd[i]);
          ATH_CHECK(evtStore()->record(VtxwritehandlesAdd[i]   , m_cascadeOutputsKeysAdd[i]       ));
          ATH_CHECK(evtStore()->record(VtxwritehandlesauxAdd[i], m_cascadeOutputsKeysAdd[i] + "Aux."));
      }
        
      //----------------------------------------------------
      // retrieve primary vertices
      //----------------------------------------------------
      const xAOD::Vertex * primaryVertex(nullptr);
      const xAOD::VertexContainer *pvContainer(nullptr);
      ATH_CHECK(evtStore()->retrieve(pvContainer, m_VxPrimaryCandidateName));
      ATH_MSG_DEBUG("Found " << m_VxPrimaryCandidateName << " in StoreGate!");

      if (pvContainer->size()==0){
        ATH_MSG_WARNING("You have no primary vertices: " << pvContainer->size());
        return StatusCode::RECOVERABLE;
      } else {
        primaryVertex = (*pvContainer)[0];
      }

      //----------------------------------------------------
      // Try to retrieve refitted primary vertices
      //----------------------------------------------------
      xAOD::VertexContainer*    refPvContainer    = nullptr;
      xAOD::VertexAuxContainer* refPvAuxContainer = nullptr;
      if (m_refitPV) {
        if (evtStore()->contains<xAOD::VertexContainer>(m_refPVContainerName)) {
          // refitted PV container exists. Get it from the store gate
          ATH_CHECK(evtStore()->retrieve(refPvContainer   , m_refPVContainerName       ));
          ATH_CHECK(evtStore()->retrieve(refPvAuxContainer, m_refPVContainerName + "Aux."));
        } else {
          // refitted PV container does not exist. Create a new one.
          refPvContainer = new xAOD::VertexContainer;
          refPvAuxContainer = new xAOD::VertexAuxContainer;
          refPvContainer->setStore(refPvAuxContainer);
          ATH_CHECK(evtStore()->record(refPvContainer   , m_refPVContainerName));
          ATH_CHECK(evtStore()->record(refPvAuxContainer, m_refPVContainerName+"Aux."));
        }
      }

      ATH_CHECK(performSearch(&cascadeinfoContainer,&cascadeinfoContainer2));
      BPhysPVCascadeTools helper(&(*m_CascadeTools), &m_beamSpotSvc);
      helper.SetMinNTracksInPV(m_PV_minNTracks);
      // Decorators for the main vertex: chi2, ndf, pt and pt error, plus the D0 vertex variables
      SG::AuxElement::Decorator<VertexLinkVector> CascadeLinksDecor("CascadeVertexLinks");
      SG::AuxElement::Decorator<VertexLinkVector> CascadeAddLinksDecor("CascadeVertexAddLinks");
      SG::AuxElement::Decorator<VertexLinkVector> MuPiLinksDecor("MuPiVertexLinks");
      SG::AuxElement::Decorator<VertexLinkVector> D0LinksDecor("D0VertexLinks");
      SG::AuxElement::Decorator<VertexLinkVector> MuPiTrkLinksDecor("MuPiTrkVertexLinks");
      SG::AuxElement::Decorator<VertexLinkVector> D0AddLinksDecor("D0AddVertexLinks");
      SG::AuxElement::Decorator<TrackLinkVector> TrackLinksDecor("TrackLinks");
      SG::AuxElement::Decorator<float> chi2_decor("ChiSquared");
      SG::AuxElement::Decorator<float> ndof_decor("NumberDoF");
      SG::AuxElement::Decorator<float> Pt_decor("Pt");
      SG::AuxElement::Decorator<float> PtErr_decor("PtErr");
      SG::AuxElement::Decorator<float> Mass_svdecor("D0_mass");
      SG::AuxElement::Decorator<float> MassErr_svdecor("D0_massErr");
      SG::AuxElement::Decorator<float> Pt_svdecor("D0_Pt");
      SG::AuxElement::Decorator<float> PtErr_svdecor("D0_PtErr");
      SG::AuxElement::Decorator<float> Lxy_svdecor("D0_Lxy");
      SG::AuxElement::Decorator<float> LxyErr_svdecor("D0_LxyErr");
      SG::AuxElement::Decorator<float> Tau_svdecor("D0_Tau");
      SG::AuxElement::Decorator<float> TauErr_svdecor("D0_TauErr");
        SG::AuxElement::Decorator<float> PtAdd_decor("PtAdd");
        SG::AuxElement::Decorator<float> PtAddErr_decor("PtAddErr");
        SG::AuxElement::Decorator<float> D0AddMass_svdecor("D0Add_mass");
        SG::AuxElement::Decorator<float> D0AddMassErr_svdecor("D0Add_massErr");
        SG::AuxElement::Decorator<float> D0AddPt_svdecor("D0Add_Pt");
        SG::AuxElement::Decorator<float> D0AddPtErr_svdecor("D0Add_PtErr");
        SG::AuxElement::Decorator<float> D0AddLxy_svdecor("D0Add_Lxy");
        SG::AuxElement::Decorator<float> D0AddLxyErr_svdecor("D0Add_LxyErr");
        SG::AuxElement::Decorator<float> D0AddTau_svdecor("D0Add_Tau");
        SG::AuxElement::Decorator<float> D0AddTauErr_svdecor("D0Add_TauErr");


      SG::AuxElement::Decorator<float> massMuPi_decor("MuPi_mass"); //mu+pi_soft mass before fit
      SG::AuxElement::Decorator<float> MassKpi_svdecor("Kpi_mass");
      SG::AuxElement::Decorator<float> MassMuPiAft_decor("MuPiAft_mass"); //mu+pi_soft mass after fit
      SG::AuxElement::Decorator<float> MassPiD0_decor("PiD0_mass");
      SG::AuxElement::Decorator<float> MassMuPiPiK_decor("MuPiPiK_mass");

      SG::AuxElement::Decorator<float> ChargePi_decor("Pi_charge");
      SG::AuxElement::Decorator<float> ChargeMu_decor("Mu_charge");
      SG::AuxElement::Decorator<float> ChargeK_decor("K_charge");
      SG::AuxElement::Decorator<float> ChargePi_s_decor("Pi_s_charge");
      SG::AuxElement::Decorator<float> Chi2Mu_decor("Mu_chi2");
      SG::AuxElement::Decorator<float> nDoFMu_decor("Mu_nDoF");
      //muon contribution to chi2 of the cascade fit
      SG::AuxElement::Decorator<float> MuChi2B_decor("Mu_chi2_B");
      SG::AuxElement::Decorator<float> MunDoFB_decor("Mu_nDoF_B");
        //chi2 and ndof for additional cascade woth track
        SG::AuxElement::Decorator<float> Chi2Add_decor("Chi2Add");
        SG::AuxElement::Decorator<float> ndofAdd_decor("ndofAdd");
        SG::AuxElement::Decorator<std::vector<int>> numOfVtx_decor("numOfVtx");

      // Obtaining containers for setting links to the original tracks
      // Get mu+pi container
      const xAOD::VertexContainer  *MuPiContainer(nullptr);
      ATH_CHECK(evtStore()->retrieve(MuPiContainer   , m_vertexContainerKey       ));
      // Get D0 container
      const xAOD::VertexContainer  *d0Container(nullptr);
      ATH_CHECK(evtStore()->retrieve(d0Container   , m_vertexD0ContainerKey       )); //"D0Vertices"
      // Get TrackParticle container
      const xAOD::TrackParticleContainer  *trackContainer(nullptr);
      ATH_CHECK(evtStore()->retrieve(trackContainer   , "InDetTrackParticles"      ));

      for (Trk::VxCascadeInfo* x : cascadeinfoContainer) {
        if(x==nullptr) ATH_MSG_ERROR("cascadeinfoContainer is null");

        // the cascade fitter returns:
        // std::vector<xAOD::Vertex*>, each xAOD::Vertex contains the refitted track parameters (perigee at the vertex position)
        //   vertices[iv]              the links to the original TPs and a covariance of size 3+5*NTRK; the chi2 of the total fit
        //                             is split between the cascade vertices as per track contribution
        // std::vector< std::vector<TLorentzVector> >, each std::vector<TLorentzVector> contains the refitted momenta (TLorentzVector)
        //   momenta[iv][...]          of all tracks in the corresponding vertex, including any pseudotracks (from cascade vertices)
        //                             originating in this vertex; the masses are as assigned in the cascade fit
        // std::vector<Amg::MatrixX>,  the corresponding covariance matrices in momentum space
        //   covariance[iv]
        // int nDoF, double Chi2
        //
        // the invariant mass, pt, lifetime etc. errors should be calculated using the covariance matrices in momentum space as these
        // take into account the full track-track and track-vertex correlations
        //
        // in the case of Jpsi+V0: vertices[0] is the V0 vertex, vertices[1] is the B/Lambda_b(bar) vertex, containing the 2 Jpsi tracks.
        // The covariance terms between the two vertices are not stored. In momentum space momenta[0] contains the 2 V0 tracks,
        // their momenta add up to the momentum of the 3rd track in momenta[1], the first two being the Jpsi tracks

        const std::vector<xAOD::Vertex*> &cascadeVertices = x->vertices();
        if(cascadeVertices.size()!=topoN)
          ATH_MSG_ERROR("Incorrect number of vertices");
        if(cascadeVertices[0] == nullptr || cascadeVertices[1] == nullptr) ATH_MSG_ERROR("Error null vertex");
        // Keep vertices (bear in mind that they come in reverse order!)
        for(int i =0;i<topoN;i++) Vtxwritehandles[i]->push_back(cascadeVertices[i]);

        x->getSVOwnership(false); // Prevent Container from deleting vertices
        const auto mainVertex = cascadeVertices[1];   // this is the B_c+/- vertex
        const std::vector< std::vector<TLorentzVector> > &moms = x->getParticleMoms();
          
        // Set links to cascade vertices
        std::vector<xAOD::Vertex*> verticestoLink;
        verticestoLink.push_back(cascadeVertices[0]);
        if(Vtxwritehandles[1] == nullptr) ATH_MSG_ERROR("Vtxwritehandles[1] is null");
        if(!BPhysPVCascadeTools::LinkVertices(CascadeLinksDecor, verticestoLink, Vtxwritehandles[0], cascadeVertices[1]))
            ATH_MSG_ERROR("Error decorating with cascade vertices");

        // Identify the input mu+pi_soft
        xAOD::Vertex* MuPiVertex = BPhysPVCascadeTools::FindVertex<2>(MuPiContainer, cascadeVertices[1]);
        ATH_MSG_DEBUG("1 pt mu+pi_soft tracks " << cascadeVertices[1]->trackParticle(0)->pt() << ", " << cascadeVertices[1]->trackParticle(1)->pt());
        if (MuPiVertex) ATH_MSG_DEBUG("2 pt mu+pi_soft tracks " << MuPiVertex->trackParticle(0)->pt() << ", " << MuPiVertex->trackParticle(1)->pt());
          
        // Identify the input D0
        xAOD::Vertex* d0Vertex = BPhysPVCascadeTools::FindVertex<2>(d0Container, cascadeVertices[0]);;
        ATH_MSG_DEBUG("1 pt D0 tracks " << cascadeVertices[0]->trackParticle(0)->pt() << ", " << cascadeVertices[0]->trackParticle(1)->pt());
        if (d0Vertex) ATH_MSG_DEBUG("2 pt D0 tracks " << d0Vertex->trackParticle(0)->pt() << ", " << d0Vertex->trackParticle(1)->pt());

        // Set links to input vertices
        std::vector<xAOD::Vertex*> MuPiVerticestoLink;
        if (MuPiVertex) MuPiVerticestoLink.push_back(MuPiVertex);
        else ATH_MSG_WARNING("Could not find linking mu+pi_soft");
        if(!BPhysPVCascadeTools::LinkVertices(MuPiLinksDecor, MuPiVerticestoLink, MuPiContainer, cascadeVertices[1]))
            ATH_MSG_ERROR("Error decorating with mu+pi_soft vertices");

        std::vector<xAOD::Vertex*> d0VerticestoLink;
        if (d0Vertex) d0VerticestoLink.push_back(d0Vertex);
        else ATH_MSG_WARNING("Could not find linking D0");
        if(!BPhysPVCascadeTools::LinkVertices(D0LinksDecor, d0VerticestoLink, d0Container, cascadeVertices[1]))
            ATH_MSG_ERROR("Error decorating with D0 vertices");

        bool tagD0(true);
        if (MuPiVertex){
          if(std::abs(m_Dx_pid)==421 && (MuPiVertex->trackParticle(1)->charge()==-1)) tagD0 = false; //checking soft pion charge
        }
          
          //--------- mass hypo for main fit of B vertex
          double mass_b = m_vtx0MassHypo;
          double mass_d0 = m_vtx1MassHypo;
          std::vector<double> massesMuPi;
          massesMuPi.push_back(m_vtx0Daug1MassHypo); //mu
          massesMuPi.push_back(m_vtx0Daug2MassHypo); //pi_soft
          std::vector<double> massesD0;
          if(tagD0){
              massesD0.push_back(m_vtx1Daug1MassHypo); //pi
              massesD0.push_back(m_vtx1Daug2MassHypo); //K
          }else{ // Change the order of masses for D*-->D0bar pi-, D0bar->K+pi-
              massesD0.push_back(m_vtx1Daug2MassHypo); //K
              massesD0.push_back(m_vtx1Daug1MassHypo); //pi
          }
          std::vector<double> Masses; // masses of all particles "inside" of B
          Masses.push_back(m_vtx0Daug1MassHypo); //mu
          Masses.push_back(m_vtx0Daug2MassHypo); //pi_soft
          Masses.push_back(m_vtx1MassHypo); //D0 mass

          //--------- mass hypo for additional fit of B vertex with track
          std::vector<double> initialCVertexMass; // masses of all particles "inside" of B before additional fit
          initialCVertexMass.push_back(m_vtx0Daug1MassHypo); //mu
          initialCVertexMass.push_back(m_vtx0Daug2MassHypo); //pi_soft
          initialCVertexMass.push_back(massesD0.at(0)); //D0's track1 mass
          initialCVertexMass.push_back(massesD0.at(1)); //D0's track2 mass
          
          std::vector<double> secondCVertexMass; // masses of all particles фаеук additional fit
          secondCVertexMass.push_back(m_vtx0Daug1MassHypo); //mu
          secondCVertexMass.push_back(m_vtx0Daug2MassHypo); //pi_soft
          secondCVertexMass.push_back(massesD0.at(0)); //D0's track1 mass
          secondCVertexMass.push_back(massesD0.at(1)); //D0's track2 mass
          secondCVertexMass.push_back(m_vtx1Daug1MassHypo); //pi (for now, might add K)
          //---------

        // reset beamspot cache
        helper.GetBeamSpot(true);
        // loop over candidates -- Don't apply PV_minNTracks requirement here
        // because it may result in exclusion of the high-pt PV.
        // get good PVs

        xAOD::BPhysHypoHelper vtx(m_hypoName, mainVertex);

        // Get refitted track momenta from all vertices, charged tracks only
        BPhysPVCascadeTools::SetVectorInfo(vtx, x);
          

        // Decorate main vertex
        //
        // 1.a) mass, mass error
        BPHYS_CHECK( vtx.setMass(m_CascadeTools->invariantMass(moms[1])) );
        BPHYS_CHECK( vtx.setMassErr(m_CascadeTools->invariantMassError(moms[1],x->getCovariance()[1])) );
        // 1.b) pt and pT error (the default pt of mainVertex is != the pt of the full cascade fit!)
        Pt_decor(*mainVertex) = m_CascadeTools->pT(moms[1]);
        PtErr_decor(*mainVertex) = m_CascadeTools->pTError(moms[1],x->getCovariance()[1]);
        // 1.c) chi2 and ndof (the default chi2 of mainVertex is != the chi2 of the full cascade fit!)
        chi2_decor(*mainVertex) = x->fitChi2();
        ndof_decor(*mainVertex) = x->nDoF();
        Chi2Mu_decor(*mainVertex) = cascadeVertices[1]->trackParticle(0)->chiSquared();
        nDoFMu_decor(*mainVertex) = cascadeVertices[1]->trackParticle(0)->numberDoF();
          
        // track contribution to chi2 of cascasde fit
        std::vector< Trk::VxTrackAtVertex > trkAtB = cascadeVertices[1]->vxTrackAtVertex();
        MuChi2B_decor(*mainVertex) = trkAtB.at(0).trackQuality().chiSquared();
        MunDoFB_decor(*mainVertex) = trkAtB.at(0).trackQuality().numberDoF();

        float massMuPi = 0.;
        if (MuPiVertex) {
          TLorentzVector  p4_mu1, p4_pi_s;
          p4_mu1.SetPtEtaPhiM(MuPiVertex->trackParticle(0)->pt(),
                              MuPiVertex->trackParticle(0)->eta(),
                              MuPiVertex->trackParticle(0)->phi(), m_vtx0Daug1MassHypo); //mu
          p4_pi_s.SetPtEtaPhiM(MuPiVertex->trackParticle(1)->pt(),
                              MuPiVertex->trackParticle(1)->eta(),
                              MuPiVertex->trackParticle(1)->phi(), m_vtx0Daug2MassHypo);  // pi_soft
          massMuPi = (p4_mu1 + p4_pi_s).M();
            ChargePi_s_decor(*mainVertex) = MuPiVertex->trackParticle(1)->charge();
            ChargeMu_decor(*mainVertex) = MuPiVertex->trackParticle(0)->charge();
        }
        massMuPi_decor(*mainVertex) = massMuPi;

        //float massKpi = 0.;
        if (d0Vertex) {
          TLorentzVector  p4_ka, p4_pi;
          if(tagD0){
            p4_pi.SetPtEtaPhiM(d0Vertex->trackParticle(0)->pt(), 
                               d0Vertex->trackParticle(0)->eta(),
                               d0Vertex->trackParticle(0)->phi(), m_vtx1Daug1MassHypo); 
            p4_ka.SetPtEtaPhiM(d0Vertex->trackParticle(1)->pt(), 
                               d0Vertex->trackParticle(1)->eta(),
                               d0Vertex->trackParticle(1)->phi(), m_vtx1Daug2MassHypo);
              ChargeK_decor(*mainVertex) = d0Vertex->trackParticle(1)->charge();
              ChargePi_decor(*mainVertex) = d0Vertex->trackParticle(0)->charge();

          }else{ // Change the oreder of masses for D*-->D0bar pi-, D0bar->K+pi-
            p4_pi.SetPtEtaPhiM(d0Vertex->trackParticle(1)->pt(), 
                               d0Vertex->trackParticle(1)->eta(),
                               d0Vertex->trackParticle(1)->phi(), m_vtx1Daug1MassHypo); 
            p4_ka.SetPtEtaPhiM(d0Vertex->trackParticle(0)->pt(), 
                               d0Vertex->trackParticle(0)->eta(),
                               d0Vertex->trackParticle(0)->phi(), m_vtx1Daug2MassHypo);
              ChargeK_decor(*mainVertex) = d0Vertex->trackParticle(0)->charge();
              ChargePi_decor(*mainVertex) = d0Vertex->trackParticle(1)->charge();
          }
          //massKpi = (p4_ka + p4_pi).M();
        }
        MassMuPiAft_decor(*mainVertex) = (moms[1][0] + moms[1][1]).M(); //mu + pi_soft
        MassPiD0_decor(*mainVertex) = (moms[1][1] + moms[0][0] + moms[0][1]).M(); //pi_soft + D0
        MassKpi_svdecor(*mainVertex) = (moms[0][0] + moms[0][1]).M();//D0
        MassMuPiPiK_decor(*mainVertex) = m_CascadeTools->invariantMass(moms[1]); //B0
          
        ATH_CHECK(helper.FillCandwithRefittedVertices(m_refitPV, pvContainer, refPvContainer, &(*m_pvRefitter), m_PV_max, m_DoVertexType, x, 1, mass_b, vtx));

        // 4) decorate the main vertex with D0 vertex mass, pt, lifetime and lxy values (plus errors) 
        // D0 points to the main vertex, so lifetime and lxy are w.r.t the main vertex
        Mass_svdecor(*mainVertex) = m_CascadeTools->invariantMass(moms[0]);
        MassErr_svdecor(*mainVertex) = m_CascadeTools->invariantMassError(moms[0],x->getCovariance()[0]);
        Pt_svdecor(*mainVertex) = m_CascadeTools->pT(moms[0]);
        PtErr_svdecor(*mainVertex) = m_CascadeTools->pTError(moms[0],x->getCovariance()[0]);
        Lxy_svdecor(*mainVertex) = m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]);
        LxyErr_svdecor(*mainVertex) = m_CascadeTools->lxyError(moms[0],x->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
        Tau_svdecor(*mainVertex) = m_CascadeTools->tau(moms[0],cascadeVertices[0],cascadeVertices[1]);
        TauErr_svdecor(*mainVertex) = m_CascadeTools->tauError(moms[0],x->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);

          //--------------------------------------------------------------------------
          //saving number of additional cascade veteces for each candidate
          //vector of tsuch numbers for each one
          std::vector<int> count;
          for (std::vector<Trk::VxCascadeInfo*> xx : cascadeinfoContainer2) {
              count.push_back(xx.size());
          }
          numOfVtx_decor(*mainVertex) = count;
          //--------------------------------------------------------------------------

        // Some checks in DEBUG mode
        ATH_MSG_DEBUG("chi2 " << x->fitChi2() //DEBUG->INFO
                  << " chi2_1 " << m_V0Tools->chisq(cascadeVertices[0])
                  << " chi2_2 " << m_V0Tools->chisq(cascadeVertices[1])
                  << " vprob " << m_CascadeTools->vertexProbability(x->nDoF(),x->fitChi2()));
        ATH_MSG_DEBUG("ndf " << x->nDoF() << " ndf_1 " << m_V0Tools->ndof(cascadeVertices[0]) << " ndf_2 " << m_V0Tools->ndof(cascadeVertices[1]));
        ATH_MSG_DEBUG("V0Tools mass_d0 " << m_V0Tools->invariantMass(cascadeVertices[0],massesD0)
                   << " error " << m_V0Tools->invariantMassError(cascadeVertices[0],massesD0)
                   << " mass_J " << m_V0Tools->invariantMass(cascadeVertices[1],massesMuPi)
                   << " error " << m_V0Tools->invariantMassError(cascadeVertices[1],massesMuPi));
        // masses and errors, using track masses assigned in the fit
        double Mass_B = m_CascadeTools->invariantMass(moms[1]);
        double Mass_D0 = m_CascadeTools->invariantMass(moms[0]);
        double Mass_B_err = m_CascadeTools->invariantMassError(moms[1],x->getCovariance()[1]);
        double Mass_D0_err = m_CascadeTools->invariantMassError(moms[0],x->getCovariance()[0]);
        ATH_MSG_DEBUG("Mass_B " << Mass_B << " Mass_D0 " << Mass_D0);
        ATH_MSG_DEBUG("Mass_B_err " << Mass_B_err << " Mass_D0_err " << Mass_D0_err);
        double mprob_B = m_CascadeTools->massProbability(mass_b,Mass_B,Mass_B_err);
        double mprob_D0 = m_CascadeTools->massProbability(mass_d0,Mass_D0,Mass_D0_err);
        ATH_MSG_DEBUG("mprob_B " << mprob_B << " mprob_D0 " << mprob_D0);
        // masses and errors, assigning user defined track masses
        ATH_MSG_DEBUG("Mass_b " << m_CascadeTools->invariantMass(moms[1],Masses)
                  << " Mass_d0 " << m_CascadeTools->invariantMass(moms[0],massesD0));
        ATH_MSG_DEBUG("Mass_b_err " << m_CascadeTools->invariantMassError(moms[1],x->getCovariance()[1],Masses)
                  << " Mass_d0_err " << m_CascadeTools->invariantMassError(moms[0],x->getCovariance()[0],massesD0));
        ATH_MSG_DEBUG("pt_b " << m_CascadeTools->pT(moms[1])
                  << " pt_d " << m_CascadeTools->pT(moms[0])
                  << " pt_d0 " << m_V0Tools->pT(cascadeVertices[0]));
        ATH_MSG_DEBUG("ptErr_b " << m_CascadeTools->pTError(moms[1],x->getCovariance()[1])
                  << " ptErr_d " << m_CascadeTools->pTError(moms[0],x->getCovariance()[0])
                  << " ptErr_d0 " << m_V0Tools->pTError(cascadeVertices[0]));
        ATH_MSG_DEBUG("lxy_B " << m_V0Tools->lxy(cascadeVertices[1],primaryVertex) << " lxy_D " << m_V0Tools->lxy(cascadeVertices[0],cascadeVertices[1]));
        ATH_MSG_DEBUG("lxy_b " << m_CascadeTools->lxy(moms[1],cascadeVertices[1],primaryVertex) << " lxy_d " << m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]));
        ATH_MSG_DEBUG("lxyErr_b " << m_CascadeTools->lxyError(moms[1],x->getCovariance()[1],cascadeVertices[1],primaryVertex)
                  << " lxyErr_d " << m_CascadeTools->lxyError(moms[0],x->getCovariance()[0],cascadeVertices[0],cascadeVertices[1])
                  << " lxyErr_d0 " << m_V0Tools->lxyError(cascadeVertices[0],cascadeVertices[1]));
        ATH_MSG_DEBUG("tau_B " << m_CascadeTools->tau(moms[1],cascadeVertices[1],primaryVertex,mass_b)
                   << " tau_d0 " << m_V0Tools->tau(cascadeVertices[0],cascadeVertices[1],massesD0));
        ATH_MSG_DEBUG("tau_b " << m_CascadeTools->tau(moms[1],cascadeVertices[1],primaryVertex)
                   << " tau_d " << m_CascadeTools->tau(moms[0],cascadeVertices[0],cascadeVertices[1])
                   << " tau_D " << m_CascadeTools->tau(moms[0],cascadeVertices[0],cascadeVertices[1],mass_d0));
        ATH_MSG_DEBUG("tauErr_b " << m_CascadeTools->tauError(moms[1],x->getCovariance()[1],cascadeVertices[1],primaryVertex)
                  << " tauErr_d " << m_CascadeTools->tauError(moms[0],x->getCovariance()[0],cascadeVertices[0],cascadeVertices[1])
                  << " tauErr_d0 " << m_V0Tools->tauError(cascadeVertices[0],cascadeVertices[1],massesD0));
        ATH_MSG_DEBUG("TauErr_b " << m_CascadeTools->tauError(moms[1],x->getCovariance()[1],cascadeVertices[1],primaryVertex,mass_b)
                  << " TauErr_d " << m_CascadeTools->tauError(moms[0],x->getCovariance()[0],cascadeVertices[0],cascadeVertices[1],mass_d0)
                  << " TauErr_d0 " << m_V0Tools->tauError(cascadeVertices[0],cascadeVertices[1],massesD0,mass_d0));

        ATH_MSG_DEBUG("CascadeTools main vert wrt PV " << " CascadeTools SV " << " V0Tools SV");
        ATH_MSG_DEBUG("a0z " << m_CascadeTools->a0z(moms[1],cascadeVertices[1],primaryVertex)
                   << ", " << m_CascadeTools->a0z(moms[0],cascadeVertices[0],cascadeVertices[1])
                   << ", " << m_V0Tools->a0z(cascadeVertices[0],cascadeVertices[1]));
        ATH_MSG_DEBUG("a0zErr " << m_CascadeTools->a0zError(moms[1],x->getCovariance()[1],cascadeVertices[1],primaryVertex)
                   << ", " << m_CascadeTools->a0zError(moms[0],x->getCovariance()[0],cascadeVertices[0],cascadeVertices[1])
                   << ", " << m_V0Tools->a0zError(cascadeVertices[0],cascadeVertices[1]));
        ATH_MSG_DEBUG("a0xy " << m_CascadeTools->a0xy(moms[1],cascadeVertices[1],primaryVertex)
                   << ", " << m_CascadeTools->a0xy(moms[0],cascadeVertices[0],cascadeVertices[1])
                   << ", " << m_V0Tools->a0xy(cascadeVertices[0],cascadeVertices[1]));
        ATH_MSG_DEBUG("a0xyErr " << m_CascadeTools->a0xyError(moms[1],x->getCovariance()[1],cascadeVertices[1],primaryVertex)
                   << ", " << m_CascadeTools->a0xyError(moms[0],x->getCovariance()[0],cascadeVertices[0],cascadeVertices[1])
                   << ", " << m_V0Tools->a0xyError(cascadeVertices[0],cascadeVertices[1]));
        ATH_MSG_DEBUG("a0 " << m_CascadeTools->a0(moms[1],cascadeVertices[1],primaryVertex)
                   << ", " << m_CascadeTools->a0(moms[0],cascadeVertices[0],cascadeVertices[1])
                   << ", " << m_V0Tools->a0(cascadeVertices[0],cascadeVertices[1]));
        ATH_MSG_DEBUG("a0Err " << m_CascadeTools->a0Error(moms[1],x->getCovariance()[1],cascadeVertices[1],primaryVertex)
                   << ", " << m_CascadeTools->a0Error(moms[0],x->getCovariance()[0],cascadeVertices[0],cascadeVertices[1])
                   << ", " << m_V0Tools->a0Error(cascadeVertices[0],cascadeVertices[1]));
        ATH_MSG_DEBUG("x0 " << m_V0Tools->vtx(cascadeVertices[0]).x() << " y0 " << m_V0Tools->vtx(cascadeVertices[0]).y() << " z0 " << m_V0Tools->vtx(cascadeVertices[0]).z());
        ATH_MSG_DEBUG("x1 " << m_V0Tools->vtx(cascadeVertices[1]).x() << " y1 " << m_V0Tools->vtx(cascadeVertices[1]).y() << " z1 " << m_V0Tools->vtx(cascadeVertices[1]).z());
        ATH_MSG_DEBUG("X0 " << primaryVertex->x() << " Y0 " << primaryVertex->y() << " Z0 " << primaryVertex->z());
        ATH_MSG_DEBUG("rxy0 " << m_V0Tools->rxy(cascadeVertices[0]) << " rxyErr0 " << m_V0Tools->rxyError(cascadeVertices[0]));
        ATH_MSG_DEBUG("rxy1 " << m_V0Tools->rxy(cascadeVertices[1]) << " rxyErr1 " << m_V0Tools->rxyError(cascadeVertices[1]));
        ATH_MSG_DEBUG("Rxy0 wrt PV " << m_V0Tools->rxy(cascadeVertices[0],primaryVertex) << " RxyErr0 wrt PV " << m_V0Tools->rxyError(cascadeVertices[0],primaryVertex));
        ATH_MSG_DEBUG("Rxy1 wrt PV " << m_V0Tools->rxy(cascadeVertices[1],primaryVertex) << " RxyErr1 wrt PV " << m_V0Tools->rxyError(cascadeVertices[1],primaryVertex));
        ATH_MSG_DEBUG("number of covariance matrices " << (x->getCovariance()).size());
      } // loop over cascadeinfoContainer
        
      // Deleting cascadeinfo since this won't be stored.
      // Vertices have been kept in m_cascadeOutputs and should be owned by their container
      for (auto x : cascadeinfoContainer) delete x;

        //loop for the additional cascade fit
        //result is stored in double vector format -> double loop
        //several possible additional fits for each B candidate
        //but we write them all together
        //will distinguish them in NM
        std::vector<int> count2;
        for (std::vector<Trk::VxCascadeInfo*> xx : cascadeinfoContainer2) {
            count2.push_back(xx.size());
            for (auto x : xx){
                if(x==nullptr) {
                    ATH_MSG_DEBUG("cascadeinfoContainer is null");
                    for(int i =0;i<topoN;i++) VtxwritehandlesAdd[i]->push_back(0);
                }else{
                    const std::vector<xAOD::Vertex*> &cascadeVertices = x->vertices();
                    if(cascadeVertices.size()!=topoN)
                        ATH_MSG_ERROR("Incorrect number of vertices");
                    if(cascadeVertices[0] == nullptr || cascadeVertices[1] == nullptr) ATH_MSG_ERROR("Error null vertex");
                    // Keep vertices (bear in mind that they come in reverse order!)
                    for(int i =0;i<topoN;i++) VtxwritehandlesAdd[i]->push_back(cascadeVertices[i]);
                    
                    x->getSVOwnership(false); // Prevent Container from deleting vertices
                    const auto mainVertex = cascadeVertices[1];   // this is the B_c+/- vertex
                    const std::vector< std::vector<TLorentzVector> > &moms = x->getParticleMoms();
                    
                    // Set links to cascade vertices
                    std::vector<xAOD::Vertex*> verticestoLink;
                    verticestoLink.push_back(cascadeVertices[0]);
                    if(VtxwritehandlesAdd[1] == nullptr) ATH_MSG_ERROR("VtxwritehandlesAdd[1] is null");
                    if(!BPhysPVCascadeTools::LinkVertices(CascadeAddLinksDecor, verticestoLink, VtxwritehandlesAdd[0], cascadeVertices[1]))
                        ATH_MSG_ERROR("Error decorating with cascade vertices");
                    
                    //Identify and decorate mu+pi+trk input
                    //----------------------------------- trk
                    // Identify additional track
                    TrackBag selectedIDTracks; selectedIDTracks.clear();
                    for(auto tp : *trackContainer){
                        if (tp == cascadeVertices[1]->trackParticle(2)) selectedIDTracks.push_back(tp); //there is always only one muon
                    }
                    ATH_MSG_DEBUG("selectedIDtrack size "<<selectedIDTracks.size()); //always only one track
                    
                    // Create link for track
                    // create tmp vector of preceding vertex links
                    TrackLinkVector preTrkLinks;
                    
                    // loop over input precedingMuons
                    auto trkItr = selectedIDTracks.begin();
                    for(; trkItr != selectedIDTracks.end(); trkItr++) {
                        // create element link
                        TrackLink trkLink;
                        trkLink.setElement(*trkItr);
                        trkLink.setStorableObject(*trackContainer);
                        
                        // sanity check : is the link valid?
                        if( !trkLink.isValid() ) continue;
                        
                        // link is OK, store it in the tmp vector
                        preTrkLinks.push_back( trkLink );
                        
                    } // end of loop over preceding vertices
                    
                    // all OK: store preceding vertex links in the aux store
                    TrackLinksDecor(*cascadeVertices[1]) = preTrkLinks;
                    //----------------------------------- trk
                    
                    //----------------------------------- mu+pi
                    std::vector<xAOD::Vertex*> MuPiAddVerticestoLink;
                    xAOD::Vertex* MuPiAddVertex;
                    
                    for(auto vxcItr : *MuPiContainer){
                        unsigned int NTracks = vxcItr->nTrackParticles();
                        std::array<const xAOD::TrackParticle*, 2> a1;
                        std::array<const xAOD::TrackParticle*, 2> a2;
                        for(size_t i=0;i<NTracks;i++){
                            a1[i] = vxcItr->trackParticle(i);
                            a2[i] = cascadeVertices[1]->trackParticle(i);
                        }
                        std::sort(a1.begin(), a1.end());
                        std::sort(a2.begin(), a2.end());
                        if(a1 == a2){
                            MuPiAddVertex = vxcItr;
                        }
                    }
                    if (MuPiAddVertex) MuPiAddVerticestoLink.push_back(MuPiAddVertex);
                    else ATH_MSG_WARNING("Could not find linking mu+pi_soft additional");
                    if(!BPhysPVCascadeTools::LinkVertices(MuPiTrkLinksDecor, MuPiAddVerticestoLink, MuPiContainer, cascadeVertices[1]))
                        ATH_MSG_ERROR("Error decorating with mu+pi_soft additional vertices");
                    
                    //----------------------------------- mu+pi
                    //END Identify and decorate mu+pi+trk input
                    
                    //-----------------------------------
                    // Identify and decorate the input D0
                    xAOD::Vertex* d0AddVertex = BPhysPVCascadeTools::FindVertex<2>(d0Container, cascadeVertices[0]);;
                    ATH_MSG_DEBUG("1 pt D0 (add) tracks " << cascadeVertices[0]->trackParticle(0)->pt() << ", " << cascadeVertices[0]->trackParticle(1)->pt());
                    if (d0AddVertex) ATH_MSG_DEBUG("2 pt D0 (add) tracks " << d0AddVertex->trackParticle(0)->pt() << ", " << d0AddVertex->trackParticle(1)->pt());
                    
                    std::vector<xAOD::Vertex*> d0AddVerticestoLink;
                    if (d0AddVertex) d0AddVerticestoLink.push_back(d0AddVertex);
                    else ATH_MSG_WARNING("Could not find linking D0 (add)");
                    if(!BPhysPVCascadeTools::LinkVertices(D0AddLinksDecor, d0AddVerticestoLink, d0Container, cascadeVertices[1]))
                        ATH_MSG_ERROR("Error decorating with D0 (add) vertices");
                    //-----------------------------------
                    //--------- mass hypo for main fit of B vertex
                    double mass_b = m_vtx0MassHypo;
                    
                    // reset beamspot cache
                    helper.GetBeamSpot(true);
                    // loop over candidates -- Don't apply PV_minNTracks requirement here
                    // because it may result in exclusion of the high-pt PV.
                    // get good PVs
                    
                    xAOD::BPhysHypoHelper vtx(m_hypoName, mainVertex);
                    
                    // Get refitted track momenta from all vertices, charged tracks only
                    BPhysPVCascadeTools::SetVectorInfo(vtx, x);
                    // Decorate main vertex
                    //
                    // 1.a) mass, mass error
                    BPHYS_CHECK( vtx.setMass(m_CascadeToolsAdd->invariantMass(moms[1])) );
                    BPHYS_CHECK( vtx.setMassErr(m_CascadeToolsAdd->invariantMassError(moms[1],x->getCovariance()[1])) );
                    // 1.b) pt and pT error (the default pt of mainVertex is != the pt of the full cascade fit!)
                    PtAdd_decor(*mainVertex) = m_CascadeToolsAdd->pT(moms[1]);
                    PtAddErr_decor(*mainVertex) = m_CascadeToolsAdd->pTError(moms[1],x->getCovariance()[1]);
                    // 1.c) chi2 and ndof (the default chi2 of mainVertex is != the chi2 of the full cascade fit!)
                    Chi2Add_decor(*mainVertex) = x->fitChi2();
                    ndofAdd_decor(*mainVertex) = x->nDoF();
                    
                    ATH_CHECK(helper.FillCandwithRefittedVertices(m_refitPV, pvContainer, refPvContainer, &(*m_pvRefitter), m_PV_max, m_DoVertexType, x, 1, mass_b, vtx));
                    
                    // 4) decorate the main vertex with D0 vertex mass, pt, lifetime and lxy values (plus errors)
                    // D0 points to the main vertex, so lifetime and lxy are w.r.t the main vertex
                    D0AddMass_svdecor(*mainVertex) = m_CascadeToolsAdd->invariantMass(moms[0]);
                    D0AddMassErr_svdecor(*mainVertex) = m_CascadeTools->invariantMassError(moms[0],x->getCovariance()[0]);
                    D0AddPt_svdecor(*mainVertex) = m_CascadeToolsAdd->pT(moms[0]);
                    D0AddPtErr_svdecor(*mainVertex) = m_CascadeToolsAdd->pTError(moms[0],x->getCovariance()[0]);
                    D0AddLxy_svdecor(*mainVertex) = m_CascadeToolsAdd->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]);
                    D0AddLxyErr_svdecor(*mainVertex) = m_CascadeToolsAdd->lxyError(moms[0],x->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
                    D0AddTau_svdecor(*mainVertex) = m_CascadeToolsAdd->tau(moms[0],cascadeVertices[0],cascadeVertices[1]);
                    D0AddTauErr_svdecor(*mainVertex) = m_CascadeToolsAdd->tauError(moms[0],x->getCovariance()[0],cascadeVertices[0],cascadeVertices[1]);
                }
            } //loop over x
            

        }// loop over xx - additional cascadeinfoContainer

        // Deleting cascadeinfo since this won't be stored.
        // Vertices have been kept in m_cascadeOutputs and should be owned by their container
        for (std::vector<Trk::VxCascadeInfo*> xx : cascadeinfoContainer2) {
            for (auto x : xx) delete x;
        }
      return StatusCode::SUCCESS;
    }


    MuPlusDpstCascade::MuPlusDpstCascade(const std::string& t, const std::string& n, const IInterface* p)  : AthAlgTool(t,n,p),
    m_vertexContainerKey(""),
    m_vertexD0ContainerKey(""),
    m_cascadeOutputsKeys{ "MuPlusDpstCascadeVtx1", "MuPlusDpstCascadeVtx2" },
m_cascadeOutputsKeysAdd{"TrkPlusBCascadeVtx1","TrkPlusBCascadeVtx2"},
    m_VxPrimaryCandidateName("PrimaryVertices"),
    m_MuPiMassLower(0.0),
    m_MuPiMassUpper(10000.0),
    m_D0MassLower(0.0),
    m_D0MassUpper(10000.0),
    m_DstMassLower(0.0),
    m_DstMassUpper(10000.0),
    m_DstMassUpperAft(10000.0),
    m_MassLower(0.0),
    m_MassUpper(20000.0),
    m_PtLower(9500.0),
    m_vtx0MassHypo(-1),
    m_vtx1MassHypo(-1),
    m_vtx0Daug1MassHypo(-1),
    m_vtx0Daug2MassHypo(-1),
    m_vtx1Daug1MassHypo(-1),
    m_vtx1Daug2MassHypo(-1),
    m_Dx_pid(421),
    m_constrD0(true),
    m_constrMuPi(false),
    m_chi2cut(-1.0),
    m_beamSpotSvc("BeamCondSvc",n),
    m_iVertexFitter("Trk::TrkVKalVrtFitter"),
m_iVertexFitter2("Trk::TrkVKalVrtFitter"),

    m_pvRefitter("Analysis::PrimaryVertexRefitter"),
    m_V0Tools("Trk::V0Tools"),
    m_CascadeTools("DerivationFramework::CascadeTools"),
m_CascadeToolsAdd("DerivationFramework::CascadeTools")

    {
       declareProperty("MuPiVertices",              m_vertexContainerKey);
       declareProperty("D0Vertices",                m_vertexD0ContainerKey);
       declareProperty("VxPrimaryCandidateName",    m_VxPrimaryCandidateName);
       declareProperty("RefPVContainerName",        m_refPVContainerName  = "RefittedPrimaryVertices");
       declareProperty("MuPiMassLowerCut",          m_MuPiMassLower);
       declareProperty("MuPiMassUpperCut",          m_MuPiMassUpper);
       declareProperty("D0MassLowerCut",            m_D0MassLower);
       declareProperty("D0MassUpperCut",            m_D0MassUpper);
       declareProperty("DstMassLowerCut",           m_DstMassLower);
       declareProperty("DstMassUpperCut",           m_DstMassUpper);
       declareProperty("DstMassUpperCutAft",        m_DstMassUpperAft); //mass cut after cascade fit
       declareProperty("MassLowerCut",              m_MassLower);
       declareProperty("MassUpperCut",              m_MassUpper);
       declareProperty("PtLowerCut",                m_PtLower);
       declareProperty("HypothesisName",            m_hypoName               = "B");
       declareProperty("Vtx0MassHypo",              m_vtx0MassHypo);
       declareProperty("Vtx1MassHypo",              m_vtx1MassHypo);
       declareProperty("Vtx0Daug1MassHypo",         m_vtx0Daug1MassHypo);
       declareProperty("Vtx0Daug2MassHypo",         m_vtx0Daug2MassHypo);
       declareProperty("Vtx0Daug3MassHypo",         m_vtx0Daug2MassHypo);
       declareProperty("Vtx1Daug1MassHypo",         m_vtx1Daug1MassHypo);
       declareProperty("Vtx1Daug2MassHypo",         m_vtx1Daug2MassHypo);
       declareProperty("DxHypothesis",              m_Dx_pid);
       declareProperty("ApplyD0MassConstraint",     m_constrD0);
       declareProperty("ApplyMuPiMassConstraint",   m_constrMuPi);
       declareProperty("Chi2Cut",                   m_chi2cut);
       declareProperty("RefitPV",                   m_refitPV                = true);
       declareProperty("MaxnPV",                    m_PV_max                 = 999);
       declareProperty("MinNTracksInPV",            m_PV_minNTracks          = 0);
       declareProperty("DoVertexType",              m_DoVertexType           = 7);
       declareProperty("TrkVertexFitterTool",       m_iVertexFitter);
       declareProperty("TrkVertexFitterToolAdd",    m_iVertexFitter2);
       declareProperty("PVRefitter",                m_pvRefitter);
       declareProperty("V0Tools",                   m_V0Tools);
       declareProperty("CascadeTools",              m_CascadeTools);
       declareProperty("CascadeVertexCollections",  m_cascadeOutputsKeys);
       declareProperty("AdditionalCascadeVertexCollections",  m_cascadeOutputsKeysAdd);

    }

    MuPlusDpstCascade::~MuPlusDpstCascade(){ }
    bool MuPlusDpstCascade::isContainedIn(const xAOD::TrackParticle* theTrack, std::vector<const xAOD::TrackParticle*> theColl) {

        bool isContained(false);
        size_t MuPiTrkNum = theColl.size();
        for( unsigned int it=0; it<MuPiTrkNum; it++)
            if ( theColl.at(it)->index() == theTrack->index()  ) {isContained=true; break;}
        return isContained;
    }
    double MuPlusDpstCascade::getInvariantMass(const TrackBag &Tracks, const std::vector<double> &massHypotheses){
      TLorentzVector total;
      total.SetVectM(Tracks[0]->p4().Vect(), massHypotheses[0]);
      TLorentzVector temp;
      for(size_t i=1; i < Tracks.size(); i++){
           temp.SetVectM(Tracks[i]->p4().Vect(), massHypotheses[i]);
           total += temp;
      }
      return total.M();
    }

    StatusCode MuPlusDpstCascade::performSearch(std::vector<Trk::VxCascadeInfo*> *cascadeinfoContainer, std::vector<std::vector<Trk::VxCascadeInfo*>> *cascadeinfoContainer2) const
    {

        assert(cascadeinfoContainer!=nullptr);

        // Get TrackParticle container
        const xAOD::TrackParticleContainer  *trackContainer(nullptr);
        ATH_CHECK(evtStore()->retrieve(trackContainer   , "InDetTrackParticles"      ));

        // Get mu+pi container
        const xAOD::VertexContainer  *MuPiContainer(nullptr);
        ATH_CHECK(evtStore()->retrieve(MuPiContainer   , m_vertexContainerKey       ));

        // Get D0 container
        const xAOD::VertexContainer  *d0Container(nullptr);
        ATH_CHECK(evtStore()->retrieve(d0Container   , m_vertexD0ContainerKey       )); //"D0Vertices"

        //--------- mass hypo for main fit of B vertex
        double mass_d0 = m_vtx1MassHypo;
        std::vector<const xAOD::TrackParticle*> tracksMuPi;
        std::vector<const xAOD::TrackParticle*> tracksD0;
        std::vector<double> massesMuPi;
        massesMuPi.push_back(m_vtx0Daug1MassHypo); //mass mu
        massesMuPi.push_back(m_vtx0Daug2MassHypo); //mass pi
        std::vector<double> massesD0;
        massesD0.push_back(m_vtx1Daug1MassHypo); //mass pi
        massesD0.push_back(m_vtx1Daug2MassHypo); //mass K
        std::vector<double> massesD0b; // Change the oreder of masses for D*-->D0bar pi-, D0bar->K+pi-
        massesD0b.push_back(m_vtx1Daug2MassHypo);
        massesD0b.push_back(m_vtx1Daug1MassHypo);
        
        //--------- mass hypo for additional fit of B vertex with track
        std::vector<double> secondCVertexMass; // masses of all particles фаеук additional fit
        secondCVertexMass.push_back(m_vtx0Daug1MassHypo); //mu
        secondCVertexMass.push_back(m_vtx0Daug2MassHypo); //pi_soft
        secondCVertexMass.push_back(m_vtx1Daug1MassHypo); //pi
        //---------


        // Select mu+pi_soft candidates before calling cascade fit
        std::vector<const xAOD::Vertex*> selectedMuPiCandidates;
        for ( auto vxcItr : *MuPiContainer ){
            // Check mu+pi_soft candidate invariant mass and skip if need be
            TLorentzVector p4Mup_in, p4Mum_in;
            p4Mup_in.SetPtEtaPhiM(vxcItr->trackParticle(0)->pt(),
                                  vxcItr->trackParticle(0)->eta(),
                                  vxcItr->trackParticle(0)->phi(), m_vtx0Daug1MassHypo);
            p4Mum_in.SetPtEtaPhiM(vxcItr->trackParticle(1)->pt(),
                                  vxcItr->trackParticle(1)->eta(),
                                  vxcItr->trackParticle(1)->phi(), m_vtx0Daug2MassHypo);
            double mass_MuPi = (p4Mup_in + p4Mum_in).M();
            ATH_MSG_DEBUG("mu pi_soft mass " << mass_MuPi);
            if (mass_MuPi < m_MuPiMassLower || mass_MuPi > m_MuPiMassUpper) {
                ATH_MSG_DEBUG(" Original mu & pi_soft candidate rejected by the mass cut: mass = "
                           << mass_MuPi << " != (" << m_MuPiMassLower << ", " << m_MuPiMassUpper << ")" );
                continue;
            }

            // Track selection - Loose
            // for soft pion wich is (2nd) in MuPi vertex
            if ( !m_trackSelectionTools->accept(vxcItr->trackParticle(1)) ){
                ATH_MSG_DEBUG(" Original mu & pi_soft candidate rejected by the track's cut level - loose");
                continue;
            }

            selectedMuPiCandidates.push_back(vxcItr);
        } //for(auto vxcItr : *MuPiContainer)
        if(selectedMuPiCandidates.size()<1) return StatusCode::SUCCESS;
        SG::AuxElement::Accessor<Char_t> flagAcc1("passed_D0");
        SG::AuxElement::Accessor<Char_t> flagAcc2("passed_D0b");
        // Select the D0/D0b candidates before calling cascade fit
        std::vector<const xAOD::Vertex*> selectedD0Candidates;
        for(auto vxcItr : *d0Container){
           // Check the passed flag first
           xAOD::Vertex* vtx = vxcItr;
           bool isD0(true);
           bool isD0b(true);
           if(flagAcc1.isAvailable(*vtx)){
              if(!flagAcc1(*vtx)) isD0 = false;
           }
           if(flagAcc2.isAvailable(*vtx)){
              if(!flagAcc2(*vtx)) isD0b = false;
           }
           if(!(isD0||isD0b)) continue;

           // Track selection - Loose
           if ( !m_trackSelectionTools->accept(vxcItr->trackParticle(0)) ){
               ATH_MSG_DEBUG(" Original D0/D0-bar candidate rejected by the track's cut level - loose ");
               continue;
           }
           if ( !m_trackSelectionTools->accept(vxcItr->trackParticle(1)) ){
               ATH_MSG_DEBUG(" Original D0/D0-bar candidate rejected by the track's cut level - loose ");
               continue;
           }


           // Ensure the total charge is correct
           if (vxcItr->trackParticle(0)->charge() != 1 || vxcItr->trackParticle(1)->charge() != -1) {
              ATH_MSG_DEBUG(" Original D0/D0-bar candidate rejected by the charge requirement: "
                              << vxcItr->trackParticle(0)->charge() << ", " << vxcItr->trackParticle(1)->charge() );
             continue;
           }
            
           // Check D0/D0bar candidate invariant mass and skip if need be
           double mass_D0 = m_V0Tools->invariantMass(vxcItr,massesD0);
           double mass_D0b = m_V0Tools->invariantMass(vxcItr,massesD0b);
           ATH_MSG_DEBUG("D0 mass " << mass_D0 << ", D0b mass "<<mass_D0b);
           if ((mass_D0 < m_D0MassLower || mass_D0 > m_D0MassUpper) && (mass_D0b < m_D0MassLower || mass_D0b > m_D0MassUpper)) {
              ATH_MSG_DEBUG(" Original D0/D0-bar candidate rejected by the mass cut: mass = "
                            << mass_D0 << " != (" << m_D0MassLower << ", " << m_D0MassUpper << ") " 
                            << mass_D0b << " != (" << m_D0MassLower << ", " << m_D0MassUpper << ") " );
             continue;
           }

           selectedD0Candidates.push_back(vxcItr);
        } //for(auto vxcItr : *d0Container)
        if(selectedD0Candidates.size()<1) return StatusCode::SUCCESS;

        // Select the inner detector tracks for additional cascade fit: trk + mu+pi + D0
        TrackBag selectedIDTracks;
        for (auto tp : *trackContainer){
            //checking trk not in the mu+pi_soft pair
            //if (isContainedIn(tp, selectedMuPiCandidates)) continue;
            
            //checking trk not in the D0
            //if (isContainedIn(tp, selectedD0Candidates)) continue;
            
            //if ( m_trkSelector->decision(*tp, NULL) ) selectedIDTracks.push_back(tp);
            if ( !m_trackSelectionTools->accept(tp)){
                ATH_MSG_DEBUG(" Inner detector track is rejected by the cut level - loose ");
                continue;
            }
            selectedIDTracks.push_back(tp);
        }
        ATH_MSG_DEBUG("CUSTOM:: selected track number "<<selectedIDTracks.size());
        
        // Select mu D*+ candidates
        // Iterate over mu+pi_soft vertices
        for(auto MuPiItr:selectedMuPiCandidates){
            size_t MuPiTrkNum = MuPiItr->nTrackParticles();

            tracksMuPi.clear();
            for( unsigned int it=0; it<MuPiTrkNum; it++) tracksMuPi.push_back(MuPiItr->trackParticle(it));

            if (tracksMuPi.size() != 2 || massesMuPi.size() != 2 ) {
                ATH_MSG_INFO("problems with mu+pi_soft input");
            }

            bool tagD0(true);
            if(std::abs(m_Dx_pid)==421 && MuPiItr->trackParticle(1)->charge()==-1) tagD0 = false;

            TLorentzVector p4_pi1; // Momentum of soft pion1 = our soft pion (1)
            p4_pi1.SetPtEtaPhiM(MuPiItr->trackParticle(1)->pt(),
                                MuPiItr->trackParticle(1)->eta(),
                                MuPiItr->trackParticle(1)->phi(), m_vtx0Daug2MassHypo);
            // Iterate over D0/D0bar vertices
            for(auto d0Itr : selectedD0Candidates){

                // Check identical tracks in input
                //if(std::find(tracksMuPi.cbegin(), tracksMuPi.cend(), d0Itr->trackParticle(0)) != tracksMuPi.cend()) continue;
                //if(std::find(tracksMuPi.cbegin(), tracksMuPi.cend(), d0Itr->trackParticle(1)) != tracksMuPi.cend()) continue;
                if (isContainedIn(d0Itr->trackParticle(0), tracksMuPi)) continue;
                if (isContainedIn(d0Itr->trackParticle(1), tracksMuPi)) continue;

                TLorentzVector p4_ka, p4_pi2;
                if(tagD0){ // for D*+
                p4_pi2.SetPtEtaPhiM(d0Itr->trackParticle(0)->pt(),
                                    d0Itr->trackParticle(0)->eta(),
                                    d0Itr->trackParticle(0)->phi(), m_vtx1Daug1MassHypo);
                p4_ka.SetPtEtaPhiM( d0Itr->trackParticle(1)->pt(),
                                    d0Itr->trackParticle(1)->eta(),
                                    d0Itr->trackParticle(1)->phi(), m_vtx1Daug2MassHypo);
                }else{ // change the order in the case of D*-
                p4_pi2.SetPtEtaPhiM(d0Itr->trackParticle(1)->pt(),
                                    d0Itr->trackParticle(1)->eta(),
                                    d0Itr->trackParticle(1)->phi(), m_vtx1Daug1MassHypo);
                p4_ka.SetPtEtaPhiM( d0Itr->trackParticle(0)->pt(),
                                    d0Itr->trackParticle(0)->eta(),
                                    d0Itr->trackParticle(0)->phi(), m_vtx1Daug2MassHypo);
                }
                // Check D*+/- candidate invariant mass and skip if need be
                double mass_Dst= (p4_pi1 + p4_ka + p4_pi2).M();
                ATH_MSG_DEBUG("D*+/- mass " << mass_Dst);
                if (mass_Dst < m_DstMassLower || mass_Dst > m_DstMassUpper) {
                    ATH_MSG_DEBUG(" Original D*+/- candidate rejected by the mass cut: mass = "
                              << mass_Dst << " != (" << m_DstMassLower << ", " << m_DstMassUpper << ")" );
                    continue;
                }

              size_t d0TrkNum = d0Itr->nTrackParticles(); //2
              tracksD0.clear();
              for( unsigned int it=0; it<d0TrkNum; it++) tracksD0.push_back(d0Itr->trackParticle(it));
              if (tracksD0.size() != 2 || massesD0.size() != 2 ) {
                ATH_MSG_INFO("problems with D0 input");
              }
            
              //Leaving for the possible mods in the future
              //SG::AuxElement::Accessor<xAOD::Vertex_v1::TrackParticleLinks_t> trackAcc( "trackParticleLinks" );
              //ATH_MSG_INFO("CUSTOM:: "<<*(trackAcc(*d0Itr)).at(0));
              //ATH_MSG_INFO("CUSTOM2:: "<<tracksD0.at(0));             //-> gives the same result

              // Apply the user's settings to the fitter
              // Reset
              m_iVertexFitter->setDefault();
              // Robustness
              int robustness = 0;
              m_iVertexFitter->setRobustness(robustness);
              // Build up the topology
              // Vertex list
              std::vector<Trk::VertexID> vrtList;
              // D0 vertex
              Trk::VertexID vID;
              if (m_constrD0) { //ApplyD0MassConstraint = true
                if(tagD0) vID = m_iVertexFitter->startVertex(tracksD0,massesD0,mass_d0);
                else vID = m_iVertexFitter->startVertex(tracksD0,massesD0b,mass_d0);
              } else {
                if(tagD0) vID = m_iVertexFitter->startVertex(tracksD0,massesD0);
                else vID = m_iVertexFitter->startVertex(tracksD0,massesD0b);
              }
              vrtList.push_back(vID);
              // B vertex including mu+pi_soft
              Trk::VertexID vID2 = m_iVertexFitter->nextVertex(tracksMuPi,massesMuPi,vrtList);
              if (m_constrMuPi) {
                std::vector<Trk::VertexID> cnstV;
                cnstV.clear();
                if ( !m_iVertexFitter->addMassConstraint(vID2,tracksMuPi,cnstV,m_vtx0MassHypo).isSuccess() ) {
                    ATH_MSG_WARNING("addMassConstraint failed");
                    //return StatusCode::FAILURE;
                }
              }

              // Do the work
              std::unique_ptr<Trk::VxCascadeInfo> result(m_iVertexFitter->fitCascade());
 
                if (result != nullptr) {
                // reset links to original tracks
                BPhysPVCascadeTools::PrepareVertexLinks(result.get(), trackContainer);
                ATH_MSG_DEBUG("storing tracks " << ((result->vertices())[0])->trackParticle(0) << ", "
                                                << ((result->vertices())[0])->trackParticle(1) << ", "
                                                << ((result->vertices())[1])->trackParticle(0) << ", "
                                                << ((result->vertices())[1])->trackParticle(1));
                // necessary to prevent memory leak
                result->getSVOwnership(true);

                // Chi2/DOF cut
                double bChi2DOF = result->fitChi2()/result->nDoF();
                ATH_MSG_DEBUG("Candidate chi2/DOF is " << bChi2DOF);
                bool chi2CutPassed = (m_chi2cut <= 0.0 || bChi2DOF < m_chi2cut);

                const std::vector< std::vector<TLorentzVector> > &moms = result->getParticleMoms();
                const std::vector<xAOD::Vertex*> &cascadeVertices = result->vertices();
                   
                double mass = m_CascadeTools->invariantMass(moms[1]);
                double DstMassAft = (moms[1][1] + moms[0][0] + moms[0][1]).M(); //pi_soft + D0
                  /*
                  //----------------------------------------------------
                  // retrieve primary vertices for Lxy(B)
                  const xAOD::Vertex * primaryVertex(nullptr);
                  const xAOD::VertexContainer *pvContainer(nullptr);
                  ATH_CHECK(evtStore()->retrieve(pvContainer, m_VxPrimaryCandidateName));
                  ATH_MSG_DEBUG("Found " << m_VxPrimaryCandidateName << " in StoreGate!");

                  if (pvContainer->size()==0){
                      ATH_MSG_WARNING("You have no primary vertices: " << pvContainer->size());
                      return StatusCode::RECOVERABLE;
                  } else {
                      primaryVertex = (*pvContainer)[0];
                  }
                  //----------------------------------------------------
                  */
                if(chi2CutPassed) {
                  if (mass >= m_MassLower && mass <= m_MassUpper) {
                      if (m_CascadeTools->pT(moms[1]) > m_PtLower){ //B_pT
                          if (m_CascadeTools->lxy(moms[0],cascadeVertices[0],cascadeVertices[1]) > 0){ //D0_Lxy>0
                              if (DstMassAft < m_DstMassUpperAft){
                                  
                                  cascadeinfoContainer->push_back(result.release());

                                  //---------------------------------------------------
                                  //after the successfull cascade fit we do another cascade with additional track from InDetCollection
                                  // only for those candidates selected for cascasdeinfoContainer
                                  // trk  +  mu+pi + D0_tracks
                                  //putting together tracks for initial cascade vertex and second(additional cascade vertex)
                                  TrackBag secondCVertexTracks;

                                  for(unsigned int i=0; i<tracksMuPi.size(); i++){
                                      secondCVertexTracks.push_back(tracksMuPi.at(i));
                                  }
                                  //putting axtra particle in secondary tracks to "reserve" place for the additional track, which will be placed later
                                  secondCVertexTracks.push_back(tracksMuPi.at(0));

                                  if(secondCVertexTracks.size() != secondCVertexMass.size()){
                                      ATH_MSG_INFO("Mass hypothesis not correctly set, aborting");
                                      return StatusCode::SUCCESS;
                                  }
                                    if(selectedIDTracks.size()<1) {
                                          return StatusCode::SUCCESS;
                                      }

                                  std::vector<Trk::VxCascadeInfo*> cascadeTEMPcontainer;
                                      cascadeTEMPcontainer.clear();
                                      for(auto newtrack : selectedIDTracks){
                                          //Skip any track already used in vertex
                                          //if(std::find(tracksD0.begin(), tracksD0.end(), newtrack) != tracksD0.end()) {
                                          if (isContainedIn(newtrack, tracksD0)) {
                                              ATH_MSG_INFO("CUSTOM:: track is already in use");
                                              continue;
                                          }
                                          /*if(std::find(tracksMuPi.begin(), tracksMuPi.end(), newtrack) != tracksMuPi.end()) {
                                              ATH_MSG_INFO("CUSTOM:: track is already in use");
                                              continue;
                                          }*/
                                          if (isContainedIn(newtrack, tracksMuPi)) {
                                              ATH_MSG_INFO("CUSTOM:: track is already in use");
                                              continue;
                                          }
                                          //putting current track to the back of secondary tracks vector
                                          secondCVertexTracks.back() = newtrack;

                                          double roughmass = getInvariantMass(secondCVertexTracks, secondCVertexMass);
                                          if(m_MassUpper > 0.0 && (roughmass < m_MassLower || roughmass > m_MassUpper+500)) continue;
                                          
                                          // Apply the user's settings to the fitter
                                          // Reset
                                          m_iVertexFitter2->setDefault();
                                          // Robustness
                                          m_iVertexFitter2->setRobustness(robustness);
                                          // Build up the topology
                                          // Vertex list
                                          std::vector<Trk::VertexID> vrtList2;
                                          // D0 vertex
                                          Trk::VertexID vID2_1;
                                          if (m_constrD0) { //ApplyD0MassConstraint = true  m_constrD0
                                            if(tagD0) vID2_1 = m_iVertexFitter2->startVertex(tracksD0,massesD0,mass_d0);
                                            else vID2_1 = m_iVertexFitter2->startVertex(tracksD0,massesD0b,mass_d0);
                                          } else {
                                            if(tagD0) vID2_1 = m_iVertexFitter2->startVertex(tracksD0,massesD0);
                                            else vID2_1 = m_iVertexFitter2->startVertex(tracksD0,massesD0b);
                                          }
                                          vrtList2.push_back(vID2_1);
                                          // B vertex including mu+pi_soft
                                          Trk::VertexID vID2_2;
                                          vID2_2 = m_iVertexFitter2->nextVertex(secondCVertexTracks,secondCVertexMass,vrtList2);
                                          if (m_constrMuPi) {
                                            std::vector<Trk::VertexID> cnstV;
                                            cnstV.clear();
                                            if ( !m_iVertexFitter->addMassConstraint(vID2_2,tracksMuPi,cnstV,m_vtx0MassHypo).isSuccess() ) {
                                                ATH_MSG_WARNING("addMassConstraint failed");
                                                //return StatusCode::FAILURE;
                                            }
                                          }

                                          std::unique_ptr<Trk::VxCascadeInfo> result2(m_iVertexFitter2->fitCascade());
                                          if(result2 != nullptr ){
                                              ATH_MSG_DEBUG("Additional cascade Fit succeded");
                                          }
                                          if(result2 == nullptr ){
                                              ATH_MSG_DEBUG("Additional cascade Fit failed");
                                              continue;
                                          }
                                          BPhysPVCascadeTools::PrepareVertexLinks(result2.get(), trackContainer);
                                          // necessary to prevent memory leak
                                          result2->getSVOwnership(true);
                                          assert(result2->vertices().size()==2);
                                          
                                          const std::vector< std::vector<TLorentzVector> > &momsAdd = result2->getParticleMoms();
                                          const std::vector<xAOD::Vertex*> &cascadeVerticesAdd = result2->vertices();
                                          //----------------------------------------------------
                                          // retrieve primary vertices for Lxy(B)
                                          const xAOD::Vertex * primaryVertexAdd(nullptr);
                                          const xAOD::VertexContainer *pvContainerAdd(nullptr);
                                          ATH_CHECK(evtStore()->retrieve(pvContainerAdd, m_VxPrimaryCandidateName));
                                          ATH_MSG_DEBUG("Found " << m_VxPrimaryCandidateName << " in StoreGate!");

                                          if (pvContainerAdd->size()==0){
                                              ATH_MSG_WARNING("You have no primary vertices: " << pvContainerAdd->size());
                                              return StatusCode::RECOVERABLE;
                                          } else {
                                              primaryVertexAdd = (*pvContainerAdd)[0];
                                          }
                                          //----------------------------------------------------
                                         
                                          // Chi2/DOF cut
                                          //same as for the main vertex chi2/ndof = 35/7 = 5
                                          double bChi2DOF2 = result2->fitChi2()/result2->nDoF();
                                          //ATH_MSG_INFO("Candidate chi2/DOF for additional fit is " << bChi2DOF2<<" "<<result2->nDoF());
                                          bool chi2CutPassed2 = (m_chi2cut <= 0.0 || bChi2DOF2 < m_chi2cut);
                                          double massAdd = m_CascadeToolsAdd->invariantMass(momsAdd[1]);
                                          double bLxyAdd = m_CascadeToolsAdd->lxy(momsAdd[1],cascadeVerticesAdd[1],primaryVertexAdd);
                                          double DstAddMassAft = (momsAdd[1][1] + momsAdd[0][0] + momsAdd[0][1]).M(); //pi_soft + D0

                                          if(chi2CutPassed2){
                                              if (massAdd >= m_MassLower && massAdd <= m_MassUpper) { //same as for the main cascade
                                                  if(bLxyAdd > 0.1){
                                                      if (DstAddMassAft < m_DstMassUpperAft){
                                                          //ATH_MSG_INFO("CUSTOM:: passed chi2 check");
                                                          cascadeTEMPcontainer.push_back(result2.release());
                                                      }
                                                  }//B_Lxy for add fit check
                                              }//mass check
                                        }//chi2 check
                                          
                                      }//loop for additional tracks
                                        //vector of cascades for each candidate
                                      cascadeinfoContainer2->push_back(cascadeTEMPcontainer);

                              } else {
                                ATH_MSG_DEBUG("Candidate rejected by the mass cut on m(D^{*})" );
                              } //Dst_m < m_DstMassUpperAft
                          } else {
                            ATH_MSG_DEBUG("Candidate rejected by the L_{xy}(D_{0}) cut" );
                          } //D0_Lxy>0
                      } else {
                        ATH_MSG_DEBUG("Candidate rejected by the p_{T}(B) cut" );
                      } //B_pT
                  } else {
                    ATH_MSG_DEBUG("Candidate rejected by the mass cut: mass = "
                                  << mass << " != (" << m_MassLower << ", " << m_MassUpper << ")" );
                  } //mass Upper/Lower
                } else {
                  ATH_MSG_DEBUG("Candidate rejected by the chi2 cut" );
                } //chi2CutPassed



              } //if (result != nullptr)

            } //Iterate over D0 vertices

         } //Iterate over mu+pi_soft vertices
         
         


        ATH_MSG_DEBUG("cascadeinfoContainer size " << cascadeinfoContainer->size());
        ATH_MSG_DEBUG("cascadeinfoContainer (additional) size " << cascadeinfoContainer2->size());

        return StatusCode::SUCCESS;
    }//end of perform search

    
}

