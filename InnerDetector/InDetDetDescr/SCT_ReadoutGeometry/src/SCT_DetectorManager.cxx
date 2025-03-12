/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/ExtendedAlignableTransform.h"
#include "StoreGate/StoreGateSvc.h"

#include <iostream>

namespace InDetDD {

  const int FIRST_HIGHER_LEVEL = 2;


  SCT_DetectorManager::SCT_DetectorManager( StoreGateSvc* detStore )
    : SCT_DetectorManager(detStore, "SCT") {}

  SCT_DetectorManager::SCT_DetectorManager( StoreGateSvc* detStore,
                                            const std::string& name,
                                            const bool doEndcapEtaNeighbour )
    : SiDetectorManager(detStore,name),
      m_idHelper(nullptr),
      m_isLogical(false), // Change to true to change the definition of local module corrections
      m_doEndcapEtaNeighbour(doEndcapEtaNeighbour)
  {
    ATH_MSG_VERBOSE("Creating SCT_DetectorManager named " << name);
    //  
    // Initialized the Identifier helper.
    //
    StatusCode sc = detStore->retrieve(m_idHelper, "SCT_ID");  
    if (sc.isFailure()) {
      ATH_MSG_ERROR("Could not retrieve SCT id helper");
    }
    // Initialize the collections.
    if (m_idHelper) {
      m_elementCollection.resize(m_idHelper->wafer_hash_max());
      m_alignableTransforms.resize(m_idHelper->wafer_hash_max());
      m_moduleAlignableTransforms.resize(m_idHelper->wafer_hash_max()/2);
    } 
  }

  unsigned int SCT_DetectorManager::getNumTreeTops() const
  {
    return m_volume.size(); 
  }

  PVConstLink SCT_DetectorManager::getTreeTop(unsigned int i) const
  {
    return m_volume[i];
  }

  void SCT_DetectorManager::addTreeTop(const PVConstLink& vol){
    m_volume.push_back(vol);
  }

  SiDetectorElement* SCT_DetectorManager::getDetectorElement(const Identifier & id) const
  {  
    // NB the id helpers implementation for getting a hash is not optimal.
    // Essentially does a binary search.
    // Make sure it is a wafer Id
    Identifier waferId =  m_idHelper->wafer_id(id);
    IdentifierHash idHash = m_idHelper->wafer_hash(waferId);
    if (idHash.is_valid()) {
      return m_elementCollection[idHash];
    } else {
      return nullptr;
    }
  }

  SiDetectorElement* SCT_DetectorManager::getDetectorElement(const IdentifierHash & idHash) const
  {
    return m_elementCollection[idHash];
  }

  SiDetectorElement* SCT_DetectorManager::getDetectorElement(int barrel_endcap, int layer_wheel, int phi_module, int eta_module, int side) const
  {
    return getDetectorElement(m_idHelper->wafer_id(barrel_endcap, layer_wheel, phi_module, eta_module, side));
  }

  const SiDetectorElementCollection* SCT_DetectorManager::getDetectorElementCollection() const
  { 
    return &m_elementCollection;
  }

  SiDetectorElementCollection::const_iterator SCT_DetectorManager::getDetectorElementBegin() const
  {
    return m_elementCollection.begin();
  }

  SiDetectorElementCollection::const_iterator SCT_DetectorManager::getDetectorElementEnd() const
  {
    return m_elementCollection.end();
  }


  void SCT_DetectorManager::addDetectorElement(SiDetectorElement * element)
  {
    IdentifierHash idHash = element->identifyHash();
    if (idHash >=  m_elementCollection.size())
      throw std::runtime_error("SCT_DetectorManager: Error adding detector element.");
    m_elementCollection[idHash] = element;
  }

