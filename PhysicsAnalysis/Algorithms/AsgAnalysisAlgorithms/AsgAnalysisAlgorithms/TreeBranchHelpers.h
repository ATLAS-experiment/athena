// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//
#ifndef ASGANALYSISALGORITHMS_TREEBRANCHHELPERS_H
#define ASGANALYSISALGORITHMS_TREEBRANCHHELPERS_H
// System include(s):
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <list>

// Framework include(s):
#include "AsgMessaging/AsgMessaging.h"
#include "AsgMessaging/AsgMessagingForward.h"
#include "AsgServices/ServiceHandle.h"
#include "AnaAlgorithm/AnaAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "SystematicsHandles/SysListHandle.h"
#include <AsgTools/PropertyWrapper.h>

// EDM include(s):
#include "AthContainersInterfaces/IAuxTypeVector.h"
#include "AthContainers/AuxElement.h"

// Forward declaration(s):
class TClass;
class TTree;
class TVirtualCollectionProxy;
namespace SG {
   class AuxVectorBase;
   class IAuxTypeVectorFactory;
}

namespace CP
{
  /// @brief a namespace for helper functions and objects for filling
  /// tree branches
  ///
  /// There used to be quite a bit of logic and member classes inside
  /// `AsgxAODNTupleMakerAlg`, which has been moved into standalone code
  /// during refactoring. That makes it easier to reorganize the code,
  /// and if needed also share it better. There also used to be an
  /// algorithm `AsgxAODMetNTupleMakerAlg`, that had a lot of overlap
  /// and movign the code here allowed to share code and eventually
  /// merge all functionality into the main algorithm.

  namespace TreeBranchHelpers
  {
    /// the type of the event store in the current environment
#ifdef XAOD_STANDALONE
    using StoreType = asg::SgTEvent;
#else
    using StoreType = StoreGateSvc;
#endif



    /// @brief the user configuration of an output branch
    struct BranchConfig
    {
      /// the original user configuration string
      std::string branchDecl;

      /// the SG name of the object to read from
      std::string sgName;

      /// the aux data variable name to read from
      std::string auxName;

      /// the name of the output branch
      std::string branchName;

      /// the name of the type (or empty to read from aux-registry)
      std::string typeName;

      /// whether we only want to write out the nominal
      bool nominalOnly = false;

      /// MET ONLY: the name of the MET term to write out
      std::string metTermName;


      /// the aux-id for the nominal decoration
      SG::auxid_t nominalAuxId = SG::null_auxid;

      /// the type of the decoration we read
      const std::type_info* auxType = nullptr;

      /// the vector type of the decoration we read
      const std::type_info* auxVecType = nullptr;

      /// pointer to the aux vector factory
      const SG::IAuxTypeVectorFactory* auxFactory = nullptr;


      /// the affecting systematics for the sgName
      CP::SystematicSet sgNameFilterSys;

      /// the affecting systematics for the auxName
      CP::SystematicSet auxNameFilterSys;

      /// the affecting systematics for the branchName
      CP::SystematicSet branchNameFilterSys;


      /// parse the user configuration string
      StatusCode parse (const std::string& branchDecl, MsgStream& msg);

      /// configure the associated decoration types
      StatusCode configureTypes (std::set<std::string>& decosWithoutType, MsgStream& msg);

      /// configure the systematics
      StatusCode configureSystematics (ISystematicsSvc& sysSvc, MsgStream& msg);
    };



    /// @brief the data for a single output branch
    struct OutputBranchData
    {
      /// the @ref BranchConfig we are based on
      const BranchConfig *branchConfig = nullptr;

      /// the index in the systematics list
      std::size_t sysIndex = 0u;

      /// whether this is unaffected by systematics (i.e. nominal)
      bool isNominal = false;

      /// the SG name of the object to read from
      std::string sgName;

      /// @brief the name of the decoration in the aux-store
      std::string auxName;

      /// @brief the name of the output branch
      std::string branchName;


      /// @brief configure names for systematics
      StatusCode configureNames (const BranchConfig& branchConfig, const CP::SystematicSet& sys, ISystematicsSvc& sysSvc, MsgStream& msg);
    };



    /// @brief the interface class for branch processors
    class IBranchProcessor
    {
    public:
      /// virtual destructor
      virtual ~IBranchProcessor() = default;
    };



