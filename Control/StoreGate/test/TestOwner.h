// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file StoreGate/test/TestOwner.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2019
 * @brief Helper for testing auto-declaring constructors.
 *
 * This is a class that can be used as the owner of
 * an auto-declaring constructor for the purpose of unit tests.
 */


#ifndef STOREGATE_TESTOWNER_H
#define STOREGATE_TESTOWNER_H


#include "StoreGate/VarHandleKeyProperty.h"
#include "StoreGate/VarHandleKeyArrayProperty.h"
#include "GaudiKernel/PropertyHolder.h"
#include "GaudiKernel/IProperty.h"


class TestOwnerBase
  : virtual public extend_interfaces<IProperty, INamedInterface, IDataHandleHolder>
{
public:
  TestOwnerBase& operator= (TestOwnerBase&&) = delete;

  virtual const std::string& name() const override { return m_name; }

  virtual std::vector<Gaudi::DataHandle*> inputHandles() const  override { std::abort(); }
  virtual std::vector<Gaudi::DataHandle*> outputHandles() const override { std::abort(); }

  virtual const DataObjIDColl& extraInputDeps() const  override { std::abort(); }
  virtual const DataObjIDColl& extraOutputDeps() const override { std::abort(); }

  virtual void acceptDHVisitor( IDataHandleVisitor* ) const override { std::abort(); }

  virtual const DataObjIDColl& inputDataObjs() const  override { std::abort(); }
  virtual const DataObjIDColl& outputDataObjs() const override { std::abort(); }

  virtual void addDependency( const DataObjID& dep, const Gaudi::DataHandle::Mode& mode) override
  {
    m_deps.push_back (dep);
    m_modes.push_back (mode);
  } 
  virtual void declare( Gaudi::DataHandle& )  override { std::abort(); }
  virtual void renounce( Gaudi::DataHandle& ) override { std::abort(); }
  virtual bool renounceInput(const DataObjID&) override { std::abort(); }


  std::string m_name = "TestOwner";
  std::vector<DataObjID> m_deps;
  std::vector<Gaudi::DataHandle::Mode> m_modes;
};

class TestOwner
  : public PropertyHolder<implements<TestOwnerBase>>
{
};


#endif // not STOREGATE_TESTOWNER_H
