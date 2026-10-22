/**
 ** @file    Resolutions.cxx
 ** @author  sutt@cern.ch
 **
 **          based on a class by Thomas Strebler <thomas.strebler@cern.ch>
 **
 ** Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/



/** NB: no point making this an actual athena configurable class -
 **     better to make it an internal class of some global histogram
 **     class - although independent, then class HistogramDriver { ... }
 **     can containe any number of IHistogramFiller* and call them itself.
 **     then the python for the HistogramDriver can have separate
 **     functions to book the relevant histograms, so we can do
 **     function HistogramDriver_build:
 **        ...
 **        histogramFiller.filler = { filler1, filler2 }
 **
 **     and so on, and these become functions that create histograms
 **     and for the c++ code it can loop over the fillers, and call their
 **     histogram bookers itself. The only issue, is in the use of
 **     histogram creating functions from the GenericMonTool: maybe
 **     we pass in the HistogramDriver so that they can used the histogram
 **     creators from there, or else they would need to begime
 **     GenericMonTools themselves. That would actually be more complicted,
 **     since to add a new tool, you would need to implement an entire new
 **     generic MonTool, but to add a helper class for the HistogramDriver,
 **     all you would need to do would be to add a basic python class
 **     which creates the histograms with a provided function, so less
 **     boiler plate, with simpler, and more flexible code. 
 **/

#include "AsgMessaging/StatusCode.h"
#include "AthenaBaseComps/AthMessaging.h"

#include "GaudiKernel/SystemOfUnits.h"

#include "InDetPhysValMonitoring/ResolutionHelper.h"

/// local include(s)
#include "Resolutions.h"
#include "TrackView.h"

#include "MsgMacros.h"

#include <set>



//// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM2::Resolutions::Resolutions( const std::string& name, ToolHandle<GenericMonitoringTool>* m ) 
  : IPlotBase( name, m ), m_method(0) 
{ }



/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------
void IDTPM2::Resolutions::initialize()
{
  MY_MSG_DEBUG <<  "Booking resolutions: " << name()  << endmsg; /// !!!! ARGH !! NO WAY !!!
  
  for( unsigned int i=1; i<NPARAMS; i++ ) {
    
    m_pull[i]         = TIDA::Histogram<float>(   monTool(), "pull_"+m_paramName[i] );
    
    m_pull[i]         = TIDA::Histogram<float>(   monTool(), "pull_"+m_paramName[i] );
    m_res[i]          = TIDA::Histogram<float>(   monTool(), "res_"+m_paramName[i] );
    m_sigma[i]        = TIDA::Histogram<float>(   monTool(), "sigma_"+m_paramName[i] );
    m_significance[i] = TIDA::Histogram<float>(   monTool(), "significance_"+m_paramName[i] );
    m_corr[i]         = TIDA::Histogram2D<float>( monTool(), "corr_test_vs_ref_"+m_paramName[i] ); /// corr ? correlation ???

    m_resolution_vs_pt[i]  = TIDA::Histogram2D<float>( monTool(), "2D", "resolution_"+m_paramName[i]+"_vs_pt" );
    m_resolution_vs_eta[i] = TIDA::Histogram2D<float>( monTool(), "2D", "resolution_"+m_paramName[i]+"_vs_eta" );
    m_resolution_vs_z0[i]  = TIDA::Histogram2D<float>( monTool(), "@D", "resolution_"+m_paramName[i]+"_vs_z0" );
    
    /// loop all vs variables
    for( unsigned int j=0; j<NPARAMSOUT; j++ ) {

      /// not even sure what these histograms are for ...
      /// resolutions
      m_resHelper[i][j] = TIDA::Histogram<float>( monTool(), "resHelper_"+m_paramName[i]+"_vs_"+m_paramName[j] );
      m_reswidth[i][j]  = TIDA::Histogram<float>( monTool(), "resolution_"+m_paramName[i]+"_vs_"+m_paramName[j] );
      m_resmean[i][j]   = TIDA::Histogram<float>( monTool(), "resmean_"+m_paramName[i]+"_vs_"+m_paramName[j] );
      
      /// pulls
      m_pullHelper[i][j] = TIDA::Histogram<float>( monTool(), "pullHelper_"+m_paramName[i]+"_vs_"+m_paramName[j] ); /// pull helper ???
      m_pullmean[i][j]   = TIDA::Histogram<float>( monTool(), "pullmean_"+m_paramName[i]+"_vs_"+m_paramName[j] );
      m_pullwidth[i][j]  = TIDA::Histogram<float>( monTool(), "pullwidth_"+m_paramName[i]+"_vs_"+m_paramName[j] );
      
    } // close NPARAMSOUT loop
  } // close NPARAMS loop
 
}


