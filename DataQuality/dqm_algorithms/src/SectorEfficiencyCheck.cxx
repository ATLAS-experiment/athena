/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include <dqm_algorithms/SectorEfficiencyCheck.h>

#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <dqm_core/AlgorithmManager.h>
#include "dqm_core/AlgorithmConfig.h"
#include <dqm_core/exceptions.h>

#include <TEfficiency.h>
#include <TH1.h>
#include <iostream>
#include <sstream>
#include <iomanip> // For std::fixed, std::setprecision

namespace {
    static dqm_algorithms::SectorEfficiencyCheck instance;
}

dqm_algorithms::SectorEfficiencyCheck::SectorEfficiencyCheck() {
    dqm_core::AlgorithmManager::instance().registerAlgorithm("SectorEfficiencyCheck", this);
}

dqm_algorithms::SectorEfficiencyCheck::~SectorEfficiencyCheck() {
}

dqm_algorithms::SectorEfficiencyCheck*
dqm_algorithms::SectorEfficiencyCheck::clone() {
    return new SectorEfficiencyCheck();
}

dqm_core::Result*
dqm_algorithms::SectorEfficiencyCheck::execute( const std::string& name,
                                            const TObject& object,
                                            const dqm_core::AlgorithmConfig& config ) {
    
    if ( !object.IsA()->InheritsFrom( "TEfficiency" ) ) {
        throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TEfficiency" );
    }

    auto efficiency_object = static_cast<const TEfficiency*>( &object );
    
    const TH1* h_total = efficiency_object->GetTotalHistogram();
    if (h_total->GetEntries() == 0) {
        auto result = new dqm_core::Result();
        result->status_ = dqm_core::Result::Undefined;
        return result;
    }


    int MIN_STATISTICS_CUT = 50.0;
    float EFF_THRESH_low = 0.50;
    float EFF_THRESH_medium = 0.80;
    float EFF_THRESH_high = 0.90;
    int N_Sect_low_max = 4;
    int N_Sect_medium = 5;
    int N_Sect_high_min = 6;
    try {
      MIN_STATISTICS_CUT = static_cast<int>( dqm_algorithms::tools::GetFirstFromMap(        "MinStat", config.getParameters() ) );
      EFF_THRESH_high = static_cast<float>( dqm_algorithms::tools::GetFirstFromMap(        "EffThreshHigh", config.getParameters()  ));
      EFF_THRESH_medium = static_cast<float>( dqm_algorithms::tools::GetFirstFromMap(        "EffThreshMedium", config.getParameters()  ));
      N_Sect_low_max = static_cast<float>( dqm_algorithms::tools::GetFirstFromMap(        "NSectLowMax", config.getParameters()  ));
      N_Sect_medium = static_cast<float>( dqm_algorithms::tools::GetFirstFromMap(        "NSectMedium", config.getParameters()  ));
      N_Sect_high_min = static_cast<float>( dqm_algorithms::tools::GetFirstFromMap(        "NSectHighMin", config.getParameters() ));
    }catch ( dqm_core::Exception & ex ) {
      MIN_STATISTICS_CUT = 50;
      EFF_THRESH_low = 0.50;
      EFF_THRESH_medium = 0.80;
      EFF_THRESH_high = 0.90;
      N_Sect_low_max = 4;
      N_Sect_medium = 5;
      N_Sect_high_min = 6;
    }

  
    int nLBs = h_total->GetNbinsX();
    int nSectors = h_total->GetNbinsY();
    
    bool red_flag_triggered = false;
    
    uint32_t total_low_eff_bins = 0;
    
    //loop on lb
    for (int x_lb_bin = 1; x_lb_bin <= nLBs; ++x_lb_bin) {
        
      int count_eff_lt_50 = 0;
      int count_eff_lt_80 = 0;
      int count_eff_lt_90 = 0;
        
	//loop on sectors
        for (int y_sector_bin = 1; y_sector_bin <= nSectors; ++y_sector_bin) {
            
            int bin_idx = h_total->GetBin(x_lb_bin, y_sector_bin);
            double total_events = h_total->GetBinContent(bin_idx);

            // min stat cut
            if (total_events > MIN_STATISTICS_CUT) {
                
                double bin_eff = efficiency_object->GetEfficiency(bin_idx);

                if (bin_eff < EFF_THRESH_high) {
                    count_eff_lt_90++;
                }
                if (bin_eff < EFF_THRESH_medium) {
                    count_eff_lt_80++;
                }
                if (bin_eff < EFF_THRESH_low) {
                    count_eff_lt_50++;
                    total_low_eff_bins++;
                }
            }
        } // sector loop

        // ---  RED Flag criteria ---

        if (count_eff_lt_90 >= N_Sect_high_min) {
            red_flag_triggered = true;
            break; 
        }
        else if (count_eff_lt_80 == N_Sect_medium) {
            red_flag_triggered = true;
            break; 
        }
        else if (count_eff_lt_50 >= 2 && count_eff_lt_50 <= N_Sect_low_max) {
            red_flag_triggered = true;
            break; 
        }

    } //  loop LB
    
    auto result = new dqm_core::Result();

    if (red_flag_triggered) {
        result->status_ = dqm_core::Result::Red;
    } else {
        result->status_ = dqm_core::Result::Green;
    }
    

    result->tags_[ "N_Bad_Bins_Lt_50_Percent" ] = (double)total_low_eff_bins;
    result->tags_[ "Red_Flag_Triggered" ] = (double)(red_flag_triggered ? 1.0 : 0.0);
    

    return result;
}

void dqm_algorithms::SectorEfficiencyCheck::printDescriptionTo( std::ostream& out ) {
    out << "SectorEfficiencyCheck: analysis of an 2d efficiency map \n"
	<< std::endl;
}
