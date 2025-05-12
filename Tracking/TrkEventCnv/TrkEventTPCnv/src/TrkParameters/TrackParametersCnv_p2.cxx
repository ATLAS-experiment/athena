/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
//
// file:   TrackParametersCnv_p2.h
// author: edward.moyse@cern.ch
//
//-----------------------------------------------------------------------------

#include "TrkParameters/TrackParameters.h"
#include "TrkParametersBase/ParametersT.h"
#include "TrkParametersBase/CurvilinearParametersT.h"
#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "TrkSurfaces/ConeSurface.h"

#include "TrkEventTPCnv/TrkParameters/TrackParametersCnv_p2.h"
#include "TrkEventTPCnv/helpers/EigenHelpers.h"
#include "TrkEventTPCnv/helpers/CLHEPHelpers.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"

#include <cassert>
#include <iostream>

void TrackParametersCnv_p2::persToTrans(
    const Trk ::TrackParameters_p2* /**persObj*/,
    Trk ::TrackParameters* /**transObj*/, MsgStream& /**log*/) {
  throw std::runtime_error(
      "TrackParametersCnv_p2::persToTrans shouldn't be called any more!");
}

Trk::TrackParameters* TrackParametersCnv_p2::createTransient(
    const Trk::TrackParameters_p2* persObj, MsgStream& log) {
  // ---- Covariance matrix
  std::optional<AmgSymMatrix(5)> cov = std::nullopt;
  auto transcov =
      std::unique_ptr<AmgSymMatrix(5)>(transErrorMatrix(persObj, log));
  if (transcov) {
    cov = (*transcov);
  }
  // ---- Parameters
  Trk::TrackParameters    *transObj=nullptr;
  unsigned int size=persObj->m_parameters.size();
  if (size==7){ // size 7 means we chose to write
                // as a Curvillinear representation
                // In principle we could check
                // the persObject for Trk::SurfaceType::Curvilinear
    AmgVector(7) parameters;
    for (unsigned int i=0; i<size; ++i) parameters[i]=persObj->m_parameters[i];
    transObj= new Trk::CurvilinearParameters(parameters, cov);
    return transObj;
  } else {
    // Not a curvilinear representation.
    // We need to have a surface to handle local->global transformations etc/
    // Get surface type
    Trk::SurfaceType type = static_cast<Trk::SurfaceType>(persObj->m_surfaceType);
    // Get surface & fill parameter vector
    const Trk::Surface* surface = transSurface(persObj, type, log);
    AmgVector(5) parameters;
    for (unsigned int i = 0; i < size; ++i){
      parameters[i] = persObj->m_parameters[i];
    }
    // Now create concrete parameters ...
    if (surface){
      if (type == Trk::SurfaceType::Perigee) {
        transObj = new Trk::Perigee(
          parameters, static_cast<const Trk::PerigeeSurface*>(surface), cov);
        return transObj;
      } else if (type == Trk::SurfaceType::Plane) {
        transObj = new Trk::AtaPlane(
          parameters, static_cast<const Trk::PlaneSurface*>(surface), cov);
        return transObj;
      } else if (type == Trk::SurfaceType::Line) {
        transObj = new Trk::AtaStraightLine(
          parameters,
          static_cast<const Trk::StraightLineSurface*>(surface),
          cov);
        return transObj;
      }
    } else if (!m_nosurf) {
      // FIXME: next line changed to DEBUG to avoid filling the derivation job
      // options with garbage. Underlying issue should be fixed.
      log << MSG::DEBUG << "No surface of type=" << static_cast<int>(type)
          << " created - so these parameters cannot be made!" << endmsg;
      return nullptr;
    }
  }
  return nullptr;
}

AmgSymMatrix(5)* TrackParametersCnv_p2::transErrorMatrix(const Trk :: TrackParameters_p2 *persObj,MsgStream& log){
  AmgSymMatrix(5)* cov=nullptr;
  if (!persObj->m_errorMatrix.isNull()){
    // fill errormatrix
    cov = new AmgSymMatrix(5);
    Trk::ErrorMatrix dummy;
    fillTransFromPStore( &m_emConverter, persObj->m_errorMatrix, &dummy, log );
    Amg::expand(dummy.values.begin(), dummy.values.end(), *cov);
  }
  return cov;
}

const Trk::Surface*
TrackParametersCnv_p2::transSurface(const Trk ::TrackParameters_p2* persObj,
                                    Trk::SurfaceType type,
                                    MsgStream& log)
{
  const Trk::Surface* surface = nullptr;
  // check if surface had transform.
  if (!persObj->m_transform.empty()){
    auto transform = std::make_unique<Amg::Transform3D>();
    EigenHelpers::vectorToEigenTransform3D( persObj->m_transform, *transform.get());
    // recreate free surface
    if (type==Trk::SurfaceType::Perigee) {
      surface = new Trk::PerigeeSurface(*transform);
    } else if (type==Trk::SurfaceType::Plane){
      surface = new Trk::PlaneSurface(*transform);
    } else if (type==Trk::SurfaceType::Line){
      surface = new Trk::StraightLineSurface(*transform);
    }
    if (!surface){
      log << MSG::WARNING << "Free surface of type=" << static_cast<int>(type)
          << " isn't currently supported in TrackParametersCnv_p2" << endmsg;
      return nullptr;
    }
  } else {
    // Surface must have belonged to a ReadoutElement, or some part of the geometry or have a nominal/default perigee surface.
    if (type!=Trk::SurfaceType::Perigee) {
      Identifier id=Identifier32(persObj->m_associatedDetElementId);
      if (!id.get_compact() && persObj->m_associatedDetElementId != 0)
        id = Identifier(persObj->m_associatedDetElementId);
      if (m_nosurf)surface = nullptr;
      else {
        const Trk::Surface* detSurf = m_eventCnvTool->getSurface(id);
        if (!detSurf){
          log << MSG::WARNING << "Surface of type=" << static_cast<int>(type)
              << " was not found by the eventCnvTool." << endmsg;
        }
        surface = detSurf;
      }
    } else {
      surface = new Trk::PerigeeSurface(); // FIXME! How do we support nominal Perigee Surfaces now?
    }
  }
  return surface;
}

