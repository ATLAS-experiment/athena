/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRTPhysicsTool_H
#define TRTPhysicsTool_H

// Include files

#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPhysicsOptionTool.h"

//to handle
#include "G4AtlasInterfaces/IPhysicsConstructor.h"
#include "GaudiKernel/ToolHandle.h"

/** @class TRTPhysicsTool TRTPhysicsTool.h "TRT:TR_Process/TRTPhysicsTool.h"
 *
 *
 *
 *  @author Edoardo Farina
 *  @date  18-05-2015
 */
class TRTPhysicsTool final : public extends<AthAlgTool, IPhysicsOptionTool> {
 public:
  /// Standard constructor
  TRTPhysicsTool( const std::string& type , const std::string& name,
                  const IInterface* parent ) ;

  virtual ~TRTPhysicsTool(){}; ///< Destructor

  /// Initialize method
  virtual StatusCode initialize() override;

  /// IPhysicsOptionTool method; simply returns self.
  virtual UPPhysicsConstructor GetPhysicsOption() override;

  class PhysicsConstructor : public IPhysicsContructor{
   public:
    PhysicsConstructor(const std::string& name, MSG::Level level,
                       const std::string& xml_file)
        : IPhysicsContructor(name, level), m_xmlFile(xml_file) {}

    virtual void ConstructParticle() override;
    virtual void ConstructProcess() override;

   private:
    std::string m_xmlFile;
   };

 private:

  std::string m_xmlFile;
};

#endif
