/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "TRT_ReadoutGeometry/TRT_Numerology.h"
#include "TRT_ReadoutGeometry/TRT_BarrelCode.h"
#include "TRT_ReadoutGeometry/TRT_EndcapCode.h"
#include "TRT_ReadoutGeometry/TRT_BarrelElement.h"
#include "TRT_ReadoutGeometry/TRT_EndcapElement.h"
#include "InDetReadoutGeometry/ExtendedAlignableTransform.h"
#include "TRT_ReadoutGeometry/TRT_Conditions.h"

#include "GeoModelKernel/GeoXF.h"
#include "GeoModelKernel/GeoAlignableTransform.h"

#include "GeoPrimitives/CLHEPtoEigenConverter.h"

#include "DetDescrConditions/AlignableTransform.h"
#include "DetDescrConditions/AlignableTransformContainer.h"

#include "StoreGate/StoreGateSvc.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"

#include "TRT_ConditionsData/StrawDxContainer.h"

namespace InDetDD {

    const int FIRST_HIGHER_LEVEL = 1;

    TRT_DetectorManager::TRT_DetectorManager(StoreGateSvc * detStore)
        :InDetDetectorManager(detStore, "TRT"),
        m_numerology(new TRT_Numerology()),
        m_idHelper(nullptr),
        m_ownsIdHelper(false),
        m_gasType(unknown),
        m_digvers(9999),
        m_digversname("ERROR:DIGVERSNOTSET!")
    {
      m_elementContainer.setNumerology(m_numerology);

    // If detstore no passed then get it from bootstrap.
        if (m_detStore == nullptr) {
            StatusCode sc = Gaudi::svcLocator()->service("DetectorStore", m_detStore);
            if (sc.isFailure()) msg(MSG::ERROR) << "Could not locate DetectorStore" << endmsg;
        }

        m_barrelXF[0]=m_barrelXF[1]=m_barrelXF[2]=nullptr;
        m_endcapXF[0]=m_endcapXF[1]=m_endcapXF[2]=nullptr;
    }



    TRT_Numerology * TRT_DetectorManager::getNumerology() {
        return m_numerology;
    }

    const TRT_Numerology * TRT_DetectorManager::getNumerology() const {
        return m_numerology;
    }

    TRT_DetectorManager::~TRT_DetectorManager()
    {
        for (auto & i : m_volume) {
            i->unref();
        }
        delete m_numerology;
        if (m_ownsIdHelper)    delete m_idHelper;
        for (auto & i : m_barrelXF) delete i;
        for (auto & i : m_endcapXF) delete i;


        for (auto & m : m_alignableTransforms) {
            for (auto & j : m) {
                delete j.second;
            }
        }

        for (const TRT_BarrelDescriptor* barrelDescriptor : m_barrelDescriptors) {
            delete barrelDescriptor;
        }
        for (const TRT_EndcapDescriptor* endcapDescriptor : m_endcapDescriptors) {
            delete endcapDescriptor;
        }
    }


    unsigned int TRT_DetectorManager::getNumTreeTops() const
    {
        return m_volume.size();
    }

    PVConstLink TRT_DetectorManager::getTreeTop(unsigned int i) const
    {
        return m_volume[i];
    }

    void  TRT_DetectorManager::addTreeTop(PVLink vol){
        vol->ref();
        m_volume.push_back(vol);
    }
  
    // Manage the barrel elements:
    void TRT_DetectorManager::manageBarrelElement(TRT_BarrelElement *barrel) 
    {
      m_elementContainer.manageBarrelElement(barrel,m_idHelper);
    }
  
    // Manage the endcap elements:
    void TRT_DetectorManager::manageEndcapElement(TRT_EndcapElement *endcap) 
    {
      m_elementContainer.manageEndcapElement(endcap,m_idHelper);
    }

    const TRT_BarrelElement *TRT_DetectorManager::getBarrelElement(unsigned int positive
                                                                   , unsigned int moduleIndex
                                                                   , unsigned int phiIndex
                                                                   , unsigned int strawLayerIndex) const 
    {
      return m_elementContainer.getBarrelDetElement(positive,moduleIndex,phiIndex,strawLayerIndex);
    }

    TRT_BarrelElement *TRT_DetectorManager::getBarrelElement(unsigned int positive
							     , unsigned int moduleIndex
							     , unsigned int phiIndex
							     , unsigned int strawLayerIndex)
    {
      return m_elementContainer.getBarrelDetElement(positive,moduleIndex,phiIndex,strawLayerIndex);
    }