void TrackParametersCnv_p2::transToPers(const Trk ::TrackParameters* transObj,
                                        Trk ::TrackParameters_p2* persObj,
                                        MsgStream& log) {

  bool isCurvilinear = (transObj->type() == Trk::Curvilinear);
  bool deleteAtEnd = false;
  if (isCurvilinear){
    convertTransCurvilinearToPers(transObj,persObj);
    // Not bothering with Surface here - not relevant for curvilinear
  } else {
    // normal track parameters - check if the type is 'permitted' to be written.
    if (isPersistifiableType(transObj)) {
      unsigned int nRows = transObj->parameters().rows();
      persObj->m_parameters.resize( nRows);
      for( unsigned int i = 0; i < nRows; i++ ){
        persObj->m_parameters[i]   = transObj->parameters()[i];
      }
      fillPersSurface(transObj, persObj, log);
    } else {  // if not, convert to curvilinear
      std::optional<AmgSymMatrix(5)> newcov = std::nullopt;
      if (transObj->covariance()) {
        newcov = *(transObj->covariance());
      }
      const Trk::CurvilinearParameters* curvilinear =
          new Trk::CurvilinearParameters(transObj->position(),
                                         transObj->momentum(),
                                         transObj->charge(), newcov);
      transObj = curvilinear;
      deleteAtEnd = true;  // Because the curvilinear will leak otherwise
      convertTransCurvilinearToPers(transObj, persObj);
    }
  }

  // Errormatrix
  if (transObj->covariance()){
    Trk::ErrorMatrix pMat;
    EigenHelpers::eigenMatrixToVector(pMat.values, *transObj->covariance(), "TrackParametersCnv_p2");
    persObj->m_errorMatrix = toPersistent( &m_emConverter, &pMat, log );
  }

  if (deleteAtEnd) {
    delete transObj;
  }
}

void TrackParametersCnv_p2::convertTransCurvilinearToPers(
    const Trk ::TrackParameters* transObj, Trk ::TrackParameters_p2* persObj) {
  // Curvilinear: here we store the 3 position + 3 momentum, rather than the 5
  // parameters.
  // This avoids writing the surface
  persObj->m_parameters.resize(7);
  for (unsigned int i = 0; i < 3; ++i) {
    persObj->m_parameters[i] = transObj->position()[i];
    persObj->m_parameters[i + 3] = transObj->momentum()[i];
  }
  persObj->m_parameters[6] = transObj->charge();
  // And lets remember to always set the type of the surface
  persObj->m_surfaceType = static_cast<uint8_t>(Trk::SurfaceType::Curvilinear);
}

bool TrackParametersCnv_p2::isPersistifiableType(const Trk :: TrackParameters    *transObj) {
  const Trk::Surface* surf = transObj->associatedSurface ().baseSurface();
  assert (surf);
  Trk::SurfaceType type = surf->type();
  return type==Trk::SurfaceType::Perigee || type==Trk::SurfaceType::Plane || type==Trk::SurfaceType::Line;
}

void TrackParametersCnv_p2::fillPersSurface(const Trk :: TrackParameters    *transObj, Trk :: TrackParameters_p2 *persObj, MsgStream& /*log*/){
  //----- Surface
  const Trk::Surface* surf = transObj->associatedSurface ().baseSurface();
  assert (surf);
  persObj->m_surfaceType = static_cast<uint8_t>(surf->type()); // Store type

  persObj->m_associatedDetElementId = surf->associatedDetectorElementIdentifier().get_identifier32().get_compact();
  static const Trk::PerigeeSurface s_nominalPerigeeSurface; // FIXME - should there be a  common 'nominal' surface ie on Perigee, as before?
  // Need to write out transforms for TG owned surfaces, and 'free' (noOwn) surfaces - i.e. anything which isn't on det element
  if( surf->cachedTransform()!=nullptr ) {
    // FIXME - I think maybe we can just remove all of the code below and ALWAYS write out the transform if it exists - i.e. it won't exist if the surface is 'nominal'
    if (surf->type() != Trk::SurfaceType::Perigee ||
        (surf->type() == Trk::SurfaceType::Perigee &&
         *surf != s_nominalPerigeeSurface)) {
      EigenHelpers::eigenTransform3DToVector(*(surf->cachedTransform()),
                                             persObj->m_transform);
    }
  }
}


