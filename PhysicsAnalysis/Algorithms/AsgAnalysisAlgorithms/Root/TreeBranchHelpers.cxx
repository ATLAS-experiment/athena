/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//
// includes
//

#include "AsgAnalysisAlgorithms/TreeBranchHelpers.h"

// EDM include(s):
#include "AthContainersInterfaces/IAuxStoreIO.h"
#include "AthContainersInterfaces/IAuxTypeVectorFactory.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxVectorBase.h"
#include "AthContainers/normalizedTypeinfoName.h"
#include "xAODRootAccess/tools/THolder.h"
#include "xAODMissingET/MissingETContainer.h"

// ROOT include(s):
#include <TClass.h>
#include <TTree.h>
#include <TBranch.h>
#include <TVirtualCollectionProxy.h>

// System include(s):
#include <regex>
#include <algorithm>
#include <functional>
#include <sstream>

//
// method implementations
//

namespace {


class TempInterface
  : public SG::AuxVectorData
{
public:
  TempInterface (size_t size) : m_size (size) {}
  TempInterface (size_t size, SG::auxid_t auxid, void* ptr) :
    m_size (size)
  {
    setCache (auxid, ptr);
  }

  using AuxVectorData::setStore;

  virtual size_t size_v() const { return m_size; }
  virtual size_t capacity_v() const { return m_size; }

private:
  size_t m_size;
};


} // anonymous namespace


namespace {

#ifdef XAOD_STANDALONE

   /// Get an xAOD container from the event store
   ///
   /// This is the "standalone implementation" of the function.
   ///
   /// @param key The name of the container in the event store
   /// @param evtStore The value of evtStore() from the algorithm
   /// @param allowMissing Set to @c true to print an error message in case
   ///                     of a failure
   /// @param[out] cl type of container
   /// @param msg Reference to the caller's @c MsgStream object
   /// @return A pointer to the container if successful, @c nullptr if not
   ///
   const SG::AuxVectorBase* getVector( const std::string& key,
                                       asg::SgTEvent& evtStore,
                                       bool allowMissing,
                                       const TClass*& cl,
                                       MsgStream& msg ) {
      if( allowMissing &&
          ( ! evtStore.contains< const SG::AuxVectorBase >( key ) ) ) {
         return nullptr;
      }
      const SG::AuxVectorBase* c = nullptr;
      if( ! evtStore.retrieve( c, key ).isSuccess() ) {
         msg << MSG::ERROR << "Couldn't retrieve container with key \"" << key
             << "\"" << endmsg;
         return nullptr;
      }
      const xAOD::THolder* holder = evtStore.tds()->holder( key );
      if( holder != nullptr ) {
         // If the object is in the transient store, get the type of it from
         // the transient store itself. So that ConstDataVector types would be
         // handled correctly.
         const std::type_info* ti = holder->getTypeInfo();
         cl = TClass::GetClass( *ti );
      } else {
         // If the object is not in the transient store, let's just use its
         // "actual type".
         cl = TClass::GetClass( typeid( *c ) );
      }
      if( ( allowMissing == false ) && ( cl == nullptr ) ) {
         msg << MSG::ERROR
             << "Couldn't find TClass dictionary for container \"" << key
             << "\"" << endmsg;
         return nullptr;
      }

      // Return the vector object.
      return c;
   }

   /// Get a standalone xAOD object from the event store
   ///
   /// This is the "standalone implementation" of the function.
   ///
   /// @param key The name of the container in the event store
   /// @param evtStore The value of evtStore() from the algorithm
   /// @param allowMissing Set to @c true to print an error message in case
   ///                     of a failure
   /// @param msg Reference to the caller's @c MsgStream object
   /// @return A pointer to the container if successful, @c nullptr if not
   ///
   const SG::AuxElement* getElement( const std::string& key,
                                     asg::SgTEvent& evtStore,
                                     bool allowMissing,
                                     MsgStream& msg ) {
      if( allowMissing &&
          ( ! evtStore.contains< const SG::AuxElement >( key ) ) ) {
         return nullptr;
      }
      const SG::AuxElement* e = nullptr;
      if( ! evtStore.retrieve( e, key ).isSuccess() ) {
         msg << MSG::ERROR << "Couldn't retrieve object with key \"" << key
             << "\"" << endmsg;
         return nullptr;
      }
      return e;
   }

#else

   /// Predicate for finding a proxy with a given name
   class ProxyWithName {
   public:
      /// Type of the predicate's argument
      typedef const SG::DataProxy* argument_type;
      /// Constructor with key/name
      ProxyWithName( const std::string& name ) : m_name( name ) {}
      /// Operator evaluating whether this is a good proxy or not
      bool operator()( argument_type proxy ) const {
         return ( proxy->name() == m_name );
      }
   private:
      std::string m_name; ///< Name for the proxy to find
   }; // class ProxyWithName

