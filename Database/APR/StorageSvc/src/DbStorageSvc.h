/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOL_DBSTORAGESVC_H
#define POOL_DBSTORAGESVC_H

// Framework include files
#include "StorageSvc/DbSession.h"
#include "StorageSvc/DbDomain.h"
#include "StorageSvc/IStorageSvc.h"
#include "POOLCore/DbPrint.h"

/*
 *   POOL namespace declaration
 */
namespace pool  {

  // Forward declarations
  class DbOption;

  /** @class DbStorageSvc DbStorageSvc.h POOLCore/DbStorageSvc.h
    *
    * The DbStorageSvc class is able to handle user request for
    *     - transient objects to become persistent and
    *     - persistent objects to be made transient.
    *     What the storage manager however needs to perform this task 
    *     are object mappings, which translate between the two representations.
    *
    * This functionality is defined in the IDbStorageSvc interface and 
    * implemented in the DbStorageSvc class. Please refer to the header
    * file POOLCore/IDbStorageSvc for further details.
    *
    * @author  Markus Frank
    * @version 1.0
    */
  class DbStorageSvc  : virtual public IStorageSvc, virtual public APRMessaging
  {
    typedef std::vector<const Token*> TokenVec;
  private:
    /// Service Name                               
    std::string         m_name;
    /// Reference counter                          
    unsigned int        m_refCount;
    /// Database session handle
    DbSession           m_sesH;
    /// Database domain handle
    DbDomain            m_domH;
    /// Property: AgeLimit indicating the maximal allowed age of files
    int                 m_ageLimit;
    /// Technology type
    DbType              m_type;
  public:

    /// Standard Constructor: Constructs an object of type DbStorageSvc.
    DbStorageSvc();

    /// Initializing Constructor: Constructs an object of type DbStorageSvc.
    explicit DbStorageSvc(const std::string& name);

    /// Standard destructor.
    virtual ~DbStorageSvc();

    DbStorageSvc (const DbStorageSvc&) = delete;
    DbStorageSvc& operator= (const DbStorageSvc&) = delete;

    /// Label of the specific class
    static const char* catalogLabel()  {   return "pool_DbStorageSvc";       }

    /// Database session handle
    DbSession& sessionHdl()                               {   return m_sesH;  }
    /// Database domain handle
    DbDomain& domainHdl()                                 {   return m_domH;  }

    /// IInterface implementation: Query interfaces of Interface
    virtual StatusCode queryInterface(const Guid& riid, void** ppvUnknown) override final;

    /// IInterface implementation: Reference Interface instance               
    virtual unsigned int addRef() override final;

    /// IInterface implementation: Release Interface instance                 
    virtual unsigned int release() override final;

    /**@name IService interface                                   */

    /// IService implementation override: Initilize Service
    virtual StatusCode initialize();

    /// IService implementation override: Finalize Service     
    virtual StatusCode finalize();

    /// IService implementation: Retrieve name of the service               
    virtual const std::string& name() const    { return m_name;  }

    /// Register object for write
    /**
      * @param   refDB     [IN] Reference to Database descriptor 
      * @param   refCont   [IN] Reference to container name 
      * @param   technology[IN] Specialised sub-technology
      * @param   object    [IN] Pointer to persistent data object.
      * @param   shapeH    [IN] Handle to persistent type information
      * @param   refpTok  [OUT] Reference to location for storing the
      *                         pointer of the persistent object token.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode allocate( FileDescriptor&       refDB,
                                 const std::string&    refCont,
                                 int                   technology,
                                 const void*           object,
                                 ShapeH                shapeH,
                                 Token*&               refpTok) override final;

    /// Read a persistent object from the medium.
    /** Reading an object does not create the object.
      *
      * @param   refDB     [IN] Reference to Database descriptor 
      * @param   persToken [IN] Reference to persistent token information.
      * @param   object   [OUT] Pointer to persistent data pointer.
      * @param   shapeH    [IN] Desired object shape to be read
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode read(     const FileDescriptor& refDB,
                                 const Token&          persToken,
                                 ShapeH                shapeH,
                                 void**                object) override final;

    /// Get container name for object
    /**
      * @param   refDB     [IN] Reference to Database descriptor
      * @param   pToken    [IN] Token to the persistent object.
      * @return                 std::string container name.
      */
    virtual std::string getContName(FileDescriptor& refDB,
                                    Token&          persToken) override final;

    /// Retrieve persistent shape from Storage manager.
    /** The persistent shape is saved at write time to a Database.
      * To match the transient shape of an object to the persistent shape of 
      * the data at the time the data were written should allow for schema
      * evolution and object transformation(s).
      *
      * @param   refDB     [IN] Reference to Database descriptor 
      * @param   objType   [IN] Shape identifier.
      * @param   shapeH   [OUT] Handle to persistent mapping of a given
      *                         object type.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode getShape(  FileDescriptor&       refDB,
                                  const Guid&           objType,
                                  ShapeH&               shapeH) override final;

    /// Create a Shape representation based on a ShapeID
    /**
      * @param   shapeID   [IN] Shape identifier.
      *
      * @return            Handle to persistent mapping of a given object type.
      */
    virtual ShapeH     createShape( const Guid& shapeID ) override final;

