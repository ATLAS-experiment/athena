/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//
//        Package    : RootStorageSvc (The POOL project)
//
//        Author     : M.Frank
//====================================================================
#ifndef POOL_ROOTSTORAGESVC_ROOTDBASE_H
#define POOL_ROOTSTORAGESVC_ROOTDBASE_H 1

// Framework include files
#include "StorageSvc/IDbDatabase.h"
#include "StorageSvc/DbDatabase.h"

#include <set>
#include <map>
#include <mutex>
#include <unordered_map>


// Forward declarations
namespace ROOT { namespace Experimental { namespace Detail {
   class RPageSource;
} } }
using ROOT::Experimental::Detail::RPageSource;


class TFile;
class TTree;
class TBranch;
class IFileMgr;
namespace RootAuxDynIO {
   class IRootAuxDynReader;
   class IRNTupleWriter;
}

/*
 * POOL namespace declaration
 */
namespace pool  {  

   class RootTreeContainer;
   
  /** @class RootDatabase RootDatabase.h src/RootDatabase.h
    *
    * Description:
    * ROOT specific implementation of Database file.
    *
    * @author  M.Frank
    * @date    1/8/2002
    * @version 1.0
    */
   class RootDatabase : public IDbDatabase
   {
  public:
    enum { READ_COUNTER = 0, WRITE_COUNTER = 1, OTHER_COUNTER = 2 };

  private:
    /// Parent Database handle
    DbDatabase    m_dbH;
    /// Reference to the actual implemented file 
    TFile*        m_file;
    /// Persistency format version
    std::string   m_version;
    /// Counter statistics
    long long int m_counters[3];
    /// Default compression level
    int           m_defCompression;
    /// Default compressionalgorithm 
    int           m_defCompressionAlg;
    /// Default split level
    int           m_defSplitLevel;
    /// Default Autosave parameter for trees
    int           m_defAutoSave;
    /// Default buffer size parameter for Branches
    int           m_defBufferSize;
    /// Maximum buffer size parameter for Branches
    int           m_maxBufferSize;
    /// Minimum buffer entries parameter for Branches
    int           m_minBufferEntries;
    /// Default policy mode for keyed objects
    int           m_defWritePolicy;
    /// Offset table length for branches
    int		  m_branchOffsetTabLen;
    /// Name of tree with cache
    std::string   m_treeNameWithCache;
    /// Default tree cache learn events
    int           m_defTreeCacheLearnEvents;
    /// Flag to enable/disable buffered RNTuple writing
    bool          m_rntBufferedWriteEnabled;
    /// Flag to enable/disable RNTupleReader metrics
    bool          m_rntReaderMetricsEnabled;
    /// Flag to enable/disable RNTupleWriter metrics
    bool          m_rntWriterMetricsEnabled;

    /// name of the container with master index ('*' means use the biggest)
    std::string   m_indexMaster;
    /// nextID of the master index
    uint64_t      m_indexMasterID;

    /// Keep index sizes here, because Branches are emptied when fast merged by SharedWriter
    std::map<TBranch*, uint64_t>   m_indexSizeMap;

    /// marks if the index (for index Containers) was rebuilt for given TTree
    std::set< std::string > m_indexRebuilt;

    /* ---  variables used with TREE_AUTO_FLUSH option for
            managing combined TTree::Fill for branch containers
    */
    // TTree names with requested AUTO_FLUSH value
    std::map< std::string, int >        m_autoFlushTrees;

    // mapping of opened (for update) branch containers, to their TTree
    typedef    std::set< RootTreeContainer* >   ContainerSet_t;
    std::map< TTree*, ContainerSet_t >  m_containersInTree;
    
    std::map< std::string, int >        m_customSplitLevel;

    IFileMgr*     m_fileMgr;

    // mutex to prevent concurrent read I/O from AuxDynReader
    std::recursive_mutex  m_iomutex;

    std::map<std::string, std::unique_ptr<RootAuxDynIO::IRNTupleWriter> >  m_ntupleWriterMap;
    std::map<std::string, std::unique_ptr<RPageSource> >                   m_ntupleReaderMap;

    using indexLookup_t = std::unordered_map<uint64_t, uint64_t>;
    std::map<void*, indexLookup_t>                                         m_ntupleIndexMap;

  public:
    /// Standard Constuctor
    RootDatabase();