/// -----------------------------
/// --- Dedicated fill method ---
/// -----------------------------

/// this stuff is insanely complicated, and unattractive

namespace { 
  double deltaPhi( double dphi ) {
    while ( dphi>M_PI  ) dphi -= 2*M_PI;
    while ( dphi<-M_PI ) dphi += 2*M_PI;
    return dphi;
  }
}


void IDTPM2::Resolutions::execute( const TrackAnalysisStore& collections ) const {
  
  /// all the IDTPM fillers take a "weight". Where does this "weight" come from ? 
  /// do we even need it ?
  ///  double weight = 1;
  
  
  /// --- reference and test tracks  ---                                                                        
  /// don't actually need these here, as we are explicitly iterating over the
  /// matched pairs to avoid all the lookups
  
  //  const std::vector<const TrackView*>& refTracks  = collections.reference_tracks();
  //  const std::vector<const TrackView*>& testTracks = collections.test_tracks();
  
  /// track matcher
  /// just loop over the matched tracks from here ...
  const TIDA::Associator<const TrackView>* matcher = collections.matcher();
  
  std::array<float,NPARAMS>       refP = {};
  std::array<float,NPARAMS>  refErrorP = {};
  
  std::array<float,NPARAMS>      testP = {};
  std::array<float,NPARAMS> testErrorP = {};
  
  
  /// we can get all the matches pairs directly ...
  
  const TIDA::Associator<const TrackView>::map_type&  matches = matcher->matched();
  
  /// then can just iterate over the match pairs rather than
  /// look anything up ...
  
  TIDA::Associator<const TrackView>::map_type::const_iterator itr = matches.begin();

  for ( ; itr!=matches.end() ; itr++ ) {  //cppcheck-suppress postfixOperator
    
    const TrackView* ref  = itr->first;
    const TrackView* test = itr->second;
    
    //  for ( size_t i=0 ; i<refTracks.size() ; i++ ) { 
    
    /// this "fill a vector, then for the tools from the vector"
    /// approach is quite nasty
    /// would it be better to define the array once outside the
    /// loop and over write ? More denagerous, but faster, better
    /// still to just use a vector<float> anyway then we can clear()
    /// for each track  
    
    getTrackParameters(  *ref,  refP,  refErrorP );
    getTrackParameters( *test, testP, testErrorP );

    /// this is why I don't like this NPARAMS enum appraoch -
    /// now you have to start indexing from 1, rather than 0,
    /// so you need to know about the enum stucture, really 
    /// the loop should be
    ///   for ( unsigned i=PT ; i<NPARAMS ; i++ )   
    
    for( unsigned i=1 ; i<NPARAMS ; i++ ) {
      
      float residual = testP[i] - refP[i];

      if( i==PHI ) residual = deltaPhi( testP[i] - refP[i] ); // angular difference
      
      /// nope, should test is testError!=0 and only fill the pulls etc if
      /// the error is set to a non-zero value, than can also be used for
      /// truth particles, jets, clusers etc

      if ( testErrorP[i]!=0 )  { 
	
	float pull = residual/testErrorP[i];
	
	float sigma_test        = testErrorP[i];

	float significance_test = testP[i]/testErrorP[i];
	
	m_pull[i].Fill( pull ); // , weight );

	/// do we really want the significance for eg phi ? ie
	/// phi/dphi ? clealy we only want the significance for
	/// valiables that might have a resolution comparable to
	/// the actual values
	m_significance[i].Fill( significance_test ); // , weight );

	if ( i==QOVERPT ) {
	  /// this isn't the residual, we really need 1/testP[i] - 1/refP[i]; 
	  /// but is testP[i] = 1/qpt, then just testP[i]-refP[i] is
	  /// what we should be using, this 
	  residual    = testP[i]/refP[i] - 1;    // Relative q/pt resolution
	  sigma_test *= 1/std::fabs( testP[i] ); // relative q/pt error
	}

	m_sigma[i].Fill( sigma_test ); //, weight );

      }
		
      //    m_corr[i].Fill( refP[i], testP[i], weight ); /// !!! 2D histograms not implemented yet - next thing to do

      m_res[i].Fill( residual ); //, weight );

      m_corr[i].Fill( refP[i], testP[i] );

      m_resolution_vs_pt[i].Fill(  ref->pt()*0.001, residual );
      m_resolution_vs_eta[i].Fill( ref->eta(),      residual );
      m_resolution_vs_z0[i].Fill(  ref->z0(),       residual );
      
      /// why do we only have the residuals and pulls vs "inclusive", "eta" and "pt" ??
      /// surely we ALSO want these vs, z0, d0 and (for confirmation) vs phi
      /// but then we would ALSO want them versus the number of tracks,
      /// number of vertices, and also versus <mu> 
      // for( unsigned int j=0; j<NPARAMSOUT; j++ ) {
      //      m_resHelper[i][j],Fill(  refP[j], residual ); //, weight ); 
      //      m_pullHelper[i][j].Fill( refP[j], pull );     //, weight ); 
      // }
    }
  }
}