    /// Start a new Database Session.
    /** The Database session handles Databases of one given type. This
      * involves specific handling of a given domain represented 
      * by a technology type. All subsequent actions involving Database
      * actions will re-use this technology identifier.
      * The session is a purely logical concept, which typically cannot fail
      * unless the underlying technology requires global initialization calls.
      *
      * @param    mode     [IN] Flag to indicate the accessmode of the session.
      *                         READ, NEW/CREATE/WRITE, UPDATE, RECREATE
      * @param    tech     [IN] Flag indicating the technology type of the
      *                         Database  the user  wants to connect to.
      * @param    session [OUT] Token or handle to the Database session.
      *                         This handle may later be used to open a
      *                         new Database connection.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode startSession(int                 mode,
                                    int                 tech,
                                    SessionH&           session) override final;

    /// End the Database session.
    /** The  request to end a Database session requires, that all pending 
      * Transactions and connections are closed. Otherwise internally the
      * close will be forced and  potentially data on pending Transactions
      * will be lost. The token will be invalidated may not be used at any 
      * longer once the session ended.
      *
      * @param    session  [IN] Handle to the Database 
      *                         session. This handle was retrieved when 
      *                         starting the session. 
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode endSession(  const SessionH       session) override final;

    /// Check the existence of a logical Database unit.
    /** 
      *
      * @param    sessionH [IN] Session context to be used to open the Database.
      * @param    mode     [IN] Flag to indicate the accessmode of the session.
      *                         READ, NEW/CREATE/WRITE, UPDATE, RECREATE.
      * @param    refDB   [I/O] Descriptor of the Database to be opened. 
      *                         On successful return the Database handle is
      *                         valid.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode existsConnection(const SessionH        sessionH,
                                        int                   mode,
                                        const FileDescriptor& refDB) override final;

    /// Connect to a logical Database unit.
    /** A connection is equivalent to the triple (OCISession, OCIServer, 
      * OCISvcCtx) in ORACLE, a login to a datasource using ODBC, or a 
      * single federation for Objectivity. For file based technologies, 
      * such as root, MS Access, ODBC/Text etc., this is involves the 
      * opening of the requested file.
      *
      * @param    sessionH [IN] Session context to be used to open the Database.
      * @param    mode     [IN] Flag to indicate the accessmode of the session.
      *                         READ, NEW/CREATE/WRITE, UPDATE, RECREATE.
      * @param    refDB   [I/O] Descriptor of the Database to be opened. 
      *                         On successful return the Database handle is
      *                         valid.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode connect( const SessionH      sessionH,
                                int                 mode,
                                FileDescriptor&     refDB) override final;

    /// Disconnect from a logical Database unit.
    /** The  request for disconnect requires, that all pending Transactions
      * are already commited. Otherwise data are lost. On disconnection the 
      * access to the Database is finalized, closed. The token will be 
      * invalidated may not be used at any longer after disconnection.
      *
      * @param    refDB    [IN] Descriptor of the Database access. 
      *                         This handle was retrieved when connecting 
      *                         to the logical Database.
      *                         On successful return the Database handle will
      *                         be invalidated.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode disconnect(  FileDescriptor&  refDB) override final;

    /// Query the access mode of a Database unit.
    /**
      * @param    refDB    [IN] Descriptor of the Database to be queried. 
      * @param    mode    [OUT] Open mod to the database.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode openMode( FileDescriptor&     refDB,
                                 int&                mode ) override final;


    /// End/Finish an existing Transaction sequence.
    /** At  this  phase all  objects, which were marked for  write when 
      * the Transaction was started, are either going to be made persistent
      * or scratched. After the Transaction ended, the Transaction context
      * is invalidated and may no longer be used independent wether the 
      * Transaction was successful or not.
      *
      * @param    conn  [IN]    DB connection
      * @param    typ   [IN]    Enum indicating an action to be performed.
      *                         Valid arguments are COMMIT
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode endTransaction( ConnectionH conn,
                                       Transaction::Action typ) override final;

    /// Access options for a given database domain.
    /** Domain options are global options, which refer to the
      * database technology and not to a particular connection.
      *
      * Note: The options depend on the underlying implementation
      * and are not normalized.
      *
      *  @param   sessionH  [IN] Session context to be used to open the Database.
      *  @param   opt       [IN] Reference to option object.
      *
      *  @return StatusCode code indicating success or failure.  
      */
    virtual StatusCode getDomainOption(const SessionH  sessionH,
                                       DbOption&       opt) override final;

    /// Set options for a given database domain.
    /** Domain options are global options, which refer to the
      * database technology and not to a particular connection.
      *
      * Note: The options depend on the underlying implementation
      * and are not normalized.
      *
      *  @param   sessionH  [IN] Session context to be used to open the Database.
      *  @param   opt       [IN] Reference to option object.
      *
      *  @return StatusCode code indicating success or failure.
      */
    virtual StatusCode setDomainOption(const SessionH  sessionH, 
                                       const DbOption& opt) override final;
  };
}       // End namespace pool
#endif  // POOL_DBSTORAGESVC_H