   /// Get an xAOD container from the event store
   ///
   /// This is the "Athena implementation" of the function.
   ///
   /// @param key The name of the container in the event store
   /// @param evtStore The value of evtStore() from the algorithm
   /// @param allowMissing Set to @c true to print an error message in case
   ///                     of a failure
   /// @param[out] cl type of container
   /// @param msg Reference to the caller's @c MsgStream object
   /// @return A pointer to the container if successful, @c nullptr if not
   ///
   const SG::AuxVectorBase* getVector ATLAS_NOT_CONST_THREAD_SAFE ( const std::string& key,
                                                                    IProxyDict& evtStore,
                                                                    bool allowMissing,
                                                                    const TClass*& cl,
                                                                    MsgStream& msg ) {

      // Find all proxies with this key:
      auto proxies = evtStore.proxies();
      proxies.erase( std::remove_if( proxies.begin(), proxies.end(),
                                     std::not_fn( ProxyWithName( key ) ) ),
                     proxies.end() );
      // Now iterate over them:
      for( const SG::DataProxy* proxy : proxies ) {
         // We need a non-const version of it... :-(
         SG::DataProxy* proxy_nc = const_cast< SG::DataProxy* >( proxy );
         // Try to get the right object out of it.
         DataBucketBase* bucket =
            dynamic_cast< DataBucketBase* >( proxy_nc->accessData() );
         if( ! bucket ) {
            // This is a big problem in the job. Return right away.
            msg << MSG::ERROR
                << "Couldn't access data object as a data bucket?!?" << endmsg;
            return nullptr;
         }
         // Get the dictionary for the type:
         cl = TClass::GetClass( bucket->tinfo() );
         if( ! cl ) {
            if( msg.level() <= MSG::VERBOSE ) {
               msg << MSG::VERBOSE << "No dictionary found for: "
                   << bucket->tinfo().name() << endmsg;
            }
            continue;
         }
         // Check whether the object inherits from AuxVectorBase:
         if( ! cl->InheritsFrom( "SG::AuxVectorBase" ) ) {
            if( msg.level() <= MSG::VERBOSE ) {
               msg << MSG::VERBOSE << "Object \"" << key << "/" << cl->GetName()
                   << "\" does not inherit from SG::AuxVectorBase" << endmsg;
            }
            continue;
         }
         // If all is well, just assume that the inheritance is direct/simple:
         const SG::AuxVectorBase* result =
            reinterpret_cast< const SG::AuxVectorBase* >( bucket->object() );
         return result;
      }

      // Apparently we failed...
      if( ! allowMissing ) {
         msg << MSG::ERROR << "Couldn't retrieve object \"" << key
             << "\" as SG::AuxVectorBase" << endmsg;
      }
      return nullptr;
   }

   /// Get a standalone xAOD object from the event store
   ///
   /// This is the "Athena implementation" of the function.
   ///
   /// @param key The name of the container in the event store
   /// @param evtStore The value of evtStore() from the algorithm
   /// @param allowMissing Set to @c true to print an error message in case
   ///                     of a failure
   /// @param msg Reference to the caller's @c MsgStream object
   /// @return A pointer to the container if successful, @c nullptr if not
   ///
   const SG::AuxElement* getElement ATLAS_NOT_CONST_THREAD_SAFE ( const std::string& key,
                                                                  StoreGateSvc& evtStore,
                                                                  bool allowMissing,
                                                                  MsgStream& msg ) {


      const SG::AuxElement* e = nullptr;
      if( !evtStore.retrieve( e, key ).isSuccess() ) {
        if(!allowMissing) {
          msg << MSG::ERROR << "Couldn't retrieve object with key \"" << key
             << "\"" << endmsg;
        }
        return nullptr;
      }
      return e;

   }
#endif // XAOD_STANDALONE

   /// This function is used internally in the code when creating primitive
   /// branches. I just took the code from xAODRootAccess, which itself too it
   /// from SFrame... :-P
   ///
   /// @param typeidType The type name coming from typeid(...).name()
   /// @param msg The caller's @c MsgStream object
   /// @return The character describing this type for @c TTree::Branch
   ///
   char rootType( char typeidType, MsgStream& msg ) {

      // Do the hard-coded translation:
      switch( typeidType ) {

         case 'c':
            return 'B';
            break;
         case 'h':
            return 'b';
            break;
         case 's':
            return 'S';
            break;
         case 't':
            return 's';
            break;
         case 'i':
            return 'I';
            break;
         case 'j':
            return 'i';
            break;
         case 'f':
            return 'F';
            break;
         case 'd':
            return 'D';
            break;
         case 'x':
            return 'L';
            break;
         case 'y':
         case 'm': // Not sure how platform-independent this one is...
            return 'l';
            break;
         case 'b':
            return 'O';
            break;
         default:
            // If we didn't find this type:
            msg << MSG::ERROR << "Received an unknown type: " << typeidType
                << endmsg;
            return '\0';
            break;
      }
   }
} // private namespace

