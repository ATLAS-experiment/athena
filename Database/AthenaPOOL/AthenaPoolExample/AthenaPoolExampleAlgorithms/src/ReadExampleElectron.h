/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLEXAMPLEALGORITHMS_READSIMPLEELECTRON_H
#define ATHENAPOOLEXAMPLEALGORITHMS_READSIMPLEELECTRON_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolExampleData/ExampleElectronContainer.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

namespace AthPoolEx {

/** @class ReadExampleElectron
 *  @brief This class provides an example for reading event data objects from
 *Pool.
 **/
class ReadExampleElectron : public AthReentrantAlgorithm {
 public:
  ReadExampleElectron(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~ReadExampleElectron() = default;

 public:
  /// Gaudi Service Interface method implementations:
  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;
  virtual StatusCode finalize() override final;

 private:
  // Reading through the container of example electrons
  SG::ReadHandleKey<xAOD::ExampleElectronContainer>
      m_exampleElectronContainerKey{this, "ExampleElectronContainerName",
                                    "TestContainer"};

  // Also want to read through '.decor1' decorations, ignoring '.decor2' for
  // sake of demonstration see python/AthenaPoolExample_Write.py:
  //     ItemList = [ ... ,
  //     'xAOD::ExampleElectronAuxContainer#TestContainerAux.-decor2' ]
  SG::ReadDecorHandleKey<xAOD::ExampleElectronContainer> m_decor1Key{
      this, "ExampleElectronContainerDecorKey1", "TestContainer.decor1",
      "decorator1 key"};
};

}  // namespace AthPoolEx

#endif
