/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Jan-2008: (M.Fincke)  To be used for new Module-Geometry

#ifndef LArG4_HEC_LArHECCalibrationWheelCalculator_H
#define LArG4_HEC_LArHECCalibrationWheelCalculator_H

#include "LArG4HEC/IHECGeometry.h"
#include "LArG4Code/LArCalibCalculatorSvcImp.h"
#include "LArG4Code/LArG4Identifier.h"
#include "CaloG4Sim/SimulationEnergies.h"

#include "globals.hh"

#include <vector>

// Forward declaractions:
class G4Step;

namespace LArG4 {

  namespace HEC {

    class LArHECCalibrationWheelCalculator : public LArCalibCalculatorSvcImp {
    public:

      LArHECCalibrationWheelCalculator(const std::string& name, ISvcLocator *pSvcLocator);
      void GeometryTypeUpdateHandler(Gaudi::Details::PropertyBase&);
      StatusCode initialize() override final;
      virtual ~LArHECCalibrationWheelCalculator();

      virtual G4bool Process (const G4Step* step, LArG4Identifier & identifier,
                              std::vector<G4double> & energies,
                              const eCalculatorProcessing process = kEnergyAndID) const override final;
    private:

      ServiceHandle<IHECGeometry> m_geometryCalculator{this, "GeometryCalculator", "HECGeometry"}; //FIXME LArG4::HEC::HECGeometry
      UnsignedShortProperty m_geometryTypeProp{this, "GeometryType", 0, &LArHECCalibrationWheelCalculator::GeometryTypeUpdateHandler};
      eHECGeometryType m_geometryType;

      CaloG4::SimulationEnergies m_energyCalculator;

    };

  } // namespace HEC

} // namespace LArG4

#endif // LArG4_HEC_LArHECCalibrationWheelCalculator_H
