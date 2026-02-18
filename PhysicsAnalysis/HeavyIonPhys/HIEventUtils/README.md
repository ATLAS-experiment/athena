# Event Selection Tools for HI Analyses

## Run 3

Run 3 event selection is implemented through a single tool and a corresponding algorithm.

Using the **tool directly** allows the analyzer to access multiple validation methods that determine whether an event is suitable for analysis (e.g., pileup rejection and related quality checks).

Using the **algorithm** applies a predefined set of selection cuts to the processed data. The outcome of each cut is encoded in a bit mask that is stored as the `HIEventSelection` decoration on `EventInfo`. The bit encoding scheme is defined by enums in the tool interface.

---

### Histogram-Based Cut Configuration

Some selection cuts depend on centrality or FCal energy and are therefore parameterized as histograms.

These histograms are version-controlled within the repository and stored as JSON files in the `data` subdirectory. To generate a compatible histogram file:

1. Open the histogram in ROOT.
2. Execute:

```c++
   // get the hist you need
   h->SaveAs("myhist.json")
```
This command produces a JSON representation of the histogram.

To use the histogram with the selection tool, extract the following fields from the generated file and copy them into a flat dictionary in the JSON configuration file:
```c++
"fName"
"fTitle"
"fNbins"
"fXmin"
"fXmax"
"fArray"
```
Example configuration files are available in the data subdirectory of this package.