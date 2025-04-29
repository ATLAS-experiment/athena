/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RNTCollection.h"
#include "RNTCollectionQuery.h"
#include "CollectionCommon.h"

#include "PersistentDataModel/Token.h"
#include "POOLCore/Exception.h"
#include "RootUtils/APRDefaults.h"

#include "CollectionBase/ICollectionColumn.h"
#include "CollectionBase/CollectionBaseNames.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IFileMgr.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#define corENDL coral::MessageStream::endmsg

#include "TFile.h"
#include "TDirectory.h"
#include "TSystem.h"

#include "ROOT/RNTuple.hxx"
#include "ROOT/RNTupleReader.hxx"
#include "ROOT/RNTupleWriter.hxx"
#include "ROOT/RNTupleWriteOptions.hxx"

#include <map>

using namespace std;
using namespace pool::RootCollection;
using namespace pool::CollectionBaseNames;

#if ROOT_VERSION_CODE < ROOT_VERSION( 6, 35, 0 )
namespace ROOT {
   using RFieldBase = ROOT::Experimental::RFieldBase;
   using RNTupleWriteOptions = ROOT::Experimental::RNTupleWriteOptions;
}
#endif

RNTCollection::RNTCollection(
   const pool::ICollectionDescription* description,
   pool::ICollection::OpenMode mode,
   pool::ISession* )
   : m_description( *description ),
     m_name( description->name() ),
     m_fileName( description->name() + ".root" ),
     m_mode( mode ),
     m_file( 0 ),
     m_session( 0 ),
     m_open( false ),
     m_readOnly( mode == ICollection::READ ? true : false ),
     m_poolOut( "RNTCollection")
{
   RNTCollection::open();
}

     
RNTCollection::~RNTCollection()
{
   if( m_open ) try {
      RNTCollection::close();
   } catch( std::exception& exception ) {
      m_poolOut << coral::Error << exception.what() << corENDL;
      cleanup();
   }
   else cleanup();
}


void  RNTCollection::delayedFileOpen( const std::string& method )
{
   if( m_open && !m_file && m_session && m_mode != ICollection::READ ) {
      m_file = TFile::Open(m_fileName.c_str(), poolOptToRootOpt[m_mode] );
      if(!m_file || m_file->IsZombie()) {
         throw pool::Exception( string("ROOT cannot \"") + poolOptToRootOpt[m_mode] + "\" file " + m_fileName,
                                std::string("RNTCollection::") + method,
                                "RNTCollection" );
      }
      m_poolOut << coral::Info << "File " << m_fileName << " opened in " << method <<  coral::MessageStream::endmsg;
// (Write Schema)
   }
}

std::unique_ptr< ROOT::RNTupleReader > RNTCollection::getCollectionRNTuple()
{
   if( m_file ) {
      auto reader = ROOT::RNTupleReader::Open( APRDefaults::RNTupleNames::EventTag, m_fileName );
      if( reader )
         m_poolOut << coral::Debug << "Retrieved Collection RNTuple  \""
                   << reader->GetDescriptor().GetName() << "\" from file " << m_fileName
                   << coral::MessageStream::endmsg;
      return reader;
   }
   return nullptr;
}


