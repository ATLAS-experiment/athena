/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack
/// @author Matthew Feickert
/// @author Giordon Stark

#include <ColumnarToolWrapperPython/PythonToolHandle.h>
#include <ColumnarCore/ColumnarDef.h>
#include <ColumnarCore/ColumnInfoHelpers.h>
#include <ColumnarModeDefault/ColumnarModeDefault.h>
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

#include <cstdio>
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

// this function converts us from type_info to a human-readable name
std::string get_type_name(const std::type_info& type_info) {
    if (type_info == typeid(int)) {
        return "int32";
    } else if (type_info == typeid(unsigned int)) {
        return "uint32";
    } else if (type_info == typeid(short)) {
        return "int16";
    } else if (type_info == typeid(unsigned short)) {
        return "uint16";
    } else if (type_info == typeid(char)) {
        /// escape hatch for char
        return "int8";
    } else if (type_info == typeid(unsigned char)) {
        return "uint8";
    } else if (type_info == typeid(float)) {
        return "float32";
    } else if (type_info == typeid(double)) {
        return "float64";
    } else if (type_info == typeid(long)) {
        return "int64";
    } else if (type_info == typeid(unsigned long)) {
        return "uint64";
    } else if (type_info == typeid(bool)) {
        return "bool";
    } else {
        // If the type is unknown, you can return the mangled name or a default message
        return std::string("unknown ('") + type_info.name() + "')";
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
  auto [size, ptr, type] = self.getColumnVoid(key);
  // Wrap the raw pointer in a numpy array without copying.  The caller is
  // responsible for keeping the PythonToolHandle alive while using the result.
#define MAKE_NDARRAY(T) \
  nb::ndarray<nb::numpy, T, nb::ro>(static_cast<const T*>(ptr), {size}).cast()
  if (*type == typeid(float))          return MAKE_NDARRAY(float);
  if (*type == typeid(double))         return MAKE_NDARRAY(double);
  if (*type == typeid(char))           return MAKE_NDARRAY(char);
  if (*type == typeid(int))            return MAKE_NDARRAY(int);
  if (*type == typeid(std::uint8_t))   return MAKE_NDARRAY(std::uint8_t);
  if (*type == typeid(std::uint16_t))  return MAKE_NDARRAY(std::uint16_t);
  if (*type == typeid(std::uint32_t))  return MAKE_NDARRAY(std::uint32_t);
  if (*type == typeid(std::uint64_t))  return MAKE_NDARRAY(std::uint64_t);
  if (*type == typeid(std::int16_t))   return MAKE_NDARRAY(std::int16_t);
  if (*type == typeid(std::int32_t))   return MAKE_NDARRAY(std::int32_t);
  if (*type == typeid(std::int64_t))   return MAKE_NDARRAY(std::int64_t);
#undef MAKE_NDARRAY
  throw std::runtime_error("getColumnVoid: unsupported column type: " + std::string(type->name()));
}

void setColumnVoid(columnar::PythonToolHandle &self, const std::string& key, nb::ndarray<> column, bool is_const = true) {
  // TODO: figure out how to get type_info from handle instead...
  // nb::handle handle = column.handle();

  const std::type_info* type_info = nullptr;
  const nb::dlpack::dtype dtype = column.dtype();
  switch ((nb::dlpack::dtype_code) dtype.code) {
     case nb::dlpack::dtype_code::Int:
         switch (dtype.bits) {
             // escape hatch to handle char for now
             // we should rely on signed/unsigned and nbits, instead of std::type_info
             // case 8: type_info = &typeid(std::int8_t); break;
             case 8: type_info = &typeid(char); break;
             case 16: type_info = &typeid(std::int16_t); break;
             case 32: type_info = &typeid(std::int32_t); break;
             case 64: type_info = &typeid(std::int64_t); break;
         }
         break;

     case nb::dlpack::dtype_code::UInt:
         switch (dtype.bits) {
             case 8: type_info = &typeid(std::uint8_t); break;
             case 16: type_info = &typeid(std::uint16_t); break;
             case 32: type_info = &typeid(std::uint32_t); break;
             case 64: type_info = &typeid(std::uint64_t); break;
         }
         break;

     case nb::dlpack::dtype_code::Float:
         switch (dtype.bits) {
             case 32: type_info = &typeid(float); break;
             case 64: type_info = &typeid(double); break;
         }
         break;

     default:
         break;
  }

  if (type_info == nullptr) throw std::runtime_error ("unsupported column type passed in");
  self.setColumnVoid(key, column.shape(0), column.data(), *type_info, is_const);
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

    if (!std::is_same_v<CMode,columnar::ColumnarModeArray>)
        throw nb::import_error("This module can only be used in columnar access mode. Try setting up a ColumnarAnalysis release instead.");

    module.attr("numberOfEventsName") = &columnar::eventRangeColumnName;
    module.attr("eventRangeColumnName") = &columnar::eventRangeColumnName;

    // the value used for an invalid (null/empty) element link in array mode
    module.attr("invalid_link_value") = columnar::ColumnarModeArray::invalidLinkValue;

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
            "(AnalysisBase/ColumnarAnalysis) builds, not in Athena/AthAnalysis.");
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

    nb::class_<columnar::ColumnInfo>(module, "ColumnInfo")
        .def(nb::init<>()) // Default constructor
        .def_ro("name", &columnar::ColumnInfo::name)
        .def_ro("index", &columnar::ColumnInfo::index)
        .def_prop_ro("dtype", [](const columnar::ColumnInfo &self){
            return get_type_name(*self.type);
        })
        .def_ro("access_mode", &columnar::ColumnInfo::accessMode)
        .def_ro("offset_name", &columnar::ColumnInfo::offsetName)
        .def_ro("fixed_dimensions", &columnar::ColumnInfo::fixedDimensions)
        .def_ro("sole_link_target_name", &columnar::ColumnInfo::soleLinkTargetName)
        .def_ro("sole_link_target_clid", &columnar::ColumnInfo::soleLinkTargetClid)
        .def_ro("is_variant_link", &columnar::ColumnInfo::isVariantLink)
        .def_ro("variant_link_target_names", &columnar::ColumnInfo::variantLinkTargetNames)
        .def_ro("key_column_for_variant_link", &columnar::ColumnInfo::keyColumnForVariantLink)
        .def_ro("is_offset", &columnar::ColumnInfo::isOffset)
        .def_ro("replaces_column", &columnar::ColumnInfo::replacesColumn)
        .def_ro("is_optional", &columnar::ColumnInfo::isOptional)
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
                   ", dtype='" + (self.type ? get_type_name(*self.type) : "" ) + "'" +
                   (self.isOptional ? ", optional": "") +
                   ">";
        })
        .def("to_dict", [](const columnar::ColumnInfo& self) {
            nb::dict d;
            d["name"] = self.name;
            d["index"] = self.index;
            d["dtype"] = self.type ? get_type_name(*self.type) : "";
            d["access_mode"] = static_cast<int>(self.accessMode);
            d["offset_name"] = self.offsetName;
            d["fixed_dimensions"] = self.fixedDimensions;
            d["sole_link_target_name"] = self.soleLinkTargetName;
            d["sole_link_target_clid"] = self.soleLinkTargetClid;
            d["is_variant_link"] = self.isVariantLink;
            d["variant_link_target_names"] = self.variantLinkTargetNames;
            d["key_column_for_variant_link"] = self.keyColumnForVariantLink;
            d["is_offset"] = self.isOffset;
            d["replaces_column"] = self.replacesColumn;
            d["is_optional"] = self.isOptional;
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
             [](columnar::PythonToolHandle &self, const std::string& key, nb::ndarray<float> column) {
                 self.setColumn<float>(key, column.shape(0), column.data());
             },
             "key"_a, "column"_a,
             "Set a float column pointer.")

        .def("set_column",
             [](columnar::PythonToolHandle &self, const std::string& key, nb::ndarray<char> column) {
                 self.setColumn<char>(key, column.shape(0), column.data());
             },
             "key"_a, "column"_a,
             "Set a char column pointer.")

        .def("set_column",
             [](columnar::PythonToolHandle &self, const std::string& key, nb::ndarray<int> column) {
                 self.setColumn<int>(key, column.shape(0), column.data());
             },
             "key"_a, "column"_a,
             "Set an int column pointer.")

        .def("set_column",
             [](columnar::PythonToolHandle &self, const std::string& key, nb::ndarray<std::uint8_t> column) {
                 self.setColumn<uint8_t>(key, column.shape(0), column.data());
             },
             "key"_a, "column"_a,
             "Set a uint8_t column pointer.")

        .def("set_column",
             [](columnar::PythonToolHandle &self, const std::string& key, nb::ndarray<std::uint16_t> column) {
                 self.setColumn<uint16_t>(key, column.shape(0), column.data());
             },
             "key"_a, "column"_a,
             "Set a uint16_t column pointer.")

        .def("set_column",
             [](columnar::PythonToolHandle &self, const std::string& key, nb::ndarray<std::uint32_t> column) {
                 self.setColumn<uint32_t>(key, column.shape(0), column.data());
             },
             "key"_a, "column"_a,
             "Set a uint32_t column pointer.")

        .def("set_column",
             [](columnar::PythonToolHandle &self, const std::string& key, nb::ndarray<std::uint64_t> column) {
                 self.setColumn<uint64_t>(key, column.shape(0), column.data());
             },
             "key"_a, "column"_a,
             "Set a uint64_t column pointer.")

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
