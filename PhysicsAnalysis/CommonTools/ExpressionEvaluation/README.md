# ExpressionEvaluation

The ExpressionEvaluation package enables event and object selections to be defined with human-readable strings. It can be applied either to xAOD content or n-tuples. It is primarily used in the derivation framework for event skimming and object thinning, enabling a wide range of complex selections to be made without recourse to dedicated tools. ExpressionEvaluation was orginally written by Thomas Gillam, then a Ph.D. student at the University of Cambridge. These notes are primarily taken from his slides [here](https://indico.cern.ch/event/273466/contributions/616597/attachments/492353/680556/summary.pdf) and [here](https://indico.cern.ch/event/333980/contributions/1726332/attachments/651824/896284/summary.pdf)

For information on using ExpressionEvaluation within the derivation framework, please see the [derivation framework manual](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DerivationFramework)

## Examples of strings

* Simple event selection: `count(abs(Electrons.eta) < 1.0) > 0 && count(Muons.pt > (20 * GeV)) >= 1`
* Event selection including strings: `HLT_mu24 && count(abs(ElectronCs.eta) < 1.0) > 0 && count(Muons.pt > (20 * GeV)) >= 1`
* Object selection: `InDetTrackParticles.pt > 10.0`

As can be seen, the syntax `ContainerName.MethodName` should be used, where

* ContainerName is the name of the object container in StoreGate, e.g. `Muons`, `Electrons`
* MethodName is the name of the object's C++ method which returns this quantity, e.g. eta, pt as seen above (omit the () in the string parsing).

## Capabilities and limitations

ExpressionEvaluation can comprehend any quantity that:

* is a unique single value for a given object (e.g. electron pT is OK because there is only one value of pT per electron)
* can be accessed from the xAOD object directly by a simple method returning the single value - e.g. `pt()`, `eta()` etc
* is a decoration added to the objects
* is a quantity in the xAOD aux store (even those accessed through helper functions)
* MET map-based accessors
* columns in a flat ROOT n-tuple

However, the tool cannot handle:

* access to specific elements of vectors
* vectors-of-vector
* arbitrary function calls and multiple layers of accessors, e.g. `myObj->someFunction().someOtherFunction().someValue()`
* any data types that can’t meaningfully be cast to scalar types  such as `int`, `float`, `double` etc or vectors thereof
* ElementLinks between xAOD objects

Dedicated C++ should be written if such things are needed in a selection (and if this dedicated C++ writes its decisions to SG as decorations they can be accessed via ExpressionEvaluation for the final selection).

## Expressions 

Expressions are built from four basic components: operators, functions, constants and identifiers.

* **Operators**: All C++ numerical and logical operators are supported, following the usual precedence rules.
* **Functions**: A handful of unary mathematical functions such as sqrt, abs, sin etc.
* **Constants**: Numerical constants (integral or floating point), and strings e.g. e and pi. Is also an extensible unit system – more on this below.
* **Identifiers**: Strings that evaluate to numbers loaded from an external source (e.g. StoreGate). Can be integral or floating point. Can be scalar (event-level) or vector (object-level).

### Operators

This table summarises the set of supported operators, grouped by precedence, highest first:

| Operator | Action |
|----------|-------------|
| `(...)`  | Evaluate inside parentheses first |
| `!,-` | Logical not, unary negation |
| `**` | Power (`a**b` is a<sup>b</sup>) |
| `*,/ | Multiply and divide |
| `+,-` | Addition and subtraction |
| `>,<,>=,<=` | Relational |
| `==,!=` | (In)equality |
| `&&,||` | Logical *and*, *or* | 

### Functions

The supported functions are as follows:

| Function | Meaning |
|----------|-------------|
| `abs` | Absolute value, (`abs(a)` is `|a|`) |
| `sqrt, cbrt` | Square and cube root |
| `sin, cos, tan` | Ordinary trig |
| `asin, acos, atan` | Inverse trig |
| `sinh, cosh, tanh` | Hyperbolic trig |
| `asinh, acosh, atanh` | Inverse hyperbolic trig |
| `log` | Natural logarithm |
| `exp` | Exponential (`exp(a)` is e<sup>a</sup>)|
| `sum` | Sum of all quantities in vector |
| `count` | No. of non-zero vector elements |
 
### Scalar types

Constants are either integers or floating point (doubles, internally). The outcome of logical operations are represented by 1 and 0.
Sensible integer ←→ floating point conversions occur automatically: 

* `int * double` ⇒ `double`
* `sqrt(int)` ⇒ `double`
* `double > double` ⇒ `int`
* `!double` ⇒ `int`
* `int / int` ⇒ `int`

### Vector types

When dealing with identifiers that map to object-level quanties, these are modelled as vectors of numbers, one for each element.
In addition to the integer/floating point conversions, vector/scalar interactions are handled like so:

* vector + scalar ⇒ vector 
* abs(vector) ⇒ vector 
* vector * vector ⇒ vector 
* sum(vector) ⇒ scalar

### Constants and units

Four basic constants are supported: `e`, `pi`, `true` and `false`.

Additionally, unit conversions can be available. This is modular and extensible via implementing the interface IUnitInterpreter, since different sources of data might need different factors. Currently support exists for `keV`, `MeV`, `GeV`, `TeV`, `mm`, `cm`.
The strings are written directly into the expression – if the string is not a recongnised constant, the string is treated as an identifier. 

### Identifiers

Identifiers are strings for which values (and types!) are determined at run-time. There is an interface (IProxyLoader) to be implemented for any required source. The existing implementations are the StoreGate-based SGNTUPProxyLoader and xAODProxyLoader. Object-level identifiers are converted (if necessary) to vector types by the ProxyLoader. All integer-like and float-like types are supported

### Combining event- and object-level quantities

We use the special functions `sum` and `count` to turn vector operands into scalar results.

* `sum(· · · )`: Add up all elements of the vector operand.
* `count(· · · )`: Count all non-zero elements of the vector operand.

`count` is intended for use on boolean sub-expressions, e.g. “give me events with at least 3 muons with pT > 10 GeV”, whereas sum is more if you want to compute e.g. a scalar sum of pTs.

## Overview of workflow and components

ExpressionEvaluation operates in two steps, in the `initialise` and `execute` steps of the Athena job.

In the **initialise** step:

* the user-provided expression is converted into an *abstract syntax tree* via the `Boost::spirit` [library](http://boost-spirit.com/home/)
* the tree is convertex into bytecode via a "*compiler*"

In the **execute step**:

* a "*virtual machine*" loads the relevant identifiers from the event and compares against the bytecode, which acts as a mask
* the virtual machine returns the results

The consequence of this is that the text itself is only parsed once, in the initialise method. The execute method, which runs per event, benefits from the pre-compiled bytecode. Note that the terms "compiler" and "virtual machine" are Spirit shorthand terms and are not to be taken literally. 

### Parsing

Parsing is performed using [Boost’s Spirit framework](http://boost-spirit.com/home/). 

* The grammar is defined in C++. Operator precedences are enforced here.
* A parsing algorithm is created at compile-time, using template meta-programming techniques.
* Compile times are hence long, but efficient at run-time.
* Parsing is done on initialisation of the tool. The output of the parsing step is an ‘abstract syntax tree’.

### Compilation

The syntax tree is traversed, producing a bytecode. The bytecode objects are all instances of the StackElement class:

* Holds op-codes, scalars, vectors, and identifiers.
* Contains bulk of the logic for integral↔floating-point and scalar↔vector conversions.
* Identifiers are not resolved into values at compilation

### Execution

For each event, `StackElements` corresponding to identifers are evaluated to numbers using the given instance of `IProxyLoader`. The expression is evaluated using a simple stack-based VM, iterating over the byte-code. This maximising the amount of work done in initialisation.

## xAOD support

xAOD EDM support is approached in two ways:

1. Try to use `SG::ConstAccessor` first. This is the generic way of retrieving data from `SG::AuxElement` or
`SG::AuxVectorData`, from which all xAOD classes and DataVectors thereof derive.
1. If a variable (e.g. `xAOD::MissingET::met()`) is actually a function without a directly corresponding value stored in the aux store, function calls are still required. Only functions with no arguments returning an `int`, `float` or `double` type are supported.
These are only used if the aux store doesn’t contain an element of the correct name.

## Description of main files

The main user-facing class is `ExpressionParserUser.h` which should be used to set up the expression parser in the initialise method of any C++ tool that needs to use it. An example of this in action can be found [here](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkTools/src/GenericObjectThinning.cxx):

`ATH_CHECK(initializeParser( m_selectionString) );`

where `m_selectionString` is user-defined string described above. The object made available by this can then be used to execute the relevant selections, e.g.

* `std::vector<int> entries =  m_parser->evaluateAsVector();` for per-object decisions (e.g. thinning or decoration of objects)
* `std::vector<int> entries =  m_parser->evaluateAsBool();` for per-event decisions (e.g. skimming)

`ExpressionParser` is the class that provides the above methods, but user code should not directly interact with this, instead using the helper class `ExpressionParserUser.h`. This is to allow the framework to "renounce" ReadHandles for variables used in the expression parser, that is, remove them from the AthenaMT scheduler if there are WriteHandles for them in previous tools. This avoids circular dependencies.

`MultipleProxyLoader` is the class that choreographs the use of the appropriate proxy loader to access the data, based on the input string.  The other files related to the inner workings of the expression evaluation, including the individual proxy loaders and the machinery for processing the strings and building the bytecode.
