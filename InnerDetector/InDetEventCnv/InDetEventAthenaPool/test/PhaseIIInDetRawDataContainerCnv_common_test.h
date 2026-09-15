/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#include "TestTools/initGaudi.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/Bootstrap.h"
#include "IdDictParser/IdDictParser.h"
#include "InDetIdentifier/PixelID.h"
#include "PersistentDataModel/TokenAddress.h"
#include "PersistentDataModel/Guid.h"
#include "RoundTripCnvSvc.h"

#ifdef NDEBUG
#  undef NDEBUG
#endif

template<typename T_IDHelper>
struct HelperBase {
   static std::unique_ptr<T_IDHelper> makeIdHelper() {
      std::unique_ptr<T_IDHelper> id = std::make_unique<T_IDHelper>();
      IdDictParser parser;
      IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
      id->initialize_from_dictionary (idd);
      return id;
   }
};


template <typename T_Container, typename T_Helper>
std::unique_ptr<T_Container> makeTestData(const T_Helper &rdo_maker, std::span<const unsigned int> hits_per_module) {
   //    auto cont_coll = std::make_unique< T_Container>(m_idHelper->wafer_hash_max(), container_list_size);
    std::unique_ptr<T_Container> container_collection = std::make_unique<T_Container>(hits_per_module.size(),1u);

    unsigned int total_n_hits=std::accumulate(hits_per_module.begin(),hits_per_module.end(),0u);
    auto container = container_collection->getNewContainerPtr();
    container->reserve(total_n_hits);
    unsigned int module_i=0;
    for (unsigned int n_hits : hits_per_module) {
       PhaseII::ContainerRangeGuard<PhaseII::DataRange, typename T_Container::ContainerPtr> range_guard(container);
       for (unsigned int hit_i=0; hit_i<n_hits; ++hit_i) {
          PhaseII::addDataForModule(*container_collection,
                                    range_guard,
                                    rdo_maker.makeCoordinates(module_i, hit_i),
                                    rdo_maker.makeDataWord(module_i, hit_i));
       }
       if (!range_guard.empty()) {
          assert(container_collection->registerOrEraseNewData(module_i, range_guard.range()));
       }
       ++module_i;
    }

    return container_collection;
}

ISvcLocator *init() {
   ISvcLocator*svcloc=nullptr;
   CHECK (Athena_test::initGaudi("PhaseIIPixelRawDataContainerCnv_test.txt", svcloc));

   SmartIF<StoreGateSvc> detstore{svcloc->service ("DetectorStore")};
   SmartIF<StoreGateSvc> condstore{svcloc->service ("StoreGateSvc/ConditionStore")};

   EventIDBase now(0, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM, 0, 1);
   EventContext ctx(1);
   ctx.setEventID( now );
   ctx.setExtension( Atlas::ExtendedEventContext(condstore) );
   Gaudi::Hive::setCurrentContext(ctx);

   RoundTripCnvSvc* svc = new RoundTripCnvSvc ("AthenaPoolCnvSvc", svcloc);
   ISvcManager* mgr = dynamic_cast<ISvcManager*> (svcloc);
   CHECK(mgr);
   CHECK(mgr->addService (svc).isSuccess());

   return svcloc;
}


template <class T_Base, typename T_Pers, typename T_Trans>
class CnvWrapper :  public T_Base {
public:
   using BASE=T_Base;
   using BASE::BASE;
   using TRANS = T_Trans;
   using PERS  = T_Pers;

   virtual StatusCode initialize() override {
      return BASE::initialize();
   }
   virtual PERS* createPersistentWithKey(TRANS* obj, const std::string& key) override {
      return BASE::createPersistentWithKey(obj,key);
   }
   virtual TRANS* createTransient(const Token* token) override {
      return BASE::createTransient(token);
   }

};


