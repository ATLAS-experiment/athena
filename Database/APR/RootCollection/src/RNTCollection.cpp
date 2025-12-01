/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RNTCollection.h"
#include "RNTCollectionCursor.h"
#include "CollectionCommon.h"

#include "PersistentDataModel/Token.h"
#include "RootUtils/APRDefaults.h"

#include "CollectionBase/ICollectionColumn.h"
#include "CollectionBase/CollectionBaseNames.h"
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
using namespace pool::CollectionBaseNames;

RNTCollection::RNTCollection(
   const pool::ICollectionDescription* description,
   pool::ICollection::OpenMode mode,
   pool::ISession* )
   : APRMessaging("RNTCollection"),
     m_description( *description ),
     m_name( description->name() ),
     m_fileName( description->name() + ".root" ),
     m_mode( mode ),
     m_file( 0 ),
     m_session( 0 ),
     m_open( false )
{
   RNTCollection::open();
}

     
RNTCollection::~RNTCollection()
{
   if( m_open ) try {
      RNTCollection::close();
   } catch( std::exception& exception ) {
      ATH_MSG_ERROR( exception.what() );
      cleanup();
   }
   else cleanup();
}


void  RNTCollection::delayedFileOpen( const std::string& method )
{
   if( m_open && !m_file && m_session && m_mode != ICollection::READ ) {
      m_file = TFile::Open(m_fileName.c_str(), poolOptToRootOpt[m_mode] );
      if(!m_file || m_file->IsZombie()) {
         throw std::runtime_error( string("ROOT cannot \"") + poolOptToRootOpt[m_mode] + "\" file " + m_fileName + " (APR: \" RNTCollection::" + method +" \" from \" RNTCollection \")");
      }
   ATH_MSG_INFO( "File " << m_fileName << " opened in " << method );
// (Write Schema)
   }
}

