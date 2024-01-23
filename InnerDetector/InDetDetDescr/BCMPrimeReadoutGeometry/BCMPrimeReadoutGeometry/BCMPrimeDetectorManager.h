/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BCMPRIMEREADOUTGEOMETRY_BCMPRIMEDETECTORMANAGER_H
#define BCMPRIMEREADOUTGEOMETRY_BCMPRIMEDETECTORMANAGER_H

#include "GeoPrimitives/GeoPrimitives.h"

#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoVDetectorManager.h"

#include "GeoModelKernel/GeoAlignableTransform.h"

#include "AthenaKernel/IIOVSvc.h"

class StoreGateSvc;

/** @class BCMPrimeDetectorManager

    The Detector manager registers the call backs and infrastructure to
    associate the alignment transforms with the appropriate alignable
    transform in GeoModel.

    @author Jakob Novak <jakob.novak@cern.ch>

    */

namespace InDetDD {

    class BCMPrimeDetectorManager : public GeoVDetectorManager {
    public:

        /** Constructor */
        BCMPrimeDetectorManager(const std::string & name);

        /** Access to raw geometry: */
        virtual unsigned int getNumTreeTops()           const override;
        virtual PVConstLink  getTreeTop(unsigned int i) const override;

        /** Add a Tree top: */
        void addTreeTop (PVConstLink treeTop);

        void addAlignableTransform (int /*id*/, GeoAlignableTransform * /*transform*/, const GeoVPhysVol * /*child*/);
        StatusCode align( IOVSVC_CALLBACK_ARGS ) const;

    private:

        /** Prevent copy and assignment */
        const BCMPrimeDetectorManager & operator=(const BCMPrimeDetectorManager &right);
        BCMPrimeDetectorManager(const BCMPrimeDetectorManager &right);

        /** Private member data */
        std::vector<PVConstLink>              m_volume;
    };

} // namespace InDetDD

#ifndef GAUDI_NEUTRAL
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(InDetDD::BCMPrimeDetectorManager, 162824294, 1)
#endif

#endif // BCMPRIMEREADOUTGEOMETRY_BCMPRIMEDETECTORMANAGER_H
