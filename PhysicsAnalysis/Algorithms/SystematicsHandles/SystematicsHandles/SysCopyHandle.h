/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef SYSTEMATICS_HANDLES__SYS_COPY_HANDLE_H
#define SYSTEMATICS_HANDLES__SYS_COPY_HANDLE_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/VarHandleKey.h>
#include <AsgMessaging/AsgMessagingForward.h>
#include <AsgMessaging/MsgStream.h>
#include <AsgMessaging/StatusCode.h>
#include <AthContainers/CurrentContext.h>
#include <PATInterfaces/SystematicSet.h>
#include <SystematicsHandles/ISysHandleBase.h>
#include <SystematicsHandles/ISystematicsSvc.h>
#include <SystematicsHandles/SysListHandle.h>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace CP
{
  /// @brief a systematics data handle that will either copy the input object or
  /// act like an update handle
  ///
  /// There are several use cases for this data handle:

  /// * It needs a mutable container, e.g. to apply momentum corrections to it.
  ///   This is the main use case and other use cases either derive from it or are
  ///   discouraged.
  /// * In needs to add systematic variations to the object.  This will
  ///   generally coincide with changing the object, as otherwise the systematics
  ///   are usually applied to the decorations instead of the container.
  /// * It needs to copy the input object, generally because it comes directly
  ///   from the input file and we want to make a copy so we don't add decorations
  ///   to the input object.  This is generally discouraged and instead it is
  ///   preferred to schedule a separate algorithm to do the copy.
  ///
  /// Normally this handle will be configured both with the name of the input
  /// object and the name of the output object to create.  In that case it will
  /// create a shallow copy of the input object and add it to the event store.
  ///
  /// Alternatively if the second name is not given, it will do a non-const
  /// retrieve of the input object and effectively act like an update handle.
  /// This is essentially an optimization to avoid shallow copies when no
  /// systematics are added.  This is not common and may break some assumptions
  /// in Athena.
  ///
  /// The template parameter `T` can optionally be `const` qualified to indicate
  /// that only a `const` object will be needed.  This can be used if only the
  /// copy behavior of this handle is needed, but the copy won't be modified.
  /// That is a rather specialized behavior, since normally we only need copies
  /// to modify them.  As such it is recommended to switch those algorithms to a
  /// `SysReadHandle` and schedule an `CP::AsgShallowCopyAlg` beforehand to do
  /// the copy.

  template<typename T> class SysCopyHandle final
    : public ISysObjectHandleBase, public asg::AsgMessagingForward
  {
    /// Public Members
    /// ==============
  public:

    /// \brief Standard constructor
    /// \tparam T2 The type of the owner
    /// \param owner Used to declare the property and for its messaging
    /// \param propertyName The name of the property to declare. An additional
    ///        propertyName+"Out" property will be declared to set the output
    ///        name
    /// \param propertyValue The default value for the property
    /// \param propertyDescription The description of the property
    ///
    /// This version of the constructor declares a property on the parent object
    /// and should usually be preferred when the container to be copied should
    /// be configurable
    template<typename T2>
    SysCopyHandle (T2 *owner, const std::string& propertyName,
                   const std::string& propertyValue,
                   const std::string& propertyDescription);


    /// \brief Construct directly without declaring properties
    /// \tparam T2 The owner that provides the messaging and event store
    /// \param inputName The name of the input container
    /// \param outputName The name of the output container (acts like an update
    ///        handle if set to the empty string)
    /// \param owner The owner that provides the messaging and event store
    template<typename T2>
    SysCopyHandle (const std::string &inputName, const std::string &outputName, T2 *owner);


    /// \brief whether we have a name configured
    virtual bool empty () const noexcept override;

    /// \brief !empty()
    explicit operator bool () const noexcept;

    /// \brief get the name pattern before substitution
    virtual std::string getNamePattern () const override;


    /// \brief initialize this handle
    /// \{
    StatusCode initialize (SysListHandle& sysListHandle);
    StatusCode initialize (SysListHandle& sysListHandle, SG::AllowEmptyEnum);
    /// \}


    /// \brief retrieve the object for the given name
    ::StatusCode getCopy (T*& object,
                          const CP::SystematicSet& sys,
                          const EventContext& ctx = Gaudi::Hive::currentContext()) const;



    /// Inherited Members
    /// =================
  private:

    virtual CP::SystematicSet getInputAffecting (const ISystematicsSvc& svc) const override;
    virtual StatusCode
    fillSystematics (const ISystematicsSvc& svc,
                     const CP::SystematicSet& fullAffecting,
                     const std::vector<CP::SystematicSet>& sysList) override;
    virtual StatusCode
    addDecorationDependency (const ISystematicsSvc& svc, const std::string& decoName, bool decoWrite) override;



    /// Private Members
    /// ===============
  private:

    /// \brief the input name we use
    std::string m_inputName;

    /// \brief the (optional) name of the copy we create
    std::string m_outputName;

    /// \brief the (optional) type of the container
    ///
    /// This is needed to declare outputs in AthenaMT with the correct
    /// type.
    std::string m_typeName;

    /// \brief the data held per-systematic (filled in `initialize`)
    struct SysData
    {
      /// \brief retrieve the (possibly copied) object for this systematic
      ///
      /// Depending on how the handle is configured this either does a
      /// plain (update/read) retrieve of the input via the event store,
      /// or reads the input through a read handle and (shallow) copies
      /// it into the configured output.
      std::function<StatusCode(MsgStream&, T*&, const EventContext&)> getCopy;
    };
    std::unordered_map<CP::SystematicSet,SysData> m_sysData;


    /// \brief the type of the event store we use
    typedef std::decay<decltype(
      *(std::declval<EL::AnaAlgorithm>().evtStore()))>::type StoreType;

    /// \brief the event store we use
    ///
    /// This is only used in the update/read mode (no output name
    /// configured), where we need a non-const retrieve of the input
    /// object that a read handle can not provide.
    StoreType *m_evtStore = nullptr;

    /// \brief the function to retrieve the event store
    ///
    /// This is an std::function to allow the parent to be either a
    /// tool or an algorithm.  Though we are not really supporting
    /// tools as parents when using \ref SysListHandle, so in
    /// principle this could be replaced with a pointer to the
    /// algorithm instead.
    std::function<StoreType*()> m_evtStoreGetter;

#ifndef XAOD_STANDALONE
    /// \brief a function to add a data dependency to the parent algorithm
    ///
    /// This wraps the owner's addDependency call and is used by
    /// addDecorationDependency to register MT dependencies.
    std::function<void(const DataObjID&, Gaudi::DataHandle::Mode)> m_addAlgDependency;
#endif
  };
}

#include "SysCopyHandle.icc"

#endif
