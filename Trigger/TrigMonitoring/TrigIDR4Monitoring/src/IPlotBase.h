/* emacs: this is -*- c++ -*- */
/**
 **   @file    IPlotBase.h        
 **                   
 **   @author  sutt
 **   @date    Wed  1 Jul 2026 12:14:36 BST
 **
 **   $Id: IPlotBase.h, v0.0   Wed  1 Jul 2026 12:14:36 BST sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/


#ifndef  IDTPM2_IPLOTBASE_H
#define  IDTPM2_IPLOTBASE_H

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AsgMessaging/StatusCode.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "TrigInDetAnalysisExample/TIDAHistogram.h"

#include "TrackAnalysisStore.h"

#include <memory>
#include <string>
#include <stdexcept>


namespace IDTPM2 { 

class IPlotBase : public AthMessaging {

public:

  /// built from dirName and anaTag
  IPlotBase( const std::string& name, ToolHandle<GenericMonitoringTool>* m=0 )  
    : AthMessaging(name), m_name(name), m_monTool(m) 
  { }

  virtual ~IPlotBase() = default;

  
  /// factory creation mathod
  template<typename Derived>
  static std::unique_ptr<IPlotBase> create(  const std::string& name,
					    ToolHandle<GenericMonitoringTool>* m ) {
    return std::make_unique<Derived>( name, m );
  } 
  
  /// Book the histograms
  virtual void initialize() = 0; // needed to override IPlotBase
  
  /// Finalize resolution and bias histograms
  /// should be called from the calling montools
  /// stop() method
  virtual void finalize() = 0;

  /// this can be over ridden if need be
  virtual void execute( const TrackAnalysisStore& /* collections */ ) const=0;
  
  /// generic utilities, just in case

  const std::string& name() const { return m_name; }

  /// mon tool 
  ToolHandle<GenericMonitoringTool>* monTool() { return m_monTool; }

  const ToolHandle<GenericMonitoringTool>* monTool() const { return m_monTool; }
  
  /// do we need / should we have this specific setter ??
  void set_monTool( ToolHandle<GenericMonitoringTool>* m ) { m_monTool=m; }

  std::string realPath( const std::string& tag="" ); 
  
protected:

  /// resolution porocessing stuff
  /// these don't really even need to be class functions, they could
  /// be simply utility functions
  
  void copyHist( TH1* hsrc, TH1* hdest, bool copyoverflow=true ) const;

  void getHist( const std::string& hname, TH1* hsrc ) { return getHist( histSvc(), hname, hsrc ); }
  void processResolution( const std::string& path, bool writeflag=false){ return processResolution( histSvc(), path, writeflag ); }
  
  void getHist( ServiceHandle<ITHistSvc>& histsvc, const std::string& hname, TH1* hsrc ) const;

  void processResolution( ServiceHandle<ITHistSvc>& histsvc, const std::string& path, bool writeflag=false ) const;

  /// we may well need these at some point ...
  
  //  void binWidthNormalise( TH1* h ) const;
  
  //  void binWidthNormalise( ServiceHandle<ITHistSvc>& histsvc, const std::string& hname ) const;
  
protected:
  
  /// this method is dangerous - should not be used anywhere
  /// except finalise or stop 
  virtual ServiceHandle<ITHistSvc>& histSvc() {
    if ( monTool()==0 ) throw std::runtime_error("IPlotBase: monTool not set");
    return (*monTool())->histogramService();
  }

  
  
protected:

  const std::string& m_name;

  ToolHandle<GenericMonitoringTool>* m_monTool;

};


}

  
inline std::ostream& operator<<( std::ostream& s, const IDTPM2::IPlotBase& ) { 
  return s;
}


#endif  // IDTPM2_IPLOTBASE_H 










