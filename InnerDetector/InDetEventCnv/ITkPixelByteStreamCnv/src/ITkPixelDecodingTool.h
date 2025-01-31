/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXELBYTESTREAMCNV_ITKPIXELDECODINGTOOL_H
#define ITKPIXELBYTESTREAMCNV_ITKPIXELDECODINGTOOL_H

#include <vector>
#include <memory>
#include <cstdint>
#include "AthenaBaseComps/AthAlgTool.h"
#include "ITkPixLayout.h"

class ITkPixelDecodingTool: public AthAlgTool {
    public:

        typedef ITkPixLayout<uint16_t> HitMap; 
        
        ITkPixelDecodingTool(const std::string& type,const std::string& name,const IInterface* parent);

        virtual StatusCode initialize() override;

        std::unique_ptr<HitMap> decodeStream(std::vector<uint32_t> *dataStream) const;


};


#endif