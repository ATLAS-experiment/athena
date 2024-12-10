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
#include "InDetRawData/PixelRDO_Container.h"
#include "ITkPixLayout.h"

class PixelID;

class ITkPixelHitSortingTool: public AthAlgTool {
    public:

        typedef ITkPixLayout<uint16_t> HitMap;
        
        ITkPixelHitSortingTool(const std::string& type,const std::string& name,const IInterface* parent);

        StatusCode initialize();

        StatusCode sortRDOHits(SG::ReadHandle<PixelRDO_Container> &rdoContainer) const;

    private:

    ServiceHandle< InDetDD::IPixelReadoutManager > m_pixelReadout {this, "PixelReadoutManager", "ITkPixelReadoutManager", "Pixel readout manager" };

    const PixelID* m_pixIdHelper{};
};


#endif