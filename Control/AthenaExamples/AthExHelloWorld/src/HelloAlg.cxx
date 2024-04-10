/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "HelloAlg.h"

#include <AthExHelloWorld/IHelloTool.h>

HelloAlg::HelloAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : AthAlgorithm(name, pSvcLocator), m_myTable() {

  // override some default values (headers are preferred)
  m_myDict["Bonjour"] = "Guten Tag";
  m_myDict["Good Morning"] = "Bonjour";
  m_myDict["one"] = "uno";

  // legacy way of properties (headers are preferred)
  declareProperty("MyTable", m_myTable, "A table of <double,double>");
  // some default values
  m_myTable.push_back(std::make_pair(1., 1.));
  m_myTable.push_back(std::make_pair(2., 2. * 2.));
  m_myTable.push_back(std::make_pair(3., 3. * 3.));
}

StatusCode HelloAlg::initialize() {
  // Part 1: print where you are
  ATH_MSG_INFO("initialize()");

  // Part 2: Print out the property values
  ATH_MSG_INFO("  MyInt =    " << m_myInt.value() << endmsg
                               << "  MyBool =   " << m_myBool.value() << endmsg
                               << "  MyDouble = " << m_myDouble.value());

  for (size_t i = 0; i < m_myStringVec.size(); i++) {
    ATH_MSG_INFO("  MyStringVec[" << i << "] = " << m_myStringVec[i]);
  }

  for (const auto &[key, value] : m_myDict) {
    ATH_MSG_INFO("  MyDict['" << key << "'] = '" << value << "'");
  }
  for (auto [key, value] : m_myTable) {
    ATH_MSG_INFO("  MyTable['" << key << "'] = '" << value << "'");
  }
  for (size_t row = 0; row < m_myMatrix.size(); row++) {
    msg(MSG::INFO) << "  MyMatrix[" << row << "] = [ ";
    for (double value : m_myMatrix[row]) {
      msg() << value << " ";
    }
    msg() << "]" << endmsg;
  }

  ATH_MSG_INFO("  " << m_myPrivateHelloTool.propertyName() << " = "
                    << m_myPrivateHelloTool.type() << endmsg << "  "
                    << m_myPublicHelloTool.propertyName() << " = "
                    << m_myPublicHelloTool.type());

  // Part 3: Retrieve the tools using the ToolHandles
  if (m_myPrivateHelloTool.retrieve().isFailure()) {
    ATH_MSG_FATAL(m_myPrivateHelloTool.propertyName()
                  << ": Failed to retrieve tool "
                  << m_myPrivateHelloTool.type());
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO(m_myPrivateHelloTool.propertyName()
                 << ": Retrieved tool " << m_myPrivateHelloTool.type());
  }

  // or just use ATH_CHECK to be less verbose
  ATH_CHECK(m_myPublicHelloTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode HelloAlg::execute() {
  // Part 1: print where you are (should never be INFO)
  ATH_MSG_DEBUG("execute()");

  // Part 1: Print out the different levels of messages
  ATH_MSG_DEBUG("A DEBUG message");
  ATH_MSG_INFO("An INFO message");
  ATH_MSG_WARNING("A WARNING message");
  ATH_MSG_ERROR("An ERROR message");
  ATH_MSG_FATAL("A FATAL error message");

  // Part 1a: Let publicly declared tool say something
  ATH_MSG_INFO("Let the tool " << m_myPublicHelloTool.propertyName()
                               << " say something:");
  ATH_CHECK(m_myPublicHelloTool->saySomething());

  // Part 1b: Let privately declared tool say something
  ATH_MSG_INFO("Let the tool " << m_myPrivateHelloTool.propertyName()
                               << " say something:");
  ATH_CHECK(m_myPrivateHelloTool->saySomething());

  return StatusCode::SUCCESS;
}

StatusCode HelloAlg::finalize() {
  // Part 1: print where you are (empty finalize is generally discouraged)
  ATH_MSG_INFO("finalize()");

  return StatusCode::SUCCESS;
}