    /// @brief the interface class for classes reading an object from
    /// the event store and processing it
    ///
    /// The idea is that I need a different class depending on whether I
    /// have to read a standalone object, a container object, or an
    /// individual MET term. The hope is that by having a common
    /// interface for all of them, I only need to distinguish between
    /// them where there is an actual difference in handling.
    class IObjectProcessor
    {
    public:
      /// virtual destructor
      virtual ~IObjectProcessor() = default;

      /// @brief retrieve and process the object
      ///
      /// This function is called during the event processing to first
      /// retrieve the object from the event store, and then to extract
      /// all configured variables from the xAOD object into the output
      /// variables set up via @ref addBranch.
      virtual StatusCode retrieveProcess (StoreType& evtStore) = 0;

      /// Add one branch to the output tree
      ///
      /// This function is used during the setup of the output tree to create
      /// one branch in it, from one specific auxiliary variable. The type of
      /// the variable is figured out at runtime using the auxiliary store
      /// infrastructure.
      ///
      /// @param tree The tree to create the branch in
      /// @param auxName Name of the auxiliary variable to create the branch
      ///                from
      /// @param branchName The name of the branch to create in the tree
      /// @return The usual @c StatusCode values
      ///
      virtual StatusCode addBranch( TTree& tree, const BranchConfig& branchConfig, OutputBranchData& outputData ) = 0;
    };





    /// Class writing one variable from an xAOD object into a branch
    ///
    /// It is used for both setting up the branch in the outut @c TTree
    /// during the setup of the tree, and then to fill the "output
    /// variable" with the right payload during the event processing.
    ///
    /// Note that since we may have a *lot* of such objects, I didn't want
    /// to make it inherit from @c asg::AsgMessaging. Which means that all
    /// of the class's functions need to receive its parent's message
    /// stream object to be able to log error messages "nicely".
    ///
    /// Also note that since this is very much an internal class, all of
    /// its members are public. Since the owner of such objects should know
    /// perfectly well how they behave.
    ///
    class ElementBranchProcessor : public IBranchProcessor {

    public:
      /// Function setting up the object, and the branch
      ///
      /// This is pretty much the constructor of the class. I just decided
      /// to implement it as a regular function and not a "real"
      /// constructor, to be able to return a @c StatusCode value from the
      /// call. Since the setup of the object may very well fail.
      ///
      /// @param tree The tree to set up the new branch in
      /// @param auxName The name of the auxiliary variable to create
      ///                a branch from
      /// @param branchName Name of the branch to create in the tree
      /// @param msg Reference to the parent's @c MsgStream object
      /// @return The usual @c StatusCode values
      ///
      StatusCode setup( TTree& tree, const BranchConfig& branchConfig, OutputBranchData& outputData, MsgStream& msg );

      /// Function processing the object, filling the variable
      ///
      /// This function is called by @c ElementProcessorRegular, to extract one
      /// variable from the standalone object, and move its payload into
      /// the memory address from which the output tree is writing its
      /// branch.
      ///
      /// @param element The standalone object to get the auxiliary
      ///                variable from
      /// @param msg Reference to the parent's @c MsgStream object
      /// @return The usual @c StatusCode values
      ///
      StatusCode process( const SG::AuxElement& element,
                          MsgStream& msg );

      /// Name of the branch being written
      std::string m_branchName;
      /// Object accessing the variable in question
      std::unique_ptr< SG::TypelessConstAccessor > m_acc;
      /// Pointer to the helper object that handles this variable
      const SG::IAuxTypeVectorFactory* m_factory = nullptr;
      /// The object managing the memory of the written variable
      std::unique_ptr< SG::IAuxTypeVector > m_data;
      /// Helper variable, pointing at the object to be written
      void* m_dataPtr = nullptr;

    }; // class ElementBranchProcessor



    /// Class writing one variable from an xAOD object into a branch
    ///
    /// It is used for both setting up the branch in the outut @c TTree
    /// during the setup of the tree, and then to fill the "output
    /// variable" with the right payload during the event processing.
    ///
    /// Note that since we may have a *lot* of such objects, I didn't want
    /// to make it inherit from @c asg::AsgMessaging. Which means that all
    /// of the class's functions need to receive its parent's message
    /// stream object to be able to log error messages "nicely".
    ///
    /// Also note that since this is very much an internal class, all of
    /// its members are public. Since the owner of such objects should know
    /// perfectly well how they behave.
    ///
    /// Finally, note that it is more complicated than the
    /// @c ElementBranchProcessor class. Since in this case we
    /// need to explicitly deal with @c std::vector types, which we need to
    /// fill explicitly when extracting the variables from the xAOD
    /// objects.
    ///
    class ContainerBranchProcessor : public IBranchProcessor {

