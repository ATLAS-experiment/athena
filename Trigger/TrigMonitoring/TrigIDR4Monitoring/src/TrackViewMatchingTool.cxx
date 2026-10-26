/**
 **     @file    TrackViewMatchingTool.cxx
 **
 **     @author  hsimpson
 **     @date    Mon 30 Mar 2026
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#include "TrackViewMatchingTool.h"
#include "TBestMatch.h"
#include "OfflineTruthMatcher.h"

#include "MsgMacros.h"

template<typename T, typename R>
IDTPM2::TMatchingTool<T,R>::TMatchingTool( const std::string& type,
                                    const std::string& name,
                                    const IInterface* parent )
    : AthAlgTool( type, name, parent )
{ }


template<typename T, typename R>
StatusCode IDTPM2::TMatchingTool<T,R>::initialize() {

    MY_MSG_DEBUG <<  "TMatchingTool: type=" << m_matcherType << endmsg;

    if      ( m_matcherType == "DeltaR"   )        m_matcher = std::make_unique<TDeltaRMatcher<T,R>>(          "DeltaRMatcher",          require_set(m_dr)    );
    else if ( m_matcherType == "DeltaPhi" )        m_matcher = std::make_unique<TDeltaPhiMatcher<T,R>>(        "DeltaPhiMatcher",        require_set(m_dphi)  );
    else if ( m_matcherType == "Sigma"    )        m_matcher = std::make_unique<TSigmaMatcher<T,R>>(           "SigmaMatcher",           require_set(m_dsigma) );
    else if ( m_matcherType == "SecondBestpT" )    m_matcher = std::make_unique<TSecondBestpTMatcher<T,R>>(    "SecondBestpTMatcher",    require_set(m_dpt)   );
    else if ( m_matcherType == "OfflineTruth" )    m_matcher = std::make_unique<TOfflineTruthMatcher<T,R>>(    "OfflineTruthMatcher",    require_set(m_prob)  );
    else if ( m_matcherType == "DeltaRZ"  )        m_matcher = std::make_unique<TDeltaRZMatcher<T,R>>(         "DeltaRZMatcher",         require_set(m_deta), require_set(m_dphi), require_set(m_dzed) );
    else if ( m_matcherType == "DeltaRZSinTheta" ) m_matcher = std::make_unique<TDeltaRZSinThetaMatcher<T,R>>( "DeltaRZSinThetaMatcher", require_set(m_deta), require_set(m_dphi), require_set(m_dzed) );
    else if ( m_matcherType == "Default" ) {
      float dr = 0.01;
      if ( m_dr.isset() ) dr = m_dr;
      m_matcher = std::make_unique<TDeltaRMatcher<T,R>>( "DeltaRMatcher", dr );
    }
    else {
        MY_MSG_FATAL <<  "Unknown matcher type: " << m_matcherType  << endmsg;
        return StatusCode::FAILURE;
    }
    
    return StatusCode::SUCCESS;
}


template<typename T, typename R>
std::unique_ptr<TIDA::Associator<T,R> >
IDTPM2::TMatchingTool<T,R>::clone() const { return m_matcher->uclone(); }

template<typename T, typename R>
std::unique_ptr<TIDA::Associator<T,R>>
IDTPM2::TMatchingTool<T,R>::uclone() const { return m_matcher->uclone(); }


/// Explicit instantiation for the TrackView specialisation
template class IDTPM2::TMatchingTool<const TrackView>;
