/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonGeoModelR4/MuonGeoUtilityTool.h>

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <GeoModelKernel/GeoBox.h>
#include <GeoModelKernel/GeoTrd.h>
#include <GeoModelKernel/GeoSimplePolygonBrep.h>

#include <GeoModelKernel/GeoTube.h>

#include <GeoModelKernel/GeoShapeUnion.h>
#include <GeoModelKernel/GeoShapeIntersection.h>
#include <GeoModelKernel/GeoShapeSubtraction.h>
#include <GeoModelKernel/GeoShapeShift.h>

#include <GeoModelKernel/GeoTransform.h>

#include <GeoModelKernel/GeoVolumeCursor.h>

#include <GeoModelHelpers/GeoShapeUtils.h>
#include <GeoModelHelpers/TransformToStringConverter.h>

#include <set>
#include <sstream>
#include <string>


using namespace ActsTrk;
namespace MuonGMR4{

MuonGeoUtilityTool::~MuonGeoUtilityTool() = default;
MuonGeoUtilityTool::MuonGeoUtilityTool(const std::string &type, const std::string &name,
                                       const IInterface *parent):
    AthAlgTool(type,name,parent) {
    declareInterface<IMuonGeoUtilityTool>(this);
}

const GeoShape* MuonGeoUtilityTool::extractShape(const PVConstLink& physVol) const {
    const GeoLogVol* logVol = physVol->getLogVol();
    if (!logVol) {
        ATH_MSG_ERROR(__FILE__<<":"<<__LINE__<<" Physical volume has no logical volume attached ");
        return nullptr;
    }
    return extractShape(logVol->getShape());
}
const GeoShape* MuonGeoUtilityTool::extractShape(const GeoShape* inShape) const {
   
    if (!inShape) {
      ATH_MSG_INFO(__FILE__<<":"<<__LINE__<<" "<<__func__<<" nullptr given ");
      return nullptr;
    }    
    if (inShape->typeID() == GeoShapeShift::getClassTypeID()) {
        const GeoShapeShift* shift =  dynamic_pointer_cast<const GeoShapeShift>(compressShift(inShape));
        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<
                        "Shape is a shift by "<<GeoTrf::toString(shift->getX())
                        << ". Continue navigation "<<shift);
        return extractShape(shift->getOp());
    }
    if (inShape->typeID() == GeoShapeSubtraction::getClassTypeID()){
      ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<
                      "Shape is a subtraction. Extract the basic shape. Continue navigation "<<inShape);
      const GeoShapeSubtraction* subtract = static_cast<const GeoShapeSubtraction*>(inShape);
      return extractShape(subtract->getOpA());
    }    
    return inShape;
}   
Amg::Transform3D MuonGeoUtilityTool::extractShifts(const PVConstLink& physVol) const { 
    const GeoLogVol* logVol = physVol->getLogVol();
    if (!logVol) {
      ATH_MSG_ERROR(__FILE__<<":"<<__LINE__<<" Physical volume has no logical volume attached. ");
      return Amg::Transform3D::Identity();
    }
    return extractShifts(logVol->getShape());
}

