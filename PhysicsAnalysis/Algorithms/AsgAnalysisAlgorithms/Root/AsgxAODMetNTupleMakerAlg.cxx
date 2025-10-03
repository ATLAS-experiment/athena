// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s):
#include "AsgAnalysisAlgorithms/AsgxAODMetNTupleMakerAlg.h"

// EDM include(s):
#include "AthContainers/normalizedTypeinfoName.h"
#include "xAODMissingET/MissingETContainer.h"

// ROOT include(s):
#include <TTree.h>

// System include(s):
#include <regex>

// Framework include(s):
#include "PATInterfaces/SystematicSet.h"


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

   /// Check if an aux item exists in the aux store
   ///
   /// @param key The name of the container in the event store
   /// @return True if branch exists, false if not
   ///
   bool auxItemExists( const std::string& key ) {
      // Get a pointer to the vector factory.
      const SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();

      // Try to find the aux item
      return reg.findAuxID( key ) != SG::null_auxid;
   }
} // private namespace

namespace CP {

   AsgxAODMetNTupleMakerAlg::AsgxAODMetNTupleMakerAlg( const std::string& name,
                                                 ISvcLocator* svcLoc )
   : EL::AnaAlgorithm( name, svcLoc ) {
   }

   StatusCode AsgxAODMetNTupleMakerAlg::initialize() {

      // Check that at least one branch is configured.
      if( m_branches.empty() ) {
         ATH_MSG_ERROR( "No branches set up for writing" );
         return StatusCode::FAILURE;
      }

      // Set up the systematics list.
      ATH_CHECK( m_systematicsService.retrieve() );

      // Reset the initialisation flag:
      m_isInitialized = false;

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   StatusCode AsgxAODMetNTupleMakerAlg::execute() {

      // Initialise the processor objects on the first event.
      if( ! m_isInitialized ) {
         // Initialise the output tree.
         m_tree = tree( m_treeName );
         if( ! m_tree ) {
            ATH_MSG_ERROR( "Could not find output tree \"" << m_treeName
                           << "\"" );
            return StatusCode::FAILURE;
         }
         // Call the setup function.
         ATH_CHECK( setupTree() );
         // The processor objects are now set up.
         m_isInitialized = true;
      }

      // Process the standalone objects:
      for( auto& element_itr : m_elements ) {
         const xAOD::MissingETContainer *met = nullptr;
         ANA_CHECK (evtStore()->retrieve (met, element_itr.first));
         ANA_CHECK( element_itr.second.process( *(*met)[m_termName] ) );
      }

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   StatusCode AsgxAODMetNTupleMakerAlg::finalize() {

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   StatusCode AsgxAODMetNTupleMakerAlg::setupTree() {

      // First process nominal
      CP::SystematicSet nominal{};
      for( const std::string& branchDecl : m_branches ) {
         ATH_CHECK( setupBranch( branchDecl, nominal ) );
      }

      // Consider all systematics but skip the nominal one
      for( const auto& sys : m_systematicsService->makeSystematicsVector() ) {
         // Nominal already processed
         if( sys.empty() ) {
            continue;
         }

         // Iterate over the branch specifications.
         for( const std::string& branchDecl : m_branches ) {
            ATH_CHECK( setupBranch( branchDecl, sys ) );
         }
      }

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   StatusCode AsgxAODMetNTupleMakerAlg::setupBranch( const std::string &branchDecl,
                                                  const CP::SystematicSet &sys ) {

      // The regular expression used to extract the needed info. The logic
      // is supposed to be:
      //
      // (match[1]).(match[2])<any whitespace>-><any whitespace>(match[3])
      //
      // Like:
      //    "Electrons.eta  -> el_eta"
      //
      // , where we would pick up "Electrons", "eta" and "el_eta" as the
      // three words using this regexp.
      static const std::regex
         re( "\\s*([\\w%]+)\\.([\\w%]+)\\s*->\\s*([\\w%]+)" );

      // Interpret this branch declaration.
      std::smatch match;
      if( ! std::regex_match( branchDecl, match, re ) ) {
         ATH_MSG_ERROR( "Expression \"" << branchDecl
                        << "\" doesn't match \"<object>.<variable> ->"
                        " <branch>\"" );
         return StatusCode::FAILURE;
      }

      // Check if we are running nominal
      bool nominal = sys.empty();
      // Check if we are affected by the systematics
      bool systematicsContainer{false};
      bool systematicsDecoration{false};
      bool affectedContainer{true};
      bool affectedDecoration{true};

      // Event store key for the object under consideration.
      std::string key = match[ 1 ];
      if( key.find( "%SYS%" ) != std::string::npos )
      {
         systematicsContainer = true;
         const CP::SystematicSet affecting = m_systematicsService->getObjectSystematics( key );
         CP::SystematicSet matching;
         ANA_CHECK( SystematicSet::filterForAffectingSystematics( sys, affecting, matching ) );
         if( !nominal && matching.empty() ) {
            ATH_MSG_VERBOSE( "Container \"" << key << "\" is not affected by systematics \"" << sys.name() << "\"" );
            affectedContainer = false;
         }
         ANA_CHECK( m_systematicsService->makeSystematicsName( key, match[ 1 ], matching ) );
      }
      // Auxiliary variable name for the object under consideration.
      std::string auxName = match[ 2 ];
      if( auxName.find( "%SYS%" ) != std::string::npos )
      {
         systematicsDecoration = true;
         CP::SystematicSet affecting = m_systematicsService->getDecorSystematics( match[ 1 ], auxName );
         if( affecting.empty() )
         {
            // Sometimes while object systematics were applied we are not interested in them,
            // NOSYS will then be used on the container name.
            // Decoration systematics however will only be aware of containers with %SYS% included.
            // Some special handling is needed to translate from NOSYS back to %SYS%.
            const auto nosysInKey = key.find( "NOSYS" );
            if( nosysInKey != std::string::npos )
            {
               std::string sysKey = key;
               sysKey.replace (nosysInKey, 5, "%SYS%");
               // these will be all systematics (object+decor)
               const CP::SystematicSet affectingDecor = m_systematicsService->getDecorSystematics( sysKey, auxName );
               // we now need to filter-out object systematics
               const CP::SystematicSet affectingObject = m_systematicsService->getObjectSystematics( sysKey );
               for( const CP::SystematicVariation &variation : affectingDecor )
               {
                  if( affectingObject.find( variation ) == affectingObject.end() )
                  {
                     affecting.insert( variation );
                  }
               }
            }
         }
         CP::SystematicSet matching;
         ANA_CHECK( SystematicSet::filterForAffectingSystematics( sys, affecting, matching ) );
         if( !nominal && matching.empty() ) {
            ATH_MSG_VERBOSE( "Decoration \"" << auxName << "\" is not affected by systematics \"" << sys.name() << "\"" );
            affectedDecoration = false;
         }
         ANA_CHECK( m_systematicsService->makeSystematicsName( auxName, match[ 2 ], matching ) );
      }

      // Ignore the branch if neither container nor decoration are affected by the systematic
      if( !nominal
       && ( ( systematicsContainer && systematicsDecoration && !affectedContainer && !affectedDecoration )
         || ( !systematicsContainer && systematicsDecoration && !affectedDecoration )
         || ( systematicsContainer && !systematicsDecoration && !affectedContainer ) ) )
      {
         ANA_MSG_VERBOSE( "Neither container nor decoration are affected by systematics \"" << sys.name() << "\""
                          << " for branch rule \"" << branchDecl << "\"" );
         return StatusCode::SUCCESS;
      }

      // Branch name for the variable.
      std::string brName = match[ 3 ];
      if( brName.find( "%SYS%" ) != std::string::npos )
         ANA_CHECK (m_systematicsService->makeSystematicsName( brName, match[ 3 ], sys ));

      // If the %SYS% pattern was not used in this setup, then stop
      // on non-nominal systematic.
      if( ! nominal &&
          ( key == match[ 1 ] ) && ( auxName == match[ 2 ] ) &&
          ( brName == match[ 3 ] ) ) {
         return StatusCode::SUCCESS;
      }

      // Check that we use the %SYS% pattern reasonably in the names.
      if( ( ( key == match[ 1 ] ) && ( auxName == match[ 2 ] ) &&
            ( brName != match[ 3 ] ) ) ||
            ( ( ( key != match[ 1 ] ) || ( auxName != match[ 2 ] ) ) &&
            ( brName == match[ 3 ] ) ) ) {
         ATH_MSG_ERROR( "The systematic variation pattern is used "
                        "inconsistently in: \"" << branchDecl
                        << "\"" );
         return StatusCode::FAILURE;
      }

      // Flag keeping track whether any branch was set up for this rule.
      static const bool ALLOW_MISSING = false;
      bool branchCreated = false;

      {
        bool created = false;
        ATH_CHECK( m_elements[ key ].addBranch( *m_tree,
                                                auxName,
                                                brName,
                                                ALLOW_MISSING,
                                                created ) );
        if( created ) {
          ATH_MSG_DEBUG( "Writing branch \"" << brName
                         << "\" from object/variable \"" << key
                         << "." << auxName << "\"" );
          branchCreated = true;
        } else {
          ATH_MSG_DEBUG( "Skipping branch \"" << brName
                         << "\" from object/variable \"" << key
                         << "." << auxName << "\"" );
        }
      }

      // Check if the rule was meaningful or not:
      if( ! branchCreated ) {
         ATH_MSG_ERROR( "No branch was created for rule: \""
                        << branchDecl << "\""
                        << " and systematics: \""
                        << sys.name() << "\"" );
         return StatusCode::FAILURE;
      }

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   AsgxAODMetNTupleMakerAlg::ElementProcessor::ElementProcessor()
   : asg::AsgMessaging( "CP::AsgxAODMetNTupleMakerAlg::ElementProcessor" ) {

   }

   StatusCode AsgxAODMetNTupleMakerAlg::ElementProcessor::
   process( const SG::AuxElement& element ) {

      // Process all branches.
      for( BranchProcessor& p : m_branches ) {
         ATH_CHECK( p.process( element, msg() ) );
      }

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   StatusCode AsgxAODMetNTupleMakerAlg::ElementProcessor::
   addBranch( TTree& tree, const std::string& auxName,
              const std::string& branchName,
              bool allowMissing,
              bool &created ) {

      /// Helper class for finding an already existing branch processor.
      class BranchFinder {
      public:
         /// Type of the predicate's argument
         typedef const BranchProcessor& argument_type;
         /// Constructor with key/name
         BranchFinder( const std::string& branchName ) : m_name( branchName ) {}
         /// Operator evaluating whether this is the branch we're looking for
         bool operator()( argument_type bp ) const {
            return ( bp.m_branchName == m_name );
         }
      private:
         std::string m_name; ///< Name of the branch
      }; // class BranchFinder

      // Check if the corresponding aux item exists
      bool validAuxItem = auxItemExists( auxName );
      if( ! validAuxItem ) {
         if( allowMissing ) {
            // Return gracefully.
            ATH_MSG_DEBUG( "Aux item \"" << auxName
                           << "\" not readable for branch \""
                           << branchName << "\"" );
            return StatusCode::SUCCESS;
         } else {
            // Return gracefully.
            ATH_MSG_ERROR( "Aux item \"" << auxName
                           << "\" not readable for branch \""
                           << branchName << "\"" );
            return StatusCode::FAILURE;
         }
      }

      // Check whether this branch is already set up:
      auto itr = std::find_if( m_branches.begin(), m_branches.end(),
                               BranchFinder( branchName ) );
      if( itr != m_branches.end() ) {
         ATH_MSG_WARNING( "Duplicate setup received for branch: " << branchName );
         return StatusCode::SUCCESS;
      }

      created = true;

      // Set up the new branch.
      m_branches.emplace_back();
      ATH_CHECK( m_branches.back().setup( tree, auxName, branchName, msg() ) );

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   StatusCode
   AsgxAODMetNTupleMakerAlg::ElementProcessor::BranchProcessor::
   setup( TTree& tree, const std::string& auxName,
          const std::string& branchName, MsgStream& msg ) {

      // Remember the branch name.
      m_branchName = branchName;

      // Create the accessor.
      m_acc.reset( new SG::TypelessConstAccessor( auxName ) );

      // Get a pointer to the vector factory.
      const SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();
      const std::type_info* ti = reg.getType( m_acc->auxid() );
      if( ! ti ) {
         msg << MSG::ERROR
             << "No std::type_info available for auxiliary variable: "
             << auxName << endmsg;
         return StatusCode::FAILURE;
      }
      m_factory = reg.getFactory( m_acc->auxid() );
      if( ! m_factory ) {
         msg << MSG::ERROR << "No factory found for auxiliary variable: "
             << auxName << endmsg;
         return StatusCode::FAILURE;
      }

      // Create the data object.
      m_data = m_factory->create( m_acc->auxid(), 1, 1, false );

      // Pointer to the branch, to be created.
      TBranch* br = nullptr;

      // Decide whether we're dealing with a "primitive" or an "object" branch.
      if( strlen( ti->name() ) == 1 ) {

         // This is a "primitive" variable...

         // Get the type identifier for it that ROOT will understand.
         const char rType = rootType( ti->name()[ 0 ], msg );
         if( rType == '\0' ) {
            msg << MSG::ERROR << "Type not recognised for variable: "
                << branchName << endmsg;
            return StatusCode::FAILURE;
         }

         // Construct the type description.
         std::ostringstream typeDesc;
         typeDesc << branchName << "/" << rType;

         // Create the primitive branch.
         br = tree.Branch( branchName.c_str(), m_data->toPtr(),
                           typeDesc.str().c_str() );

      } else {

         // This is an "object" variable...

         // Get a proper type name for the variable.
         const std::string typeName = SG::normalizedTypeinfoName( *ti );

         // Access the dictionary for the type.
         TClass* cl = TClass::GetClass( *ti );
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
         br = tree.Branch( branchName.c_str(), cl->GetName(), &m_dataPtr );

      }

      // Check that the branch creation succeeded.
      if( ! br ) {
         msg << MSG::ERROR << "Failed to create branch: " << branchName
             << endmsg;
         return StatusCode::FAILURE;
      }

      // Return gracefully.
      return StatusCode::SUCCESS;
   }

   StatusCode
   AsgxAODMetNTupleMakerAlg::ElementProcessor::BranchProcessor::
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

} // namespace CP
