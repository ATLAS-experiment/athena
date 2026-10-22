/**
 **     @file    AnalysisR4.cxx
 **
 **     @author  mark sutton
 **     @date    Thu 28 Sep 2025 15:35:34 CEST 
 **
 **     $Id: AnalysisR4.cxx, v0.0   Thu 28 Sep 2025 15:35:34 CEST 
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/



#include "AnalysisR4.h"

#include "TrigInDetAnalysisExample/ChainString.h"
#include "TrackView.h"

// #include "Tier0.h"
#include "Resolutions.h"
#include "Efficiencies.h"
// these are coming soon !!...
// #include "VertexPlots.h"
#include "TrackParameters.h"
// #include "SiHitPlots.h"
// #include "TRTPlots.h"

#include <cmath>
#include <iostream>


IDTPM2::AnalysisR4::AnalysisR4(const std::string& name)
  : m_name(name), m_monTool(0)
{
  /// register the different plotters ... 
  //  registerCreator( "Tier0",           &IPlotBase::create<Tier0> );
  //  registerCreator( "Vertices",        &IPlotBase::create<VertexPlots> );
  registerCreator( "Resolutions",     &IPlotBase::create<Resolutions> );
  registerCreator( "Efficiencies",    &IPlotBase::create<Efficiencies> );
  registerCreator( "TrackParameters", &IPlotBase::create<TrackParameters> );
  // registerCreator( "SiHits",          &IPlotBase::create<SiHitPlots> );
  // registerCreator( "TRTHits",         &IPlotBase::create<TRTPlots> );
}



void IDTPM2::AnalysisR4::initialise() {

  if ( !monTool() ) throw std::runtime_error( "AnalysisR4:initialize() monTool not found" ); 

  /// we have to tell the class which sets of plots it
  /// should create from the factory
  /// This can be done externally using the addFillTool() method, 
  /// but any default tools that should always be enabled, 
  /// should be created here, like this ...
  //  m_tools.emplace_back( create( "Tier0" ) );
  //  m_tools.emplace_back( create( "Resolutions" ) );
  //  m_tools.emplace_back( create( "Efficiencies" ) );
  //  ...

  /// all the tools are set from the TrackAnalysis by calling addFillTool()
  for ( size_t i=m_tools.size() ; i-- ; ) m_tools[i]->initialize();
      
}



void IDTPM2::AnalysisR4::execute( TrackAnalysisStore& collections ) const { 
  // if ( m_tools.empty() ) throw std::runtime_error( "AnalysisR4::execute() cowardly: won't run analysis with no output" );
  for ( size_t i=m_tools.size() ; i-- ; ) m_tools[i]->execute( collections );
}



void IDTPM2::AnalysisR4::finalize() {
  for ( size_t i=m_tools.size() ; i-- ; ) m_tools[i]->finalize();
} 







