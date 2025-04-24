/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*  L1Calo_BinsDiffFromStripMedian.h 
   Author: will buttinger
   Email: will@cern.ch
*/

#ifndef L1Calo_BinsDiffFromStripMedian_H
#define L1Calo_BinsDiffFromStripMedian_H

#include <dqm_core/Algorithm.h>
#include <vector>
#include <iosfwd>
#include <string>


namespace dqm_algorithms
{
	struct L1Calo_BinsDiffFromStripMedian : public dqm_core::Algorithm
        {
	  L1Calo_BinsDiffFromStripMedian();
	  ~L1Calo_BinsDiffFromStripMedian();
	  L1Calo_BinsDiffFromStripMedian* clone( );

	  dqm_core::Result* execute( const std::string & , const TObject & , const dqm_core::AlgorithmConfig & );

      using dqm_core::Algorithm::printDescription;
	  void  printDescription(std::ostream& out);

        class bin{
          public:
            int    m_ix{};
            int    m_iy{};
            double m_value{};
            double m_outstandingRatio{};
        } ;
      
	};
}


#endif // L1Calo_BinsDiffFromStripMedian_H
