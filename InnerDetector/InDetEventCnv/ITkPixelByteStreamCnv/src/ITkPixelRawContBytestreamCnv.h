/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 03/2025
* Description: The BS converter for ITk pixels
*/


#ifndef ITKPIXELRAWCONTBYTESTREAMCNV_H
#define ITKPIXELRAWCONTBYTESTREAMCNV_H

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthConstConverter.h"
#include "ByteStreamCnvSvcBase/IByteStreamEventAccess.h"
#include "ITkPixelCnvTool.h"

class ITkPixelRawContByteStreamCnv : public AthConstConverter {
    public:

        ITkPixelRawContByteStreamCnv(ISvcLocator* svcloc);

        virtual StatusCode initialize() override;

        //Create BS from RDO
        virtual StatusCode createRepConst(DataObject* pObj, IOpaqueAddress*& pAddr) const override;

        virtual long repSvcType() const override { return i_repSvcType(); }

        static long storageType();
        static const CLID& classID();

    private:

        ServiceHandle<IByteStreamEventAccess> m_ByteStreamEventAccess;

        ToolHandle<ITkPixelCnvTool> m_cnvTool;

};

#endif