/* emacs: this is -*- c++ -*- */
/**
 **     @file    TrackDistributions.h
 **
 **     @author  mark sutton
 **     @date    Sun 18 Jan 2009 19:08:11 GMT 
 **
 **     Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 **/


#include <iostream>

#include "TrigInDetAnalysis/TrackAnalysis.h"

// class Track;
// class Effplot;
// class Resplot;

#include "TH1D.h"

class TrackDistributions : public TrackAnalysis { 
   
public:
  
  using TrackAnalysis::execute;
  
public:
  
  TrackDistributions( const std::string& name ) :
    TrackAnalysis( name )
  { }  
  
  virtual void initialise();

  virtual void execute(const std::vector<TIDA::Track*>& tracks1,
		       const std::vector<TIDA::Track*>& tracks2,
		       TrackAssociator* matcher );

  virtual void finalise();
  
private:

  TH1D*   m_heta = nullptr;
  TH1D*   m_hphi = nullptr;
  TH1D*   m_hz0 = nullptr;
  TH1D*   m_hd0 = nullptr;
  TH1D*   m_hpT = nullptr;

  TH1D*   m_hdeta = nullptr;
  TH1D*   m_hdphi = nullptr;
  TH1D*   m_hdz0 = nullptr;
  TH1D*   m_hdd0 = nullptr;
  TH1D*   m_hdpT = nullptr;

  TH1D*   m_hchi2 = nullptr;

  TH1D*   m_hblayer = nullptr;
  TH1D*   m_hpixel = nullptr;
  TH1D*   m_hsct = nullptr;
  TH1D*   m_hsilicon = nullptr;
  TH1D*   m_hstraw = nullptr;
  TH1D*   m_htr = nullptr;
  
};


