/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// Name    : MSVVariableFactory.h
/// Created : sept 14
///
/// DESCRIPTION:
///
/// This class is a "variable factory". It generates all
/// the variables per vertex to MSV.
///
///////////////////////////////////////////////////////////////////////////////////////////////////////
#include "VxVertex/VxTrackAtVertex.h"

#include "TrkParameters/TrackParameters.h"
#include "VxSecVertex/VxSecVertexInfo.h"
#include "VxSecVertex/VxSecVKalVertexInfo.h"

#include <TMath.h>

#include "CLHEP/Vector/LorentzVector.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include "JetTagTools/MSVVariablesFactory.h"
#include "JetTagTools/VertexSignificance.h"

#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODBTagging/SecVtxHelper.h"

#include <vector>
#include <string>

namespace Analysis {

  MSVVariablesFactory::MSVVariablesFactory(const std::string& name,
					   const std::string& n,
					   const IInterface* p):
    AthAlgTool(name, n,p)
  {
    declareInterface<IMSVVariablesFactory>(this);
  }

  StatusCode MSVVariablesFactory::initialize() {
    ATH_MSG_DEBUG(" Initialization of MSVVariablesFactory succesfull");
    return StatusCode::SUCCESS;
  }

  StatusCode MSVVariablesFactory::finalize() {
    ATH_MSG_DEBUG(" Finalization of MSVVariablesFactory succesfull");
    return StatusCode::SUCCESS;
  }

  StatusCode MSVVariablesFactory::createMSVContainer
  (const xAOD::Jet &myJet, const Trk::VxSecVKalVertexInfo* myVertexInfoVKal,
   xAOD::VertexContainer* VertexContainer,const xAOD::Vertex& PrimaryVtx) const {

    Amg::Vector3D jet_V3(myJet.p4().Px(), myJet.p4().Py(), myJet.p4().Pz());
    float jetenergy=0.;
    const xAOD::Vertex* priVtx = &PrimaryVtx;
    std::vector< ElementLink< xAOD::VertexContainer > > MSVVertexLinks;
    const std::vector<xAOD::Vertex*> myVertices = myVertexInfoVKal->vertices();
    if(myVertices.empty()){
      ATH_MSG_DEBUG("#BTAG# no MSV vertices...fill default values only... ");
      xAOD::Vertex* vertex = new xAOD::Vertex();
      VertexContainer->push_back(vertex);
      xAOD::SecVtxHelper::setVertexMass(vertex, -9.);
      xAOD::SecVtxHelper::setEnergyFraction(vertex, -9.);
      xAOD::SecVtxHelper::setVtxNtrk(vertex, -9);
      xAOD::SecVtxHelper::setVtxpt(vertex, -9.);
      xAOD::SecVtxHelper::setVtxeta(vertex, -9.);
      xAOD::SecVtxHelper::setVtxphi(vertex, -9.);
      xAOD::SecVtxHelper::setVtxnormDist(vertex, -9.);
      return StatusCode::SUCCESS;
    }

    jetenergy = myVertexInfoVKal->energyTrkInJet();

    for (const auto& vertex : myVertexInfoVKal->vertices()){
      VertexContainer->push_back(vertex);
      //additional info per vertex
      double sumpx = 0.0;
      double sumpy = 0.0;
      double sumpz = 0.0;
      double sume = 0.0;
      const std::vector<ElementLink<xAOD::TrackParticleContainer> > myTrackLinks = vertex->trackParticleLinks();
      if (myTrackLinks.empty()) {
        ATH_MSG_WARNING("#BTAG# No Track Links attached to the track at the sec vertex... ");
      }
      int npsec = 0;
      const std::vector<Trk::VxTrackAtVertex> myTracks=vertex->vxTrackAtVertex();
      if (!myTracks.empty()) {
        npsec=myTracks.size();
	for (const auto& track : myTracks) {
          const Trk::Perigee* perigee = dynamic_cast<const Trk::Perigee*>(track.perigeeAtVertex());
          if(perigee){
            sumpx += perigee->momentum().x();
            sumpy += perigee->momentum().y();
            sumpz += perigee->momentum().z();
            sume += std::hypot(perigee->momentum().mag(), ParticleConstants::chargedPionMassInMeV);
          }else{
            ATH_MSG_WARNING("#BTAG# perigee for VxTrackAtVertex not found");
          }
        }
      }

      CLHEP::HepLorentzVector vtxp4(sumpx,sumpy,sumpz,sume);
      float efrac = (jetenergy>0) ? vtxp4.e()/jetenergy : 0;
      xAOD::SecVtxHelper::setVertexMass(vertex, vtxp4.m());
      xAOD::SecVtxHelper::setEnergyFraction(vertex, efrac);
      xAOD::SecVtxHelper::setVtxNtrk(vertex, npsec);
      xAOD::SecVtxHelper::setVtxpt(vertex, vtxp4.perp());
      xAOD::SecVtxHelper::setVtxeta(vertex, vtxp4.eta());
      xAOD::SecVtxHelper::setVtxphi(vertex, vtxp4.phi());

      ATH_MSG_DEBUG("#BTAG# mass per vertex = "<<vtxp4.m());
      double localdistnrm = 0;
      std::vector<const xAOD::Vertex*> vecVtxHolder;
      vecVtxHolder.push_back(vertex);

      if (priVtx) {
        ATH_MSG_DEBUG("Factory PVX x = " << priVtx->x() << " y = " << priVtx->y() << " z = " << priVtx->z());
        localdistnrm = get3DSignificance(*priVtx, vecVtxHolder, jet_V3);
      } else {
        ATH_MSG_WARNING("#BTAG# Tagging requested, but no primary vertex supplied.");
      }
      xAOD::SecVtxHelper::setVtxnormDist(vertex, localdistnrm);
      //track links,
      vertex->setTrackParticleLinks(myTrackLinks);

    } //end loop vertexcontainer

    return StatusCode::SUCCESS;
  }

