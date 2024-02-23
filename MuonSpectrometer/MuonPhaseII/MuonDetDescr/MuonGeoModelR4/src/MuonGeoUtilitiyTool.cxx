/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonGeoModelR4/MuonGeoUtilityTool.h>

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <GeoModelKernel/GeoBox.h>
#include <GeoModelKernel/GeoTrd.h>
#include <GeoModelKernel/GeoSimplePolygonBrep.h>
#include <GeoModelKernel/GeoShapeShift.h>
#include <GeoModelKernel/GeoTube.h>
#include <GeoModelKernel/GeoShapeUnion.h>
#include <GeoModelKernel/GeoTransform.h>
#include <GeoModelKernel/GeoShapeSubtraction.h>
#include <GeoModelKernel/GeoSerialTransformer.h>
#include <GeoModelKernel/GeoVolumeCursor.h>
#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <set>
#include <sstream>
#include <string>

using namespace ActsTrk;
namespace MuonGMR4{
using geoShapeWithShift = IMuonGeoUtilityTool::geoShapeWithShift;

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
        const GeoShapeShift* shift = static_cast<const GeoShapeShift*>(inShape);
        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<
                        "Shape is a shift by "<<Amg::toString(shift->getX())
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
        const GeoShapeShift* shift = static_cast<const GeoShapeShift*>(inShape);
        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Shape is a shift . Continue navigation "<<shift);
        sumTrans = extractShifts(shift->getOp()) * shift->getX();
    }
    ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Extacted transformation "<<Amg::toString(sumTrans));
    return sumTrans;
}
std::string MuonGeoUtilityTool::dumpShape(const GeoShape* shape) const {
  std::stringstream sstr{};
  if (shape->typeID() == GeoTrd::getClassTypeID()){
      const GeoTrd* trd = static_cast<const GeoTrd*>(shape);
      sstr<<"Trapezoidal geometry shape with width: "<<trd->getXHalfLength1()
          <<", height: "<<trd->getXHalfLength2()<<" -- short/long length: "
          <<trd->getYHalfLength1()<<"/" <<trd->getYHalfLength2()<<", chamber width: "<<trd->getZHalfLength();
  } else if (shape->typeID() == GeoBox::getClassTypeID()){
      const GeoBox* box = static_cast<const GeoBox*>(shape);
      const Amg::Vector3D span{box->getXHalfLength(),
                               box->getYHalfLength(),
                               box->getZHalfLength()};
      sstr<<"Box geometry shape: Half lengths "<<Amg::toString(span,1);
  } else if (shape->typeID() == GeoTube::getClassTypeID()){
    const GeoTube* tube = static_cast<const GeoTube*>(shape);
    sstr<<"Tube with minimal and maximal radii of "<<tube->getRMin()<<", "<<tube->getRMax()<<" and length "<<tube->getZHalfLength();
  } else if (shape->typeID() == GeoShapeUnion::getClassTypeID()){
      const GeoShapeUnion* unionShape = static_cast<const GeoShapeUnion*>(shape);
      std::vector<geoShapeWithShift> constiuents = getComponents(unionShape);
      sstr<<"Union of  <<<<";
      for (const geoShapeWithShift& childShape: constiuents) {
          sstr<<dumpShape(childShape.shape);
          if (!Amg::isIdentity(childShape.transform)) {
             sstr<<" - shifted by "<<Amg::toString(childShape.transform);
          }
          sstr<<", ";
      }
      sstr<<" >>>>";
  } else {
      sstr<<"Cake tastes good "<<shape->type();
  }
  return sstr.str();
}
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
    const GeoVPhysVol* pv = &*physVol; // avoid clang warning
    if (typeid(*pv) == typeid(GeoFullPhysVol)){
      const Amg::Transform3D absTrans = static_cast<const GeoFullPhysVol&>(*physVol).getAbsoluteTransform();
      sstr<<"absolute pos: "<<Amg::toString(absTrans) << ", ";
    } else{
        sstr<<"relative pos: "<<Amg::toString(physVol->getX())<<", ";  
    }
  }
  sstr<<dumpShape(shape)<<", ";
  const Amg::Transform3D shift = extractShifts(physVol);
  if (!Amg::isIdentity(shift)) {
    sstr<<" shape shifted by "<<Amg::toString(shift);
  } 
  sstr<<"number of children "<<physVol->getNChildVols()<<", ";
  sstr<<std::endl;
  GeoVolumeCursor aV(physVol);
  unsigned int child{1};
  while (!aV.atEnd()) {
    sstr<<childDelim<<child<<": "<<Amg::toString(aV.getTransform())<<", "<< dumpVolume(aV.getVolume(),
                                                                                childDelim + "    ");
    ++child;
    aV.next();
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
  std::vector<physVolWithTrans> foundVols{};
  GeoVolumeCursor aV(physVol);
   while (!aV.atEnd()) {    
    PVConstLink childVol = aV.getVolume();
    const Amg::Transform3D childTrans{aV.getTransform()};
    /// The logical volume has precisely the name for what we're searching for
    
    if (childVol->getLogVol()->getName() == volumeName ||
        aV.getName() == volumeName) {
        physVolWithTrans foundNode{};
        foundNode.physVol = childVol;
        foundNode.transform = childTrans;
        foundVols.push_back(std::move(foundNode));
    }
    /// There are no grand children of this volume. We're at a leaf node
    if (!childVol->getNChildVols()) {
      aV.next();
      continue;
    }
    
    std::vector<physVolWithTrans> grandChildren = findAllLeafNodesByName(childVol, volumeName);
    std::transform(std::make_move_iterator(grandChildren.begin()),
                   std::make_move_iterator(grandChildren.end()), std::back_inserter(foundVols),[&childTrans](physVolWithTrans&& vol){
                      vol.transform = childTrans * vol.transform;
                      return vol;
                  });
    
    aV.next();
  }
  return foundVols;
}
std::vector<geoShapeWithShift> MuonGeoUtilityTool::getComponents(const GeoShapeUnion* unionShape) const {
   std::vector<geoShapeWithShift> shapes{};
   
   auto fill_shape = [&shapes, this](const GeoShape* shape) {
        if (shape->typeID() == GeoShapeUnion::getClassTypeID()) {
           std::vector<geoShapeWithShift> childShapes = getComponents(static_cast<const GeoShapeUnion*>(shape));
           shapes.insert(shapes.end(), std::make_move_iterator(childShapes.begin()),
                                       std::make_move_iterator(childShapes.end()));
        } else {
          geoShapeWithShift compound{};
          compound.shape = extractShape(shape);
          compound.transform = extractShifts(shape);
          shapes.push_back(std::move(compound));
        }
   };
   fill_shape(unionShape->getOpA());
   fill_shape(unionShape->getOpB());
 
   return shapes;
}

std::vector<Amg::Vector2D> MuonGeoUtilityTool::polygonEdges(const GeoSimplePolygonBrep& polygon) const {
  std::vector<Amg::Vector2D> polygonEdges{};
  polygonEdges.reserve(polygon.getNVertices());
  for (unsigned int i = 0; i < polygon.getNVertices(); ++i) {
      polygonEdges.emplace_back(polygon.getXVertex(i), polygon.getYVertex(i));
      ATH_MSG_VERBOSE("Polygon vertext point  " << i << ": "<< Amg::toString(polygonEdges.back(),2));
    }
  return polygonEdges;
}

}