namespace CP
{
  namespace TreeBranchHelpers
  {
    StatusCode BranchConfig ::
    parse (const std::string& branchDecl, MsgStream& msg)
    {
      // The regular expression used to extract the needed info. The logic
      // is supposed to be:
      //
      // (match[1]).(match[2])<any whitespace>-><any whitespace>(match[3])[<any whitespace>type=(match[5])][<any whitespace>metTerm=(match[7])][<any whitespace>basketSize=(match[9])]
      //
      // Like:
      //    "Electrons.eta  -> el_eta"
      //    "Electrons.eta  -> el_eta type=float"
      //    "MissingET.px   -> met_px metTerm=Final"
      static const std::regex
         re( "\\s*([\\w%]+)\\.([\\w%]+)\\s*->\\s*([\\w%]+)(\\s+type=([\\w%]+))?(\\s+metTerm=([\\w%]+))?(\\s+basketSize=([\\w%]+))?" );

      // Interpret this branch declaration.
      std::smatch match;
      if( ! std::regex_match( branchDecl, match, re ) ) {
        msg << MSG::ERROR << "Expression \"" << branchDecl << "\" doesn't match \"<object>.<variable> -> <branch>\"" << endmsg;
         return StatusCode::FAILURE;
      }
      this->branchDecl = branchDecl;
      sgName = match[ 1 ];
      auxName = match[ 2 ];
      branchName = match[ 3 ];
      typeName = match[ 5 ];
      metTermName = match[ 7 ];
      if (match[9].matched) {
        try {
          basketSize = std::stoi(match[9]);
        } catch (const std::exception& ) {
          msg << MSG::ERROR << "Could not parse basket size value: " << match[9] << endmsg;
          return StatusCode::FAILURE;
        }
      }
      return StatusCode::SUCCESS;
    }



    StatusCode BranchConfig ::
    configureTypes (std::set<std::string>& decosWithoutType, MsgStream& msg)
    {
      std::string nominalAuxName = auxName;
      if (auto pos = nominalAuxName.find ("%SYS%"); pos != std::string::npos)
        nominalAuxName.replace (pos, 5, "NOSYS");
      if (!typeName.empty())
      {
        if (typeName == "char")
          SG::ConstAccessor<char> {nominalAuxName};
        else if (typeName == "float")
          SG::ConstAccessor<float> {nominalAuxName};
        else if (typeName == "int")
          SG::ConstAccessor<int> {nominalAuxName};
        else if (typeName == "unsigned")
          SG::ConstAccessor<unsigned> {nominalAuxName};
        else if (typeName == "uint16")
          SG::ConstAccessor<std::uint16_t> {nominalAuxName};
        else if (typeName == "uint32")
          SG::ConstAccessor<std::uint32_t> {nominalAuxName};
        else
        {
          unsigned line = __LINE__ - 2;
          std::string file = __FILE__;
          file = file.substr (file.find_last_of("/\\") + 1);
          msg << MSG::ERROR << "Unknown type requested, please extend " << file << " near line " << line << " for type " << typeName << endmsg;
          return StatusCode::FAILURE;
        }
      }
      const SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();
      if (nominalAuxId == SG::null_auxid)
      {
        nominalAuxId = reg.findAuxID (nominalAuxName);
        if (nominalAuxId == SG::null_auxid)
        {
          decosWithoutType.insert (nominalAuxName);
          msg << MSG::DEBUG << "No aux ID found for auxiliary variable: " << nominalAuxName << endmsg;
          // just returning SUCCESS here, our caller will report failure
          return StatusCode::SUCCESS;
        }
      }
      if (auxType == nullptr)
      {
        auxType = reg.getType (nominalAuxId);
        if (auxType == nullptr)
        {
          msg << MSG::ERROR
              << "No std::type_info available for aux-store variable: "
              << nominalAuxName << endmsg;
          return StatusCode::FAILURE;
        }
      }
      if (auxVecType == nullptr)
      {
        auxVecType = reg.getVecType (nominalAuxId);
        if (auxVecType == nullptr)
        {
          msg << MSG::ERROR
              << "No std::type_info available for aux-store variable: "
              << nominalAuxName << endmsg;
          return StatusCode::FAILURE;
        }
      }
      if (auxFactory == nullptr)
      {
        auxFactory = reg.getFactory (nominalAuxId);
        if (auxFactory == nullptr)
        {
          msg << MSG::ERROR
              << "No factory found for auxiliary variable: "
              << nominalAuxName << endmsg;
          return StatusCode::FAILURE;
        }
      }
      return StatusCode::SUCCESS;
    }



