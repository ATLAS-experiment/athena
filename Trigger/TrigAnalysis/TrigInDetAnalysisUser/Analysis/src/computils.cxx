/**
 **     @file    computils.cxx
 **
 **     @author  mark sutton
 **     @date    Sat Aug 30 2014 14:38:03 CEST  
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

// cppcheck-suppress-file stlIfStrFind; cannot use C++20 starts_with in this standalone code

#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <glob.h>
#include <stdint.h>

#include <iostream>
#include <string>
#include <vector>

#include "label.h"
#include "DrawLabel.h" 

#include "TFile.h"
#include "TKey.h"
#include "TTree.h"
#include "TList.h"
#include "TObject.h"
#include "TDirectory.h"
#include "TH1D.h"
#include "TLine.h"

#include "TLegend.h"
#include "TColor.h"

#include "computils.h"

bool LINEF = true;
bool LINES = false;


bool JLflag = false;

bool Plots::s_watermark = true;

int   colours[6] = {  1,    2, kBlue-4,  6, kCyan-2,  kMagenta+2 };
int   markers[6] = { 20,   24,      25, 26,      25,          22 };
double msizes[6] = {  0.85,  1,       1,  1,       1,           1 };


void band_intersect( double& x0, double& x1, double& y0, double& y1, double ylo, double yhi ) { 
  
  double x0t = x0;
  double x1t = x1;

  double y0t = y0;
  double y1t = y1;

  if ( ylo!=-999 ) { 
    if ( y0>ylo && y1<ylo) { 
      double y =  ylo;
      double x = (ylo-y0)*(x1-x0)/(y1-y0)+x0;
      x1t = x;
      y1t = y;
    }
    
    if ( y0<ylo && y1>ylo) { 
      double y =  ylo;
      double x = x1-(y1-ylo)*(x1-x0)/(y1-y0);
      x0t = x;
      y0t = y;
    }

    if ( y0<ylo && y1<ylo) { 
      y0t = ylo;
      y1t = ylo;
    }

    x0 = x0t;
    x1 = x1t;
    y0 = y0t;
    y1 = y1t;

    return;
  }

  if ( yhi!=-999 ) { 
    if ( y0<yhi && y1>yhi) { 
      double y =  yhi;
      double x = (yhi-y0)*(x1-x0)/(y1-y0)+x0;
      x1t = x;
      y1t = y;
    }
    
    if ( y0>yhi && y1<yhi) { 
      double y =  yhi;
      double x = x1-(y1-yhi)*(x1-x0)/(y1-y0);
      x0t = x;
      y0t = y;
    } 

    x0 = x0t;
    x1 = x1t;
    y0 = y0t;
    y1 = y1t;

    return;
  }

}


void band_plot( TH1* h, double xlo, double xhi, double ylo, double yhi ) { 

  //  bool first = true;
  bool last = true;

  int ifirst = -1; 
  //  int ilast  = -1;

  for ( int i=h->GetNbinsX()+1 ; i-- ;  ) {
    double yt = h->GetBinContent(i);
    if ( last && ( yt<ylo || ( yt==0 && ylo==0 ) ) ) continue;
    // ilast = i+1;
    last = false;
    break;
  }

  if ( JLflag ) { 
    //  if ( contains( std::string(h->GetName()), "vs_mu") ) ilast = h->FindBin(80);
    if ( contains( std::string(h->GetName()), "vs_mu") )  h->GetXaxis()->SetRangeUser(   18,  58 );
    if ( contains( std::string(h->GetName()), "a0_eff") ) h->GetXaxis()->SetRangeUser( -190, 200 );
    // if ( contains( std::string(h->GetName()), "a0") )     h->GetXaxis()->SetRangeUser( -200, 200 );
  }

  for ( int i=1 ; i<=h->GetNbinsX()+1 ; i++ ) {
    
    double x0 = h->GetBinCenter(i);
    double x1 = h->GetBinCenter(i+1);

    if ( JLflag ) { 
      if ( contains( std::string(h->GetName()), "vs_mu") && x0<18 ) continue;
      if ( contains( std::string(h->GetName()), "vs_mu") && x1>58 ) continue;
      
      if ( contains( std::string(h->GetName()), "a0_eff") && x0<-200 ) continue;
      if ( contains( std::string(h->GetName()), "a0_eff") && x1> 200 ) continue;
    }
    
    //    if ( first && ( _y<ylo || ( _y==0 && ylo==0 ) ) ) continue;

    if ( ifirst<0 ) ifirst = i;

    //    if ( ilast>0 && (i+1)>ilast ) continue;

    //    first = false;

    double x0_1 = x0;
    double x1_1 = x1;

    if ( xlo!=-999 && x0<xlo ) continue;
    if ( xhi!=-999 && x1>xhi ) continue;

    double y0_up = h->GetBinContent(i)   + h->GetBinError(i);
    double y1_up = h->GetBinContent(i+1) + h->GetBinError(i+1);
    double y0_lo = h->GetBinContent(i)   - h->GetBinError(i);
    double y1_lo = h->GetBinContent(i+1) - h->GetBinError(i+1);

    band_intersect( x0,   x1,   y0_up, y1_up, ylo, yhi );
    band_intersect( x0_1, x1_1, y0_lo, y1_lo, ylo, yhi );
   
    TLine* t0 = new TLine( x0, y0_up, x1, y1_up );
    TLine* t1 = new TLine( x0_1, y0_lo, x1_1, y1_lo );

    t0->SetLineColor( h->GetLineColor() );
    t0->Draw();

    t1->SetLineColor( h->GetLineColor() );
    t1->Draw();

    //    std::cout << h->GetName() << "\tx1: " << x1 << std::endl;

  }

  //  if ( contains( std::string(h->GetName()), "vs_mu") ) ilast = h->FindBin(80);
  
  // if ( ilast>0 && ifirst>0 ) h->GetXaxis()->SetRange( ifirst, ilast );

  if ( h->GetLineColor()>10 )  h->SetFillStyle(3395);
  else                         h->SetFillStyle(3354);

  h->SetFillColor(h->GetLineColor());
  //  h->DrawCopy("same e3lhist" );
  

}




double  Entries( TH1* h ) {
  double n = 0;
  for ( int i=h->GetNbinsX()+1 ; --i ; ) n += h->GetBinContent(i);
  return n;
}


double integral( TH1* h ) { 
  double n=0;
  for ( int i=h->GetNbinsX() ; i>0 ; i-- ) n += h->GetBinContent(i);
  return n;
}



void Norm( TH1* h, double scale, double xmin, double xmax ) {
  double n = 0;

  if ( xmin==0 && xmax==0 ) {   
    for ( int i=h->GetNbinsX()+2 ; --i ; ) n += h->GetBinContent(i);
  }  
  else { 
    for ( int i=h->GetNbinsX()+2 ; --i ; ) { 
      double x = h->GetBinCenter(i);
      if ( x>xmin && x<xmax ) n += h->GetBinContent(i);
    }
  }
  
  if ( n!=0 ) {
    double in=scale/n;
    for ( int i=h->GetNbinsX()+2 ; --i ; ) {
      h->SetBinContent(i, h->GetBinContent(i)*in );
      h->SetBinError(i, h->GetBinError(i)*in );
    }
  }
  
}



true_mean::true_mean( TH1F* h ) : 
  m_mean(0), m_error(0) { 
  
  double f   = 0;
  double fx  = 0;
  double fx2 = 0;

  for ( int i=0 ; i<h->GetNbinsX() ; i++ ) {
    double w = h->GetBinLowEdge(i+2)-h->GetBinLowEdge(i+1);
    f   += h->GetBinContent(i+1)*w;
    fx  += h->GetBinContent(i+1)*w*h->GetBinCenter(i+1);
    fx2 += h->GetBinContent(i+1)*w*h->GetBinCenter(i+1)*h->GetBinCenter(i+1);
  }

  if ( f!=0 ) {   
    m_mean  = fx/f;
    m_error = std::sqrt( (fx2/f - m_mean*m_mean )/f );
  }    

}






 
union floaty_t {
    floaty_t( float n = 0.0f ) : f(n) {}

    /// portable extraction of components
    /// sign
    bool negative() const { return i < 0; }

    /// float has 23 bit mentissa
    int32_t raw_mantissa() const { return i & ((1 << 23) - 1); }
    /// and an 8 bit exponent 
    int32_t raw_exponent() const { return (i >> 23) & 0xff; }
 
    int32_t i;
    float   f;
};
 

bool almost_equal( floaty_t a, floaty_t b, int max_diff ) {

  // Check for trivial equality to make sure +0==-0
  if ( a.f == b.f ) return true;
  
  // Different signs means they do not match.
  if ( a.negative() != b.negative() ) return false;
  
  // Find the difference in last place units
  int ulps_diff = std::abs( a.i - b.i );
  if (ulps_diff <= max_diff) return true;
  
  return false;
}


bool almost_equal( float a, float b, int max_diff ) {
    return almost_equal( floaty_t(a), floaty_t(b), max_diff );
}


bool operator==( floaty_t a, floaty_t b ) { 
  /// use a maximum 5 float separation between the two - could be more precise
  return almost_equal( a, b, 5 ); 
}


void trim_tgraph( TH1* h, TGraphAsymmErrors* t ) {
  
  double ylo = h->GetMinimum();

  int ih=1; 

  for ( int i=0 ; i<t->GetN() && ih<=h->GetNbinsX() ; i++, ih++ ) { 
   
    double yt = 0;
    double xt = 0;
    double ye = 0;

    t->GetPoint( i, xt, yt );
    ye = t->GetErrorYlow( i );

    double yh = h->GetBinContent(ih);
    double xh = h->GetBinCenter(ih);

    while( !almost_equal( xh, xt, 5 ) && ih<=h->GetNbinsX() ) { 
      ih++;
      yh = h->GetBinContent(ih);
      xh = h->GetBinCenter(ih);
    }

    if ( !almost_equal( yh, yt, 5 ) ) throw data_mismatch(std::string("for histogram ")+h->GetName());

    if ( (yt-ye) < ylo ) { 
      h->SetBinContent(ih, ylo-100 );
      t->SetPoint( i, xt, ylo-10000 ); 
    }

  }
}


void ATLASFORAPP_LABEL( double x, double  y, int color, double size ) 
{
  TLatex* lat  = new TLatex(); //lat.SetTextAlign(12); lat.SetTextSize(tsize); 
  lat->SetNDC();
  lat->SetTextFont(72);
  lat->SetTextColor(color);
  lat->SetTextSize(size);
  lat->DrawLatex(x,y,"ATLAS");

  TLatex* lat2 = new TLatex(); //lat.SetTextAlign(12); lat.SetTextSize(tsize);   
  lat2->SetNDC();
  lat2->SetTextFont(52);
  lat2->SetTextColor(color);
  lat2->SetTextSize(size);  // this 0.13 should really be calculated as a ratio of the width of the canvas
  lat2->DrawLatex(x+0.13,y,"For Approval"); 
}

void myText( Double_t x, Double_t y, Color_t color, const std::string& text, Double_t tsize) {
  //Double_t tsize=0.05;
  TLatex lat; lat.SetTextAlign(12); lat.SetTextSize(tsize); 
  lat.SetNDC();
  lat.SetTextColor(color);
  lat.DrawLatex(x,y,text.c_str());
}


std::string stime() { 
  time_t t;
  time(&t);
  return label("%s", ctime(&t) );
}


/// contains a string
bool contains( const std::string& s, const std::string& p) { 
  return (s.find(p)!=std::string::npos);
}


bool contains( const std::string& s, char p) noexcept { 
  return (s.find(p)!=std::string::npos);
}


/// contains a string at the *beginning* of the string
bool fcontains( const std::string& s, const std::string& p) { 
  return (s.find(p)==0);
}


double plotable( TH1* h ) { // , double xlo, double xhi ) {
  double n = 0;
    
  double xlo = h->GetBinLowEdge(1);
  double xhi = h->GetBinLowEdge(h->GetNbinsX()+1);

  //  if ( xlo!=-999 ) _xlo = xlo;
  //  if ( xhi!=-999 ) _xhi = xhi;

  for ( int i=h->GetNbinsX()+1 ; --i ; ) {
    if ( h->GetBinCenter(i)>xlo && h->GetBinCenter(i)<xhi ) n += h->GetBinContent(i);
  } 
  return n;
}



bool exists( const std::string& filename ) { 
  struct stat sb;
  if ( stat( filename.c_str(), &sb)==0 ) return true; // && S_ISREG(sb.st_mode ))
  else return false;
}



std::string globbed( const std::string& s ) { 
  /// glob for a file based on the pattern, then return the name 
  /// of the first matching file
  
  glob_t glob_result;
  glob( s.c_str(), GLOB_TILDE, 0, &glob_result );

  std::vector<std::string> ret;
  for( unsigned i=0 ; i<glob_result.gl_pathc ; i++ ){
    ret.push_back( std::string(glob_result.gl_pathv[i]) );
  }
  globfree(&glob_result);

  
  if ( ret.empty() ) { 
    std::cerr << "no matching file: " << s << std::endl;
    return "";
  }  

  if ( ret.size()>1 ) { 
    for ( unsigned i=0 ; i<ret.size() ; i++ ) { 
      std::cout << "matched " << ret[i] << std::endl;
    }
  }    

  return ret[0];
}





bool empty( TH1* h ) { 
  for ( int i=h->GetNbinsX() ; i>0 ; i-- ) if ( h->GetBinContent(i)!=0 ) return false;
  return true;
}


std::string tail( std::string s, const std::string& pattern ) { 
  size_t pos = s.find(pattern);
  while ( pos != std::string::npos ) { 
    s.erase( 0, pos+1 ); 
    pos = s.find(pattern);
  }
  return s;
}


std::string head( std::string s, const std::string& pattern ) {
  size_t pos = s.find_last_of(pattern);
  if ( pos != std::string::npos ) {
    s.erase( pos, s.size() );
  }
  return s;
}


void contents( std::vector<std::string>&  keys, TDirectory* td, 
	       const std::string& directory, const std::string& pattern, const std::string& path ) { 
  std::vector<std::string> patterns;
  patterns.push_back(pattern);
  contents( keys, td, directory, patterns, path );
}




void contents( std::vector<std::string>&  keys, TDirectory* td, 
	       const std::string& directory, const std::vector<std::string>& patterns, const std::string& path ) { 

  bool print = false;
  
  TList* tl  = td->GetListOfKeys();
  
  for ( int i=tl->GetSize() ; i-- ; ) {

    TKey* tobj = (TKey*)tl->At(i);

    if ( tobj==0 ) continue;
    
    if ( std::string(tobj->GetClassName()).find("TDirectory")!=std::string::npos ) { 
      
      TDirectory* tnd = (TDirectory*)tobj->ReadObj();
      
      /// directory, cd to it ...
      std::string dname = tnd->GetName();
      
      std::string newpath = path+dname+"/";
      contents( keys, tnd, directory, patterns, newpath );
    
    }
    else { 
      /// test to see whether we are searhing for a specific directory name
      /// not a directory so include this ...
      if ( directory == "" || contains( path, directory ) ) {
	
	
	bool matched = true;
	for ( size_t i=patterns.size() ; i-- ; ) { 
	  std::string pattern = patterns[i];  
	  if ( contains(std::string(tobj->GetName()), pattern ) )  matched &=true;
	  else matched = false;
	}
	if ( matched ) { 
	  if ( print ) std::cout << "will process " << td->GetName() << " \t:: " << tobj->GetName() << "\tpatterns: " << patterns.size() << std::endl;
	  print = false;
	  keys.push_back( path+tobj->GetName() );
	}
      }
    }
  }
}





double realmax( TH1* h, bool include_error, double lo, double hi ) { 

  double rm = 0;
  if ( h->GetNbinsX()==0 )  return 0; 

  bool first = true;
  for ( int i=1 ; i<=h->GetNbinsX() ; i++ ) { 

    if ( lo!=hi ) { 
      double c = h->GetBinCenter(i);
      if ( lo>c || hi<c ) continue;
    }

    double re = h->GetBinContent(i);
    if ( include_error ) re += h->GetBinError(i);
    if ( re!=0 ) {
      if ( first || rm<re ) { 
	rm = re;
	first = false;
      }
    }
  }

  return rm;
}


// double realmin( TH1* h, bool include_error, double lo, double hi ) { 
double realmin( TH1* h, bool , double lo, double hi ) { 

  if ( h->GetNbinsX()==0 )  return 0; 

  double rm = 0;

  bool first = true;
  for ( int i=1 ; i<=h->GetNbinsX() ; i++ ) { 

    if ( lo!=hi ) { 
      double c = h->GetBinCenter(i);
      if ( lo>c || hi<c ) continue;
    }

    double re = h->GetBinContent(i);

    if ( re!=0 ) {
      //      if ( include_error ) re -= h->GetBinError(i);
      if ( first || rm>re ) rm = re;
      first = false;
    } 
  }

  return rm;
}


double hmean( TH1* h ) {
  double N = integral(h);
  double n=0;
  for ( int i=h->GetNbinsX() ; i>0 ; i-- ) { 
    n += h->GetBinContent(i);
    if ( 2*n>N ) return h->GetBinCenter(i);
  }
  return 0;
}




std::vector<int>  findxrange(TH1* h, bool symmetric ) { 

  int ilo = 1;
  int ihi = h->GetNbinsX();

  h->GetXaxis()->SetRange( ilo, ihi );

  std::vector<int> limits(2,0);
  limits[0] = ilo;
  limits[1] = ihi;

  if ( empty(h) ) return limits;

#if 1

  /// zoom on non-empty bins
  for ( ; ilo<=ihi ; ilo++ ) if ( h->GetBinContent(ilo)!=0 ) break; 
  for ( ; ihi>=ilo ; ihi-- ) if ( h->GetBinContent(ihi)!=0 ) break;

#else

  /// experimental zooming ...

  /// zoom on the central region containg more than some fraction X
 
  double icont = 1/content;
  
  double flo = 0;
  double fhi = 0;
  for ( ; ilo<=ihi ; ilo++ ) { 
    flo += h->GetBinContent(ilo);
    if ( (flo*icont)>0.0005 )  break;
  }
 
  for ( ; ihi>=ilo  ; ihi-- ) { 
    fhi += h->GetBinContent(ihi);
    if ( (fhi*icont)>0.0005 )  break;
  }

#endif 

  int delta_lo = ilo-1;
  int delta_hi = h->GetNbinsX()-ihi;

  if ( symmetric ) { 
    if ( delta_hi<delta_lo ) { 
      limits[0] = 1+delta_hi; 
      limits[1] = ihi; 
    }
    else { 
      limits[0] = 1+delta_lo; 
      limits[1] = h->GetNbinsX()-delta_lo;
    }
  }
  else { 
    
    if ( ilo>1 ) ilo--;
    if ( ihi<h->GetNbinsX() ) ihi++;
    
    limits[0] = ilo;
    limits[1] = ihi;
  }

  return limits; 

}



void xrange(TH1* h, bool symmetric ) { 
  std::vector<int> limits = findxrange( h, symmetric );
  h->GetXaxis()->SetRange( limits[0], limits[1] );
}




std::vector<double>  findxrangeuser(TH1* h, bool symmetric ) {
  
  std::vector<int> limits = findxrange( h, symmetric );

  std::vector<double> dlimits(2,0);

  double dx = h->GetBinLowEdge(limits[1]+1)-h->GetBinLowEdge(limits[1]);  

  dlimits[0] = h->GetBinLowEdge(limits[0]);  
  dlimits[1] = h->GetBinLowEdge(limits[1]+1)-dx*1e-11;  
  
  return dlimits;
}


void xrangeuser(TH1* h, bool symmetric ) { 
  std::vector<double> limits = findxrangeuser( h, symmetric );
  h->GetXaxis()->SetRangeUser( limits[0], limits[1] );
}



std::string findcell( std::string name, const std::string& regex, const std::string& splitex ) { 
  
  size_t posex = name.find( regex );
  
  if ( posex==std::string::npos ) return "";

  size_t pos = name.find_last_of( splitex );

  std::string duff = name;

  while ( pos!=std::string::npos && pos>posex+regex.size() ) { 
    name.resize(pos); //pos must be <=string length
    pos = name.find_last_of( splitex );
  }
  
  pos = name.find( regex );

  name = name.substr( pos, name.size() );

  pos = name.find( splitex );

  if ( pos!=std::string::npos ) return name.substr( 0, pos );

  return name; 
} 



std::string findrun( TFile* f ) { 

  TDirectory* here = gDirectory;
  
  f->cd();

  std::cout << "gDirectory::GetName() " << gDirectory->GetName() << std::endl;
  
  //  gDirectory->pwd();
  
  //  gDirectory->ls();
  
  TList* tl  = gDirectory->GetListOfKeys();
  
  /// go through sub directories

  for ( int i=0 ; i<tl->GetSize() ; i++ ) { 
    
    TKey* tobj = (TKey*)tl->At(i);
    
    if ( tobj==0 ) continue;
    
    if ( std::string(tobj->GetClassName()).find("TDirectory")!=std::string::npos ) { 
      
      TDirectory* tnd = (TDirectory*)tobj->ReadObj();
      
      std::string name = tnd->GetName();

      if ( name.find( "run_" )==0 ) { 
	here->cd();
	return name;
      }
    }
  }  

  here->cd();

  return "";
}




/// copy the release info TTree

void copyReleaseInfo( TFile* finput, TFile* foutdir ) { 

  std::vector<std::string> release_data;

  if ( finput && foutdir ) { 

    TTree* tree  = (TTree*)finput->Get("dataTree");    
    TTree* clone = tree->CloneTree();

    foutdir->cd();
    clone->Write("", TObject::kOverwrite);

    delete clone;

  }
  
}
