/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "dqm_algorithms/AFP_ToFEfficiency.h"

#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <dqm_core/AlgorithmManager.h>
#include "dqm_core/AlgorithmConfig.h"
#include <dqm_core/exceptions.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>

namespace {
    static dqm_algorithms::AFP_ToFEfficiency instance;
}

dqm_algorithms::AFP_ToFEfficiency::AFP_ToFEfficiency() {
    dqm_core::AlgorithmManager::instance().registerAlgorithm( "AFP_ToFEfficiency", this );
}

dqm_algorithms::AFP_ToFEfficiency::~AFP_ToFEfficiency() {
}

dqm_algorithms::AFP_ToFEfficiency*
dqm_algorithms::AFP_ToFEfficiency::clone() {
    return new AFP_ToFEfficiency();
}

dqm_core::Result*
dqm_algorithms::AFP_ToFEfficiency::execute( const std::string& name,
                                            const TObject& object,
                                            const dqm_core::AlgorithmConfig& config ) {
    if ( !object.IsA()->InheritsFrom( "TH2" ) ) {
        throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH2" );
    }

    auto histogram = static_cast<const TH2D*>( &object );

    auto gthreshold = static_cast<uint32_t>( dqm_algorithms::tools::GetFromMap( "NbadTrains", config.getGreenThresholds() ) );
    auto rthreshold = static_cast<uint32_t>( dqm_algorithms::tools::GetFromMap( "NbadTrains", config.getRedThresholds() ) );
    auto eff_limit   = static_cast<float>( dqm_algorithms::tools::GetFirstFromMap( "eff_limit", config.getParameters() ) );

    uint32_t each_tr_status[2] = {};
    for (uint32_t i = 1; i < 5; ++i)
    {
        float current_eff = histogram->GetBinContent(histogram->GetBin(5,i));
        if (current_eff >= eff_limit)
            each_tr_status[0]++;
        else
            each_tr_status[1]++;
    }

    auto result = new dqm_core::Result();

    // publish problematic bins
    result->tags_[ ("N Trains with efficiency less than " + (std::to_string(eff_limit)).substr(0,4)).c_str() ] = each_tr_status[1];

    if ( each_tr_status[0]==0 && each_tr_status[1]==0 )
        result->status_ = dqm_core::Result::Undefined;
    else if ( each_tr_status[0] == 4 )
        result->status_ = dqm_core::Result::Green;
    else if ( each_tr_status[1] >= gthreshold && each_tr_status[1] < rthreshold )
        result->status_ = dqm_core::Result::Yellow;
    else
        result->status_ = dqm_core::Result::Red;

    return result;
}

void dqm_algorithms::AFP_ToFEfficiency::printDescriptionTo( std::ostream& out ) {
    out << "AFP_ToFEfficiency: Print out how many trains have efficiency less than limit\n"
        << "Required Parameter: eff_limit: threshold percentage of efficiency" << std::endl;
}
