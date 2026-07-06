/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BCMPRIMEREADOUTGEOMETRY_BCMPRIMEDETECTORMANAGER_H
#define BCMPRIMEREADOUTGEOMETRY_BCMPRIMEDETECTORMANAGER_H

#include "GeoModelKernel/GeoVPhysVol.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetReadoutGeometry/ExtendedAlignableTransform.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetReadoutGeometry/SiDetectorManager.h"
#include "ReadoutGeometryBase/InDetDD_Defs.h"

#include <memory>
#include <vector>

class CondAttrListCollection;
class GeoAlignableTransform;
class GeoVAlignmentStore;
class Identifier;
class IdentifierHash;
class StoreGateSvc;

/** @class BCMPrimeDetectorManager

    The Detector manager registers the call backs and infrastructure to
    associate the alignment transforms with the appropriate alignable
    transform in GeoModel.

    @author Jakob Novak <jakob.novak@cern.ch>

    */

namespace InDetDD {

    class BCMPrimeDiamondDesign;
    class SiDetectorElement;

    class BCMPrimeDetectorManager : public SiDetectorManager {
    public:

        /** Constructor */
        BCMPrimeDetectorManager(StoreGateSvc* detStore, const std::string& name);

        /** Access to raw geometry: */
        virtual unsigned int getNumTreeTops()           const override;
        virtual PVConstLink  getTreeTop(unsigned int i) const override;

        /** Add a Tree top: */
        void addTreeTop (const PVConstLink& treeTop);

        /** Access readout elements. */
        virtual const SiDetectorElement* getDetectorElement(const Identifier& id) const override;
        virtual const SiDetectorElement* getDetectorElement(const IdentifierHash& idHash) const override;
        const SiDetectorElement* getDetectorElement(int barrelEndcap,
                                                    int layerWheel,
                                                    int phiModule,
                                                    int etaModule) const;

        virtual const SiDetectorElementCollection* getDetectorElementCollection() const override;
        virtual SiDetectorElementCollection::const_iterator getDetectorElementBegin() const override;
        virtual SiDetectorElementCollection::const_iterator getDetectorElementEnd() const override;
        virtual SiDetectorElementCollection::iterator getDetectorElementBegin() override;
        virtual SiDetectorElementCollection::iterator getDetectorElementEnd() override;

        /** Add detector element during construction. */
        virtual void addDetectorElement(SiDetectorElement* element) override;

        /** Initialize neighbours after construction. BCMPrime has no neighbour map yet. */
        virtual void initNeighbours() override;

        virtual bool isPixel() const override { return true; }
        virtual bool identifierBelongs(const Identifier& id) const override;

        virtual void addAlignableTransform(int level,
                                           const Identifier& id,
                                           GeoAlignableTransform* transform) override;

        bool processSpecialAlignment(const std::string& key,
                                     InDetDD::AlignFolderType alignfolder) override;
        bool processSpecialAlignment(const std::string& key,
                                     const CondAttrListCollection* obj = nullptr,
                                     GeoVAlignmentStore* alignStore = nullptr) const override;

        /** Get number of detector elements: */
        unsigned int getNumDetectorElements() const;

    private:
        virtual bool setAlignableTransformDelta(int level,
                                                const Identifier& id,
                                                const Amg::Transform3D& delta,
                                                FrameType frame,
                                                GeoVAlignmentStore* alignStore = nullptr) const override;

        virtual const PixelID* getIdHelper() const override;

        /** Prevent copy and assignment */
        const BCMPrimeDetectorManager & operator=(const BCMPrimeDetectorManager &right);
        BCMPrimeDetectorManager(const BCMPrimeDetectorManager &right);

        /** Private member data */
        std::vector<PVConstLink>              m_volume;
        SiDetectorElementCollection           m_elementCollection;
        std::vector<std::unique_ptr<ExtendedAlignableTransform>> m_alignableTransforms;
        const PixelID*                        m_idHelper{};
    };

} // namespace InDetDD

#ifndef GAUDI_NEUTRAL
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(InDetDD::BCMPrimeDetectorManager, 162824294, 1)
#endif

#endif // BCMPRIMEREADOUTGEOMETRY_BCMPRIMEDETECTORMANAGER_H