    public:
      /// Function setting up the object, and the branch
      StatusCode setup( TTree& tree, const BranchConfig& branchConfig, OutputBranchData& outputData, MsgStream& msg );
      /// Function (re)sizing the variable for a new event
      StatusCode resize( size_t size, MsgStream& msg );
      /// Function processing the object, filling the variable
      StatusCode process( const SG::AuxElement& element, size_t index,
                          MsgStream& msg );

      /// Name of the branch being written
      std::string m_branchName;
      /// Object accessing the variable in question
      std::unique_ptr< SG::TypelessConstAccessor > m_acc;
      /// Pointer to the helper object that handles this variable
      const SG::IAuxTypeVectorFactory* m_factory = nullptr;
      /// The object managing the memory of the written variable
      std::unique_ptr< SG::IAuxTypeVector > m_data;
      /// Helper variable, pointing at the object to be written
      void* m_dataPtr = nullptr;

    }; // class ContainerBranchProcessor




    /// Class writing all variables from one standalone object
    ///
    /// It is designed to work with any type inheriting from
    /// @c SG::AuxElement. Like @c xAOD::EventInfo. Which is its main user
    /// at the moment...
    ///
    class ElementProcessorRegular : public asg::AsgMessaging, public IObjectProcessor {

    public:
      /// Default constructor
      ///
      /// We have to have a default constructor to initialise the
      /// @c asg::AsgMessaging base class correctly. Members of this class
      /// would not need an explicit constructor themselves.
      ///
      ElementProcessorRegular(const std::string& sgName);

      /// retrieve and process the object
      StatusCode retrieveProcess (StoreType& evtStore) override;

      /// Add one branch to the output tree
      ///
      /// This function is used during the setup of the output tree to create
      /// one branch in it, from one specific auxiliary variable. The type of
      /// the variable is figured out at runtime using the auxiliary store
      /// infrastructure.
      ///
      /// @param tree The tree to create the branch in
      /// @param auxName Name of the auxiliary variable to create the branch
      ///                from
      /// @param branchName The name of the branch to create in the tree
      /// @param allowMissing Set to @c true to print an error message in case
      ///                     of a failure
      /// @param created Used to store if the branch was actually created
      /// @return The usual @c StatusCode values
      ///
      StatusCode addBranch( TTree& tree, const BranchConfig& branchConfig, OutputBranchData& outputData ) override;

    private:

      /// List of branch processors set up for this xAOD object
      ///
      /// Note that when we set up a branch, we tell @c TTree to remember a
      /// physical address in memory. To make sure that the address of the
      /// object held by the branch processors are not moved in memory after
      /// their construction, we have to use an @c std::list container here.
      /// @c std::vector would not work. (As it can relocate objects when
      /// increasing the size of the container.)
      ///
      std::vector<std::unique_ptr<ElementBranchProcessor>> m_branches;

      /// Name of the object in the event store
      std::string m_sgName;

    }; // class ElementProcessorRegular


    /// Class writing all variables from one @c DataVector container
    ///
    /// It is designed to work with *any* @c DataVector<SG::AuxElement> type,
    /// it doesn't have to be an @c xAOD::IParticleContainer. But of course
    /// that is the main use case for it...
    ///
    /// It expects an @c SG::AuxVectorBase object from the caller, iterates
    /// over the elements of that container using the ROOT dictionary of the
    /// type, and writes individual variables from the elements of the
    /// container using the same machinery that @c ElementProcessorRegular employs.
    ///
    class ATLAS_NOT_THREAD_SAFE ContainerProcessorRegular : public asg::AsgMessaging, public IObjectProcessor {

    public:
      /// Default constructor
      ///
      /// We have to have a default constructor to initialise the
      /// @c asg::AsgMessaging base class correctly. Members of this class
      /// would not need an explicit constructor themselves.
      ///
      ContainerProcessorRegular(const std::string& sgName);

      /// retrieve and process the object
      StatusCode retrieveProcess (StoreType& evtStore) override;

