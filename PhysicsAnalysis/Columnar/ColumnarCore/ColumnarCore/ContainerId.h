/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_CONTAINER_ID_H
#define COLUMNAR_CORE_CONTAINER_ID_H

#include <ColumnarCore/ColumnarDef.h>

class EventContext;

namespace columnar
{
  /// @brief the id for the different "virtual" containers
  ///
  /// To first order these are just the different xAOD types, and have a
  /// direct mapping to the xAOD types.  In the columnar world we don't
  /// really have objects like that, but there are still columns that
  /// belong together, which (mostly) share an offset map and a common
  /// prefix to their names.  In effect these are virtual containers.
  ///
  /// One big difference to the xAOD world is that the container id is
  /// really referring to just a single container.  If you have e.g. two
  /// JetContainer instances in your tool, you need to use two different
  /// container ids for them.  That is because in the columnar world the
  /// virtual containers come with completely separate columns, have
  /// separate offset maps, etc.
  ///
  /// A given container id also only has a meaning within the context of
  /// one specific tool instance.  If you have multiple instances of a
  /// tool they may be connected to different containers/columns each.
  /// If a tool has subtools they need to coordinate their container ids
  /// as well, making subtools a lot more tightly connected than in the
  /// xAOD world.
  ///
  /// For xAOD code the container id must match the container used, as
  /// there is an underlying xAOD object for the code.  For columnar
  /// code they are essentially arbitrary, but to make it work for xAOD
  /// mode.  There are some containers that have special meaning (e.g.
  /// `event`), but most of them have the same behavior, just for
  /// different containers.
  ///
  /// By default all referenced xAOD objects are const-qualified, as
  /// most tools will anyways work on const-qualified objects.  And
  /// since the prototype works mostly with accessors directly, it
  /// bypasses most of the situations in which the xAOD objects are
  /// required to be mutable.  I now (13 Jan 25) started to add a
  /// mutable version of some containers, because if you need to
  /// interface with preexisting xAOD code it can matter.  For now I'd
  /// say use it sparingly.
  ///
  /// Most classes in the prototype are templates that take the
  /// container id as a template parameter.  In part this is to support
  /// the xAOD mode, but it also serves an important role in columanr
  /// mode, as it allows to perform a lot of safety checks at compile
  /// time.  E.g. it allows to guarantee that an ObjectId always refers
  /// to a valid entry in the container and that it can only be used
  /// with columns that are associated with that container.
  ///
  /// @note Whenever you add a new container id here, you also need to
  /// add an associated ContainerIdTraits specialization for that
  /// container id (in the correct header file).  You should also add
  /// proper type aliases for ObjectId, ObjectRange, OptObjectId, and
  /// ColumnAccessor.  These type aliases are essential for keeping user
  /// code reasonably compact.

  enum class ContainerId
  {
    jet,
    mutableJet,
    muon,
    electron,
    photon,
    egamma,
    cluster,
    track,
    track0 = track,
    track1,
    track2,
    vertex,
    particle,
    particle0 = particle,
    particle1,
    met,
    met0 = met,
    met1,
    mutableMet,
    metAssociation,
    eventInfo,
    eventContext
  };

  template<ContainerId> struct ContainerIdTraits final
  {
    static constexpr bool isDefined = false;
  };

  // including this here, since everyone needs EventContextId/EventContextRange
  template<> struct ContainerIdTraits<ContainerId::eventContext> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = false;
    static constexpr bool perEventRange = false;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const EventContext;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const EventContext;
  };
}

#endif
