/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Surface.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/SurfaceBounds.h"

// Gaudi
#include "GaudiKernel/MsgStream.h"

//CxxUtils
#include "CxxUtils/inline_hints.h"
// STD
#include <iomanip>
#include <iostream>
#include <utility>

Trk::Surface::Surface() = default;

ATH_FLATTEN
Trk::Surface::Surface(const Amg::Transform3D& tform)
  : Trk::ObjectCounter<Trk::Surface>()
  , m_transforms(std::make_unique<Transforms>(tform))
{
}

Trk::Surface::Surface(const Trk::TrkDetElementBase& detelement)
  : Trk::ObjectCounter<Trk::Surface>()
  , m_associatedDetElement(&detelement)
  , m_owner(Trk::DetElOwn)
{
}

Trk::Surface::Surface(const Trk::TrkDetElementBase& detelement,
                      const Identifier& id)
  : Trk::ObjectCounter<Trk::Surface>()
  , m_associatedDetElement(&detelement)
  , m_associatedDetElementId(id)
  , m_owner(Trk::DetElOwn)
{
}

// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
ATH_FLATTEN
// copy constructor - Attention! sets the associatedDetElement to 0 and the
// identifier to invalid
Trk::Surface::Surface(const Surface& sf)
  : Trk::ObjectCounter<Trk::Surface>(sf)
  , m_transforms(std::make_unique<Transforms>(sf.transform()))
  , m_associatedLayer(sf.m_associatedLayer)
  , m_materialLayer(sf.m_materialLayer)
  , m_owner(Trk::noOwn)
{
}

// We compile this function with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
ATH_FLATTEN
// copy constructor with shift - Attention! sets the associatedDetElement to 0
// and the identifier to invalid also invalidates the material layer
Trk::Surface::Surface(const Surface& sf, const Amg::Transform3D& shift)
  : Trk::ObjectCounter<Trk::Surface>(sf)
  , m_owner(Trk::noOwn)
{
  if (sf.m_transforms) {
    m_transforms = std::make_unique<Transforms>(
      shift * sf.m_transforms->transform, shift * sf.m_transforms->center);
  } else {
    m_transforms = std::make_unique<Transforms>(Amg::Transform3D(shift));
  }
}

// destructor
Trk::Surface::~Surface() = default;

// assignment operator
// the assigned surfaces loses its link to the detector element
Trk::Surface&
Trk::Surface::operator=(const Trk::Surface& sf)
{
  if (this != &sf) {
    m_transforms = std::make_unique<Transforms>(sf.transform());
    m_associatedDetElement = nullptr;
    m_associatedDetElementId = Identifier();
    m_associatedLayer = sf.m_associatedLayer;
    m_materialLayer = sf.m_materialLayer;
    m_owner = Trk::noOwn;
  }
  return *this;
}

// returns the LocalPosition on a surface of a GlobalPosition
std::optional<Amg::Vector2D>
Trk::Surface::positionOnSurface(const Amg::Vector3D& glopo,
                                const BoundaryCheck& bchk,
                                double tol1,
                                double tol2) const
{
  std::optional<Amg::Vector2D> posOnSurface = globalToLocal(glopo, tol1);
  if (!bchk){
    return posOnSurface;
  }
  if (posOnSurface && insideBounds(*posOnSurface, tol1, tol2)){
    return posOnSurface;
  }
  return std::nullopt;
}

// checks if GlobalPosition is on Surface and inside bounds
bool
Trk::Surface::isOnSurface(const Amg::Vector3D& glopo,
                          const BoundaryCheck& bchk,
                          double tol1,
                          double tol2) const
{
  std::optional<Amg::Vector2D> posOnSurface =
    positionOnSurface(glopo, bchk, tol1, tol2);
  return static_cast<bool>(posOnSurface);
}

// return the measurement frame
Amg::RotationMatrix3D
Trk::Surface::measurementFrame(const Amg::Vector3D&, const Amg::Vector3D&) const
{
  return transform().rotation();
}


// overload dump for MsgStream operator
MsgStream&
Trk::Surface::dump(MsgStream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(4);
  sl << name() << std::endl;
  if (associatedDetectorElement()!=nullptr){
    sl<<"     Detector Type              = "<<associatedDetectorElement()->detectorTypeString()<<std::endl;
  }
  sl << "     Center position  (x, y, z) = (" << center().x() << ", " << center().y() << ", " << center().z() << ")"
     << std::endl;
  Amg::RotationMatrix3D rot(transform().rotation());
  Amg::Vector3D rotX(rot.col(0));
  Amg::Vector3D rotY(rot.col(1));
  Amg::Vector3D rotZ(rot.col(2));
  sl << std::setprecision(6);
  sl << "     Rotation:             colX = (" << rotX(0) << ", " << rotX(1) << ", " << rotX(2) << ")" << std::endl;
  sl << "                           colY = (" << rotY(0) << ", " << rotY(1) << ", " << rotY(2) << ")" << std::endl;
  sl << "                           colZ = (" << rotZ(0) << ", " << rotZ(1) << ", " << rotZ(2) << ")" << std::endl;
  sl << "     Bounds  : " << bounds();
  sl << std::setprecision(-1);
  return sl;
}

// overload dump for MsgStream operator
std::ostream&
Trk::Surface::dump(std::ostream& sl) const
{
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(4);
  sl << name() << std::endl;
  if (associatedDetectorElement()!=nullptr){
    sl<<"     Detector Type              = "<<associatedDetectorElement()->detectorTypeString()<<std::endl;
  }
  sl << "     Center position  (x, y, z) = (" << center().x() << ", " << center().y() << ", " << center().z() << ")"
     << std::endl;
  Amg::RotationMatrix3D rot(transform().rotation());
  Amg::Vector3D rotX(rot.col(0));
  Amg::Vector3D rotY(rot.col(1));
  Amg::Vector3D rotZ(rot.col(2));
  sl << std::setprecision(6);
  sl << "     Rotation:             colX = (" << rotX(0) << ", " << rotX(1) << ", " << rotX(2) << ")" << std::endl;
  sl << "                           colY = (" << rotY(0) << ", " << rotY(1) << ", " << rotY(2) << ")" << std::endl;
  sl << "                           colZ = (" << rotZ(0) << ", " << rotZ(1) << ", " << rotZ(2) << ")" << std::endl;
  sl << "     Bounds  : " << bounds();
  sl << std::setprecision(-1);
  return sl;
}

/**Overload of << operator for both, MsgStream and std::ostream for debug output*/
MsgStream&
Trk::operator<<(MsgStream& sl, const Trk::Surface& sf)
{
  return sf.dump(sl);
}

std::ostream&
Trk::operator<<(std::ostream& sl, const Trk::Surface& sf)
{
  return sf.dump(sl);
}
