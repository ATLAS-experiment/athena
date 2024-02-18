/* emacs: this is -*- c++ -*- */
/**
 **     @file    Efficiency1D.h
 **
 **     @author  mark sutton
 **     @date    Mon 26 Oct 2009 01:22:40 GMT 
 **
 **     Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef TIDA_EFFICIENCY1D_H
#define TIDA_EFFICIENCY1D_H

#include "T_Efficiency.h"
#include "TH1F.h"


class Efficiency1D : public T_Efficiency<TH1F> {

public:

  Efficiency1D(TH1F* h, const std::string& n="") :
    T_Efficiency<TH1F>( h, n ) { }  

  Efficiency1D(TH1F* hnum, TH1F* hden, const std::string& n, double scale=100) :
    T_Efficiency<TH1F>( hnum, hden, n, scale ) { 
    getibinvec(true);
    finalise( scale );
  }  

  ~Efficiency1D() { } 


  // fill methods ..

  virtual void Fill( double x, double w=1) { 
    m_hnumer->Fill(float(x),float(w));
    m_hdenom->Fill(float(x),float(w));
  }

  virtual void FillDenom( double x, float w=1) { 
    m_hdenom->Fill(float(x),float(w));
    m_hmissed->Fill(float(x),float(w));
  }

  /// evaluate the uncertainties correctly ...

  TGraphAsymmErrors* Bayes(double scale=100) { 
    return BayesInternal( m_hnumer, m_hdenom, scale );
  }

protected:

  virtual void getibinvec(bool force=false) { 
    if ( !force && !m_ibin.empty() ) return;
    for ( int i=1 ; i<=m_hdenom->GetNbinsX() ; i++ ) { 
      m_ibin.push_back( i );
    }
  }

};



inline std::ostream& operator<<( std::ostream& s, const Efficiency1D&  ) { 
  return s;
}


#endif  // TIDA_EFFICIENCY1D_H 