      /// Add one branch to the output tree
      ///
      /// This function is used during the setup of the output tree to create
      /// one branch in it, from one specific auxiliary variable. The type of
      /// the variable is figured out at runtime using the auxiliary store
      /// infrastructure.
      ///
      /// @param tree The tree to create the branch in
      /// @param auxName Name of the auxiliary variable to create the branch
      ///                from
      /// @param branchName The name of the branch to create in the tree
      /// @param allowMissing Set to @c true to print an error message in case
      ///                     of a failure
      /// @param created Used to store if the branch was actually created
      /// @return The usual @c StatusCode values
      ///
      StatusCode addBranch( TTree& tree, const BranchConfig& branchConfig, OutputBranchData& outputData ) override;

    private:
      /// List of branch processors set up for this xAOD object
      ///
      /// Note that when we set up a branch, we tell @c TTree to remember a
      /// physical address in memory. To make sure that the address of the
      /// object held by the branch processors are not moved in memory after
      /// their construction, we have to use an @c std::list container here.
      /// @c std::vector would not work. (As it can relocate objects when
      /// increasing the size of the container.)
      ///
      std::vector<std::unique_ptr<ContainerBranchProcessor>> m_branches;
      /// Collection proxy used for iterating over the container
      TVirtualCollectionProxy* m_collProxy = nullptr;
      /// Offset of the element type to @c SG::AuxElement
      int m_auxElementOffset = -1;

      /// Name of the object in the event store
      std::string m_sgName;

    }; // class ContainerProcessorRegular



    /// Class writing all variables from one standalone object
    ///
    /// It is designed to work with any type inheriting from
    /// @c SG::AuxElement. Like @c xAOD::EventInfo. Which is its main user
    /// at the moment...
    ///
    class ElementProcessorMet : public asg::AsgMessaging, public IObjectProcessor {

    public:
      /// Default constructor
      ///
      /// We have to have a default constructor to initialise the
      /// @c asg::AsgMessaging base class correctly. Members of this class
      /// would not need an explicit constructor themselves.
      ///
      ElementProcessorMet (const std::string& sgName, const std::string& termName);

      /// retrieve and process the object
      StatusCode retrieveProcess (StoreType& evtStore) override;

      /// Add one branch to the output tree
      ///
      /// This function is used during the setup of the output tree to create
      /// one branch in it, from one specific auxiliary variable. The type of
      /// the variable is figured out at runtime using the auxiliary store
      /// infrastructure.
      ///
      /// @param tree The tree to create the branch in
      /// @param auxName Name of the auxiliary variable to create the branch
      ///                from
      /// @param branchName The name of the branch to create in the tree
      /// @param allowMissing Set to @c true to print an error message in case
      ///                     of a failure
      /// @param created Used to store if the branch was actually created
      /// @return The usual @c StatusCode values
      ///
      StatusCode addBranch( TTree& tree, const BranchConfig& branchConfig, OutputBranchData& outputData ) override;

    private:

      /// List of branch processors set up for this xAOD object
      ///
      /// Note that when we set up a branch, we tell @c TTree to remember a
      /// physical address in memory. To make sure that the address of the
      /// object held by the branch processors are not moved in memory after
      /// their construction, we have to use an @c std::list container here.
      /// @c std::vector would not work. (As it can relocate objects when
      /// increasing the size of the container.)
      ///
      std::vector<std::unique_ptr<ElementBranchProcessor>> m_branches;

      /// Name of the object in the event store
      std::string m_sgName;

      /// Name of the MET term to retrieve
      std::string m_termName;

    }; // class ElementProcessorMet



    class ProcessorList : public asg::AsgMessagingForward {
    public:
      using AsgMessagingForward::AsgMessagingForward;

      /// Function setting up the internal data structures on the first
      /// event for regular branches
      StatusCode setupTree(const std::vector<std::string>& branches, std::unordered_set<std::string> nonContainers, ISystematicsSvc& sysSvc, TTree& tree );

      /// Function setting up an individual branch on the first event
      StatusCode setupBranch( const BranchConfig& branchConfig, OutputBranchData& outputData, TTree& tree );

      StatusCode process (StoreType& evtStore);

      IObjectProcessor& getObjectProcessor (const BranchConfig& branchConfig, const std::string& sgName);

      /// the non-containers
      std::unordered_set<std::string> m_nonContainers;

      /// object processors
      std::unordered_map< std::string, std::unique_ptr<IObjectProcessor>> m_processors;
    };
  }
}

#endif