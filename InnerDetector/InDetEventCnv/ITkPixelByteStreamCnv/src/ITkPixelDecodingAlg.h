/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXEL_DECODINGALG_H
#define ITKPIXEL_DECODINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "ITkPixelDataPackingTool.h"
#include "ITkPixLayout.h"
#include "ITkPixelDecodingTool.h"
#include "ITkPixelHitSortingTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"

class ITkPixelDecodingAlg : public AthReentrantAlgorithm 
{
  public:

    ITkPixelDecodingAlg(const std::string &name, ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute (const EventContext& ctx) const override;
  private:

    typedef std::vector< std::vector<uint32_t >> ITkPacketCollection;

    ServiceHandle<IROBDataProviderSvc>    m_robDataProviderSvc{ this, "ROBDataProvider", "ROBDataProviderSvc" };
    
    ToolHandle<ITkPixelDataPackingTool> m_packingTool;
    ToolHandle<ITkPixelDecodingTool> m_decodingTool;
    //ToolHandle<ITkPixelHitSortingTool> m_hitSortingTool; #commented out due to clang compilation warning, will be reintroduced in next MR

};
#endif

