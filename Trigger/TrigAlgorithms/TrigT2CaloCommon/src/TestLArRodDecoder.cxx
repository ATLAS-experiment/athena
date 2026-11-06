
#include "TestLArRodDecoder.h"

TestLArRodDecoder::TestLArRodDecoder( const std::string& name, ISvcLocator* pSvcLocator ) : ::AthAlgorithm( name, pSvcLocator ){
   std::cout << "Constructor" << name << std::endl;
}

TestLArRodDecoder::~TestLArRodDecoder(){}

StatusCode
TestLArRodDecoder::initialize( ){

   ATH_CHECK( m_larDecoder.retrieve() );
   ATH_CHECK( m_robDataProvider.retrieve() );
   ATH_CHECK( m_onOffIdMappingKey.initialize() );
   ATH_CHECK( m_larRoIMapKey.initialize() );
   ATH_CHECK( m_febRodMappingKey.initialize() );
   ATH_CHECK( m_mcsymKey.initialize() );
   ATH_CHECK( m_bcContKey.initialize() );
   ATH_CHECK( m_caloMgrKey.initialize());
   std::cout << "Initializing " << name() << std::endl;
   return StatusCode::SUCCESS;
}

void TestLArRodDecoder::lateInit(const EventContext& context){

for (EventCache& cache : m_testSlot) {
    cache.count=0;
    cache.larContainer = std::make_unique<LArCellCont>();
    SG::ReadCondHandle<LArMCSym> mcsym (m_mcsymKey, context);
    SG::ReadCondHandle<LArFebRodMapping> febrod(m_febRodMappingKey, context);
    SG::ReadCondHandle<LArBadChannelCont> larBadChan{ m_bcContKey, context };
    SG::ReadCondHandle<LArOnOffIdMapping> onoff ( m_onOffIdMappingKey, context);
    SG::ReadCondHandle<LArRoIMap> roimap ( m_larRoIMapKey, context);
    SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey, context};
    const CaloDetDescrManager* theCaloDDM = *caloMgrHandle;
    cache.larContainer->initialize(**roimap, **onoff, **mcsym, **febrod, **larBadChan, *theCaloDDM).orThrow("Cannot initialize LAr container", name());
}
}

StatusCode
TestLArRodDecoder::finalize( ){
   std::cout << "Finalizing " << name() << std::endl;
   //ATH_CHECK( m_testSlot->larContainer->finalize() );
   return StatusCode::SUCCESS;
}

StatusCode
TestLArRodDecoder::execute( const EventContext& context) {
   std::call_once(m_lateInitFlag, &TestLArRodDecoder::lateInit, this,context);
   if ( m_printOut ) std::cout << "Executing " << name() << " " << m_testSlot.get(context)->count << std::endl;
   m_testSlot.get(context)->count=m_testSlot.get(context)->count+1;
   m_testSlot.get(context)->larContainer->eventNumber( context.evt() );
   LArCellCont* larcell = m_testSlot.get(context)->larContainer.get();

   std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> robFrags;
   m_robDataProvider->getROBData( context, m_vrodid32fullDet, robFrags );
   if ( m_printOut ) std::cout << "robFrags.size() : " << robFrags.size() << std::endl;
   if (robFrags.size() < 1 ){ // this is the end
       return StatusCode::SUCCESS;
   }
   std::vector<LArCellCollection*> coll_of_coll;
   struct timeval t1, t2;
   int nunpacked=0;
   gettimeofday(&t1, NULL);
   for( auto rob : robFrags ){
      if ( m_printOut ) std::cout << "Interpreting fragment" << std::endl;
      // Get Rod Data and size of fragment
      const uint32_t* roddata = 0;
      rob->rod_data( roddata );
      const size_t roddatasize = rob->rod_ndata();
      if ( m_printOut ) { std::cout << "roddatasize : " << roddatasize << std::endl;
         if (roddatasize > 30){
            for(size_t i=0;i<30;i++){
	       std::cout << "0x" << std::hex << roddata[i] << " " << std::dec << roddata[i] << std::endl;
            } //end of for
         } // end of rodddatasize
      } // end of print out if
      uint32_t sourceID = rob->source_id();
      const auto it = larcell->find( sourceID );
      m_larDecoder->setsecfeb( larcell->findsec( sourceID ) );
      if ( it != larcell->end() && ( *it )->size() != 0 ) { 
         LArCellCollection* coll = *it; 
         if ( m_printOut ) std::cout <<  "ROB of ID 0x" << std::hex << sourceID << std::dec << " to be decoded" << std::endl;
         if ( !m_larDecoder->check_valid(rob,msg())) {std::cout << "Wrong Block" << std::endl;}
         else {
            LArRodBlockStructure* larRodBlockStructure_per_slot = nullptr;
            uint16_t rodMinorVersion = 0;
            uint32_t robBlockType = 0;
            m_larDecoder->fillCollectionHLT( *rob, roddata, roddatasize, *coll, larRodBlockStructure_per_slot, rodMinorVersion, robBlockType );
            coll_of_coll.push_back(coll);
            nunpacked++;
         } // enf of else
      } // end of if
   } // end of rob for
   gettimeofday(&t2, NULL);
   double delay = 1e6*(t2.tv_sec-t1.tv_sec) + (t2.tv_usec-t1.tv_usec);
   delay*=1e-3;
   std::cout << "TIME : " << delay << "; n unpacked : " << nunpacked << "; TIME/unpacked : " << delay/((double)nunpacked) << std::endl; 
   for( auto coll : coll_of_coll )
            for( const auto& cell : *coll ) 
                if ( cell && cell->caloDDE() ) std::cout << "Cell : 0x" << std::hex << cell->ID().get_identifier32().get_compact() << std::dec << " " <<  cell->energy() << " " << cell->eta() << " " << cell->phi() << " " << cell->caloDDE()->getSampling() << std::endl;

   return StatusCode::SUCCESS;
}

