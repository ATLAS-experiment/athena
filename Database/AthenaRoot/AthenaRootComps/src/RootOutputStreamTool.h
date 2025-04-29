///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// RootOutputStreamTool.h
// Header file for class Athena::RootOutputStreamTool
// Author Peter van Gemmeren <gemmeren@anl.gov>
///////////////////////////////////////////////////////////////////
#ifndef ATHENAROOTCOMPS_ROOTOUTPUTSTREAMTOOL_H
#define ATHENAROOTCOMPS_ROOTOUTPUTSTREAMTOOL_H 1

/** @file Athena::RootOutputStreamTool.h
 *  @brief This is the AthenaRoot version of AthenaOutputStreamTool.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaKernel/IAthenaOutputStreamTool.h"

// fwk
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

class StoreGateSvc;
class IConversionSvc;
class IClassIDSvc;

namespace Athena {
/** @class Athena::RootOutputStreamTool
 *  @brief This is the AthenaRoot version of AthenaServices/AthenaOutputStreamTool.
 **/
class RootOutputStreamTool : public extends<::AthAlgTool, ::IAthenaOutputStreamTool> {
public:
  /// Standard AlgTool Constructor
  RootOutputStreamTool(const std::string& type, const std::string& name, const IInterface* parent);

  /// Destructor
  virtual ~RootOutputStreamTool();

  /// Gaudi AlgTool Interface method implementations:
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  /// Specify which data store and conversion service to use
  /// and whether to extend provenence
  ///   Only use if one wants to override jobOptions
  virtual StatusCode connectServices(const std::string& dataStore, const std::string& cnvSvc, bool extendProvenenceRecord) override;

  /// Connect to the output stream
  ///   Must connectOutput BEFORE streaming
  ///   Only specify "outputName" if one wants to override jobOptions
  virtual StatusCode connectOutput(const std::string& outputName) override;

  /// Commit the output stream after having streamed out objects
  ///   Must commitOutput AFTER streaming
  virtual StatusCode commitOutput(bool doCommit = false) override;

  /// Finalize the output stream after the last commit, e.g. in
  /// finalize
  virtual StatusCode finalizeOutput() override;

  /// Stream out objects. Provide vector of typeName/key pairs.
  ///   If key is empty, assumes only one object and this
  ///   will fail if there is more than one
  virtual StatusCode streamObjects(const IAthenaOutputStreamTool::TypeKeyPairs& typeKeys, const std::string& outputName = "") override;

  /// Stream out a vector of objects
  ///   Must convert to DataObject, e.g.
  ///   #include "AthenaKernel/StorableConversions.h"
  ///     T* obj = xxx;
  ///     DataObject* dataObject = SG::asStorable(obj);
  virtual StatusCode streamObjects(const IAthenaOutputStreamTool::DataObjectVec& dataObjects, const std::string& outputName = "") override;

  virtual StatusCode getInputItemList(SG::IFolder* m_p2BWrittenFromTool) override;

private:
  /// ServiceHandle to the data store service
  ServiceHandle< ::StoreGateSvc> m_storeSvc;
  /// ServiceHandle to the data conversion service
  ServiceHandle< ::IConversionSvc> m_conversionSvc;
  /// ServiceHandle to clid service
  ServiceHandle< ::IClassIDSvc> m_clidSvc;

  /// Name of the output file
  std::string m_outputName;

  /// Name of the output tuple
  std::string m_treeName;
};

}//> end namespace Athena

#endif