void RNTCollection::insertRow( const pool::CollectionRowBuffer& inputRowBuffer )
{
   if( m_mode == pool::ICollection::READ ) {
      throw pool::Exception( "Cannot modify data of a collection in READ open mode.", "RNTCollection::insertRow", MODULE_NAME );
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
      throw pool::Exception( "Fill() failed", "RNTCollection::insertRow", MODULE_NAME );
}


void RNTCollection::commit( bool )
{
   delayedFileOpen("commit");

   if( m_open ) {
      m_poolOut << coral::Debug
                << "Commit: saving collection to file: " << ""
                << coral::MessageStream::endmsg;
   }
}

     
void RNTCollection::close()
{
   m_poolOut << coral::Info << "Closing " << (m_open? "open":"not open")
             << " collection '" << m_fileName << "'" << coral::MessageStream::endmsg;
   if(m_open) {
      delayedFileOpen("close");
              
      if( m_mode == ICollection::CREATE || m_mode == ICollection::CREATE_AND_OVERWRITE ) {
         if( true )
            m_mode = ICollection::UPDATE;
         else {
            // unregister if the collection was not created
            m_mode = ICollection::CREATE_AND_OVERWRITE;
            if(m_fileCatalog){
               m_fileCatalog->start();
               m_fileCatalog->deleteFID( m_fileCatalog->lookupPFN(m_fileName) );
               m_fileCatalog->commit();
            }
         }
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

   if( !m_fileCatalog
       && m_fileName.starts_with( "PFN:")
       && m_description.connection().empty() )
   {
      // special case with no catalog and PFN specified
      // create the collection with exactly PFN file name
      m_fileName = m_description.name().substr(4);   // remove the PFN: prefix
   }
   else if( fileCatalogRequired() ) {
      m_fileName = "";

      if(!m_fileCatalog)
         m_fileCatalog = make_unique<pool::IFileCatalog>();
        
      if( m_mode == ICollection::CREATE ){
         string fid = retrieveFID();
         if(fid!="")
            throw pool::Exception( "Cannot CREATE already registered collections",
                                   "RNTCollection::open", 
                                   "RNTCollection");
         else{
            m_fileName = retrievePFN();
            FileCatalog::FileID dummy;
            m_fileCatalog->start();
            m_fileCatalog->registerPFN( m_fileName, myFileType, dummy);
            m_fileCatalog->commit();
         }
      }

      else if(m_mode == ICollection::CREATE_AND_OVERWRITE){
         string fid = retrieveFID();
         if(fid!="")
            m_fileName = retrieveUniquePFN(fid);
         else{
            m_fileName = retrievePFN();
            FileCatalog::FileID dummy;
            m_fileCatalog->start();
            m_fileCatalog->registerPFN( m_fileName, myFileType, dummy);
            m_fileCatalog->commit();
         }
      }

      else if(m_mode == ICollection::UPDATE){
         string fid = retrieveFID();
         if(fid!="")
            m_fileName = retrieveUniquePFN(fid);
         else
            throw pool::Exception( "Cannot UPDATE non registered collections",
                                   "RNTCollection::open", 
                                   "RNTCollection");
      }

      else if(m_mode == ICollection::READ) {
         string fid = retrieveFID();
         if(fid!="") {
            string dummy;
            m_fileCatalog->start();
            m_fileCatalog->getFirstPFN(fid, dummy, dummy);
            m_fileCatalog->commit();
         }else
            throw pool::Exception( "Cannot READ non registered collections",
                                   "RNTCollection::open", 
                                   "RNTCollection");
      }
   }

   TDirectory::TContext dirctxt;
   if( m_session == 0 || m_mode == ICollection::READ || m_mode == ICollection::UPDATE ) {
      // first step: Try to open the file
      m_poolOut << coral::Info << "Opening Collection File '" << m_fileName << "' in mode: "
                << poolOptToRootOpt[m_mode] << coral::MessageStream::endmsg;
      bool fileExists = !gSystem->AccessPathName( m_fileName.c_str() );
      m_poolOut << coral::Debug << "File '" << m_fileName << "'"
                << (fileExists? " exists." : " does not exist." ) << corENDL;
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
               m_poolOut << coral::Error 
                         << "unable to get the FileMgr, will not manage TFiles"
                         << coral::MessageStream::endmsg;
            }
         }
         if (m_fileMgr && m_fileMgr->hasHandler(Io::ROOT).isFailure()) {
            m_poolOut << coral::Info
                      << "Unable to locate ROOT file handler via FileMgr. "
                      << "Will use default TFile::Open"
                      << coral::MessageStream::endmsg;
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
               m_poolOut << coral::Error << "unable to open \"" << m_fileName
                         << "\" for " << root_mode
                         << coral::MessageStream::endmsg;
            } else {
               m_file = (TFile*)vf;
            }
         }
      }
      if (!m_file || m_file->IsZombie()) {
         throw pool::Exception(string("ROOT cannot \"") +
                               poolOptToRootOpt[m_mode] + "\" file " +
                               m_fileName,
                               "RNTCollection::open", "RNTCollection");
      }
      m_poolOut << coral::Info << "File " << m_fileName << " opened"
                << coral::MessageStream::endmsg;
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
         throw pool::Exception(
            string("RNTuple Collection not found in file ") + m_fileName,
            "RNTCollection::open", "RNTCollection");
      }
      // Read Schema 
      CollectionDescription desc( m_description.name(),
                                  m_description.type(),
                                  m_description.connection() );
      // clear the description
      m_description = desc;
      bool      foundToken = false;
   
      const auto& rntdesc = m_reader->GetDescriptor();
      for( const auto &f : rntdesc.GetTopLevelFields() ) {
         const std::string field_name = f.GetFieldName();
         // ignore the index column, it's not a user data
         if( field_name == APRDefaults::IndexColName )
            continue;
         std::string field_type = f.GetTypeName();
   
         m_poolOut << coral::Debug << "  + field name: " << field_name <<  corENDL;
         m_poolOut << coral::Debug << "    field type: " << field_type <<  corENDL;
   
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
            m_poolOut << coral::Debug << "Replaced type  " << field_type << " with " << it->second << corENDL;
            field_type = it->second;
         }
   
         if( (field_name == defaultEventReferenceColumnName || field_name == m_description.eventReferenceColumnName())
             and foundToken ) {
            throw pool::Exception( "can't reconstruct Description if more than one Token column",
                                   "pool::RNTCollection::readSchema",
                                   "RNTCollection" );
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
         m_poolOut << coral::Warning
                   << "Cleaning previous collection object from the file..."
                   << coral::MessageStream::endmsg;
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

      m_poolOut << coral::Debug
                << "Created RNTCollection, collection file will be "
                << m_fileName << coral::MessageStream::endmsg;

      m_poolOut << coral::Info << "RNTuple Collection created" << corENDL;
   }
   else {
      m_poolOut << coral::Info
                << "RNTuple Collection opened, size = " << m_reader->GetNEntries()
                << corENDL;
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
   m_poolOut << coral::Debug << "Open() failed with expception: " << e.what()
             << corENDL;
   cleanup();
   throw;
}