  void SCT_DetectorManager::initNeighbours()
  {
    SiDetectorElementCollection::iterator iter;

    // Loop over all elements and set the neighbours
    for (iter = m_elementCollection.begin(); iter != m_elementCollection.end(); ++iter){

      SiDetectorElement * element = *iter;
      if (element) {

        IdentifierHash idHash = element->identifyHash();
        IdentifierHash idHashOther;

        int result;
        // If no neighbour, result != 0 in which case we leave neighbour as null
        if(element->isBarrel()){
          result = m_idHelper->get_next_in_eta(idHash, idHashOther);
          if (result==0) element->setNextInEta(m_elementCollection[idHashOther]);
        }
        else if(m_doEndcapEtaNeighbour){
          // In endcaps the neighbours cannot be found with id+/-1, therefore we cannot rely on SCT_ID and need
          // a dedicated search, considering only neighbour at larger radius (compatible wih outgoing particle)
          result = getStripEndcapEtaNeighbour(element, idHashOther, false);
          if(result==0) element->setNextInEta(m_elementCollection[idHashOther]);

          // In ITk strip endcap, when we move from eta_module=9 to eta_module=10 we have change of module 
          // granularity in phi: twice more module in eta_module=10, therefore for module at eta_module=9
          // and phi_module=phi we have two neighbours at larger radius: both at eta_module=10, but one with
          // phi_module=2*phi the other with phi_module=2*phi+1, for completeness we store this neighbour in
          // "PrevInEta" even if it is a misnomer in this case
          if( m_idHelper->eta_module(element->identify())==9 ){
            result = getStripEndcapEtaNeighbour(element, idHashOther, true);
            if(result==0) element->setPrevInEta(m_elementCollection[idHashOther]);
          }
        }

        result = m_idHelper->get_prev_in_eta(idHash, idHashOther);
        if (result==0) element->setPrevInEta(m_elementCollection[idHashOther]);

        result = m_idHelper->get_next_in_phi(idHash, idHashOther);
        if (result==0) element->setNextInPhi(m_elementCollection[idHashOther]);

        result = m_idHelper->get_prev_in_phi(idHash, idHashOther);
        if (result==0) element->setPrevInPhi(m_elementCollection[idHashOther]);

        result = m_idHelper->get_other_side(idHash, idHashOther);
        if (result==0) element->setOtherSide(m_elementCollection[idHashOther]);
      }
    }
  }

int SCT_DetectorManager::getStripEndcapEtaNeighbour(const SiDetectorElement* element, 
                                                    IdentifierHash& idHashNeighbour,
                                                    const bool phi_plus_one) const
{
  // Check we are well in strip endcap
  if( !(element->isSCT() && element->isEndcap()) ) return 1;

  // Now we try to find a neighbour at larger radius
  Identifier id = element->identify();
  int bec = m_idHelper->barrel_ec(id);
  int layer_disk = m_idHelper->layer_disk(id);
  int phi_module = m_idHelper->phi_module(id);
  int eta_module = m_idHelper->eta_module(id);
  int side = m_idHelper->side(id);

  // We want to find neighbour only for elements on "main" side 
  // since the space points are made from trigger cluster (on side 0 for ITk)
  // and stero cluster (on side 1 for ITk)
  if(element->isStereo()) return 1;

  // Most of the time we want to find neighbours in same phi
  // but in transition region (eta_module 9->10) the module granularity in phi changes
  // we have to look for two neighours, at: 2*phi and 2*phi+1
  int target_phi = phi_module;
  if (eta_module==9) target_phi = (phi_plus_one)? 2*phi_module+1 : 2*phi_module;

  // Brute force search, loop on all elements to find a neighbour
  for(const SiDetectorElement* other_element : m_elementCollection){

    Identifier other_id = other_element->identify();
    
    // To speed up the search, screening firt on bec, then layer_disk, etc...
    int other_bec = m_idHelper->barrel_ec(other_id);
    if(other_bec != bec) continue;

    int other_layer_disk = m_idHelper->layer_disk(other_id);
    if(other_layer_disk != layer_disk) continue;

    int other_phi_module = m_idHelper->phi_module(other_id);
    if(other_phi_module != target_phi) continue;

    // We keep only neighbour at eta_module+1 (larger radius)
    int other_eta_module = m_idHelper->eta_module(other_id);
    if( (other_eta_module-eta_module) != 1 ) continue;

    // We keep only neighbour on stereo side (side=1 for ITk)
    int other_side = m_idHelper->side(other_id);
    if(other_side==side) continue;

    // If we are here, we found it!
    idHashNeighbour = other_element->identifyHash();
    ATH_MSG_VERBOSE(__FUNCTION__<<"found strip endcap neighbour for id="<<id<<" neigh="<<other_id
                                <<" eta:"<<eta_module<<" "<<other_eta_module);
    return 0;
  }

  return 1;
}

  const SCT_ID* SCT_DetectorManager::getIdHelper() const
  {
    return m_idHelper;
  }


