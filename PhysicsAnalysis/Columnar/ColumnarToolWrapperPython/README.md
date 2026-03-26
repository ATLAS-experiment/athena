# ColumnarToolWrapperPython

Python bindings for ATLAS columnar CP tools.

## Setup

```bash
setupATLAS -q
asetup <your release>
python -m venv --system-site-packages venv
./venv/bin/pip install awkward uproot
```

## API Overview

Three levels of interface are provided, from highest to lowest level:

| Level | Class / module | Best for |
|-------|---------------|----------|
| Analyzer | `atlascp` | Simplest and full of sugar |
| High-level | `Tool` | General non-expert developer access |
| Low-level | `PythonToolHandle` | Expert access, manual buffer control, custom integrations |

---

## Analyzer API (`atlascp`)

The `atlascp` submodule lets you construct tools by name without writing the
`CP::ToolType/instanceName` string manually. It returns a `Tool` subclass
instance named after the tool type.

```python
from ColumnarToolWrapperPython import atlascp

# Equivalent to Tool("CP::MuonEfficiencyScaleFactors/myTool")
tool = atlascp.MuonEfficiencyScaleFactors("myTool")

# With properties and container renaming
tool = atlascp.MuonEfficiencyScaleFactors(
    "myTool",
    properties={"WorkingPoint": "Tight"},
    rename_containers={
        "EventInfo": "EventInfoAuxDyn",
        "Muons": "AnalysisMuonsAuxDyn",
    },
)
result = tool(events)
```

The `namespace` keyword (default `"CP"`) selects the C++ namespace:

```python
# Equivalent to Tool("columnar::VectorExampleTool/vecEx")
tool = atlascp.VectorExampleTool("vecEx", namespace="columnar")
```

---

## High-level API (`Tool`)

`Tool` takes an awkward array of events and handles buffer extraction,
column setting, calling, and output reconstruction automatically.

```python
import awkward as ak
import uproot
from ColumnarToolWrapperPython import Tool

tool = Tool(
    "CP::MuonEfficiencyScaleFactors/myTool",
    rename_containers={
        "EventInfo": "EventInfoAuxDyn",
        "Muons": "AnalysisMuonsAuxDyn",
    },
)

# Load events from a PHYSLITE file
with uproot.open("DAOD_PHYSLITE.root") as f:
    tree = f["CollectionTree"]
    # Optional columns (e.g. isLRT) may not be present in all files
    input_fields = [c.name for c in tool.input_columns if not c.is_optional]
    events = tree.arrays(input_fields, entry_stop=100)

# Returns an ak.Array with one field per output column
result = tool(events)
print(result["AnalysisMuonsAuxDyn.sfOut"].to_list())
```

### Inspecting columns and systematics

```python
# All columns (ColumnInfo objects)
for col in tool.columns:
    print(col.name, col.access_mode, col.is_optional)

# Input / output columns separately
print([c.name for c in tool.input_columns])
print([c.name for c in tool.output_columns])

# Properties set at construction time
print(tool.properties)

# Recommended systematic variations
print(tool.recommended_systematics)
```

### Systematics

```python
for sys_name in tool.recommended_systematics:
    tool.apply_systematic_variation(sys_name)
    result = tool(events)
    print(sys_name, result["AnalysisMuonsAuxDyn.sfOut"].to_list())
```

---

## Low-level API (`PythonToolHandle`)

For direct control over column buffers — useful when your data is already in
numpy arrays rather than awkward arrays, or when writing custom integrations.

```python
import numpy as np
from ColumnarToolWrapperPython import PythonToolHandle

handle = PythonToolHandle()
handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/myTool")
handle.initialize()

# Set properties before initialize()
handle = PythonToolHandle()
handle.set_type_and_name("CP::MuonCalibTool/myTool")
handle.set_property("IsRun3Geo", False)   # via set_property()
handle.calibMode = 0                       # or via attribute assignment
handle.initialize()
```

### Setting and calling columns

Input columns use `__setitem__`; output columns (written by the tool) require
`set_column_void` with `is_const=False` so the tool can write into the buffer.
**Hold references to all numpy arrays until after `call()`** — the handle
stores raw pointers, so arrays must not be garbage-collected mid-call.

```python
handle = PythonToolHandle()
handle.set_type_and_name("columnar::SimpleSelectorExampleTool/myTool")
handle.initialize()

event_info   = np.array([0, 2],          dtype=np.uint64)   # offsets: [start, end]
particles    = np.array([0, 1, 3],       dtype=np.uint64)   # offsets: [0, 1, 2, 3]
particles_pt = np.array([10e5, 10e5, 1e3], dtype=np.float32)
selection    = np.array([0, 0, 0],       dtype=np.int8)     # output buffer

handle["EventInfo"]          = event_info
handle["Particles"]          = particles
handle["Particles.pt"]       = particles_pt
handle.set_column_void("Particles.selection", selection, False)  # mutable output

handle.call()
print(list(selection))  # [1, 1, 0]
```

### Reading columns back

You can read any column (input or output) via `__getitem__`:

```python
sf_arr = handle["Muons.sfOut"]    # returns a read-only numpy view
```

