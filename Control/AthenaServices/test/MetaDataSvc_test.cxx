/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaServices/MetaDataSvc_test.cxx
 * @author Shaun Roe
 * @date June 2023
 * @brief Unit tests for MetaDataSvc in the Boost framework
 *
 * Each test case creates its own MetaDataSvc instance with two
 * MetaDataToolStub tools (A and B), so that cases do not share state.
 * B never fails on a source ID, which shows whether MetaDataSvc goes on
 * calling the remaining tools after a failure.
 *
 * Every case checks the number of ERROR messages it produced. The ERROR
 * lines themselves are compared with share/MetaDataSvc_test.ref.
 *
 * Reading metadata from a real input file is covered by the
 * AthenaPoolExample tests, not here.
 */

#define BOOST_TEST_MODULE TEST_METADATASVC
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/MetaCont.h"
#include "Gaudi/Interfaces/IOptionsSvc.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/StoreGateSvc.h"
#include "TestTools/initGaudi.h"
#include "src/MetaDataSvc.h"
#include "src/OutputStreamSequencerSvc.h"
#include "MetaDataToolStub.h"
#include "SharedIOCnvSvcStub.h"

// Use the MetaDataSvc compiled into this test, not the one from the
// AthenaServices component library (which may come from the release).
DECLARE_COMPONENT( MetaDataSvc )
DECLARE_COMPONENT( OutputStreamSequencerSvc )
DECLARE_COMPONENT( MetaDataToolStub )
DECLARE_COMPONENT( SharedIOCnvSvcStub )

// Metadata types for the record/remove hooks and remapMetaContCLID
struct TestMeta {};
CLASS_DEF( TestMeta, 245100001, 1 )
METACONT_DEF( TestMeta, 245100002 );
struct TestMetaNoCont {};
CLASS_DEF( TestMetaNoCont, 245100003, 1 )

namespace tt = boost::test_tools;
using Calls = std::vector<std::string>;

struct TestFixture {
  TestFixture() :
    msgSvc( "MessageSvc", "MetaDataSvc_test" ),
    toolSvc( "ToolSvc", "MetaDataSvc_test" )
  {
    BOOST_REQUIRE( msgSvc.retrieve().isSuccess() );
    BOOST_REQUIRE( toolSvc.retrieve().isSuccess() );
  }

  struct Options {
    std::string container{};      ///< MetaDataContainer property
    std::string addressCreator{}; ///< ConversionService property
    bool failStopA{false};        ///< tool A fails metaDataStop
  };

  /// Create a fresh MetaDataSvc called @c name with tools @c name_A and @c name_B.
  ServiceHandle<MetaDataSvc> makeSvc( const std::string& name ) {
    return makeSvc( name, Options{} );
  }
  ServiceHandle<MetaDataSvc> makeSvc( const std::string& name, const Options& o ) {
    auto& opts = Gaudi::svcLocator()->getOptsSvc();
    opts.set( name + ".OutputLevel", "1" );
    opts.set( name + ".MetaDataTools",
              "['MetaDataToolStub/" + name + "_A', 'MetaDataToolStub/" + name + "_B']" );
    opts.set( "ToolSvc." + name + "_B.FailSourceID", "'never'" );
    if( o.failStopA ) opts.set( "ToolSvc." + name + "_A.FailMetaDataStop", "True" );
    if( !o.container.empty() ) opts.set( name + ".MetaDataContainer", "'" + o.container + "'" );
    if( !o.addressCreator.empty() ) opts.set( name + ".ConversionService", "'" + o.addressCreator + "'" );
    ServiceHandle<MetaDataSvc> svc( "MetaDataSvc/" + name, "MetaDataSvc_test" );
    BOOST_REQUIRE( svc.retrieve().isSuccess() );
    return svc;
  }

  /// Retire the input with a good source ID, so that finalize does not warn
  /// about a missing EndInputFile.
  void closeInput( MetaDataSvc& svc ) {
    const int nErr = errorCount();
    BOOST_TEST( svc.retireMetadataSource( FileIncident( "test", "EndInputFile", "dummy.txt", "closeGuid" ) ).isSuccess() );
    BOOST_TEST( errorCount() == nErr );
  }

