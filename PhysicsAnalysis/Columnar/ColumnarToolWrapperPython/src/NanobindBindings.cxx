/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack
/// @author Matthew Feickert
/// @author Giordon Stark

#include <ColumnarToolWrapperPython/PythonToolHandle.h>
#include <ColumnarCore/ColumnarDef.h>
#include <ColumnarCore/ColumnInfoHelpers.h>
#include <ColumnarInterfaces/ColumnInfoHelpers.h>
#include <CxxUtils/crc64.h>

#ifdef XAOD_STANDALONE
#include <AsgMessaging/IMessagePrinter.h>
#include <AsgMessaging/MessagePrinterOverlay.h>
#endif
#include <AsgMessaging/MsgLevel.h>

#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/operators.h>
#include <nanobind/stl/list.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/map.h>

#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>

namespace nb = nanobind;

using namespace nb::literals;

// this function lets us take an object and translate the pointer to a hexadecimal representation
// 140164452316520 becomes "0x7f7a9463f568"
std::string getAddressString(const columnar::PythonToolHandle &obj) {
    char buffer[32]; // Ensure the buffer is large enough for the hex representation
    std::snprintf(buffer, sizeof(buffer), "0x%llx", reinterpret_cast<unsigned long long>(&obj));
    return std::string(buffer);
}

// the dlpack dtype matching a column with the given type category and
// size (or nothing if there is no matching dtype)
std::optional<nb::dlpack::dtype> columnDtype(columnar::ColumnTypeCategory category, unsigned size) {
    nb::dlpack::dtype result {0, static_cast<std::uint8_t>(size * 8), 1};
    switch (category) {
        case columnar::ColumnTypeCategory::floatingPoint:
            result.code = static_cast<std::uint8_t>(nb::dlpack::dtype_code::Float);
            break;
        // a sign-agnostic column (e.g. `char`) is reported as signed,
        // but it will accept an unsigned array as well
        case columnar::ColumnTypeCategory::signedInteger:
        case columnar::ColumnTypeCategory::signAgnosticInteger:
            result.code = static_cast<std::uint8_t>(nb::dlpack::dtype_code::Int);
            break;
        case columnar::ColumnTypeCategory::unsignedInteger:
            result.code = static_cast<std::uint8_t>(nb::dlpack::dtype_code::UInt);
            break;
        // a numpy bool is a single byte holding 0 or 1, which is what a
        // boolean column holds (if it has that size)
        case columnar::ColumnTypeCategory::boolean:
            result.code = static_cast<std::uint8_t>(size == 1 ? nb::dlpack::dtype_code::Bool : nb::dlpack::dtype_code::UInt);
            break;
        default:
            return std::nullopt;
    }
    if (size == 0)
        return std::nullopt;
    return result;
}

// the numpy name of the dtype for a column with the given type
// category and size, e.g. "float32", "uint64" or "bool"
std::string columnTypeName(columnar::ColumnTypeCategory category, unsigned size) {
    auto dtype = columnDtype(category, size);
    if (!dtype)
        return std::string("unknown (") + columnar::columnTypeCategoryName(category) + ", " + std::to_string(size) + " bytes)";
    switch (static_cast<nb::dlpack::dtype_code>(dtype->code)) {
        case nb::dlpack::dtype_code::Float: return "float" + std::to_string(dtype->bits);
        case nb::dlpack::dtype_code::Int: return "int" + std::to_string(dtype->bits);
        case nb::dlpack::dtype_code::UInt: return "uint" + std::to_string(dtype->bits);
        case nb::dlpack::dtype_code::Bool: return "bool";
        default: return "unknown";
    }
}

void setProperty(columnar::PythonToolHandle &self, const std::string& key, nb::object value){
    if (nb::isinstance<nb::str>(value)) {
        self.setProperty(key, nb::cast<std::string>(value));
    } else if (nb::isinstance<nb::int_>(value)) {
        self.setProperty(key, nb::cast<int>(value));
    } else if (nb::isinstance<nb::float_>(value)) {
        self.setProperty(key, nb::cast<double>(value));
    } else {
        throw std::runtime_error("Unsupported property type. Must be str, int, or float.");
    }
}

nb::object getColumnVoid(columnar::PythonToolHandle &self, const std::string& key) {
  auto [size, ptr, header] = self.getColumnVoid(key);
  auto dtype = columnDtype(header->typeCategory, header->sizeOfType);
  if (!dtype)
    throw std::runtime_error("getColumnVoid: unsupported column type for " + key + ": " + columnTypeName(header->typeCategory, header->sizeOfType));
  // Wrap the raw pointer in a numpy array without copying.  The caller is
  // responsible for keeping the PythonToolHandle alive while using the result.
  const std::size_t shape[1] = {size};
  return nb::ndarray<nb::numpy, nb::ro>(ptr, 1, shape, nb::handle(), nullptr, *dtype).cast();
}

