/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "dqm_algorithms/AFP_ToFSiTCorrCheck.h"

#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <dqm_core/AlgorithmManager.h>
#include "dqm_core/AlgorithmConfig.h"
#include <dqm_core/exceptions.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>

namespace {
    static dqm_algorithms::AFP_ToFSiTCorrCheck instance;
}

dqm_algorithms::AFP_ToFSiTCorrCheck::AFP_ToFSiTCorrCheck() {
    dqm_core::AlgorithmManager::instance().registerAlgorithm( "AFP_ToFSiTCorrCheck", this );
}


dqm_algorithms::AFP_ToFSiTCorrCheck*
dqm_algorithms::AFP_ToFSiTCorrCheck::clone() {
    return new AFP_ToFSiTCorrCheck();
}

dqm_core::Result*
dqm_algorithms::AFP_ToFSiTCorrCheck::execute( const std::string& name,
                                            const TObject& object,
                                            const dqm_core::AlgorithmConfig& config ) {
    if ( !object.IsA()->InheritsFrom( "TH2" ) ) {
        throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH2" );
    }

    auto histogram = static_cast<const TH2*>( &object );
    
    if ( histogram->GetDimension() > 2 ) {
        throw dqm_core::BadConfig( ERS_HERE, name, "histogram has more than 2 dimensions" );
    }

    auto gthreshold_tr = static_cast<uint32_t>( dqm_algorithms::tools::GetFromMap( "NbadTrains", config.getGreenThresholds() ) );
    auto rthreshold_tr = static_cast<uint32_t>( dqm_algorithms::tools::GetFromMap( "NbadTrains", config.getRedThresholds() ) );
    auto gthreshold_st = static_cast<uint32_t>( dqm_algorithms::tools::GetFromMap( "NbadStairs", config.getGreenThresholds() ) );
    auto rthreshold_st = static_cast<uint32_t>( dqm_algorithms::tools::GetFromMap( "NbadStairs", config.getRedThresholds() ) );
    auto pronounciation_level_down   = static_cast<double>( dqm_algorithms::tools::GetFirstFromMap( "pronounciation_level_down", config.getParameters() ) );
    auto pronounciation_level_up   = static_cast<int>( dqm_algorithms::tools::GetFirstFromMap( "pronounciation_level_up", config.getParameters() ) );
    const std::string RANGES = dqm_algorithms::tools::GetFirstFromMap( "RANGES", config.getGenericParameters() );

    //from string of ranges to int array
    int y_bins[8] = {};
    std::string s = RANGES;
    std::string delimiter = ",";
    uint32_t pos = 0;
    s.erase(s.begin());
    s.pop_back();
    for (uint32_t i = 0; i < 8; ++i) 
    {
        pos = s.find(delimiter);
        y_bins[i] = stoi(s.substr(0, pos));
        s.erase(0, pos + delimiter.length());
    } 

    //from 2D hist to projections of each train and get bin content 
    const std::string trains[4] = {"0", "1", "2", "3"};
    auto projections_tr = std::vector<TH1D *>(4);
    int max_bins[4] = {};
    float max_bins_content[4] = {};
    float all_bins_content[4][4] = {};

    for (uint32_t i = 0; i < 4; ++i)
    {
        projections_tr[i] = new TH1D(("projections_tr"+trains[i]).c_str(),("projections_tr"+trains[i]).c_str(), 16, 0, 4);
        projections_tr[i] = histogram->ProjectionX(("projections_tr"+trains[i]).c_str(), y_bins[i*2], y_bins[i*2+1], "cutg");
        projections_tr[i]->Rebin(4);
        max_bins[i] = (projections_tr[i]->GetMaximumBin())-2;
        max_bins_content[i] = (projections_tr[i]->GetBinContent(max_bins[i]+2));
        for (size_t j = 0; j < 4; j++)
            all_bins_content[i][j] = (projections_tr[i]->GetBinContent(j+2));
    }   

    //check if any train is not max
    uint32_t false_bins = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (max_bins[i] != i)
            false_bins++;
    }
    uint32_t status_bins = 0;
    if (false_bins == 0)
        status_bins = 1; 
    else if (false_bins == gthreshold_tr)
        status_bins = 2;
    else if (false_bins >= rthreshold_tr)
        status_bins = 3;

    //check on how good pronounce stairs are
    float check_devided_bins[4][4] = {};
    int false_bins_devided[4] = {};
    uint32_t false_bins_devided_tr = 0;
    for (uint32_t i = 0; i < 4; ++i)
    {
        for (uint32_t j = 0; j < 4; ++j)
            if (i != j) 
            {
                check_devided_bins[i][j] = max_bins_content[i]/all_bins_content[i][j];
                if ((check_devided_bins[i][j] < pronounciation_level_down) || (check_devided_bins[i][j] > pronounciation_level_up))
                    false_bins_devided[i]++;
            }  
        if (false_bins_devided[i] > 1)
            false_bins_devided_tr++;        
    }
    uint32_t status_bins_devided = 0;
    if (false_bins_devided_tr == 0)
        status_bins_devided = 1;
    else if ((false_bins_devided_tr >= gthreshold_st) && (false_bins_devided_tr < rthreshold_st))
        status_bins_devided = 2;
    else if (false_bins_devided_tr >= rthreshold_st)
        status_bins_devided = 3;

    //get general status
    uint32_t status_general = 0;
    if (status_bins_devided > status_bins) {status_general = status_bins_devided;}
    else {status_general = status_bins;}

    auto result = new dqm_core::Result();

    // publish problematic bins
    result->tags_[ "N Trains on wrong positions/not existing" ] = false_bins;
    result->tags_[ "N Stairs not pronounced" ] = false_bins_devided_tr;

    if ( status_general == 0 )
        result->status_ = dqm_core::Result::Undefined;
    else if ( status_general == 3 )
        result->status_ = dqm_core::Result::Red;
    else if ( status_general == 2 )
        result->status_ = dqm_core::Result::Yellow;
    else if ( status_general == 1 )
        result->status_ = dqm_core::Result::Green;

    return result;
}

void dqm_algorithms::AFP_ToFSiTCorrCheck::printDescriptionTo( std::ostream& out ) {
    out << "AFP_ToFSiTCorrCheck: Print out how many stairs are not on their places and how many are not pronounced\n"
        << "Required Parameter: pronounciation_level_down: how pronounced the stair is (down limit)\n" 
        << "Required Parameter: pronounciation_level_up: how pronounced the stair is (upper limit)\n" 
        << "Required Parameter: RANGES: ranges of SiT, corresponding to ToF trains, in bins of hist"<< std::endl;
}
