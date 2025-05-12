/* emacs: this is -*- c++ -*- */
/**
 **     @file    generate.h
 **
 **     @author  mark sutton
 **     @date    Fri 11 Jan 2019 07:06:39 CET 
 **
 **     Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef GENERATE_H
#define GENERATE_H


#include <cmath>
#include <vector>
#include <iostream>

#include "TH1D.h"

#include "BasicRandom.h"


namespace generate { 

/// mean and rms 
double get_mean( const std::vector<double>& m );
double get_rms( const std::vector<double>& m );


/// generate an integrated distribution
TH1D* integrate( TH1D* hin );

/// scale a distribution by the integral
void ScaleIntegral(TH1D* hin );

/// generate a (normalised) pdf from a distribution
TH1D* PDF(TH1D* hin);

/// smooth a distribution about the maximum
/// NB: maximum is not smoothed
TH1D* smooth( TH1D* hin, bool sym=true );


/// base class for a "generator" class
class generator_base { 
public:
  virtual ~generator_base() { }
  virtual double generate() const = 0;
};



/// breit-wigner generator
class breit_generator : public generator_base { 

 public: 
  
  virtual double generate() const {
    static BasicRandom r;
    double x = std::tan((r.uniform()-0.5)*M_PI);
    return x;
  }
  
};




/// generate a distribution according to an input 
/// histogram (after smoothing)
class hist_generator : public generator_base { 

public:

  hist_generator(TH1D* h, bool smooth_=true );
  
  hist_generator(const hist_generator &) = delete;
  hist_generator operator=(const hist_generator &) = delete;
  
  virtual ~hist_generator() { delete m_s; if ( m_random ) delete m_random; }

  /// actually generate a random number from the distribution
  double generate() const {
    return invert(m_random->uniform());
  }

  TH1D* histogram()       { return m_s; }
  TH1D* rawhistogram()    { return m_raw; }
  TH1D* smoothhistogram() { return m_smooth; }

private:

  int getbin( double y ) const {
    for ( unsigned i=0 ; i<m_y.size() ; i++ ) if ( y<=m_y[i] ) return i;
    return m_y.size()-1;
  } 
    
  double invert( double y ) const {
    int i = getbin(y);
    if ( i==0 ) return 0;
    else return (y-m_y[i-1])*m_dxdy[i-1]+m_x[i-1];
  }


private: 
 
  std::vector<double> m_y;
  std::vector<double> m_x;
  

  std::vector<double> m_dx;
  std::vector<double> m_dy;
  
  std::vector<double> m_dxdy;

  TH1D* m_s;   // accumated histogram
  TH1D* m_raw;    // raw histogram
  TH1D* m_smooth; // smoothed raw histogram

  BasicRandom* m_random;

};




/// given a histogram, get a generator for the distribution 
/// in that histogram, then run some pseudo-experiments and 
/// collect the mean and rms  

class experiment { 

public:

  experiment( TH1D* h, int Nexperiments=10, int fevents=0 );
  experiment(const experiment& ) = delete;
  experiment operator=(const experiment& ) = delete;

  double hmean() const { return m_hmean; }  
  double hrms()  const { return m_hrms; }  

  double mean()       const { return m_global_mean; }  
  double mean_error() const { return m_global_mean_error; }  

  double rms()       const { return m_global_rms; }  
  double rms_error() const { return m_global_rms_error; }  

  generator_base* gen() { return m_gen; }

  TH1D* rmshisto() { return m_THrms; } 
  
private:

  int m_N; /// number of experiments

  std::vector<double> m_mean;  /// means from each experiment
  std::vector<double> m_rms;   /// rms from each experiment

  double m_hmean; /// actual histogram mean95
  double m_hrms;  /// actual histogram rms95

  /// overall mean and rms from the pseudo experiments
  double m_global_mean;
  double m_global_mean_error;

  double m_global_rms;
  double m_global_rms_error;
    
  generator_base* m_gen;

  TH1D* m_THrms;
};



} // namespace generate

#endif // GENERATE_H
