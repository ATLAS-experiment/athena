/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 05/2024
* Description: Athena tool wrapper around the ITkPix encoder
*/

#ifndef ITKPIXELBYTESTREAMCNV_ITKPIXELHITSORTINGTOOL_H
#define ITKPIXELBYTESTREAMCNV_ITKPIXELHITSORTINGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "ITkPixelRDO_Container.h"
#include "ITkPixLayout.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"
#include "ITkPixelCabling/ITkPixelOnlineId.h"

class PixelID;

class ITkPixelHitSortingTool: public AthAlgTool {
    public:

        typedef ITkPixLayout<uint16_t> HitMap;
        
        ITkPixelHitSortingTool(const std::string& type,const std::string& name,const IInterface* parent);

        StatusCode initialize();

        std::map<ITkPixelOnlineId, HitMap> sortRDOHits(const ITkPixelRDO_Container* rdoContainer) const;

        StatusCode createRDO(std::map<ITkPixelOnlineId, HitMap> &EventHitMaps, ITkPixelRDO_Container *rdoContainer) const;


    private:

    ServiceHandle< InDetDD::IPixelReadoutManager > m_pixelReadout {this, "PixelReadoutManager", "ITkPixelReadoutManager", "Pixel readout manager" };
    const PixelID* m_pixIdHelper{};
    const InDetDD::PixelDetectorManager* m_detManager{};

    const ITkPixelCablingData m_cablingHelper;
};


#endif