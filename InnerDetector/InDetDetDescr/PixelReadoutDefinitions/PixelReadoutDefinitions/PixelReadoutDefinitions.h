/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELREADOUTDEFINITIONS_H
#define PIXELREADOUTDEFINITIONS_H
#include <cstddef> //for size_t
#include <cstdint> //for uint32_t
#include <string>
#include <string_view>
#include <stdexcept>
#include <typeinfo>
#include <format> // For std::format (C++20)


namespace InDetDD{

  enum class PixelModuleType{
    NONE =-1,
    DBM,
    IBL_PLANAR,
    IBL_3D,
    PIX_BARREL,
    PIX_ENDCAP,
    N_PIXELMODULETYPES
  };

  enum class PixelDiodeType{
    NONE=-1,
    NORMAL,
    LONG,
    GANGED,
    LARGE,
    N_DIODETYPES
  };

  enum class PixelReadoutTechnology{
    NONE =-1,
    FEI3,
    FEI4,
    RD53,
    N_TECHNOLOGIES
  };
  
  constexpr uint32_t invalidRow = 0xFFFFFFFF;
  constexpr uint32_t invalidColumn = 0xFFFFFFFF;
  constexpr uint32_t invalidFrontEnd = 0xFFFFFFFF;
  ///Convert an enum class to size_t for use as an array index
  template <typename T>
  constexpr std::size_t
  enum2uint(T n, std::string_view callingFunctionName = ""){
    if (n==T::NONE){
      throw std::out_of_range(std::format("{} InDetDD::enum2uint: 'NONE' type is out of range for {}", callingFunctionName, typeid(T).name()));
    }
    return static_cast<size_t>(n);
  }
  
  std::string 
  PixelModuleTypeName(const PixelModuleType & t);
  
  std::string 
  PixelDiodeTypeName(const PixelDiodeType & t);
  
  std::string 
  PixelReadoutTechnologyName(const PixelReadoutTechnology & t);
}

#endif
