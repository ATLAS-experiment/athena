/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef DQM_Algorithms_SectorEfficiencyCheck_H
#define DQM_Algorithms_SectorEfficiencyCheck_H

#include <dqm_core/Algorithm.h>
#include <dqm_core/AlgorithmConfig.h>
#include <dqm_core/Result.h>
#include <TObject.h>

#include <ostream>
#include <string>

namespace dqm_algorithms {
      class SectorEfficiencyCheck : public dqm_core::Algorithm {
      public:
        SectorEfficiencyCheck();
        ~SectorEfficiencyCheck() override;

        SectorEfficiencyCheck* clone() override;
        
        dqm_core::Result* execute( const std::string& name, 
                                   const TObject& object, 
                                   const dqm_core::AlgorithmConfig& config ) override;
        
        void printDescriptionTo( std::ostream& out ) override;

    };
} 

#endif // DQM_Algorithms_SectorEfficiencyCheck_H
