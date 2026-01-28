/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RNTCollection.h"
#include "RNTCollectionCursor.h"
#include "CollectionCommon.h"

#include "PersistentDataModel/Token.h"
#include "RootUtils/APRDefaults.h"

#include "CollectionSvc/ICollectionColumn.h"
#include "CollectionSvc/CollectionNames.h"
#include "POOLCore/SystemTools.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IFileMgr.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"

#include "TFile.h"
#include "TDirectory.h"
#include "TSystem.h"

#include "ROOT/RNTuple.hxx"
#include "ROOT/RNTupleReader.hxx"
#include "ROOT/RNTupleWriter.hxx"
#include "ROOT/RNTupleWriteOptions.hxx"

#include <exception>
#include <map>

using namespace std;
using namespace pool::RootCollection;
using namespace pool::CollectionNames;

RNTCollection::RNTCollection(
   const pool::ICollectionDescription* description,
   pool::ICollection::OpenMode mode,
   pool::ISession* )
   : APRMessaging("RNTCollection"),
     m_description( *description ),
     m_name( description->name() ),
     m_fileName( description->connection() ),
     m_mode( mode ),
     m_file( 0 ),
     m_session( 0 ),
     m_open( false ) {
   RNTCollection::open();
}


RNTCollection::~RNTCollection() {
   RNTCollection::close();
}


void RNTCollection::insertRow( const pool::CollectionRowBuffer& inputRowBuffer )
{
   if( m_mode == pool::ICollection::READ ) {
      throw std::runtime_error( std::string("Cannot modify data of a collection in READ open mode.") + " (APR: \" RNTCollection::insertRow \" from \" RNTCollection \")");
   }
   // MN: TODO: migrate to a const REntry API once ROOT delivers it.
   auto entry = m_rntupleWriter->GetModel().CreateBareEntry();
   std::deque<std::string> stringBuffer;
   for( pool::TokenList::const_iterator iToken = inputRowBuffer.tokenList().begin();
        iToken != inputRowBuffer.tokenList().end(); ++iToken )  {
      stringBuffer.push_back( iToken->toString() );
      entry->BindRawPtr( iToken.tokenName(), &stringBuffer.back() );
   }
   for( const coral::Attribute& att : inputRowBuffer.attributeList() ) {
      void* ptr ATLAS_THREAD_SAFE = const_cast<void*>(att.addressOfData());
      entry->BindRawPtr( att.specification().name(), ptr );
   }
   auto wbytes = m_rntupleWriter->Fill(*entry);
   if( wbytes <= 0 )
      throw std::runtime_error( std::string("Fill() failed.") + " (APR: \" RNTCollection::insertRow \" from \" RNTCollection \")");
}


void RNTCollection::commit( bool )
{
   if( m_open ) {
      ATH_MSG_DEBUG( "Commit: saving collection to file: " << "" );
   }
}


void RNTCollection::close()
{
   ATH_MSG_INFO( "Closing " << (m_open? "open":"not open") << " collection '" << m_fileName << "'" );
   if(m_open) {
      cleanup();
   }
}


void RNTCollection::cleanup()
{
   // delete RNTuple writer before closing the file (or else!)
   m_rntupleWriter.reset();
   if( m_file ) {
      int n = 0;
      if( m_fileMgr ) {
         n = m_fileMgr->close(m_file, "RNTCollection");
      } else {
         m_file->Close();
      }
      if( n==0 ) delete m_file;
      m_file = 0;
   }
   m_open = false;
}