/// finalize 
/// NB: actually this CAN'T be called from finalize, since if we create and histograms
///     in the finalize() and register them with the HistSvc for some reason athena
///     crashes in the clean up, even if we setDirectory(0) on the histograms, so
///     all the finalisation processing much take place in the stop() method
///     so this has to be called indirectly by the algoritm stop() 

void IDTPM2::Resolutions::finalize() {

  MY_MSG_DEBUG <<  "Finalising resolution plots: " << name() << endmsg;

  std::string realpath = realPath( "/Resolutions/" ); 
  
  std::string names[3] = { "pt", "eta", "z0" };
    
  for( unsigned i=1 ; i<NPARAMS ; i++ ) {
    for ( int j=0 ; j<3 ; j++ ) {
      processResolution( realpath+"resolution_"+m_paramName[i]+"_vs_"+names[j]+"/" );
    }
  }
  
#if 0
  IDPVM::ResolutionHelper resolutionHelper;
  IDPVM::ResolutionHelper::methods method = IDPVM::ResolutionHelper::methods( m_method );

  /// still need to sort out these helpers and what they
  /// actually are doing - essentially it is the same as the Resplot but with
  /// less functionality and a more complicted interface
  for( unsigned int i=1; i<NPARAMS; i++ ) {
    for( unsigned int j=0; j<NPARAMSOUT; j++ ) {
      
      std::cout << "\t"  <<  m_resHelper[i][j].name() << std::endl;
      std::cout << "\t"  <<  m_reswidth[i][j].name()  << std::endl;
      std::cout << "\t"  <<  m_resmean[i][j].name()   << std::endl;
      
      std::cout << "\t"  <<  m_pullHelper[i][j].name() << std::endl;
      std::cout << "\t"  <<  m_pullwidth[i][j].name()  << std::endl;
      std::cout << "\t"  <<  m_pullmean[i][j].name()   << std::endl;	
      
      //      resolutionHelper.makeResolutions(  m_resHelper[i][j], m_reswidth[i][j], m_resmean[i][j],  method );
      //      resolutionHelper.makeResolutions( m_pullHelper[i][j], m_pullwidth[i][j, m_pullmean[i][j], method );
    }
  }
#endif
  
}
 

/// --------------------------
/// --- getTrackParameters ---
/// --------------------------
void IDTPM2::Resolutions::getTrackParameters( const TrackView& p,
					      std::array<float,NPARAMS>& params,
					      std::array<float,NPARAMS>& errors ) const
{
  /// parameters

  params.fill(0);
  
  params[ D0 ]      = p.d0();
  params[ Z0 ]      = p.z0();
  params[ QOVERP ]  = p.qOverP() * Gaudi::Units::GeV;
  params[ QOVERPT ] = p.qOverP()*std::sin(p.theta()) * Gaudi::Units::GeV;
  params[ THETA ]   = p.theta();
  params[ PHI ]     = p.phi();
  params[ PT ]      = p.pt() / Gaudi::Units::GeV;
  /// do we really want to store such a value ? in the wrapper, we will likely
  /// calculate it inside the function so it is no less helpfull to calculate it
  /// explicitly
  params[ Z0SIN ]     = p.z0()*std::sin(p.theta()); 
  params[ ETA ]       = p.eta();
  params[ INCLUSIVE ] = 1;

  errors.fill(0);
  
  if ( p.is<xAOD::TrackParticle>() ) { 
    /// no point testing for all these if it is not a track particle
    /// uncertainties
    errors[ D0 ]        =  p.dd0();      //error( p, Trk::d0 );
    errors[ Z0 ]        =  p.dz0();      //, Trk::z0 );
    errors[ QOVERP ]    =  p.dqOverP();   // p, Trk::qOverP ) * Gaudi::Units::GeV;
    
    // errors[ QOVERPT ]   = qOverPTError( p ) * Gaudi::Units::GeV;
    errors[ QOVERPT ]   =  0;   // for the time being
    
    errors[ THETA ]     =  p.dtheta();   // error( p, Trk::theta );
    errors[ PHI ]       =  p.dphi();     // error( p, Trk::phi );
    errors[ PT ]        =  p.dpt();      // pTError( p ) / Gaudi::Units::GeV;
    //   errors[ Z0SIN ]     = z0SinThetaError( p ); ///
    errors[ Z0SIN ]     =  0; /// for the time being ...
    errors[ ETA ]       =  p.deta();     // etaError( p );
    errors[ INCLUSIVE ] =  0;
  }
}