  /// The two stub tools of the service created by makeSvc(name).
  std::pair<const MetaDataToolStub*, const MetaDataToolStub*> stubs( const std::string& name ) {
    return { stub( name + "_A" ), stub( name + "_B" ) };
  }

  /// Number of ERROR and FATAL messages reported so far in the job.
  int errorCount() const {
    return msgSvc->messageCount( MSG::ERROR ) + msgSvc->messageCount( MSG::FATAL );
  }

  ServiceHandle<IMessageSvc> msgSvc;
  ServiceHandle<IToolSvc> toolSvc;

private:
  const MetaDataToolStub* stub( const std::string& toolName ) {
    IAlgTool* tool = nullptr;
    BOOST_REQUIRE( toolSvc->retrieveTool( "MetaDataToolStub/" + toolName, tool, nullptr, false ).isSuccess() );
    const auto* s = dynamic_cast<const MetaDataToolStub*>( tool );
    BOOST_REQUIRE( s != nullptr );
    return s;
  }
};


BOOST_FIXTURE_TEST_SUITE(
  MetaDataSvcTest,
  TestFixture,
  * boost::unit_test::fixture<Athena_test::InitGaudi>(std::string("AthenaServices/MetaDataSvc_test.txt"))
)

BOOST_AUTO_TEST_CASE( stream_keys ) {
  auto svc = makeSvc( "MDS_keys" );
  const int nErr = errorCount();
  // removeStreamFromKey modifies its argument
  std::string key{"SomeTest__STREAM[ some other text]"};
  BOOST_TEST( svc->removeStreamFromKey( key ) == " some other text" );
  BOOST_TEST( key == "SomeTest" );
  std::string plain{"SomeTest"};
  BOOST_TEST( svc->removeStreamFromKey( plain ) == "" );
  BOOST_TEST( plain == "SomeTest" );
  // keys never recorded per stream map to themselves
  BOOST_TEST( ( svc->getPerStreamKeysFor( "SomeTest" ) == std::set<std::string>{"SomeTest"} ) );
  BOOST_TEST( errorCount() == nErr );
}

BOOST_AUTO_TEST_CASE( newMetadataSource_method ) {
  auto svc = makeSvc( "MDS_new" );
  auto [a, b] = stubs( "MDS_new" );

  // not a FileIncident: "Unable to get FileName"; no tool is called
  int nErr = errorCount();
  BOOST_TEST( svc->newMetadataSource( Incident( "test", "dummy" ) ).isFailure() );
  BOOST_TEST( errorCount() == nErr + 1 );
  BOOST_TEST( a->calls().empty() );

  // without MetaDataContainer, the file itself is not opened
  nErr = errorCount();
  BOOST_TEST( svc->newMetadataSource( FileIncident( "test", "BeginInputFile", "dummy.txt", "goodGuid" ) ).isSuccess() );
  BOOST_TEST( errorCount() == nErr );

  // tool A fails: "Unable to call beginInputFile"; tool B is still called
  nErr = errorCount();
  BOOST_TEST( svc->newMetadataSource( FileIncident( "test", "BeginInputFile", "dummy.txt", "badGuid" ) ).isFailure() );
  BOOST_TEST( errorCount() == nErr + 1 );

  BOOST_TEST( a->calls() == ( Calls{"begin:goodGuid", "begin:badGuid"} ), tt::per_element() );
  BOOST_TEST( b->calls() == ( Calls{"begin:goodGuid", "begin:badGuid"} ), tt::per_element() );
  closeInput( *svc );
}

BOOST_AUTO_TEST_CASE( newMetadataSource_bytestream ) {
  // with MetaDataContainer set, a BSF: file name skips the POOL lookup
  auto svc = makeSvc( "MDS_bsf", { .container = "MetaDataHdr" } );
  auto [a, b] = stubs( "MDS_bsf" );
  const int nErr = errorCount();
  BOOST_TEST( svc->newMetadataSource( FileIncident( "test", "BeginInputFile", "BSF:dummy.data", "bsGuid" ) ).isSuccess() );
  BOOST_TEST( errorCount() == nErr );
  BOOST_TEST( a->calls() == ( Calls{"begin:bsGuid"} ), tt::per_element() );
  closeInput( *svc );
}

