/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*! \file BinThreshold.h file declares the dqm_algorithms::BinThreshold  class.
 * \author Haleh Hadavand
*/

#ifndef DQM_ALGORITHMS_BINTHRESHOLD_H
#define DQM_ALGORITHMS_BINTHRESHOLD_H

#include <dqm_core/Algorithm.h>
#include <string>
#include <iosfwd>

namespace dqm_algorithms
{
	struct BinThreshold : public dqm_core::Algorithm
        {
        BinThreshold(const std::string & name);

	    //overwrites virtual functions
        BinThreshold * clone( );
        dqm_core::Result * execute( const std::string & , const TObject & , const dqm_core::AlgorithmConfig & );
          bool CompareBinThreshold( const std::string & objname, double bincontent, double threshold );
          using dqm_core::Algorithm::printDescription;
        void  printDescription(std::ostream& out);
        void  parseIgnoreList(const std::string& bins, 
                              std::vector<std::string>& ignoredRows, 
                              std::vector<std::string>& ignoredCols,
                              std::vector<std::pair<std::string,std::string>>& ignoredBins);
              /* Parse string input from IgnoreBins into lists of rows, columns and bins
               * A list of bins to ignore is provided (bins) using the format "x_1:y_1,x_2:y_2"
               * Accepted values for x_i, y_i are the bin index and the bin label (wildcards can be used)
               */
	  private:
	  std::string m_name;
	};
}

#endif // DQM_ALGORITHMS_BINTHRESHOLD_H
