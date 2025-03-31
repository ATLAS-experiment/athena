/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "dqm_algorithms/AFP_Sync_check.h"

#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <dqm_core/AlgorithmManager.h>
#include "dqm_core/AlgorithmConfig.h"
#include <dqm_core/exceptions.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TFile.h>

namespace {
    static dqm_algorithms::AFP_Sync_check instance;
}

dqm_algorithms::AFP_Sync_check::AFP_Sync_check() {
    dqm_core::AlgorithmManager::instance().registerAlgorithm( "AFP_Sync_check", this );
}

dqm_algorithms::AFP_Sync_check::~AFP_Sync_check() {
}

dqm_algorithms::AFP_Sync_check*
dqm_algorithms::AFP_Sync_check::clone() {
    return new AFP_Sync_check();
}

dqm_core::Result*
dqm_algorithms::AFP_Sync_check::execute( const std::string& name,
                                            const TObject& object,
                                            const dqm_core::AlgorithmConfig& config ) {
    if ( !object.IsA()->InheritsFrom( "TProfile" ) ) {
        throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TProfile" );
    }

    auto histogram = static_cast<const TProfile*>( &object );

    auto gthreshold = static_cast<uint32_t>( dqm_algorithms::tools::GetFromMap( "FractionBadLBs", config.getGreenThresholds() ) );
    auto rthreshold = static_cast<uint32_t>( dqm_algorithms::tools::GetFromMap( "FractionBadLBs", config.getRedThresholds() ) );
    auto dif_limit   = static_cast<float>( dqm_algorithms::tools::GetFirstFromMap( "dif_limit", config.getParameters() ) );

    std::vector<double> bad_errs;
    std::vector<int> bad_lbs;
    int nonZerocounter = 0;
    double percentBadBins = -10.0;

    for (int i = 1; i <= 2000; i++)
    {
        if (abs(histogram->GetBinContent(i)) >= dif_limit)
        {
            bad_errs.push_back( histogram->GetBinContent(i) );
            bad_lbs.push_back(i);
        }
        if (histogram->GetBinContent(i) != 0)
            nonZerocounter++;
    }
    percentBadBins = double( bad_errs.size() )/double(nonZerocounter)*100;

    auto result = new dqm_core::Result();

    // publish problematic bins
    result->tags_[ "% Bad bins " ] = percentBadBins;
    for ( int i = 0; i < int(bad_errs.size()); ++i ) 
    {
        auto tag    = ( std::ostringstream() << "LB " << bad_lbs[i] ).str();
        result->tags_[ tag ] = bad_errs[i];
    }

    if ( nonZerocounter == 0 )
        result->status_ = dqm_core::Result::Undefined;
    else if ( percentBadBins > rthreshold )
        result->status_ = dqm_core::Result::Red;
    else if ( percentBadBins > gthreshold )
        result->status_ = dqm_core::Result::Yellow;
    else
        result->status_ = dqm_core::Result::Green;

    return result;
}

void dqm_algorithms::AFP_Sync_check::printDescriptionTo( std::ostream& out ) {
    out << "AFP_Sync_check: Print out fraction of bad bins where module/station is out of synchronization\n"
        << "Required Parameter: dif_limit: threshold for content of the individual bin to be assumed out of sync" << std::endl;
}
