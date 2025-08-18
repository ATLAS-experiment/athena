/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
  InDetConversionFinderTools.cxx  -  Description
  -------------------
  begin   : 28-08-2006
  authors : Tatjana Lenz
  email   : tatjana.lenz@cern.ch
  changes :
***************************************************************************/
#include "InDetConversionFinderTools/InDetConversionFinderTools.h"

#include "AthLinks/ElementLink.h"
#include "TrkTrack/LinkToTrack.h"
#include "AthContainers/Accessor.h"

#include <vector>
#include <utility> 

namespace InDet
{

InDetConversionFinderTools::InDetConversionFinderTools(const std::string& t,
                                                       const std::string& n,
                                                       const IInterface* p)
  : AthAlgTool(t, n, p)
{
  declareInterface<IVertexFinder>(this);
}
  
  InDetConversionFinderTools::~InDetConversionFinderTools()= default;
  
  StatusCode InDetConversionFinderTools::initialize()
  {
    StatusCode sc = AthAlgTool::initialize();
    if (sc.isFailure()) {
      ATH_MSG_FATAL("Unable to initialize InDetConversionFinderTools");
      return StatusCode::FAILURE;
    }
    /* Get the right vertex fitting tool from ToolSvc */
    if (m_iVertexFitter.retrieve().isFailure()) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_iVertexFitter);
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Retrieved tool " << m_iVertexFitter);

