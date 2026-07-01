/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PixelConditionsData/ITkPixFieldMaps.h
 * @author Shaun Roe, based on code by Soshi Tsuno 
 * @date June 2026
 * @brief Data object for fields, ramo potential etc, used in radiation damage simulations.
 */

#ifndef PixelConditionsData_ITkPixFieldMaps_h
#define PixelConditionsData_ITkPixFieldMaps_h

#include "AthenaKernel/CLASS_DEF.h"

#include "AthenaKernel/CondCont.h"
#include "PixelConditionsData/PixelHistoConverter.h"
#include <vector>

class ITkPixFieldMaps {
  public:
    ITkPixFieldMaps();
    virtual ~ITkPixFieldMaps();

    //3D sensor field maps not included here, even if they are in the referenced file
    void setLorentzMap_e(std::vector<PixelHistoConverter> lorentzMap_e);
    void setLorentzMap_h(std::vector<PixelHistoConverter> lorentzMap_h);
    void setDistanceMap_e(std::vector<PixelHistoConverter> distanceMap_e);
    void setDistanceMap_h(std::vector<PixelHistoConverter> distanceMap_h);
    void setRamoPotentialMap(std::vector<PixelHistoConverter> ramoPotentialMap);

    const PixelHistoConverter& getLorentzMap_e(int layer) const;
    const PixelHistoConverter& getLorentzMap_h(int layer) const;
    const PixelHistoConverter& getDistanceMap_e(int layer) const;
    const PixelHistoConverter& getDistanceMap_h(int layer) const;
    const PixelHistoConverter& getRamoPotentialMap(int layer) const;

    void clear();

  private:
    std::vector<PixelHistoConverter> m_lorentzMap_e;
    std::vector<PixelHistoConverter> m_lorentzMap_h;
    std::vector<PixelHistoConverter> m_distanceMap_e;
    std::vector<PixelHistoConverter> m_distanceMap_h;
    std::vector<PixelHistoConverter> m_ramoPotentialMap;

};
//Magic numbers from command line scripts:
//"clid ITkPixFieldMaps"
CLASS_DEF( ITkPixFieldMaps , 133345057 , 1 )
//"clid -cs ITkPixFieldMaps"
CONDCONT_DEF( ITkPixFieldMaps, 90281761 );

#endif
