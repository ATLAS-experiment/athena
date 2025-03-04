/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_INTERFACES_I_COLUMNAR_TOOL_H
#define COLUMNAR_INTERFACES_I_COLUMNAR_TOOL_H

#include <string>
#include <vector>

namespace columnar
{
  /// @brief the type used for the size and offsets in the columnar data
  ///
  /// @todo This type still needs to be adjusted to match whatever uproot
  /// uses for its offset maps.
  using ColumnarOffsetType = std::size_t;

  /// @brief the value for an invalid element index
  ///
  /// This is mostly used for invalid element links
  inline constexpr ColumnarOffsetType invalidObjectIndex = static_cast<ColumnarOffsetType>(-1);


  /// @brief the name used for the column containing the number of
  /// events
  ///
  /// Essentially this shows up in a large number of places, and I'm not
  /// sure what the best name for it is.  I'm currently using
  /// "EventInfo", because technically it is the number of `EventInfo`
  /// objects the data has.  I'm pretty sure that this will be confusing
  /// to many people.
  ///
  /// @todo Find a better name than "EventInfo".
  inline const std::string numberOfEventsName = "EventInfo";


  struct ColumnInfo;


  /// @brief an interface for tools that operate on columnar data
  ///
  /// This provides a counter-part to the columnar prototype (in mode
  /// 2), that gives a common interface for all columnar tools.  This
  /// interface requires a very specific layout of the data for the
  /// individual columns, though it gives the user some freedom on how
  /// to do the name-column lookup.  And it should provide a clean
  /// hand-off point between tool implementations and tool users, while
  /// also providing some capabilities for tool composition frameworks.
  ///
  /// This interface should make it fairly simple to provide generic
  /// wrappers to interface the tool with columnar data in various
  /// environments.  In particular it ought to be possible to provide an
  /// interface to work with Awkward Arrays in python, to provide
  /// function wrappers that work for RDF, or to extract the underlying
  /// decoration vectors from an xAOD objects/container and pass them
  /// into this tool.  If needed it should also be fairly
  /// straightforward to wrap it for other languages.
  ///
  /// It should also be fairly straightforward to implement this
  /// interface in a number of ways, as long as the underlying code can
  /// operate on columnar data.  For a tool implemented as a POD
  /// function, one would add a wrapper that implements the event and
  /// object loop.  For tools that prefer/need a higher level data
  /// interface the columnar prototype can provide such a tool
  /// interface.  It would also (with some strict limitations) be
  /// possible to wrap existing xAOD tools to provide this interface.
  /// It should also be possible to wrap tools implemented with other
  /// tool kits or in other languages, as long as they comply with the
  /// specific data layout chosen.
  ///
  /// This interface should allow to implement a number of reasonable
  /// and common optimizations.  The two most important ones are that it
  /// is zero copy, and that it moves the event and object loops into
  /// the tool.  Being zero copy is assumed as an obvious neessicty.
  /// And we have a number of benchmarks that show that for some classes
  /// of tools moving the loop into the tool itself can be a significant
  /// optimization.  And for tools that can't benefit, implementing the
  /// outer loop in the tool ought to be generally just a few lines of
  /// boiler-plate code.
  ///
  /// This interface should also be flexible enough to allow for future
  /// extensions and to build infrastructure on top of if.  In
  /// particular it ought to be possible to combine multiple smaller
  /// tools into an overall tool, e.g. a tool could calculate some
  /// kinematic properties using tool-specific C++ code and then pass
  /// those onto a tool that performs a highly optimized histogram
  /// lookup.
  ///
  /// It is expected that this interface will be extended in the future
  /// as new tools or frameworks may require new capabilities.  For tool
  /// implementations the expectation is that they generally won't need
  /// updating, but wrappers that won't to interact with the new
  /// features may need updating.  It should also often be possible to
  /// add/remove features by wrapping a tool in an adapter tool.
  ///
  /// At this point (24 May 24) even the base interface is still under
  /// development and not yet fully stable.  The basic design ideas
  /// should stay the same though, so a tool that is implemented based
  /// on this interface could likely be easily updated to the final
  /// version of this interface.
  ///
  ///
  /// Basic Design
  /// ============
  ///
  /// Let's take as a basic tool a tool that calculates `pt` from `px`
  /// and `py`.  This is a lot simpler than any real CP tool we
  /// currently have (24 May 24), and it seems unlikely that a full CP
  /// tool will ever be this simple.  So if this feels like too much
  /// infrastructure for this task, please keep in mind that actual
  /// tools will be substantially more complex and can benefit a lot
  /// more from the functionality this interface provides.
  ///
  /// At its simplest, a pt calculator would just be a simple function:
  /// ```
  ///   float calcPT (float px, float py) {
  ///     return sqrt (px*px + py*py);}
  /// ```
  ///
  /// As noted above, in general we want to move the loop into the
  /// tools, so that the tools that can benefit from a custom loop can.
  /// Due to its simplicity this tool would likely benefit as well,
  /// particularly if we added some `#pragma` statements that allow the
  /// compiler to vectorize the loop.  To move the loop into the tool,
  /// we need to pass in the number of objects to process, pass in the
  /// arguments as vectors, and provide an output vector/column as well:
  /// ```
  ///   void calcPT (SizeType numObjects, const float px[], const float py[], float pt[]) {
  ///     for (SizeType i = 0; i != numObjects; i++)
  ///       pt[i] = sqrt (px[i]*px[i] + py[i]*py[i]);}
  /// ```
  ///
  /// Next to make columnar operations more efficient we need to allow
  /// passing in many events at once.  Technically the interface above
  /// would already allow that, but a lot of tools will need to know
  /// which object belongs to which event, e.g. their correction may
  /// have a run number dependence.  To facilitate this we need to
  /// define an offset vector that defines where the objects for each
  /// event start.  If e.g. we have 3 events with 3, 7, and 5 objects
  /// respectively, the offset vector would be `{0, 3, 10, 15}`.  Note
  /// that there is one more entry than the number of events, and that
  /// the last entry is the total number of objects.  This then results
  /// in a double loop:
  /// ```
  ///   void calcPT (SizeType numEvents, const SizeType objectOffset[],
  ///                const float px[], const float py[], float pt[]) {
  ///     for (SizeType iEvent = 0; iEvent != numEvents; iEvent++)
  ///       for (SizeType iObject = objectOffset[iEvent]; iObject != objectOffset[iEvent+1]; iObject++)
  ///         pt[iObject] = sqrt (px[iObject]*px[iObject] + py[iObject]*py[iObject]);}
  /// ```
  ///
  /// For tools that do *not* care about the event structure, this can
  /// be simplified to a single loop again:
  /// ```
  ///   void calcPT (SizeType numEvents, const SizeType objectOffset[],
  ///                const float px[], const float py[], float pt[]) {
  ///     for (SizeType i = 0; i != objectOffset[numEvents]; i++)
  ///       pt[i] = sqrt (px[i]*px[i] + py[i]*py[i]);}
  /// ```
  ///
  /// For consistency the number of events can be turned into an offset
  /// vector as well (of length 2).  As an added benefit this also
  /// allows to have the tool run more easily on a subset of the events:
  /// ```
  ///   void calcPT (const SizeType eventOffset[], const SizeType objectOffset[],
  ///                const float px[], const float py[], float pt[]) {
  ///     for (SizeType i = objectOffset[eventOffset[0]]; i != objectOffset[eventOffset[1]]; i++)
  ///       pt[i] = sqrt (px[i]*px[i] + py[i]*py[i]);}
  /// ```
  ///
  /// To allow this to be passed through a virtual interface this is
  /// then turned into an array of `void*` with the meaning of each
  /// pointer dependent on the position in the array:
  /// ```
  ///   void calcPT (void** data) {
  ///     auto *eventOffset = static_cast<const SizeType*>(data[0]);
  ///     auto *objectOffset = static_cast<const SizeType*>(data[1]);
  ///     auto *px = static_cast<const float*>(data[2]);
  ///     auto *py = static_cast<const float*>(data[3]);
  ///     auto *pt = static_cast<float*>(data[4]);
  ///     for (SizeType i = objectOffset[eventOffset[0]]; i != objectOffset[eventOffset[1]]; i++)
  ///       pt[i] = sqrt (px[i]*px[i] + py[i]*py[i]);}
  /// ```
  ///
  /// The general expectation is that both the caller of the tool and
  /// the tool implementation would utilize some helper code to pack and
  /// unpack the `data` vector, so users wouldn't have to deal with the
  /// type-erased pointers directly.  To facilitate assembling the data
  /// vector, the tool has to report all the data columns it needs
  /// together with some meta-information (e.g. the name, type, access
  /// mode, and associated offset vector).  The wrapper on the caller
  /// side can then use that not only to assemble the data vector, but
  /// also to validate e.g. that all the columns have the right size
  /// and type.
  ///
  /// As an additional benefit of passing the data as a `void*` array,
  /// it is actually possible to make the number and names of inputs and
  /// outputs dynamic.  E.g. the tool can change the exact columns it
  /// needs based on the configuration, which some of the existing tools
  /// actually do.  And for analysis performance it is actually critical
  /// that we only read variables we actually use.
  ///
  /// As an extra benefit of reporting the data columns the tool needs,
  /// it can also report any extra meta-information the wrapper may
  /// need, or attach extra requirements it has for the columns.
  /// Besides the immediate use this also provides some design space for
  /// future extensions.