void RNTCollection::open()  try
{
   if (m_mode == ICollection::READ) {
      throw std::runtime_error(  "Attempt to open RNTCollection, read via RootCollection. (APR: \" RNTCollection::open \" from \" RNTCollection \")");
   }

   if( m_fileName.starts_with( "PFN:") ) {
      m_fileName = m_fileName.substr(4);   // remove the PFN: prefix
   }

   TDirectory::TContext dirctxt;
   if( m_session == 0 ) {
      // first step: Try to open the file
      ATH_MSG_INFO( "Opening RNTCollection File '" << m_fileName << "' in mode: " << poolOptToRootOpt[m_mode] );
      const char* root_mode = poolOptToRootOpt[m_mode];
      Io::IoFlags io_mode = poolOptToFileMgrOpt[m_mode];
      if( !m_fileMgr ) {
         m_fileMgr = Gaudi::svcLocator()->service("FileMgr");
         if ( !m_fileMgr ) {
            ATH_MSG_ERROR( "unable to get the FileMgr, will not manage TFiles" );
         }
      }
      if (m_fileMgr && m_fileMgr->hasHandler(Io::ROOT).isFailure()) {
         ATH_MSG_INFO( "Unable to locate ROOT file handler via FileMgr. Will use default TFile::Open" );
         m_fileMgr.reset();
      }

      if (!m_fileMgr) {
         m_file = TFile::Open(m_fileName.c_str(), root_mode);
      } else {
         void* vf(0);
         // open in shared mode only for writing
         bool SHARED(false);
         if (io_mode.isWrite()) {
            SHARED = true;
         }
         int r = m_fileMgr->open(Io::ROOT, "RNTCollection", m_fileName,
                                 io_mode, vf, "TAG", SHARED);
         if (r < 0) {
            ATH_MSG_ERROR( "unable to open '" << m_fileName << "' for " << root_mode );
         } else {
            m_file = (TFile*)vf;
         }
      }
      if (!m_file || m_file->IsZombie()) {
         throw std::runtime_error(  string("ROOT cannot \"") + poolOptToRootOpt[m_mode] + "\" file " + m_fileName + " (APR: \" RNTCollection::open \" from \" RNTCollection \")");
      }
      ATH_MSG_INFO( "File " << m_fileName << " opened" );
   }

   if( m_mode == ICollection::CREATE_AND_OVERWRITE ) {
      // create a new Collection
      std::string rntupleName = std::string(APRDefaults::RNTupleNames::EventTag);
      ATH_MSG_DEBUG( "Creating collection in overwrite mode..." );
      m_file->Delete( (rntupleName+";*").c_str() );
      // (Create Schema)
      auto model { ROOT::RNTupleModel::Create() };
      model->SetDescription( rntupleName );
      for( int col_id = 0; col_id < m_description.numberOfTokenColumns(); col_id++ ) {
         std::string columnName = m_description.tokenColumn(col_id).name();
         addField( model.get(), columnName, CollectionNames::tokenTypeName );
      }
      for( int col_id = 0; col_id < m_description.numberOfAttributeColumns(); col_id++ ) {
         const ICollectionColumn& column = m_description.attributeColumn(col_id);
         addField( model.get(), column.name(), column.type() );
      }

      ROOT::RNTupleWriteOptions opts;
      opts.SetCompression( m_file->GetCompressionSettings() );
      opts.SetUseBufferedWrite( true );
      m_rntupleWriter = ROOT::RNTupleWriter::Append(std::move(model), rntupleName, *m_file, opts);
      ATH_MSG_DEBUG( "Created RNTCollection, collection file will be " << m_fileName );
      ATH_MSG_INFO( "RNTuple Collection created" );
   }

   m_open = true;

} catch (std::exception& e) {
   ATH_MSG_DEBUG( "Open() failed with exception: " << e.what() );
   cleanup();
   throw;
}


void RNTCollection::addField(ROOT::RNTupleModel* model, const std::string& field_name, const std::string& field_type)
{
   ATH_MSG_DEBUG( "Adding new column: name=" << field_name << " of type " << field_type );
   const std::string actual_type = (field_type == tokenTypeName? "std::string" : field_type);
   auto field = ROOT::RFieldBase::Create(field_name, actual_type).Unwrap();
   model->AddField( std::move(field) );
}


const pool::ICollectionDescription& RNTCollection::description() const {
   return m_description;
}

pool::ICollectionCursor& RNTCollection::cursor() {
   throw std::runtime_error(  "Attempt to get cursor for RNTCollection, read via RootCollection. (APR: \" RNTCollection::cursor \" from \" RNTCollection \")");
}
