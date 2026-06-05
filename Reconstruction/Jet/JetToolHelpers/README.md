# JetToolHelpers

`JetToolHelpers` provides a common framework for accessing the inputs used by jet tools, including jet variables, ROOT histograms, and event-level quantities.

The functionality has been used extensively in the new JetCalibTools package, and users are encouraged to refer to this package for examples of JetToolHelpers in production use.

---

# Overview

The central interface in JetToolHelpers is:

```cpp
JetHelper::IVarTool
```

Every helper tool implements a single function:

```cpp
float getValue(
    const xAOD::Jet& jet,
    const JetHelper::JetContext& context
) const;
```

A jet tool (e.g. a calibration step) therefore interacts with all helper tools in exactly the same way,

```cpp
float value = m_inputTool->getValue(jet, context);
```
and does not need to know whether the value came from a histogram, a jet attribute, or another source.

---

# Available Tools

| Tool             | Purpose                                             |
| ---------------- | --------------------------------------------------- |
| `VarTool`        | Read jet or event-level variables                   |
| `HistoInput1D`   | Read values from a 1D ROOT histogram                |
| `HistoInput2D`   | Read values from a 2D ROOT histogram                |
| `HistoInput3D`   | Read values from a 3D ROOT histogram                |

---

## VarTool

`VarTool` provides access to jet properties and event-level information.

Currently available variables can be found in `InputVariable::createVariable()`. If you need a variable that is not already implemented, please add it and submit an MR!

To set-up a VarTool:

```python
from HelperConfig import VarToolCfg

ptTool = VarToolCfg(
    flags,
    {
        "Name": "pt", # variable name
        "Type": "float", # variable type
        "Scale": 1e-3, # scale factor (useful for converting units)
        "isJetVar": True # whether the variable should be read from a jet or from the JetContext
    }
)
```

The configuration automatically recognises common jet variables (such as pt) and applies sensible defaults, for example

```python
ptTool = VarToolCfg(flags, "pt")
```

automatically configures

```python
Name = "pt"
isJetVar = True
Scale = 1e-3
```

---

## HistoInput1D, HistoInput2D, HistoInput3D

`HistoInput1D`/`HistoInput2D`/`HistoInput3D` read values from a 1D/2D/3D ROOT histogram. 

### Configuring HistoInput1D

```python
from JetToolHelpers.HelperConfig import HistoInputCfg

histTool = HistoInputCfg(
    flags,
    Tname="MyReader", # internal tool name
    inputFile="MyFile.root", # input ROOT file containing the histogram
    histName="Response", # name of the histogram within the file
    varX="pt" # variable used for the x-axis
    InterpType="Full", # interpolation type to apply
)
```
Note that `varX` sets up a `VarTool` instance. Therefore `varX` can be set to a dictionary of `VarTool` properties as described above, or a common variable such as `pt` for which sensible defaults are applied as described in the [VarTool](#vartool) section above.

See [below](#histogram-interpolation) for the various `InterpType` options.

### Configuring HistoInput2D

The set-up for a `HistoInput2D` tool is very similar, but includes a `varY` property to specify the y-axis histogram variable and set-up a corresponding `VarTool`:

```python
histTool = HistoInputCfg(
    flags,
    Tname="MyReader", # internal tool name
    inputFile="MyFile.root", # input ROOT file containing the histogram
    histName="Response", # name of the histogram within the file
    varX="pt", # variable used for the x-axis
    varY="eta" # variable used for the y-axis
    InterpType="Full", # interpolation type to apply
)
```

### Configuring HistoInput3D

`HistoInput3D` extends the same interface to 3D ROOT histograms, by including a `varZ` property:

The presence of `varZ` automatically causes `HistoInputCfg` to create a `HistoInput3D`.

```python
histTool = HistoInputCfg(
    flags,
    Tname="MyReader", # internal tool name
    inputFile="MyFile.root", # input ROOT file containing the histogram
    histName="Response3D", # name of the histogram within the file
    varX="pt", # variable used for the x-axis
    varY="eta", # variable used for the y-axis
    varZ="mass", # variable used for the z-axis
    InterpType="Full", # interpolation type to apply
)
```

#### Histogram Interpolation

Histogram readers support several interpolation modes:

| `InterpType` | Description                                                       |
| ------------ | ----------------------------------------------------------------- |
| `Full`       | Perform the full histogram interpolation.                         |
| `None`       | Disable interpolation and use the histogram bin content directly. |
| `OnlyX`      | Interpolate only along the x-axis.                                |
| `OnlyY`      | Interpolate only along the y-axis.                                |
| `OnlyZ`      | Interpolate only along the z-axis.                                |

---

# Developer Notes

This section is intended for developers wishing to understand the package architecture.

## Core Components

```text
        IVarTool
            |
    +-------+------+
    |              |              
 VarTool     HistoInputBase  
                   |              
         +---------+---------+    
         |         |         |    
     Histo1D   Histo2D   Histo3D  
                                  
                           
```

Every helper tool ultimately implements:

```cpp
getValue(jet, context)
```

which provides a unified interface for all input types.

---

## InputVariable

`InputVariable` is responsible for extracting information from either:

* an `xAOD::Jet`, or
* a `JetContext`.

`VarTool` acts as a configurable wrapper around an `InputVariable`.

---

## HistoInput Classes

The histogram readers inherit from a common histogram base class and differ only in the dimensionality of the underlying histogram.

Each reader:

1. Retrieves coordinate values from one or more `VarTool`s.
2. Evaluates the corresponding histogram.
3. Returns the resulting value through `getValue()`.