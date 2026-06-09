/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBXMLWRITER_H
#define TBREC_TBXMLWRITER_H
///////////////////////////////////////////////////////////////////////////////
/// \brief XML writer algorithm for 2004 event display
///
/// The TBXMLWriter is an algorithm invoking writer tools and providing the
/// general frame for an event XML file.
///
///////////////////////////////////////////////////////////////////////////////

#include "AthenaBaseComps/AthAlgorithm.h"

class TBXMLWriterToolBase;

#include <string>
#include <vector>
#include <map>

class TBXMLWriter : public AthAlgorithm
{
 public:
  
  /////////////////////////////////
  // Constructors and Destructor //
  /////////////////////////////////

  /// \brief Algorithm constructor
  TBXMLWriter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TBXMLWriter();

  ///////////////////////
  // Algorithm Methods //
  ///////////////////////

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

  // tool support
  const std::string&  getFileDir() const { return m_topDirectory; }


 private: 

  ////////////////
  // Properties //
  ////////////////

  unsigned int m_outputFrequency;
  unsigned int m_eventCounter;

  std::vector<std::string> m_writerToolNames;
 
  std::string m_topDirectory;

  std::vector<TBXMLWriterToolBase*> m_writerTools;

  ////////////
  // Stores //
  ////////////

  std::map<std::string,unsigned int> m_toolAccept;
  std::map<std::string,unsigned int> m_toolReject;
  std::map<std::string,unsigned int> m_toolInvoke;

  ///////////////
  // Functions //
  ///////////////

};
#endif
