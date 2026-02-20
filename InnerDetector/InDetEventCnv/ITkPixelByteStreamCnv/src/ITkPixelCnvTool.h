/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 03/2025
* Description: Top-level tool to be called from BS converter
*/

#ifndef ITKPIXELCNVTOOL_H
#define ITKPIXELCNVTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/IByteStreamCnvSvc.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"
#include "ITkPixelHitSortingTool.h"
#include "ITkPixelEncodingTool.h"
#include "ITkPixelDataRateMonTool.h"

/**
 * @class ITkPixelCnvTool
 * This tool orchestrates the individual steps
 * in the BS conversion
 */

class ITkPixelCnvTool : public AthAlgTool {
    
    public:

        ITkPixelCnvTool(const std::string& type,const std::string& name,const IInterface* parent);

        virtual StatusCode initialize() override;

        template<class ContainerType>
        StatusCode convertToByteStream(const ContainerType* cont) const;

    private:

        ToolHandle<ITkPixelHitSortingTool> m_hitSortingTool{this, "HitSortingTool", ""};

        ToolHandle<ITkPixelEncodingTool> m_encodingTool{this, "EncodingTool", "", "The encoding tool"};

        ServiceHandle<IByteStreamCnvSvc> m_byteStreamCnvSvc{this, "ByteStreamConvertionService", "ByteStreamCnvSvc", "The Byte stream coversion service"};
        
        SG::ReadCondHandleKey<ITkPixelCablingData> m_pixelCablingKey{this, "PixelCablingKey", "", "Cond Key of Pixel Cabling"};

        ToolHandle<ITkPixelDataRateMonTool> m_dataRateMonTool{this, "DataRateMonitoringTool", "", "Monitoring tool for data rate evaluation"};

};

#endif
