/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarHelperTools/ColumnarLinkTool.h>

#include <ColumnarCore/ContainerId.h>
#include <ColumnarInterfaces/KnownSgKeys.h>

#include <stdexcept>
#include <string>


//
// method implementations
//

namespace columnar
{
  namespace
  {
    /// @brief name of the source container used for the link columns
    constexpr const char* sourceContainerName = "Container";

    /// @brief base name of the link triple's primary column, used as
    /// the prefix for offset columns in the nested-vector case
    constexpr const char* linkBaseName = "Container.linkIndex";


    /// @brief register a single column via the low-level
    /// `ColumnAccessorDataArray` mechanism
    void registerColumn (ColumnarTool<CMode>& tool,
                         ColumnarLinkTool::ColumnSlot& slot,
                         const std::string& name,
                         const std::type_info* type,
                         ColumnAccessMode mode,
                         const std::string& offsetName,
                         bool isOffset)
    {
      slot.accessorData = std::make_unique<ColumnAccessorDataArray>
        (&slot.dataIndex, &slot.accessorData, type, mode);
      ColumnInfo info;
      info.offsetName = offsetName;
      info.isOffset = isOffset;
      tool.addColumn (name, slot.accessorData.get(), std::move (info));
    }


    /// @brief compute the name of the @c level -th link-offset column
    /// (level 0 = outermost), given a total nesting depth of
    /// @c depth.  Naming follows the convention from
    /// `ColumnarCore/VectorColumn.h`.
    std::string linkOffsetName (unsigned level, unsigned depth)
    {
      if (depth == 1u)
        return std::string (linkBaseName) + ".offset";
      if (depth == 2u)
        return std::string (linkBaseName) + (level == 0u ? ".outerOffset" : ".innerOffset");
      return std::string (linkBaseName) + ".offset" + std::to_string (level);
    }
  }



  ColumnarLinkTool ::
  ColumnarLinkTool (const std::string& name)
    : AsgTool (name)
  {}



  StatusCode ColumnarLinkTool ::
  initialize ()
  {
    if (m_targetContainerNames.empty())
    {
      ATH_MSG_ERROR ("targetContainerNames is empty");
      return StatusCode::FAILURE;
    }

    m_targetSlots.resize (m_targetContainerNames.size());
    for (std::size_t i = 0; i < m_targetContainerNames.size(); ++ i)
    {
      const auto& name = m_targetContainerNames.value()[i];
      auto keyIter = knownSgKeys.find (name);
      if (keyIter == knownSgKeys.end())
      {
        ATH_MSG_ERROR ("unknown target container name: " << name);
        return StatusCode::FAILURE;
      }

      auto& slot = m_targetSlots[i];
      slot.sgKey = keyIter->second;
      slot.accessorData = std::make_unique<ColumnAccessorDataArray>
        (&slot.dataIndex, &slot.accessorData,
         &typeid (ColumnarOffsetType), ColumnAccessMode::input);

      ColumnInfo info;
      info.offsetName = eventContextCIName;
      info.isOffset = true;
      addColumn (name, slot.accessorData.get(), std::move (info));
    }

    // source-container per-event particle offsets
    registerColumn (*this, m_srcContainerSlot, sourceContainerName,
                    &typeid (ColumnarOffsetType), ColumnAccessMode::input,
                    std::string (eventContextCIName), true);

    // nested-vector offset columns; outermost first
    const unsigned depth = m_nestingDepth.value();
    m_linkOffsetSlots.resize (depth);
    std::string parentOffsetName = sourceContainerName;
    for (unsigned i = 0; i < depth; ++ i)
    {
      const std::string name = linkOffsetName (i, depth);
      registerColumn (*this, m_linkOffsetSlots[i], name,
                      &typeid (ColumnarOffsetType), ColumnAccessMode::input,
                      parentOffsetName, true);
      parentOffsetName = name;
    }

    // the innermost offset name becomes the offset for the flat data
    // columns
    const std::string& innerOffsetName = parentOffsetName;
    const std::string dataSuffix = (depth == 0u ? "" : ".data");

    registerColumn (*this, m_indexSlot,
                    std::string ("Container.linkIndex") + dataSuffix,
                    &typeid (ColumnarModeArray::LinkIndexType),
                    ColumnAccessMode::input, innerOffsetName, false);
    registerColumn (*this, m_keySlot,
                    std::string ("Container.linkSGKey") + dataSuffix,
                    &typeid (SG::sgkey_t),
                    ColumnAccessMode::input, innerOffsetName, false);
    registerColumn (*this, m_outLinkSlot,
                    std::string ("Container.outLink") + dataSuffix,
                    &typeid (ColumnarModeArray::LinkIndexType),
                    ColumnAccessMode::output, innerOffsetName, false);

    // give the base class a chance to initialize the column accessor
    // backends
    ANA_CHECK (initializeColumns());
    return StatusCode::SUCCESS;
  }



  void ColumnarLinkTool ::
  callEvents (EventContextRange events) const
  {
    using CM = ColumnarModeArray;

    for (EventContextId event : events)
    {
      void** dataArea = event.getDataArea();
      const std::size_t eventIdx = event.getIndex();

      // compute the flat-link range for this event by walking the
      // source-particle offset and the nested-vector offset chain
      const auto *srcPartOff = static_cast<const ColumnarOffsetType*>
        (dataArea[m_srcContainerSlot.dataIndex]);
      std::size_t flatStart = srcPartOff[eventIdx];
      std::size_t flatEnd   = srcPartOff[eventIdx + 1];
      for (const auto& off : m_linkOffsetSlots)
      {
        const auto *a = static_cast<const ColumnarOffsetType*>
          (dataArea[off.dataIndex]);
        flatStart = a[flatStart];
        flatEnd   = a[flatEnd];
      }

      const auto *indexArr = static_cast<const CM::LinkIndexType*>
        (dataArea[m_indexSlot.dataIndex]);
      const auto *keyArr   = static_cast<const SG::sgkey_t*>
        (dataArea[m_keySlot.dataIndex]);
      auto       *outArr   = static_cast<CM::LinkIndexType*>
        (dataArea[m_outLinkSlot.dataIndex]);

      for (std::size_t j = flatStart; j < flatEnd; ++ j)
      {
        const CM::LinkIndexType index = indexArr[j];
        const SG::sgkey_t       key   = keyArr[j];

        if (index == 0 && key == 0)
        {
          outArr[j] = CM::invalidLinkValue;
          continue;
        }

        std::size_t t = 0;
        while (t < m_targetSlots.size() && m_targetSlots[t].sgKey != key)
          ++ t;

        if (t == m_targetSlots.size())
        {
          if (m_errorOnUnknownKey.value())
          {
            throw std::runtime_error
              ("ColumnarLinkTool: unknown sgkey " + std::to_string (key));
          }
          outArr[j] = CM::mergeLinkKeyIndex (0xfe, index);
          continue;
        }

        const auto *targetOffsets = static_cast<const ColumnarOffsetType*>
          (dataArea[m_targetSlots[t].dataIndex]);
        const CM::LinkIndexType myIndex = index + targetOffsets[eventIdx];
        if (myIndex >= targetOffsets[eventIdx + 1])
        {
          throw std::runtime_error
            ("ColumnarLinkTool: link index " + std::to_string (index)
             + " out of range for target "
             + m_targetContainerNames.value().at (t));
        }

        outArr[j] = CM::mergeLinkKeyIndex (t, myIndex);
      }
    }
  }
}
