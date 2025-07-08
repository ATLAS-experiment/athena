///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// PyAthenaAud.h 
// Header file for class PyAthena::Aud
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENAPYTHON_PYATHENAAUD_H 
#define ATHENAPYTHON_PYATHENAAUD_H 

// STL includes
#include <string>

// FrameWork includes
#include "Gaudi/Auditor.h"
#include "AthenaPython/IPyComponent.h"
#include "CxxUtils/checker_macros.h"

// Forward declaration
class INamedInterface;
// Python
struct _object;
typedef _object PyObject;

namespace PyAthena {

class ATLAS_NOT_THREAD_SAFE Aud : virtual public ::IPyComponent,
                                  public Gaudi::Auditor
{ 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  Aud( const std::string& name, ISvcLocator* svcLocator );

  /// Destructor: 
  virtual ~Aud(); 

  /// Gaudi Aud Implementation
  //@{
  virtual StatusCode initialize() override;
  virtual StatusCode sysInitialize() override;
  virtual StatusCode finalize() override;
  //@}


  /** return the @c std::type_info name of the underlying py-component
   *  This is used by concrete implementations to connect a python
   *  component to its C++ counter-part
   */
  virtual const char* typeName() const override;

  /// @c Auditor interface
  //@{
  virtual void before(const std::string& evt, const std::string& name,
                      const EventContext& ctx) override;

  virtual void after(const std::string& evt, const std::string& name,
                     const EventContext& ctx, const StatusCode& sc) override;
  //@}

  /** @brief return associated python object. BORROWED reference.
   */ 
  virtual PyObject* self() override { return m_self; }

 protected: 

  /** attach the C++ component to its python cousin
   */
  virtual bool setPyAttr( PyObject* pyobj ) override;

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Default constructor: 
  Aud();

  /////////////////////////////////////////////////////////////////// 
  // Protected data: 
  /////////////////////////////////////////////////////////////////// 
 protected: 

  /// Pointer to self (from the python world)
  PyObject* m_self;

}; 

// I/O operators
//////////////////////

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

} //> end namespace PyAthena

#endif //> ATHENAPYTHON_PYATHENAAUD_H
