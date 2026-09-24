/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "AsgAnalysisAlgorithms/RNtupleFieldHelpers.h"

// EDM include(s):
#include "AthContainersInterfaces/IAuxTypeVectorFactory.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/normalizedTypeinfoName.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxVectorBase.h"
#include "xAODRootAccess/tools/THolder.h"
#include "xAODMissingET/MissingETContainer.h"
#include "AsgAnalysisAlgorithms/TreeBranchHelpers.h"

// ROOT include(s):
#include <TClass.h>
#include <TVirtualCollectionProxy.h>
#include <ROOT/RNTupleModel.hxx>

// System include(s):
#include <iostream>
#include <regex>
#include <algorithm>

namespace {
    // Mirroring TempInterface from TreeBranchHelpers.cxx
    class TempInterface : public SG::AuxVectorData {
    public:
        TempInterface( size_t size, SG::auxid_t auxid, void* ptr ) : m_size(size) {
            setCache( auxid, ptr );
        }
        using AuxVectorData::setStore;
        virtual size_t size_v() const { return m_size; }
        virtual size_t capacity_v() const { return m_size; }
    private:
        size_t m_size;
    };

    // Helper to abstract .data() access, as std::vector<bool> does not support it
    template<typename T>
    void* getVectorData( std::vector<T>& v ) {
        return v.data();
    }
    
    // Specialization for vector<bool>
    void* getVectorData( std::vector<bool>& ) {
        return nullptr;
    }

    // Helper to generate RNTuple fields based on type_info
    template <typename TYPE>
    std::shared_ptr<void> makeScalarField(ROOT::RNTupleModel& model,
                                        const std::string& fieldName,
                                        void*& rawPtr) {
        auto ptr = model.MakeField<TYPE>(fieldName);
        rawPtr = ptr.get();
        return ptr;
    }

    template <typename TYPE>
    std::shared_ptr<void> makeVecField(ROOT::RNTupleModel& model,
                                           const std::string& fieldName,
                                           void*& rawPtr,
                                           CP::RNtupleFieldHelpers::FieldOps& ops) {
        auto ptr = model.MakeField<std::vector<TYPE>>(fieldName);
        rawPtr = ptr.get();
        ops.resize = [ptr](size_t n){ ptr->resize(n); };
        ops.getData = [ptr]() -> void* { return getVectorData(*ptr); };
        return ptr;
    }

    std::shared_ptr<void> makeField( ROOT::RNTupleModel& model,
                                     const std::string& fieldName,
                                     const std::type_info& type_info,
                                     void*& rawPtr,
                                     CP::RNtupleFieldHelpers::FieldOps& ops,
                                     MsgStream& msg ) {

        using namespace ROOT;

        // Scalar types
        if ( type_info == typeid(float) ) return makeScalarField<float>(model, fieldName, rawPtr);
        if ( type_info == typeid(double) ) return makeScalarField<double>(model, fieldName, rawPtr);
        if ( type_info == typeid(int) ) return makeScalarField<int>(model, fieldName, rawPtr);
        if ( type_info == typeid(unsigned int) ) return makeScalarField<unsigned int>(model, fieldName, rawPtr);
        if ( type_info == typeid(short) ) return makeScalarField<short>(model, fieldName, rawPtr);
        if ( type_info == typeid(unsigned short) ) return makeScalarField<unsigned short>(model, fieldName, rawPtr);
        if ( type_info == typeid(long) ) return makeScalarField<long>(model, fieldName, rawPtr);
        if ( type_info == typeid(unsigned long) ) return makeScalarField<unsigned long>(model, fieldName, rawPtr);
        if ( type_info == typeid(long long) ) return makeScalarField<long long>(model, fieldName, rawPtr);
        if ( type_info == typeid(unsigned long long) ) return makeScalarField<unsigned long long>(model, fieldName, rawPtr);
        if ( type_info == typeid(char) ) return makeScalarField<char>(model, fieldName, rawPtr);
        if ( type_info == typeid(unsigned char) ) return makeScalarField<unsigned char>(model, fieldName, rawPtr);
        if ( type_info == typeid(bool) ) return makeScalarField<bool>(model, fieldName, rawPtr);

        // Vector types
        if ( type_info == typeid(std::vector<float>) ) return makeVecField<float>(model, fieldName, rawPtr, ops);
        if ( type_info == typeid(std::vector<double>) ) return makeVecField<double>(model, fieldName, rawPtr, ops);
        if ( type_info == typeid(std::vector<int>) ) return makeVecField<int>(model, fieldName, rawPtr, ops);
        if ( type_info == typeid(std::vector<unsigned int>) ) return makeVecField<unsigned int>(model, fieldName, rawPtr, ops);
        if ( type_info == typeid(std::vector<short>) ) return makeVecField<short>(model, fieldName, rawPtr, ops);
        if ( type_info == typeid(std::vector<unsigned short>) ) return makeVecField<unsigned short>(model, fieldName, rawPtr, ops);
        if ( type_info == typeid(std::vector<long>) ) return makeVecField<long>(model, fieldName, rawPtr, ops);
        if ( type_info == typeid(std::vector<unsigned long>) ) return makeVecField<unsigned long>(model, fieldName, rawPtr, ops);
        if ( type_info == typeid(std::vector<long long>) ) return makeVecField<long long>(model, fieldName, rawPtr, ops);
        if ( type_info == typeid(std::vector<unsigned long long>) ) return makeVecField<unsigned long long>(model, fieldName, rawPtr, ops);
        if ( type_info == typeid(std::vector<char>) ) return makeVecField<char>(model, fieldName, rawPtr, ops);
        if ( type_info == typeid(std::vector<unsigned char>) ) return makeVecField<unsigned char>(model, fieldName, rawPtr, ops);
        if ( type_info == typeid(std::vector<bool>) ) return makeVecField<bool>(model, fieldName, rawPtr, ops);
        if ( type_info == typeid(std::vector<std::string>) ) return makeVecField<std::string>(model, fieldName, rawPtr, ops);

        if ( type_info == typeid(std::string) ) {
            auto ptr = model.MakeField<std::string>(fieldName);
            rawPtr = ptr.get();
            return ptr;
        }

        msg << MSG::ERROR << "Unsupported type for RNTuple field \"" << fieldName << "\"" << endmsg;
        return nullptr;
    }

