/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4HEC_ILOCALGEOMETRY_H
#define LARG4HEC_ILOCALGEOMETRY_H

#include "GaudiKernel/IService.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include <string>

class G4Step;
class LArG4Identifier;

namespace LArG4
{
  namespace HEC
  {

    enum eLocalGeometryType { kLocActive, kLocInactive, kLocDead };

    class ILocalGeometry : virtual public IService
    {
    public:
      ILocalGeometry() {}
      virtual ~ILocalGeometry() {}

      DeclareInterfaceID(ILocalGeometry,1,0);

      virtual LArG4Identifier CalculateIdentifier( const G4Step* a_step, const eLocalGeometryType type = kLocActive,
                                                   int depthadd = 0, double deadzone = 4.*CLHEP::mm, double locyadd = 0.*CLHEP::mm) const = 0;

    };
  }
}
#endif //LARG4HEC_ILOCALGEOMETRY_H
