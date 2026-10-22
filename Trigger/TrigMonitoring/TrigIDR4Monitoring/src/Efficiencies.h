/** emacs: this is -*- c++ -*-
 **
 **  @file   Efficiencies.h
 **  @author sutt@csrn.ch 
 **
 **         based on a class from Marco Aparo <Marco.Aparo@cern.ch>
 **
 **  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef IDTPM2_EFFICIENCIES_H
#define IDTPM2_EFFICIENCIES_H

#include "IPlotBase.h"

namespace IDTPM2 {

class Efficiencies : public IPlotBase {

public:

  /// Constructor
  Efficiencies( const std::string& name, ToolHandle<GenericMonitoringTool>* m=0 );

  /// Destructor
  virtual ~Efficiencies() = default;
  
  /// Dedicated fill method (for tracks and/or truth particles)

  void initialize(); // needed to override IPlotBase

  void execute( const TrackAnalysisStore& collections ) const;
  
  /// Print out final stats on histograms
  void finalize();
  
private:

  /// NB: the egregious root convention for a TEfficiency is
  ///     Fill( ismatched, variable ), but that is awful, and
  ///     inconsistent, so we swap that round
  
  TIDA::Histogram2D<float,bool> m_eff_vs_inclusive;

  TIDA::Histogram2D<float,bool> m_eff_vs_ntracks;

  TIDA::Histogram2D<float,bool> m_eff_vs_pt;
  //  TIDA::Histogram2D<float,bool> m_eff_vs_logPt;
  //  TIDA::Histogram2D<float,bool> m_eff_vs_lowPt;
  TIDA::Histogram2D<float,bool> m_eff_vs_eta;
  TIDA::Histogram2D<float,bool> m_eff_vs_phi;
  TIDA::Histogram2D<float,bool> m_eff_vs_d0;
  TIDA::Histogram2D<float,bool> m_eff_vs_z0;

  TIDA::Histogram2D<float,bool> m_eff_vs_Et;
  TIDA::Histogram2D<float,bool> m_eff_vs_EtovPt;

  TIDA::Histogram2D<float,bool> m_eff_vs_actualMu;
  // TIDA::Histogram2D<float,bool> m_eff_vs_truthMu;

  /// not at the moment, but will add these at some point ...
  // TIDA::Histogram2D<float,bool> m_eff_vs_prodR;
  // TIDA::Histogram2D<float,bool> m_eff_vs_prodZ;
  // TIDA::Histogram2D<float,bool> m_eff_vs_eta_vs_pt;
  // TIDA::Histogram2D<float,bool> m_eff_vs_eta_vs_phi;
  // TIDA::Histogram2D<float,bool> m_eff_vs_z0_vs_d0;
  // TIDA::Histogram2D<float,bool> m_eff_vs_z0sin_vs_d0;
    
}; // class Efficiencies

} // namespace IDTPM2

#endif // > ! IDTPM2_EFFICIENCIES_H
