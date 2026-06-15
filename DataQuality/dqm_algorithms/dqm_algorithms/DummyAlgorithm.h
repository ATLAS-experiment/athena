/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/*! \file DummyAlgorithm.h
 *  Declares dqm_algorithms::DummyAlgorithm.
 *  Always returns Result::Undefined — use for histograms whose detector
 *  channel is intentionally disabled (e.g. switched-off sTGC Qi layers).
 */
#ifndef DQM_ALGORITHMS_DUMMY_ALGORITHM_H
#define DQM_ALGORITHMS_DUMMY_ALGORITHM_H

#include <dqm_core/Algorithm.h>
#include <string>
#include <iosfwd>

namespace dqm_algorithms {

struct DummyAlgorithm : public dqm_core::Algorithm {
    DummyAlgorithm( const std::string& name );
    DummyAlgorithm*       clone();
    dqm_core::Result*     execute( const std::string&,
                                   const TObject&,
                                   const dqm_core::AlgorithmConfig& );
    using dqm_core::Algorithm::printDescription;
    void printDescription( std::ostream& out );
private:
    std::string m_name;
};

} // namespace dqm_algorithms
#endif // DQM_ALGORITHMS_DUMMY_ALGORITHM_H