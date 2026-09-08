/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BPhysBGammaFinder.h"
#include "xAODBPhys/BPhysHypoHelper.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "AthContainers/ConstAccessor.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"


using VertexLink = ElementLink<xAOD::VertexContainer>;

namespace DerivationFramework {

BPhysBGammaFinder::BPhysBGammaFinder(const std::string& t, const std::string& n, const IInterface* p)
    : base_class(t,n,p) {
}


StatusCode BPhysBGammaFinder::initialize() {

  ATH_MSG_DEBUG("in initialize()");

  ATH_CHECK( m_v0Tools.retrieve() );
  ATH_CHECK( m_vertexFitter.retrieve() );
  ATH_CHECK( m_vertexEstimator.retrieve() );
  ATH_CHECK( m_BVertexCollectionsToCheck.initialize() );
  ATH_CHECK( m_inputTrackParticleContainerName.initialize() );
  ATH_CHECK( m_inputLowPtTrackContainerName.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_conversionContainerName.initialize() );
  return StatusCode::SUCCESS;
}


StatusCode BPhysBGammaFinder::execute(const EventContext& ctx) const {

  std::vector<const xAOD::Vertex*> BVertices;
  std::vector<const xAOD::TrackParticle*> BVertexTracks;


  // Output conversion container
  std::unique_ptr<xAOD::VertexContainer> conversionContainer(new xAOD::VertexContainer());
  std::unique_ptr<xAOD::VertexAuxContainer> conversionAuxContainer(new xAOD::VertexAuxContainer());
  conversionContainer->setStore(conversionAuxContainer.get());

  // Retrieve track particles from StoreGate
  SG::ReadHandle<xAOD::TrackParticleContainer> inputTrackParticles{m_inputTrackParticleContainerName, ctx};
  ATH_MSG_DEBUG( "Track particle container size " << inputTrackParticles->size() );

  std::vector<const xAOD::TrackParticle*> trackPair(2);

  static const SG::Decorator< std::vector< VertexLink > > BGammaLinks( "BGammaLinks" );

  auto flaghandles = m_passFlagsToCheck.makeHandles(ctx);
  // Retrieve vertex containers
    for (SG::ReadHandle<xAOD::VertexContainer>& BVtxContainer : m_BVertexCollectionsToCheck.makeHandles(ctx)) {
       if (!BVtxContainer.isValid()) {          // replaces the CHECK
           ATH_MSG_ERROR("Failed to retrieve VertexContainer " << BVtxContainer.key());
           return StatusCode::FAILURE;          // or ATH_CHECK if you prefer the macro style
       }

       ATH_MSG_DEBUG( "Vertex Container (" << BVtxContainer.key() << ") contains " << BVtxContainer->size() << " vertices" );


    for (const xAOD::Vertex* vertex : *BVtxContainer) {
      auto &vect = BGammaLinks(*vertex) = std::vector< VertexLink >();

      bool passedHypothesis = false;
      BVertexTracks.clear();

      for (const auto &flag : flaghandles) {
        bool pass = flag(*vertex);
        if (pass) passedHypothesis = true;
      }

      if (!passedHypothesis) continue;
      xAOD::BPhysHypoHelper Bc("Bc", vertex);

      // link to Bc+ vertex
      std::vector<const xAOD::Vertex*> precedingVertices(1, vertex);

      // Collect up B-vertex tracks
      for (size_t i = 0; i < vertex->nTrackParticles(); ++i) BVertexTracks.push_back(vertex->trackParticle(i));

      // Track Selection
      // Track1 Loop
      for (xAOD::TrackParticleContainer::const_iterator tpIt1 = inputTrackParticles->begin(); tpIt1 != inputTrackParticles->end(); ++tpIt1) {
        trackPair[0] = *tpIt1;

        auto itr1 = std::find(BVertexTracks.begin(), BVertexTracks.end(), trackPair[0]);
        if (itr1 != BVertexTracks.end()) continue;

        const Trk::Perigee& trackPerigee1 = trackPair[0]->perigeeParameters();

        // Track2 Loop
        for (xAOD::TrackParticleContainer::const_iterator tpIt2 = tpIt1 + 1; tpIt2 != inputTrackParticles->end(); ++tpIt2) {
  	    trackPair[1] = *tpIt2;
          if (trackPair[0] == trackPair[1]) continue;

          auto itr2 = std::find(BVertexTracks.begin(), BVertexTracks.end(), trackPair[1]);
          if (itr2 != BVertexTracks.end()) continue;

          const Trk::Perigee& trackPerigee2 = trackPair[1]->perigeeParameters();

          // Track pair selection
          TLorentzVector e1, e2, gamma_m, BcStar;
          e1.SetPtEtaPhiM(trackPair[0]->pt(), trackPair[0]->eta(), trackPair[0]->phi(), Trk::electron);
          e2.SetPtEtaPhiM(trackPair[1]->pt(), trackPair[1]->eta(), trackPair[1]->phi(), Trk::electron);

          gamma_m = e1 + e2;
          if (gamma_m.M() > m_maxGammaMass) continue;

          TLorentzVector mu1 = Bc.refTrk(0, Trk::muon);
          TLorentzVector mu2 = Bc.refTrk(1, Trk::muon);
          TLorentzVector mu3 = Bc.refTrk(2, Trk::muon);

          BcStar = mu1 + mu2 + mu3 + e1 + e2;
          double Q = BcStar.M() - Bc.mass() - 2 * Trk::electron;
          if (Q > m_maxDeltaQ) continue;

          // Estimate starting point + cuts on compatiblity of tracks
          int sflag = 0;
          int errorcode = 0;
          Amg::Vector3D startingPoint = m_vertexEstimator->getCirclesIntersectionPoint(&trackPerigee1, &trackPerigee2, sflag, errorcode);
          if (errorcode != 0) startingPoint = Amg::Vector3D::Zero(3);

          std::vector<float> RefTrackPx, RefTrackPy, RefTrackPz, RefTrackE;
          std::vector<float> OrigTrackPx, OrigTrackPy, OrigTrackPz, OrigTrackE;


          // Do the vertex fit
          auto convVertexCandidate = m_vertexFitter->fit(ctx, trackPair, startingPoint);

          // Check for successful fit
          if (convVertexCandidate) {
            if (convVertexCandidate->chiSquared() / convVertexCandidate->numberDoF() > m_Chi2Cut) continue;

            xAOD::BPhysHelper Photon(convVertexCandidate.get());
            // set link to the parent Bc+ vertex
            Photon.setPrecedingVertices(precedingVertices, BVtxContainer.cptr());

            // Parameters at vertex
            convVertexCandidate->clearTracks();
            ElementLink<xAOD::TrackParticleContainer> newLink1;
            newLink1.setElement(*tpIt1);
            newLink1.setStorableObject(*inputTrackParticles);
            ElementLink<xAOD::TrackParticleContainer> newLink2;
            newLink2.setElement(*tpIt2);
            newLink2.setStorableObject(*inputTrackParticles);
            convVertexCandidate->addTrackAtVertex(newLink1);
            convVertexCandidate->addTrackAtVertex(newLink2);

            std::vector<Amg::Vector3D> positionList;

            //Get photon momentum 3-vector
            Amg::Vector3D momentum = m_v0Tools->V0Momentum(convVertexCandidate.get());

            TLorentzVector photon, electron1, electron2, ph;
            electron1.SetVectM( trackMomentum( *convVertexCandidate, 0 ), Trk::electron );
            electron2.SetVectM( trackMomentum( *convVertexCandidate, 1 ), Trk::electron );
            photon = electron1 + electron2;
            ph.SetXYZM(momentum.x(), momentum.y(), momentum.z(), 0.);

            // Use to keep track of which dimuon(s) gave a chi_c/b candidate
            static const SG::Accessor<std::vector<float> > RefTrackPxAcc("RefTrackPx");
            static const SG::Accessor<std::vector<float> > RefTrackPyAcc("RefTrackPy");
            static const SG::Accessor<std::vector<float> > RefTrackPzAcc("RefTrackPz");
            std::vector<float> B_Px = RefTrackPxAcc(*vertex);
            std::vector<float> B_Py = RefTrackPyAcc(*vertex);
            std::vector<float> B_Pz = RefTrackPzAcc(*vertex);

            TLorentzVector muon1, muon2, muon3;
            muon1.SetXYZM(B_Px.at(0), B_Py.at(0), B_Pz.at(0), Trk::muon);
            muon2.SetXYZM(B_Px.at(1), B_Py.at(1), B_Pz.at(1), Trk::muon);
            muon3.SetXYZM(B_Px.at(2), B_Py.at(2), B_Pz.at(2), Trk::muon);

            TLorentzVector B_m = muon1 + muon2 + muon3;

            const double deltaQ = (B_m + photon).M() - Bc.mass() - 2 * Trk::electron;
            const double mass = photon.M();

            RefTrackPx.push_back(trackMomentum(*convVertexCandidate, 0).Px());
            RefTrackPx.push_back(trackMomentum(*convVertexCandidate, 1).Px());

            RefTrackPy.push_back(trackMomentum(*convVertexCandidate, 0).Py());
            RefTrackPy.push_back(trackMomentum(*convVertexCandidate, 1).Py());

            RefTrackPz.push_back(trackMomentum(*convVertexCandidate, 0).Pz());
            RefTrackPz.push_back(trackMomentum(*convVertexCandidate, 1).Pz());

            for (size_t i = 0; i < B_Px.size(); i++) {
              RefTrackPx.push_back(B_Px.at(i));
              RefTrackPy.push_back(B_Py.at(i));
              RefTrackPz.push_back(B_Pz.at(i));
            }

            RefTrackE.push_back(electron1.E());
            RefTrackE.push_back(electron2.E());
            RefTrackE.push_back(muon1.E());
            RefTrackE.push_back(muon2.E());
            RefTrackE.push_back(muon3.E());

            OrigTrackPx.push_back(e1.Px());
            OrigTrackPx.push_back(e2.Px());

            OrigTrackPy.push_back(e1.Py());
            OrigTrackPy.push_back(e2.Py());

            OrigTrackPz.push_back(e1.Pz());
            OrigTrackPz.push_back(e2.Pz());

            OrigTrackE.push_back(e1.E());
            OrigTrackE.push_back(e2.E());


            ATH_MSG_DEBUG( "pt = " << photon.Pt() << " ph " << ph.Pt() << " mass " << photon.M() << " px size " << RefTrackPx.size() );
            ATH_MSG_DEBUG( "Candidate DeltaM = " << (B_m + photon).M() << " MeV DiMuon " << " ( Mass = " << B_m.M() << " MeV )");

            // Decorate selected conversions
            ATH_MSG_DEBUG( "Decorating conversion vertices" );

            static const SG::Accessor<float> pxAcc("px");
            static const SG::Accessor<float> pyAcc("py");
            static const SG::Accessor<float> pzAcc("pz");
            pxAcc(*convVertexCandidate) = momentum.x();
            pyAcc(*convVertexCandidate) = momentum.y();
            pzAcc(*convVertexCandidate) = momentum.z();

            static const SG::Accessor<float> deltaQAcc("deltaQ");
            static const SG::Accessor<float> gamma_massAcc("gamma_mass");
            static const SG::Accessor< std::vector<float> > RefTrackEAcc("RefTrackE");
            deltaQAcc(*convVertexCandidate) = deltaQ;
            gamma_massAcc(*convVertexCandidate) = mass;
            RefTrackPxAcc(*convVertexCandidate) = RefTrackPx;
            RefTrackPyAcc(*convVertexCandidate) = RefTrackPy;
            RefTrackPzAcc(*convVertexCandidate) = RefTrackPz;
            RefTrackEAcc(*convVertexCandidate) = RefTrackE;

            static const SG::Accessor< std::vector<float> > OrigTrackPxAcc("OrigTrackPx");
            static const SG::Accessor< std::vector<float> > OrigTrackPyAcc("OrigTrackPy");
            static const SG::Accessor< std::vector<float> > OrigTrackPzAcc("OrigTrackPz");
            static const SG::Accessor< std::vector<float> > OrigTrackEAcc("OrigTrackE");
            OrigTrackPxAcc(*convVertexCandidate) = OrigTrackPx;
            OrigTrackPyAcc(*convVertexCandidate) = OrigTrackPy;
            OrigTrackPzAcc(*convVertexCandidate) = OrigTrackPz;
            OrigTrackEAcc(*convVertexCandidate) = OrigTrackE;

            static const SG::Accessor<Char_t> passed_GammaAcc("passed_Gamma");
            passed_GammaAcc(*convVertexCandidate) = true; // Used in event skimming


            // add cross-link to the original Bc+ vertex
            VertexLink BGammaLink;
            BGammaLink.setElement(convVertexCandidate.get());
            BGammaLink.setStorableObject(*conversionContainer);
            conversionContainer->push_back( std::move(convVertexCandidate) );
            vect.push_back(std::move(BGammaLink));
          }
          else {
            ATH_MSG_DEBUG( "Vertex Fit Failed" );
          }

        }  // end of Track2 Loop
      }  // end of Track1 Loop

    } // end of Bc loop

  } // end of vertex container loop

  SG::WriteHandle<xAOD::VertexContainer> wh(m_conversionContainerName, ctx);
  ATH_CHECK( wh.record(std::move(conversionContainer), std::move(conversionAuxContainer)) );

  return StatusCode::SUCCESS;
}


// trackMomentum: returns refitted track momentum
TVector3 BPhysBGammaFinder::trackMomentum(const xAOD::Vertex &vxCandidate, int trkIndex) const {

  double px = 0.;
  double py = 0.;
  double pz = 0.;
  const Trk::TrackParameters* aPerigee = vxCandidate.vxTrackAtVertex()[trkIndex].perigeeAtVertex();
  px = aPerigee->momentum()[Trk::px];
  py = aPerigee->momentum()[Trk::py];
  pz = aPerigee->momentum()[Trk::pz];

  return TVector3(px,py,pz);
}

}  // end of namespace DerivationFramework
