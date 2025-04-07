/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LucidPhysicsTool_H
#define LucidPhysicsTool_H

// Include files

#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPhysicsOptionTool.h"

//to handle
#include "G4AtlasInterfaces/IPhysicsConstructor.h"
#include "GaudiKernel/ToolHandle.h"

/** @class LucidPhysicsTool LucidPhysicsTool.h "G4AtlasInfrstructure/LucidPhysicsTool.h"
 *
 *  Tool for the concrete implementation of a Physics Tool class
 *
 *  @author Edoardo Farina
 *  @date   18-05-2015
 */
class LucidPhysicsTool : public AthAlgTool, virtual public IPhysicsOptionTool {
 public:
  /// Standard constructor
  LucidPhysicsTool( const std::string& type , const std::string& name,
                    const IInterface* parent ) ;

  virtual ~LucidPhysicsTool( ); ///< Destructor

  /// Initialize method
  virtual StatusCode initialize() override final;

  /** Implements
   */
  UPPhysicsConstructor GetPhysicsOption() override;

  class PhysicsConstructor : public IPhysicsContructor {
   public:
    using IPhysicsContructor::IPhysicsContructor;

    virtual void ConstructParticle() override;
    virtual void ConstructProcess() override;
  };
};

#endif
