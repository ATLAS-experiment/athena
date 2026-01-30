/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include "ITkPixLayout.h"
#include "ITkPixelCabling/ITkPixelOnlineId.h"
#include "InDetRawData/InDetRawDataCollection.h"
#include "InDetRawData/InDetRawDataContainer.h"


class PixelID;
class ITkPixelCablingData;

namespace InDetDD{
    class PixelDetectorManager;
}

class ITkPixelHitSortingTool: public AthAlgTool {
    public:

        typedef ITkPixLayout<uint16_t> HitMap;
        
        ITkPixelHitSortingTool(const std::string& type,const std::string& name,const IInterface* parent);

        StatusCode initialize();

        template<class ContainerType>
        std::map<ITkPixelOnlineId, HitMap> sortRDOHits(const ContainerType* rdoContainer, const ITkPixelCablingData* cabling) const;

        template<class ContainerType, class RDOType>
        StatusCode createRDO(std::map<ITkPixelOnlineId, HitMap> &EventHitMaps, ContainerType *rdoContainer) const;


    private:

    ServiceHandle< InDetDD::IPixelReadoutManager > m_pixelReadout{this, "PixelReadoutManager", "ITkPixelReadoutManager", "Pixel readout manager"};
    
    const PixelID* m_pixIdHelper{};
    
    const InDetDD::PixelDetectorManager* m_detManager{};

};

#endif
