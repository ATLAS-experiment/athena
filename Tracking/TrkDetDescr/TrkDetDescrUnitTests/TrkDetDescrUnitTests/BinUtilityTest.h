/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// BinUtilityTest.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRUNITTESTS_BinUtilityTEST_H
#define TRKDETDESCRUNITTESTS_BinUtilityTEST_H

// Athena & Gaudi includes
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
// Trk includes
#include "TrkDetDescrUnitTests/TrkDetDescrUnitTestBase.h"
#include "TrkDetDescrInterfaces/ISurfaceBuilder.h"

#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>

namespace Trk {
             
    /** @class BinUtilityTest
       
        Test calling the internal surface intersection methods 
        
        @author Andreas.Salzburger@cern.ch       
      */
      
    class BinUtilityTest : public TrkDetDescrUnitTestBase  {
     public:

       /** Standard Athena-Algorithm Constructor */
       using Trk::TrkDetDescrUnitTestBase::TrkDetDescrUnitTestBase;
       
       /* specify the test here */
       StatusCode runTest();
              
      private:
        /** preparation of std::vector and std::map for comparison */
        void prepareData(std::vector<float>& vec, std::map<float, std::size_t>& map, float& low, float& high);

        /** A linear search  - superior in O(10) searches*/
        std::size_t searchInVectorWithBoundary(std::vector<float>& v, float value)
        {   
            if (v.empty())[[unlikely]]{
              throw std::runtime_error("searchInVectorWithBoundary: vector is empty");
            }
            std::size_t bin{};
            while (bin < v.size() && v[bin] < value){
              ++bin;
            }
            return bin == 0 ? 0 : std::min(bin - 1, v.size() - 1);         
        }

        /** A binary search with underflow/overflow */
        std::size_t binarySearchWithBoundary(const std::vector<float>& v, float value)
        {   
            if (v.empty())[[unlikely]]{
              throw std::runtime_error("binarySearchWithBoundary: vector is empty");
            }
            const auto it = std::upper_bound(v.begin(), v.end(), value);
            if (it == v.begin()){
              return 0;
            }
            return std::min<std::size_t>(std::distance(v.begin(), it) - 1,
              v.size() - 1);
        }

        Gaudi::Property<std::size_t> m_numberOfSegments
	  {this, "NumberOfSegments", 10};
        Gaudi::Property<std::size_t> m_numberOfTestsPerSet
	  {this, "NumberOfTetsPerSet", 100000};

   };
}

#endif