    const TRT_EndcapElement *TRT_DetectorManager::getEndcapElement(unsigned int positive
								   , unsigned int wheelIndex
								   , unsigned int strawLayerIndex
								   , unsigned int phiIndex) const 
    {
      return m_elementContainer.getEndcapDetElement(positive,wheelIndex,strawLayerIndex,phiIndex);
    }

    TRT_EndcapElement *TRT_DetectorManager::getEndcapElement(unsigned int positive
							     , unsigned int wheelIndex
							     , unsigned int strawLayerIndex
							     , unsigned int phiIndex) 
    {
      return m_elementContainer.getEndcapDetElement(positive,wheelIndex,strawLayerIndex,phiIndex);
    }

    const TRT_ID *TRT_DetectorManager::getIdHelper() const 
    {
        return m_idHelper;
    }

    void TRT_DetectorManager::setIdHelper(const TRT_ID *idHelper, bool owns) 
    {
        m_idHelper=idHelper;
        m_ownsIdHelper=owns;
    }



    const TRT_BaseElement *TRT_DetectorManager::getElement(Identifier id) const 
    {
      // Make sure it is a straw_layer id
      Identifier strawLayerId = m_idHelper->layer_id(id);
      IdentifierHash hashId = m_idHelper->straw_layer_hash(strawLayerId);
      const TRT_DetElementCollection* elements = m_elementContainer.getElements();
      if (hashId>=elements->size()) return nullptr;
      return (*elements)[hashId];
    }

    const TRT_BaseElement *TRT_DetectorManager::getElement(IdentifierHash id) const 
    {
      const TRT_DetElementCollection* elements = m_elementContainer.getElements();
      if (id>=elements->size()) return nullptr;
      return (*elements)[id];
    }

    const TRT_DetElementContainer* TRT_DetectorManager::getDetectorElementContainer() const
    {
      return &m_elementContainer;
    }

    const TRT_DetElementCollection * TRT_DetectorManager::getDetectorElementCollection() const 
    {
      return m_elementContainer.getElements();
    }

    TRT_DetElementCollection::const_iterator TRT_DetectorManager::getDetectorElementBegin() const 
    {
      return m_elementContainer.getElements()->begin();
    }

    TRT_DetElementCollection::const_iterator TRT_DetectorManager::getDetectorElementEnd() const 
    {
      return m_elementContainer.getElements()->end();
    }


    void TRT_DetectorManager::setBarrelTransformField(size_t i, const GeoXF::Function * f){
        if (m_barrelXF[i]!=f)  delete  m_barrelXF[i];
        m_barrelXF[i] = f;
    }

    const GeoXF::Function * TRT_DetectorManager::barrelTransformField(size_t i) const {
        return m_barrelXF[i];
    }

    void TRT_DetectorManager::setEndcapTransformField(size_t i, const GeoXF::Function *f) {
        if (m_endcapXF[i]!=f) delete  m_endcapXF[i];
        m_endcapXF[i]=f;
    }

    const GeoXF::Function *TRT_DetectorManager::endcapTransformField(size_t i) const{
        return m_endcapXF[i];
    }


    TRT_DetectorManager::ActiveGasType TRT_DetectorManager::gasType() const
    {
        return m_gasType;
    }

    void TRT_DetectorManager::setGasType(const ActiveGasType & activeGasType)
    {
        m_gasType = activeGasType;
    }

    unsigned int TRT_DetectorManager::digitizationVersion() const
    {
        return m_digvers;
    }

    std::string TRT_DetectorManager::digitizationVersionName() const
    {
        return m_digversname;
    }

    void TRT_DetectorManager::setDigitizationVersion(const unsigned int & dv, const std::string& name )
    {
        m_digvers = dv; m_digversname = name;
    }

    // Register the call back for this key and the corresponding level in
    // in the hierarchy.
    // DEPRECATED
    void TRT_DetectorManager::addKey ATLAS_NOT_THREAD_SAFE (const std::string & key, int level) // Thread unsafe m_detStore->regFcn (callback) is used.
    {
        if(msgLvl(MSG::DEBUG))
            msg(MSG::DEBUG) << "Registering alignmentCallback with key " << key << ", at level " << level
            << endmsg;

        const DataHandle<AlignableTransform> transformCollection;
        if (m_detStore->regFcn(&TRT_DetectorManager::alignmentCallback, this, transformCollection, key).isFailure()) {
          ATH_MSG_ERROR("Cannot register callback with DetectorStore");
        }
        addKey(key, level, InDetDD::other);
    }

