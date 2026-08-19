/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_CONTAINER_ID_H
#define COLUMNAR_CORE_CONTAINER_ID_H

#include <ColumnarCore/ColumnarDef.h>
#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarInterfaces/ColumnarDef.h>
#include <xAODCore/CLASS_DEF.h>

class EventContext;

namespace columnar
{
  /// @brief Container id definitions for the columnar infrastructure
  ///
  /// Container ids identify the different "virtual" containers used by
  /// the columnar infrastructure. Each container id is represented by a
  /// struct (typically named with a `Def` suffix, e.g. `JetDef`,
  /// `ElectronDef`, `ParticleDef`) that describes the container. These
  /// structs are defined directly in the `columnar` namespace and are
  /// used as template parameters throughout the columnar code. The user
  /// should not be trying to create instances of these structs, but
  /// rather use them as identifiers to pass into various columnar
  /// templates.
  ///
  /// To first order there is one container id for each xAOD type, and
  /// there is a direct mapping from container id to xAOD type. And for
  /// your code to compile in xAOD mode, your objects will be
  /// represented by a pointer to the underlying xAOD type. So to make
  /// your code compile in xAOD mode, you need to use the container id
  /// that corresponds to the xAOD type you want to use.
  ///
  /// In the columnar world we don't really have objects like that, and
  /// the container ids are mostly arbitrary (objects in columnar mode
  /// are usually represented by integers). However, there are still
  /// columns that belong together, share an offset map, and have a
  /// common prefix to their names. These columns form a "virtual
  /// container" and the container id is used to identify that virtual
  /// container.
  ///
  /// One important difference between xAOD types and container ids is
  /// that a container id is referring to a single container of a type.
  /// If you have e.g. two JetContainer instances in your tool, you need
  /// to use two different container ids for them. That is because in
  /// the columnar world the virtual containers come with completely
  /// separate columns, have separate offset maps, etc.
  ///
  /// A given container id also only has a meaning within the context of
  /// one specific tool instance. If you have multiple instances of a
  /// tool they may be connected to different containers/columns each.
  /// If a tool has subtools they need to coordinate their container ids
  /// as well, making subtools a lot more tightly connected than in the
  /// xAOD world.
  ///
  /// By default all referenced xAOD objects are const-qualified, as
  /// most tools will anyways work on const-qualified objects. In case
  /// your tool needs a non-const version of an object there is usually
  /// a "mutable" version of the container id available, which gives
  /// access to a mutable xAOD object and can be used wherever the
  /// corresponding const-qualified container id can be used, i.e. it
  /// doesn't introduce a separate container, but a different version of
  /// the same container.
  ///
  /// Most classes in the columnar infrastructure are templates that
  /// take the container id as a template parameter. In part this is to
  /// use the correct type in xAOD mode, but it also serves an important
  /// role in columnar mode, as it allows to perform a lot of safety
  /// checks at compile time. E.g. it allows to guarantee that an
  /// ObjectId always refers to a valid entry in the container and that
  /// it can only be used with columns that are associated with that
  /// container.
  ///
  /// For many of the containers there will be type aliases defined for
  /// some of the "common" infrastructure types, i.e. @ref ObjectId,
  /// @ref ObjectRange, @ref OptObjectId, @ref AccessorTemplate. This is
  /// meant to shorten the syntax for classes that users will regularly
  /// use in their own code. This is born out of experience during the
  /// prototyping stage when I found that the syntax that included all
  /// template parameters quickly became unwieldy.
  ///
  /// The main motivation for representing container ids as "traits"
  /// structs is that it makes it possible to define parametric container
  /// ids. It also alleviates the need to have a single list of all
  /// container ids in a single place — each package can define its own
  /// container id structs as needed.

  namespace detail
  {
    /// the CLID of a container type, or 0 when it cannot be determined
    ///
    /// CLASS_DEF is a no-op when parsed by rootcling (see
    /// xAODCore/CLASS_DEF.h), so the traits are unavailable there and
    /// this returns 0 during dictionary generation.
    template<typename T>
    CLID clidForType ()
    {
#ifdef __CLING__
      return 0;
#else
      return ClassID_traits<T>::ID();
#endif
    }
  }


  /// @brief a template that provides a base definition of container id
  /// for a regular container
  ///
  /// Essentially most container ids represent xAOD types underneath
  /// (in xAOD mode), and will as such share a lot of traits. Instead
  /// of repeating these over and over, I am instead defining them all
  /// here once, and then the individual definitions just need to
  /// derive from this and define a unique `idName`.
  template<typename ObjectType,typename ContainerType>
  struct RegularContainerId
  {
    /// identify this as a container id definition
    static constexpr bool isContainerId = true;

    /// whether to use the regular ObjectId/ObjectRange
    static constexpr bool regularObjectId = true;

    /// whether to use a variant ObjectId
    static constexpr bool variantObjectId = false;

    /// whether to use a regular column accessor in array mode
    static constexpr bool regularColumnAccessorArray = true;

    /// whether this is a non-const container
    static constexpr bool isMutable = false;

    /// whether this can be retrieved as a range per event
    static constexpr bool perEventRange = true;

    /// whether this can be retrieved as a single object per event
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const ObjectType;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const ContainerType;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = ContainerType;

    /// the CLID of the xAOD container type (from its CLASS_DEF)
    ///
    /// This is evaluated lazily, so container types without a
    /// CLASS_DEF only fail to compile if this actually gets called
    /// (e.g. when the container id is used as a link target).
    static CLID containerClid ()
    {
      return detail::clidForType<ContainerType> ();
    }
  };

  /// a template to define a mutable version of a given container id
  ///
  /// By default all xAOD objects will be held by `const` references,
  /// but some tools expect to have non-`const` access (usually to
  /// call xAOD-only code). This needs a separate mutable container
  /// id, and this template allows to define one with a simple `using`
  /// statement.
  template<typename CI>
    requires (CI::isContainerId && CI::regularObjectId)
  struct MutableContainerId : public CI
  {
    static constexpr bool isMutable = true;
    using constId = CI;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = std::remove_const_t<typename CI::xAODObjectIdType>;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = std::remove_const_t<typename CI::xAODObjectRangeType>;
  };

  // including this here, since everyone needs EventContextId/EventContextRange
  struct EventContextDef : RegularContainerId<EventContext,EventContext>
  {
    static constexpr std::string_view idName = eventContextCIName;

    // disable retrieve as either ObjectId or ObjectRange, the event
    // context will always be passed into tool code by the caller
    static constexpr bool perEventRange = false;
    static constexpr bool perEventId = false;
  };

  /// concept for a container id
  template<typename CI>
  concept ContainerIdConcept = CI::isContainerId;
  template<typename CI>
  concept RegularContainerIdConcept = CI::isContainerId && CI::regularObjectId;

  // forward declarations of columnar core classes for which I often
  // provide specific aliases for different container ids.
  template<ContainerIdConcept CI,typename CM = ColumnarModeDefault> class ObjectRange;
  template<ContainerIdConcept CI, typename CM = ColumnarModeDefault> class ObjectId;
  template<ContainerIdConcept CI, typename CM = ColumnarModeDefault> class OptObjectId;
  template<ContainerIdConcept CI, typename CM = ColumnarModeDefault> class ObjectLink;
  template<ContainerIdConcept CI,typename CT,ColumnAccessMode CAM,ColumnarMode CM> class AccessorTemplate;


  using EventContextRange = ObjectRange<EventContextDef>;
  using EventContextId = ObjectId<EventContextDef>;
}

#endif
