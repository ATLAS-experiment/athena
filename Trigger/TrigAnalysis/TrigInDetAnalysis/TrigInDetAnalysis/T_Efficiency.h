/* emacs: this is -*- c++ -*- */
/**
 **     @file    T_Efficiency.h
 **
 **     @author  mark sutton
 **     @date    Mon 26 Oct 2009 01:22:40 GMT 
 **
 **     Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef TIDA_T_EFFICIENCY_H
#define TIDA_T_EFFICIENCY_H

#include <iostream>
#include <string>
#include <cmath>

#include "TPad.h"
#include "TH1.h"
#include "TGraphAsymmErrors.h"

template<typename T>
class T_Efficiency {

public:

  T_Efficiency(T* h, const std::string& n) {

    std::string effname = n;
    if ( effname=="" )  { 
      effname = std::string(h->GetName())+"_eff";
    }
    
    m_name = effname;

    m_hnumer = (T*)h->Clone( (effname+"_n").c_str() );
    m_hdenom = (T*)h->Clone( (effname+"_d").c_str() );
    m_heff   = (T*)h->Clone( effname.c_str() );

    m_hmissed = (T*)h->Clone( (effname+"_missed").c_str() );

    m_hnumer->Reset();
    m_hdenom->Reset();
    m_heff->Reset();
    m_hmissed->Reset();
    
  } 


  T_Efficiency(T* hnum, T* hden, const std::string& n, double  ) {

    std::string effname = n+"_eff";
    
    m_name = effname;

    m_hnumer = (T*)hnum->Clone( (effname+"_n").c_str() );
    m_hdenom = (T*)hden->Clone( (effname+"_d").c_str() );
    m_heff   = (T*)hnum->Clone( effname.c_str() );

    m_hmissed = (T*)hnum->Clone( (effname+"_missed").c_str() );

    m_heff->Reset();
    m_hmissed->Reset();

    /// can't call finialise here - need to call it in 
    /// the derived class
    ///    this->finalise(scale);
  } 

  virtual ~T_Efficiency() { } 

  std::string name() const { return m_name; } 

  T* Hist() { return m_heff; }


  /// actually calculate the efficiencies 
 
  void finalise(double scale=100) { 
    getibinvec();
    m_heff->Reset();
    for ( size_t i=0 ; i<m_ibin.size() ; i++ ) { 
      double n = m_hnumer->GetBinContent(m_ibin[i]);
      double d = m_hdenom->GetBinContent(m_ibin[i]);
      
      double e  = 0;
      double ee = 0;
      if ( d!=0 ) { 
	e  = n/d; 
	ee = e*(1-e)/d; 
      } 
      
      // need proper error calculation...
      m_heff->SetBinContent( m_ibin[i], scale*e );
      m_heff->SetBinError( m_ibin[i], scale*std::sqrt(ee) );

    }
  }


  /// these 1D and 2D Fill versionms should never be called directly
  /// in fact, are they even needed at all ? 

  // 1D fill 
  //  virtual void Fill( double x, double w=1) { } 

  //  virtual void FillDenom( double x, float w=1) { }

  // 2D fill
  //  virtual void Fill( double x, double y, double w=1) { } 

  //  virtual void FillDenom( double x, double y, float w=1) { }
 
#if 0

  /// are these SetNumerator/Denominator metyhods 
  ///really ever needed ?
  
  void SetDenominator( T* h ) {
    getibinvec();
    for ( size_t i=0 ; i<m_ibin.size() ; i++ ) { 
      m_hdenom->SetBinContent( m_ibin[i], h->GetBinContent(m_ibin[i]) );
      m_hdenom->SetBinError( m_ibin[i], h->GetBinError(m_ibin[i]) );
    }
  }

  virtual void SetNumerator( T* h ) {
    getibinvec();
    for ( size_t i=0 ; i<m_ibin.size() ; i++ ) {
      m_hnumer->SetBinContent( m_ibin[i], h->GetBinContent(m_ibin[i]) );
      m_hnumer->SetBinError( m_ibin[i], h->GetBinError(m_ibin[i]) );
    }
  }

#endif

  /// calculate the efficiencies ...

  double findTotalEfficiency() {

    getibinvec();

    double n_tot = 0;
    double d_tot = 0;
    
    for ( size_t i=0 ; i<m_ibin.size() ; i++ ) { 
      double n = m_hnumer->GetBinContent(m_ibin[i]);
      double d = m_hdenom->GetBinContent(m_ibin[i]);
      
      n_tot += n;
      d_tot += d;
    }

    if ( d_tot!=0 ) {
      return n_tot / d_tot;
    }

    return 0.;
  }

  void Write() { 
    m_hnumer->Write();
    m_hdenom->Write();
    m_hmissed->Write();
    m_heff->Write();
  }

protected:

  virtual void getibinvec(bool force=false) = 0;

protected:

  TGraphAsymmErrors* BayesInternal( TH1* hn, TH1* hd, double scale=100) const { 

    /// stupid root, told to divide, it skips bins where the 
    /// nukber of entries is 0 (ok) but then complains that
    /// "number of points is not the same as the number of 
    /// bins" now that would be ok, *if these were user input
    /// values*, but is *stupid* if this is some root policy.
    /// : root decides to do X and then prints a warning 
    /// so instead, set the bin contents, for these bins to 
    /// something really, really, *really* tiny ... 

    for ( int i=1 ; i<=hd->GetNbinsX() ; i++ ) { 
      double y = hd->GetBinContent(i);
      if ( y==0 ) hd->SetBinContent(i, 1e-20);
    }

    TGraphAsymmErrors* tg = new TGraphAsymmErrors( hn, hd, "cl=0.683 b(1,1) mode" );


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
    
    return tg;

  }


protected:

  std::string m_name;

  T* m_hnumer;
  T* m_hdenom;

  T* m_hmissed;

  T* m_heff;
  
  std::vector<int> m_ibin;

};




#endif  // TIDA_T_EFFICIENCY_H 










