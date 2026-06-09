/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <BCMPrimeReadoutGeometry/BCMPrimeDiamondDesign.h>
#include "TrkSurfaces/RectangleBounds.h"


namespace InDetDD
{

BCMPrimeDiamondDesign::BCMPrimeDiamondDesign(double sizeX, double sizeY, double thickness)
  : SiDetectorDesign(thickness,           // thickness in mm
                     true,                // phi symmetric
                     true,                // eta symmetric
                     true,                // depth symmetric
                     InDetDD::electrons, // carrier type (use electrons by default)
                     +1),                 // readout side
    m_sizeX(sizeX),
    m_sizeY(sizeY),
    m_thickness(thickness)
{
}

const Trk::SurfaceBounds& BCMPrimeDiamondDesign::bounds() const
{
    static const Trk::RectangleBounds rectBounds(0.5 * m_sizeX, 0.5 * m_sizeY);
    return rectBounds;
}

} // namespace InDetDD