  bool SCT_DetectorManager::setAlignableTransformDelta(int level, 
                                                       const Identifier & id, 
                                                       const Amg::Transform3D & delta,
                                                       FrameType frame,
                                                       GeoVAlignmentStore* alignStore) const
  {

    if (level == 0) { // 0 - At the element level

      // We retrieve it via a hashId.
      IdentifierHash idHash = m_idHelper->wafer_hash(id);
      if (!idHash.is_valid()) return false;

      if (frame == InDetDD::global) { // global shift
        // Its a global transform
        return setAlignableTransformGlobalDelta(m_alignableTransforms[idHash].get(), delta, alignStore);

      } else if (frame == InDetDD::local) { // local shift

        SiDetectorElement * element =  m_elementCollection[idHash];
        if (!element) return false;


        // Its a local transform
        //See header file for definition of m_isLogical          
        if( m_isLogical ){
          //Ensure cache is up to date and use the alignment corrected local to global transform
          element->setCache();
          return setAlignableTransformLocalDelta(m_alignableTransforms[idHash].get(), element->transform(), delta, alignStore);
        } else 
          //Use default local to global transform
          return setAlignableTransformLocalDelta(m_alignableTransforms[idHash].get(), element->defTransform(), delta, alignStore);

      } else {   
        // other not supported
        ATH_MSG_WARNING("Frames other than global or local are not supported.");
        return false;
      }

    } else if (level == 1) { // module level

      // We retrieve it via a hashId.
      IdentifierHash idHash = m_idHelper->wafer_hash(id);
      if (!idHash.is_valid()) return false;

      int idModuleHash = idHash / 2;

      if (idHash%2) {
        ATH_MSG_WARNING("Side 1 wafer id used for module id");
        return false;
      }

      if (frame == InDetDD::global) { // global shift
        // Its a global transform
        return setAlignableTransformGlobalDelta(m_moduleAlignableTransforms[idModuleHash].get(), delta, alignStore);
      } else if (frame == InDetDD::local) { // local shift
        SiDetectorElement * element =  m_elementCollection[idHash];
        if (!element) return false;
       
        // Its a local transform
        //See header file for definition of m_isLogical          
        if( m_isLogical ){
          //Ensure cache is up to date and use the alignment corrected local to global transform
          element->setCache();
          return setAlignableTransformLocalDelta(m_moduleAlignableTransforms[idModuleHash].get(), element->moduleTransform(), delta, alignStore);
        } else 
          //Use default local to global transform
          return setAlignableTransformLocalDelta(m_moduleAlignableTransforms[idModuleHash].get(), element->defModuleTransform(), delta, alignStore);

      } else {
        // other not supported
        ATH_MSG_WARNING("Frames other than global or local are not supported.");
        return false;
      }

    } else { // higher level

      if (frame != InDetDD::global) {
        ATH_MSG_WARNING("Non global shift at higher levels is not supported.");
        return false;
      }

      int index = level - FIRST_HIGHER_LEVEL; // level 0 and 1 is treated separately.
      if (index  >=  static_cast<int>(m_higherAlignableTransforms.size())) return false;

      // We retrieve it from a map. 
      AlignableTransformMap::const_iterator iter;    
      iter = m_higherAlignableTransforms[index].find(id);
      if (iter == m_higherAlignableTransforms[index].end()) return false;      

      // Its a global transform
      return setAlignableTransformGlobalDelta((iter->second).get(), delta, alignStore);
    }

  }

  void SCT_DetectorManager::addAlignableTransform (int level, 
                                                   const Identifier & id, 
                                                   GeoAlignableTransform *transform,
                                                   const GeoVPhysVol * child)
  {
    if (m_idHelper) {

      const GeoVFullPhysVol * childFPV = dynamic_cast<const GeoVFullPhysVol *>(child);
      if (!childFPV) { 
        ATH_MSG_ERROR("Child of alignable transform is not a full physical volume");
      } else {
        addAlignableTransform (level, id, transform, childFPV);
      }
    }
  }

  void SCT_DetectorManager::addAlignableTransform (int level, 
                                                   const Identifier & id, 
                                                   GeoAlignableTransform *transform,
                                                   const GeoVFullPhysVol * child)
  { 
    if (m_idHelper) {
      if (level == 0) { 
        // Element
        IdentifierHash idHash = m_idHelper->wafer_hash(id);
        if (idHash.is_valid()) {
          m_alignableTransforms[idHash] = std::make_unique<ExtendedAlignableTransform>(transform, child);
        } 
      } else if (level == 1) {
        // Module
        IdentifierHash idHash = m_idHelper->wafer_hash(id);
        if (idHash.is_valid()) {
          m_moduleAlignableTransforms[idHash/2] = std::make_unique<ExtendedAlignableTransform>(transform, child);
        } 

      } else {

        // Higher levels are saved in a map. NB level=0,1 is treated above.   
        int index = level - FIRST_HIGHER_LEVEL; // level 0 and 1 is treated separately.
        if (index >= static_cast<int>(m_higherAlignableTransforms.size())) m_higherAlignableTransforms.resize(index+1); 
        m_higherAlignableTransforms[index][id] = std::make_unique<ExtendedAlignableTransform>(transform, child);
      }  
    }
  }

