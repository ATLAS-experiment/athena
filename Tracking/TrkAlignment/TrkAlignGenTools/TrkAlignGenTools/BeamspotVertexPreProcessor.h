/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRKALIGNGENTOOLS_BEAMSPOTVERTEXPREPROCESSOR_H
#define  TRKALIGNGENTOOLS_BEAMSPOTVERTEXPREPROCESSOR_H

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IAlgTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"

#include "TrkAlignInterfaces/IAlignTrackPreProcessor.h"

#include "TrkAlignInterfaces/IAlignModuleTool.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkFitterInterfaces/IGlobalTrackFitter.h"
#include "TrkVertexFitterInterfaces/ITrackToVertexIPEstimator.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"

#include "TrkEventPrimitives/ParticleHypothesis.h"

#include "xAODTracking/VertexContainer.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

/**
   @brief Tool used to create a collection of AlignTracks from Tracks
   while the tracks are being refitted with a vertex/beam-spot constraint.
   The AlignTracks are filled into a collection of Tracks.
   The default control flow is like this:
   firstly do vertex constraint, if can't, then do beamspot again;
   if can't again, then do normal refit.
   if want to do vertex constraint only, set:
   doVertexConstraint = True and doBeamspotConstraint = False 
   if want to do beamspot constraint only, set:
   doVertexConstraint = False and doBeamspotConstraint = True 
   @author Jike Wang <jwang@cern.ch>
   @author Daniel Kollar <daniel.kollar@cern.ch>
   -> updated 2015 Matthias Danninger
*/
 

namespace Trk {
  class Track;
  class AlignTrack;
  class AlignVertex;
  class VertexOnTrack;
  class VxTrackAtVertex;

  class BeamspotVertexPreProcessor : virtual public Trk::IAlignTrackPreProcessor, public AthAlgTool
  {

  public:

    BeamspotVertexPreProcessor(const std::string & type, const std::string & name, const IInterface * parent);
    virtual ~BeamspotVertexPreProcessor();
    
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    virtual DataVector<Track> * processTrackCollection(const DataVector<Track> * trks) override;

    void accumulateVTX(AlignTrack* alignTrack) override;

    void solveVTX() override;

    /** Print processing summary to logfile. */
    virtual void printSummary() override;



  private:

    void  prepareAllTracksVector();

    bool  isAssociatedToPV(const Track * track, const xAOD::VertexContainer* vertices);
    bool  isAssociatedToVertex(const Track * track, const xAOD::Vertex * vertex);

    bool  selectVertices(const xAOD::Vertex * vtx) const;
    bool  selectUpdatedVertices(const xAOD::Vertex * updatedVtx) const;
    
    const xAOD::Vertex* findVertexCandidate(const Track* track) const; // MD: changed to be now only a vertex
    const VertexOnTrack* provideVotFromVertex(const Track* track, const xAOD::Vertex* &vtx) const;
    const VertexOnTrack* provideVotFromBeamspot(const Track* track) const;
    void provideVtxBeamspot(const AlignVertex* b, AmgSymMatrix(3)* q, Amg::Vector3D* v) const;


    const Track* doConstraintRefit(ToolHandle<IGlobalTrackFitter>& fitter, const Track* track,  const VertexOnTrack* vot, const ParticleHypothesis& particleHypothesis) const;
    bool  doBeamspotConstraintTrackSelection(const Track* track);
    AlignTrack* doTrackRefit(const Track* track);


    ToolHandle<IGlobalTrackFitter> m_trackFitter{
      this, "TrackFitter", "Trk::GlobalChi2Fitter/InDetTrackFitter",
      "normal track fitter"};
    ToolHandle<IGlobalTrackFitter> m_SLTrackFitter{
      this, "SLTrackFitter", "", "straight line track fitter"};
    ToolHandle<IExtrapolator> m_extrapolator{
      this, "Extrapolator", "Trk::Extrapolator/AtlasExtrapolator"};
    ToolHandle<InDet::IInDetTrackSelectionTool> m_trkSelector{
      this, "TrackSelector", "", "new track selector tool"};
    ToolHandle<InDet::IInDetTrackSelectionTool> m_BSTrackSelector{
      this, "BSConstraintTrackSelector", "",
      "new track selector tool for tracks to be used with beam-spot constraint"};
    ToolHandle<ITrackToVertexIPEstimator> m_trackToVertexIPEstimatorTool{
      this, "TrackToVertexIPEstimatorTool", ""};
    
