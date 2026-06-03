/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBXMLEVENTWRITERTOOL_H
#define TBREC_TBXMLEVENTWRITERTOOL_H
///////////////////////////////////////////////////////////////////////////////
/// \brief writes out event header
///////////////////////////////////////////////////////////////////////////////

#include "TBXMLWriterToolBase.h"

#include <iosfwd>
#include <string>
#include <string_view>

class TBXMLWriter;

class TBXMLEventWriterTool : public TBXMLWriterToolBase
{

 public:

  /////////////////////////////////
  // Constructors and Destructor //
  /////////////////////////////////

  TBXMLEventWriterTool(const std::string& type,
		       const std::string& name,
		       const IInterface* parent);

  ~TBXMLEventWriterTool();

  ////////////
  // Action //
  ////////////

  // virtual StatusCode initialize();

 protected:

  virtual StatusCode writeRunFiles(const std::string& /* fileDir */,
				   unsigned int /*runNumber*/ ) override;

  virtual StatusCode writeEvent( std::ostream& outFile,
				 std::string_view entryTag) override;
};
#endif