    void TRT_DetectorManager::addKey(const std::string & key, int level, FrameType frame)
    {
        addChannel(key, level, frame);
    }

    void TRT_DetectorManager::addAlignableTransform (int level,
                                                     const Identifier &id,
                                                     GeoAlignableTransform *transform,
                                                     const GeoVPhysVol * child,
                                                     const GeoVPhysVol * frameVol)
    {
        if (m_idHelper) {
            // Check if child and frame are actually full physical volumes.
            // if they are non zero.
            const GeoVFullPhysVol * childFPV = nullptr;
            if (child) {
                childFPV = dynamic_cast<const GeoVFullPhysVol *>(child);
            }
            const GeoVFullPhysVol * frameFPV = nullptr;
            if (frameVol) {
                frameFPV = dynamic_cast<const GeoVFullPhysVol *>(frameVol);
            }
            if (child && !childFPV) {
                msg(MSG::ERROR)
                    << "Child of alignable transform is not a full physical volume"
                    << endmsg;
            } else if (frameVol && !frameFPV) {
                msg(MSG::ERROR)
                    << "Frame for alignable transform is not a full physical volume"
                    << endmsg;
            } else {
                addAlignableTransform (level, id, transform, childFPV, frameFPV);
            }
        }
    }

    void TRT_DetectorManager::addAlignableTransform (int level,
                                                     const Identifier &id,
                                                     GeoAlignableTransform *transform,
                                                     const GeoVFullPhysVol *child,
                                                     const GeoVFullPhysVol *frameVol)
    {
        if (m_idHelper) {
            if (level == 0) {
                // Nothing implemented. Reserved in case we want alignable straws.
            } else {

                ExtendedAlignableTransform * extAlignableTransform = new ExtendedAlignableTransform(transform, child, frameVol);
                if(msgLvl(MSG::VERBOSE)) {
                    msg(MSG::VERBOSE) << "TRT: Adding alignment at level " << level << " " << m_idHelper->show_to_string(id);
                    if (child && !frameVol) {
                        msg(MSG::VERBOSE) << " using global frame";
                    } else if (!child || child == frameVol ) {
                        msg(MSG::VERBOSE) << " using local frame";
                    } else {
                        msg(MSG::VERBOSE) << " using other frame";
                    }
                    msg(MSG::VERBOSE) << endmsg;
                }
                // Save in map
                int index = level - FIRST_HIGHER_LEVEL; // level 0 treated separately.
                if (index >= static_cast<int>(m_alignableTransforms.size())) m_alignableTransforms.resize(index+1);
                m_alignableTransforms[index][id] = extAlignableTransform;
            }
        }
    }

    bool TRT_DetectorManager::setAlignableTransformDelta(int level,
                                                         const Identifier & id,
                                                         const Amg::Transform3D & delta,
                                                         FrameType frame,
                                                         GeoVAlignmentStore* alignStore) const
    {
        if (level == 0) {
      // Nothing implemented. Reserved in case we want alignable straws
            return false;
        } else {

            int index = level - FIRST_HIGHER_LEVEL; // level 0 treated separately.
            if (index  >=  static_cast<int>(m_alignableTransforms.size())) return false;

            // We retrieve it from a map.
            AlignableTransformMap::const_iterator iter;
            iter = m_alignableTransforms[index].find(id);
            if (iter == m_alignableTransforms[index].end()) return false;

            return setAlignableTransformAnyFrameDelta(iter->second, delta, frame, alignStore);

        }
    }