    /** Pointer to AlignModuleTool*/
    PublicToolHandle<IAlignModuleTool> m_alignModuleTool{
      this, "AlignModuleTool", "InDet::InDetAlignModuleTool/InDetAlignModuleTool"};
    
    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey {
      this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

    SG::ReadHandleKey<xAOD::VertexContainer> m_PVContainerName{
      this, "PVContainerName", "PrimaryVertices"};
    
    BooleanProperty m_runOutlierRemoval{this, "RunOutlierRemoval", false,
      "switch whether to run outlier logics or not"};
    IntegerProperty m_particleNumber{this, "ParticleNumber", 3,
      "type of material interaction in extrapolation, 3=pion, 0=non-interacting"};
    BooleanProperty m_doTrkSelection{this, "DoTrackSelection", true,
      "to activate the preprocessor track selection"};
    BooleanProperty m_doBSTrackSelection{this, "DoBSTrackSelection", false,
      "the selection mechanism which is based on cutting the perigee parameters, pt, etc."};
    BooleanProperty m_doAssociatedToPVSelection{
      this, "DoAssociatedToPVSelection", true,
      "the selection mechanism that only use the tracks associated to PV"};

    UnsignedIntegerProperty m_constraintMode{this, "ConstraintMode", 0};

    std::vector< std::pair< const xAOD::Vertex*, std::vector<VxTrackAtVertex> > >  m_allTracksVector;

    BooleanProperty m_doBeamspotConstraint{this, "DoBSConstraint", true,
      "Constrain tracks to the beamspot (x,y) position"};
    BooleanProperty m_doPrimaryVertexConstraint{this, "DoPVConstraint", false,
      "Constrain tracks to the associated primary vertex (x,y,z) position"};
    BooleanProperty m_doFullVertexConstraint{this, "DoFullVertex", false,
      "Full 3D vertex constraint.  Note DoPVConstraint needs to be set to true to use this option. If DoBSConstraint vertex position will be constrained to the BS"};
    BooleanProperty m_doNormalRefit{this, "doNormalRefit", true,
      "provide tracks in the case failed BS, PV and FullVertex constraints."};
    
    DoubleProperty m_maxPt{this, "maxPt", 0.,
      "Max pT range for refitting tracks"};
 
    BooleanProperty m_refitTracks{this, "RefitTracks", true,
      "flag to refit tracks"};
    BooleanProperty m_storeFitMatrices{this, "StoreFitMatrices", true,
      "flag to store derivative and covariance matrices after refit"};
    BooleanProperty m_useSingleFitter{this, "UseSingleFitter", false,
      "only use 1 fitter for refitting track"};
    DoubleProperty m_BSScalingFactor{this, "BeamspotScalingFactor", 1.,
      "scaling factor on beasmpot width"};
    DoubleProperty m_PVScalingFactor{this, "PrimaryVertexScalingFactor", 1.,
      "scaling factor on primary vertex position error"};

    IntegerProperty m_minTrksInVtx{this, "MinTrksInVtx", 3,
      "requirement to the minimal number of tracks in the vertex"};

    int m_nTracks = 0;
    std::vector<int> m_trackTypeCounter{};
    int m_nFailedNormalRefits = 0;
    int m_nFailedBSRefits = 0;
    int m_nFailedPVRefits = 0;

    DataVector<AlignVertex> m_AlignVertices;	         //!< collection of AlignVertices used in FullVertex constraint option


  };



  class CompareTwoTracks {

    public:
         CompareTwoTracks(const Track* track, const std::string& compareMethod)
         :m_method(compareMethod)
         ,m_track(track)
         { //std::cout <<"compareMethod: "<< m_method <<std::endl; 
         }

    bool operator()(VxTrackAtVertex vtxTrk);

    private:
         std::string m_method;
         const Track* m_track;
  };


}

#endif // TRKALIGNGENTOOLS_BEAMSPOTVERTEXPREPROCESSOR_H

