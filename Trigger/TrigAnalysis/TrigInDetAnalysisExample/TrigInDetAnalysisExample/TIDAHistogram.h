/// emacs: this is -* c++ -*-
/**
 **   @file         TIDAHistogram.h  
 **
 **   @author       sutt  
 **   @date         Sun  2 Jan 2022 06:57:06 GMT  
 ** 
 **   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef   TIDA_HISTOGRAM_H
#define   TIDA_HISTOGRAM_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "AthenaMonitoringKernel/Monitored.h"

#include <string>
#include <stdexcept>

namespace TIDA {
 
class HistogramBase { 

public:
  
  HistogramBase() : m_monTool(0), m_name("UNINITIALISED"), m_varname(), m_initialised(false) { } 
  
  HistogramBase( ToolHandle<GenericMonitoringTool>* m, const std::string& name, const std::string& domain="" ) :
    m_monTool(m), m_name(name), m_varname(domain.empty() ? name : domain+"_"+name), m_initialised(false)  { 
    if ( !m_name.empty() && m_monTool ) m_initialised = true;
  } 

  const std::string&    name() const { return m_name; }
  const std::string& varname() const { return m_varname; }

  bool           initialised() const { return m_initialised; }
  
  const ToolHandle<GenericMonitoringTool>* monTool() const { return m_monTool; };

private:

  ToolHandle<GenericMonitoringTool>* m_monTool;

  std::string m_name;
  std::string m_varname;

  bool        m_initialised;
  
};


template<typename T> 
class Histogram : public HistogramBase { 

public:

  using HistogramBase::HistogramBase;
  
  void Fill( T d ) const {
    if ( !initialised() ) throw std::runtime_error("TIDA::Histogram not initialised: "+name());
    auto s = Monitored::Scalar<T>( varname(), d ); 
    Monitored::Group( *monTool(), s );
  }

  void Fill( T d, T w ) const {
    if ( !initialised() ) throw std::runtime_error("TIDA::Histogram not initialised: "+name());
    auto s  = Monitored::Scalar<T>( varname(), d ); 
    auto sw = Monitored::Scalar<T>( varname()+"_weight", w ); 
    Monitored::Group( *monTool(), s, sw );
  }

  const Histogram* operator->() const { return this; }
  
};


template<typename T,typename U=T> 
class Histogram2D : public HistogramBase { 

public:

  using HistogramBase::HistogramBase;
  
  void Fill( T x, U y ) const {
    if ( !initialised() ) throw std::runtime_error("TIDA::Histogram not initialised: "+name());
    auto sx = Monitored::Scalar<T>( varname()+"__x", x ); 
    auto sy = Monitored::Scalar<T>( varname()+"__y", y ); 
    Monitored::Group( *monTool(), sx, sy );
  }

  void Fill( T x, U y, T w) const {
    if ( !initialised() ) throw std::runtime_error("TIDA::Histogram not initialised: "+name());
    auto sx  = Monitored::Scalar<T>( varname()+"__x", x ); 
    auto sy  = Monitored::Scalar<T>( varname()+"__y", y ); 
    auto sw  = Monitored::Scalar<T>( varname()+"_weight", w ); 
    Monitored::Group( *monTool(), sx, sy, sw );
  }
  
  const Histogram2D* operator->() const { return this; }
  
};


}


#endif  /* TIDA_HISTOGRAM_H */