    StatusCode BranchConfig ::
    configureSystematics (ISystematicsSvc& sysSvc, MsgStream& msg)
    {
      if (sgName.find ("%SYS%") == std::string::npos &&
          auxName.find ("%SYS%") == std::string::npos &&
          branchName.find ("%SYS%") == std::string::npos)
      {
        nominalOnly = true;
      }
      if (!nominalOnly)
      {
        if (branchName.find ("%SYS%") == std::string::npos)
        {
          msg << MSG::ERROR << "Branch with systematics without %SYS% in branch name: "
              << branchName << endmsg;
          return StatusCode::FAILURE;
        }
        if (sgName.find ("%SYS%") == std::string::npos &&
            auxName.find ("%SYS%") == std::string::npos)
        {
          msg << MSG::ERROR << "Branch with systematics without %SYS% in SG or aux name: "
              << sgName << "." << auxName << endmsg;
          return StatusCode::FAILURE;
        }
        if (auxName.find ("NOSYS") != std::string::npos)
        {
          msg << MSG::ERROR << "Branch with systematics with NOSYS in aux name: "
              << sgName << "." << auxName << endmsg;
          return StatusCode::FAILURE;
        }
        if (sgName.find ("NOSYS") != std::string::npos && auxName.find ("%SYS%") == std::string::npos)
        {
          msg << MSG::ERROR << "Branch with NOSYS in SG name but without %SYS% in aux name: "
              << sgName << "." << auxName << endmsg;
          return StatusCode::FAILURE;
        }

        if (sgName.find ("%SYS%") != std::string::npos)
          sgNameFilterSys = sysSvc.getObjectSystematics( sgName );

        if (auxName.find ("%SYS%") != std::string::npos)
        {
          if (auto pos = sgName.find ("NOSYS"); pos == std::string::npos)
            auxNameFilterSys = sysSvc.getDecorSystematics (sgName, auxName);
          else
          {
            // Sometimes while object systematics were applied we are not interested in them,
            // NOSYS will then be used on the container name.
            // Decoration systematics however will only be aware of containers with %SYS% included.
            // Some special handling is needed to translate from NOSYS back to %SYS%.
            std::string sgNameSys = sgName;
            sgNameSys.replace (pos, 5, "%SYS%");

            // these will be the object systematics
            auto objectSys = sysSvc.getObjectSystematics (sgNameSys);

            // these will be all systematics (object+decor)
            auto allSys = sysSvc.getDecorSystematics (sgNameSys, auxName);

            // we now need to filter-out object systematics
            for (auto& variation : allSys)
            {
              if (objectSys.find (variation) == objectSys.end())
                auxNameFilterSys.insert (variation);
            }
          }
        }

        branchNameFilterSys = sgNameFilterSys;
        branchNameFilterSys.insert (auxNameFilterSys);
        if (branchNameFilterSys.empty())
          nominalOnly = true;
      }

      return StatusCode::SUCCESS;
    }



    StatusCode OutputBranchData ::
    configureNames (const BranchConfig& branchConfig, const CP::SystematicSet& sys, ISystematicsSvc& sysSvc, MsgStream& msg)
    {
      isNominal = true;

      if (branchConfig.sgName.find ("%SYS%") != std::string::npos)
      {
        CP::SystematicSet matching;
        if (SystematicSet::filterForAffectingSystematics (sys, branchConfig.sgNameFilterSys, matching).isFailure())
          return StatusCode::FAILURE;
        if (sysSvc.makeSystematicsName (sgName, branchConfig.sgName, matching).isFailure())
          return StatusCode::FAILURE;
        if (!matching.empty())
          isNominal = false;
      } else
        sgName = branchConfig.sgName;

      if (branchConfig.auxName.find ("%SYS%") != std::string::npos)
      {
        CP::SystematicSet matching;
        if (SystematicSet::filterForAffectingSystematics (sys, branchConfig.auxNameFilterSys, matching).isFailure())
          return StatusCode::FAILURE;
        if (sysSvc.makeSystematicsName (auxName, branchConfig.auxName, matching).isFailure())
          return StatusCode::FAILURE;
        if (!matching.empty())
          isNominal = false;
      } else
        auxName = branchConfig.auxName;

      if (branchConfig.branchName.find ("%SYS%") != std::string::npos)
      {
        CP::SystematicSet matching;
        if (SystematicSet::filterForAffectingSystematics (sys, branchConfig.branchNameFilterSys, matching).isFailure())
          return StatusCode::FAILURE;
        if (sysSvc.makeSystematicsName (branchName, branchConfig.branchName, matching).isFailure())
          return StatusCode::FAILURE;
        if (matching.empty() && !isNominal)
        {
          msg << MSG::FATAL << "Branch \"" << branchName << "\" is not affected by any of the requested systematics but is not nominal." << endmsg;
          return StatusCode::FAILURE;
        }
      } else
      {
        branchName = branchConfig.branchName;
        if (!sys.empty())
        {
          msg << MSG::FATAL << "Branch \"" << branchName << "\" without systematics is evaluated in a non-nominal context." << endmsg;
          return StatusCode::FAILURE;
        }
      }

      return StatusCode::SUCCESS;
    }



