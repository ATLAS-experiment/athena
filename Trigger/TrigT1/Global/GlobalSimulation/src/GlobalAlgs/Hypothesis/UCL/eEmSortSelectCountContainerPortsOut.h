/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMSORTSELECTCOUNTCONTAINERPORTSOUT_H
#define GLOBALSIM_EEMSORTSELECTCOUNTCONTAINERPORTSOUT_H

#include "AlgoDataTypes.h"

#include <ostream>
#include <memory>
#include <vector>


#include "AthenaKernel/CLASS_DEF.h"

namespace GlobalSim {

  struct eEmSortSelectCountContainerPortsOut {

       //eEmSortSelectCount

    constexpr static std::size_t eEmNumSort{7};

    // +1 is spare
    constexpr static std::size_t NumSelect{eEmNumSort+1};

    // no of sorts = No of non-spare selections
    constexpr static std::size_t NumSort{eEmNumSort};


    // no of sorts = No of items to keep for each sort
    constexpr static std::array<std::size_t, eEmNumSort> eEmSortOutWidth {
      {6UL, 6UL, 6UL, 10UL, 10UL, 10UL, 6UL}
    };

 

    // No Sort still to be added (adds another 144 elements) FIXME
    constexpr static std::size_t eEmNumTotalTobWidth{
      std::accumulate(std::begin(eEmSortOutWidth),
		      std::end(eEmSortOutWidth),
		      0U)};


    // indices to place sorted tobs in output array found by compile
    // time summing of width values.
    constexpr static std::array<std::size_t, eEmNumSort> eEmSortOutStart =
      []{
	std::array<std::size_t, eEmNumSort> a{};
	std::partial_sum(std::cbegin(eEmSortOutWidth),
			 std::cend(eEmSortOutWidth)-1,
			 a.begin()+1,
			 std::plus<std::size_t>());
	return a;
      }();

    constexpr static std::size_t eEmNumCount{24};
    constexpr static std::array<unsigned, eEmNumCount> eEmCountOutWidth {
      3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
    };

    // calculate the total width from the individual widths
    constexpr static std::size_t eEmNumTotalCountWidth{
      std::accumulate(std::begin(eEmCountOutWidth),
		      std::end(eEmCountOutWidth),
		      0U)};

    // calculate the start position in the output bits for each count
    constexpr static std::array<std::size_t, eEmNumCount> eEmCountOutStart =
      []{
	std::array<std::size_t, eEmNumCount> a{};
	std::partial_sum(std::cbegin(eEmCountOutWidth),
			 std::cend(eEmCountOutWidth)-1,
			 a.begin()+1,
			 std::plus<std::size_t>());
	return a;
      }();


    // [2^n-1..]  where n are the elements of eEmCountOutWidth
    constexpr static std::array<std::size_t, eEmNumCount> max_counts = [] {
      std::array<std::size_t, eEmNumCount> a{};
      for (std::size_t ind =0; ind != eEmCountOutWidth.size(); ++ind) {
	std::size_t result = 1;
	for(unsigned i = 1; i <= eEmCountOutWidth[ind]; ++i) {
	  result *= 2;
	}
	
	a[ind] = result-1;
      }
      return a;
    }();
    

    using GenTobPtr = std::shared_ptr<GenericTob>;
    using BSPtrNumTotalCountWidth =
      std::shared_ptr<std::bitset<eEmNumTotalCountWidth>>;

    // Output GenericTobs. VHDL variable is an array of GenericTobs
    std::array<GenTobPtr, eEmNumTotalTobWidth>  m_O_eEmGenTob;

    // Output counts. VHDL variable is a bit array
    BSPtrNumTotalCountWidth
    m_O_Multiplicity{std::make_shared<std::bitset<eEmNumTotalCountWidth>>()};
  
  };

}

std::ostream&
operator<< (std::ostream&,
	    const GlobalSim::eEmSortSelectCountContainerPortsOut&);

CLASS_DEF( GlobalSim::eEmSortSelectCountContainerPortsOut , 1289475565 , 1 )

#endif 
  