BOOST_AUTO_TEST_CASE( newMetadataSource_missingFile ) {
  // with MetaDataContainer set, the input file has to exist: PoolSvc throws.
  // ROOT and APR log ERRORs on the way; their number is not under the
  // control of MetaDataSvc, so only require that there are some.
  auto svc = makeSvc( "MDS_nofile", { .container = "MetaDataHdr" } );
  auto [a, b] = stubs( "MDS_nofile" );
  const int nErr = errorCount();
  BOOST_CHECK_EXCEPTION( svc->newMetadataSource( FileIncident( "test", "BeginInputFile", "MetaDataSvc_test_missing.root", "goodGuid" ) ).ignore(),
                         std::runtime_error,
                         []( const std::runtime_error& e ) {
                           return std::string( e.what() ).find( "\"MetaDataSvc_test_missing.root\" is not existing" ) != std::string::npos;
                         } );
  BOOST_TEST( errorCount() > nErr );
  BOOST_TEST( a->calls().empty() );
  closeInput( *svc );
}

BOOST_AUTO_TEST_CASE( retireMetadataSource_method ) {
  auto svc = makeSvc( "MDS_retire" );
  auto [a, b] = stubs( "MDS_retire" );

  int nErr = errorCount();
  BOOST_TEST( svc->retireMetadataSource( Incident( "test", "dummy" ) ).isFailure() );
  BOOST_TEST( errorCount() == nErr + 1 );

  nErr = errorCount();
  BOOST_TEST( svc->retireMetadataSource( FileIncident( "test", "EndInputFile", "dummy.txt", "goodGuid" ) ).isSuccess() );
  BOOST_TEST( errorCount() == nErr );

  // tool A fails: ATH_CHECK returns at once, so (unlike newMetadataSource)
  // tool B is not called
  nErr = errorCount();
  BOOST_TEST( svc->retireMetadataSource( FileIncident( "test", "EndInputFile", "dummy.txt", "badGuid" ) ).isFailure() );
  BOOST_TEST( errorCount() == nErr + 1 );

  BOOST_TEST( a->calls() == ( Calls{"end:goodGuid", "end:badGuid"} ), tt::per_element() );
  BOOST_TEST( b->calls() == ( Calls{"end:goodGuid"} ), tt::per_element() );
}

BOOST_AUTO_TEST_CASE( handle_method ) {
  // called directly: firing through IncidentSvc would reach the services of
  // the other test cases as well
  auto svc = makeSvc( "MDS_handle" );
  auto [a, b] = stubs( "MDS_handle" );

  int nErr = errorCount();
  svc->handle( FileIncident( "test", "FirstInputFile", "dummy.txt", "firstGuid" ) );
  svc->handle( FileIncident( "test", "BeginInputFile", "dummy.txt", "guid1" ) );
  svc->handle( FileIncident( "test", "EndInputFile", "dummy.txt", "guid1" ) );
  svc->handle( FileIncident( "test", "BeginInputMemFile", "dummy.txt", "guid2" ) );
  svc->handle( FileIncident( "test", "EndInputMemFile", "dummy.txt", "guid2" ) );
  svc->handle( FileIncident( "test", "SomeOtherIncident", "dummy.txt", "guid3" ) );
  BOOST_TEST( errorCount() == nErr );
  BOOST_TEST( a->calls() == ( Calls{"begin:guid1", "end:guid1", "begin:guid2", "end:guid2"} ), tt::per_element() );
  BOOST_TEST( b->calls() == a->calls(), tt::per_element() );

  // failures are reported, not returned: inner ERROR + "Could not process/retire"
  nErr = errorCount();
  svc->handle( FileIncident( "test", "BeginInputFile", "dummy.txt", "badGuid" ) );
  BOOST_TEST( errorCount() == nErr + 2 );
  nErr = errorCount();
  svc->handle( FileIncident( "test", "EndInputFile", "dummy.txt", "badGuid" ) );
  BOOST_TEST( errorCount() == nErr + 2 );

  // not a FileIncident
  nErr = errorCount();
  svc->handle( Incident( "test", "BeginInputFile" ) );
  BOOST_TEST( errorCount() == nErr + 1 );
  closeInput( *svc );
}

