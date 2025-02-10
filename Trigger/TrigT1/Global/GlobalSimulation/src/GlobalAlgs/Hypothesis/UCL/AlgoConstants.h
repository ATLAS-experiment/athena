//  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBALSIM_ALGOCONSTAMTS_H
#define  GLOBALSIM_ALGOCONSTAMTS_H


#include <cstddef>
#include <array>
#include <numeric>

namespace GlobalSim {
  struct AlgoConstants {
    constexpr static std::size_t n32parameters{1108};
    constexpr static std::size_t totalResultBits{256};
    constexpr static std::size_t eFexEtBitWidth{12};
    constexpr static std::size_t eFexDiscriminantBitWidth{2};
    constexpr static std::size_t eFexEtaBitWidth{8};
    constexpr static std::size_t eFexPhiBitWidth{6};

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
    constexpr static std::size_t NumTotalTobWidth{
      std::accumulate(std::begin(eEmSortOutWidth),
		      std::end(eEmSortOutWidth),
		      0U)};


    // indices to place sorted tobs in output array fiuynbd by scompile
    // time sum of width values.
    constexpr static std::array<std::size_t, eEmNumSort> eEmSortOutStart =
      []{
	std::array<std::size_t, eEmNumSort> a{};
	std::partial_sum(std::cbegin(eEmSortOutWidth),
			 std::cend(eEmSortOutWidth)-1,
			 a.begin()+1,
			 std::plus<std::size_t>());
	return a;
      }();

    constexpr static std::size_t NumTotalCountWidth{52}; // 4*3 + 2*20

  };

}


#endif
