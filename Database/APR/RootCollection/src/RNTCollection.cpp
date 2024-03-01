/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "RNTCollection.h"
#include "RNTCollectionQuery.h"
#include "RNTCollectionSchemaEditor.h"
#include "CollectionCommon.h"

#include "PersistentDataModel/Token.h"
#include "POOLCore/Exception.h"
#include "POOLCore/APRDefaults.h"

#include "CollectionBase/ICollectionColumn.h"
#include "CollectionBase/CollectionBaseNames.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/IService.h"

#include "TFile.h"
#include "TNetFile.h"
#include "TSocket.h"
#include "TMessage.h"
#include "TDirectory.h"

#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 0 )
#include "ROOT/RNTupleReader.hxx"
#else
#include "ROOT/RNTuple.hxx"
#endif
#include "ROOT/RNTupleModel.hxx"
using ROOT::Experimental::RNTupleModel;

#include "CxxUtils/starts_with.h"

#define corENDL coral::MessageStream::endmsg

#include <map>
#include <vector>
#include <ctype.h>

#include <iostream>


using namespace std;
using namespace pool::RootCollection;

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
     m_poolOut( "RNTCollection"),
     m_dataEditor( 0 ),
     m_fileMgr( 0 )
{
   RNTCollection::open();
}

     
RNTCollection::~RNTCollection()
{
   if( m_open ) RNTCollection::close();
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
 
      //m_schemaEditor->writeSchema();
   }
}

std::unique_ptr< RNTupleReader > RNTCollection::getCollectionRNTuple()
{
   if( m_file ) {
      std::string rntprefix { "RNT:" };  // MN: TODO: standarize RNTuple naming 
      auto reader = RNTupleReader::Open( rntprefix + APRDefaults::RNTupleNames::EventTag, m_fileName /* ,opts */ );
      if( reader )
         m_poolOut << coral::Debug << "Retrieved Collection RNTuple  \""
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 0 )
                   << reader->GetDescriptor().GetName() << "\" from file " << m_fileName
#endif            
                   << coral::MessageStream::endmsg;
      return reader;
   }
   return nullptr;
}


void RNTCollection::commit( bool )
{
   delayedFileOpen("commit");

   if( m_open ) {
      m_poolOut << coral::Debug
                << "Commit: saving collection TTree to file: " << ""
                << coral::MessageStream::endmsg;
      Long64_t bytes = -1;
      m_poolOut << "   bytes written " << (size_t)bytes
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
         // m_schemaEditor->writeSchema();
         // m_tree->Print();
         // m_file->Write( "0", TObject::kOverwrite );
      }
      cleanup();
   }
}

     
void RNTCollection::cleanup()
{
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
   //delete m_schemaEditor;   m_schemaEditor = 0;
   //delete m_dataEditor;   m_dataEditor = 0;
}       
       
     
void RNTCollection::open()  try
{
   const string myFileType = "RNTCollectionFile";

   if( m_open ) close();

   if( !m_fileCatalog
       && CxxUtils::starts_with (m_fileName, "PFN:")
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
      m_poolOut << coral::Info << "Opening Collection File " << m_fileName << " in mode: "
                << poolOptToRootOpt[m_mode] << coral::MessageStream::endmsg;
      bool fileExists = !gSystem->AccessPathName( m_fileName.c_str() );
      m_poolOut << coral::Debug << "File " << m_fileName
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
         if( m_fileMgr == 0 ) {
            IService *is(0);
            if (Gaudi::svcLocator()->getService("FileMgr",is,true).isFailure()) {
               m_poolOut << coral::Error 
                         << "unable to get the FileMgr, will not manage TFiles"
                         << coral::MessageStream::endmsg;
            } else {
               m_fileMgr = dynamic_cast<IFileMgr*>(is);
            }
         }
         if (m_fileMgr != 0 && m_fileMgr->hasHandler(Io::ROOT).isFailure()) {
            m_poolOut << coral::Info
                      << "Unable to locate ROOT file handler via FileMgr. "
                      << "Will use default TFile::Open"
                      << coral::MessageStream::endmsg;
            m_fileMgr = 0;
         }

         if (m_fileMgr == 0) {
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
         if (m_fileMgr == 0) {
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
      m_schemaEditor = make_unique<RNTCollectionSchemaEditor>(*this, m_description, *m_reader.get());
      m_schemaEditor->readSchema();
   }

   if (m_mode == ICollection::CREATE || m_mode == ICollection::CREATE_AND_OVERWRITE) {
      // create a new TTree
      if (0 && m_mode == ICollection::CREATE_AND_OVERWRITE) {
         m_poolOut << coral::Warning
                   << "Cleaning previous collection object from the file..."
                   << coral::MessageStream::endmsg;
         std::string rntupleName = std::string(APRDefaults::RNTupleNames::EventTag) + ";*";
         m_file->Delete(rntupleName.c_str());
      }
      //m_tree = new TTree(APRDefaults::TTreeNames::EventTag, m_name.c_str());
      m_poolOut << coral::Debug
                << "Created Collection TTree. Collection file will be "
                << m_fileName << coral::MessageStream::endmsg;
      //m_schemaEditor = new RNTCollectionSchemaEditor(*this, m_description, m_tree);
      //m_schemaEditor->createTreeBranches();
   }

   //m_dataEditor = new RNTCollectionDataEditor(m_description, m_tree, m_poolOut);
   m_poolOut << coral::Info
             << "Root collection opened, size = " << m_reader->GetNEntries()
             << corENDL;
      
   if (m_session && m_mode == ICollection::UPDATE) {
      int n(0);
      if (m_fileMgr == 0) {
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

bool RNTCollection::isOpen() const{
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

     
string RNTCollection::retrievePFN() const {
   if (m_name.substr (0, 4) != "PFN:")
      throw pool::Exception( "In CREATE mode a PFN has to be provided",
                             "RNTCollection::open", 
                             "RNTCollection");
   return m_name.substr(4,string::npos);
}

     
string  RNTCollection::retrieveFID() {

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

     
pool::ICollectionSchemaEditor& RNTCollection::schemaEditor()
{
   if ( m_mode == ICollection::READ )   {
      std::string errorMsg = "Cannot modify the schema of a collection in READ open mode.";
      throw pool::Exception( errorMsg,
                             "RNTCollection::schemaEditor",
                             "RNTCollection" );
   } 
   return *m_schemaEditor.get(); 
}

     
pool::ICollectionDataEditor& RNTCollection::dataEditor()
{
   if( m_mode == ICollection::READ ) {
      throw pool::Exception( "Cannot modify the data of a collection in READ open mode.", "RNTCollection::dataEditor", "RNTCollection" );
   }
   return *m_dataEditor;
}

     
pool::ICollectionQuery* RNTCollection::newQuery()
{
   if( !isOpen() ) {
      throw pool::Exception( "Attempt to query a closed collection.", "RNTCollection::dataEditor", "RNTCollection" );
   }
   return new RNTCollectionQuery( m_description, m_reader.get() );
}