BOOST_AUTO_TEST_CASE( prepareOutput_method ) {
  auto svc = makeSvc( "MDS_prep" );
  auto [a, b] = stubs( "MDS_prep" );
  const int nErr = errorCount();
  // per-stream version calls metaDataStop every time
  BOOST_TEST( svc->prepareOutput( "stream1" ).isSuccess() );
  BOOST_TEST( svc->prepareOutput( "stream2" ).isSuccess() );
  BOOST_TEST( a->calls() == ( Calls{"stop", "stop"} ), tt::per_element() );
  // serial version only once, until transitionMetaDataFile
  BOOST_TEST( svc->prepareOutput().isSuccess() );
  BOOST_TEST( svc->prepareOutput().isSuccess() );
  BOOST_TEST( a->calls() == ( Calls{"stop", "stop", "stop"} ), tt::per_element() );
  BOOST_TEST( svc->transitionMetaDataFile( "out.root", false ).isSuccess() );
  BOOST_TEST( svc->prepareOutput().isSuccess() );
  BOOST_TEST( a->calls() == ( Calls{"stop", "stop", "stop", "end:", "stop"} ), tt::per_element() );
  // io_reinit also resets it; an empty output name means the serial version
  BOOST_TEST( svc->io_reinit().isSuccess() );
  BOOST_TEST( svc->prepareOutput( "" ).isSuccess() );
  BOOST_TEST( svc->prepareOutput( "" ).isSuccess() );
  BOOST_TEST( a->calls() == ( Calls{"stop", "stop", "stop", "end:", "stop", "stop"} ), tt::per_element() );
  BOOST_TEST( b->calls() == a->calls(), tt::per_element() );
  BOOST_TEST( errorCount() == nErr );
}

BOOST_AUTO_TEST_CASE( prepareOutput_failure ) {
  // tool A fails metaDataStop: "Unable to call metaDataStop"; B is still called
  auto svc = makeSvc( "MDS_prepfail", { .failStopA = true } );
  auto [a, b] = stubs( "MDS_prepfail" );
  const int nErr = errorCount();
  BOOST_TEST( svc->prepareOutput( "stream1" ).isFailure() );
  BOOST_TEST( errorCount() == nErr + 1 );
  BOOST_TEST( b->calls() == ( Calls{"stop"} ), tt::per_element() );
}

BOOST_AUTO_TEST_CASE( shmProxy_method ) {
  // only AthenaPoolSharedIOCnvSvc is supported as AddressCreator
  auto svc = makeSvc( "MDS_shm" );
  const int nErr = errorCount();
  BOOST_TEST( svc->shmProxy( "dummy.txt" ).isFailure() );
  BOOST_TEST( errorCount() == nErr + 1 );
}

namespace {
  /// Token as sent by the SharedWriter client
  std::string shmToken( const std::string& key, const std::string& pname, unsigned long num ) {
    return "[DB=00000000-0000-0000-0000-000000000000][CNT=MetaDataHdr(" + key + ")]"
           "[FILE=out.root][CONT=MetaDataHdr(" + key + ")][PNAME=" + pname + "]"
           "[NUM=" + std::to_string( num ) + "]";
  }
}