    bool auxItemExists( const std::string& key ) {
        const SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();
        return reg.findAuxID( key ) != SG::null_auxid;
    }

#ifdef XAOD_STANDALONE

   const SG::AuxVectorBase* getVector( const std::string& key,
                                       asg::SgEvent& evtStore,
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
         const std::type_info* ti = holder->getTypeInfo();
         cl = TClass::GetClass( *ti );
      } else {
         cl = TClass::GetClass( typeid( *c ) );
      }
      if( ( allowMissing == false ) && ( cl == nullptr ) ) {
         msg << MSG::ERROR
             << "Couldn't find TClass dictionary for container \"" << key
             << "\"" << endmsg;
         return nullptr;
      }
      return c;
   }

   const SG::AuxElement* getElement( const std::string& key,
                                     asg::SgEvent& evtStore,
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

   class ProxyWithName {
   public:
      typedef const SG::DataProxy* argument_type;
      ProxyWithName( const std::string& name ) : m_name( name ) {}
      bool operator()( argument_type proxy ) const {
         return ( proxy->name() == m_name );
      }
   private:
      std::string m_name;
   };

   const SG::AuxVectorBase* getVector ATLAS_NOT_CONST_THREAD_SAFE ( const std::string& key,
                                                                    IProxyDict& evtStore,
                                                                    bool allowMissing,
                                                                    const TClass*& cl,
                                                                    MsgStream& msg ) {

      auto proxies = evtStore.proxies();
      proxies.erase( std::remove_if( proxies.begin(), proxies.end(),
                                     std::not_fn( ProxyWithName( key ) ) ),
                     proxies.end() );
      for( const SG::DataProxy* proxy : proxies ) {
         SG::DataProxy* proxy_nc = const_cast< SG::DataProxy* >( proxy );
         DataBucketBase* bucket =
            dynamic_cast< DataBucketBase* >( proxy_nc->accessData() );
         if( ! bucket ) {
            msg << MSG::ERROR
                << "Couldn't access data object as a data bucket?!?" << endmsg;
            return nullptr;
         }
         cl = TClass::GetClass( bucket->tinfo() );
         if( ! cl ) {
            if( msg.level() <= MSG::VERBOSE ) {
               msg << MSG::VERBOSE << "No dictionary found for: "
                   << bucket->tinfo().name() << endmsg;
            }
            continue;
         }
         if( ! cl->InheritsFrom( "SG::AuxVectorBase" ) ) {
            if( msg.level() <= MSG::VERBOSE ) {
               msg << MSG::VERBOSE << "Object \"" << key << "/" << cl->GetName()
                   << "\" does not inherit from SG::AuxVectorBase" << endmsg;
            }
            continue;
         }
         const SG::AuxVectorBase* result =
            reinterpret_cast< const SG::AuxVectorBase* >( bucket->object() );
         return result;
      }

      if( ! allowMissing ) {
         msg << MSG::ERROR << "Couldn't retrieve object \"" << key
             << "\" as SG::AuxVectorBase" << endmsg;
      }
      return nullptr;
   }

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

}