    StatusCode
    ElementBranchProcessor::
    setup( TTree& tree, const BranchConfig& branchConfig, OutputBranchData& outputData, MsgStream& msg ) {

      // Remember the branch name.
      m_branchName = outputData.branchName;

      // Create the accessor.
      m_acc.reset( new SG::TypelessConstAccessor( *branchConfig.auxType, outputData.auxName ) );

      // Get a pointer to the vector factory.
      m_factory = branchConfig.auxFactory;

      // Create the data object.
      m_data = m_factory->create( m_acc->auxid(), 1, 1, false );

      // Pointer to the branch, to be created.
      TBranch* br = nullptr;

      // Decide whether we're dealing with a "primitive" or an "object" branch.
      if( strlen( branchConfig.auxType->name() ) == 1 ) {

        // This is a "primitive" variable...

        // Get the type identifier for it that ROOT will understand.
        const char rType = rootType( branchConfig.auxType->name()[ 0 ], msg );
        if( rType == '\0' ) {
            msg << MSG::ERROR << "Type not recognised for variable: "
                << outputData.branchName << endmsg;
            return StatusCode::FAILURE;
        }

        // Construct the type description.
        std::ostringstream typeDesc;
        typeDesc << outputData.branchName << "/" << rType;

        // Create the primitive branch.
        br = tree.Branch( outputData.branchName.c_str(), m_data->toPtr(),
                          typeDesc.str().c_str() );
        if (branchConfig.basketSize.has_value())
          br->SetBasketSize(branchConfig.basketSize.value());

      } else {

        // This is an "object" variable...

        // Get a proper type name for the variable.
        const std::string typeName = SG::normalizedTypeinfoName( *branchConfig.auxType );

        // Access the dictionary for the type.
        TClass* cl = TClass::GetClass( *branchConfig.auxType );
        if( ! cl ) {
            cl = TClass::GetClass( typeName.c_str() );
        }
        if( ! cl ) {
            msg << MSG::ERROR << "Couldn't find dictionary for type: "
                << typeName << endmsg;
            return StatusCode::FAILURE;
        }
        if( ! cl->GetStreamerInfo() ) {
            msg << MSG::ERROR << "No streamer info available for type: "
                << cl->GetName() << endmsg;
            return StatusCode::FAILURE;
        }

        // Create the object branch.
        m_dataPtr = m_data->toPtr();
        br = tree.Branch( outputData.branchName.c_str(), cl->GetName(), &m_dataPtr );
        if (branchConfig.basketSize.has_value())
          br->SetBasketSize(branchConfig.basketSize.value());

      }

      // Check that the branch creation succeeded.
      if( ! br ) {
        msg << MSG::ERROR << "Failed to create branch: " << outputData.branchName
            << endmsg;
        return StatusCode::FAILURE;
      }

      // Return gracefully.
      return StatusCode::SUCCESS;
    }

    StatusCode
    ElementBranchProcessor::
    process( const SG::AuxElement& element, MsgStream& msg ) {

      // A security check.
      if( ( ! m_acc ) || ( ! m_factory ) || ( ! m_data ) ) {
        msg << MSG::FATAL << "Internal logic error detected" << endmsg;
        return StatusCode::FAILURE;
      }

      // Get the data out of the xAOD object.
      //const void* auxData = ( *m_acc )( element );

      // Copy it into the output variable.
      TempInterface dstiface (m_data->size(), m_acc->auxid(), m_data->toPtr());
      m_factory->copy( m_acc->auxid(), dstiface, 0,
                      *element.container(), element.index(), 1 );

      // Return gracefully.
      return StatusCode::SUCCESS;
    }

