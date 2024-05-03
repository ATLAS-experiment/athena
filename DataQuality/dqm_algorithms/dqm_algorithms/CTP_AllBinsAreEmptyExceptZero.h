/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DQM_ALGORITHMS_CTP_ALLBINSAREEMPTYEXCEPTZERO_H
#define DQM_ALGORITHMS_CTP_ALLBINSAREEMPTYEXCEPTZERO_H

#include <string>
#include "TObject.h"
#include "dqm_core/Algorithm.h"


namespace dqm_algorithms {

class CTP_AllBinsAreEmptyExceptZero : public dqm_core::Algorithm {
public:

  CTP_AllBinsAreEmptyExceptZero();
  virtual ~CTP_AllBinsAreEmptyExceptZero() = default;
  
  virtual dqm_core::Algorithm*  clone() override;
  virtual dqm_core::Result*     execute( const std::string& name, const TObject& data, const dqm_core::AlgorithmConfig& config ) override;
  using dqm_core::Algorithm::printDescription;
  virtual void                  printDescription(std::ostream& out);

private:
  std::string  m_name;
};

} //namespace dqm_algorithms

#endif // DQM_ALGORITHMS_CTP_ALLBINSAREEMPTYEXCEPTZERO_H