void RNTCollection::addField(ROOT::RNTupleModel* model, const std::string& field_name, const std::string& field_type)
{
   m_poolOut << coral::Debug << "Adding new column: name=" << field_name
             << " of type " << field_type << corENDL;
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

     
bool RNTCollection::fileCatalogRequired() const
{
   return m_name.find("PFN:")==0 || 
      m_name.find("FID:")==0 || 
      m_name.find("LFN:")==0; 
}

     
string RNTCollection::retrievePFN() const
{
   if (m_name.substr (0, 4) != "PFN:")
      throw pool::Exception( "In CREATE mode a PFN has to be provided",
                             "RNTCollection::open", 
                             "RNTCollection");
   return m_name.substr(4,string::npos);
}

     
string  RNTCollection::retrieveFID()
{
   FileCatalog::FileID fid="";
   string fileType="";        

   if (m_name.substr (0, 4) == "PFN:") {
      string pfn = m_name.substr(4,string::npos);
      m_fileCatalog->start();
      m_fileCatalog->lookupFileByPFN(pfn,fid,fileType);
      m_fileCatalog->commit();
   }
   else if (m_name.substr (0, 4) == "LFN:") {
      string lfn = m_name.substr(4,string::npos);
      m_fileCatalog->start();
      m_fileCatalog->lookupFileByLFN(lfn,fid);
      m_fileCatalog->commit();
   }
   else if (m_name.substr (0, 4) == "FID:") {
      fid = m_name.substr(4,string::npos);
   }else
      throw pool::Exception( "A FID, PFN or and LFN has to be provided",
                             "RNTCollection::retrieveFID", 
                             "RNTCollection");
   return fid;
}


string RNTCollection::retrieveUniquePFN(const FileCatalog::FileID& fid)
{
   IFileCatalog::Files       pfns;
   m_fileCatalog->start();
   m_fileCatalog->getPFNs(fid, pfns);
   m_fileCatalog->commit();
   if( pfns.empty() )
      throw pool::Exception( "This exception should never have been thrown, please send a bug report",
                             "RNTCollection::retrieveUniquePFN", 
                             "RNTCollection"); 
   if( pfns.size() > 1 )
      throw pool::Exception( "Cannot UPDATE or CREATE_AND_OVERWRITE since there are replicas",
                             "RNTCollection::retrieveUniquePFN", 
                             "RNTCollection");
   return pfns[0].first;
}

   
const pool::ICollectionDescription& RNTCollection::description() const
{
   return m_description;
}

     
pool::ICollectionQuery* RNTCollection::newQuery()
{
   if( !isOpen() ) {
      throw pool::Exception( "Attempt to query a closed collection.", "RNTCollection::newQuery", "RNTCollection" );
   }
   return new RNTCollectionQuery( m_description, m_reader.get() );
}