`dict(handle)` returns all columns as a `{name: array}` mapping. Unset
columns return empty arrays; `call()` resets all columns back to empty:

```python
# After setting columns, before call():
cols = dict(handle)
print(cols["Muons.pt"])   # populated
print(cols["Muons.isLRT"])  # empty — optional, not set

handle.call()
cols = dict(handle)       # all empty again after call()
```

### Inspecting column metadata

```python
for col in handle.columns:
    print(col.name, col.access_mode, col.is_optional, col.is_offset)

print(handle.get_recommended_systematics())
```

---

## The same task at each level

The following three snippets all do the same thing: run
`MuonEfficiencyScaleFactors` on a PHYSLITE file and print the output scale
factors. They differ only in how much boilerplate you write.

### Level 1 — `atlascp` (least boilerplate)

```python
import uproot
from ColumnarToolWrapperPython import atlascp

tool = atlascp.MuonEfficiencyScaleFactors(
    "myTool",
    properties={"WorkingPoint": "Tight"},
    rename_containers={"EventInfo": "EventInfoAuxDyn", "Muons": "AnalysisMuonsAuxDyn"},
)

with uproot.open("DAOD_PHYSLITE.root") as f:
    events = f["CollectionTree"].arrays(
        [c.name for c in tool.input_columns if not c.is_optional], virtual=True
    )

result = tool(events)
print(result["AnalysisMuonsAuxDyn.sfOut"].to_list())
```

### Level 2 — `Tool` (explicit type string)

```python
import uproot
from ColumnarToolWrapperPython import Tool

tool = Tool(
    "CP::MuonEfficiencyScaleFactors/myTool",
    properties={"WorkingPoint": "Tight"},
    rename_containers={"EventInfo": "EventInfoAuxDyn", "Muons": "AnalysisMuonsAuxDyn"},
)

with uproot.open("DAOD_PHYSLITE.root") as f:
    events = f["CollectionTree"].arrays(
        [c.name for c in tool.input_columns if not c.is_optional], virtual=True
    )

result = tool(events)
print(result["AnalysisMuonsAuxDyn.sfOut"].to_list())
```

### Level 3 — `PythonToolHandle` (full manual control)

```python
import numpy as np
import uproot
from ColumnarToolWrapperPython import PythonToolHandle

handle = PythonToolHandle()
handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/myTool")
handle.set_property("WorkingPoint", "Tight")
handle.initialize()
handle.rename_containers({"EventInfo": "EventInfoAuxDyn", "Muons": "AnalysisMuonsAuxDyn"})

# Load flat numpy arrays from uproot
input_names = [c.name for c in handle.columns if not c.is_offset and not c.is_optional]
with uproot.open("DAOD_PHYSLITE.root") as f:
    arrays = f["CollectionTree"].arrays(input_names, library="np")

# Allocate output buffers
n_muons = len(arrays["AnalysisMuonsAuxDyn.pt"])
sf_out    = np.zeros(n_muons, dtype=np.float32)
valid_out = np.zeros(n_muons, dtype=np.int8)

# Set offsets, inputs, and outputs — hold all references until after call()
event_info_offsets = np.array([0, len(arrays["AnalysisMuonsAuxDyn.pt"])], dtype=np.uint64)
muon_offsets       = np.arange(n_muons + 1, dtype=np.uint64)
handle["EventInfoAuxDyn"]             = event_info_offsets
handle["EventInfoAuxDyn.runNumber"]   = arrays["EventInfoAuxDyn.runNumber"]
handle["EventInfoAuxDyn.RandomRunNumber"] = arrays["EventInfoAuxDyn.RandomRunNumber"]
handle["EventInfoAuxDyn.eventTypeBitmask"] = arrays["EventInfoAuxDyn.eventTypeBitmask"]
handle["AnalysisMuonsAuxDyn"]         = muon_offsets
handle["AnalysisMuonsAuxDyn.pt"]      = arrays["AnalysisMuonsAuxDyn.pt"]
handle["AnalysisMuonsAuxDyn.eta"]     = arrays["AnalysisMuonsAuxDyn.eta"]
handle["AnalysisMuonsAuxDyn.phi"]     = arrays["AnalysisMuonsAuxDyn.phi"]
handle["AnalysisMuonsAuxDyn.muonType"] = arrays["AnalysisMuonsAuxDyn.muonType"]
handle.set_column_void("AnalysisMuonsAuxDyn.sfOut",    sf_out,    False)
handle.set_column_void("AnalysisMuonsAuxDyn.validOut", valid_out, False)

handle.call()
print(sf_out.tolist())
```

---

## Nested Vectors

Support for tools that read `std::vector<std::vector<T>>` columns (e.g.
`VectorExampleTool` reading `NumTrkPt500`) is planned. The `ColumnInfo`
metadata already exposes nested-vector offsets, but buffer extraction for
these columns is not yet implemented in the high-level `Tool` API.

At the `PythonToolHandle` level, nested-vector columns can be set manually
using the `.offset` / `.data` naming convention:

```python
handle["Met.name.offset"] = np.array([0, 9, 14], dtype=np.uint64)
handle["Met.name.data"]   = np.array([ord(c) for c in "InvisibleFinal"], dtype=np.int8)
```