    StatusCode ContainerBranchProcessor::
    setup( TTree& tree, const BranchConfig& branchConfig, OutputBranchData& outputData, MsgStream& msg ) {

      // Remember the branch name.
      m_branchName = outputData.branchName;

      // Create the accessor.
      m_acc.reset( new SG::TypelessConstAccessor( *branchConfig.auxType, outputData.auxName ) );

      // Get a pointer to the vector factory.
      m_factory = branchConfig.auxFactory;

      // Create the data object.
      m_data = m_factory->create( m_acc->auxid(), 0, 0, false );

      // Get a proper type name for the variable.
      const std::string typeName = SG::normalizedTypeinfoName( *branchConfig.auxVecType );

      // Access the dictionary for the type.
      TClass* cl = TClass::GetClass( *branchConfig.auxVecType );
      if( ! cl ) {
        cl = TClass::GetClass( typeName.c_str() );
      }
      if( ! cl ) {
        msg << MSG::ERROR << "Couldn't find dictionary for type: "
            << typeName << endmsg;
        return StatusCode::FAILURE;
      }
      if( ! cl->GetStreamerInfo() ) {
        msg << MSG::ERROR << "No streamer info available for type: "
            << cl->GetName() << endmsg;
        return StatusCode::FAILURE;
      }

      // Create the branch.
      m_dataPtr = m_data->toVector();
      TBranch* br = tree.Branch( outputData.branchName.c_str(), cl->GetName(),
                                &m_dataPtr );
      if( ! br ) {
        msg << MSG::ERROR << "Failed to create branch: " << outputData.branchName
            << endmsg;
        return StatusCode::FAILURE;
      }
      if (branchConfig.basketSize.has_value())
        br->SetBasketSize(branchConfig.basketSize.value());

      // Return gracefully.
      return StatusCode::SUCCESS;
    }

    StatusCode ContainerBranchProcessor::
    resize( size_t size, MsgStream& msg ) {

      // A security check.
      if( ! m_data ) {
        msg << MSG::FATAL << "Internal logic error detected" << endmsg;
        return StatusCode::FAILURE;
      }

      // Do the deed.
      m_data->resize( 0 );
      m_data->resize( size );

      // Return gracefully.
      return StatusCode::SUCCESS;
    }

    StatusCode ContainerBranchProcessor::
    process( const SG::AuxElement& element, size_t index, MsgStream& msg ) {

      // A security check.
      if( ( ! m_acc ) || ( ! m_factory ) || ( ! m_data ) ) {
        msg << MSG::FATAL << "Internal logic error detected" << endmsg;
        return StatusCode::FAILURE;
      }

      // Get the data out of the xAOD object.
      //const void* auxData = ( *m_acc )( element );

      // Copy it into the output variable.
      TempInterface dstiface (m_data->size(), m_acc->auxid(), m_data->toPtr());
      m_factory->copy( m_acc->auxid(), dstiface, index,
                      *element.container(), element.index(), 1 );

      // Return gracefully.
      return StatusCode::SUCCESS;
    }





    ElementProcessorRegular::ElementProcessorRegular(const std::string& sgName)
    : asg::AsgMessaging( ("CP::TreeBranchHelpers::ElementProcessorRegular/" + sgName).c_str() ),
      m_sgName(sgName) {

    }

    StatusCode ElementProcessorRegular::
    retrieveProcess( StoreType& evtStore ) {

      // Retrieve the object:
      static const bool ALLOW_MISSING = false;
      const SG::AuxElement* el = getElement( m_sgName,
                                            evtStore,
                                            ALLOW_MISSING, msg() );
      if( ! el ) {
        ATH_MSG_ERROR( "Failed to retrieve object \"" << m_sgName
                        << "\"" );
        return StatusCode::FAILURE;
      }
      const SG::AuxElement& element = *el;

      // Process all branches.
      for( auto& p : m_branches ) {
        ATH_CHECK( p->process( element, msg() ) );
      }

      // Return gracefully.
      return StatusCode::SUCCESS;
    }

    StatusCode ElementProcessorRegular::
    addBranch( TTree& tree, const BranchConfig& branchConfig, OutputBranchData& outputData ) {

      // Set up the new branch.
      m_branches.emplace_back(std::make_unique<ElementBranchProcessor>());
      ATH_CHECK( m_branches.back()->setup( tree, branchConfig, outputData, msg() ) );

      // Return gracefully.
      return StatusCode::SUCCESS;
    }

    ContainerProcessorRegular::ContainerProcessorRegular(const std::string& sgName)
    : asg::AsgMessaging( ("CP::TreeBranchHelpers::ContainerProcessorRegular/" + sgName).c_str() ),
      m_sgName(sgName) {

    }