    /// Standard destructor
    virtual ~RootDatabase();

    /// Access to the actual implemented file 
    TFile* file()                             { return m_file;    }

    /// Access to the version string
    const std::string& fmtVersion() const     { return m_version; }

    /// Check for file-existence
    /** @param nam      [IN]  Name of the database to be checked.
      *
      * @return Boolean value indicating the database existence.
      */
    static bool exists(const std::string& nam);

    /// Access the size of the database: May be undefined for some technologies
    virtual long long int size()  const;

    /// Do some statistics: add number of bytes read/written/other
    void addByteCount(int which, long long int num_bytes);

    /// Do some statistics: retrieve number of bytes read/written/other
    long long int byteCount(int which) const;


    DbStatus    markBranchContainerForFill(RootTreeContainer*);
    
    void        registerBranchContainer(RootTreeContainer*);

    uint64_t    currentIndexMasterID() const   { return m_indexMasterID; }

    /// get index size for indexed containers
    uint64_t    indexSize(TBranch *branch) { return m_indexSizeMap[branch]; }

    /// increase index size counter for indexed containers (by 1)
    uint64_t    indexSizeInc(TBranch *branch) { return ++m_indexSizeMap[branch]; }


    /// Check if a given TTree had its index rebuilt
    bool	wasIndexRebuilt(const std::string& treeName)   { return m_indexRebuilt.count(treeName)!=0; }
    /// Mark that a given TTree had its index rebuilt
    void	markIndexRebuilt(const std::string& treeName)  { m_indexRebuilt.insert(treeName); };

    /// provide access to the I/O mutex for AuxDynReader and Containers
    std::recursive_mutex& ioMutex()         { return m_iomutex; }
    
    /// Access options
    /** @param opt      [IN]  Reference to option object.
      *
      * @return DbStatus code indicating success or failure.  
      */
    virtual DbStatus getOption(DbOption& opt);

    /// Set options
    /** @param opt      [IN]  Reference to option object.
      *
      * @return DbStatus code indicating success or failure.  
      */
    virtual DbStatus setOption(const DbOption& opt);

    /// implementation of TREE_AUTO_FLUSH option - called from setOption()
    virtual DbStatus setAutoFlush(const DbOption& opt);

    /// Open Database object
    /** @param domH     [IN]  Handle to valid domain object
      *                       (validity ensured by upper levels).
      * @param nam      [IN]  Name of the database to be opened.
      * @param mode     [IN]  Desired session access mode.
      *
      * @return DbStatus code indicating success or failure.  
      */
    virtual DbStatus open(const DbDomain&     domH, 
                          const std::string&  nam, 
                          DbAccessMode        mode);

    /// Re-open database with changing access permissions
    /** @param mode     [IN]  Desired session access mode.
      *
      * @return DbStatus code indicating success or failure.  
      */
    virtual DbStatus    reopen(DbAccessMode mode);

    /// Callback after successful open of a database object
    /** @param dbH      [IN]  Handle to valid database object
      * @param mode     [IN]  Desired session access mode.
      *
      * @return DbStatus code indicating success or failure.  
      */
    virtual DbStatus    onOpen(DbDatabase& dbH, DbAccessMode      mode);

    /// Close database access
    /** @param mode     [IN]  Desired session access mode.
      *
      * @return DbStatus code indicating success or failure.  
      */
    virtual DbStatus    close(DbAccessMode mode);

    /// Execute Database Transaction action
    virtual DbStatus    transAct(Transaction::Action action);

    RPageSource*        getNTupleReader(std::string ntuple_name);

    // translate index value to row# for a given RNTuple  
    uint64_t            indexLookup(RPageSource *ps, uint64_t idx_val);

    /// return NTupleWriter for a given ntuple_name
    /// create a new one if needed when create==true
    RootAuxDynIO::IRNTupleWriter*  getNTupleWriter(std::string ntuple_name, bool create=false);

  protected:
    // Execute any pending Fills before commit or flush
    DbStatus            fillBranchContainerTrees();

    // Reduce branches' baskets' size to m_maxBufferSize for a give TTree
    void                reduceBasketsSize(TTree* tree);

    void                increaseBasketsSize(TTree* tree);

    DbStatus            close();
   };

}       // End namespace pool
#endif  /* POOL_ROOTSTORAGESVC_ROOTDBASE_H */
