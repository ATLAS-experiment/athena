/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef SYSTEMATICS_HANDLES__COPY_HELPERS_H
#define SYSTEMATICS_HANDLES__COPY_HELPERS_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgMessaging/MessageCheck.h>
#include <AsgMessaging/MsgStream.h>
#include <AsgMessaging/StatusCode.h>
#include <CxxUtils/checker_macros.h>
#include <xAODBase/IParticleContainer.h>
#include <xAODBase/IParticleHelpers.h>
#include <xAODCore/ShallowCopy.h>
#include <AthContainers/CurrentContext.h>

#include <memory>
#include <type_traits>
#include <utility> //std::declval

namespace CP
{
  namespace detail
  {
    /// \brief check what kind of object/container the argument is
    template <typename T>
    struct ContainerType
    {
      template <class, class> class checker;

      template <typename C>
      static std::true_type test_iparticle(checker<C, decltype((*(const xAOD::IParticle**)nullptr) = ((C*)nullptr)->at(0))> *);

      template <typename C>
      static std::false_type test_iparticle(...);

      template <typename C>
      static std::true_type test_container(checker<C, decltype((*(const SG::AuxVectorBase**)nullptr) = ((C*)nullptr))>*);

      template <typename C>
      static std::false_type test_container(...);

      /// Value evaluating to:
      ///  - 1 for xAOD::IParticleContainer types;
      ///  - 2 for other DataVector types;
      ///  - 3 for non-vector types.
      static const int value = ( std::is_same< std::true_type, decltype( test_iparticle< T >( nullptr ) ) >::value ?
                                 1 : ( std::is_same< std::true_type, decltype( test_container< T >( nullptr ) ) >::value ?
                                       2 : 3 ) );
    };

    /// \brief a helper class to create shallow copies and register
    /// them in the event store
    ///
    /// The main purpose of this class is to make it (fairly)
    /// straightforward to do partial specializations for base classes
    /// which need special handling to register their objects with the
    /// correct type.
    template<typename T, int type = ContainerType<T>::value>
    struct ShallowCopy
    {
      static_assert ((type==1)||(type==2)||(type==3),
                     "Type can not be shallow copied");
    };

