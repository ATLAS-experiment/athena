/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_PYTHON_WRAPPER_PYTHON_TOOL_WRAPPER_H
#define COLUMNAR_PYTHON_WRAPPER_PYTHON_TOOL_WRAPPER_H

#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace columnar
{
  struct ColumnInfo;
  class IColumnarTool;


  /// @brief a class that wraps an IColumnarTool for use in Python
  ///
  /// This is not necessarily specific to python, but that is the
  /// primary use case.  Essentially all it does is provide a way to
  /// preload columns by name, check that the columns are are consistent
  /// with what the tool expects, and can then call the tool on those
  /// columns.
  ///
  /// This class only does the meta-data handling, whereas the actual
  /// handling of columnar data is done by @ref ColumnarToolWrapperData.  Please
  /// check its documentation for details.

  class ColumnarToolWrapper final
  {
    /// Public Members
    /// ==============

  public:

    /// @brief constructor: wrap the given tool (non-owning)
    explicit ColumnarToolWrapper (IColumnarTool *val_tool);

    /// @brief constructor: wrap the given tool (owning)
    explicit ColumnarToolWrapper (std::shared_ptr<IColumnarTool> val_tool);


    // copying would need special handling, so disabling it until needed
    ColumnarToolWrapper (const ColumnarToolWrapper&) = delete;
    ColumnarToolWrapper& operator= (const ColumnarToolWrapper&) = delete;


    /// @brief get information on all defined columns
    ///
    /// This is mostly to make it easy for the caller to know which
    /// columns are defined and connect them to the dataframe
    /// automatically.  You can ask either for the list of column names
    /// or the full `ColumnInfo`, depending on what you need.
    /// @{
      [[nodiscard]] std::vector<std::string> getColumnNames () const;
      [[nodiscard]] std::vector<ColumnInfo> getColumnInfo () const;
      /// @}
  


    /// Private Members
    /// ===============

  private:

    // These two classes work closely together, but for new users it
    // seemed clearer to make it a friend instead of a nested class.
    friend class ColumnarToolWrapperData;


    /// @brief the wrapped tool
    const IColumnarTool *m_tool = nullptr;

    /// @brief the owning pointer to the tool
    std::shared_ptr<const IColumnarTool> m_toolOwn;


    /// @brief my cached information for the various columns needed
    struct MyColumnInfo
    {
      const std::type_info *type = nullptr;

      bool isConst = false;

      unsigned index = 0u;

      unsigned fixedDimensions = 1u;

      bool isOffset = false;

      bool isOptional = false;

      int numpyType = -1;

      unsigned numpyBits = 0u;

      const std::pair<const std::string,MyColumnInfo> *offsets = nullptr;
    };
    std::unordered_map<std::string,MyColumnInfo> m_columns;


    /// @brief the number of columns that the tool expects (equal to the
    /// greatest column index + 1)
    unsigned m_numColumns = 0u;
  };



  /// @brief a class that holds the columnar data for a single call to
  /// @ref ColumnarToolWrapper
  ///
  /// The idea is that this is a fairly lightweight class that can be
  /// instantiated once per call and contain all thread-local
  /// information (i.e. the pointers to the user data).  For every use
  /// of this tool the caller should create a new instance of this
  /// class.
  ///
  /// The basic usage is something like this:
  /// ```
  ///   ColumnarToolWrapper wrapper {...};
  ///   ColumnarToolWrapperData data {&wrapper};
  ///   std::span<const float> input1 = ...;
  ///   data.setColumn ("input1", input1.size(), input1.data());
  ///   std::span<float> output1 = ...;
  ///   data.setColumn ("output1", output1.size(), output1.data());
  ///   data.call ();
  /// ```

  class ColumnarToolWrapperData
  {
    /// Public Members
    /// ==============

  public:

    /// @brief constructor: wrap the given tool
    explicit ColumnarToolWrapperData (const ColumnarToolWrapper *val_wrapper) noexcept;


    /// @brief set the data for the given column picking up the type via
    /// a template
    template<typename CT>
    void setColumn (const std::string& name, std::size_t size, CT* dataPtr) {
      auto voidPtr = reinterpret_cast<const void*>(const_cast<const CT*>(dataPtr));
      setColumnVoid (name, size, voidPtr, typeid (std::decay_t<CT>), std::is_const_v<CT>);
    }


    /// @brief set the data for the given column with the user passing
    /// in the type
    void setColumnVoid (const std::string& name, std::size_t size, const void *dataPtr, const std::type_info& type, bool isConst);


    /// @brief set the data for the given column with the user passing
    /// in the type information from numpy
    void setColumnNumpy (const std::string& name, std::size_t size, const void *dataPtr, int type, unsigned bits, bool isConst);


    /// @brief call the tool
    void call ();



    /// Testing/Validation Members
    /// ==========================
    ///
    /// These members shouldn't be needed for regular users, but it can
    /// help in debugging issues with the tool or the wrapper.

  public:


    /// @brief check that all columns are valid
    void checkColumnsValid ();


    /// @brief get the data for the given column picking up the type via
    /// a template
    template<typename CT>
    [[nodiscard]] std::pair<std::size_t,CT*>
    getColumn (const std::string& name)
    {
      auto [size, ptr] = getColumnVoid (name, &typeid (std::decay_t<CT>), std::is_const_v<CT>);
      if constexpr (std::is_const_v<CT>)
        return std::make_pair (size, static_cast<CT*>(ptr));
      else
        return std::make_pair (size, static_cast<CT*>(const_cast<void*>(ptr)));
    }

    /// @brief get the data for the given column in a type-erased manner
    [[nodiscard]] std::pair<std::size_t,const void*>
    getColumnVoid (const std::string& name, const std::type_info *type, bool isConst);

    /// @brief the data vector we have assembled
    [[nodiscard]] void** data () noexcept {
      return m_data.data();}



    /// Private Members
    /// ===============

  private:

    const ColumnarToolWrapper *m_wrapper = nullptr;

    std::vector<void*> m_data;
    std::vector<std::size_t> m_dataSize;
    std::vector<bool> m_columnIsChecked;
    std::vector<bool> m_columnIsFilled;

    void checkColumn (const std::pair<const std::string,ColumnarToolWrapper::MyColumnInfo>& column);
  };
}

#endif
