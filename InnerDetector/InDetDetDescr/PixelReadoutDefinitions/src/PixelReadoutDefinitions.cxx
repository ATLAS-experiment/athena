/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "PixelReadoutDefinitions/PixelReadoutDefinitions.h"
#include <array>


namespace InDetDD{
  std::string 
  PixelModuleTypeName(const PixelModuleType & t){
    if (t==PixelModuleType::NONE) return "unknown";
    static const std::array<std::string, enum2uint(PixelModuleType::N_PIXELMODULETYPES)> PixelModuleTypeNames{
    "DBM", "IBL_PLANAR", "IBL_3D", "PIX_BARREL", "PIX_ENDCAP"
    };
    return PixelModuleTypeNames[enum2uint(t)];
  }
  
  std::string 
  PixelDiodeTypeName(const PixelDiodeType & t){
    if (t==PixelDiodeType::NONE) return "unknown";
    static const std::array<std::string, enum2uint(PixelDiodeType::N_DIODETYPES)> PixelDiodeTypeNames{
        "NORMAL", "LONG", "GANGED", "LARGE"
    };
    return PixelDiodeTypeNames[enum2uint(t)];
  }
  
  std::string 
  PixelReadoutTechnologyName(const PixelReadoutTechnology & t){
    if (t==PixelReadoutTechnology::NONE) return "unknown";
    static const std::array<std::string, enum2uint(PixelReadoutTechnology::N_TECHNOLOGIES)> PixelReadoutTechnologyNames{
      "FEI3", "FEI4", "RD53"
    };
    return PixelReadoutTechnologyNames[enum2uint(t)];
  }
}
