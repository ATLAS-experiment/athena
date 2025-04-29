/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgMessaging/MessageCheck.h>
#include <AsgTesting/UnitTest.h>
#include <AsgTools/AsgTool.h>
#include <AsgTools/CurrentContext.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteHandleKey.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgDataHandles/WriteDecorHandle.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/ElectronAuxContainer.h>

//
// method implementations
//

using namespace asg::msgUserCode;

TEST (AsgDataHandlesTest, readInvalid)
{
  xAOD::TEvent event;
  xAOD::TStore store;
  auto tool = std::make_unique<asg::AsgTool>("AsgTool");
  SG::ReadHandleKey<xAOD::ElectronContainer> key {tool.get(), "electrons", "electrons", "Electron Container"};
  ASSERT_SUCCESS (key.initialize());
  ASSERT_SUCCESS (tool->initialize());

  auto handle = makeHandle (key, Gaudi::Hive::currentContext());
  ASSERT_FALSE (handle.isValid());
}

TEST (AsgDataHandlesTest, readValid)
{
  xAOD::TEvent event;
  xAOD::TStore store;
  auto tool = std::make_unique<asg::AsgTool>("AsgTool");
  SG::ReadHandleKey<xAOD::ElectronContainer> key {tool.get(), "electrons", "electrons", "Electron Container"};
  ASSERT_SUCCESS (key.initialize());
  ASSERT_SUCCESS (tool->initialize());

  auto electrons = new xAOD::ElectronContainer;
  ASSERT_SUCCESS (store.record (electrons, "electrons"));
  auto handle = makeHandle (key, Gaudi::Hive::currentContext());
  ASSERT_TRUE (handle.isValid());
  ASSERT_EQ (electrons, handle.get());
}

TEST (AsgDataHandlesTest, readRepoint)
{
  xAOD::TEvent event;
  xAOD::TStore store;
  auto tool = std::make_unique<asg::AsgTool>("AsgTool");
  SG::ReadHandleKey<xAOD::ElectronContainer> key {tool.get(), "electrons", "electrons", "Electron Container"};
  ASSERT_SUCCESS (tool->setProperty ("electrons", "myElectrons"));
  ASSERT_SUCCESS (key.initialize());
  ASSERT_SUCCESS (tool->initialize());

  auto electrons = new xAOD::ElectronContainer;
  ASSERT_SUCCESS (store.record (electrons, "myElectrons"));
  auto handle = makeHandle (key, Gaudi::Hive::currentContext());
  ASSERT_TRUE (handle.isValid());
  ASSERT_EQ (electrons, handle.get());
}

TEST (AsgDataHandlesTest, writeDecorBasic)
{
  xAOD::TEvent event;
  xAOD::TStore store;
  auto tool = std::make_unique<asg::AsgTool>("AsgTool");
  SG::WriteDecorHandleKey<xAOD::ElectronContainer> key {tool.get(), "decor", "electrons.decor", "Electron Decoration"};
  ASSERT_SUCCESS (key.initialize());
  ASSERT_SUCCESS (tool->initialize());

  auto electrons = new xAOD::ElectronContainer;
  auto aux = new xAOD::ElectronAuxContainer;
  electrons->setStore (aux);
  electrons->push_back (new xAOD::Electron);
  ASSERT_SUCCESS (store.record (electrons, "electrons"));
  ASSERT_SUCCESS (store.record (aux, "electronsAux."));
  auto handle = makeHandle<float> (key, Gaudi::Hive::currentContext());
  ASSERT_TRUE (handle.isValid());
  ASSERT_EQ (electrons, handle.get());
  handle(*electrons->at(0)) = 3;
  ASSERT_EQ (3, electrons->at(0)->auxdata<float>("decor"));
}

TEST (AsgDataHandlesTest, writeDecorIndirect)
{
  xAOD::TEvent event;
  xAOD::TStore store;
  auto tool = std::make_unique<asg::AsgTool>("AsgTool");
  SG::WriteHandleKey<xAOD::ElectronContainer> baseKey {tool.get(), "electrons", "electrons", "Electron Container"};
  SG::WriteDecorHandleKey<xAOD::ElectronContainer> key {tool.get(), "decor", baseKey, "decor", "Electron Decoration"};
  ASSERT_SUCCESS (baseKey.initialize());
  ASSERT_SUCCESS (key.initialize());
  ASSERT_SUCCESS (tool->initialize());

  auto electrons = new xAOD::ElectronContainer;
  auto aux = new xAOD::ElectronAuxContainer;
  electrons->setStore (aux);
  electrons->push_back (new xAOD::Electron);
  ASSERT_SUCCESS (store.record (electrons, "electrons"));
  ASSERT_SUCCESS (store.record (aux, "electronsAux."));
  auto handle = makeHandle<float> (key, Gaudi::Hive::currentContext());
  ASSERT_TRUE (handle.isValid());
  ASSERT_EQ (electrons, handle.get());
  handle(*electrons->at(0)) = 3;
  ASSERT_EQ (3, electrons->at(0)->auxdata<float>("decor"));
}

TEST (AsgDataHandlesTest, writeDecorIndirectReset)
{
  xAOD::TEvent event;
  xAOD::TStore store;
  auto tool = std::make_unique<asg::AsgTool>("AsgTool");
  SG::WriteHandleKey<xAOD::ElectronContainer> baseKey {tool.get(), "electrons", "electrons", "Electron Container"};
  SG::WriteDecorHandleKey<xAOD::ElectronContainer> key {tool.get(), "decor", baseKey, "decor", "Electron Decoration"};
  ASSERT_SUCCESS (tool->setProperty ("electrons", "myElectrons"));
  ASSERT_SUCCESS (baseKey.initialize());
  ASSERT_SUCCESS (key.initialize());
  ASSERT_SUCCESS (tool->initialize());

  auto electrons = new xAOD::ElectronContainer;
  auto aux = new xAOD::ElectronAuxContainer;
  electrons->setStore (aux);
  electrons->push_back (new xAOD::Electron);
  ASSERT_SUCCESS (store.record (electrons, "myElectrons"));
  ASSERT_SUCCESS (store.record (aux, "myElectronsAux."));
  auto handle = makeHandle<float> (key, Gaudi::Hive::currentContext());
  ASSERT_TRUE (handle.isValid());
  ASSERT_EQ (electrons, handle.get());
  handle(*electrons->at(0)) = 3;
  ASSERT_EQ (3, electrons->at(0)->auxdata<float>("decor"));
}

ATLAS_GOOGLE_TEST_MAIN