namespace CP {
   namespace RNtupleFieldHelpers {
      // ======================================================================
      // ElementFieldProcessor
      // ======================================================================

    StatusCode ElementFieldProcessor::setup( ROOT::RNTupleModel& model,
                                               const BranchConfig& branchConfig,
                                               OutputBranchData& outputData,
                                               MsgStream& msg ) {
        m_fieldName = outputData.branchName;
        m_acc = std::make_unique<SG::TypelessConstAccessor>( *branchConfig.auxType, outputData.auxName );
        
        if( branchConfig.auxFactory && branchConfig.auxType ) {
            m_factory = branchConfig.auxFactory;
            const std::type_info* type_info = branchConfig.auxType;
            
            FieldOps dummyOps;
            m_field = makeField( model, m_fieldName, *type_info, m_dataPtr, dummyOps, msg );
        } else {
            msg << MSG::ERROR << "BranchConfig not properly configured for " << outputData.auxName << endmsg;
            return StatusCode::FAILURE;
        }
        
        return m_field ? StatusCode::SUCCESS : StatusCode::FAILURE;
    }
    
    StatusCode ElementFieldProcessor::setup( TTree& /*tree*/, 
                                               const BranchConfig& /*branchConfig*/, 
                                               OutputBranchData& /*outputData*/, 
                                               MsgStream& msg ) {
        msg << MSG::ERROR << "setup(TTree, ...) called, but only setup(ROOT::RNTupleModel, ...) should be implemented for this processor" << endmsg;
        return StatusCode::FAILURE;
    }

    StatusCode ElementFieldProcessor::process( const SG::AuxElement& element, MsgStream& /*msg*/ ) {
        // Use High-Level copy via TempInterface
        // copy( auxid, dst_container, dst_index, src_container, src_index, n )
        TempInterface dstiface( 1, m_acc->auxid(), m_dataPtr );
        m_factory->copy( m_acc->auxid(), dstiface, 0, *element.container(), element.index(), 1 );
        return StatusCode::SUCCESS;
    }


      // ======================================================================
      // ContainerFieldProcessor
      // ======================================================================

