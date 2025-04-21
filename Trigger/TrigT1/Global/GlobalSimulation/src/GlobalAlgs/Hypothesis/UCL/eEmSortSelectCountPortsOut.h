/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMSORTSELECTCOUNTPORTSOUT_H
#define GLOBALSIM_EEMSORTSELECTCOUNTPORTSOUT_H

#include "GenericTob.h"

#include <ostream>
#include <memory>
#include <vector>


#include "AthenaKernel/CLASS_DEF.h"

namespace GlobalSim {

  struct eEmSortSelectCountPortsOut {

    //eEmSortSelectCount

    constexpr static std::size_t NumSort{7};
    // 0:eEms 1:eEmsl 2:eEmsm, 3:eEmab, 4:eEmabm
    constexpr static std::size_t NumSelect{NumSort+1};
    constexpr static std::size_t NumNoSort{1};

    // no of sorts = No of items to keep for each sort
    constexpr static std::array<std::size_t, NumSort> SortOutWidths {
      {6UL, 6UL, 6UL, 10UL, 10UL, 10UL, 6UL}
    };

    
    constexpr static std::size_t SortOutWidth{
      std::accumulate(std::begin(SortOutWidths),
		      std::end(SortOutWidths),
		      0U)};

    constexpr static std::size_t NoSortOutWidth{144};

    constexpr static std::size_t NumTotalTobWidth{SortOutWidth +
						  NoSortOutWidth};

    // indices to place sorted tobs in output array found by compile
    // time summing of width values.
    constexpr static std::array<std::size_t, NumSort> SortOutStart =
      []{
	std::array<std::size_t, NumSort> a{};
	std::partial_sum(std::cbegin(SortOutWidths),
			 std::cend(SortOutWidths)-1,
			 a.begin()+1,
			 std::plus<std::size_t>());
	return a;
      }();

    constexpr static std::size_t NumCount{24};
    constexpr static std::array<unsigned, NumCount> CountOutWidth {
      3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
    };

    // calculate the total width from the individual widths
    constexpr static std::size_t NumTotalCountWidth{
      std::accumulate(std::begin(CountOutWidth),
		      std::end(CountOutWidth),
		      0U)};

    // calculate the start position in the output bits for each count
    constexpr static std::array<std::size_t, NumCount> CountOutStart =
      []{
	std::array<std::size_t, NumCount> a{};
	std::partial_sum(std::cbegin(CountOutWidth),
			 std::cend(CountOutWidth)-1,
			 a.begin()+1,
			 std::plus<std::size_t>());
	return a;
      }();


    // [2^n-1..]  where n are the elements of CountOutWidth
    constexpr static std::array<std::size_t, NumCount> max_counts = [] {
      std::array<std::size_t, NumCount> a{};
      for (std::size_t ind =0; ind != CountOutWidth.size(); ++ind) {
	std::size_t result = 1;
	for(unsigned i = 1; i <= CountOutWidth[ind]; ++i) {
	  result *= 2;
	}
	
	a[ind] = result-1;
      }
      return a;
    }();
    

    using GenTobPtr = std::shared_ptr<GenericTob>;
    using BSPtrNumTotalCountWidth =
      std::shared_ptr<std::bitset<NumTotalCountWidth>>;

    // Output GenericTobs. VHDL variable is an array of GenericTobs
    std::array<GenTobPtr, NumTotalTobWidth>  m_O_eEmGenTob;

    // Output counts. VHDL variable is a bit array
    BSPtrNumTotalCountWidth
    m_O_Multiplicity{std::make_shared<std::bitset<NumTotalCountWidth>>()};

    eEmSortSelectCountPortsOut(){
      for(std::size_t i = 0; i != NumTotalTobWidth; ++i) {
	m_O_eEmGenTob[i] = std::make_shared<GenericTob>();
      }
    }    
  };

}

std::ostream&
operator<< (std::ostream&,
	    const GlobalSim::eEmSortSelectCountPortsOut&);

CLASS_DEF( GlobalSim::eEmSortSelectCountPortsOut , 98012723 , 1 )
#endif 
  
