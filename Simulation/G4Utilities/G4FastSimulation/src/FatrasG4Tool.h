/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FATRASG4TOOL_H
#define G4FASTSIMULATION_FATRASG4TOOL_H

/* Fast simulation base include */
#include "G4AtlasTools/FastSimulationBase.h"

class G4VFastSimulationModel;

class FatrasG4Tool: public FastSimulationBase
{
 public:

  FatrasG4Tool(const std::string& type, const std::string& name, const IInterface *parent);   //!< Default constructor

protected:
  /** Method to make the actual fast simulation model itself, which
   will be owned by the tool.  Must be implemented in all concrete
   base classes. */
  virtual G4VFastSimulationModel* makeFastSimModel() override final;  

  private:
  // Flag to enable FatrasG4 photon conversion normalizing flow
  Gaudi::Property<bool> m_flowConversion{this, "flowConversion", false, "Flag to enable normalizing flow conversion."};
  // Calibration-area-relative path of the normalizing flow photon conversion ONNX model
  Gaudi::Property<std::string> m_flowConversionModelPath{this, "flowConversionModelPath", "", "Calibration-area-relative path of the normalizing flow photon conversion ONNX model."};

};

#endif //G4FASTSIMULATION_FATRASG4TOOL_H
