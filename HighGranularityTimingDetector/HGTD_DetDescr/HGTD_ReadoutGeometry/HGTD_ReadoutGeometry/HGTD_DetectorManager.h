/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HGTD_READOUTGEOMETRY_HGTD_DETECTORMANAGER_H
#define HGTD_READOUTGEOMETRY_HGTD_DETECTORMANAGER_H

#include "GeoPrimitives/GeoPrimitives.h"

#include "GeoModelKernel/GeoVPhysVol.h"
#include "GeoModelKernel/GeoVDetectorManager.h"

#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElementCollection.h"

#include "ReadoutGeometryBase/SiCommonItems.h"

#include "HGTD_Identifier/HGTD_ID.h"

// Message Stream Member
#include "AthenaBaseComps/AthMessaging.h"

#include <span>

class StoreGateSvc;

/** @class HGTD_DetectorManager

    The Detector manager has methods to retrieve the Identifier
    helper and methods to retrieve the detector elements, as well
    as the relevant physical volumes.

    */

class HGTD_DetectorManager : public GeoVDetectorManager, public AthMessaging {
public:

    /** Constructor */
    HGTD_DetectorManager(StoreGateSvc* detStore);

    /** Destructor */
    ~HGTD_DetectorManager();

    /** Access to raw geometry: */
    virtual unsigned int getNumTreeTops()           const override;
    virtual PVConstLink  getTreeTop(unsigned int i) const override;

    /** Add a Tree top: */
    void addTreeTop (PVConstLink treeTop);

    //
    // Access Readout Elements
    //

    /** access to individual elements : via Identifier */
    const InDetDD::HGTD_DetectorElement * getDetectorElement(const Identifier &id) const;

    /** access to individual elements : via IdentifierHash */
    const InDetDD::HGTD_DetectorElement * getDetectorElement(const IdentifierHash &idHash) const;

    /** access to individual elements : via element identification */
    const InDetDD::HGTD_DetectorElement * getDetectorElement(int endcap,
                                                             int layer,
                                                             int phi_module,
                                                             int eta_module) const;

    /** Get the collection of element that is held*/
    const InDetDD::HGTD_DetectorElementCollection* getDetectorElementCollection() const;

    /** Add elememts */
    void addDetectorElement(InDetDD::HGTD_DetectorElement * element);

    /** Invalidate cache for all detector elements */
    void invalidateAll();

    /** Update all caches */
    void updateAll();

    /** Set SiCommonItems */
    void setCommonItems(std::unique_ptr<const InDetDD::SiCommonItems>&& commonItems);

private:

    /** Prevent copy and assignment */
    const HGTD_DetectorManager & operator=(const HGTD_DetectorManager &right);
    HGTD_DetectorManager(const HGTD_DetectorManager &right);

    const HGTD_ID  * getIdHelper() const;

    // Private member data
    std::vector<PVConstLink>                         m_volume;
    // Note that the elements can be altered
    InDetDD::HGTD_DetectorElementCollection          m_elementCollection;
    const HGTD_ID*                                   m_idHelper;

    std::unique_ptr<const InDetDD::SiCommonItems> m_commonItems;

};

#ifndef GAUDI_NEUTRAL
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(HGTD_DetectorManager, 73488296, 1)
#endif

#endif // HGTD_READOUTGEOMETRY_HGTD_DETECTORMANAGER_H
