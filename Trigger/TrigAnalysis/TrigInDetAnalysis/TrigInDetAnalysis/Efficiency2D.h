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

#include <iostream>
#include <string>
#include <cmath>

#include "TH1F.h"
#include "TH2F.h"
#include "TPad.h"
#include "TGraphAsymmErrors.h"

class Efficiency2D {
  
public:

  Efficiency2D(TH2F* h, const std::string& n="") {

    std::string effname = n;
    if ( effname=="" )  { 
      effname = std::string(h->GetName())+"_eff";
    }
    
    m_name = effname;

    m_hnumer = (TH2F*)h->Clone( (effname+"_n").c_str() );
    m_hdenom = (TH2F*)h->Clone( (effname+"_d").c_str() );
    m_heff   = (TH2F*)h->Clone( effname.c_str() );

    m_hmissed = (TH2F*)h->Clone( (effname+"_missed").c_str() );

    m_hnumer->Reset();
    m_hdenom->Reset();
    m_heff->Reset();
    m_hmissed->Reset();
    
  } 


  Efficiency2D(TH2F* hnum, TH2F* hden, const std::string& n, double scale=100) {

    std::string effname = n+"_eff";
    
    m_name = effname;

    m_hnumer = (TH2F*)hnum->Clone( (effname+"_n").c_str() );
    m_hdenom = (TH2F*)hden->Clone( (effname+"_d").c_str() );
    m_heff   = (TH2F*)hnum->Clone( effname.c_str() );

    m_hmissed = (TH2F*)hnum->Clone( (effname+"_missed").c_str() );
    
    m_heff->Reset();
    m_hmissed->Reset();
    
    finalise(scale);
  } 


  ~Efficiency2D() { } 

  std::string name() const { return m_name; } 

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
  
  
  void SetDenominator( TH2F* h ) {
    for ( int i=0 ; i<=h->GetNbinsX()+1 ; i++ ) {
      for ( int j=0 ; j<=m_hdenom->GetNbinsY()+1 ; j++ ) { 	
	int ibin = m_hdenom->GetBin(i,j);
	m_hdenom->SetBinContent( ibin, h->GetBinContent(i) );
	m_hdenom->SetBinError( ibin, h->GetBinError(i) );
      }
    }
  }
  
  
  void SetNumerator( TH2F* h ) {
    for ( int i=0 ; i<=h->GetNbinsX()+1 ; i++ ) {
      for ( int j=0 ; j<=m_hdenom->GetNbinsY()+1 ; j++ ) { 	
	int ibin = m_hdenom->GetBin(i,j);
	m_hnumer->SetBinContent( ibin, h->GetBinContent(ibin) );
	m_hnumer->SetBinError( ibin, h->GetBinError(ibin) );
      }
    }
  }
  
  double findTotalEfficiency() {
    double n_tot = 0;
    double d_tot = 0;
    
    for ( int i=1 ; i<=m_hdenom->GetNbinsX() ; i++ ) { 
      for ( int j=1 ; j<=m_hdenom->GetNbinsY() ; j++ ) { 
	
	int ibin = m_hdenom->GetBin(i,j);
	double n = m_hnumer->GetBinContent(ibin);
	double d = m_hdenom->GetBinContent(ibin);
	
	n_tot += n;
	d_tot += d;
      }
    }      
    
    if ( d_tot!=0 ) {
      return n_tot / d_tot;
    }
    
    return 0.;
  }

  void finalise(double scale=100) { 
    m_heff->Reset();
    for ( int i=1 ; i<=m_hdenom->GetNbinsX() ; i++ ) { 
      for ( int j=1 ; j<=m_hdenom->GetNbinsY() ; j++ ) { 

	int ibin = m_hdenom->GetBin(i,j);

	double n = m_hnumer->GetBinContent(ibin);
	double d = m_hdenom->GetBinContent(ibin);
	
	double e  = 0;
	double ee = 0;
	if ( d!=0 ) { 
	  e  = n/d; 
	  ee = e*(1-e)/d; 
	} 
	
	// need proper error calculation...
	m_heff->SetBinContent( ibin, scale*e );
	m_heff->SetBinError( ibin, scale*std::sqrt(ee) );
	
      }
    }
  }

  TH2F* Hist() { return m_heff; }

  TGraphAsymmErrors* Bayes(int slice, double scale=100) { 
    
    TGraphAsymmErrors* tg = 0;
    
#if 0

    /// Not finished yet - will need to select a particular 
    /// slice in x or y to use for the calculated efficiency

    /// stupid root, told to divide, it skips bins where the 
    /// nukber of entries is 0 (ok) but then complains that
    /// "number of points is not the same as the number of 
    /// bins" now that would be ok, *if these were user input
    /// values*, but is *stupid* if this is some root policy.
    /// : root decides to do X and then prints a warning 
    /// so instead, set the bin contents, for these bins to 
    /// something really, really, *really* tiny ... 

    for ( int i=0 ; i<m_hdenom->GetNbinsX() ; i++ ) { 
      double y = m_hdenom->GetBinContent(i+1);
      if ( y==0 ) m_hdenom->SetBinContent(i+1, 1e-20);
    }

    tg = new TGraphAsymmErrors( m_hnumer, m_hdenom, "cl=0.683 b(1,1) mode" );


    double* x      = tg->GetX();
    double* y      = tg->GetY();
  
    int n = tg->GetN();

    for ( int i=0 ; i<n ; i++ ) { 
    
      y[i]      *= scale;
 
      double yeup   = tg->GetErrorYhigh(i);
      double yedown = tg->GetErrorYlow(i);
      
      yeup   *= scale;
      yedown *= scale;
   
      /// root is just such a pain - can't just get/set specific y values
      /// I mean - why *some* functions to get an x or y value and others 
      /// only get *all* the x values, but the functions to return the errors 
      /// only get ONE AT A TIME ?
      /// why isn't there a simple ScaleX() function? All this functionality 
      /// no one cares about, and basic functionality missing
 
      tg->SetPoint( i, x[i], y[i] ); 
      
      tg->SetPointEYhigh( i, yeup ); 
      tg->SetPointEYlow( i, yedown ); 

      tg->SetPointEXhigh( i, 0 ); 
      tg->SetPointEXlow( i, 0 ); 
      
    } 
    
#endif
    
    return tg;
    
  }
  
private:

  std::string m_name;

  TH2F* m_hnumer;
  TH2F* m_hdenom;

  TH2F* m_hmissed;

  TH2F* m_heff;
  
};



  

inline std::ostream& operator<<( std::ostream& s, const Efficiency2D&  ) { 
  return s;
}


#endif  // TIDA_EFFICIENCY2D_H 