    bool TRT_DetectorManager::setAlignableTransformAnyFrameDelta(ExtendedAlignableTransform * extXF,
                                                                 const Amg::Transform3D & delta,
                                                                 FrameType frame,
                                                                 GeoVAlignmentStore* alignStore) const
    {
    //---------------------
    // For Local:
    //---------------------
    // The geomodel alignable transform delta is already a local delta so we just pass it directly

    //---------------------
    // For global frame
    //---------------------
    // Sets the alignable transform delta when the supplied delta is in the global frame.

    // If the default transform down to the alignable transform is
    // T = A*B*C
    // and the alignable transform is C with delta c and the delta in the global frame is g, then
    // A*B*C*c = g*A*B*C
    // T*c = g*T
    //  c = T.inverse() * g * T

    // To get default transform up and including the alignable transform,
    // we assume the next volume down is a fullphys volume and so its
    // transform is the transform we want (ie T=A*B*C in the above example).

    //---------------------
    // For Other frame
    //---------------------
    // Sets the alignable transform delta when the supplied delta is in the frame of the
    // volume "frameVol".

    // If the default transform down to the alignable transform is
    // T = A*B*C
    // and the alignable transform is C with delta c and the delta g is expressed in the frame A, then
    // A*B*C*c = A*g*B*C
    // c = (BC).inverse * g * (BC)
    // BC = A.inverse() * T
    // C = T.inverse() * A * g * A.inverse() * T

    // To get default transform up and including the alignable transform,
    // we assume the next volume down is a fullphys volume and so its
    // transform is the transform we want (ie T=A*B*C in the above example).
    // The Transform to the frame is T = A which we get from the fullphys volume frameVol.

        if (!extXF) return false;
        if (!extXF->alignableTransform()) return false;

        const GeoVFullPhysVol * child = extXF->child();
        const GeoVFullPhysVol * frameVol = extXF->frame();

        FrameType newFrame = frame;
        // If frame is other then check if "other" is actually local or global
        if (frame == InDetDD::other) {
            if (child && !frameVol) {
                // frame = 0 indicates to use global frame
                newFrame =  InDetDD::global;
            } else if (!child || child == frameVol){
                // if child is 0 or the they are the same volumes then its local
                newFrame =  InDetDD::local;
            } // else its "other" already.
        }

        if (newFrame == InDetDD::global)  { // Global
            if (!child) {
                msg(MSG::ERROR) << "global frame specified, but child == 0" << endmsg;
            } else {
                const GeoTrf::Transform3D & childXF = child->getDefAbsoluteTransform(alignStore);
                extXF->alignableTransform()->setDelta(childXF.inverse() * delta * childXF);
            }

        } else if (frame == InDetDD::local) { // Local
            // if its a local frame then no transform necessary. We set it directly.
            extXF->alignableTransform()->setDelta(delta);

        } else { // Other frame
            // if child or frame is zero it will have been set to local or global above
            if (!child) { // CID 113112
              // shouldn't be happening, if child is null then something is terribly wrong
              ATH_MSG_ERROR("Child can't be null if frame is 'other'");
              return false;
            } else {
	            const GeoTrf::Transform3D & xfChild = child->getDefAbsoluteTransform(alignStore);
	            const GeoTrf::Transform3D & xfFrame = frameVol->getDefAbsoluteTransform(alignStore);
	            extXF->alignableTransform()->setDelta(xfChild.inverse() * xfFrame * delta * xfFrame.inverse() * xfChild);
            }
        }

        return true;
    }


    StatusCode TRT_DetectorManager::alignmentCallback( IOVSVC_CALLBACK_ARGS_P(I,keys) )
    {
        return align(I, keys);
    }


  // We invalidate all the elements if at least one alignment changed.
    void TRT_DetectorManager::invalidateAll() const
    {
        for (TRT_DetElementCollection::const_iterator element_iter = getDetectorElementBegin();
        element_iter != getDetectorElementEnd();
        ++element_iter) {

            if (*element_iter) {
                (*element_iter)->invalidate();
            }
        }
    }

    void TRT_DetectorManager::updateAll() const
    {
        for (TRT_DetElementCollection::const_iterator element_iter = getDetectorElementBegin();
        element_iter != getDetectorElementEnd();
        ++element_iter) {

            if (*element_iter) {
                (*element_iter)->updateAllCaches();
            }
        }
    }


    bool TRT_DetectorManager::identifierBelongs(const Identifier & id) const
    {
        return getIdHelper()->is_trt(id);
    }


    bool TRT_DetectorManager::processSpecialAlignment(const std::string & key, InDetDD::AlignFolderType /*dummy*/) const
    {
        if(msgLvl(MSG::DEBUG))
            msg(MSG::DEBUG) << "Processing TRT fine alignment." << endmsg;

        const TRTCond::StrawDxContainer* container = nullptr;
        StatusCode sc = StatusCode::FAILURE;
        if (m_detStore->contains<TRTCond::StrawDxContainer>(key)) {
            sc = m_detStore->retrieve(container, key);
        }

        if (sc.isFailure()) {
            if (msgLvl(MSG::INFO))
                msg(MSG::INFO) << "Cannot find StrawDxContainer for key "
                << key << " - no fine alignment " << endmsg;
            throw std::runtime_error("Unable to apply TRT fine alignment. This is normal for simulation");
      //return false;
        } else {

      // Loop trough all barrel elements and pass container.

            this->setDxContainer(container);

            for (TRT_DetElementCollection::const_iterator element_iter = getDetectorElementBegin();
            element_iter != getDetectorElementEnd();
            ++element_iter) {
                TRT_BaseElement * element = *element_iter;
                if (element) {
                    element->invalidate();
                }
            }
            return false; // we return false as we have already invalidated the elements
        }
    }

