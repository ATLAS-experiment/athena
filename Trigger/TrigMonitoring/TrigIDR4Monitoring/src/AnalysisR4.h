/* emacs: this is -*- c++ -*- */
/**
 **     @file    AnalysisR4.h
 **
 **     @author  mark sutton
 **     @date    Thu 28 Sep 2025 15:35:34 CEST 
 **
 **     $Id: AnalysisR4.h, v0.0   Thu 28 Sep 2025 15:35:34 CEST 
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef IDTPM2_AnalysisR4_H
#define IDTPM2_AnalysisR4_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "TrigInDetAnalysis/TIDDirectory.h"
#include "TrigInDetAnalysisExample/TIDAHistogram.h"

#include "TrackAnalysisStore.h"

#include "IPlotBase.h"

#include <map>
#include <stdexcept>


namespace IDTPM2 {
  
class AnalysisR4 {

public:

  AnalysisR4(const std::string& name);
  
  virtual void initialise();

  virtual void execute( TrackAnalysisStore& collections ) const;

  virtual void finalize();

  const std::string& name() const { return m_name; }
  
  void set_monTool( ToolHandle<GenericMonitoringTool>* m ) { m_monTool=m; }

  ToolHandle<GenericMonitoringTool>* monTool() { return m_monTool; }

  /// add a fill tool = this is called from the parent class, otherwise
  /// there are no fill tools booked 
  void addFillTool( const std::string& toolname ) { m_tools.emplace_back( create( toolname ) ); }
  
protected:

  /// factory stuff ..

  std::unique_ptr<IPlotBase> create( const std::string& key ) {
    std::map<std::string, FactoryFn>::const_iterator itr = m_factory.find(key);    
    if (itr == m_factory.end()) throw std::runtime_error("IPlotBase Factory: No class of type " + key ); 
    return itr->second( key+"-"+name(), monTool() );
  }
  

protected:
  
  /// factory stuff ..
  
  using FactoryFn = std::unique_ptr<IPlotBase>(*)( const std::string&, ToolHandle<GenericMonitoringTool>* );

  /// do we want this to be public ?? 
  void registerCreator( const std::string& key, FactoryFn builder ) {
    m_factory.insert( std::map<std::string, FactoryFn>::value_type( key, builder ) );
  }
  
  /// initialise the factory in the constructor,
  /// so the header does not need to be modified
  /// to add a tool
  /// Also some factory helper function should be
  /// added so that tools can be added to the factory
  /// on the fly, without needing to recompile
  /// the code
  std::map<std::string, FactoryFn> m_factory;

private:

  std::string m_name;

  /// the parent montool
  ToolHandle<GenericMonitoringTool>* m_monTool;

  /// the tool children 
  std::vector<std::unique_ptr<IPlotBase>> m_tools;

};

}

#endif // IDTPM2_AnalysisR4_H