  StatusCode MSVVariablesFactory::fillMSVVariables
  (const xAOD::Jet &myJet, xAOD::BTagging* BTag,
   const Trk::VxSecVKalVertexInfo* myVertexInfoVKal,
   xAOD::VertexContainer* VertexContainer, const xAOD::Vertex& PrimaryVtx,
   std::string basename) const {

    Amg::Vector3D jet_V3(myJet.p4().Px(), myJet.p4().Py(), myJet.p4().Pz());
    int nvsec = 0;
    float jetenergy = 0.;
    int n2t = 0;
    float distnrm = 0.;
    const xAOD::Vertex* priVtx = &PrimaryVtx;
    std::vector< ElementLink< xAOD::VertexContainer > > MSVVertexLinks;
    const std::vector<xAOD::Vertex*> myVertices = myVertexInfoVKal->vertices();
    if(myVertices.empty()){
      ATH_MSG_DEBUG("#BTAG# no MSV vertices...fill default values only... ");
      BTag->setVariable<int>(basename, "N2Tpair", n2t);
      BTag->setVariable<float>(basename, "energyTrkInJet", jetenergy);
      BTag->setVariable<int>(basename, "nvsec", nvsec);
      BTag->setVariable<float>(basename, "normdist", distnrm);
      BTag->setVariable<std::vector<ElementLink<xAOD::VertexContainer> > >(basename, "vertices", MSVVertexLinks);
      BTag->setDynVxELName(basename, "vertices");
      xAOD::Vertex* vertex = new xAOD::Vertex();
      VertexContainer->push_back(vertex);
      xAOD::SecVtxHelper::setVertexMass(vertex, -9.);
      xAOD::SecVtxHelper::setEnergyFraction(vertex, -9.);
      xAOD::SecVtxHelper::setVtxNtrk(vertex, -9);
      xAOD::SecVtxHelper::setVtxpt(vertex, -9.);
      xAOD::SecVtxHelper::setVtxeta(vertex, -9.);
      xAOD::SecVtxHelper::setVtxphi(vertex, -9.);
      xAOD::SecVtxHelper::setVtxnormDist(vertex, -9.);
      return StatusCode::SUCCESS;
    }

    jetenergy = myVertexInfoVKal->energyTrkInJet();
    n2t = myVertexInfoVKal->n2trackvertices();
    BTag->setVariable<int>(basename, "N2Tpair", n2t);
    BTag->setVariable<float>(basename, "energyTrkInJet", jetenergy);

    std::vector<const xAOD::Vertex*> vecVertices;
    for (const auto& vertex : myVertexInfoVKal->vertices()) {
      VertexContainer->push_back(vertex);
      //additional info per vertex
      vecVertices.push_back(vertex);
      double sumpx = 0.0;
      double sumpy = 0.0;
      double sumpz = 0.0;
      double sume = 0.0;
      const std::vector<ElementLink<xAOD::TrackParticleContainer> > myTrackLinks = vertex->trackParticleLinks();
      if (myTrackLinks.empty()) {
        ATH_MSG_WARNING("#BTAG# No Track Links attached to the track at the sec vertex... ");
      }
      int npsec = 0;
      const std::vector<Trk::VxTrackAtVertex> myTracks=vertex->vxTrackAtVertex();
      if (!myTracks.empty()) {
        npsec=myTracks.size();
	for (const auto& track : myTracks) {
          const Trk::Perigee* perigee = dynamic_cast<const Trk::Perigee*>(track.perigeeAtVertex());
          if(perigee){
            sumpx += perigee->momentum().x();
            sumpy += perigee->momentum().y();
            sumpz += perigee->momentum().z();
            sume += std::hypot(perigee->momentum().mag(), ParticleConstants::chargedPionMassInMeV);
          }else{
            ATH_MSG_WARNING("#BTAG# perigee for VxTrackAtVertex not found");
          }
        }
      }

      CLHEP::HepLorentzVector vtxp4(sumpx,sumpy,sumpz,sume);
      float efrac = (jetenergy>0) ? vtxp4.e()/jetenergy : 0;
      xAOD::SecVtxHelper::setVertexMass(vertex, vtxp4.m());
      xAOD::SecVtxHelper::setEnergyFraction(vertex, efrac);
      xAOD::SecVtxHelper::setVtxNtrk(vertex, npsec);
      xAOD::SecVtxHelper::setVtxpt(vertex, vtxp4.perp());
      xAOD::SecVtxHelper::setVtxeta(vertex, vtxp4.eta());
      xAOD::SecVtxHelper::setVtxphi(vertex, vtxp4.phi());

      ATH_MSG_DEBUG("#BTAG# mass per vertex = "<<vtxp4.m());
      double localdistnrm = 0;
      std::vector<const xAOD::Vertex*> vecVtxHolder;
      vecVtxHolder.push_back(vertex);

      
      if (priVtx) {
        ATH_MSG_DEBUG("Factory PVX x = " << priVtx->x() << " y = " << priVtx->y() << " z = " << priVtx->z());
        localdistnrm = get3DSignificance(*priVtx, vecVtxHolder, jet_V3);
      } else {
        ATH_MSG_WARNING("#BTAG# Tagging requested, but no primary vertex supplied.");
      }
      xAOD::SecVtxHelper::setVtxnormDist(vertex, localdistnrm);
      //track links,
      vertex->setTrackParticleLinks(myTrackLinks);

      ElementLink< xAOD::VertexContainer> linkBTagVertex;
      linkBTagVertex.toContainedElement(*VertexContainer, vertex);
      MSVVertexLinks.push_back(linkBTagVertex);
    } //end loop vertexcontainer

    BTag->setVariable<std::vector<ElementLink<xAOD::VertexContainer> > >(basename, "vertices", MSVVertexLinks);
    BTag->setDynVxELName(basename, "vertices");

    if (priVtx) {
      distnrm = get3DSignificance(*priVtx, vecVertices, jet_V3);
    } else {
      ATH_MSG_WARNING("#BTAG# Tagging requested, but no primary vertex supplied.");
      distnrm=0.;
    }
    nvsec = vecVertices.size();
    BTag->setVariable<int>(basename, "nvsec", nvsec);
    BTag->setVariable<float>(basename, "normdist", distnrm);

    return StatusCode::SUCCESS;

  }

}//end Analysis namespace