    StatusCode ContainerProcessorRegular::
    retrieveProcess( StoreType& evtStore ) {

      // Retrieve the container:
      static const bool ALLOW_MISSING = false;
      const TClass* cl = nullptr;
      const SG::AuxVectorBase* vec = getVector( m_sgName,
                                                evtStore,
                                                ALLOW_MISSING, cl, msg() );
      if( ! vec ) {
        ATH_MSG_ERROR( "Failed to retrieve container \""
                        << m_sgName << "\"" );
        return StatusCode::FAILURE;
      }
      const SG::AuxVectorBase& container = *vec;

      // Get the collection proxy for the type if it's not available yet.
      if( ! m_collProxy ) {

        // Get the collection proxy from the dictionary.
        m_collProxy = cl->GetCollectionProxy();
        if( ! m_collProxy ) {
            ATH_MSG_ERROR( "No collection proxy provided by type: "
                          << cl->GetName() );
            return StatusCode::FAILURE;
        }

        // Get the offset that one needs to use to get from the element
        // pointers to SG::AuxElement pointers.
        static const TClass* const auxElementClass =
            TClass::GetClass( typeid( SG::AuxElement ) );
        m_auxElementOffset =
            m_collProxy->GetValueClass()->GetBaseClassOffset( auxElementClass );
        if( m_auxElementOffset < 0 ) {
            ATH_MSG_ERROR( "Vector element type \""
                          << m_collProxy->GetValueClass()->GetName()
                          << "\" doesn't seem to inherit from \""
                          << auxElementClass->GetName() << "\"" );
            return StatusCode::FAILURE;
        }
      }

      // Set up the iteration over the elements of the container. In a really
      // low level / ugly way...
      void* cPtr =
        const_cast< void* >( static_cast< const void* >( &container ) );
      TVirtualCollectionProxy::TPushPop helper( m_collProxy, cPtr );
      const UInt_t cSize = m_collProxy->Size();

      // Tell all branch processors to resize their variables.
      for( auto& p : m_branches ) {
        ATH_CHECK( p->resize( cSize, msg() ) );
      }

      // Now iterate over the container.
      for( UInt_t i = 0; i < cSize; ++i ) {

        // Get the element.
        char* elPtr = static_cast< char* >( m_collProxy->At( i ) );
        if( ! elPtr ) {
            ATH_MSG_ERROR( "Failed to get element " << i << " from container" );
            return StatusCode::FAILURE;
        }
        const SG::AuxElement* element =
            reinterpret_cast< const SG::AuxElement* >( elPtr +
                                                      m_auxElementOffset );

        // Execute all branch processors on this element.
        for( auto& p : m_branches ) {
            ATH_CHECK( p->process( *element, i, msg() ) );
        }
      }

      // Return gracefully.
      return StatusCode::SUCCESS;
    }

    StatusCode ContainerProcessorRegular::
    addBranch( TTree& tree, const BranchConfig& branchConfig, OutputBranchData& outputData ) {

      // Set up the new branch.
      m_branches.emplace_back(std::make_unique<ContainerBranchProcessor>());
      ATH_CHECK( m_branches.back()->setup( tree, branchConfig, outputData, msg() ) );

      // Return gracefully.
      return StatusCode::SUCCESS;
    }

    ElementProcessorMet::ElementProcessorMet (const std::string& sgName, const std::string& termName)
    : asg::AsgMessaging( ("CP::TreeBranchHelpers::ElementProcessorMet/" + sgName).c_str() ),
      m_sgName(sgName),
      m_termName(termName) {

    }

    StatusCode ElementProcessorMet::
    retrieveProcess( StoreType& evtStore ) {

        const xAOD::MissingETContainer *met = nullptr;
        ANA_CHECK (evtStore.retrieve (met, m_sgName));
        const SG::AuxElement& element = *(*met)[m_termName];
        // Process all branches.
        for( auto& p : m_branches ) {
          ATH_CHECK( p->process( element, msg() ) );
        }

        // Return gracefully.
        return StatusCode::SUCCESS;
    }

    StatusCode ElementProcessorMet::
    addBranch( TTree& tree, const BranchConfig& branchConfig, OutputBranchData& outputData ) {

        // Set up the new branch.
        m_branches.emplace_back(std::make_unique<ElementBranchProcessor>());
        ATH_CHECK( m_branches.back()->setup( tree, branchConfig, outputData, msg() ) );

        // Return gracefully.
        return StatusCode::SUCCESS;
    }



