/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLEXAMPLEALGORITHMS_READMETA_H
#define ATHENAPOOLEXAMPLEALGORITHMS_READMETA_H

/** @file ReadMeta.h
 *  @brief This file contains the class definition for the ReadMeta class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IIncidentListener.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IMetaDataTool.h"

class StoreGateSvc;

namespace AthPoolEx {

/** @class AthPoolEx::ReadMeta
 *  @brief This class provides an example for reading in file meta data objects from Pool.
 **/
class ReadMeta : public extends<AthAlgTool, IMetaDataTool, IIncidentListener> {
public: // Constructor and Destructor
   /// Standard Service Constructor
   ReadMeta(const std::string& type, const std::string& name, const IInterface* parent);
   /// Destructor
   virtual ~ReadMeta();

public:
   /// Gaudi AlgTool Interface method implementations:
   virtual StatusCode initialize() override;

   /// Function called when a new input file is opened
   virtual StatusCode beginInputFile(const SG::SourceID&) override;

   /// Function called when the currently open input file got completely
   /// processed
   virtual StatusCode endInputFile(const SG::SourceID&) override {return StatusCode::SUCCESS;}

   /// Function writing the collected metadata to the output
   virtual StatusCode metaDataStop() override {return StatusCode::SUCCESS;}

   /// Incident service handle listening for BeginInputFile and EndInputFile.
   virtual void handle(const Incident& incident) override;

private:
   ServiceHandle<StoreGateSvc> m_pMetaDataStore;
   ServiceHandle<StoreGateSvc> m_pInputStore;
};

} // end AthPoolEx namespace

#endif
