/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack
/// @author Matthew Feickert
/// @author Matthias Vigl
/// @author Giordon Stark

#include <AsgMessaging/MessageCheck.h>
#include <AsgMessaging/AsgMessaging.h>
#include <AsgTools/AsgTool.h>
#include <AsgTools/AsgToolConfig.h>
#include <AsgTools/ToolHandle.h>
#include <ColumnarInterfaces/IColumnarTool.h>
#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarToolWrapper/ColumnarToolWrapper.h>
#include <PATInterfaces/ISystematicsTool.h>
#include <PATInterfaces/SystematicSet.h>
#include <PATInterfaces/SystematicsUtil.h>
#include <span>

namespace columnar
{
  /// @brief a handle to a python tool for use via nanobind
  ///
  /// This is meant to be used together with a nanobind python binding,
  /// to provide a higher level and stable interface to the tool
  /// wrappers.  For other uses people should use the underlying
  /// interfaces directly or write a new higher level interface
  /// (depending on what's appropriate).
  ///
  /// For now this is not thread-safe.  The underlying code is
  /// thread-safe, as long as there is a separate `ColumnarToolWrapperData`
  /// object for each thread.  Also, the way the tools handle
  /// setting systematics is currently not thread-safe.

  class PythonToolHandle final : public asg::AsgMessaging
  {
    /// Public Members
    /// ==============

  public:

    /// standard constructor
    PythonToolHandle() : asg::AsgMessaging("PythonToolHandle") {}

    /// set the type and name for the tool
    void setTypeAndName (const std::string& typeAndName)
    {
      m_config.setTypeAndName (typeAndName);
    }

    /// set a property on the tool
    template<typename T>
    void setProperty (const std::string& key, T&& value)
    {
      if (m_config.setProperty (key, std::forward<T> (value)).isFailure())
        throw std::runtime_error ("failed to set property: " + key);
    }

    /// get the AsgToolConfig
    [[nodiscard]] const asg::AsgToolConfig& getConfig () const
    {
      return m_config;
    }

    /// preinitialize the tool
    void preinitialize ()
    {
      ANA_MSG_DEBUG("preinitializing with " << m_toolHandle << " and cleanup " << m_cleanup);
      if (!m_config.makeTool (m_toolHandle, m_cleanup).isSuccess())
        throw std::runtime_error ("failed to create tool");

      ANA_MSG_DEBUG("m_config created tool" << m_toolHandle);

      m_tool = dynamic_cast<IColumnarTool*> (&*m_toolHandle);

      ANA_MSG_DEBUG("attempting to dynamically cast to IColumnarTool* gives " << m_tool);
      if (m_tool == nullptr)
        throw std::runtime_error ("The tool does not implement IColumnarTool. First, check to make sure you're in the ColumnarAnalysis release. Then, check to see if the tool inherits from ColumnarTool.");
      m_systTool = dynamic_cast<CP::ISystematicsTool*> (m_tool);
    }

    /// rename the columns the tool uses
    void renameContainers (const std::vector<std::pair<std::string,std::string>>& renames)
    {
      if (m_tool == nullptr)
        preinitialize ();
      {
        auto columnInfo = m_tool->getColumnInfo ();
        for (auto& [from, to] : renames)
        {
          for (auto& column : columnInfo)
          {
            if (column.name.starts_with (from) && (column.name.size() == from.size() || column.name[from.size()] == '.'))
            {
              std::string newName = to + column.name.substr (from.size());
              m_tool->renameColumn (column.name, newName);
            }
          }
        }
      }
      if (m_toolWrapper)
      {
        m_toolWrapper = std::make_shared<ColumnarToolWrapper> (m_tool);
        m_columns = std::make_unique<ColumnarToolWrapperData> (m_toolWrapper.get());
      }
    }

    /// initialize the tool
    void initialize ()
    {
      if (m_tool == nullptr)
        preinitialize ();

      m_toolWrapper = std::make_shared<ColumnarToolWrapper> (m_tool);
      m_columns = std::make_unique<ColumnarToolWrapperData> (m_toolWrapper.get());
    }

    /// set the tool to apply the given systematic variation
    void applySystematicVariation (const std::string& sysName)
    {
      // by convention setting a systematic on a non-systematics tool
      // will do nothing
      if (m_systTool == nullptr)
        return;
      if (!m_systTool->applySystematicVariation (CP::SystematicSet (sysName)).isSuccess())
        throw std::runtime_error ("failed to apply systematic variation");
    }

    /// set a column pointer (raw pointer version)
    template<typename CT>
    void setColumn (const std::string& key, std::size_t size, CT* dataPtr)
    {
      if (!m_columns)
        throw std::runtime_error ("tool not initialized");
      m_columns->setColumn (key, size, dataPtr);
    }

    /// set a column pointer
    void setColumnVoid (const std::string& name, std::size_t size, const void *dataPtr, const std::type_info& type, bool isConst) {
      if (!m_columns)
        throw std::runtime_error ("tool not initialized");
      m_columns->setColumnVoid (name, size, dataPtr, type, isConst);
    }

    /// set a column pointer
    void setColumnNumpy (const std::string& name, std::size_t size, const void *dataPtr, int type, unsigned bits, bool isConst) {
      if (!m_columns)
        throw std::runtime_error ("tool not initialized");
      m_columns->setColumnNumpy (name, size, dataPtr, type, bits, isConst);
    }

    /// call the tool and reset the columns
    void call ()
    {
      if (!m_columns)
        throw std::runtime_error ("no columns set");
      m_columns->call ();
      m_columns = std::make_unique<ColumnarToolWrapperData> (m_toolWrapper.get());
    }

    /// get the expected column info
    [[nodiscard]] std::vector<ColumnInfo> getColumnInfo () const
    {
      if (!m_toolWrapper)
        throw std::runtime_error ("tool not initialized");
      return m_toolWrapper->getColumnInfo ();
    }

    /// get the expected column names
    std::vector<std::string> getColumnNames () const
    {
      if (!m_toolWrapper)
        throw std::runtime_error ("tool not initialized");
      return m_toolWrapper->getColumnNames ();
    }

    /// get the recommended systematics
    std::vector<std::string> getRecommendedSystematics () const
    {
      if (!m_systTool)
        return {""};
      std::vector<std::string> result;
      for (auto& sys : CP::make_systematics_vector (m_systTool->recommendedSystematics()))
        result.push_back (sys.name());
      return result;
    }



    /// Private Members
    /// ===============

  private:

    asg::AsgToolConfig m_config;
    ToolHandle<asg::AsgTool> m_toolHandle;
    std::shared_ptr<void> m_cleanup;

    IColumnarTool* m_tool = nullptr;
    CP::ISystematicsTool* m_systTool = nullptr;

    std::shared_ptr<const ColumnarToolWrapper> m_toolWrapper;
    std::unique_ptr<ColumnarToolWrapperData> m_columns;
  };
}
