/**
 **   @file    SiHitPlots.cxx         
 **   
 **
 **   @author sutt
 **   @date   Wed 22 Jul 2026 16:16:38 BST
 **
 **   $Id: SiHitPlots.cxx, v0.0   Wed 22 Jul 2026 16:16:38 BST sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#include "SiHitPlots.h"

IDTPM2::SiHitPlots::SiHitPlots( const std::string& name, ToolHandle<GenericMonitoringTool>* m )
  : IPlotBase( name, m ) { 
}




/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------

void IDTPM2::SiHitPlots::initialize() {
  /// parameter histograms ...
  m_reference.initialize( this, "reference");
  m_test.initialize(      this, "test");

}



void IDTPM2::SiHitPlots::Histograms::initialize( SiHitPlots* p, const std::string& s ) {

  static const std::array<bool,NHitPars> proc = { true, true, false, false, true, true, true, true, true, false, true, true, true };
  
  for ( size_t i=0 ; i<nHitParNames() ; i++ ) {

    if ( !proc[i] ) continue;
    
    m_hits[i] = TIDA::Histogram<float>( p->monTool(), hitParName(i), s );
  
    /// should use Resplots for these really, as profiles are affected by outlyera...

    m_hits_vs_pt[i]  = TIDA::Histogram<float>( p->monTool(), hitParName(i)+"_vs_pt",  s );
    m_hits_vs_eta[i] = TIDA::Histogram<float>( p->monTool(), hitParName(i)+"_vs_eta", s );
    m_hits_vs_phi[i] = TIDA::Histogram<float>( p->monTool(), hitParName(i)+"_vs_phi", s );
    m_hits_vs_d0[i]  = TIDA::Histogram<float>( p->monTool(), hitParName(i)+"_vs_d0",  s );
 
  }
  
}


void  IDTPM2::SiHitPlots::Histograms::fill( const TrackView* t ) const {

  /// need a mechanism to get the TrackParticle from say an xAOD::Electron
  /// say by an or of
  /// t.is<xAOD::TrackParticle>() || ( t.hashChild() && t.child(0).is<sADO::TrackParticle>() ) 

  /// actually, if we are using the TrackTracks access we don't need to 
  /// test if it is a TrackParticle
  
  //  const xAOD::TrackParticle* tp = t->as<xAOD::TrackParticle>();

  //  if ( !tp ) return; /// no point trying to fill anything unless it's an xAOD::TrackParticle 
  
  /// there are far, far too many "hit type" parameters on the tracks,
  /// the bloated tracks classes with the ludicrously long names
  /// are really not nice and do not lead to nice code ...

  /// I mean 28 !!! how mamny layers might a track go through ?
  /// with a max os say 20 layers it would be more efficient storing a status
  /// for each layer - so for each layer you could could have
  /// a number or bits - hit | hole | ... then you could store say 1 uint8_t
  /// per layer rather than all these individual (28) uint8_t counters
  /// for all the variables. or just store the hit patterns and let people
  /// work out what they wanted themselves. With all this bloat, no wonder
  /// the tracks are so large 

  // can add the rest of these histgrams as and when we need to ...

  /// Compute hit parameters
  std::array<float,NHitPars> hitP = {};

#if 1
  hitP[NInnerMostPixelHits]                   = t->nPixelsInner();
  hitP[NInnerMostPixelEndcapHits]             = t->nPixelsInnerEndcap();
  //  hitP[NNextToInnerMostPixelHits]         = t->nNextToInnerMostPixelHits();
  //  hitP[NNextToInnerMostPixelEndcapHits]   = t->nNextToInnerMostPixelEndcapHits();
  hitP[NInnerMostPixelSharedHits]             = t->nPixelsInnerShared();
  hitP[NInnerMostPixelSharedEndcapHits]       = t->nPixelsInnerSharedEndcap();
  hitP[NPixelHits]                            = t->nPixels();
  hitP[NPixelHoles]                           = t->nPixelHoles();
  hitP[NPixelSharedHits]                      = t->nPixelsShared();
  //  hitP[PixeldEdx]                         = t->pixeldEdx();
  hitP[NSCTHits]                              = t->nSCT();
  hitP[NSCTHoles]                             = t->nSCTHoles(); 
  hitP[NSCTSharedHits]                        = t->nSCTShared();

  //  hitP[NInnerMostPixelOutliers]           = t->nPixelsInnerOutliers();
  //  hitP[NInnerMostPixelEndcapOutliers]     = t->nPixelsInnerEndcapOutliers();
  //  hitP[NInnerMostPixelSplitHits]          = t->nPixelsInnerSplit();
  //  hitP[NInnerMostPixelSplitEndcapHits]    = t->nPixelsEndcapSplit();
  
  //  hitP[NExpectedInnerMostPixelHits]       = t->nExpectedInnerMostPixelHits();
  //  hitP[NExpectedNextToInnerMostPixelHits] = t->nExpectedNextToInnerMostPixelHits();

  //  hitP[NPixelOutliers]                    = t->nPixelOutliers();
  //  hitP[NPixelContribLayers]               = t->nPixelContribLayers();
  //  hitP[NPixelSplitHits]                   = t->nPixelsSplit();
  //  hitP[NPixelGangedHits]                  = t->nPixelsGanged();
  //  hitP[NPixelGangedHitsFlaggedFakes]      = t->nPixelGangedHitsFlaggedFakes();
  //  hitP[NPixelDeadSensors]                 = t->nPixelDeadSensors();
  //  hitP[NSCTOutliers]                      = t->nSCTOutliers();
  //  hitP[NSCTDoubleHoles]                   = t->nSCTDoubleHoles();
  //  hitP[NSCTDeadSensors]                   = t->nSCTDeadSensors();
  
#else
  /// NB, these names are not correct, this code won't work as is
  hitP[NInnerMostPixelHits]               = t->nInnerMostPixelHits();
  hitP[NInnerMostPixelEndcapHits]         = t->nInnerMostPixelEndcapHits();
  hitP[NNextToInnerMostPixelHits]         = t->nNextToInnerMostPixelHits();
  hitP[NNextToInnerMostPixelEndcapHits]   = t->nNextToInnerMostPixelEndcapHits();
  hitP[NInnerMostPixelSharedHits]         = t->nInnerMostPixelSharedHits();
  hitP[NInnerMostPixelSharedEndcapHits]   = t->nInnerMostPixelSharedEndcapHits();
  hitP[NPixelHits]                        = t->nPixelHits();
  hitP[NPixelHoles]                       = t->nPixelHoles();
  hitP[NPixelSharedHits]                  = t->nPixelSharedHits();
  hitP[PixeldEdx]                         = t->pixeldEdx();
  hitP[NSCTHits]                          = t->nSCTHits();
  hitP[NSCTHoles]                         = t->nSCTHoles(); 
  hitP[NSCTSharedHits]                    = t->nSCTSharedHits();
  hitP[NInnerMostPixelOutliers]           = t->nInnerMostPixelOutliers();
  hitP[NInnerMostPixelEndcapOutliers]     = t->nInnerMostPixelEndcapOutliers();
  hitP[NInnerMostPixelSplitHits]          = t->nInnerMostPixelSplitHits();
  hitP[NInnerMostPixelSplitEndcapHits]    = t->nInnerMostPixelSplitEndcapHits();
  hitP[NExpectedInnerMostPixelHits]       = t->nExpectedInnerMostPixelHits();
  hitP[NExpectedNextToInnerMostPixelHits] = t->nExpectedNextToInnerMostPixelHits();
  hitP[NPixelOutliers]                    = t->nPixelOutliers();
  hitP[NPixelContribLayers]               = t->nPixelContribLayers();
  hitP[NPixelSplitHits]                   = t->nPixelSplitHits();
  hitP[NPixelGangedHits]                  = t->nPixelGangedHits();
  hitP[NPixelGangedHitsFlaggedFakes]      = t->nPixelGangedHitsFlaggedFakes();
  hitP[NPixelDeadSensors]                 = t->nPixelDeadSensors();
  hitP[NSCTOutliers]                      = t->nSCTOutliers();
  hitP[NSCTDoubleHoles]                   = t->nSCTDoubleHoles();
  hitP[NSCTDeadSensors]                   = t->nSCTDeadSensors();
#endif

  float pt  = std::fabs(t->pt())*0.001;
  float eta = t->eta();
  float phi = t->phi();
  float d0  = t->d0();

  //  std::cout << "SiHits: track: " << *t << std::endl; 

  static const std::array<bool,NHitPars> proc = { true, true, false, false, true, true, true, true, true, false, true, true, true };
  
  for ( int i=0 ; i<NHitPars ; i++ ) {

    if ( !proc[i] ) continue;
    
    //    std::cout << "\tfill par; " << hitParName(i) << "\t: " << hitP[i] << std::endl;
    
    m_hits[i].Fill( hitP[i] );

    m_hits_vs_pt[i].Fill(  pt,  hitP[i] );
    m_hits_vs_eta[i].Fill( eta, hitP[i] );
    m_hits_vs_phi[i].Fill( phi, hitP[i] );
    m_hits_vs_d0[i].Fill(  d0,  hitP[i] );
  }

}



void IDTPM2::SiHitPlots::execute( const TrackAnalysisStore& collections ) const {

#if 0
  /// if we need histograms vs mu ...
  
  double  mu = 0;
  
  const xAOD::EventInfo* eventinfo = collections.eventInfo();
  
  if (eventinfo) {
    /// don't need run and event number ??
    mu  = collections.eventInfo()->averageInteractionsPerCrossing();
  }
#endif
  
  const std::vector<const TrackView*>& ref  = collections.reference_tracks();
  const std::vector<const TrackView*>& test = collections.test_tracks();

  for ( size_t i=0 ;  i<ref.size() ; i++ ) {  m_reference.fill(  ref[i] ); }
  for ( size_t i=0 ; i<test.size() ; i++ ) {       m_test.fill( test[i] ); }

  /// do we want to do anything looping over the matched histograms ??

}
  
  
  /// Print out final stats on histograms
void IDTPM2::SiHitPlots::finalize() { } 
  