Amg::Transform3D MuonGeoUtilityTool::extractShifts(const GeoShape* inShape) const { 
  if (!inShape) {
      ATH_MSG_ERROR(__FILE__<<":"<<__LINE__<<" "<<__func__<<" nullptr given ");
      return Amg::Transform3D::Identity();
  }  
  Amg::Transform3D sumTrans{Amg::Transform3D::Identity()};
  if (inShape->typeID() == GeoShapeShift::getClassTypeID()) {
        const GeoShapeShift* shift = dynamic_pointer_cast<const GeoShapeShift>(compressShift(inShape));
        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Shape is a shift . Continue navigation "<<shift);
        sumTrans = shift->getX();
    }
    ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Extacted transformation "<<GeoTrf::toString(sumTrans));
    return sumTrans;
}
std::string MuonGeoUtilityTool::dumpShape(const GeoShape* shape) const { return printGeoShape(shape); }
std::string MuonGeoUtilityTool::dumpVolume(const PVConstLink& physVol) const {
   return dumpVolume(physVol, "");
}
std::string MuonGeoUtilityTool::dumpVolume(const PVConstLink& physVol, const std::string& childDelim) const { 
  std::stringstream sstr{};
  if (!physVol || !physVol->getLogVol()){
    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" "<<__func__<<" No logical volume attached ");
    return sstr.str();        
  }
  const GeoShape* shape = extractShape(physVol);
  if (!shape) {
      ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<" "<<__func__
                      <<" Failed to extract shape from phys volume "
                      << physVol->getLogVol()->getName());
      return sstr.str();
  }
  sstr<<"logical volume "<<physVol->getLogVol()->getName()<<", ";
  if (physVol->isShared() || !physVol->getParent()){
    sstr<<"shared volume, ";
  } else {
    const GeoVPhysVol* pv = physVol;
    if (typeid(*pv) == typeid(GeoFullPhysVol)){
      const Amg::Transform3D absTrans = static_cast<const GeoFullPhysVol&>(*physVol).getAbsoluteTransform();
      sstr<<"absolute pos: "<<GeoTrf::toString(absTrans,true) << ", ";
    } else{
        sstr<<"relative pos: "<<GeoTrf::toString(physVol->getX(), true)<<", ";  
    }
  }
  sstr<<dumpShape(shape)<<", ";
  const Amg::Transform3D shift = extractShifts(physVol);
  if (!Amg::isIdentity(shift)) {
    sstr<<" shape shifted by "<<GeoTrf::toString(shift, true);
  } 
  sstr<<"number of children "<<physVol->getNChildVols()<<", "<<std::endl;
  std::vector<GeoChildNodeWithTrf> children = getChildrenWithRef(physVol, false);
  for (unsigned int child = 0; child < children.size(); ++child) {
    sstr<<childDelim<<(child+1)<<": "<<GeoTrf::toString(children[child].transform, true)
        <<", "<< dumpVolume(children[child].volume, childDelim + "    ");
  }
  return sstr.str();
}

const GeoAlignableTransform* MuonGeoUtilityTool::findAlignableTransform(const PVConstLink& physVol) const {
    PVConstLink parent{physVol->getParent()}, child{physVol};
    while (parent) {
       const GeoGraphNode * const * node1 = parent->findChildNode(child);
       const GeoGraphNode * const * fence =  parent->getChildNode(0);
       for(const GeoGraphNode * const * current = node1 - 1; current>=fence; current--) {
          const GeoGraphNode* node{*current};
          if (dynamic_cast<const GeoVPhysVol*>(node)) break;
          const GeoAlignableTransform* alignTrans{dynamic_cast<const GeoAlignableTransform*>(node)};
          if (alignTrans) return alignTrans;
       }       
       child = parent;
       parent = child->getParent();
    }
    return nullptr;
}

std::vector<MuonGeoUtilityTool::physVolWithTrans> MuonGeoUtilityTool::findAllLeafNodesByName(const PVConstLink& physVol, const std::string& volumeName) const {
  const std::vector<physVolWithTrans> children = getChildrenWithRef(physVol, false);
  std::vector<physVolWithTrans> foundVols{};
  for (const physVolWithTrans& child : children) {    
    /// The logical volume has precisely the name for what we're searching for
    if (child.volume->getLogVol()->getName() == volumeName || child.nodeName == volumeName) {
        foundVols.push_back(child);
    }
    /// There are no grand children of this volume. We're at a leaf node
    if (!child.volume->getNChildVols()) {
      continue;
    }    
    std::vector<physVolWithTrans> grandChildren = findAllLeafNodesByName(child.volume, volumeName);
    std::transform(std::make_move_iterator(grandChildren.begin()),
                   std::make_move_iterator(grandChildren.end()), std::back_inserter(foundVols),
                   [&child](physVolWithTrans&& vol){
                      vol.transform = child.transform * vol.transform;
                      return vol;
                  });
  }
  return foundVols;
}
std::vector<const GeoShape*> MuonGeoUtilityTool::getComponents(const GeoShape* booleanShape) const {
   return getBooleanComponents(booleanShape);
}

std::vector<Amg::Vector2D> MuonGeoUtilityTool::polygonEdges(const GeoSimplePolygonBrep& polygon) const {
  std::vector<Amg::Vector2D> polygonEdges{};
  polygonEdges.reserve(polygon.getNVertices());
  for (unsigned int i = 0; i < polygon.getNVertices(); ++i) {
      polygonEdges.emplace_back(polygon.getXVertex(i), polygon.getYVertex(i));
      ATH_MSG_VERBOSE("Polygon vertext point  " << i << ": "<< GeoTrf::toString(polygonEdges.back(), 2));
    }
  return polygonEdges;
}

std::vector<Amg::Vector3D> MuonGeoUtilityTool::shapeEdges(const GeoShape* shape,
                                                          const Amg::Transform3D& refTrf) const {
    return getPolyShapeEdges(shape, refTrf);
}


}
