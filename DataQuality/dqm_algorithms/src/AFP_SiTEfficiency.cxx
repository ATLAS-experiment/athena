/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "dqm_algorithms/AFP_SiTEfficiency.h"

#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <dqm_core/AlgorithmManager.h>
#include "dqm_core/AlgorithmConfig.h"
#include <dqm_core/exceptions.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TEfficiency.h>
#include <TFile.h>

namespace {
    static dqm_algorithms::AFP_SiTEfficiency instance;
}

dqm_algorithms::AFP_SiTEfficiency::AFP_SiTEfficiency() {
    dqm_core::AlgorithmManager::instance().registerAlgorithm( "AFP_SiTEfficiency", this );
}

dqm_algorithms::AFP_SiTEfficiency::~AFP_SiTEfficiency() {
}

dqm_algorithms::AFP_SiTEfficiency*
dqm_algorithms::AFP_SiTEfficiency::clone() {
    return new AFP_SiTEfficiency();
}

dqm_core::Result*
dqm_algorithms::AFP_SiTEfficiency::execute( const std::string& name,
                                            const TObject& object,
                                            const dqm_core::AlgorithmConfig& config ) {
    if ( !object.IsA()->InheritsFrom( "TEfficiency" ) ) {
        throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TEfficiency" );
    }

    auto histogram = static_cast<const TEfficiency*>( &object );

    auto gthreshold = static_cast<uint32_t>( dqm_algorithms::tools::GetFromMap( "NEfficiency", config.getGreenThresholds() ) );
    auto rthreshold = static_cast<uint32_t>( dqm_algorithms::tools::GetFromMap( "NEfficiency", config.getRedThresholds() ) );
    float efficiency = 0;

    TH1* h_total = histogram->GetCopyTotalHisto();
    TH1* h_passed = histogram->GetCopyPassedHisto();

    float n_total = float ( h_total->GetEntries() );
    float n_passed = float ( h_passed->GetEntries() );
    efficiency = n_passed/n_total*100;

    auto result = new dqm_core::Result();

    // publish problematic bins
    result->tags_[ "Plane efficiency = " ] = efficiency;

    if ( efficiency == 0 )
        result->status_ = dqm_core::Result::Undefined;
    else if ( efficiency > gthreshold )
        result->status_ = dqm_core::Result::Green;
    else if ( ( efficiency <= gthreshold ) && (efficiency > rthreshold) )
        result->status_ = dqm_core::Result::Yellow;
    else 
        result->status_ = dqm_core::Result::Red;

    return result;
}

void dqm_algorithms::AFP_SiTEfficiency::printDescriptionTo( std::ostream& out ) {
    out << "AFP_SiTEfficiency: Print out if general plane efficiency is less than limit"<< std::endl;
}