  bool
  SCT_DetectorManager::identifierBelongs(const Identifier & id) const
  {
    return getIdHelper()->is_sct(id);
  }


  const SCT_ModuleSideDesign* SCT_DetectorManager::getSCT_Design(int i) const
  {
    return dynamic_cast<const SCT_ModuleSideDesign *>(getDesign(i));
  }

  // New global alignment folders
  bool SCT_DetectorManager::processGlobalAlignment(const std::string & key, int level, FrameType frame,
                                                   const CondAttrListCollection* obj, GeoVAlignmentStore* alignStore) const
  {
    ATH_MSG_INFO("Processing new global alignment containers with key " << key << " in the " << frame << " frame at level ");

    const CondAttrListCollection* atrlistcol=obj;
    if(atrlistcol==nullptr and m_detStore->retrieve(atrlistcol,key)!=StatusCode::SUCCESS) {
      ATH_MSG_WARNING("Cannot find new global align Container for key "
                      << key << " - no new global alignment ");
      return false;
    }

    bool alignmentChange = false;
    Identifier ident=Identifier();

    // loop over objects in collection
    for (CondAttrListCollection::const_iterator citr=atrlistcol->begin(); citr!=atrlistcol->end();++citr) {
      const coral::AttributeList& atrlist=citr->second;
      // SCT manager, therefore ignore all that is not a SCT Identifier
      if (atrlist["det"].data<int>()!=2) continue;

      ident = getIdHelper()->wafer_id(atrlist["bec"].data<int>(),
                                      atrlist["layer"].data<int>(),
                                      atrlist["ring"].data<int>(),
                                      atrlist["sector"].data<int>(),
                                      0); // The last is the module side which is at this ident-level always the 0-side

      // construct new transform
      // Order of rotations is defined as around z, then y, then x.
      Amg::Translation3D  newtranslation(atrlist["Tx"].data<float>(),atrlist["Ty"].data<float>(),atrlist["Tz"].data<float>());
      Amg::Transform3D newtrans = newtranslation * Amg::RotationMatrix3D::Identity();
      newtrans *= Amg::AngleAxis3D(atrlist["Rz"].data<float>()*CLHEP::mrad, Amg::Vector3D(0.,0.,1.));
      newtrans *= Amg::AngleAxis3D(atrlist["Ry"].data<float>()*CLHEP::mrad, Amg::Vector3D(0.,1.,0.));
      newtrans *= Amg::AngleAxis3D(atrlist["Rx"].data<float>()*CLHEP::mrad, Amg::Vector3D(1.,0.,0.));

      ATH_MSG_DEBUG("New global DB -- channel: " << citr->first
                    << " ,det: "    << atrlist["det"].data<int>()
                    << " ,bec: "    << atrlist["bec"].data<int>()
                    << " ,layer: "  << atrlist["layer"].data<int>()
                    << " ,ring: "   << atrlist["ring"].data<int>()
                    << " ,sector: " << atrlist["sector"].data<int>()
                    << " ,Tx: "     << atrlist["Tx"].data<float>()
                    << " ,Ty: "     << atrlist["Ty"].data<float>()
                    << " ,Tz: "     << atrlist["Tz"].data<float>()
                    << " ,Rx: "     << atrlist["Rx"].data<float>()
                    << " ,Ry: "     << atrlist["Ry"].data<float>()
                    << " ,Rz: "     << atrlist["Rz"].data<float>());

      // Set the new transform; Will replace existing one with updated transform
      bool status = setAlignableTransformDelta(level,
                                               ident,
                                               newtrans,
                                               frame,
                                               alignStore);

      if (!status) {
        ATH_MSG_DEBUG("Cannot set AlignableTransform for identifier."
                      << getIdHelper()->show_to_string(ident)
                      << " at level " << level << " for new global DB ");
      }

      alignmentChange = (alignmentChange || status);
    }
    return alignmentChange;
  }

bool SCT_DetectorManager::processSpecialAlignment(
    const std::string &, InDetDD::AlignFolderType) const {
  return false;
}

bool SCT_DetectorManager::processSpecialAlignment(const std::string& /*key*/,
                                                  const CondAttrListCollection* /*obj*/,
                                                  GeoVAlignmentStore* /*alignStore*/) const {
  return false;

}

  void SCT_DetectorManager::addMotherDesign(std::unique_ptr<const SCT_ModuleSideDesign>&& motherDesign){
    m_motherDesigns.push_back(std::move(motherDesign));
  }

} // namespace InDetDD