    /* Get the track pairs selector tool from ToolSvc */
    if (m_trackPairsSelector.retrieve().isFailure()) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_trackPairsSelector);
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Retrieved tool " << m_trackPairsSelector);

    /* Get the vertex point estimator tool from ToolSvc */
    if (m_vertexEstimator.retrieve().isFailure()) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_vertexEstimator);
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Retrieved tool " << m_vertexEstimator);

    /* Get the postselector tool from ToolSvc */
    if (m_postSelector.retrieve().isFailure()) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_postSelector);
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Retrieved tool " << m_postSelector);

    /* Get the single track conversion tool from ToolSvc */
    if (m_singleTrkConvTool.retrieve().isFailure()) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_singleTrkConvTool);
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Retrieved tool " << m_singleTrkConvTool);

    /* Get the track selector tool from ToolSvc */
    if (m_trkSelector.retrieve().isFailure()) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_trkSelector);
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Retrieved tool " << m_trkSelector);

    ATH_MSG_DEBUG("Initialization successful");
    return StatusCode::SUCCESS;
  }
  
  StatusCode InDetConversionFinderTools::finalize()
  {
    return StatusCode::SUCCESS;
  }

  //TrackCollection
  std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*>
  InDetConversionFinderTools::findVertex(const EventContext& /*ctx*/,
                                         const TrackCollection* /*trk_coll*/) const
  {
    
    ATH_MSG_ERROR("Using Track Container not currently supported returning an empty conatiner");

    // Make collection for conversions.
    xAOD::VertexContainer* InDetConversionContainer = new xAOD::VertexContainer();
    xAOD::VertexAuxContainer* InDetConversionContainerAux = new xAOD::VertexAuxContainer();
    InDetConversionContainer->setStore( InDetConversionContainerAux ); 

    return std::make_pair(InDetConversionContainer,InDetConversionContainerAux); 
  }

  // TrackParticle Collection
  std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*>
  InDetConversionFinderTools::findVertex(const EventContext& ctx, 
                                         const xAOD::TrackParticleContainer* trk_coll) const
  {
    // Make collection for conversions.
    xAOD::VertexContainer* InDetConversionContainer =
      new xAOD::VertexContainer();
    xAOD::VertexAuxContainer* InDetConversionContainerAux =
      new xAOD::VertexAuxContainer();
    InDetConversionContainer->setStore(InDetConversionContainerAux);
    // Count for number of successful conversions
    int numConversions = 0;

    // have to be used for the vertex fit
    Amg::Vector3D pos(0., 0., 0.);
    Amg::Vector3D initPos(0., 0., 0.);

    // Make seperate lists of positive and negative tracks (after presection
    // cuts)
    std::vector<const xAOD::TrackParticle*> negSelectedTracks;
    negSelectedTracks.clear();
    std::vector<const xAOD::TrackParticle*> posSelectedTracks;
    posSelectedTracks.clear();
    std::vector<int> negIndx;
    std::vector<int> posIndx;

    TrackPairsSelector::Cache cache{};
    // track preselection -->pt-cut
    xAOD::TrackParticleContainer::const_iterator iter;
    for (iter = (*trk_coll).begin(); iter != (*trk_coll).end(); ++iter) {
      if (m_trkSelector->decision(
            **iter,
            nullptr)) { // Only save if track passes the pt, d0, z0 and TRT PID cuts
        ATH_MSG_DEBUG("Track passed preselection");
        if ((*iter)->charge() < 0) {
          negSelectedTracks.push_back(*iter);
          negIndx.push_back(0);
        } else {
          posSelectedTracks.push_back(*iter);
          posIndx.push_back(0);
        }
      } else
        ATH_MSG_DEBUG("Track failed preselection");
    } // end pt,d0.z0-cuts

    // Make track pairs. To be used for double leg conversions
    std::vector<const xAOD::TrackParticle*>::const_iterator iter_pos;
    std::vector<const xAOD::TrackParticle*>::const_iterator iter_neg;
    std::vector<Amg::Vector3D> positionList;
    positionList.clear();
    std::vector<const xAOD::TrackParticle*> trackParticleList;
    trackParticleList.clear();
    std::vector<const xAOD::TrackParticle*> singleTrackConvList;
    singleTrackConvList.clear();
    int ipos = -1;
    int ineg = -1;
    // Outer loop: Loop over positive tracks
    for (iter_pos = posSelectedTracks.begin();
         iter_pos != posSelectedTracks.end();
         ++iter_pos) {
      ineg = -1;
      ipos++;
      // Inner loop: Loop over negative tracks
      for (iter_neg = negSelectedTracks.begin();
           iter_neg != negSelectedTracks.end();
           ++iter_neg) {
        ineg++;
        int flag = 0;

        std::map<std::string, float> intersectionDecors;
        if (!passPreSelection(cache,
                              *iter_pos,
                              *iter_neg,
                              positionList,
                              initPos,
                              flag,
                              intersectionDecors)) {
          positionList.clear();
          continue;
        }

        // Do the fit
        if (positionList.size() < 2) {
          ATH_MSG_DEBUG("No tracks to fit ");
          positionList.clear();
          continue;
        }

        trackParticleList.push_back(*iter_pos);
        trackParticleList.push_back(*iter_neg);

        std::unique_ptr<xAOD::Vertex> myVertex =
          m_iVertexFitter->fit(ctx, trackParticleList, initPos);
        trackParticleList.clear();

        // We have a new vertex
        if (myVertex) {
          ATH_MSG_DEBUG("VertexFit successful!");
          int type = -1;
          if ((m_isConversion && m_postSelector->selectConversionCandidate(
                                   myVertex.get(), flag, positionList)) ||
              (!m_isConversion &&
               m_postSelector->selectSecVtxCandidate(
                 myVertex.get(), flag, positionList, type))) {

            ATH_MSG_DEBUG(" Conversion passed postselection cuts");
            // Remove old element links
            myVertex->clearTracks();

            // If we do not have a valid type just reset
            if (!m_isConversion && !(type == 101) && !(type == 110) &&
                !(type == 11)) {
              myVertex.reset();
            }

            // If we have the right type (not reset above) lets fill information
            // and then push to the containers
            if (myVertex) {
              if (m_decorateVertices) {
                ATH_MSG_DEBUG(
                  "Decorating vertex with values used in track pair selector");
                for (const auto& kv :
                     InDet::TrackPairsSelector::getLastValues(cache)) {
                  SG::Accessor<float> acc (kv.first);
                  acc (*myVertex) = kv.second;
                }
                ATH_MSG_DEBUG("Decorating vertex with values used in vertex "
                              "point estimator");
                for (const auto& kv : intersectionDecors) {
                  SG::Accessor<float> acc (kv.first);
                  acc (*myVertex) = kv.second;
                }
              }

              ElementLink<xAOD::TrackParticleContainer> newLinkPos(*iter_pos,
                                                                   *trk_coll);
              ElementLink<xAOD::TrackParticleContainer> newLinkNeg(*iter_neg,
                                                                   *trk_coll);
              myVertex->addTrackAtVertex(newLinkPos);
              myVertex->addTrackAtVertex(newLinkNeg);

              // Now fill in the containers depending on the 2 possible
              // cases
              if (m_isConversion) {
                myVertex->setVertexType(xAOD::VxType::ConvVtx);
                InDetConversionContainer->push_back(std::move(myVertex));
              }
              else if (type == 101 || type == 110 || type == 11) { // V0
                myVertex->setVertexType(xAOD::VxType::V0Vtx);
                InDetConversionContainer->push_back(std::move(myVertex));
              }

           } // End if on right type

            negIndx[ineg] = 1;
            posIndx[ipos] = 1;
            ++numConversions;
          } else {
            ATH_MSG_DEBUG("VxCandidate failed the post selection cuts!");
            myVertex.reset();
          }
        } else {
          ATH_MSG_DEBUG("VertexFit was NOT successful!");
        }
        positionList.clear();
      } // neg loop
    }   // pos loop
    ATH_MSG_DEBUG("Number of conversions found passing post selection cuts: "
                  << numConversions);

    // single track conversions
    if (m_isConversion) {
      for (int ip = 0; ip < int(posIndx.size()); ++ip) {
        if (posIndx[ip] == 0)
          singleTrackConvList.push_back(posSelectedTracks[ip]);
      }
      for (int in = 0; in < int(negIndx.size()); ++in) {
        if (negIndx[in] == 0)
          singleTrackConvList.push_back(negSelectedTracks[in]);
      }

      std::vector<const xAOD::TrackParticle*>::iterator itk;
      std::vector<const xAOD::TrackParticle*>::iterator itke = singleTrackConvList.end();
      int numSingle = 0;
      for (itk = singleTrackConvList.begin(); itk != itke; ++itk) {
        if (!m_singleTrkConvTool->selectSingleTrackParticleConversion((*itk)))
          ATH_MSG_DEBUG("Track failed single track conversion selection");
        else {
          xAOD::Vertex* sConver(nullptr);
          sConver = m_singleTrkConvTool->buildSingleTrackParticleConversion(
            (*itk), InDetConversionContainer);
          if (sConver) {
            sConver->clearTracks();

            ElementLink<xAOD::TrackParticleContainer> newLink;
            newLink.toContainedElement(*trk_coll, *itk);
            sConver->addTrackAtVertex(newLink);
            sConver->setVertexType(xAOD::VxType::ConvVtx);
            numSingle++;

            if (m_decorateVertices) {
              ATH_MSG_DEBUG("Decorating single track vertex with dummy values "
                            "used in track pair selector");
              for (const auto& kv : InDet::TrackPairsSelector::getLastValues(cache)) {
                SG::Accessor<float> acc (kv.first);
                acc (*sConver) = 0.;
              }

              ATH_MSG_DEBUG("Decorating single track vertex with dummy values "
                            "used in vertex point estimator");
              for (const std::string& k : InDet::VertexPointEstimator::decorKeys()) {
                SG::Accessor<float> acc (k);
                acc (*sConver) = 0.;
              }

              ATH_MSG_DEBUG("Decorating single track vertex with dummy values "
                            "used in post selector");
              InDet::ConversionPostSelector::decorateVertex(*sConver, 0., 0., 0., 0., 0.);
            }
          }
        }
      }
      ATH_MSG_DEBUG("Number successful reconstructed single track conversion: "
                    << numSingle);
    }

    return std::make_pair(InDetConversionContainer,
                          InDetConversionContainerAux);
  }

  bool
  InDetConversionFinderTools::passPreSelection(
    TrackPairsSelector::Cache& cache,
    const xAOD::TrackParticle* track_pos,
    const xAOD::TrackParticle* track_neg,
    std::vector<Amg::Vector3D>& trackList,
    Amg::Vector3D& initPos,
    int& flag,
    std::map<std::string, float>& intersectionDecors) const
  {
    //Track summary information
  
    uint8_t nclusPos(0);
    uint8_t dummy(0);
    if(track_pos->summaryValue(dummy,xAOD::numberOfSCTHits)){
      nclusPos += dummy;
    } 
    if(track_pos->summaryValue(dummy,xAOD::numberOfPixelHits)){
      nclusPos += dummy;
    } 
  
    uint8_t nclusNeg(0);
    if(track_neg->summaryValue(dummy,xAOD::numberOfSCTHits)){
      nclusNeg += dummy;
    } 
    if(track_neg->summaryValue(dummy,xAOD::numberOfPixelHits)){
      nclusNeg += dummy;
    } 
  
  
    if(nclusNeg>0 && nclusPos>0) flag= 0;
    if((nclusNeg>0 && nclusPos==0) || (nclusNeg==0 && nclusPos>0)) flag = 1;
    if(nclusNeg==0 && nclusPos==0) flag = 2;
    if(m_removeTrt && (flag==1 || flag==2)) return false;

    if (m_trackPairsSelector->selectTrackParticlePair( track_pos,track_neg,cache)){
  
      const Trk::Perigee& perPos = track_pos->perigeeParameters();
      const Trk::Perigee& perNeg = track_neg->perigeeParameters();
      int errorcode = 0;
      Amg::Vector3D startingPoint(
        m_vertexEstimator->getCirclesIntersectionPoint(
          &perPos, &perNeg, flag, errorcode, intersectionDecors));
      if(m_isConversion && errorcode != 0) return false;
      if(!m_isConversion){ 
        Amg::Vector3D v_direction = perPos.momentum() + perNeg.momentum();
        double d_alpha = (startingPoint.adjoint()*v_direction)[0]/(startingPoint.mag()*v_direction.mag());
        if(d_alpha<m_MinFlightAngle) return false;
      }
      initPos = startingPoint;
      double startingPointR = startingPoint.perp(); 
      if(startingPointR<800.) {
        //Get measured track parameters at first track measurement.
        
        //Position of first hit in track particle
        Trk::CurvilinearParameters trkPar_pos; 
        Trk::CurvilinearParameters trkPar_neg; 

        int index(-1);
        for(unsigned int i(0); i< track_pos->numberOfParameters() ; ++i ){
          if( xAOD::FirstMeasurement == track_pos->parameterPosition(i) ){
            index = i;
            break;
          }
        }
        if(index!=-1){
          trkPar_pos = track_pos->curvilinearParameters(index);
        } else {
          ATH_MSG_WARNING("Track Particle does not contain first Measurement track parameters");
          return false;
        }
    
        index = -1;
        for(unsigned int i(0); i< track_neg->numberOfParameters() ; ++i ){
          if( xAOD::FirstMeasurement == track_neg->parameterPosition(i) ){
            index = i;
            break;
          }
        }
        if(index!=-1){
          trkPar_neg = track_neg->curvilinearParameters(index);
        } else {
          ATH_MSG_WARNING("Track Particle does not contain first Measurement track parameters");
          return false;
        }

        
        if(!m_isConversion){
          double posR = trkPar_pos.position().perp();
          double negR = trkPar_neg.position().perp();
          double diffR = 1000.;
          if((startingPointR-posR)<(startingPointR-negR)) diffR = startingPointR-posR;
          else                            diffR = startingPointR-negR;
          if(startingPointR<m_MinInitVtxR) return false;
          if(diffR<m_mindR || diffR>m_maxdR) return false;
        }

        //Alternative way to estimate the guess vertex
        initPos = trkPar_pos.position() + trkPar_neg.position();
        initPos *= 0.5;
        // Not correct but will do for now
        trackList.push_back(trkPar_pos.position());
        trackList.push_back(trkPar_neg.position());
        
      }
    }
    return true;
  }
}// end namespace InDet
