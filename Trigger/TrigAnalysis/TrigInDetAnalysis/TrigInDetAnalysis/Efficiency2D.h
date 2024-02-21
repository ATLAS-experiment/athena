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
    m_hnumer->Fill( float(x), float(y), float(w) );
    m_hdenom->Fill( float(x), float(y), float(w) );
  }

  void FillDenom( double x, double y, float w=1) { 
    m_hmissed->Fill(float(x), float(y), float(w));
    m_hdenom->Fill(float(x), float(y), float(w));
  }
  

  /// evaluate the uncertainties correctly ...

  TGraphAsymmErrors* BayesX(int slice, double scale=100) { 
    
    if ( slice<0 || slice>=slicesX() ) return 0; 
    
    TH1D* hn = 0;
    TH1D* hd = 0;
    
    if ( m_hnumer ) { 
      // get some slice from the numerator histo
      hn = m_hnumer->ProjectionY( slicename( name()+"_yslice", slice).c_str(), slice+1, slice+1, "e");
    }
    
    if ( m_hdenom ) { 
      // get some slice from the denominator histo
      hd = m_hdenom->ProjectionY( slicename( name()+"_yslice", slice).c_str(), slice+1, slice+1, "e");
    }
    
    return BayesInternal( hn, hd, scale );
    
  }
 


  TGraphAsymmErrors* BayesY(int slice, double scale=100) { 

    if ( slice<0 || slice>=slicesY() ) return 0; 

    TH1D* hn = 0;
    TH1D* hd = 0;

    if ( m_hnumer ) { 
      // get some slice from the numerator histo
      hn = m_hnumer->ProjectionX( slicename( name()+"_yslice", slice).c_str(), slice+1, slice+1, "e");
    }

    if ( m_hdenom ) { 
      // get some slice from the denominator histo
      hd = m_hdenom->ProjectionX( slicename( name()+"_yslice", slice).c_str(), slice+1, slice+1, "e");
    }

    return BayesInternal( hn, hd, scale );
     
  }
 

  TH1D* sliceX( int i ) { 
    if ( i<0 || i>=slicesX() ) return 0; 
    return m_heff->ProjectionY( slicename( name()+"_xslice", i).c_str(), i+1, i+1, "e");
  }

  TH1D* sliceY( int i ) { 
    if ( i<0 || i>=slicesY() ) return 0; 
    return m_heff->ProjectionX( slicename( name()+"_yslice", i).c_str(), i+1, i+1, "e");
  }


  int slicesX() const { 
    if ( m_hdenom ) return m_hdenom->GetXaxis()->GetNbins();
    return 0;
  }


  int slicesY() const { 
    if ( m_hdenom ) return m_hdenom->GetYaxis()->GetNbins();
    return 0;
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


  std::string slicename( const std::string& s, int i ) const { 
    if ( i<10 ) return s + "0" + std::to_string(i);
    else        return s + std::to_string(i);
  }

};



  

inline std::ostream& operator<<( std::ostream& s, const Efficiency2D&  ) { 
  return s;
}


#endif  // TIDA_EFFICIENCY2D_H 










