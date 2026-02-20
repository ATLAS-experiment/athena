/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                         InDetV0FinderTool.cxx  -  Description
                             -------------------
    begin   : 20-07-2005
    authors : Evelina Bouhova-Thacker (Lancaster University), Rob Henderson (Lancater University)
    email   : e.bouhova@cern.ch, r.henderson@lancaster.ac.uk
    changes : December 2014
    author  : Evelina Bouhova-Thacker <e.bouhova@cern.ch> 
              Changed to use xAOD
              Changed to receive DataHandles from caller

 ***************************************************************************/

#include "InDetV0Finder/InDetV0FinderTool.h"
#include "TrkV0Fitter/TrkV0VertexFitter.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"

#include "xAODTracking/TrackingPrimitives.h"
#include "HepPDT/ParticleDataTable.hh"
#include "TruthUtils/HepMCHelpers.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include <vector>
#include <cmath>

namespace InDet
{
  
InDetV0FinderTool::InDetV0FinderTool(const std::string& t, const std::string& n, const IInterface* p)
  :
  AthAlgTool(t,n,p)
{
  declareInterface<InDetV0FinderTool>(this);
}

InDetV0FinderTool::~InDetV0FinderTool() = default;

StatusCode InDetV0FinderTool::initialize()
{
  ATH_CHECK( m_trackParticleKey.initialize() );

  // Get the right vertex fitting tool from ToolSvc
  if (m_useV0Fitter) {
    ATH_CHECK( m_iVertexFitter.retrieve() );
    ATH_MSG_DEBUG( "Retrieved tool " << m_iVertexFitter);
  } else {
    ATH_CHECK( m_iVKVertexFitter.retrieve() );
    ATH_MSG_DEBUG("Retrieved tool " << m_iVKVertexFitter);

    // Get the VKalVrt Ks vertex fitting tool from ToolSvc
    ATH_CHECK( m_iKshortFitter.retrieve() );
    ATH_MSG_DEBUG("Retrieved tool " << m_iKshortFitter);

    // Get the VKalVrt Lambda vertex fitting tool from ToolSvc
    ATH_CHECK( m_iLambdaFitter.retrieve() );
    ATH_MSG_DEBUG( "Retrieved tool " << m_iLambdaFitter);

    // Get the VKalVrt Lambdabar vertex fitting tool from ToolSvc
    ATH_CHECK( m_iLambdabarFitter.retrieve() );
    ATH_MSG_DEBUG("Retrieved tool " << m_iLambdabarFitter);
  }

  ATH_CHECK( m_iGammaFitter.retrieve() );
  ATH_MSG_DEBUG("Retrieved tool " << m_iGammaFitter);

  // get the Particle Properties Service
  ATH_CHECK(m_partPropSvc.retrieve());
  m_particleDataTable = m_partPropSvc->PDT();

  // uploading the V0 tools
  ATH_CHECK( m_V0Tools.retrieve() );
  ATH_MSG_DEBUG("Retrieved tool " << m_V0Tools);

  // Get the TrackToVertex extrapolator tool
  ATH_CHECK( m_trackToVertexTool.retrieve() );

  // Get the extrapolator
  ATH_CHECK( m_extrapolator.retrieve() );
  ATH_MSG_DEBUG("Retrieved tool ");

  // Initialize vertex container key
  ATH_CHECK( m_vertexKey.initialize() );

  m_v0LinksDecorkeyks = m_ksKey + ".V0Link";
  m_v0LinksDecorkeylb = m_laKey + ".V0Link";
  m_v0LinksDecorkeylbb = m_lbKey + ".V0Link";
  m_v0_ksLinksDecorkey = m_v0Key + ".KshortLink";
  m_v0_laLinksDecorkey = m_v0Key + ".LambdaLink";
  m_v0_lbLinksDecorkey = m_v0Key + ".LambdabarLink";
  ATH_MSG_DEBUG("m_v0_lbLinksDecorkey = " << m_v0_lbLinksDecorkey.key());
  ATH_MSG_DEBUG("m_v0_laLinksDecorkey = " << m_v0_laLinksDecorkey.key());
  ATH_MSG_DEBUG("m_v0_ksLinksDecorkey = " << m_v0_ksLinksDecorkey.key());
  ATH_MSG_DEBUG("m_v0LinksDecorkeyks = " << m_v0LinksDecorkeyks.key());
  ATH_CHECK( m_v0LinksDecorkeyks.initialize());
  ATH_CHECK( m_v0LinksDecorkeylb.initialize());
  ATH_CHECK( m_v0LinksDecorkeylbb.initialize());
  ATH_CHECK( m_v0_ksLinksDecorkey.initialize());
  ATH_CHECK( m_v0_laLinksDecorkey.initialize());
  ATH_CHECK( m_v0_lbLinksDecorkey.initialize());

  m_mDecor_gfit = m_v0Key + ".gamma_fit"; 
  m_mDecor_gmass = m_v0Key + ".gamma_mass";
  m_mDecor_gmasserr = m_v0Key + ".gamma_massError";
  m_mDecor_gprob = m_v0Key + ".gamma_probability";
  ATH_CHECK( m_mDecor_gfit.initialize());
  ATH_CHECK( m_mDecor_gmass.initialize());
  ATH_CHECK( m_mDecor_gmasserr.initialize());
  ATH_CHECK( m_mDecor_gprob.initialize());

  m_v0_BDTScore = m_v0Key + ".BDTScore";

  ATH_MSG_DEBUG("m_v0_BDTScore = " << m_v0_BDTScore.key());
  ATH_CHECK( m_v0_BDTScore.initialize());


  if (m_useBDT){
    std::string BDTPathName = "InDetV0FinderTool/BDT/v1/" + m_BDTFile;
    std::string fullPathToFile = PathResolverFindCalibFile(BDTPathName);
    ATH_MSG_DEBUG("Using BDT File: " << fullPathToFile);
    std::unique_ptr<TFile> rootFile(TFile::Open(fullPathToFile.c_str(), "READ"));
    std::string strBDTName = "xgboost";
    std::unique_ptr<TTree> training( (TTree*)rootFile->Get(strBDTName.c_str()) );
    m_BDT = std::make_unique<MVAUtils::BDT>(training.get());
  }

  ATH_CHECK( m_eventInfo_key.initialize(!m_useBeamSpotCond));
  ATH_CHECK( m_beamSpotKey.initialize( m_useBeamSpotCond));
  ATH_CHECK( m_beamSpotDecoKey.initialize(!m_useBeamSpotCond));

// Get the track selector tool from ToolSvc
  ATH_CHECK( m_trkSelector.retrieve() );
  ATH_MSG_DEBUG("Retrieved tool " << m_trkSelector);

// Get the vertex point estimator tool from ToolSvc
  ATH_CHECK( m_vertexEstimator.retrieve() );
  ATH_MSG_DEBUG("Retrieved tool " << m_vertexEstimator);

  const HepPDT::ParticleData* pd_pi = m_particleDataTable->particle(MC::PIPLUS);
  const HepPDT::ParticleData* pd_p  = m_particleDataTable->particle(MC::PROTON);
  const HepPDT::ParticleData* pd_e  = m_particleDataTable->particle(MC::ELECTRON);
  const HepPDT::ParticleData* pd_K  = m_particleDataTable->particle(MC::K0S);
  const HepPDT::ParticleData* pd_L  = m_particleDataTable->particle(MC::LAMBDA0);
  if (m_masses == 1) {
   m_masspi     = pd_pi->mass();
   m_massp      = pd_p->mass();
   m_masse      = pd_e->mass();
   m_massK0S    = pd_K->mass();
   m_massLambda = pd_L->mass();
  }

  m_events_processed = 0;
  m_V0s_stored       = 0;
  m_Kshort_stored    = 0;
  m_Lambdabar_stored = 0;
  m_Lambda_stored    = 0;
  m_Gamma_stored     = 0;

// making a concrete fitter for the V0Fitter
  m_concreteVertexFitter = nullptr;
  if (m_useV0Fitter) {
    m_concreteVertexFitter = dynamic_cast<Trk::TrkV0VertexFitter * >(&(*m_iVertexFitter));
    if (m_concreteVertexFitter == nullptr) {
      ATH_MSG_FATAL("The vertex fitter passed is not a V0 Vertex Fitter");
      return StatusCode::FAILURE;
    }
  }

  ATH_CHECK(m_RelinkContainers.initialize());

  ATH_MSG_DEBUG( "Initialization successful" );

  return StatusCode::SUCCESS;
}

StatusCode InDetV0FinderTool::performSearch(xAOD::VertexContainer* v0Container,
                                            xAOD::VertexContainer* ksContainer,
                                            xAOD::VertexContainer* laContainer,
                                            xAOD::VertexContainer* lbContainer,
                                            const xAOD::Vertex* primaryVertex,
					    const xAOD::VertexContainer* vertColl,
                                            const EventContext& ctx
					    ) const
{

  ATH_MSG_DEBUG( "InDetV0FinderTool::performSearch" );
  std::vector<const xAOD::TrackParticleContainer*> trackCols;
  for(const auto &str : m_RelinkContainers){
    SG::ReadHandle<xAOD::TrackParticleContainer> handle(str,ctx);
    trackCols.push_back(handle.cptr());
  }

  m_events_processed ++;

  //Retrieve vertices for overlap removal
  SG::ReadHandle<xAOD::VertexContainer> vertices { m_vertexKey, ctx };
  if (!vertices.isValid())
  {
    ATH_MSG_WARNING("Primary vertex container with key " << m_vertexKey.key() << " not found");
    return StatusCode::SUCCESS;
  }

// Retrieve track particles from StoreGate
  SG::ReadHandle<xAOD::TrackParticleContainer> TPC( m_trackParticleKey, ctx );
  if ( !TPC.isValid() )
  {
      ATH_MSG_ERROR("Input TrackParticle collection is invalid!");
      return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG("Track particle container size " <<  TPC->size());

  if (m_use_vertColl) {
    ATH_MSG_DEBUG("Vertex  container size " << vertColl->size());
  }
  Amg::Vector3D beamspot;
  if(m_useBeamSpotCond){
     auto beamSpotHandle = SG::ReadCondHandle(m_beamSpotKey, ctx);
     beamspot = beamSpotHandle->beamPos();
  }else{
     SG::ReadDecorHandle<xAOD::EventInfo, float> beamPosX(m_beamSpotDecoKey[0], ctx);
     SG::ReadDecorHandle<xAOD::EventInfo, float> beamPosY(m_beamSpotDecoKey[1], ctx);
     SG::ReadDecorHandle<xAOD::EventInfo, float> beamPosZ(m_beamSpotDecoKey[2], ctx);
     beamspot = Amg::Vector3D(beamPosX(0), beamPosY(0), beamPosZ(0));
  }
// track preselection
  std::vector<const xAOD::TrackParticle*> posTracks; posTracks.clear();
  std::vector<const xAOD::TrackParticle*> negTracks; negTracks.clear();
  const xAOD::Vertex* vx = nullptr;
  if (m_pv && primaryVertex) vx = primaryVertex;

  if (TPC->size() > 1) {
    xAOD::TrackParticleContainer::const_iterator tpIt;
    for ( tpIt=TPC->begin(); tpIt!=TPC->end(); ++tpIt )
    {
      const xAOD::TrackParticle* TP = (*tpIt);
      double charge = TP->charge();

      if (m_trkSelector->decision(*TP, vx) || (!m_useTrkSel))
      {
        //PV Track Overlap Selections
        const xAOD::Vertex* foundVertex { nullptr };
        if (m_useorigin)
        {
          for (const auto *const vx : *vertices)
          {
	          for (const auto& tpLink : vx->trackParticleLinks())
	          {
	            if (*tpLink == TP)
	            {
	              foundVertex = vx;
	              break;
	            }
	          }
	        if (foundVertex) break;
          }
        }
        bool useTrack = false;
        if (!m_useorigin) useTrack = true;
        if (m_useorigin && foundVertex == nullptr) useTrack = true;
        if (!useTrack) continue;


        //d0 Selections
        bool d0wrtVertex = true;
        if (m_use_vertColl) {
          if ( !d0Pass(TP,vertColl, ctx) ) d0wrtVertex = false;
        }
        if (!m_use_vertColl && m_pv) {
          if (primaryVertex) {
            if ( !d0Pass(TP,primaryVertex, ctx) ) d0wrtVertex = false;
          } else {
            if ( !d0Pass(TP,beamspot, ctx) ) d0wrtVertex = false;
          }
        }
        if (!d0wrtVertex) continue;

        if (m_samesign) {
          posTracks.push_back(TP);
          negTracks.push_back(TP);
        } else {
          if (charge > 0) {
            posTracks.push_back(*tpIt);
          } else {
            negTracks.push_back(*tpIt);
          }
        }
      }
    }
  }
  ATH_MSG_DEBUG("number of tracks passing preselection, positive " << posTracks.size() << " negative " << negTracks.size());

  if (!posTracks.empty() && !negTracks.empty())
  {


  SG::WriteDecorHandle<xAOD::VertexContainer, ElementLink<xAOD::VertexContainer>> v0LinksDecorks(m_v0LinksDecorkeyks, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, ElementLink<xAOD::VertexContainer>> v0LinksDecorlb(m_v0LinksDecorkeylb, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, ElementLink<xAOD::VertexContainer>> v0LinksDecorlbb(m_v0LinksDecorkeylbb, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, ElementLink<xAOD::VertexContainer>> v0_ksLinksDecor(m_v0_ksLinksDecorkey, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, ElementLink<xAOD::VertexContainer>> v0_laLinksDecor(m_v0_laLinksDecorkey, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, ElementLink<xAOD::VertexContainer>> v0_lbLinksDecor(m_v0_lbLinksDecorkey, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, int> mDecor_gfit(m_mDecor_gfit, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, float> mDecor_gmass(m_mDecor_gmass, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, float> mDecor_gmasserr(m_mDecor_gmasserr, ctx);
  SG::WriteDecorHandle<xAOD::VertexContainer, float> mDecor_gprob(m_mDecor_gprob, ctx);

  SG::WriteDecorHandle<xAOD::VertexContainer, float> v0_BDTScore(m_v0_BDTScore, ctx);

  std::vector<const xAOD::TrackParticle*>::const_iterator tpIt1;
  std::vector<const xAOD::TrackParticle*>::const_iterator tpIt2;
  unsigned int i1 = 0;
  for (tpIt1 = posTracks.begin(); tpIt1 != posTracks.end(); ++tpIt1)
  {
    const xAOD::TrackParticle* TP1 = (*tpIt1);
    uint8_t temp1(0);
    uint8_t nclus1(0);
    if ( TP1->summaryValue( temp1 , xAOD::numberOfPixelHits) ) nclus1 += temp1;
    if ( TP1->summaryValue( temp1 , xAOD::numberOfSCTHits)   ) nclus1 += temp1; 
    double pt1 = TP1->pt();

    unsigned int i2 = 0;
    for (tpIt2 = negTracks.begin(); tpIt2 != negTracks.end(); ++tpIt2)
    {
      if (*tpIt1 == *tpIt2) continue;
      if (m_samesign && i1 <= i2) continue;

      const xAOD::TrackParticle* TP2 = (*tpIt2);
      uint8_t temp2(0);
      uint8_t nclus2(0);
      if ( TP2->summaryValue( temp2 , xAOD::numberOfPixelHits) ) nclus2 += temp2;
      if ( TP2->summaryValue( temp2 , xAOD::numberOfSCTHits)   ) nclus2 += temp2; 
      ATH_MSG_DEBUG("nclus1 " << (int)nclus1 << " nclus2 " << (int)nclus2);

      if (!m_useTRTplusTRT && nclus1 == 0 && nclus2 == 0) continue;
      if (!m_useTRTplusSi && (nclus1 == 0 || nclus2 == 0)) continue;

      double pt2 = TP2->pt();

      bool trk_cut1 = false;
      bool trk_cut2 = false;
      if (nclus1 != 0) trk_cut1 = true;
      if (nclus1 == 0 && pt1 >= m_ptTRT) trk_cut1 = true;
      if (!trk_cut1) continue;      
      if (nclus2 != 0) trk_cut2 = true;
      if (nclus2 == 0 && pt2 >= m_ptTRT) trk_cut2 = true;
      if (!trk_cut2) continue;      

// find a starting point
      const Trk::Perigee& aPerigee1 = TP1->perigeeParameters();
      const Trk::Perigee& aPerigee2 = TP2->perigeeParameters();
      int sflag = 0;
      int errorcode = 0;
      Amg::Vector3D startingPoint = m_vertexEstimator->getCirclesIntersectionPoint(&aPerigee1,&aPerigee2,sflag,errorcode);
      if (errorcode != 0) {startingPoint(0) = 0.0; startingPoint(1) = 0.0; startingPoint(2) = 0.0;}
      bool errorCode = false;
      if (errorcode == 0 || errorcode == 5 || errorcode == 6 || errorcode == 8) errorCode = true;
      if (!errorCode) continue;

      //re-do d0 selection if using m_use_vertColl to ensure both tracks pass for the SAME vertex
      bool d0wrtVertex = true;
      if (m_use_vertColl) {
        if ( !d0Pass(TP1,TP2,vertColl, ctx) ) d0wrtVertex = false;
      }
      if (!d0wrtVertex) continue;


// pair pre-selection cuts
          if ( doFit(TP1,TP2,startingPoint, ctx) )
          {
            std::vector<const xAOD::TrackParticle*> pairV0;
            pairV0.clear();
            pairV0.push_back(TP1);
            pairV0.push_back(TP2);

// vertex fit
            ATH_MSG_DEBUG("unconstrained fit now");

            std::unique_ptr<xAOD::Vertex> myVxCandidate;
            if (m_useV0Fitter) {
              myVxCandidate = std::unique_ptr<xAOD::Vertex>( m_concreteVertexFitter->fit(pairV0, startingPoint) );
            } else {
              myVxCandidate = std::unique_ptr<xAOD::Vertex>(  m_iVKVertexFitter->fit(pairV0, startingPoint) );
            }
      
              if (myVxCandidate)
              {
                myVxCandidate->setVertexType(xAOD::VxType::V0Vtx);
                if ( (m_V0Tools->vertexProbability(myVxCandidate.get()) >= m_minVertProb) || (m_useBDT) ) // If using BDT selections, it makes VertProb Selections
                {
                  bool doKshortFit = false;
                  doKshortFit = doMassFit(myVxCandidate.get(),310);
                  bool doLambdaFit = false;
                  doLambdaFit = doMassFit(myVxCandidate.get(),3122);
                  bool doLambdabarFit = false;
                  doLambdabarFit = doMassFit(myVxCandidate.get(),-3122);
                  float score = 0; // BDT score variable
                  if (doKshortFit || doLambdaFit || doLambdabarFit)
                  {
                    bool pointAtVert = true;
                    if (m_use_vertColl) {
                      if ( !pointAtVertexColl(myVxCandidate.get(),vertColl,score) ) pointAtVert = false;
                    }
                    if (!m_use_vertColl && m_pv && primaryVertex) {
                      if ( !pointAtVertex(myVxCandidate.get(),primaryVertex,score) ) pointAtVert = false;
                    }
                    if (m_doSimpleV0) pointAtVert = true;
                    if (pointAtVert)
                    {
                      Amg::Vector3D vertex = m_V0Tools->vtx(myVxCandidate.get());

                      std::unique_ptr<xAOD::Vertex> myKshort;
                      std::unique_ptr<xAOD::Vertex> myLambda;
                      std::unique_ptr<xAOD::Vertex> myLambdabar;
		      std::unique_ptr<xAOD::Vertex> myGamma;
                      bool foundKshort = false;
                      bool foundLambda = false;
                      bool foundLambdabar = false;

                      if (doKshortFit && !m_doSimpleV0) {
                        myKshort = std::unique_ptr<xAOD::Vertex>( massFit(310, pairV0, vertex) );
                        if (myKshort) {
                          if (m_V0Tools->vertexProbability(myKshort.get()) >= m_minConstrVertProb) {
                            myKshort->setVertexType(xAOD::VxType::V0Vtx);
                            foundKshort = true;
                          }
                        }
                      }

                      if (doLambdaFit && !m_doSimpleV0) {
                        myLambda = std::unique_ptr<xAOD::Vertex>( massFit(3122, pairV0, vertex) );
                        if (myLambda) {
                          if (m_V0Tools->vertexProbability(myLambda.get()) >= m_minConstrVertProb) {
                            myLambda->setVertexType(xAOD::VxType::V0Vtx);
                            foundLambda = true;
                          }
                        }
                      }

                      if (doLambdabarFit && !m_doSimpleV0) {
                        myLambdabar = std::unique_ptr<xAOD::Vertex>( massFit(-3122, pairV0, vertex));
                        if (myLambdabar) {
                          if (m_V0Tools->vertexProbability(myLambdabar.get()) >= m_minConstrVertProb) {
                            myLambdabar->setVertexType(xAOD::VxType::V0Vtx);
                            foundLambdabar = true;
                          }
                        }
                      }

                      bool doGamma = false;
                      int gamma_fit = 0;
                      double gamma_prob = -1., gamma_mass = -1., gamma_massErr = -1.;
                      if (foundKshort || foundLambda || foundLambdabar) doGamma = true;

                      ElementLink<xAOD::VertexContainer> v0Link;
                      ElementLink<xAOD::VertexContainer> ksLink;
                      ElementLink<xAOD::VertexContainer> laLink;
                      ElementLink<xAOD::VertexContainer> lbLink;

                      if (m_doSimpleV0 || (!m_doSimpleV0 && doGamma)) {
                        m_V0s_stored++;
                        myVxCandidate->clearTracks();
                        ElementLink<xAOD::TrackParticleContainer> newLink1 = makeLink(*tpIt1, trackCols);
                        ElementLink<xAOD::TrackParticleContainer> newLink2 = makeLink(*tpIt2, trackCols);
                        myVxCandidate->addTrackAtVertex(newLink1);
                        myVxCandidate->addTrackAtVertex(newLink2);
                        v0Container->push_back(myVxCandidate.release());

                        v0_BDTScore( *(v0Container->back()) ) = score;// decorate w/ bdt score

                        if (foundKshort && !m_doSimpleV0) {
                          m_Kshort_stored++;
                          myKshort->clearTracks();
                          ElementLink<xAOD::TrackParticleContainer> ksLink1 = makeLink(*tpIt1, trackCols);
                          ElementLink<xAOD::TrackParticleContainer> ksLink2 = makeLink(*tpIt2, trackCols);
                          myKshort->addTrackAtVertex(ksLink1);
                          myKshort->addTrackAtVertex(ksLink2);
                          ksContainer->push_back(myKshort.release());

                          v0Link.setElement(v0Container->back());
                          v0Link.setStorableObject(*v0Container);
                          v0LinksDecorks(*(ksContainer->back())) = v0Link;

                          ksLink.setElement(ksContainer->back());
                          ksLink.setStorableObject(*ksContainer);
                          v0_ksLinksDecor(*(v0Container->back())) = ksLink;
                        } else {
                          v0_ksLinksDecor(*(v0Container->back())) = ksLink;
                        }
                        if (foundLambda && !m_doSimpleV0) {
                          m_Lambda_stored++;
                          myLambda->clearTracks();
                          ElementLink<xAOD::TrackParticleContainer> laLink1 = makeLink(*tpIt1, trackCols);
                          ElementLink<xAOD::TrackParticleContainer> laLink2 = makeLink(*tpIt2, trackCols);
                          myLambda->addTrackAtVertex(laLink1);
                          myLambda->addTrackAtVertex(laLink2);
                          laContainer->push_back(myLambda.release());

                          v0Link.setElement(v0Container->back());
                          v0Link.setStorableObject(*v0Container);
                          v0LinksDecorlb(*(laContainer->back())) = v0Link;

                          laLink.setElement(laContainer->back());
                          laLink.setStorableObject(*laContainer);
                          v0_laLinksDecor(*(v0Container->back())) = laLink;
                        } else {
                          v0_laLinksDecor(*(v0Container->back())) = laLink;
                        }
                        if (foundLambdabar && !m_doSimpleV0) {
                          m_Lambdabar_stored++;
                          myLambdabar->clearTracks();
                          ElementLink<xAOD::TrackParticleContainer> lbLink1 = makeLink(*tpIt1, trackCols);
                          ElementLink<xAOD::TrackParticleContainer> lbLink2 = makeLink(*tpIt2, trackCols);
                          myLambdabar->addTrackAtVertex(lbLink1);
                          myLambdabar->addTrackAtVertex(lbLink2);
                          lbContainer->push_back(myLambdabar.release());

                          v0Link.setElement(v0Container->back());
                          v0Link.setStorableObject(*v0Container);
                          v0LinksDecorlbb(*(lbContainer->back())) = v0Link;

                          lbLink.setElement(lbContainer->back());
                          lbLink.setStorableObject(*lbContainer);
                          v0_lbLinksDecor(*(v0Container->back())) = lbLink;
                        } else {
                          v0_lbLinksDecor(*(v0Container->back())) = lbLink;
                        }
                        if (doGamma && !m_doSimpleV0) {
		          myGamma = std::unique_ptr<xAOD::Vertex>( massFit(22, pairV0, vertex) );
                          if (myGamma && m_V0Tools->vertexProbability(myGamma.get()) >= m_minConstrVertProb) {
                            gamma_fit = 1;
                            gamma_prob = m_V0Tools->vertexProbability(myGamma.get());
                            gamma_mass = m_V0Tools->invariantMass(myGamma.get(),m_masse,m_masse);
                            gamma_massErr = m_V0Tools->invariantMassError(myGamma.get(),m_masse,m_masse);
                          }

                          mDecor_gfit( *(v0Container->back()) ) = gamma_fit;
                          mDecor_gmass( *(v0Container->back()) ) = gamma_mass;
                          mDecor_gmasserr( *(v0Container->back()) ) = gamma_massErr;
                          mDecor_gprob( *(v0Container->back()) ) = gamma_prob;
                        }
                      }

                    } // pointAtVert

                  } // in mass window (doMassFit)

                } // chi2 cut failed
              } else { // unconstrained fit failed
                ATH_MSG_DEBUG("Fitter failed!");
              }

          }  // doFit

      i2++;
    }  // loop over negative tracks
    i1++;
  }  // loop over positive tracks

  }  // posTracks.size() > 0 && negTracks.size() > 0

  if (v0Container->empty()) ATH_MSG_DEBUG("No Candidates found. Empty container returned");
  if (ksContainer->empty()) ATH_MSG_DEBUG("No Kshort Candidates found. Empty container returned");
  if (laContainer->empty()) ATH_MSG_DEBUG("No Lambda Candidates found. Empty container returned");
  if (lbContainer->empty()) ATH_MSG_DEBUG("No Lambdabar Candidates found. Empty container returned");

  return StatusCode::SUCCESS;
}

StatusCode InDetV0FinderTool::finalize()
{
  msg(MSG::DEBUG)
    << "----------------------------------------------------------------------------------------------------------------------------------------------" << endmsg
    << "\tSummary" << endmsg 
    << "\tProcessed              : " << m_events_processed            << " events" << endmsg
    << "\tStored                 : " << m_V0s_stored                  << " V0s" << endmsg
    << "\tof which               : " << m_Kshort_stored               << " Kshorts" << endmsg
    << "\t                       : " << m_Lambda_stored               << " Lambdas" << endmsg
    << "\t                       : " << m_Lambdabar_stored            << " Lambdabars" << endmsg;
  msg(MSG::DEBUG) << "----------------------------------------------------------------------------------------------------------------------------------------------" << endmsg;

  return StatusCode::SUCCESS;
}

void InDetV0FinderTool::SGError(const std::string& errService) const
{
  ATH_MSG_FATAL(errService << " not found. Exiting !");
}


bool InDetV0FinderTool::doFit(const xAOD::TrackParticle* track1, const xAOD::TrackParticle* track2,
                                 Amg::Vector3D &startingPoint, const EventContext& ctx) const
{
  bool pass = false;
  double srxy = startingPoint.perp();
  if (srxy <= m_maxsxy)
  {
    double massKshort_i=2000001., massLambda_i=2000001., massLambdabar_i=2000001.;
    const Amg::Vector3D& globalPosition = startingPoint;
    Trk::PerigeeSurface perigeeSurface(globalPosition);
    std::vector<std::unique_ptr<const Trk::TrackParameters> >  cleanup;
    const Trk::TrackParameters* extrapolatedPerigee1(nullptr);
    const Trk::TrackParameters* extrapolatedPerigee2(nullptr);
    extrapolatedPerigee1 = m_extrapolator->extrapolate(ctx,track1->perigeeParameters(), perigeeSurface).release();
    if (extrapolatedPerigee1 == nullptr) extrapolatedPerigee1 = &track1->perigeeParameters();
    else cleanup.push_back(std::unique_ptr<const Trk::TrackParameters>(extrapolatedPerigee1));

    extrapolatedPerigee2 = m_extrapolator->extrapolate(ctx,track2->perigeeParameters(), perigeeSurface).release();
    if (extrapolatedPerigee2 == nullptr) extrapolatedPerigee2 = &track2->perigeeParameters();
    else cleanup.push_back(std::unique_ptr<const Trk::TrackParameters>(extrapolatedPerigee2));

    if (extrapolatedPerigee1 != nullptr && extrapolatedPerigee2 != nullptr) {
      massKshort_i = invariantMass(extrapolatedPerigee1,extrapolatedPerigee2,m_masspi,m_masspi);
      massLambda_i = invariantMass(extrapolatedPerigee1,extrapolatedPerigee2,m_massp,m_masspi);
      massLambdabar_i = invariantMass(extrapolatedPerigee1,extrapolatedPerigee2,m_masspi,m_massp);
      if ( ((massKshort_i >= m_uksmin && massKshort_i <= m_uksmax) ||
            (massLambda_i >= m_ulamin && massLambda_i <= m_ulamax) ||
            (massLambdabar_i >= m_ulamin && massLambdabar_i <= m_ulamax)) ) pass = true;
    }
  }

  return pass;
}

bool InDetV0FinderTool::d0Pass(const xAOD::TrackParticle* track1, const xAOD::TrackParticle* track2, const xAOD::VertexContainer * vertColl, const EventContext& ctx) const
{
  bool pass = false;
  int count = 0;
  bool hasInnerPixHit1 = true;
  bool hasInnerPixHit2 = true;
  if (m_use_innerPixHits){
    SG::AuxElement::ConstAccessor<uint8_t> numberOfInnermostPixelLayerHits("numberOfInnermostPixelLayerHits");
    uint8_t  nInnerHits1 = numberOfInnermostPixelLayerHits(*track1);
    if (nInnerHits1 == 0) hasInnerPixHit1 = false;
    uint8_t  nInnerHits2 = numberOfInnermostPixelLayerHits(*track2);
    if (nInnerHits2 == 0) hasInnerPixHit2 = false;
  }
  for (auto vItr=vertColl->begin(); vItr!=vertColl->end(); ++vItr )
  {
    const xAOD::Vertex* PV = (*vItr);
    auto per1 = m_trackToVertexTool->perigeeAtVertex(ctx, *track1, PV->position() );
    if (per1 == nullptr) continue;
    auto per2 = m_trackToVertexTool->perigeeAtVertex(ctx, *track2, PV->position() );
    if (per2 == nullptr) continue;
    double d0_1 = per1->parameters()[Trk::d0];
    double sig_d0_1 = sqrt((*per1->covariance())(0,0));
    double delta_z0_1 = track1->z0() + track1->vz() - PV->z();
    double d0_2 = per2->parameters()[Trk::d0];
    double sig_d0_2 = sqrt((*per2->covariance())(0,0));
    double delta_z0_2 = track2->z0() + track2->vz() - PV->z();
    bool IP_check1 = (std::abs(d0_1/sig_d0_1) > m_d0_cut) || !hasInnerPixHit1;
    IP_check1 &= std::abs(d0_1) < m_max_d0_cut;
    IP_check1 &= std::abs(delta_z0_1) < m_max_z0_cut;
    bool IP_check2 = (std::abs(d0_2/sig_d0_2) > m_d0_cut) || !hasInnerPixHit2;
    IP_check2 &= std::abs(d0_2) < m_max_d0_cut;
    IP_check2 &= std::abs(delta_z0_2) < m_max_z0_cut;
    if (IP_check1 && IP_check2) return true;
    if (++count >= m_maxPV) break;
  }
  return pass;
}

bool InDetV0FinderTool::d0Pass(const xAOD::TrackParticle* track1, const xAOD::VertexContainer * vertColl, const EventContext& ctx) const
{
  bool pass = false;
  int count = 0;
  bool hasInnerPixHit1 = true;
  if (m_use_innerPixHits){
    SG::AuxElement::ConstAccessor<uint8_t> numberOfInnermostPixelLayerHits("numberOfInnermostPixelLayerHits");
    uint8_t  nInnerHits1 = numberOfInnermostPixelLayerHits(*track1);
    if (nInnerHits1 == 0) hasInnerPixHit1 = false;
  }
  for (auto vItr=vertColl->begin(); vItr!=vertColl->end(); ++vItr )
  {
    const xAOD::Vertex* PV = (*vItr);
    auto per1 = m_trackToVertexTool->perigeeAtVertex(ctx, *track1, PV->position() );
    if (per1 == nullptr) continue;
    double d0_1 = per1->parameters()[Trk::d0];
    double sig_d0_1 = sqrt((*per1->covariance())(0,0));
    double delta_z0_1 = track1->z0() + track1->vz() - PV->z();
    if (((std::abs(d0_1/sig_d0_1) > m_d0_cut) ||
         (!hasInnerPixHit1)) &&
        (std::abs(d0_1) < m_max_d0_cut) &&
        (std::abs(delta_z0_1) < m_max_z0_cut)) return true;
    if (++count >= m_maxPV) break;
  }
  return pass;
}

bool InDetV0FinderTool::d0Pass(const xAOD::TrackParticle* track1, const xAOD::Vertex* PV, const EventContext& ctx) const
{
  bool pass = false;
  bool hasInnerPixHit1 = true;
  if (m_use_innerPixHits){
    SG::AuxElement::ConstAccessor<uint8_t> numberOfInnermostPixelLayerHits("numberOfInnermostPixelLayerHits");
    uint8_t  nInnerHits1 = numberOfInnermostPixelLayerHits(*track1);
    if (nInnerHits1 == 0) hasInnerPixHit1 = false;
  }
  auto per1 = m_trackToVertexTool->perigeeAtVertex(ctx, *track1, PV->position() );
  if (per1 == nullptr) return pass;
  double d0_1 = per1->parameters()[Trk::d0];
  double sig_d0_1 = sqrt((*per1->covariance())(0,0));
  double delta_z0_1 = track1->z0() + track1->vz() - PV->z();
  if (((std::abs(d0_1/sig_d0_1) > m_d0_cut) ||
       (!hasInnerPixHit1)) && 
      (std::abs(d0_1) < m_max_d0_cut) &&
      (std::abs(delta_z0_1) < m_max_z0_cut)) pass = true;
  return pass;
}

bool InDetV0FinderTool::d0Pass(const xAOD::TrackParticle* track1, const Amg::Vector3D& PV, const EventContext& ctx) const
{
  bool pass = false;
  bool hasInnerPixHit1 = true;
  if (m_use_innerPixHits){
    SG::AuxElement::ConstAccessor<uint8_t> numberOfInnermostPixelLayerHits("numberOfInnermostPixelLayerHits");
    uint8_t  nInnerHits1 = numberOfInnermostPixelLayerHits(*track1);
    if (nInnerHits1 == 0) hasInnerPixHit1 = false;
  }
  auto per1 = m_trackToVertexTool->perigeeAtVertex(ctx, *track1, PV );
  if (per1 == nullptr) return pass;
  double d0_1 = per1->parameters()[Trk::d0];
  double sig_d0_1 = sqrt((*per1->covariance())(0,0));
  double delta_z0_1 = track1->z0() + track1->vz() - PV.z();
  if (((std::abs(d0_1/sig_d0_1) > m_d0_cut) ||
       (!hasInnerPixHit1)) && 
      (std::abs(d0_1) < m_max_d0_cut) &&
      (std::abs(delta_z0_1) < m_max_z0_cut)) pass = true;
  return pass;
}

bool InDetV0FinderTool::pointAtVertex(const xAOD::Vertex* v0, const xAOD::Vertex* PV,  float &score) const
{
  bool pass = false;
  float v0lxy = m_V0Tools->lxy(v0,PV);
  float v0lxyError = m_V0Tools->lxyError(v0,PV);
  float cos = m_V0Tools->cosTheta(v0,PV);
  float v0a0xy = m_V0Tools->a0xy(v0,PV);
  float v0a0z = m_V0Tools->a0z(v0,PV);
  if (m_useBDT){
    float prob = m_V0Tools->vertexProbability(v0);
    float nLogProb = 999999;
    if (prob>0) nLogProb = -1*log10f(prob); //bdt model uses the log, not the raw value
    std::vector<float> bdt_vars = {
                                    nLogProb,
                                    std::abs(v0a0xy),
                                    std::abs(v0a0z),
                                    v0lxy,
                                    v0lxy/v0lxyError,
                                    cos};
      float this_Score=m_BDT->GetClassification(bdt_vars);
      if (this_Score > score) {
        score = this_Score;
      }
      if (score > m_BDTCut){
        pass = true;
      }
  }
  else if (v0lxy/v0lxyError > m_vert_lxy_sig && cos > m_vert_cos_cut &&
      std::abs(v0a0xy) < m_vert_a0xy_cut && std::abs(v0a0z) < m_vert_a0z_cut &&
      v0lxy < m_vert_lxy_cut) pass = true;
  return pass;
}

bool InDetV0FinderTool::pointAtVertexColl(xAOD::Vertex* v0, const xAOD::VertexContainer * vertColl,  float &score) const
{
  bool pass = false;
  xAOD::VertexContainer::const_iterator vItr = vertColl->begin();
  for ( vItr=vertColl->begin(); vItr!=vertColl->end(); ++vItr ) { if (pointAtVertex(v0,(*vItr),score)) pass = true; }
  return pass;
}

double InDetV0FinderTool::invariantMass(const Trk::TrackParameters* per1, const Trk::TrackParameters* per2, double m1, double m2) 
{
  double e1sq = per1->momentum().mag2() + m1*m1;
  double e1 = (e1sq>0.) ? sqrt(e1sq) : 0.;
  double e2sq = per2->momentum().mag2() + m2*m2;
  double e2 = (e2sq>0.) ? sqrt(e2sq) : 0.;
  double p = (per1->momentum()+per2->momentum()).mag();
  double msq = (e1+e2+p)*(e1+e2-p);
  double mass = (msq>0.) ? sqrt(msq) : 0.;
  return mass;
}

bool InDetV0FinderTool::doMassFit(xAOD::Vertex* vxCandidate, int pdgID) const
{
  bool pass = false;
  double mass = 1000000000.;
  double error = 1000000001.;
  bool in_mass_window = false;
  double winmass_min = 0., winmass_max = 0.;

  if (pdgID == 310) {
    winmass_min = m_ksmin;
    winmass_max = m_ksmax;
    mass = m_V0Tools->invariantMass(vxCandidate,m_masspi,m_masspi);
    error = m_V0Tools->invariantMassError(vxCandidate,m_masspi,m_masspi);
    if (mass >= winmass_min && mass <= winmass_max && error <= m_errmass) in_mass_window = true;
  } else if (pdgID == 3122 || pdgID == -3122) {
    winmass_min = m_lamin;
    winmass_max = m_lamax;
    if (pdgID == 3122) {
      mass = m_V0Tools->invariantMass(vxCandidate,m_massp,m_masspi);
      error = m_V0Tools->invariantMassError(vxCandidate,m_massp,m_masspi);
    } else if (pdgID == -3122) {
      mass = m_V0Tools->invariantMass(vxCandidate,m_masspi,m_massp);
      error = m_V0Tools->invariantMassError(vxCandidate,m_masspi,m_massp);
    }
    if (mass >= winmass_min && mass <= winmass_max && error <= m_errmass) in_mass_window = true;
  }
  if (in_mass_window) pass = true;

  return pass;
}

xAOD::Vertex* InDetV0FinderTool::massFit(int pdgID, const std::vector<const xAOD::TrackParticle*> &pairV0, const Amg::Vector3D &vertex) const
{
  xAOD::Vertex* vxCandidate(nullptr);
  std::vector<double> masses;
  if (pdgID == 310) {
    masses.push_back(m_masspi);
    masses.push_back(m_masspi);
  } else if (pdgID == 3122) {
    masses.push_back(m_massp);
    masses.push_back(m_masspi);
  } else if (pdgID == -3122) {
    masses.push_back(m_masspi);
    masses.push_back(m_massp);
  } else if (pdgID == 22) {
    masses.push_back(m_masse);
    masses.push_back(m_masse);
  }

  if (pdgID == 22) {
    vxCandidate = m_iGammaFitter->fit(pairV0, vertex);
  }
  if (pdgID ==   310) {
    if (m_useV0Fitter) {
      vxCandidate = m_concreteVertexFitter->fit(pairV0, masses, m_massK0S, nullptr, vertex);
    } else {
      vxCandidate = m_iKshortFitter->fit(pairV0, vertex);
    }
  }
  if (pdgID ==  3122) {
    if (m_useV0Fitter) {
      vxCandidate = m_concreteVertexFitter->fit(pairV0, masses, m_massLambda, nullptr, vertex);
    } else {
      vxCandidate = m_iLambdaFitter->fit(pairV0, vertex);
    }
  }
  if (pdgID == -3122) {
    if (m_useV0Fitter) {
      vxCandidate = m_concreteVertexFitter->fit(pairV0, masses, m_massLambda, nullptr, vertex);
    } else {
      vxCandidate = m_iLambdabarFitter->fit(pairV0, vertex);
    }
  }

  return vxCandidate;
}

ElementLink<xAOD::TrackParticleContainer> InDetV0FinderTool::makeLink(const xAOD::TrackParticle* tp,
          const std::vector<const xAOD::TrackParticleContainer*>& trackcols) const 
{
    ElementLink<xAOD::TrackParticleContainer> Link;
    Link.setElement(tp);
    bool elementSet = false;
    if(trackcols.empty()){
       Link.setStorableObject( *dynamic_cast<const xAOD::TrackParticleContainer*>( tp->container()  ) );
       elementSet = true;
    } else {
      for(const xAOD::TrackParticleContainer* trkcol : trackcols){
          auto itr = std::find(trkcol->begin(), trkcol->end(), tp);
          if(itr != trkcol->end()){
            Link.setStorableObject(*trkcol, true);
            elementSet = true;
            break;
          }
      }
    }
    if(!elementSet) ATH_MSG_ERROR("Track was not found when linking");
    return Link;
}


}//end of namespace InDet