    template<typename T>
    struct ShallowCopy<T,1>
    {
      static StatusCode
      getCopy (MsgStream& msgStream, const EventContext& ctx,
               T*& object, const T *inputObject,
               const std::string& outputName)
      {
        // Define the msg(...) function as a lambda.
        // Suppress thread-checker warning because this provides just a wrapper to MsgStream.
        const auto msg = [&] ATLAS_NOT_THREAD_SAFE (MSG::Level lvl) -> MsgStream& {
           msgStream << lvl;
           return msgStream;
        };

        // Make sure that we get a valid pointer.
        assert (inputObject != nullptr);

        // Handle the case when the input object is a view container.
        if( ! inputObject->getConstStore() ) {

           // Decide how to handle the container.
           if( inputObject->size() ) {
              // Get the pointer to the "owning container" from the first
              // element.
              const T* originContainer =
                 dynamic_cast< const T* >( ( *inputObject )[ 0 ]->container() );
              if (!originContainer){
                ANA_MSG_ERROR( "Dynamic cast failed." );
                return StatusCode::FAILURE;
              }
              // Make sure that every element in the view container has the same
              // parent.
              for( size_t i = 1; i < inputObject->size(); ++i ) {
                 if( ( *inputObject )[ i ]->container() != originContainer ) {
                    ANA_MSG_ERROR( "Not all elements of the received view "
                                   "container come from the same container!" );
                    return StatusCode::FAILURE;
                 }
              }
              // Postfix for the shallow-copy container of the origin container.
              static const char* const ORIGIN_POSTFIX = "_ShallowCopyOrigin";
              // Make a shallow copy of the origin container.
              auto originCopy = xAOD::shallowCopy( *originContainer );
              if( ( ! originCopy.first ) || ( ! originCopy.second ) ) {
                 ANA_MSG_ERROR( "Failed to shallow copy the origin of a view "
                                << "container, meant for: " << outputName );
                 return StatusCode::FAILURE;
              }
              // Make a view copy on top of it.
              auto viewCopy = std::make_unique< T >( SG::VIEW_ELEMENTS );
              auto viewCopyPtr = viewCopy.get();
              for( const auto* element : *inputObject ) {
                 viewCopy->push_back( originCopy.first->at( element->index() ) );
              }

              // ...and record it without locking, since the caller will
              // typically still modify the copy.
              SG::WriteHandle<T> originHandle( outputName + ORIGIN_POSTFIX, ctx );
              ANA_CHECK( originHandle.recordNonConst( std::move(originCopy.first),
                                                      std::move(originCopy.second) ) );

              // Set the origin links on it. Note that
              // xAOD::setOriginalObjectLink's "container version" doesn't work
              // with view containers, we have to call this function one-by-one
              // on the elements.
              for( size_t i = 0; i < inputObject->size(); ++i ) {
                 if( ! xAOD::setOriginalObjectLink( *( ( *inputObject )[ i ] ),
                                                    *( ( *viewCopy )[ i ] ) ) ) {
                    return StatusCode::FAILURE;
                 }
              }
              // Finally, record the view container with the requested name.
              SG::WriteHandle<T> viewHandle( outputName, ctx );
              ANA_CHECK( viewHandle.recordNonConst( std::move(viewCopy) ) );
              // The copy is done.
              object = viewCopyPtr;
              return StatusCode::SUCCESS;
           } else {
              // If the container was empty, then let's just make a new empty
              // container, and that's that...
              auto viewCopy = std::make_unique< T >( SG::VIEW_ELEMENTS );
              auto viewCopyPtr = viewCopy.get();
              SG::WriteHandle<T> viewHandle( outputName, ctx );
              ANA_CHECK( viewHandle.recordNonConst( std::move(viewCopy) ) );
              // The copy is done.
              object = viewCopyPtr;
              return StatusCode::SUCCESS;
           }

        } else {

           // We can just copy the container as is.
           auto copy = xAOD::shallowCopy( *inputObject );
           if (!copy.first || !copy.second)
           {
              ANA_MSG_ERROR ("failed to shallow copy object: " << outputName);
              ANA_MSG_ERROR ("likely shallow copying a view container");
              return StatusCode::FAILURE;
           }

           if (!xAOD::setOriginalObjectLink (*inputObject, *copy.first)) {
              return StatusCode::FAILURE;
           }
           //coverity[WRAPPER_ESCAPE]
           object = copy.first.get();
           // Record the copy and its aux store without locking, since the
           // caller will typically still modify it.
           SG::WriteHandle<T> handle (outputName, ctx);
           ANA_CHECK (handle.recordNonConst (std::move(copy.first), std::move(copy.second)));
           return StatusCode::SUCCESS;
        }
      }
    };

