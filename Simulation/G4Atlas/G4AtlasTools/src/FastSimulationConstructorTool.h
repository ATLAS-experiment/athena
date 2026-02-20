/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FastSimulationConstructorTool_H
#define FastSimulationConstructorTool_H

// Include files

#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPhysicsOptionTool.h"
#include "G4AtlasInterfaces/IPhysicsConstructor.h"

/** @class FastSimulationConstructorTool FastSimulationConstructorTool.h "G4AtlasTools/FastSimulationConstructorTool.h"
 *
 *  This tool creates a physics constructor to enable fast simulation for all particles.
 *
 *  @author Julien Esseiva
 *  @date   20-10-2025
 */
class FastSimulationConstructorTool final : public extends<AthAlgTool, IPhysicsOptionTool> {
 public:
  /// Standard constructor
  FastSimulationConstructorTool( const std::string& type , const std::string& name,
                       const IInterface* parent ) ;

  virtual ~FastSimulationConstructorTool() = default; ///< Destructor

  /// Initialize method
  virtual StatusCode initialize() override;

  UPPhysicsConstructor GetPhysicsOption() override;

  class PhysicsConstructor : public IPhysicsContructor {
   public:
    PhysicsConstructor(bool initializeFastSimulation, const std::string& name, MSG::Level level);

    virtual void ConstructParticle() override;
    virtual void ConstructProcess() override;
   private:
    bool m_initializeFastSimulation{true};
   };

 private:
  Gaudi::Property<bool> m_initializeFastSimulation{this, "InitializeFastSimulation", true, "Fast simulation initialization flag"};
};

#endif //FastSimulationConstructorTool_H
