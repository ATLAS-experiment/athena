/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DQM_ALGORITHMS_TILEBINSFILLEDOUTRANGE_H
#define DQM_ALGORITHMS_TILEBINSFILLEDOUTRANGE_H

#include <dqm_core/Algorithm.h>
#include <string>
#include <iosfwd>

namespace dqm_algorithms {
	class TileBinsOutRange : public dqm_core::Algorithm {
    public:
      TileBinsOutRange();
      virtual ~TileBinsOutRange() = default;

      virtual TileBinsOutRange* clone( ) override;
      virtual dqm_core::Result* execute(const std::string& name, const TObject& object, const dqm_core::AlgorithmConfig& config) override;
      using dqm_core::Algorithm::printDescription;
      void printDescription(std::ostream& out);

    private:
      std::string  m_name;
  };
}

#endif // DQM_ALGORITHMS_TILEBINSFILLEDOUTRANGE_H