    bool TRT_DetectorManager::processSpecialAlignment(const std::string& /*key*/,
                                                      const CondAttrListCollection* /*obj*/,
                                                      GeoVAlignmentStore* /*alignStore*/) const {
      return false;
    }


    const TRT_Conditions* TRT_DetectorManager::conditions() const
    {
      return this;
    }

  // New global alignment filders
  bool TRT_DetectorManager::processGlobalAlignment(const std::string & key, int level, FrameType frame,
                                                   const CondAttrListCollection* obj, GeoVAlignmentStore* alignStore) const
  {

    bool alignmentChange = false;

    ATH_MSG_INFO("Processing new global alignment containers with key " << key << " in the " << frame << " frame at level " << level);

    Identifier ident=Identifier();
    const CondAttrListCollection* atrlistcol=obj;
    //cppcheck-suppress nullPointerRedundantCheck
    if (!atrlistcol) {
       ATH_MSG_INFO("Read alignment from detector store with key " << key);
       if (StatusCode::SUCCESS!=m_detStore->retrieve(atrlistcol,key)) {
          ATH_MSG_WARNING("Cannot find new global align Container for key "
                          << key << " - no new global alignment");
          return alignmentChange;
       }
    }
    {
      // loop over objects in collection
      //cppcheck-suppress nullPointerRedundantCheck
      for (const auto & citr : *atrlistcol) {

        const coral::AttributeList& atrlist=citr.second;
	ident = getIdHelper()->module_id(atrlist["bec"].data<int>(),
                                        atrlist["layer"].data<int>(),
					atrlist["sector"].data<int>());

	// Follow same definitions as in TRT_AlignDbSvc.cxx
	CLHEP::Hep3Vector newtranslation(atrlist["Tx"].data<float>(),atrlist["Ty"].data<float>(),atrlist["Tz"].data<float>());
	CLHEP::HepRotation newrotation;
	newrotation.set(atrlist["phi"].data<float>(),atrlist["theta"].data<float>(),atrlist["psi"].data<float>());
	HepGeom::Transform3D newtransform(newrotation, newtranslation);

        msg(MSG::DEBUG) << "New global DB -- channel: " << citr.first
			<< " ,bec: "    << atrlist["bec"].data<int>()
                        << " ,layer: "  << atrlist["layer"].data<int>()
			<< " ,sector: " << atrlist["sector"].data<int>()
                        << " ,Tx: "     << atrlist["Tx"].data<float>()
                        << " ,Ty: "     << atrlist["Ty"].data<float>()
                        << " ,Tz: "     << atrlist["Tz"].data<float>()
                        << " ,phi: "    << atrlist["phi"].data<float>()
                        << " ,theta: "  << atrlist["theta"].data<float>()
                        << " ,psi: "    << atrlist["psi"].data<float>() << endmsg;

	// Set the new transform; Will replace existing one with updated transform
        bool status = setAlignableTransformDelta(level,
                                                 ident,
                                                 Amg::CLHEPTransformToEigen(newtransform),
                                                 frame,
                                                 alignStore);

        if (!status) {
          if (msgLvl(MSG::DEBUG)) {
            msg(MSG::DEBUG) << "Cannot set AlignableTransform for identifier."
                            << getIdHelper()->show_to_string(ident)
                            << " at level " << level << " for new global DB " << endmsg;
          }
        }

        alignmentChange = (alignmentChange || status);
      }
    }
    return alignmentChange;
  }

  void TRT_DetectorManager::setBarrelDescriptor(const TRT_BarrelDescriptor* barrelDescriptor)
  {
    m_barrelDescriptors.insert(barrelDescriptor);
  }

  void TRT_DetectorManager::setEndcapDescriptor(const TRT_EndcapDescriptor* endcapDescriptor)
  {
    m_endcapDescriptors.insert(endcapDescriptor);
  }

} // namespace InDetDD
