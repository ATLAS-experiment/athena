// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//
#ifndef ASGANALYSISALGORITHMS_ASGXAODMETNTUPLEMAKERALG_H
#define ASGANALYSISALGORITHMS_ASGXAODMETNTUPLEMAKERALG_H

// // Framework include(s):
#include "AnaAlgorithm/AnaAlgorithm.h"
#include "AsgMessaging/AsgMessaging.h"
#include "AsgServices/ServiceHandle.h"
#include "AsgTools/PropertyWrapper.h"
#include "CxxUtils/checker_macros.h"
#include "SystematicsHandles/ISystematicsSvc.h"

// EDM include(s):
#include "AthContainers/AuxElement.h"

namespace CP {

   /// Algorithm that can write MET variables to a simple ntuple from
   /// xAOD objects/variables
   ///
   /// This is a version of \ref AsgxAODNTupleMakerAlg specifically
   /// for writing out the MET variables.  MET variables are special
   /// in that they are in a container, but we really only care about
   /// the last entry in that container (i.e. "Final").  So this
   /// algorithm retrieves the container and then only writes out
   /// variables for a single AuxElement.  For details on this
   /// algorithm please check the original algorithm.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch> (for original algorithm)
   /// @author Nils Krumnack <Nils.Erik.Krumnack@cern.ch> (for this algorithm)
   ///
   class ATLAS_NOT_THREAD_SAFE AsgxAODMetNTupleMakerAlg : public EL::AnaAlgorithm {

   public:
      /// Algorithm constructor
      AsgxAODMetNTupleMakerAlg( const std::string& name, ISvcLocator* svcLoc );

      /// @name Functions inherited from @c EL::AnaAlgorithm
      /// @{

      /// Function executed as part of the job initialisation
      StatusCode initialize() override;

      /// Function executed once per event
      StatusCode execute() override;

      /// Function executed as part of the job finalisation
      StatusCode finalize() override;

      /// @}

   private:
      /// Function setting up the internal data structures on the first event
      StatusCode setupTree();

      /// Function setting up an individual branch on the first event
      StatusCode setupBranch( const std::string &branchDecl,
                              const CP::SystematicSet &sys );

      /// @name Algorithm properties
      /// @{

      /// The name of the output tree to write
      Gaudi::Property<std::string> m_treeName {this, "TreeName", "physics", "Name of the tree to write"};
      /// The branches to write into this output tree
      Gaudi::Property<std::vector< std::string >> m_branches {this, "Branches", {}, "Branches to write to the output tree"};

      /// @}

      /// Class writing all variables from one standalone object
      ///
      /// It is designed to work with any type inheriting from
      /// @c SG::AuxElement. Like @c xAOD::EventInfo. Which is its main user
      /// at the moment...
      ///
      class ElementProcessor : public asg::AsgMessaging {

      public:
         /// Default constructor
         ///
         /// We have to have a default constructor to initialise the
         /// @c asg::AsgMessaging base class correctly. Members of this class
         /// would not need an explicit constructor themselves.
         ///
         ElementProcessor();

         /// Process the object
         ///
         /// This function is called during the event processing to extract
         /// all configured variables from a standalone xAOD object into the
         /// output variables set up using @c ElementProcessor::addBranch.
         ///
         /// @param element The xAOD (interface) object to process
         /// @return The usual @c StatusCode values
         ///
         StatusCode process( const SG::AuxElement& element );

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
         StatusCode addBranch( TTree& tree, const std::string& auxName,
                               const std::string& branchName,
                               bool allowMissing,
                               bool &created );

      private:
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
         class BranchProcessor {

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
            StatusCode setup( TTree& tree, const std::string& auxName,
                              const std::string& branchName,
                              MsgStream& msg );

            /// Function processing the object, filling the variable
            ///
            /// This function is called by @c ElementProcessor, to extract one
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

         }; // class BranchProcessor

         /// List of branch processors set up for this xAOD object
         ///
         /// Note that when we set up a branch, we tell @c TTree to remember a
         /// physical address in memory. To make sure that the address of the
         /// object held by the branch processors are not moved in memory after
         /// their construction, we have to use an @c std::list container here.
         /// @c std::vector would not work. (As it can relocate objects when
         /// increasing the size of the container.)
         ///
         std::list< BranchProcessor > m_branches;

      }; // class ElementProcessor

      /// @name Variables used for the TTree filling
      /// @{

      /// The tree being written
      TTree* m_tree = nullptr;

      /// Objects to write branches from
      std::unordered_map< std::string, ElementProcessor > m_elements;

      /// Internal status flag, showing whether the algorithm is initialised
      ///
      /// This is necessary because we can only set up the output @c TTree while
      /// processing the first event, we can't do it in
      /// @c CP::AsgxAODMetNTupleMakerAlg::initialize.
      ///
      bool m_isInitialized = false;

      Gaudi::Property<std::string> m_termName {this, "termName", "Final", "the name of the MissingET term to save"};

      /// \brief the handle for the systematics service
      ServiceHandle<ISystematicsSvc> m_systematicsService {this, "systematicsSvc", "SystematicsSvc", "systematics service"};

      /// @}

   }; // class AsgxAODMetNTupleMakerAlg

} // namespace CP

#endif // ASGANALYSISALGORITHMS_ASGXAODNTUPLEMAKERALG_H