std::unique_ptr< ROOT::RNTupleReader > RNTCollection::getCollectionRNTuple()
{
   if( m_file ) {
      auto reader = ROOT::RNTupleReader::Open( APRDefaults::RNTupleNames::EventTag, m_fileName );
      if( reader )
         ATH_MSG_DEBUG( "Retrieved Collection RNTuple  '" << reader->GetDescriptor().GetName() << "' from file " << m_fileName );
      return reader;
   }
   return nullptr;
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
   delayedFileOpen("commit");

   if( m_open ) {
   ATH_MSG_DEBUG( "Commit: saving collection to file: " << "" );
   }
}

     
void RNTCollection::close()
{
   ATH_MSG_INFO( "Closing " << (m_open? "open":"not open") << " collection '" << m_fileName << "'" );
   if(m_open) {
      delayedFileOpen("close");
              
      if( m_mode == ICollection::CREATE || m_mode == ICollection::CREATE_AND_OVERWRITE ) {
         m_mode = ICollection::UPDATE;
      }
      if( m_mode != ICollection::READ ) {
         // Write Schema?  MN: not sure
      }
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
   const string myFileType = "RNTCollectionFile";

   if( m_open ) close();

   if(  m_fileName.starts_with( "PFN:") && m_description.connection().empty() )
   {
      // special case with no catalog and PFN specified
      // create the collection with exactly PFN file name
      m_fileName = m_description.name().substr(4);   // remove the PFN: prefix
   }

   TDirectory::TContext dirctxt;
   if( m_session == 0 || m_mode == ICollection::READ || m_mode == ICollection::UPDATE ) {
      // first step: Try to open the file
      ATH_MSG_INFO( "Opening Collection File '" << m_fileName << "' in mode: " << poolOptToRootOpt[m_mode] );
      bool fileExists = !gSystem->AccessPathName( m_fileName.c_str() );
       ATH_MSG_DEBUG( "File '" << m_fileName << "'" << (fileExists? " exists." : " does not exist." ) );
      // open the file if it exists, or create if requested
      if( !fileExists && m_mode != ICollection::CREATE && m_mode != ICollection::CREATE_AND_OVERWRITE )
         m_file = 0;
      else {
         const char* root_mode = poolOptToRootOpt[m_mode];
         Io::IoFlags io_mode = poolOptToFileMgrOpt[m_mode];

         if( fileExists && (m_mode == ICollection::CREATE
                            || m_mode == ICollection::CREATE_AND_OVERWRITE ) ) {
            // creating collection in an existing file
            root_mode = "UPDATE";
            io_mode = (Io::WRITE | Io::APPEND);
         }
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
      }
      if (!m_file || m_file->IsZombie()) {
         throw std::runtime_error(  string("ROOT cannot \"") + poolOptToRootOpt[m_mode] + "\" file " + m_fileName + " (APR: \" RNTCollection::open \" from \" RNTCollection \")");
      }
   ATH_MSG_INFO( "File " << m_fileName << " opened" );
   }

   if (m_mode == ICollection::READ || m_mode == ICollection::UPDATE) {
      // retrieve RNTuple from file
      m_reader = getCollectionRNTuple();

      if (!m_reader) {
         int n(0);
         if (!m_fileMgr) {
            m_file->Close();
         } else {
            n = m_fileMgr->close(m_file, "RNTCollection");
         }
         if (n == 0)
            delete m_file;
         m_file = 0;
         throw std::runtime_error(  string("RNTuple Collection not found in file ") + m_fileName + " (APR: \" RNTCollection::open \" from \" RNTCollection \")");
      }
      // Read Schema 
      CollectionDescription desc( m_description.name(),
                                  m_description.type(),
                                  m_description.connection() );
      // clear the description
      m_description = std::move(desc);
      bool      foundToken = false;
   
      const auto& rntdesc = m_reader->GetDescriptor();
      for( const auto &f : rntdesc.GetTopLevelFields() ) {
         const std::string field_name = f.GetFieldName();
         // ignore the index column, it's not a user data
         if( field_name == APRDefaults::IndexColName )
            continue;
         std::string field_type = f.GetTypeName();
   
         ATH_MSG_DEBUG( "  + field name: " << field_name );
         ATH_MSG_DEBUG( "    field type: " << field_type );
   
         // MN: TODO : may need to fix coral::Attribute to recognize the "new" typenames
         static const std::map< std::string, std::string > typenameConv = {
            { "std::string", "string" },
            { "std::uint64_t", "unsigned long" },
            { "std::uint32_t", "unsigned int" },
            { "std::uint16_t", "unsigned short" },
            { "std::int64_t", "long" },
            { "std::int32_t", "int" },
            { "std::int16_t", "short" } };
         auto it = typenameConv.find( field_type );
         if( it != typenameConv.end() ) {
            ATH_MSG_DEBUG( "Replaced type  " << field_type << " with " << it->second );
            field_type = it->second;
         }
   
         if( (field_name == defaultEventReferenceColumnName || field_name == m_description.eventReferenceColumnName())
             and foundToken ) {
           throw std::runtime_error(  "can't reconstruct Description if more than one Token column (APR: \" RNTCollection::open \" from \" RNTCollection \")");
         }
         if( field_name ==  m_description.eventReferenceColumnName() ) {
            foundToken = true;
            // do nothing more
         } else if( field_name == defaultEventReferenceColumnName ) {
            m_description.setEventReferenceColumnName( field_name );
            foundToken = true;
         } else {
            m_description.insertColumn( field_name, field_type );
         }
      }
      if( !foundToken ) {
         m_description.setEventReferenceColumnName( "DummyRef" );
      }
   }

   if( m_mode == ICollection::UPDATE || m_mode == ICollection::CREATE || m_mode == ICollection::CREATE_AND_OVERWRITE ) {
      // create a new Collection
      std::string rntupleName = std::string(APRDefaults::RNTupleNames::EventTag);
      if( m_mode == ICollection::CREATE_AND_OVERWRITE ) {
         ATH_MSG_DEBUG( "Creating collection in overwrite mode..." );
         m_file->Delete( (rntupleName+";*").c_str() );
      }
      // (Create Schema)
      auto model { ROOT::RNTupleModel::Create() };
      model->SetDescription( rntupleName );
      for( int col_id = 0; col_id < m_description.numberOfTokenColumns(); col_id++ ) {
         std::string columnName = m_description.tokenColumn(col_id).name();
         addField( model.get(), columnName, CollectionBaseNames::tokenTypeName );
      }
      for( int col_id = 0; col_id < m_description.numberOfAttributeColumns(); col_id++ ) {
         const ICollectionColumn& column = m_description.attributeColumn(col_id);
         addField( model.get(), column.name(), column.type() );
      }

      ROOT::RNTupleWriteOptions opts;
      opts.SetCompression( m_file->GetCompressionSettings() );
      opts.SetUseBufferedWrite( true );
      // MN: TODO : add support for OVERWRITE?
      m_rntupleWriter = ROOT::RNTupleWriter::Append(std::move(model), rntupleName, *m_file, opts);

   ATH_MSG_DEBUG( "Created RNTCollection, collection file will be " << m_fileName );

   ATH_MSG_INFO( "RNTuple Collection created" );
   }
   else {
   ATH_MSG_INFO( "RNTuple Collection opened, size = " << m_reader->GetNEntries() );
   }
      
   if (m_session && m_mode == ICollection::UPDATE) {
      int n(0);
      if (!m_fileMgr) {
         m_file->Close();
      } else {
         n = m_fileMgr->close(m_file, "RNTCollection");
      }

      if(n == 0) delete m_file;
      m_file = 0;
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


bool RNTCollection::isOpen() const
{
   return m_open;
}

     
pool::ICollection::OpenMode RNTCollection::openMode() const
{
   return m_mode;
}

     
const pool::ICollectionDescription& RNTCollection::description() const
{
   return m_description;
}

pool::ICollectionCursor& RNTCollection::cursor()
{
   if( !isOpen() ) {
      throw std::runtime_error(  "Attempt to get cursor for a closed collection. (APR: \" RNTCollection::cursor \" from \" RNTCollection \")");
   }
   pool::TokenList m_outputTokenList;
   coral::AttributeList m_outputAttributeList;
   for( int j = 0; j < m_description.numberOfAttributeColumns(); j++ )    {
      m_outputAttributeList.extend( m_description.attributeColumn( j ).name() , m_description.attributeColumn( j ).type() );
   }
   for( int j = 0; j < m_description.numberOfTokenColumns(); j++ )    {
      m_outputTokenList.extend( m_description.tokenColumn( j ).name() );
   }

   pool::CollectionRowBuffer collectionRowBuffer( m_outputTokenList, m_outputAttributeList );

   ICollectionCursor* m_cursor = new RNTCollectionCursor( m_description, collectionRowBuffer, m_reader.get() );
   return *m_cursor;
}