BOOST_AUTO_TEST_CASE( shmProxy_addProxy ) {
  // SharedIOCnvSvcStub stands in for AthenaPoolSharedIOCnvSvc: shmProxy
  // parses the token, creates an address and loads the object into
  // InputMetaDataStore.
  // AthenaPoolConverter uses any service called AthenaPoolSharedIOCnvSvc as
  // its IAthenaPoolCnvSvc, so the stub must not outlive this test case.
  struct RemoveStubAtExit {
    ~RemoveStubAtExit() {
      SmartIF<ISvcManager> mgr( Gaudi::svcLocator() );
      BOOST_TEST( mgr->removeService( "AthenaPoolSharedIOCnvSvc" ).isSuccess() );
    }
  } removeStub;
  auto svc = makeSvc( "MDS_shmadd", { .addressCreator = "SharedIOCnvSvcStub/AthenaPoolSharedIOCnvSvc" } );
  ServiceHandle<SharedIOCnvSvcStub> cnv( "SharedIOCnvSvcStub/AthenaPoolSharedIOCnvSvc", "MetaDataSvc_test" );
  BOOST_REQUIRE( cnv.retrieve().isSuccess() );
  ServiceHandle<StoreGateSvc> inputStore( "StoreGateSvc/InputMetaDataStore", "MetaDataSvc_test" );
  BOOST_REQUIRE( inputStore.retrieve().isSuccess() );

  int nErr = errorCount();
  // persistent class known to ClassIDSvc by its type-info name
  BOOST_TEST( svc->shmProxy( shmToken( "StubKey", "SharedIOStubObject", 7 ) ).isSuccess() );
  // T/P naming convention: <transient>_p<N>
  BOOST_TEST( svc->shmProxy( shmToken( "StubKeyTP", "SharedIOStubObject_p3", 8 ) ).isSuccess() );
  // same key again: the existing proxy is reset and reloaded
  BOOST_TEST( svc->shmProxy( shmToken( "StubKey", "SharedIOStubObject", 9 ) ).isSuccess() );
  BOOST_TEST( errorCount() == nErr );

  BOOST_TEST( cnv->addresses() == ( Calls{"SHM|StubKey|SharedIOStubObject|7",
                                           "SHM|StubKeyTP|SharedIOStubObject_p3|8",
                                           "SHM|StubKey|SharedIOStubObject|9"} ), tt::per_element() );
  BOOST_TEST( cnv->created() == 3 );
  const SharedIOStubObject* obj = nullptr;
  BOOST_TEST( inputStore->retrieve( obj, "StubKey" ).isSuccess() );
  BOOST_TEST_REQUIRE( obj != nullptr );
  BOOST_TEST( obj->num == 9u );
  BOOST_TEST( inputStore->contains<SharedIOStubObject>( "StubKeyTP" ) );

  // FileMetaData objects get stream-unique keys, which AthenaOutputStream uses
  // to put them back into the right stream. The CLID comes from clid.db; the
  // xAOD class itself is not needed.
  nErr = errorCount();
  BOOST_TEST( svc->shmProxy( shmToken( "FileMetaData", "xAOD::FileMetaData_v1", 1 ) ).isSuccess() );
  BOOST_TEST( errorCount() == nErr );
  BOOST_TEST( ( svc->getPerStreamKeysFor( "FileMetaData" )
                == std::set<std::string>{"FileMetaData__STREAM[out.root]"} ) );
  BOOST_TEST( cnv->addresses().back() == "SHM|FileMetaData__STREAM[out.root]|xAOD::FileMetaData_v1|1" );

  // no CLID for the persistent class, neither directly nor via _p<N>:
  // inner ERROR + ATH_CHECK in shmProxy for each
  nErr = errorCount();
  const int nCreated = cnv->created();
  BOOST_TEST( svc->shmProxy( shmToken( "Other", "NoSuchClass", 1 ) ).isFailure() );
  BOOST_TEST( svc->shmProxy( shmToken( "Other", "NoSuchClass_p1", 1 ) ).isFailure() );
  BOOST_TEST( svc->shmProxy( shmToken( "Other", "SharedIOStubObject_pX", 1 ) ).isFailure() );
  BOOST_TEST( errorCount() == nErr + 6 );
  BOOST_TEST( cnv->created() == nCreated );
}

BOOST_AUTO_TEST_CASE( metaCont_hooks ) {
  // recordHook/removeHook decide which metadata classes are written out as MetaCont<T>
  auto svc = makeSvc( "MDS_hooks" );
  const CLID id = ClassID_traits<TestMeta>::ID();
  const CLID contId = ClassID_traits<MetaCont<TestMeta>>::ID();
  const CLID noContId = ClassID_traits<TestMetaNoCont>::ID();
  const int nErr = errorCount();

  BOOST_TEST( svc->remapMetaContCLID( id ) == id );
  svc->recordHook( typeid( TestMeta ) );
  BOOST_TEST( svc->remapMetaContCLID( id ) == contId );
  svc->removeHook( typeid( TestMeta ) );
  BOOST_TEST( svc->remapMetaContCLID( id ) == id );

  // handled, but there is no MetaCont<TestMetaNoCont>
  svc->recordHook( typeid( TestMetaNoCont ) );
  BOOST_TEST( svc->remapMetaContCLID( noContId ) == noContId );

  // type unknown to ClassIDSvc: ignored
  struct Unknown {};
  svc->recordHook( typeid( Unknown ) );
  svc->removeHook( typeid( Unknown ) );
  BOOST_TEST( errorCount() == nErr );
}

BOOST_AUTO_TEST_SUITE_END()