  class IColumnarTool
  {
  public:
    virtual ~IColumnarTool() = default;


    /// @brief run the tool on the data vector
    ///
    /// This is the main function of the tool.  See the above
    /// description to see how this function is expected to be called
    /// and implemented.
    ///
    /// I'm still (24 May 24) trying to decide whether to pass the data
    /// pointer as `void*` or `const void*`.  Either way, either the
    /// caller or the implementation has to do a `const_cast` on the
    /// input columns.
    virtual void callVoid (void** data) const = 0;


    /// @brief the meta-information for the columns
    ///
    /// This is all the information that a wrapper for the tool needs to
    /// be able to call the tool.  For more information see the
    /// documentation of @ref ColumnInfo.
    [[nodiscard]] virtual std::vector<ColumnInfo> getColumnInfo () const = 0;


    /// @brief rename one of the columns the tool uses
    ///
    /// This is meant to make it easier to allow tools that use a
    /// generic name like "Egamma" to be renamed to "Electron" or
    /// "Photon" based on what they should look at.  In isolation this
    /// is only marginally useful, but if I have multiple tools in a
    /// sequence or try to interface them directly to names from the
    /// input files it can be quite useful.
    ///
    /// If a column is given the name of an existing column, the two
    /// columns are merged.  If the merger fails (e.g. because there is
    /// a type mismatch), an exception is thrown and the tool is left
    /// in an undefined state.
    virtual void renameColumn (const std::string& from, const std::string& to) = 0;


    /// @brief set the index for the column
    ///
    /// This allows the user or framework to set the index of the given
    /// column in the data vector.  Setting the column index from the
    /// user side allows to synchronize the column indices across tools,
    /// meaning they can share the same data vectors.  It also avoids
    /// complex logic in the tool to figure out the index of each column
    /// as the columns get declared.
    ///
    /// Note that even if we didn't allow the user to set the index of
    /// each column, the tool would still need to have the ability to do
    /// so internally, as it is needed when adding subtools.  As such,
    /// having the framework set the index for each column after
    /// initialization is actually more efficient and leads to simpler code.
    ///
    /// By convention all columns are initialized to have index 0, and
    /// the data vector is expected to be initialized with a nullptr at
    /// index 0.  That way if an optional column is not assigned an
    /// index, it will correctly recognize that it was not given an
    /// input.  And for non-optional columns the tool will segfault,
    /// which is preferable to silently using the wrong data.  Note that
    /// not assigning an index to a non-optional column is an error that
    /// the framework should check for.
    virtual void setColumnIndex (const std::string& name, std::size_t index) = 0;
  };
}

#endif