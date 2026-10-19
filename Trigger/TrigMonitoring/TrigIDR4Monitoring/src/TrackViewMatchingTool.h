/* emacs: this is -*- c++ -*- */
/**
 **     @file    TrackViewMatchingTool.h
 **
 **     @brief   Templated Athena tool that creates and owns a single
 **              TIDA::Associator matcher.  Callers obtain per-event
 **              copies via clone().
 **
 **     @author  hsimpson
 **     @date    Mon 30 Mar 2026
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef IDTPM2_TRACKVIEWMATCHINGTOOL_H
#define IDTPM2_TRACKVIEWMATCHINGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigInDetAnalysis/TIDAAssociator.h"
#include "TrackView.h"

#include "ConfiguredValueUtils.h"

#include <memory>
#include <string>



namespace IDTPM2 { 

template<typename T, typename R=T>
class TMatchingTool : public AthAlgTool {

public:

    TMatchingTool( const std::string& type,
                   const std::string& name,
                   const IInterface* parent );

    virtual ~TMatchingTool() = default;

    virtual StatusCode initialize() override;

    /// Return a fresh clone of the template matcher.
    /// Caller owns the returned object.  Thread-safe, const.
    std::unique_ptr<TIDA::Associator<T,R> > clone() const;

    std::unique_ptr<TIDA::Associator<T,R> > uclone() const;

protected:

    Gaudi::Property<std::string> m_matcherType { this, "MatcherType", "Default",
        "Matcher algorithm: DeltaR, DeltaPhi, DeltaRZ, DeltaRZSinTheta, Sigma, SecondBestpT, Default" };

    /// NB: not all these parameters need to be set, only the appropriate ones for
    ///     the requested matcher
    ConfiguredValueProperty<float> m_dr     { this, "DeltaR",     {}, "Primary distance parameter" };
    ConfiguredValueProperty<float> m_deta   { this, "DeltaEta",   {}, "Delta eta parameter" };
    ConfiguredValueProperty<float> m_dphi   { this, "DeltaPhi",   {}, "Delta phi parameter" };
    ConfiguredValueProperty<float> m_dzed   { this, "DeltaZed",   {}, "Delta zed parameter" };
    ConfiguredValueProperty<float> m_dpt    { this, "DeltaPt",    {}, "Delta pt parameter"  };
    ConfiguredValueProperty<float> m_dsigma { this, "DeltaSigma", {}, "Delta sigma parameter" };
    ConfiguredValueProperty<float> m_prob   { this, "Prob",       {}, "Match probability"   };

    std::unique_ptr<TIDA::Associator<T,R> > m_matcher;

};



using TrackViewMatchingTool = TMatchingTool<const TrackView>;

}

#endif // IDTPM2_TRACKVIEWMATCHINGTOOL_H
