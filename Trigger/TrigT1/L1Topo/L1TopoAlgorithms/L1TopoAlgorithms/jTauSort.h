/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//  jTauSort.h
//  TopoCore


#ifndef L1TOPOALGORITHMS_JTAUSORT_H
#define L1TOPOALGORITHMS_JTAUSORT_H

#include "L1TopoInterfaces/SortingAlg.h"
#include "L1TopoEvent/TOBArray.h"

#include <iostream>
#include <vector>

namespace TCS {
   
   class jTauSort : public SortingAlg {
   public:
      
      // constructor
      jTauSort(const std::string & name);

      // destructor
      virtual ~jTauSort();
      virtual TCS::StatusCode initialize() override; 
      virtual TCS::StatusCode sort(const InputTOBArray & input, TOBArray & output) override final;

   private:
      parType_t      m_numberOfjTaus = { 0 };
      parType_t      m_minEta = { 0 };
      parType_t      m_maxEta = { 0 };
      parType_t      m_iso = { 0 };
      parType_t      m_passIsolation = { 0 };

      bool checkIsolation(const TCS::jTauTOB* jtau) const;


   };

} // end of namespace TCS

#endif /* defined(__TopoCore__SortingAlg__) */
