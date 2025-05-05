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

class ITkPixelHitSortingTool;
class ITkPixelEncodingTool;

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

        ToolHandle<ITkPixelHitSortingTool> m_hitSortingTool;

        ToolHandle<ITkPixelEncodingTool> m_encodingTool;

        ServiceHandle<IByteStreamCnvSvc> m_byteStreamCnvSvc;

};

#endif