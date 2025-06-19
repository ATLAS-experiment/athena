/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLCNVSVC_ATHENAPOOLCNVSVC_H
#define ATHENAPOOLCNVSVC_ATHENAPOOLCNVSVC_H

/** @file AthenaPoolCnvSvc.h
 *  @brief This file contains the class definition for the AthenaPoolCnvSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaPoolBaseCnvSvc.h"
#include "AthenaKernel/IDataShare.h"
#include "AthenaKernel/IAthenaSerializeSvc.h"

// Forward declarations
class Guid;

template <class TYPE> class SvcFactory;

/** @class AthenaPoolCnvSvc
 *  @brief This class provides the interface between Athena and PoolSvc.
 **/
class AthenaPoolCnvSvc : public extends<AthenaPoolBaseCnvSvc,
					IDataShare> {
   // Allow the factory class access to the constructor
   friend class SvcFactory<AthenaPoolCnvSvc>;

public:

   /// Required of all Gaudi Services
   virtual StatusCode initialize() override;
   /// Required of all Gaudi Services
   virtual StatusCode finalize() override;

   /// Implementation of IConversionSvc: Connect to the output connection specification with open mode.
   /// @param outputConnectionSpec [IN] the name of the output connection specification as string.
   /// @param openMode [IN] the open mode of the file as string.
   virtual StatusCode connectOutput(const std::string& outputConnectionSpec,
		   const std::string& openMode) override;

   /// Implementation of IConversionSvc: Connect to the output connection specification with open mode.
   /// @param outputConnectionSpec [IN] the name of the output
   /// connection specification as string.
   virtual StatusCode connectOutput(const std::string& outputConnectionSpec) override;

   /// Implementation of IConversionSvc: Commit pending output.
   /// @param doCommit [IN] boolean to force full commit
   virtual StatusCode commitOutput(const std::string& outputConnectionSpec, bool doCommit) override;

   /// Disconnect to the output connection.
   virtual StatusCode disconnectOutput(const std::string& outputConnectionSpec) override;

   /// @return a string token to a Data Object written to Pool
   /// @param placement [IN] pointer to the placement hint
   /// @param obj [IN] pointer to the Data Object to be written to Pool
   /// @param classDesc [IN] pointer to the Seal class description for the Data Object.
   virtual Token* registerForWrite(Placement* placement, const void* obj, const RootType& classDesc) override;

   /// @param obj [OUT] pointer to the Data Object.
   /// @param token [IN] string token of the Data Object for which a Pool Ref is filled.
   virtual void setObjPtr(void*& obj, const Token* token) override;

   /// Create a Generic address using explicit arguments to identify a single object.
   /// @param svcType [IN] service type of the address.
   /// @param clid [IN] class id for the address.
   /// @param par [IN] string containing the database name.
   /// @param ip [IN] object identifier.
   /// @param refpAddress [OUT] converted address.
   StatusCode createAddress(long svcType,
		   const CLID& clid,
		   const std::string* par,
		   const unsigned long* ip,
		   IOpaqueAddress*& refpAddress) override;

   /// Create address from string form
   /// @param svcType [IN] service type of the address.
   /// @param clid [IN] class id for the address.
   /// @param refAddress [IN] string form to be converted.
   /// @param refpAddress [OUT] converted address.
   virtual StatusCode createAddress(long svcType,
		   const CLID& clid,
		   const std::string& refAddress,
		   IOpaqueAddress*& refpAddress) override;

   /// Make this a server.
   virtual StatusCode makeServer(int num) override;

   /// Make this a client.
   virtual StatusCode makeClient(int num) override;

   /// Read the next data object
   virtual StatusCode readData() override;

   /// Commit Catalog
   virtual StatusCode commitCatalog() override;

   /// Send abort to SharedWriter clients if the server quits on error
   /// @param client_n [IN] number of the current client, -1 if no current
   StatusCode abortSharedWrClients(int client_n);

   /// Implementation of IIncidentListener: Handle for EndEvent incidence
   virtual void handle(const Incident& incident) override;

   /// Standard Service Constructor
   AthenaPoolCnvSvc(const std::string& name, ISvcLocator* pSvcLocator);
   /// Destructor
   virtual ~AthenaPoolCnvSvc() = default;

private: // data
   ServiceHandle<IAthenaSerializeSvc> m_serializeSvc{this,"AthenaRootSerializeSvc","AthenaRootSerializeSvc"};
   ToolHandle<IAthenaIPCTool>    m_inputStreamingTool{this,"InputStreamingTool",{}};
   ToolHandle<IAthenaIPCTool>    m_outputStreamingTool{this,"OutputStreamingTool",{}};
   bool m_streamServerActive=false;
   int m_metadataClient=0;

private: // properties
   /// For SharedWriter:
   /// To use MetadataSvc to merge data placed in a certain container
   StringProperty  m_metadataContainerProp{this,"OutputMetadataContainer","MetaData"};
   StringArrayProperty m_metadataContainersAug{this, "OutputMetadataContainers", {}, "Metadata containers used for augmentations"};

   /// Make this instance a Streaming Client during first connect/write automatically
   IntegerProperty m_makeStreamingToolClient{this,"MakeStreamingToolClient",0};
   /// Use Streaming for selected technologies only
   IntegerProperty m_streamingTechnology{this,"StreamingTechnology",-1};
   /// Use Athena Object sharing for metadata only, event data is collected and send via ROOT TMemFile
   BooleanProperty m_parallelCompression{this,"ParallelCompression",true};
   /// Extension to use ROOT TMemFile for event data, "?pmerge=<host>:<port>"
   StringProperty  m_streamPortString{this,"StreamPortString","?pmerge=localhost:0"};
};

#endif