void setColumnVoid(columnar::PythonToolHandle &self, const std::string& key, nb::ndarray<> column, bool is_const = true) {
  // we don't have a C++ type for the array, so we only pass on the
  // category and size, and let the tool wrapper check whether those
  // are compatible with the column
  columnar::ColumnTypeCategory category = columnar::ColumnTypeCategory::other;
  const nb::dlpack::dtype dtype = column.dtype();
  switch ((nb::dlpack::dtype_code) dtype.code) {
     case nb::dlpack::dtype_code::Int:
         category = columnar::ColumnTypeCategory::signedInteger;
         break;
     case nb::dlpack::dtype_code::UInt:
         category = columnar::ColumnTypeCategory::unsignedInteger;
         break;
     case nb::dlpack::dtype_code::Bool:
         category = columnar::ColumnTypeCategory::boolean;
         break;
     case nb::dlpack::dtype_code::Float:
         category = columnar::ColumnTypeCategory::floatingPoint;
         break;
     default:
         throw std::runtime_error ("unsupported array type passed in for column " + key);
  }
  if (dtype.lanes != 1 || dtype.bits % 8 != 0)
    throw std::runtime_error ("unsupported array type passed in for column " + key);
  self.setColumnVoid(key, column.shape(0), column.data(), nullptr, category, dtype.bits / 8, is_const);
}

void setImmutableColumnVoid(columnar::PythonToolHandle &self, const std::string& key, nb::ndarray<> column){
  setColumnVoid(self, key, column, true);
};

namespace {

#ifdef XAOD_STANDALONE
/// Routes C++ IMessagePrinter::print() calls to a Python callable.
/// callback signature: (level_int: int, tool_name: str, text: str)
struct PyMessagePrinter : public asg::IMessagePrinter {
    nb::callable m_callback;

    explicit PyMessagePrinter(nb::callable cb)
        : m_callback(std::move(cb)) {}

    void print(MSG::Level lvl, const std::string& name,
               const std::string& text) override {
        // Guard against Python interpreter shutdown
        if (!Py_IsInitialized())
            return;
        nb::gil_scoped_acquire gil;
        m_callback(static_cast<int>(lvl), name, text);
    }
};

// Global state for printer management
static std::unique_ptr<PyMessagePrinter> g_printer;
static std::unique_ptr<asg::MessagePrinterOverlay> g_overlay;

void set_printer_from_callable(nb::callable cb) {
    g_printer = std::make_unique<PyMessagePrinter>(std::move(cb));
    g_overlay.reset();
    g_overlay = std::make_unique<asg::MessagePrinterOverlay>(g_printer.get());
}

void clear_printer() {
    g_overlay.reset();
    g_printer.reset();
}
#endif  // XAOD_STANDALONE

}  // anonymous namespace