    StatusCode ProcessorList ::
    setupTree(const std::vector<std::string>& branches, std::unordered_set<std::string> nonContainers, ISystematicsSvc& sysSvc, TTree& tree) {

      m_nonContainers = std::move (nonContainers);

      std::vector<BranchConfig> branchConfigs;
      branchConfigs.reserve( branches.size() );
      for ( const std::string& branchDecl : branches ) {
        branchConfigs.emplace_back();
        ATH_CHECK( branchConfigs.back().parse( branchDecl, msg() ) );
        if (!branchConfigs.back().basketSize.has_value())
          branchConfigs.back().basketSize = defaultBasketSize;
      }

      // This will loop over all branches, collect the name of any
      // aux-store decorations that have no type and report them at the
      // end. This allows to get the full list of missing decorations in
      // a single run, as opposed to having to re-run the job once per
      // missing decoration.
      std::set<std::string> decosWithoutType;
      for (auto& branchConfig : branchConfigs) {
        ATH_CHECK ( branchConfig.configureTypes (decosWithoutType, msg()) );
      }
      if (!decosWithoutType.empty()) {
        msg() << MSG::ERROR << "The following decorations have no type information:";
        for (const auto& deco : decosWithoutType) {
          msg() << " " << deco;
        }
        msg() << endmsg;
        return StatusCode::FAILURE;
      }


      for (auto& branchConfig : branchConfigs) {
        ATH_CHECK ( branchConfig.configureSystematics (sysSvc, msg()) );
      }

      auto sysVector = sysSvc.makeSystematicsVector();
      // Ensure that the nominal systematic is first
      if (!sysVector.at(0).empty()) {
        ATH_MSG_ERROR ("The first systematic in the list is not nominal!");
        return StatusCode::FAILURE;
      }

      // The branches we intend to write out
      std::vector<OutputBranchData> outputBranches;

      // All the branches that will be created
      std::unordered_set<std::string> allBranches;

      // Iterate over the branch specifications.
      for( const auto& branchConfig : branchConfigs ) {

        // All the branches that will be created for this rule
        std::unordered_set<std::string> branchesForRule;

        // Consider all systematics but skip the nominal one
        for( const auto& sys : sysVector ) {

            if (branchConfig.nominalOnly && !sys.empty()) continue;
            OutputBranchData outputData;
            outputData.branchConfig = &branchConfig;
            outputData.sysIndex = &sys - &sysVector.front();
            ATH_CHECK( outputData.configureNames (branchConfig, sys, sysSvc, msg()) );

            // Skip branches that have already been created for other
            // systematics for this rule. That's mostly nominal, but for
            // systematics correlation studies it can also do other things.
            if (branchesForRule.contains(outputData.branchName))
            {
              ANA_MSG_VERBOSE ("Branch \"" << outputData.branchName << "\" for rule \"" << branchConfig.branchDecl << "\" and systematic \"" << sys.name() << "\" already exists, skipping." );
              continue;
            }
            branchesForRule.insert(outputData.branchName);

            // If this branch already exists from another rule, report
            // it as an error.
            if (allBranches.contains(outputData.branchName))
            {
              ANA_MSG_ERROR ("Branch \"" << outputData.branchName << "\" would be created twice!" );
              return StatusCode::FAILURE;
            }
            allBranches.insert(outputData.branchName);
            outputBranches.push_back(outputData);
         }
      }

      // Group all branches by systematic index to ensure that when
      // reading a single systematic the branches are contiguous on
      // disk.
      std::stable_sort (outputBranches.begin(), outputBranches.end(),
                        [](const OutputBranchData& a, const OutputBranchData& b) {
                          return a.sysIndex < b.sysIndex; });

      for (auto &outputData : outputBranches)
         ATH_CHECK( setupBranch( *outputData.branchConfig, outputData, tree ) );

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   StatusCode ProcessorList::setupBranch( const BranchConfig& branchConfig, OutputBranchData& outputData, TTree& tree ) {

      ATH_CHECK( getObjectProcessor( branchConfig, outputData.sgName ).addBranch( tree,
                                                branchConfig, outputData ) );
      ATH_MSG_DEBUG( "Writing branch \"" << outputData.branchName
                      << "\" from container/variable \"" << outputData.sgName
                      << "." << outputData.auxName << "\"" );

      // Return gracefully.
      return StatusCode::SUCCESS;
    }

    StatusCode ProcessorList ::
    process (StoreType& evtStore)
    {
      // Process the standalone objects:
      for( auto& [name, processor] : m_processors )
      {
         // Process it:
         ATH_CHECK (processor->retrieveProcess (evtStore));
      }
      return StatusCode::SUCCESS;
    }



    IObjectProcessor& ProcessorList ::
    getObjectProcessor( const BranchConfig& branchConfig, const std::string& sgName )
    {
      std::string processorName = sgName;
      if (!branchConfig.metTermName.empty())
        processorName += ":metTerm=" + branchConfig.metTermName;

      if (auto iter = m_processors.find(processorName); iter != m_processors.end())
        return *iter->second;

      if (!branchConfig.metTermName.empty())
        return *m_processors.emplace (processorName, std::make_unique<ElementProcessorMet>(sgName, branchConfig.metTermName)).first->second;

      if (m_nonContainers.contains(sgName))
        return *m_processors.emplace (processorName, std::make_unique<ElementProcessorRegular>(sgName)).first->second;

      return *m_processors.emplace (processorName, std::make_unique<ContainerProcessorRegular>(sgName)).first->second;
    }
  }
}