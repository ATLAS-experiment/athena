/* emacs: this is -*- c++ -*- */
/**
 **   @file    TRTPlots.h        
 **
 **            copy of class, whichis actually unclear ie
 **            do we want plots for test and reference ?
 **            of just one set, and we create two objects
 **            if we want both ? Why do we have a "Run3"
 **            set of plots, when they are really "TRT"
 **            plots - you want TRT hits, ask for TRT hits,
 **            not "Run3", but probably better to put them
 **            in a separate class, otherwise ...
 ** 
 **                   
 **   @author  sutt
 **   @date    Wed 22 Jul 2026 08:00:05 BST
 **
 **   $Id: TRTPlots.h, v0.0   Wed 22 Jul 2026 08:00:05 BST sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef  IDTPM2_TRTPLOTS_H
#define  IDTPM2_TRTPLOTS_H

#include "IPlotBase.h"

/// all these enums for all the histograms are really nasty
/// and the names for them, and making sure they are all consistent,
/// is even worse, so once and for all, create single place for the
/// enum and string names to be generated from - guarantees the
/// enum and string names, are always consistent, sadly there
/// seems no better way


  
#define TRT_PARAMS(X)  \
  X(NTRTHits)			\
  X(NTRTHitsXe)			\
  X(NTRTHitsAr)			\
  X(NTRTHighThresholdHits)	\
  X(NTRTHighThresholdHitsXe)	\
  X(NTRTHighThresholdHitsAr)	\
				\
  X(NTRTOutliers)		\
  X(NTRTHighThresholdOutliers)

    
namespace IDTPM2 {
  
class TRTPlots : public IPlotBase { 
    
public:

  TRTPlots( const std::string& name, ToolHandle<GenericMonitoringTool>* m );
  
  virtual ~TRTPlots() { } 

  void initialize();

  void execute( const TrackAnalysisStore& collections ) const;
  
  /// Print out final stats on histograms
  void finalize();

private:

  /// oh dear, just the (too many!) histogram variables
  /// (we will need these for the test and reference, so these should
  /// go inside test, and ref structs like the TrackParaters etc

  /// can esily add histograms vs mu if required 
  
  struct Histograms {

    /// ugly, but it gets the jobs done - this is the price you pay for have
    /// SO MANY STUPIDLY LONG NAMED PARAMETERS ...

    /// if we want them all with proper names, rather than just
    /// filling histograms with numbers ...

    enum TRTPar {
#      define X(x) x,
       TRT_PARAMS(X)
#      undef X
       NTRTPars
    };
    
    /// probably should use an std::array<string,NparNames>                                                                                                                                                   
    std::vector<std::string> m_parNames = {
#     define X(x) #x,
      TRT_PARAMS(X)
#     undef X
    };

    size_t            nParNames() const { return m_parNames.size(); } 

    const std::string&  parName(int i) const { return m_parNames.at(i); }
    
    void initialize( TRTPlots* p, const std::string& s="" );
    
    void  fill( const TrackView* t ) const;

#if 1
    /// actual stored histograms

    TIDA::Histogram<float> m_hits[NTRTPars];

    /// for these "2D" histograms, could use yet another enum, 
    /// and some parname vector, but prefer to keep it explicit
    /// for a low number of entries
    /// std::string m_parName[4] = { "pt", "eta", "phi", "d0" };

    TIDA::Histogram<float>  m_hits_vs_pt[NTRTPars];
    TIDA::Histogram<float>  m_hits_vs_eta[NTRTPars];
    TIDA::Histogram<float>  m_hits_vs_phi[NTRTPars];
    TIDA::Histogram<float>  m_hits_vs_d0[NTRTPars];

#else   

    /// this would give us actual properly named, individual
    /// histograms with the complete names, but then we would need
    /// to fill them all with an explicit call, or else add them
    /// all to some vector of Histograms to be filled,
    
#   define X(x) TIDA::Histogram<float> m_##x;
    TRT_PARAMS(X)
#   undef X

#   define X(x)  TIDA::Histogram2D<float> m_##x##_vs_pt;
    TRT_PARAMS(X)
#   undef X

#   define X(x)  TIDA::Histogram2D<float> m_##x##_vs_eta;
    TRT_PARAMS(X)
#   undef X

#   define X(x) TIDA::Histogram2D<float> m_##x##_vs_phi;
    TRT_PARAMS(X)
#   undef X

#   define X(x) TIDA::Histogram2D<float> m_##x##_vs_d0;
    TRT_PARAMS(X)
#   undef X
  
#endif
  
  };

private:

  /// actual class histogram sets - this avoid needlessly
  /// duplicating all the histograms with _ref, _test etc
  /// which becomes a huge pain to maintain   ...
  
  Histograms m_reference;
  Histograms m_test;
  
};

} // namespace IDTPM2


inline std::ostream& operator<<( std::ostream& s, const IDTPM2::TRTPlots&  ) { 
  return s;
}


#endif  // IDTPM2_TRTPLOTS_H 










