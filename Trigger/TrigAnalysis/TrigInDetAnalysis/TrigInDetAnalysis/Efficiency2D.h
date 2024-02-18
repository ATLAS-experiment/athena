/* emacs: this is -*- c++ -*- */
/**
 **     @file    Efficiency2D.h
 **
 **     @author  mark sutton
 **     @date    Mon 15 Feb 2024 18:45 GMT 
 **
 **     Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 **/    


#ifndef TIDA_EFFICIENCY2D_H
#define TIDA_EFFICIENCY2D_H

#include "T_Efficiency.h"
#include "TH2F.h"


class Efficiency2D : public T_Efficiency<TH2F> {
  
public:
  
  Efficiency2D(TH2F* h, const std::string& n="") :
    T_Efficiency<TH2F>( h, n ) { }  
  
  Efficiency2D(TH2F* hnum, TH2F* hden, const std::string& n, double scale=100) :
    T_Efficiency<TH2F>( hnum, hden, n, scale ) { 
    getibinvec(true);
    finalise( scale );
  }  
  
  ~Efficiency2D() { } 

  /// fill methods ...
    
  void Fill( double x, double y, double w=1) {
    int ibin = m_hnumer->FindBin(float(x),float(y)); 
    m_hnumer->Fill(ibin,float(w));
    m_hdenom->Fill(ibin,float(w));
  }

  void FillDenom( double x, double y, float w=1) { 
    int ibin = m_hdenom->FindBin(float(x),float(y)); 
    m_hdenom->Fill(ibin,float(w));
    m_hmissed->Fill(ibin,float(w));
  }
  

  /// evaluate the uncertainties correctly ...

  TGraphAsymmErrors* Bayes(int slice, double scale=100) { 

    TH1F* hn = 0; // get some slice from the numerator histo
    TH1F* hd = 0; // get some slice from the denominator histo

    /// an operation to hopefully shut up the 
    /// compilker warnings because I haven;t got 
    /// round to implementing the functionality yet 
    slice++; 

    return BayesInternal( hn, hd, scale );
     
  }
 
protected:
 
  virtual void getibinvec(bool force=false) { 
    if ( !force && !m_ibin.empty() ) return;
    for ( int i=1 ; i<=m_hdenom->GetNbinsX() ; i++ ) { 
      for ( int j=1 ; j<=m_hdenom->GetNbinsY() ; j++ ) { 
	m_ibin.push_back( m_hdenom->GetBin(i,j) );
      }
    }
  }

};



  

inline std::ostream& operator<<( std::ostream& s, const Efficiency2D&  ) { 
  return s;
}


#endif  // TIDA_EFFICIENCY2D_H 