    StatusCode ContainerFieldProcessor::setup( ROOT::RNTupleModel& model,
                                                const BranchConfig& branchConfig,
                                                OutputBranchData& outputData,
                                                MsgStream& msg ) {
        m_fieldName = outputData.branchName;
        m_acc = std::make_unique<SG::TypelessConstAccessor>( *branchConfig.auxType, outputData.auxName );
        
        if( branchConfig.auxFactory && branchConfig.auxVecType ) {
            m_factory = branchConfig.auxFactory;
            const std::type_info* type_info = branchConfig.auxVecType;
            if( *type_info == typeid(std::vector<bool>) ) {
                // std::vector<bool> provides no contiguous storage to copy into
                msg << MSG::ERROR << "std::vector<bool> is not supported for container field " << m_fieldName << endmsg;
                return StatusCode::FAILURE;
            }
            
            m_field = makeField( model, m_fieldName, *type_info, m_dataPtr, m_ops, msg );
        } else {
            msg << MSG::ERROR << "BranchConfig not properly configured for " << outputData.auxName << endmsg;
            return StatusCode::FAILURE;
        }
        
        if( !m_field ) return StatusCode::FAILURE;

        if( !m_ops.resize || !m_ops.getData ) {
            msg << MSG::ERROR << "Container field " << m_fieldName << " must be a vector type" << endmsg;
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }

    StatusCode ContainerFieldProcessor::setup( TTree& /*tree*/, 
                                               const BranchConfig& /*branchConfig*/, 
                                               OutputBranchData& /*outputData*/, 
                                               MsgStream& msg ) {
        msg << MSG::ERROR << "setup(TTree, ...) called, but only setup(ROOT::RNTupleModel, ...) should be implemented for this processor" << endmsg;
        return StatusCode::FAILURE;
    }

      StatusCode ContainerFieldProcessor::resize( size_t size, MsgStream& /*msg*/ ) {
        if( m_ops.resize ) {
            m_ops.resize( size );
            return StatusCode::SUCCESS;
        }
        return StatusCode::FAILURE;
    }

    StatusCode ContainerFieldProcessor::process( const SG::AuxElement& element, size_t index, MsgStream& msg ) {
        void* rawDataPtr = getData();
        if( !rawDataPtr ) {
            msg << MSG::ERROR << "No data available for " << m_fieldName << endmsg;
            return StatusCode::FAILURE;
        }

        try {
            TempInterface dstiface( index + 1, m_acc->auxid(), rawDataPtr );
            m_factory->copy( m_acc->auxid(), dstiface, index, *element.container(), element.index(), 1 );
        } catch( ... ) {
            msg << MSG::ERROR << "Failed to copy data for " << m_fieldName << endmsg;
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }


      // ======================================================================
      // ElementProcessor
      // ======================================================================

    ElementProcessor::ElementProcessor(const std::string& sgName)
        : asg::AsgMessaging( "CP::RNtupleFieldHelpers::ElementProcessor/" + sgName ), m_sgName(sgName) {}

    StatusCode ElementProcessor::retrieveProcess( StoreType& evtStore ) {
        static const bool ALLOW_MISSING = false;
        const SG::AuxElement* el = getElement( m_sgName,
                                               evtStore,
                                               ALLOW_MISSING, msg() );
        if( ! el ) {
            ATH_MSG_ERROR( "Failed to retrieve object \"" << m_sgName << "\"" );
            return StatusCode::FAILURE;
        }

        for( auto& p : m_fields ) {
            ATH_CHECK( p->process( *el, msg() ) );
        }
        return StatusCode::SUCCESS;
    }

    StatusCode ElementProcessor::addBranch( ROOT::RNTupleModel& model,
                                            const BranchConfig& branchConfig,
                                            OutputBranchData& outputData ) {
        if( !auxItemExists(outputData.auxName) ) {
            ATH_MSG_ERROR("Aux item " << outputData.auxName << " missing");
            return StatusCode::FAILURE;
        }
        m_fields.emplace_back( std::make_unique<ElementFieldProcessor>() );
        ATH_CHECK( m_fields.back()->setup( model, branchConfig, outputData, msg() ) );
        return StatusCode::SUCCESS;
    }

    StatusCode ElementProcessor::addBranch( TTree& /*tree*/,
                                            const BranchConfig& /*branchConfig*/,
                                            OutputBranchData& /*outputData*/ ) {
        ATH_MSG_ERROR("ElementProcessor::addBranch for TTree should not be called");
        return StatusCode::FAILURE;
    }

      // ======================================================================
      // ContainerProcessor
      // ======================================================================

    ContainerProcessor::ContainerProcessor(const std::string& sgName)
        : asg::AsgMessaging( "CP::RNtupleFieldHelpers::ContainerProcessor/" + sgName ), m_sgName(sgName) {}

    StatusCode ContainerProcessor::addBranch( ROOT::RNTupleModel& model,
                                            const BranchConfig& branchConfig,
                                            OutputBranchData& outputData ) {
        if( !auxItemExists(outputData.auxName) ) {
            ATH_MSG_ERROR("Aux item " << outputData.auxName << " missing");
            return StatusCode::FAILURE;
        }
        m_fields.emplace_back( std::make_unique<ContainerFieldProcessor>() );
        ATH_CHECK( m_fields.back()->setup( model, branchConfig, outputData, msg() ) );
        return StatusCode::SUCCESS;
    }

    StatusCode ContainerProcessor::addBranch( TTree& /*tree*/,
                                              const BranchConfig& /*branchConfig*/,
                                              OutputBranchData& /*outputData*/ ) {
        ATH_MSG_ERROR("ContainerProcessor::addBranch for TTree should not be called");
        return StatusCode::FAILURE;
    }

    StatusCode ContainerProcessor::retrieveProcess( StoreType& evtStore ) {
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

        void* cPtr =
            const_cast< void* >( static_cast< const void* >( &container ) );
        TVirtualCollectionProxy::TPushPop helper( m_collProxy, cPtr );
        const UInt_t cSize = m_collProxy->Size();

        for( auto& p : m_fields ) {
            ATH_CHECK( p->resize( cSize, msg() ) );
        }

        for (UInt_t i = 0; i < cSize; ++i) {
            char* elPtr = static_cast< char* >( m_collProxy->At( i ) );
            if( ! elPtr ) {
                ATH_MSG_ERROR( "Failed to get element " << i << " from container" );
                return StatusCode::FAILURE;
            }
            const SG::AuxElement* element =
                reinterpret_cast< const SG::AuxElement* >( elPtr +
                                                        m_auxElementOffset );

            for ( auto& p : m_fields ) {
                ATH_CHECK( p->process( *element, i, msg() ) );
            }
        }
        return StatusCode::SUCCESS;
    }
      // ======================================================================
      // ElementProcessorMET
      // ======================================================================
        ElementProcessorMet::ElementProcessorMet(const std::string& sgName, const std::string& termName)
            : ElementProcessor(sgName),
                m_termName(termName) {
        }
    
    StatusCode ElementProcessorMet::retrieveProcess(StoreType& evtStore) {

      const xAOD::MissingETContainer* met = nullptr;
      ANA_CHECK(evtStore.retrieve(met, m_sgName));
      const xAOD::MissingET *term = (*met)[m_termName];
      if( term == nullptr ) {
        ANA_MSG_ERROR( "MET term " << m_termName << " not found in container " << m_sgName);
        return StatusCode::FAILURE;
      }
      const SG::AuxElement& element = *term;
      // Process all fields.
      for (auto& p : m_fields) {
        ATH_CHECK(p->process(element, msg()));
      }

      return StatusCode::SUCCESS;
    }

      // ======================================================================
      // ProcessorList
      // ======================================================================
    StatusCode ProcessorList::setupTree( const std::vector<std::string>& branches,
                                            std::unordered_set<std::string> nonContainers,
                                            ISystematicsSvc& sysSvc,
                                            ROOT::RNTupleModel& model ) {
        m_nonContainers = std::move(nonContainers);

        std::vector<BranchConfig> branchConfigs;
        branchConfigs.reserve( branches.size() );
        for ( const std::string& branchDecl : branches ) {
            branchConfigs.emplace_back();
            ATH_CHECK( branchConfigs.back().parse( branchDecl, msg() ) );
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
                outputBranches.push_back(std::move(outputData));
            }
        }

        // Group all branches by systematic index to ensure that when
        // reading a single systematic the branches are contiguous on
        // disk.
        std::stable_sort (outputBranches.begin(), outputBranches.end(),
                            [](const OutputBranchData& a, const OutputBranchData& b) {
                            return a.sysIndex < b.sysIndex; });

        for (auto &outputData : outputBranches)
            ATH_CHECK( setupBranch( *outputData.branchConfig, outputData, model ) );

        // Return gracefully.
        return StatusCode::SUCCESS;
    }

    StatusCode ProcessorList::setupBranch( const BranchConfig& branchConfig,
                                            OutputBranchData& outputData,
                                            ROOT::RNTupleModel& model ) {

        ATH_CHECK( getObjectProcessor( branchConfig, outputData.sgName ).addBranch( model, branchConfig, outputData ) );
        ATH_MSG_DEBUG( "Writing RNTuple field \"" << outputData.branchName
                << "\" from container/variable \"" << outputData.sgName
                << "." << outputData.auxName << "\"" );

        return StatusCode::SUCCESS; 
    }

    StatusCode ProcessorList::process( StoreType& evtStore ) {
        for( auto& [name, processor] : m_processors ) {
            ATH_CHECK( processor->retrieveProcess( evtStore ) );
        }
        return StatusCode::SUCCESS;
    }

    TreeBranchHelpers::IObjectProcessor& ProcessorList::getObjectProcessor( const BranchConfig& branchConfig, const std::string& sgName ) {
        std::string processorName = sgName;
        if (!branchConfig.metTermName.empty()) {
            processorName += ":metTerm=" + branchConfig.metTermName;
        }

        if (auto iter = m_processors.find(processorName); iter != m_processors.end()) {
            return *iter->second;
        }
        if (!branchConfig.metTermName.empty())
            return *m_processors.emplace (processorName, std::make_unique<ElementProcessorMet>(sgName, branchConfig.metTermName)).first->second;


        if (m_nonContainers.contains(sgName)) {
            return *(m_processors.emplace(processorName, std::make_unique<ElementProcessor>(sgName)).first->second);
        }
        return *(m_processors.emplace(processorName, std::make_unique<ContainerProcessor>(sgName)).first->second);
    }

} // namespace RNtupleFieldHelpers
} // namespace CP