    template<typename T>
    struct ShallowCopy<T,2>
    {
      static StatusCode
      getCopy (MsgStream& msgStream, const EventContext& ctx,
               T*& object, const T *inputObject,
               const std::string& outputName)
      {
         // Define the msg(...) function as a lambda.
         // Suppress thread-checker warning because this provides just a wrapper to MsgStream.
         const auto msg = [&] ATLAS_NOT_THREAD_SAFE (MSG::Level lvl) -> MsgStream& {
            msgStream << lvl;
            return msgStream;
         };

         // Make sure that we get a valid pointer.
         assert (inputObject != nullptr);

         // Handle the case when the input object is a view container.
         if( ! inputObject->getConstStore() ) {

            // Decide how to handle the container.
            if( inputObject->size() ) {
               // Get the pointer to the "owning container" from the first
               // element.
               const T* originContainer = dynamic_cast< const T* >( ( *inputObject )[ 0 ]->container() );
               if (!originContainer){
                 ANA_MSG_ERROR( "Dynamic cast returned nullptr!" );
                 return StatusCode::FAILURE;
               }
               // Make sure that every element in the view container has the same
               // parent.
               for( size_t i = 1; i < inputObject->size(); ++i ) {
                  if( ( *inputObject )[ i ]->container() != originContainer ) {
                     ANA_MSG_ERROR( "Not all elements of the received view "
                                    "container come from the same container!" );
                     return StatusCode::FAILURE;
                  }
               }
               // Postfix for the shallow-copy container of the origin container.
               static const char* const ORIGIN_POSTFIX = "_ShallowCopyOrigin";
               // Make a shallow copy of the origin container.
               auto originCopy = xAOD::shallowCopy( *originContainer );
               if( ( ! originCopy.first ) || ( ! originCopy.second ) ) {
                  ANA_MSG_ERROR( "Failed to shallow copy the origin of a view "
                                 << "container, meant for: " << outputName );
                  return StatusCode::FAILURE;
               }

               // Make a view copy on top of it.
               auto viewCopy = std::make_unique< T >( SG::VIEW_ELEMENTS );
               auto viewCopyPtr = viewCopy.get();
               for( const auto* element : *inputObject ) {
                  viewCopy->push_back( originCopy.first->at( element->index() ) );
               }

               // ...and record it without locking, since the caller will
               // typically still modify the copy.
               SG::WriteHandle<T> originHandle( outputName + ORIGIN_POSTFIX, ctx );
               ANA_CHECK( originHandle.recordNonConst( std::move(originCopy.first),
                                                       std::move(originCopy.second) ) );

               // Finally, record the view container with the requested name.
               SG::WriteHandle<T> viewHandle( outputName, ctx );
               ANA_CHECK( viewHandle.recordNonConst( std::move(viewCopy) ) );
               // The copy is done.
               object = viewCopyPtr;
               return StatusCode::SUCCESS;
            } else {
               // If the container was empty, then let's just make a new empty
               // container, and that's that...
               auto viewCopy = std::make_unique< T >( SG::VIEW_ELEMENTS );
               auto viewCopyPtr = viewCopy.get();
               SG::WriteHandle<T> viewHandle( outputName, ctx );
               ANA_CHECK( viewHandle.recordNonConst( std::move(viewCopy) ) );
               // The copy is done.
               object = viewCopyPtr;
               return StatusCode::SUCCESS;
              }

         } else {

            // We can just copy the container as is.
            auto copy = xAOD::shallowCopy( *inputObject );
            if (!copy.first || !copy.second)
            {
               ANA_MSG_ERROR ("failed to shallow copy object: " << outputName);
               ANA_MSG_ERROR ("likely shallow copying a view container");
               return StatusCode::FAILURE;
            }
            //coverity warns about the bare pointer outliving the 'copy' object
            //coverity[WRAPPER_ESCAPE]
            object = copy.first.get();
            // Record the copy and its aux store without locking, since the
            // caller will typically still modify it.
            SG::WriteHandle<T> handle (outputName, ctx);
            ANA_CHECK (handle.recordNonConst (std::move(copy.first), std::move(copy.second)));
            return StatusCode::SUCCESS;
         }
      }
    };

    template<typename T>
    struct ShallowCopy<T,3>
    {
       static StatusCode
       getCopy (MsgStream& msgStream, const EventContext& ctx,
                T*& object, const T *inputObject,
                const std::string& outputName)
       {
          // Define the msg(...) function as a lambda.
          // Suppress thread-checker warning because this provides just a wrapper to MsgStream.
          const auto msg = [&] ATLAS_NOT_THREAD_SAFE (MSG::Level lvl) -> MsgStream& {
             msgStream << lvl;
             return msgStream;
          };

          // We can just copy the object as is.
          auto copy = xAOD::shallowCopy( *inputObject );
          if (!copy.first || !copy.second)
          {
             ANA_MSG_ERROR ("failed to shallow copy object: " << outputName);
             ANA_MSG_ERROR ("likely shallow copying a view container");
             return StatusCode::FAILURE;
          }
          //coverity[WRAPPER_ESCAPE]
          object = copy.first.get();
          // Record the copy and its aux store without locking, since the
          // caller will typically still modify it.
          SG::WriteHandle<T> handle (outputName, ctx);
          ANA_CHECK (handle.recordNonConst (std::move(copy.first), std::move(copy.second)));
          return StatusCode::SUCCESS;
       }
    };

    template<>
    struct ShallowCopy<xAOD::IParticleContainer>
    {
      static StatusCode
      getCopy (MsgStream& msgStream, const EventContext& ctx,
               xAOD::IParticleContainer*& object,
               const xAOD::IParticleContainer *inputObject,
               const std::string& outputName);
    };
  }
}

#endif
