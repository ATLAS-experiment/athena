// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


#ifndef ITRIGFPGATrackSimMAPPINGSVC_H
#define ITRIGFPGATrackSimMAPPINGSVC_H

#include "GaudiKernel/IService.h"
#include "GaudiKernel/IInterface.h"

// Forward declarations
class FPGATrackSimRegionMap;
class FPGATrackSimPlaneMap;
class FPGATrackSimNNMap;


class IFPGATrackSimMappingSvc: virtual public IService
{
    public:
        DeclareInterfaceID(IFPGATrackSimMappingSvc, 1, 0);
  
        virtual const FPGATrackSimPlaneMap* PlaneMap_1st(int slice) const = 0;
        virtual const FPGATrackSimPlaneMap* PlaneMap_2nd(int slice) const = 0;
        virtual size_t GetPlaneMap_1stSliceSize() const = 0;
        virtual size_t GetPlaneMap_2ndSliceSize() const = 0;
        virtual const FPGATrackSimRegionMap* RegionMap_1st() const = 0;
        virtual const FPGATrackSimRegionMap* RegionMap_2nd() const = 0;
        virtual const FPGATrackSimRegionMap* SubRegionMap() const = 0;
        virtual const FPGATrackSimRegionMap* SubRegionMap_2nd() const = 0;
        virtual std::string getFakeNNMapString() const = 0;
        virtual std::string getParamNNMapString() const = 0;

};



#endif   // ITRIGFPGATrackSimMAPPINGSVC_H
