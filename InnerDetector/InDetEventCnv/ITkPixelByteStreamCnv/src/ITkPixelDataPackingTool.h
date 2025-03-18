/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXELBYTESTREAMCNV_ITKPIXELDATAPACKINGTOOL_H
#define ITKPIXELBYTESTREAMCNV_ITKPIXELDATAPACKINGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"

class ITkPixelDataPackingTool: public AthAlgTool {
    public:


        
        virtual StatusCode initialize() override;

        ITkPixelDataPackingTool(const std::string& type, const std::string& name, const IInterface* parent);

        void pack(const ITkPixelOnlineId *onlineID, std::vector<uint32_t> *encodedStream) const;


        struct UnpackedStream {
            uint32_t onlineID;
            std::vector<uint32_t> dataStream;
        };

        UnpackedStream unpack(std::vector<uint32_t> *encodedStream) const;
        

};

#endif