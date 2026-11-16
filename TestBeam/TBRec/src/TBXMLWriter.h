/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

  Gaudi::Property<unsigned int> m_outputFrequency{this, "OutputFrequency", 1, "Write out every N-th event"};
  unsigned int m_eventCounter{0};

  Gaudi::Property<std::vector<std::string>> m_writerToolNames{this, "WriteTools", {}, "List of XML writer tools (type/name)"};
 
  Gaudi::Property<std::string> m_topDirectory{this, "FileTopDirectory", "", "Top directory for XML output files"};

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
