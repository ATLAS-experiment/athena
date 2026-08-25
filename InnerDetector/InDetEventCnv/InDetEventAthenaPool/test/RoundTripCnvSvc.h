/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATESTCNVBASE_H
#define ATESTCNVBASE_H
#include "AthenaPoolCnvSvc/IAthenaPoolCnvSvc.h"
#include "AthenaBaseComps/AthService.h"
#include "StorageSvc/DbType.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbReflex.h"

#include <cstdint>
#include <string>
#include <cassert>

#ifdef NDEBUG
#  undef NDEBUG
#endif

#undef CHECK
template<typename T>
void assert_expr(T &&result) {
   if constexpr(std::is_same_v<StatusCode, std::remove_cvref_t<decltype(result)> >) {
      assert(result.isSuccess());
   }
   else {
      assert(result);
   }
}

#define CHECK(s) do { assert_expr(s); } while(0)

namespace InDetEventAthenaPool {
// copy of Database/AthenaPOOL/AthenaPoolCnvSvc/test/TestCnvSvcBase.icc
class TestCnvSvcBase
  : public extends<AthService, IAthenaPoolCnvSvc>
{
public:
  TestCnvSvcBase (const std::string& name, ISvcLocator* svcloc)
    : base_class(name, svcloc),
      m_name (name)
  {}

  virtual StatusCode disconnectOutput(const std::string& /*outputFile*/) override
  { std::abort(); }
  virtual IPoolSvc* getPoolSvc() override
  { std::abort(); }
  virtual Token* registerForWrite(Placement* /*placement*/,
                                        const void* /*obj*/,
                                        const RootType& /*classDesc*/) override
  { std::abort(); }
  virtual bool useDetailChronoStat() const override
  { std::abort(); }
  virtual StatusCode createAddress(long /*svcType*/,
                                   const CLID& /*clid*/,
                                   const std::string* /*par*/,
                                   const unsigned long* /*ip*/,
                                   IOpaqueAddress*& /*refpAddress*/) override
  { std::abort(); }
  virtual StatusCode createAddress(long /*svcType*/,
                                   const CLID& /*clid*/,
                                   const std::string& /*refAddress*/,
                                   IOpaqueAddress*& /*refpAddress*/) override
  { std::abort(); }
  virtual StatusCode convertAddress(const IOpaqueAddress* /*pAddress*/,
                                    std::string& /*refAddress*/) override

  { std::abort(); }
  virtual StatusCode cleanUp(const std::string& /*connection*/) override
  { std::abort(); }
  virtual StatusCode setInputAttributes(const std::string& /*fileName*/) override
  { std::abort(); }

  virtual StatusCode initialize() override
  { std::abort(); }
  virtual StatusCode finalize() override
  { std::abort(); }
  virtual const CLID& objType() const override
  { std::abort(); }
  virtual long repSvcType() const override
  { std::abort(); }
  virtual StatusCode setDataProvider(IDataProviderSvc* /*pService*/) override
  { std::abort(); }
  virtual SmartIF<IDataProviderSvc>& dataProvider() const override
  { std::abort(); }
  virtual StatusCode setConversionSvc(IConversionSvc* /*pService*/) override
  { std::abort(); }
  virtual SmartIF<IConversionSvc>& conversionSvc()    const override
  { std::abort(); }
  virtual StatusCode setAddressCreator(IAddressCreator* /*creator*/) override
  { std::abort(); }
  virtual SmartIF<IAddressCreator>& addressCreator()    const override
  { std::abort(); }
  virtual StatusCode createObj(IOpaqueAddress* /*pAddress*/, DataObject*& /*refpObject*/) override
  { std::abort(); }
  virtual StatusCode fillObjRefs(IOpaqueAddress* /*pAddress*/, DataObject* /*pObject*/) override
  { std::abort(); }
  virtual StatusCode updateObj(IOpaqueAddress* /*pAddress*/, DataObject* /*refpObject*/) override
  { std::abort(); }
  virtual StatusCode updateObjRefs(IOpaqueAddress* /*pAddress*/, DataObject* /*pObject*/) override
  { std::abort(); }
  virtual StatusCode createRep(DataObject* /*pObject*/, IOpaqueAddress*& /*refpAddress*/) override
  { std::abort(); }
  virtual StatusCode fillRepRefs(IOpaqueAddress* /*pAddress*/, DataObject* /*pObject*/) override
  { std::abort(); }
  virtual StatusCode updateRep(IOpaqueAddress* /*pAddress*/, DataObject* /*pObject*/)  override
  { std::abort(); }
  virtual StatusCode updateRepRefs(IOpaqueAddress* /*pAddress*/, DataObject* /*pObject*/) override
  { std::abort(); }
  virtual StatusCode addConverter(const CLID& /*clid*/) override
  { std::abort(); }
  virtual StatusCode removeConverter(const CLID& /*clid*/) override
  { std::abort(); }
  virtual IConverter* converter(const CLID& /*clid*/) override
  { std::abort(); }
  virtual StatusCode connectOutput(const std::string& /*outputFile*/) override
  { std::abort(); }
  virtual const std::string& name() const override
  { return m_name; }
  virtual StatusCode addConverter(IConverter* /*pConverter*/) override
  { std::abort(); }
  virtual StatusCode connectOutput(const std::string& /*outputFile*/,
                                   const std::string& /*openMode*/) override
  { std::abort(); }
  virtual StatusCode commitOutput(const std::string& /*outputFile*/,
                                  bool /*do_commit*/) override
  { std::abort(); }

  virtual StatusCode registerCleanUp(IAthenaPoolCleanUp* /*cnv*/) override
  {
    return StatusCode::SUCCESS;
  }

  std::string m_name;
};
}

class RoundTripCnvSvc
  : public InDetEventAthenaPool::TestCnvSvcBase
{
public:
   using InDetEventAthenaPool::TestCnvSvcBase::TestCnvSvcBase;

  virtual void setObjPtr(void*& obj, const Token* token) override
  {
     CHECK(token != nullptr);
     std::string key = computeKey(*token);
     std::map<std::string, void *>::iterator iter = m_objs.find(key);

     if (iter == m_objs.end()) {
        throw std::runtime_error(std::string("No object with key")+key);
     }
     obj=iter->second;
     m_objs.erase( iter );
  }


  virtual Token* registerForWrite(Placement* placement,
                                  const void* obj,
                                  const RootType& classDesc) override {
     CHECK(placement != nullptr);
     CHECK(obj != nullptr);
     auto token = std::make_unique<Token>();
     auto guid = pool::DbReflex::guid (classDesc);
     token->setClassID (guid);
     token->setCont(placement->containerName());
     // take ownership
     void *non_const_obj ATLAS_THREAD_SAFE = const_cast<void *>(obj);
     registerPtr(non_const_obj, *token);
     return token.release();
  }

protected:

  static std::string computeKey(const Token &token) {
     return token.toString();
  }

  void registerPtr(void *obj, const Token &token) {
     m_objs.insert( std::make_pair(computeKey(token),std::move(obj)));
  }
  std::map<std::string, void * > m_objs;
};


#endif
