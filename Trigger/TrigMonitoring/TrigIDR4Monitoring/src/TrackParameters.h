/** emacs: this is -*- c++ -*-
 **
 ** @file    TrackParameters.h
 **
 ** @author  sutt@cern.ch
 ** @date    
 **
 ** Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef IDTPM2_TRACKPARAMETERS_H
#define IDTPM2_TRACKPARAMETERS_H


#include "IPlotBase.h"


namespace IDTPM2 {

class TrackParameters : public IPlotBase {

public:

  TrackParameters( const std::string& name, ToolHandle<GenericMonitoringTool>* m );
  
  /// Destructor
  virtual ~TrackParameters() = default;

  /// Book the histograms
  void initialize(); // needed to override IPlotBase

  void execute( const TrackAnalysisStore& collections ) const;

  void finalize();

private:

  struct ParHistograms {
    
    TIDA::Histogram<float> m_pt;
    TIDA::Histogram<float> m_eta;
    TIDA::Histogram<float> m_phi;
    TIDA::Histogram<float> m_d0;
    TIDA::Histogram<float> m_z0;
    TIDA::Histogram<float> m_z0sin;
    TIDA::Histogram<float> m_theta;
    TIDA::Histogram<float> m_qoverp;
    TIDA::Histogram<float> m_prodR;
    TIDA::Histogram<float> m_prodZ;
    TIDA::Histogram<float> m_nSiHits;

    TIDA::Histogram<float> m_Et;
    TIDA::Histogram<float> m_EtovPt;

    /// should have plots of npixels and nstrips
    /// vs pt, eta, phi, z0, and z0
    TIDA::Histogram2D<float> m_nSiHits_vs_eta;

    TIDA::Histogram<float> m_chi2;
    TIDA::Histogram<float> m_ndof;
    TIDA::Histogram<float> m_chi2OverNdof;
    
    TIDA::Histogram<float> m_author;
    // TIDA::Histogram<float> m_time;
    // TIDA::Histogram<float> m_hasValidTime_eff_vs_eta;

    //    TIDA::Histogram2D<float> m_eta_vs_pt;
    //    TIDA::Histogram2D<float> m_eta_vs_phi;
    //    TIDA::Histogram2D<float> m_z0_vs_d0;
    //    TIDA::Histogram2D<float> m_z0sin_vs_d0;

    /// sigma plots ???
    TIDA::Histogram<float> m_sigma_pt;
    TIDA::Histogram<float> m_sigma_eta;
    TIDA::Histogram<float> m_sigma_phi;
    TIDA::Histogram<float> m_sigma_d0;
    TIDA::Histogram<float> m_sigma_z0;
    TIDA::Histogram<float> m_sigma_z0sin;
    TIDA::Histogram<float> m_sigma_theta;

    /// significance plots
    // TIDA::Histogram<float> m_significance_pt;
    // TIDA::Histogram<float> m_significance_eta;
    // TIDA::Histogram<float> m_significance_phi;
    TIDA::Histogram<float> m_significance_d0;
    // TIDA::Histogram<float> m_significance_z0;
    // TIDA::Histogram<float> m_significance_z0sin;
    // TIDA::Histogram<float> m_significance_theta;

    void initialize( TrackParameters* p, const std::string& s="" ); 

    void fill( const TrackView* p ) const;
    
  };

  ParHistograms m_reference;
  ParHistograms m_test;

  TIDA::Histogram<float> m_invmass;
  
}; // class TrackParameters

} // namespace IDTPM2

#endif // > ! IDTPM2_TRACKPARAMETERS_H
