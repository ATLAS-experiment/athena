/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DQM_Algorithms_AFP_Sync_check_H
#define DQM_Algorithms_AFP_Sync_check_H

#include <dqm_core/Algorithm.h>
#include <dqm_core/AlgorithmConfig.h>
#include <dqm_core/Result.h>

#include <TObject.h>

#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace dqm_algorithms {
    class AFP_Sync_check : public dqm_core::Algorithm {
      public:
        AFP_Sync_check();
        ~AFP_Sync_check();

        AFP_Sync_check* clone() override;
        dqm_core::Result* execute( const std::string& name, const TObject& object, const dqm_core::AlgorithmConfig& config ) override;
        void printDescriptionTo( std::ostream& out ) override;

    };
} // namespace dqm_algorithms

#endif // DQM_Algorithms_AFP_Sync_check_H
