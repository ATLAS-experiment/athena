/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 05/2024
* Description: Athena tool wrapper around the ITkPix encoder
*/

#ifndef ITKPIXELBYTESTREAMCNV_ITKPIXELENCODINGTOOL_H
#define ITKPIXELBYTESTREAMCNV_ITKPIXELENCODINGTOOL_H

#include <vector>
#include <memory>
#include <cstdint>
#include "AthenaBaseComps/AthAlgTool.h"
#include "ITkPixLayout.h"
#include "ITkPixV2Encoder.h"

class ITkPixelEncodingTool: public AthAlgTool {
    public:

        typedef ITkPixLayout<uint16_t> HitMap;
        
        ITkPixelEncodingTool(const std::string& type,const std::string& name,const IInterface* parent);

        StatusCode initialize();

        std::vector<uint32_t> encodeFE(const HitMap & hitMap, const uint8_t chipID) const;

};


#endif