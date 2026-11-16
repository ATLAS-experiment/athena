/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TRIGGER_TRIG_COMPOSITE_DEF_H
#define COLUMNAR_TRIGGER_TRIG_COMPOSITE_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <ColumnarInterfaces/ColumnarDef.h>
#include <xAODTrigger/TrigCompositeContainer.h>

namespace columnar
{
  /// @brief the container id for `xAOD::TrigComposite` containers
  ///
  /// This is primarily used for the HLT navigation summary (e.g.
  /// `HLTNav_Summary_DAODSlimmed`), which is a `TrigComposite`
  /// container.
  struct TrigCompositeDef : RegularContainerId<xAOD::TrigComposite,xAOD::TrigCompositeContainer>
  {
    static constexpr std::string_view idName = "trigComposite";
  };

  using TrigCompositeRange = ObjectRange<TrigCompositeDef>;
  using TrigCompositeId = ObjectId<TrigCompositeDef>;
  using OptTrigCompositeId = OptObjectId<TrigCompositeDef>;
  template<typename CT,ColumnarModeConcept CM=ColumnarMode> using TrigCompositeAccessor  = AccessorTemplate<TrigCompositeDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarModeConcept CM=ColumnarMode> using TrigCompositeDecorator = AccessorTemplate<TrigCompositeDef,CT,ColumnAccessMode::output,CM>;
}

#endif
