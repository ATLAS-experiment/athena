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

    constexpr static std::size_t maxNTob{10};
  };

}


#endif
