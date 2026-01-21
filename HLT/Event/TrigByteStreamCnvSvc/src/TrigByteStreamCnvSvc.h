/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGBYTESTREAMCNVSVC_H
#define TRIGBYTESTREAMCNVSVC_H

#include "ByteStreamCnvSvcBase/ByteStreamCnvSvcBase.h"
#include "ByteStreamCnvSvcBase/IByteStreamCnvSvc.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "EFInterfaceSvc.h"

// Forward declarations
class StoreGateSvc;
class IROBDataProviderSvc;

/** @class TrigByteStreamCnvSvc
 *  @brief A ByteStreamCnvSvc implementation for online use
 *
 *  It overrides the connectOutput and commitOutput methods of the base class. In this implementation, they create
 *  the specific online HLT output and send it out directly to the TDAQ infrastructure without using an output service.
 **/
class TrigByteStreamCnvSvc : public extends<ByteStreamCnvSvcBase,
                                            IByteStreamCnvSvc, IByteStreamEventAccess> {
public:
  /// Standard constructor
  TrigByteStreamCnvSvc(const std::string& name, ISvcLocator* svcLoc);
  /// Standard destructor
  virtual ~TrigByteStreamCnvSvc();

  // ------------------------- IService methods --------------------------------
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  // ------------------------- IConversionSvc methods --------------------------
  /// In the case of online BS data, this method creates the output FullEventFragment and fills its header
  virtual StatusCode connectOutput(const std::string& outputFile) override;
  /// In the case of online BS data, this method creates the output FullEventFragment and fills its header
  StatusCode connectOutput(const std::string& outputFile, const EventContext& eventContext);
  /// This overload is kept only for interface compatibility
  virtual StatusCode connectOutput(const std::string& outputFile, const std::string& openMode) override;
  /// In the case of online BS data, this method binds and sends out the output FullEventFragment
  virtual StatusCode commitOutput(const std::string& outputFile, bool do_commit) override;
  /// In the case of online BS data, this method binds and sends out the output FullEventFragment
  StatusCode commitOutput(const std::string& outputFile, bool do_commit, const EventContext& eventContext);

  // ------------------------- IByteStreamEventAccess methods ------------------
  /// Return a pointer to the raw event for the current event context
  virtual RawEventWrite* getRawEvent() override;
  /// Return a pointer to the raw event for the given event context
  RawEventWrite* getRawEvent(const EventContext& eventContext) const;

protected:
   // ------------------------- IByteStreamCnvSvc methods ------------------
   virtual FullEventAssemblerBase* findFullEventAssembler(const std::string& name) const override;
   virtual StatusCode storeFullEventAssembler(std::unique_ptr<FullEventAssemblerBase> fea, const std::string& name) override;

private:
  // ------------------------- Helper methods ----------------------------------
  /// Store new raw event in the cache
  RawEventWrite* setRawEvent(std::unique_ptr<RawEventWrite>&& rawEventWrite, const EventContext& eventContext);
  /// Delete raw event from the cache
  void clearRawEvent(const EventContext& eventContext);
  /// Print contents of the raw event
  void printRawEvent(const EventContext& eventContext) const;
  /// Fill histograms from contents of a FullEventFragment
  void monitorRawEvent(const std::unique_ptr<uint32_t[]>& rawEventPtr) const;
  /// Hack used in HLT to avoid using ThreadLocalContext, see explanation in the implementation
  const EventContext* currentContext() const;

  // ------------------------- Service / Tool handles --------------------------
  ServiceHandle<StoreGateSvc> m_evtStore {this, "EventStore", "StoreGateSvc"};
  ServiceHandle<IROBDataProviderSvc> m_robDataProviderSvc {this, "ROBDataProvider", "ROBDataProviderSvc"};
  ServiceHandle<EFInterfaceSvc> m_efInterfaceSvc {this, "EFInterfaceSvc", "", "Online service managing EF interface with Dataflow"};
  ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "" , "Monitoring tool"};

  // ------------------------- Other private members ---------------------------
  SG::SlotSpecificObj<std::unique_ptr<RawEventWrite>> m_rawEventWriteCache;
  bool m_hasEFInterface{false}; //! Flag to select use of legacy or EF interface
};

#endif // TRIGBYTESTREAMCNVSVC_H