NB_MODULE(python_tool_handle, module) {
    module.doc() = "Nanobind bindings for PythonToolHandle";

    static_assert(std::is_same_v<columnar::ColumnarMode,columnar::ColumnarModeArray>, "this module is only defined for array mode");

    module.attr("numberOfEventsName") = &columnar::eventRangeColumnName;
    module.attr("eventRangeColumnName") = &columnar::eventRangeColumnName;

    module.def("crc64",
        [](const std::string& data) { return CxxUtils::crc64(data); },
        "data"_a,
        "CRC-64 of a string, using the Athena default polynomial "
        "(CxxUtils::crc64).");

    module.def("crc64addint", &CxxUtils::crc64addint,
        "crc"_a, "x"_a,
        "Extend a previously-calculated CRC-64 to include an integer "
        "(CxxUtils::crc64addint).");

    module.def("sg_key",
        &columnar::computeSgKey,
        "name"_a, "clid"_a = 0,
        "StoreGate hash (sgkey) of a container name, optionally mixed with "
        "its CLID. ElementLink m_persKey values in Athena-written files are "
        "sg_key(container_name, container_clid).");

    // Install a Python callable as the global C++ message printer.
#ifdef XAOD_STANDALONE
    // Overload 1: with callback function
    module.def("set_python_printer", &set_printer_from_callable, nb::arg("callback"),
    "Install a Python callable(level: int, name: str, text: str) as the global "
    "C++ message printer.");

    // Overload 2: reset (no argument)
    module.def("set_python_printer", &clear_printer,
    "Reset to the default stdout message printer.");
#else
    // In Athena/AthAnalysis builds IMessagePrinter does not exist; expose the
    // function so Python code doesn't get AttributeError, but raise at call time.
    module.def("set_python_printer", [](nb::args, nb::kwargs) {
        throw std::runtime_error(
            "set_python_printer is only available in standalone "
            "(AnalysisBase) builds, not in Athena/AthAnalysis.");
    }, "Not available in Athena/AthAnalysis builds.");
#endif  // XAOD_STANDALONE

    /// load in the ColumnAccessMode enum
    nb::enum_<columnar::ColumnAccessMode>(module, "ColumnAccessMode")
      .value("input", columnar::ColumnAccessMode::input)
      .value("output", columnar::ColumnAccessMode::output)
      .value("update", columnar::ColumnAccessMode::update)
      .def("__repr__", [](const columnar::ColumnAccessMode &mode) -> std::string {
          switch (mode) {
              case columnar::ColumnAccessMode::input:
                  return "<ColumnAccessMode input>";
              case columnar::ColumnAccessMode::output:
                  return "<ColumnAccessMode output>";
              case columnar::ColumnAccessMode::update:
                  return "<ColumnAccessMode update>";
              default:
                  return "<ColumnAccessMode update value=" + std::to_string(static_cast<int>(mode)) + ">";
          }
      })
      .export_values(); // Makes the enum values accessible without namespace in Python

    nb::enum_<MSG::Level>(module, "MsgLevel", nb::is_arithmetic())
        .value("NIL",     MSG::NIL)
        .value("VERBOSE", MSG::VERBOSE)
        .value("DEBUG",   MSG::DEBUG)
        .value("INFO",    MSG::INFO)
        .value("WARNING", MSG::WARNING)
        .value("ERROR",   MSG::ERROR)
        .value("FATAL",   MSG::FATAL)
        .export_values();

    nb::enum_<columnar::ColumnTypeCategory>(module, "ColumnTypeCategory")
      .value("floatingPoint", columnar::ColumnTypeCategory::floatingPoint)
      .value("signedInteger", columnar::ColumnTypeCategory::signedInteger)
      .value("unsignedInteger", columnar::ColumnTypeCategory::unsignedInteger)
      .value("signAgnosticInteger", columnar::ColumnTypeCategory::signAgnosticInteger)
      .value("boolean", columnar::ColumnTypeCategory::boolean)
      .value("other", columnar::ColumnTypeCategory::other);

    nb::class_<columnar::ColumnInfo>(module, "ColumnInfo")
        .def(nb::init<>()) // Default constructor
        .def_ro("name", &columnar::ColumnInfo::name)
        .def_ro("index", &columnar::ColumnInfo::index)
        .def_prop_ro("dtype", [](const columnar::ColumnInfo &self){
            return columnTypeName(self.typeCategory, self.sizeOfType);
        })
        .def_ro("type_category", &columnar::ColumnInfo::typeCategory)
        .def_ro("size_of_type", &columnar::ColumnInfo::sizeOfType)
        .def_ro("access_mode", &columnar::ColumnInfo::accessMode)
        .def_ro("offset_name", &columnar::ColumnInfo::offsetName)
        .def_ro("fixed_dimensions", &columnar::ColumnInfo::fixedDimensions)
        .def_ro("is_offset", &columnar::ColumnInfo::isOffset)
        .def_ro("replaces_column", &columnar::ColumnInfo::replacesColumn)
        .def_ro("is_optional", &columnar::ColumnInfo::isOptional)
        .def_ro("is_event_index", &columnar::ColumnInfo::isEventIndex)
        .def_ro("name_for_links", &columnar::ColumnInfo::nameForLinks)
        .def("__repr__", [](const columnar::ColumnInfo &self) {
          std::string access_mode;
          switch (self.accessMode) {
              case columnar::ColumnAccessMode::input:
                  access_mode = "input";
                  break;
              case columnar::ColumnAccessMode::output:
                  access_mode = "output";
                  break;
              case columnar::ColumnAccessMode::update:
                  access_mode = "update";
                  break;
              default:
                  // For unknown values, return the integer value
                  access_mode = "unknown";
          }
            return "<ColumnInfo name='" + self.name + "'" +
                   (self.isOffset ? "" : ", offset='" + self.offsetName + "'") +
                   ", access_mode='" + access_mode + "'" +
                   ", dtype='" + columnTypeName(self.typeCategory, self.sizeOfType) + "'" +
                   (self.isOptional ? ", optional": "") +
                   ">";
        })
        .def("to_dict", [](const columnar::ColumnInfo& self) {
            nb::dict d;
            d["name"] = self.name;
            d["index"] = self.index;
            d["dtype"] = columnTypeName(self.typeCategory, self.sizeOfType);
            d["type_category"] = self.typeCategory;
            d["size_of_type"] = self.sizeOfType;
            d["access_mode"] = static_cast<int>(self.accessMode);
            d["offset_name"] = self.offsetName;
            d["fixed_dimensions"] = self.fixedDimensions;
            d["is_offset"] = self.isOffset;
            d["replaces_column"] = self.replacesColumn;
            d["is_optional"] = self.isOptional;
            d["is_event_index"] = self.isEventIndex;
            d["name_for_links"] = self.nameForLinks;
            return d;
        });

    nb::class_<columnar::PythonToolHandle>(module, "PythonToolHandle")
        .def(nb::init())

        // Properties
        .def_prop_ro("type", [](const columnar::PythonToolHandle &self) {
            const asg::AsgToolConfig& config = self.getConfig();
            const std::string& type = config.type();
            if (type.empty()) {
                std::cerr << "Warning: PythonToolHandle.type is empty."
                << " Set with PythonToolHandle.set_type_and_name." << std::endl;
            }
            return type;
        })

        .def_prop_ro("name", [](const columnar::PythonToolHandle &self) {
            const asg::AsgToolConfig& config = self.getConfig();
            const std::string& name = config.name();
            if (name.empty()) {
                std::cerr << "Warning: PythonToolHandle.name is empty."
                << " Set with PythonToolHandle.set_type_and_name." << std::endl;
            }
            return name;
        })

        // Methods
        .def("set_type_and_name",
             [](columnar::PythonToolHandle &self, const std::string& type_and_name) {
                 self.setTypeAndName(type_and_name);
             },
             "type_and_name"_a,
             "Set the type and name of the tool.")

        .def("set_property", &setProperty,
            "key"_a, "value"_a,
            "Set a property on the tool.")

        .def("__setattr__", &setProperty,
            "key"_a, "value"_a,
            "Set a property on the tool.")

        .def("preinitialize",
             &columnar::PythonToolHandle::preinitialize,
             "Preinitialize the tool.")

        // set_link_target_names({"container": "name", ...})
        .def("set_link_target_names",
            [](columnar::PythonToolHandle &self, const std::map<std::string,std::string>& names){
                std::vector<std::pair<std::string, std::string>> vectorized;
                for (const auto& pair : names)
                    vectorized.emplace_back(pair);

                return self.setLinkTargetNames(vectorized);
            },
            "names"_a,
            "Set the names of containers when they are the targets of links.")

        // rename_containers([("from", "to"), ...])
        .def("rename_containers",
             &columnar::PythonToolHandle::renameContainers,
             "renames"_a,
             "Rename the columns the tool uses.")

        // rename_containers({"from": "to"}, ...})
        .def("rename_containers",
            [](columnar::PythonToolHandle &self, const std::map<std::string,std::string>& renames){
                std::vector<std::pair<std::string, std::string>> vectorized;
                for (const auto& pair : renames)
                    vectorized.emplace_back(pair);

                return self.renameContainers(vectorized);
            },
            "renames"_a,
            "Rename the columns the tool uses.")

        .def("initialize",
             &columnar::PythonToolHandle::initialize,
             "Initialize the tool.")

        .def("apply_systematic_variation",
             [](columnar::PythonToolHandle &self, const std::string& sys_name) {
                 self.applySystematicVariation(sys_name);
             },
             "sys_name"_a,
             "Apply a systematic variation to the tool.")

        .def("set_column",
             [](columnar::PythonToolHandle &self, const std::string& key, nb::ndarray<> column) {
                 setColumnVoid(self, key, column, false);
             },
             "key"_a, "column"_a,
             "Set a (mutable) column pointer, the array has to be of a type compatible with the column.")

        .def("set_column_void", &setColumnVoid,
             // cppcheck-suppress assignBoolToPointer
             "key"_a, "column"_a, "is_const"_a = true,
             "Set a void column pointer (nanobind version).")

        .def("__setitem__", &setImmutableColumnVoid,
             "key"_a, "column"_a,
             "Set a void immutable column pointer (nanobind version).")

        .def("__getitem__", &getColumnVoid,
             "key"_a,
             "Get a column as a numpy array (zero-copy view into the tool's buffer).")

        .def("keys",
             &columnar::PythonToolHandle::getColumnNames,
             "Return the column names (enables dict(handle)).")

        .def("call",
             &columnar::PythonToolHandle::call,
             "Call the tool and reset the columns.")

        .def_prop_ro(
            "columns",
            &columnar::PythonToolHandle::getColumnInfo,
            "Get the expected column information."
        )

        .def("get_recommended_systematics",
             &columnar::PythonToolHandle::getRecommendedSystematics,
             "Get the recommended systematics.")

        // Make this more fancy in the future
        // <PythonToolHandle(CP::MuonEfficiencyScaleFactors/unique0) object at 0x7f2943b07568>
        .def("__repr__", [](const columnar::PythonToolHandle &self) {
            const asg::AsgToolConfig& config = self.getConfig();
            return "<PythonToolHandle(" + config.type() + "/" + config.name() + ") object at " + getAddressString(self) + ">";
        });
}
