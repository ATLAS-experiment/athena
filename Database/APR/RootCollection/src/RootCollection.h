/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROOTCOLLECTION_ROOTCOLLECTION_H
#define ROOTCOLLECTION_ROOTCOLLECTION_H

#include "CollectionBase/ICollection.h"
#include "CollectionBase/CollectionDescription.h"
#include "CollectionBase/CollectionRowBuffer.h"

#include "FileCatalog/IFileCatalog.h"
#include "CoralBase/MessageStream.h"

#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/SmartIF.h"
#include "Gaudi/PluginService.h"

#include <string>
#include <vector>
#include <memory>

class TTree;
class TFile;
class IFileMgr;


namespace pool {

   class ISession; 

   namespace RootCollection {

      class Attribute;
      class AttributeSpecification;
  
      /**
         @brief Collection (and CollectionProxy) implementation based on ROOT trees
  
         Implementation details: 
         - Token and meta data attributes are stored in a simple TTree 
         - Tokens are stored as (compressed) C-string
         - Each attribute is written to a separate branch of the TTree  
         - You can "play" with the collection tree in an interactive ROOT session 
         - The TTree is stored to the ROOT file: collectionName.root 
         - Pool FileCatalog can be utilized to locate/register the collection files
         (see: RootCollection::RootCollection)
         - Selection is performed using TTreePlayer 
         - Description of query syntax can be found at http://root.cern.ch/root/htmldoc/TTree.html#TTree:Draw
         - It is possible to read only subsets of the AttributeList 
         - This feature is controlled via the "option" parameter of the RootCollection::select method 
         - e.g. RootCollection::select( primaryQuery, secondaryQuery ,"ATTRIBUTE_LIST attribute23 attribute42") 
         - Support for remote collection access via rootd 
         - (experimental) 
         - Support for a simple server side selection 
         - (experimental) 
         . 
         . 
         ROOT documentation can be found at http://root.cern.ch/ 
      */
      class RootCollection :  public ICollection {
    
     public:
	typedef Gaudi::PluginService::Factory<ICollection*( const ICollectionDescription*, ICollection::OpenMode, ISession*)> Factory;
    
        /// Constructor
        /// @param session If you want to access the referenced objects you have to provide an ISession
        /// @param connection The location of the collection file is uniquely defined by the parameters name and connection
        /// @param name The location of the collection file is uniquely defined by the parameters name and connection
        /// @param mode The open mode of the collection
        ///
        /// - Without use of FileCatalog:
        ///   - The path to the collection file is simply created by the following concatenation:\n
        ///     connection+name+".root"
        ///   - name: Name of the collection file
        ///   - connection:
        ///     - It can be a relative or absolute path
        ///     - In case of an empty connection string it is assumed that the file is located in the current directory
        ///     - Remote access via rootd: e.g. "root://pcepsft02.cern.ch:9090//localdisk/ \n 
        ///       Further documentation can be found in the class description of TNetFile
        ///       (http://root.cern.ch/root/html/TNetFile.html)
        /// .
        /// .
        /// .
        /// - Utilization of FileCatalog:
        ///   - This mode is triggered if the name parameter starts with one of the following prefixes
        ///     "PFN:", "FID:" or "LFN:". 
        ///   - According to the prefix the name is interpreted as 
        ///     Physical File Name, unique File ID or Logical File Name
        ///   - The connection string is interpreted as URI of the FileCatalog. 
        ///     The collection retrieves the FileCatalog defined by the given URI from FileCatalogMap.
        ///     A default file catalog (empty connection string) can be defined there.
        ///   - This mode works also in the context of MultiCollection. 
        ///     If a collection created by a MultiCollection iterator is a RootCollection using  FileCatalog,
        ///     it will use the same catalog as its multi collection parent. 
        ///     On the other side the connection string defining
        ///     the FileCatalog of a RootCollection is ignored when a RootCollection using FileCatalogs is
        ///     added to a MultiCollection 
        /// .
        /// .

    
        RootCollection(  const pool::ICollectionDescription* description,
                         pool::ICollection::OpenMode mode,
                         pool::ISession* );

        /// Destructor
        ~RootCollection();
    
        virtual void addTreeBranch( const std::string& name, const std::string& type_name );

        /// Return openMode
        virtual ICollection::OpenMode openMode() const final override; 

        /// Explicitly re-opens the collection after it has been closed.
        virtual void open() final override;
    
        /// Checks if the collection is open.
        virtual bool isOpen() const final override;

        /// Adds a new row of data to the collection.
        virtual void insertRow( const pool::CollectionRowBuffer& inputRowBuffer ) final override;

        /// Commits the last changes made to the collection
        virtual void commit( bool restartTransaction = false ) final override;
    
        /// Explicitly closes the collection
        virtual void close() final override;
    
        /// Returns an object used to describe the collection properties.
        virtual const ICollectionDescription& description() const final override;

        /// Returns an object used to query the collection.
        virtual ICollectionQuery*         newQuery() final override;

     private:
    
        /// copying unimplemented in this class.
        RootCollection(const RootCollection &);
        /// copying unimplemented in this class.
        RootCollection & operator = (const RootCollection &);
    
        void delayedFileOpen( const std::string& method );
        TTree* getCollectionTree();
        void setupTree() const;
        void readAttributeListSpecification() const;
        void writeAttributeListSpecification();

        bool fileCatalogRequired()const;
        std::string retrievePFN()const;
        std::string retrieveFID();
        std::string retrieveUniquePFN(const FileCatalog::FileID& fid);
        std::string retrieveBestPFN(const FileCatalog::FileID& fid)const;  
        void retrieveFileCatalog()const;

        void cleanup();
        
     public:
        
        static const unsigned int c_maxLengthOfStrings = 5000;
        static const char* const poolOptToRootOpt[];
        static const Io::IoFlags poolOptToFileMgrOpt[];
        static const char* const c_tokenBranchName;// = "Token";
        static const char* const c_attributeListLayoutName;// = "Schema"; 

     private:

        CollectionDescription                m_description;
        
        std::string                          m_name;
        std::string                          m_fileName;
        ICollection::OpenMode                m_mode;
        TTree*                               m_tree;
        TFile*                               m_file;
        ISession*                            m_session;
        bool                                 m_open;
        bool                                 m_readOnly;
        bool                                 m_schemaWritten;
        
        std::unique_ptr<pool::IFileCatalog> m_fileCatalog;
        coral::MessageStream                m_poolOut;

        SmartIF<IFileMgr>                  m_fileMgr;

      };
   }
}
#endif



