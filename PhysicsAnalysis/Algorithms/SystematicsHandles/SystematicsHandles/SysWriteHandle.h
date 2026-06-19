/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef SYSTEMATICS_HANDLES__SYS_WRITE_HANDLE_H
#define SYSTEMATICS_HANDLES__SYS_WRITE_HANDLE_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgDataHandles/WriteHandleKey.h>
#include <AsgMessaging/AsgMessagingForward.h>
#include <AthContainers/CurrentContext.h>
#include <PATInterfaces/SystematicSet.h>
#include <SystematicsHandles/ISysHandleBase.h>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>

class StatusCode;

namespace CP
{
  class SysListHandle;
  class SystematicSet;


  /// \brief a data handle for writing systematics varied input data

  template<typename T,typename Aux = void> class SysWriteHandle final
    : public ISysObjectHandleBase, public asg::AsgMessagingForward
  {
    /// Public Members
    /// ==============
  public:

    /// \brief Standard constructor
    /// \tparam T2 The type of the owner
    /// \param owner Used to declare the property and for its messaging
    /// \param propertyName The name of the property to declare
    /// \param propertyValue The default value for the property
    /// \param propertyDescription The description of the property
    ///
    /// This version of the constructor declares a property on the parent object
    /// and should usually be preferred when the container to be written should
    /// be configurable
    template<typename T2>
    SysWriteHandle (T2 *owner, const std::string& propertyName,
                    const std::string& propertyValue,
                    const std::string& propertyDescription);

    /// \brief Direct constructor which doesn't declare a property
    template<typename T2>
    SysWriteHandle (const std::string &outputName, T2 *owner);

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


    /// \brief get the name we record to the event store
    const std::string& getName (const CP::SystematicSet& sys) const;


    /// \brief record the object for the given systematic
    template<typename X = Aux,typename = std::enable_if_t<std::is_same_v<X,void>>>
    ::StatusCode record (std::unique_ptr<T> object,
                         const CP::SystematicSet& sys,
                         const EventContext& ctx = Gaudi::Hive::currentContext()) const;


    /// \brief record the object and aux store for the given systematic
    template<typename X = Aux,typename = std::enable_if_t<!std::is_same_v<X,void>>>
    ::StatusCode record (std::unique_ptr<T> object,
                         std::unique_ptr<Aux> aux,
                         const CP::SystematicSet& sys,
                         const EventContext& ctx = Gaudi::Hive::currentContext()) const;



    /// Inherited Members
    /// ================
  private:

    virtual CP::SystematicSet
    getInputAffecting (const ISystematicsSvc& svc) const override;
    virtual StatusCode
    fillSystematics (const ISystematicsSvc& svc,
                     const CP::SystematicSet& fullAffecting,
                     const std::vector<CP::SystematicSet>& sysList) override;
    virtual StatusCode
    addDecorationDependency (const ISystematicsSvc& svc, const std::string& decoName, bool decoWrite) override;



    /// Private Members
    /// ===============
  private:

    /// \brief the output name we use
    std::string m_outputName;

    /// \brief the explicit list of type names for output dependencies
    std::string m_outputType;

    /// \brief the data held per-systematic (filled in `initialize`)
    struct SysData
    {
      /// the write handle key for the (expanded) output container
      SG::WriteHandleKey<T> writeHandle;
    };
    std::unordered_map<CP::SystematicSet,SysData> m_sysData;

    /// \brief get the write handle key for the given systematic
    const SG::WriteHandleKey<T>& getWriteHandle (const CP::SystematicSet& sys) const;


#ifndef XAOD_STANDALONE
    /// \brief a function to add a data dependency to the parent algorithm
    ///
    /// This wraps the owner's addDependency call and is used by
    /// addDecorationDependency to register MT dependencies.
    std::function<void(const DataObjID&, Gaudi::DataHandle::Mode)> m_addAlgDependency;
#endif
  };
}

#include "SysWriteHandle.icc"

#endif
