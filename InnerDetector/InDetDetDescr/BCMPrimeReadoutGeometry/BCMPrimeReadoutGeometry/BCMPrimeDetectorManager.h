/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BCMPRIMEREADOUTGEOMETRY_BCMPRIMEDETECTORMANAGER_H
#define BCMPRIMEREADOUTGEOMETRY_BCMPRIMEDETECTORMANAGER_H

#include "GeoPrimitives/GeoPrimitives.h"

#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoVDetectorManager.h"

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
        void addTreeTop (const PVConstLink& treeTop);

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
