/** emacs: this is -*- c++ -*-
 **
 ** @file    VertexPlots.h
 ** @author  sutt@cern.ch
 **
 **          vertex analysis plots class implementation
 **          need to add the efficiencies as well as the
 **          differential resolutions, I mean, no efficiency
 **          vs number of tracks ? vs z ?? versus mu ???
 **
 **          So we will add the vertex efficiencies, and resolutions
 **          however, there is a more fundamental issue;
 **          we need plots of BOTH the test and reference vertices
 **          but do we want all reference vertices, and all
 **          test vertices ? and what about the matches ?
 **          so do we want the distribution only for all test,  
 **          or only for the matched test ?
 **          can have test, and ref plots with two instances or
 **          the filler - one for the test, and one for the
 **          reference, but better is what we have done here -
 **          place the histograms with tets, and reference
 **          versions in to a struct 
 **
 **
 ** Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef IDTPM2_VERTEXPLOTS_H
#define IDTPM2_VERTEXPLOTS_H


/// local includes
#include "IPlotBase.h"


namespace IDTPM2 {

class VertexPlots : public IPlotBase {

public:

  /// Constructor
  VertexPlots( const std::string& name, ToolHandle<GenericMonitoringTool>* m=0 ); 

  /// Destructor
  virtual ~VertexPlots() = default;
  
  /// Book the histograms
  void initialize(); 
  
  void execute( const TrackAnalysisStore& collections ) const;
  
  /// Print out final stats on histograms
  void finalize();
  
private:

  struct ParHistograms {
    
    /// global vertex quantities ...
    TIDA::Histogram<float>   m_nVtx;
    TIDA::Histogram2D<float> m_nVtx_vs_actualMu;
    // TIDA::Histogram<float> m_nVtx_vs_actualMu_2D;
    
    /// vertex parameter plots ...
    TIDA::Histogram<float> m_vtx_x;
    TIDA::Histogram<float> m_vtx_y;
    TIDA::Histogram<float> m_vtx_z;
    
    TIDA::Histogram<float> m_vtx_time; 
    
    TIDA::Histogram<float> m_vtx_x_err;
    TIDA::Histogram<float> m_vtx_y_err;
    TIDA::Histogram<float> m_vtx_z_err;

    TIDA::Histogram2D<float> m_vtx_x_vs_y;
    
    TIDA::Histogram<float> m_vtx_time_err; // ??
    TIDA::Histogram<float> m_vtx_chi2OverNdof; /// no ndof as well ? 
    TIDA::Histogram<float> m_vtx_type;
    
    TIDA::Histogram<float> m_vtx_nTracks;

    /// vertex-associated tracks plots,
    /// maybe we want things like sumpt^2 as well ??
    
    TIDA::Histogram<float> m_vtx_track_weight;
    TIDA::Histogram<float> m_vtx_track_pt;
    TIDA::Histogram<float> m_vtx_track_eta;
    TIDA::Histogram<float> m_vtx_track_nSiHits; 
    TIDA::Histogram<float> m_vtx_track_nSiHoles;
    TIDA::Histogram<float> m_vtx_track_d0;
    TIDA::Histogram<float> m_vtx_track_z0;
    TIDA::Histogram<float> m_vtx_track_d0_err;
    TIDA::Histogram<float> m_vtx_track_z0_err;
    
    void initialize( VertexPlots* p, const std::string& s="" ); 

    void fill( const VertexView* v ) const;
    
  };
 

private:
  
  /// why mess about with separate tools for the distributions,
  /// the efficiencies and resolutions
  
  ParHistograms m_test;
  ParHistograms m_reference;
  /// ParHisrograms matched;

  // correlation between test and referenec multiplicities ...
  TIDA::Histogram2D<float>   m_vtx_ntest_vs_nref;
  
  /// residuals ... 
  TIDA::Histogram<float>     m_vtx_res_x;
  TIDA::Histogram<float>     m_vtx_res_y;
  TIDA::Histogram<float>     m_vtx_res_z;

  /// diferential residuals ...
  TIDA::Histogram2D<float>   m_vtx_res_z_vs_z;
  TIDA::Histogram2D<float>   m_vtx_res_z_vs_ntracks;
  TIDA::Histogram2D<float>   m_vtx_res_z_vs_nvtx;
  TIDA::Histogram2D<float>   m_vtx_res_z_vs_mu;

  /// efficiencies ...
  TIDA::Histogram2D<float>   m_vtx_eff_vs_z;
  TIDA::Histogram2D<float>   m_vtx_eff_vs_mu;
  TIDA::Histogram2D<float>   m_vtx_eff_vs_nvtx;
  TIDA::Histogram2D<float>   m_vtx_eff_vs_ntracks;
  
  /// do we need these specific "truth" level plots ? 
  /// we need reference, and test level plots
  /// vertex multiplicity vs pileup plots
  //    TIDA::Histogram<float> m_nVtx_vs_truthMu_2D;
  //    TIDA::Histogram<float> m_nVtx_vs_truthMu;

}; // class VertexPlots

} // namespace IDTPM2

#endif // > ! IDTPM2_VERTEXPLOTS_H