namespace {
   // compare the content of two PhaseIIPixelRawDataContainer
   bool operator==(const PhaseIIPixelRawDataContainer &a, const PhaseIIPixelRawDataContainer &b) {
      auto collection_proxy_a = PhaseII::makeRawDataCollectionProxy(a);
      auto collection_proxy_b = PhaseII::makeRawDataCollectionProxy(b);

      using PixelRawDataContainerProxy = PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy;
      using PixelRawDataProxy = PhaseII::PixelRawDataTypeTraits<>::RawDataProxy;
      unsigned int n_non_empty_a=0;
      unsigned int n_non_empty_b=0;
      // count modules with non empty RDO list in b, to ensure that b does not contain
      // anything that a contains.
      for (PixelRawDataContainerProxy module_proxy_b : collection_proxy_b) {
         n_non_empty_b += (!module_proxy_b.empty());
      }
      // compare for modules with non-empty RDO list in a, the content of the RDOs
      for (PixelRawDataContainerProxy module_proxy_a : collection_proxy_a) {
         if (!module_proxy_a.empty()) {
            if (collection_proxy_b.empty()) return false;
            if (module_proxy_a.identifyHash() >= collection_proxy_b.size()) return false;

            PixelRawDataContainerProxy module_proxy_b=collection_proxy_b[module_proxy_a.identifyHash()];
            // consistency check
            if (module_proxy_a.identifyHash() != module_proxy_b.identifyHash()) return false;

            // compare RDO data
            unsigned int hit_i=0;
            for (PixelRawDataProxy hit_proxy_a : module_proxy_a) {
               if (hit_i >= module_proxy_b.size()) return false;
               PixelRawDataProxy hit_proxy_b=module_proxy_b[hit_i];
               if (   hit_proxy_a.coordinates() != hit_proxy_b.coordinates()
                   || hit_proxy_a.dataWord() != hit_proxy_b.dataWord()) {
                  return false;
               }
               ++hit_i;
            }
            ++n_non_empty_a;
         }
      }
      // the amount of non-empty content in a and b must be the same otherwise there is
      // content in b that is not in a.
      return n_non_empty_a == n_non_empty_b;
   }
}


template <typename T_Container, typename T_ContainerBase, typename T_Cnv, typename T_Helper>
int testRoundTrip(ISvcLocator*svcloc, const T_Helper &helper, std::span<const unsigned int> hits_per_module) {
   std::unique_ptr<T_Container> container = makeTestData<T_Container,T_Helper>(helper, hits_per_module);

   SmartIF<IAthenaPoolCnvSvc> cnv_svc{svcloc->service ("AthenaPoolCnvSvc")};

   CnvWrapper<T_Cnv,PixelRDO_Container_PERS, PhaseIIPixelRawDataContainer> cnv(svcloc);
   CHECK( cnv.initialize().isSuccess() );
   // create persistent data
   auto pers_ptr = cnv.createPersistentWithKey(container.get(), "key");
   using PERS_TYPE = std::remove_cvref_t<std::remove_pointer_t<decltype(pers_ptr)>>;

   // and make persistent data available to the dummy athena pool conversion service
   const std::type_info& ti = typeid(PERS_TYPE);
   RootType temp = RootType(ti);
   RootType cltype((temp) ? temp : RootType(pool::DbTypeInfo::typeName(ti)));
   Placement placement;
   placement.setContainerName(typeid(T_Container).name());
   auto token=std::unique_ptr<Token>(cnv_svc->registerForWrite(&placement,
                                                               pers_ptr,
                                                               cltype));

   // get the persistent data matching the token with the help of the
   // dummy athena pool conversion service, and convert to transient type.
   CLID clid =  ClassID_traits<T_Container>::ID();
   TokenAddress taddr (0, clid, "", "", 0, std::move(token));
   DataObject* pObj = nullptr;
   assert (cnv.createObj (&taddr, pObj).isSuccess());
   auto trans = std::unique_ptr<T_ContainerBase>(SG::Storable_cast<T_ContainerBase > (pObj));

   // the objects should be different.
   assert( container.get() != trans.get() );
   // the newly created transient object should not be null
   assert( trans.get() != nullptr);
   // and the content should agree with the original.
   assert( *container == *trans );

   return 0;
}
