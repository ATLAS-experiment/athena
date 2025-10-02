/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#ifndef INDETDD_PIXELDIODEPARAMETERSPROXY_H
#define INDETDD_PIXELDIODEPARAMETERSPROXY_H

#include "ReadoutGeometryBase/PixelDiodeMatrix.h"

namespace InDetDD {
/// Helper class to cache a pixel diode position, and provide access to diode parameters
class PixelDiodeParametersProxy
{
public:
   PixelDiodeParametersProxy() = default;
   PixelDiodeParametersProxy(const PixelDiodeMatrix *cell) : m_cell(cell) {}

   /// return true if the Proxy is valid
   bool isValid() const { return m_cell != nullptr; }

   operator bool() const { return isValid(); }

   /// return the center position of the diode if the proxy is valid.
   const Amg::Vector2D &position() const { return m_position; };
   /// return the diode width in phi (aka local-x, row) and eta (aka local-y, column) direction
   Amg::Vector2D width() const { return Amg::Vector2D{phiWidth(), etaWidth()}; }
   /// return the diode width in phi (aka local-x, row)
   double phiWidth() const { return m_cell->phiWidth(); }
   /// return the diode width in eta (aka local-y, column) direction
   double etaWidth() const { return m_cell->etaWidth(); }

   /// return the diode position of the lower diode edge in phi/local-x/row direction.
   double xPhiMin() const { return m_position[0]-phiWidth()/2; }
   /// return the diode position of the upper diode edge in phi/local-x/row direction.
   double xPhiMax() const { return m_position[0]+phiWidth()/2; }
   /// return the diode position of the lower diode edge in eta/local-y/column direction.
   double xEtaMin() const { return m_position[1]-etaWidth()/2; }
   /// return the diode position of the upper diode edge in eta/local-y/column direction.
   double xEtaMax() const { return m_position[1]+etaWidth()/2; }

   /// provide access to internal storage for initializing the proxy.
   Amg::Vector2D &position() { return m_position; };
   using PixelDiodeMatrixPtr = const PixelDiodeMatrix *;
   /// provide access to internal storage for initializing the proxy.
   PixelDiodeMatrixPtr &cell_ptr() { return m_cell; }
private:
   const PixelDiodeMatrix *m_cell = nullptr;
   Amg::Vector2D m_position;
};
}
#endif
