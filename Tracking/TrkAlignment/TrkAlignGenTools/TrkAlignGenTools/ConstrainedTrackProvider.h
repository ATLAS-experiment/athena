/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKALIGNGENTOOLS_CONSTRAINEDTRACKPROVIDER_H
#define TRKALIGNGENTOOLS_CONSTRAINEDTRACKPROVIDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>

#include "TrkAlignInterfaces/ITrackCollectionProvider.h"
#include "TrkFitterUtils/FitterTypes.h"  
#include "TrkFitterInterfaces/IGlobalTrackFitter.h"
#include "TrkTrack/TrackCollection.h"


class TFile;
class TH2F;
/**
  ConstrainedTrackProvider
  
  Will provide  a track coolection with momentum conatraint applied.  A 2d histogram read  and 
  corrections (delta) are extracted as function of eta and phi.
  The momentum is scaled as 
  corrected QoverP = original QoverP * (1.+ charge *pt *delta );

  The strength of eth constraint can be varied as well.
*/
namespace Trk {
  class ConstrainedTrackProvider : virtual public ITrackCollectionProvider, public AthAlgTool {

  public:
    ConstrainedTrackProvider(const std::string & type, const std::string & name, const IInterface * parent);
    
    virtual StatusCode initialize();
    virtual StatusCode finalize();
    
    virtual StatusCode trackCollection(const TrackCollection*& tracks);

    virtual void printSummary();

  private :

    void  getCorrectedValues_P(const Trk::Perigee* mp, double& correctedQoverP,double& correctedQoverPError);
    void  getCorrectedValues_d0(const Trk::Perigee* mp, double& corrected_d0,double& corrected_d0Error);
    void  getCorrectedValues_z0(const Trk::Perigee* mp, double& corrected_z0,double& corrected_z0Error);
    bool  passTrackSelection(const Trk::Track *track);

    ToolHandle<IGlobalTrackFitter> m_trackFitter{this, "TrackFitter",
      "Trk::GlobalChi2Fitter/InDetTrackFitter", "normal track fitter"};
 
    Gaudi::Property<Trk::RunOutlierRemoval> m_runOutlierRemoval{
      this, "RunOutlierRemoval", true, "run outlier removal in the GX2 fitter"};
    BooleanProperty m_scalepmaptogev{this, "ScalePMapToGeV", false};
    BooleanProperty m_doTrackSelection{this, "doTrackSelection", true};
    BooleanProperty m_useConstrainedTrkOnly{this, "UseConstrainedTrkOnly", false};
    BooleanProperty m_useConstraintError{this, "UseConstraintError", true};

    int m_constrainedTracks = 0;
    int m_unconstrainedTracks = 0;

    DoubleProperty m_reduceConstraintUncertainty{
      this, "ReduceConstraintUncertainty", 1.,
      "Reduce the uncertainty on the track parmater constraint by this amount"};
    DoubleProperty m_reduceConstraintUncert_z0{this, "ReduceConstraintUncert_z0", 1.,
      "Reduce the uncertainty on z0 track parameter constraint by this amount"};
    DoubleProperty m_deltaScaling{this, "DeltaScaling", 1.};

    IntegerProperty m_minPIXHits{this, "MinPIXHits", 0};
    IntegerProperty m_minSCTHits{this, "MinSCTHits", 0};
    IntegerProperty m_minTRTHits{this, "MinTRTHits", 0};
    DoubleProperty m_maxd0{this, "Maxd0", 500.};
    DoubleProperty m_maxz0{this, "Maxz0", 500.};
    DoubleProperty m_minPt{this, "MinPt", 15.};
    DoubleProperty m_maxPt{this, "MaxPt", 100.};

    SG::ReadHandleKey<TrackCollection> m_inputKey{
      this, "InputTracksCollection", "Tracks"};
    SG::WriteHandleKey<TrackCollection> m_outputKey{
      this, "OutputTracksCollection", "AlignmentConstrainedTracks"};

    BooleanProperty m_CorrectMomentum{this, "CorrectMomentum", true};
    StringProperty m_constraintFileName_P{
      this, "MomentumConstraintFileName", "Constraint.root"};
    TFile* m_constraintInputFile_P = nullptr;
    StringProperty m_constraintHistName_P{
      this, "MomentumConstraintHistName", "EtaPhiMap"};
    TH2F* m_etaphiMap_P = nullptr;
      // Corrections expected to be in GeV-1

    StringProperty m_constraintFileName_d0{
      this, "d0ConstraintFileName", "Constraint.root"};
    TFile* m_constraintInputFile_d0 = nullptr;
    StringProperty m_constraintHistName_d0{this, "d0ConstraintHistName", "EtaPhiMap"};
    TH2F* m_etaphiMap_d0 = nullptr;
      // Corrections expected to be in mm

    BooleanProperty m_CorrectZ0{this, "CorrectZ0", false};
    StringProperty m_constraintFileName_z0{
      this, "z0ConstraintFileName", "Constraint.root"};
    TFile* m_constraintInputFile_z0 = nullptr;
    StringProperty m_constraintHistName_z0{this, "z0ConstraintHistName", "EtaPhiMap"};
    TH2F* m_etaphiMap_z0 = nullptr;
      // Corrections expected to be in mm

    BooleanProperty m_CorrectD0{this, "CorrectD0", false};
    BooleanProperty m_CorrectMeanD0{this, "CorrectMeanD0", false};

    BooleanProperty m_SelectByCharge{this, "SelectByCharge", false};
    BooleanProperty m_SelectPositive{this, "SelectPositive", true};

    
  }; // end class

} // end namespace

#endif // TRKALIGNGENTOOLS_CONSTRAINEDTRACKPROVIDER_H
