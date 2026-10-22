/**
 **   @file    IPlotBase.cxx        
 **                   
 **   @author  sutt
 **   @date    Wed  1 Jul 2026 08:14:36 BST
 **
 **   $Id: IPlotBase.cxx, v0.0   Thu 16 Jul 2026 12:14:36 BST sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/

#include "IPlotBase.h"
#include "MsgMacros.h"

#include "Resplot/Resplot.h"



void IDTPM2::IPlotBase::copyHist( TH1* hsrc, TH1* hdest, bool copyoverflow ) const {
  
  if ( hsrc->GetNbinsX()!=hdest->GetNbinsX() ) return; // do we want to give an error message ?? throw and exception ??

  int imin = 0;
  int imax = hsrc->GetNbinsX()+1; 

  if ( !copyoverflow ) {
    imin = 1;
    imax = hsrc->GetNbinsX(); 
  }
  
  for  ( int i=imin ; i<=imax ; i++ ) {
    hdest->SetBinContent( i, hsrc->GetBinContent(i) );
    hdest->SetBinError( i, hsrc->GetBinError(i) );
  }

  hdest->SetEntries( hsrc->GetEntries() );
}



void IDTPM2::IPlotBase::getHist( ServiceHandle<ITHistSvc>& histsvc, const std::string& hname, TH1* hsrc ) const {
  TH1* h = 0;
  if ( histsvc->getHist( hname, h ).isFailure() ) MY_MSG_WARNING << "Could not retieve histogram: " << hname << endmsg;
  else copyHist( hsrc, h );
}


void IDTPM2::IPlotBase::processResolution( ServiceHandle<ITHistSvc>& histsvc, const std::string& path, bool writeflag ) const {

  TH2* hist2d = 0;
  
  if ( histsvc->getHist( path+"2D", hist2d ).isFailure() ) MY_MSG_WARNING << "Could not retieve 2D: " << (path+"2D") << endmsg;
  else {
    
    /// process the histogramss
    /// NB: might be better if instead of adding the 2D histogram and then
    ///     copying the 1D hjistograms back, we passed in the 1D histogram 
    ///     pointers and filled them directly in the Resplot
    
    Resplot::Resplot rp( path, hist2d );

    rp.Finalise( Resplot::FitNull95 );

    if ( !writeflag ) {
      /// use the provides 1D, maan, sigme histograms etc
      /// would like to do this ...
      //  if ( histsvc->regHist( path+"1D",    rp.H1D()   ).isFailure() ) MY_MSG_WARNING << "Unable to register histogram: 1D:    " << name() << endmsg; 
      //  if ( histsvc->regHist( path+"mean",  rp.Mean()  ).isFailure() ) MY_MSG_WARNING << "Unable to register histogram: mean:  " << name() << endmsg; 
      //  if ( histsvc->regHist( path+"sigma", rp.Sigma() ).isFailure() ) MY_MSG_WARNING << "Unable to register histogram: sigma: " << name() << endmsg; 
      //  }
      //  else {
      /// use the provides 1D, maan, sigme histograms etc
      getHist( histsvc, path+"1D",    rp.H1D()   );
      getHist( histsvc, path+"mean",  rp.Mean()  );
      getHist( histsvc, path+"sigma", rp.Sigma() );
    }
  }

}


/// This is a horrific function, has to get ALL the histograms, just
/// to be able to work out how to get the actual path to the histograms
/// for this tool - there simply MUST be a better way ...
std::string IDTPM2::IPlotBase::realPath( const std::string& tag ) {

  std::string dir  = (*monTool())->getPath();

  /// can add on our "fixed" histogram "Vertex" domain
  std::string stub = dir.substr( dir.find('/', 1) + 1 ) + tag; 
  
  const std::vector<std::string>& allhists = histSvc()->getHists();

  std::string basestr = "";
  
  for ( size_t i=0 ; i<allhists.size() ; i++ ) {
    if ( allhists[i].find(stub)!=std::string::npos ) {
      std::string s = allhists[i];
      basestr = s.substr(0,s.find("/"+stub)+1);
      break;
    }
  }

  return basestr + stub;
  
}
  



 
#if 0

void IDTPM2::IPlotBase::binWidthNormalise( ServiceHandle<ITHistSvc>& histsvc, const std::string& hname ) const {
  TH1* h = 0;
  if ( histsvc->getHist( hname, h ).isFailure() ) MY_MSG_WARNING << "Could not retieve histogram: " << hname << endmsg;
  else binWidthNormalise( h ); 
}


void IDTPM2::IPlotBase::binWidthNormalize( TH1* h ) const {

  if ( !h ) return;
  
  int    nbins = h->GetNbinsX();
  double width = h->GetBinWidth(1);

  double eps = 1e-12;
  
  bool   variable = false;
  
  for ( int i=2 ; i<=nbins ; i++ ) { 
    if ( std::fabs( h->GetBinWidth(i)-width )>eps ) { 
	variable = true;
	break;
      }
  }
  
  if ( !variable ) return;
  
  for ( int i=1 ; i<=h->GetNbinsX() ; i++ ) { 
      double w = h->GetBinWidth(i);
      h->SetBinContent( i, h->GetBinContent(i)/w );  
      h->SetBinError(   i,   h->GetBinError(i)/w );  
  }
  
}

#endif
