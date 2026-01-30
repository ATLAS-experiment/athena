/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbDatabaseObj object definition
//--------------------------------------------------------------------
//
//  Package    : System (The POOL project)
//
//  Description: Generic data persistency
//
//  @author      M.Frank
//====================================================================
#ifndef POOL_DBDATABASEOBJ_H
#define POOL_DBDATABASEOBJ_H 1

// We need the definition of the domain for defining the Database
#include "PersistentDataModel/Guid.h"
#include "StorageSvc/DbAccessObj.h"
#include "StorageSvc/DbDomain.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainer.h"
#include "POOLCore/DbPrint.h"

// STL include files
#include <map>
#include <string>
#include <vector>


/*
 *  POOL namespace declaration
 */
namespace pool    {

  /** @class DbDatabaseObj DbDatabaseObj.h src/DbDatabaseObj.h
    *
    * Description:
    *
    * Implementation independent part of a Database object
    * objects.
    *
    * @author  M.Frank
    * @version 1.0
    */
  class DbDatabaseObj : public  DbAccessObj<std::string, DbContainerObj >, public APRMessaging {
  private:
    /// Reflection class identifier
    typedef RootType                            TypeH;
    /// Database parameter definition
    typedef std::pair<std::string, std::string>      Parameter;
    /// Database parameter container definition
    typedef std::vector< Parameter >                 Parameters;
    /// Collection of retired database containers
    typedef std::vector< DbContainerObj >            Containers;
    /// Parameter map definition
    typedef std::map<std::string, std::string>       ParamMap;
    /// Definition of map with link elements
    typedef std::map< Guid , Token* >                LinkMap;
    /// Definition of array with link elements
    typedef std::vector<Token*>                      LinkVector;
    /// Definition of map with shape elements
    typedef std::map< Guid , const DbTypeInfo* >     ShapeMap;
    /// Definition of map with index elements
    typedef std::map< long long int , int >          IndexMap;
    /// Base class convenience typdef
    typedef DbAccessObj<std::string, DbContainerObj > Base;
     
    /// Handle to domain
    DbDomain                      m_dom;
    /// Handle to link container
    DbContainer                   m_links;
    /// Handle to shapes container
    DbContainer                   m_shapes;
    /// Handle to the parameter container
    DbContainer                   m_params;
    /// Technology dependent implementation block
    IDbDatabase*                  m_info;
    /// Map containing all links
    LinkMap                       m_linkMap;
    /// Random access array containing all links
    LinkVector                    m_linkVec;
    /// Map containing all index
    IndexMap                      m_indexMap;
    /// Map containing all shapes
    ShapeMap                      m_shapeMap;
    /// Map with all cached file properties
    ParamMap                      m_paramMap;
    /// Map with all cached mappings between known reflection classes
    std::map<TypeH, const DbTypeInfo*> m_classMap;
    /// Collection of retired database containers
    Containers                    m_retiredConts;
    /// Internal string representation type
    const DbTypeInfo*             m_string_t;
    /// Physical Database login
    std::string                   m_logon;
    /// File age counter
    int                           m_fileAge;
    /// Token describing the object
    Token*                        m_token;

  private:
    /// Perform cleanup of internal data structures
    void cleanup();
    
  public:
    /// Standard constructor of a Database obejct
    DbDatabaseObj( DbDomain&          dom,
                   const std::string& pfn,
                   const std::string& fid,
                   DbAccessMode mode = pool::READ);
    /// Standard Destructor
    virtual ~DbDatabaseObj();
    /// Access to the logon string
    const std::string& logon() const  {  return m_logon;        }
    /// Access the size of the database: May be undefined for some technologies
    long long int size();
    /// Access to technology dependent implementation
    IDbDatabase* info()               {  return m_info;         }
    const IDbDatabase* info() const   {  return m_info;         }
    /// Access to domain handle (CONST)
    DbDomain& domain()                {  return m_dom;          }
    const DbDomain& domain() const    {  return m_dom;          }
    /// Access age value
    int  age()  const                 {  return m_fileAge;      }
    /// Access the token of the database object
    const Token* token() const        {  return m_token;        }

    /// Open Database object
    StatusCode open();
    /// Re-open database with changing access permissions
    StatusCode reopen(DbAccessMode mode);
    /// Close database object
    StatusCode close();
    /// End database access, but still leave database accessible
    StatusCode retire();
    /// Execute Database Transaction action
    StatusCode transAct(Transaction::Action action);

    /// read an object referenced by the token
    StatusCode read(const Token& token, ShapeH shape, void** object);
    /// Expand OID into a full Token, based on the Links table.
    StatusCode getLink(const Token::OID_t& oid, Token* pTok);
    /// Retrieve container name from link container (using token oid, rather than contID)
    std::string cntName(Token& token);
    /// Add association link to link container
    StatusCode makeLink(Token* pToken, Token::OID_t& refLink);
    /// Retrieve persistent type information by class handle
    const DbTypeInfo* objectShape(const TypeH& classH);
    /// Retrieve persistent type information by shape identifier
    const DbTypeInfo* objectShape(const Guid& nam);
    /// Retrieve persistent type information by container
    const DbTypeInfo* contShape(const std::string& nam);
    /// Add persistent type to the Database
    StatusCode addShape (const DbTypeInfo* pType);
    /// Access local container token (if container exists)
    const Token* cntToken(const std::string& cntName);
    /// Allow access to all known containers
    StatusCode containers(std::vector<const Token*>& conts, bool intern);
    StatusCode containers(std::vector<IDbContainer*>& conts, bool intern);
    /// Allow access to all known associations between containers
    StatusCode associations(std::vector<const Token*>& conts);
    /// Allow access to all known shapes used by the database
    StatusCode shapes(std::vector<const DbTypeInfo*>& shaps);
    /// Retrieve the number of user parameters
    int nParam();
    /// Add a persistent parameter to the file
    StatusCode addParam(const std::string& nam, const std::string& val);
    /// Retrieve existing parameter by name
    StatusCode param(const std::string& nam, std::string& val);
    /// Retrieve all parameters
    StatusCode params(std::vector< std::pair<std::string, std::string> >& vals);
    /// Set options
    StatusCode setOption(const DbOption& refOpt);
    /// Access options
    StatusCode getOption(DbOption& refOpt);

    /// Update database age
    void setAge(int value);
  };
}       // End namespace pool
#endif  // POOL_DBDATABASEOBJ_H
