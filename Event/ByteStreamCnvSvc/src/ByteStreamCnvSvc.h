/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMCNVSVC_BYTESTREAMCNVSVC_H
#define BYTESTREAMCNVSVC_BYTESTREAMCNVSVC_H

#include "ByteStreamCnvSvcBase/IByteStreamOutputSvc.h"
#include "ByteStreamCnvSvcBase/IByteStreamCnvSvc.h"
#include "ByteStreamCnvSvcBase/ByteStreamCnvSvcBase.h"
#include "ByteStreamCnvSvcBase/FullEventAssembler.h"
#include "ByteStreamCnvSvcBase/IByteStreamEventAccess.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "GaudiKernel/ServiceHandle.h"

#include <map>

class FullEventAssemblerBase;

/** @class ByteStreamCnvSvc
    @brief Gaudi Conversion Service class for ByteStream Persistency

    This class is responsible for converting data object to and from BS format
    It inherits from ByteStreamCnvSvcBase, which is used by HLT in online.

    When reading the ByteStream data, the ByteStream converters are called, which in turn call
    IRODDataProviderSvc to retrieve the raw data.

    When writing the object data to ByteStream, an FullEventFragment is assembled
    from lower level fragments using FullEventAssembler, and written out to BS in commitOutput
    method through IByteStreamOutputSvc.
*/

class ByteStreamCnvSvc : public extends<ByteStreamCnvSvcBase,
                                        IByteStreamCnvSvc, IByteStreamEventAccess> {
public:
   /// Standard Constructor
   ByteStreamCnvSvc(const std::string& name, ISvcLocator* svc);

   /// Standard Destructor
   virtual ~ByteStreamCnvSvc() = default;

   /// Gaudi Service Interface method implementations:
   virtual StatusCode initialize() override;
   virtual StatusCode finalize() override;

   /// Implements ConversionSvc's connectOutput
   virtual StatusCode connectOutput(const std::string& t, const std::string& mode) override;
   virtual StatusCode connectOutput(const std::string& t) override;

   /// Implements ConversionSvc's commitOutput
   virtual StatusCode commitOutput(const std::string& outputConnection, bool b) override;

   /// Implementation of IByteStreamEventAccess: Get RawEvent
   virtual RawEventWrite* getRawEvent() override
   {
     return m_slots->m_rawEventWrite.get();
   }

protected:
   RawEventWrite* setRawEvent (std::unique_ptr<RawEventWrite> rawEventWrite);

   /// Implementation of IByteStreamCnvSvc interface
   virtual FullEventAssemblerBase* findFullEventAssembler(const std::string& name) const override;
   virtual StatusCode storeFullEventAssembler(std::unique_ptr<FullEventAssemblerBase> fea, const std::string& name) override;

private:
   /// name of the service
   Gaudi::Property<std::string> m_ioSvcName{this, "ByteStreamOutputSvc", "", "ByteStream output service name"};

   /// list of service names
   Gaudi::Property<std::vector<std::string>> m_ioSvcNameList{this, "ByteStreamOutputSvcList", {}, "", "OrderedSet<T>"};

   /// fill trigger bits
   Gaudi::Property<bool> m_fillTriggerBits{this, "FillTriggerBits", true, "Read in xTrigDecision and use it to fill Trigger bits in event header"};

   /// Services for writing output
   std::map<std::string, IByteStreamOutputSvc*> m_ioSvcMap;

   /// Event store.
   ServiceHandle<StoreGateSvc> m_evtStore{this, "EventStore", "StoreGateSvc"};

   /// user type
   Gaudi::Property<std::string> m_userType{this, "UserType", "RawEvent", "User type"};

   /// Slot-specific state.
   struct SlotData
   {
     std::unique_ptr<RawEventWrite> m_rawEventWrite;
     std::map<std::string, std::unique_ptr<FullEventAssemblerBase>> m_feaMap;
     std::vector<uint32_t> m_tagBuff;
     std::vector<uint32_t> m_l1Buff;
     std::vector<uint32_t> m_l2Buff;
     std::vector<uint32_t> m_efBuff;

     void clear()
     {
       m_rawEventWrite.reset();
       m_feaMap.clear();
       m_tagBuff.clear();
       m_l1Buff.clear();
       m_l2Buff.clear();
       m_efBuff.clear();
     }
   };
   SG::SlotSpecificObj<SlotData> m_slots;

   /// Write the FEA to RawEvent.
   void writeFEA (SlotData& slot);
};

#endif // BYTESTREAMCNVSVC_BYTESTREAMCNVSVC_H
