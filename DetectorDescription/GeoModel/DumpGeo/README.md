# DumpGeo - Dump the ATLAS GeoModel to a local file

 * [Intro](#intro) 
 * [Setup](#build)
 * [Run](#run)
 * [Unit tests](#unit-tests)
 * [Documentation](#documentation)

## Intro

`DumpGeo` is an Athena algorithm inheriting from the class AthAlgorithm. Internally, it calls the package `GeoExporter` to dump the GeoModel tree resulting from any Geometry TAGs into a local SQLite file.

You can run these intructions on any `lxplus`-like machine (Alma9 or CC7) where the Athena framework is installed, or on macOS with Athena running inside a [a Lima container/light VM](https://atlassoftwaredocs.web.cern.ch/athena/lima/)


## Setup 

You should setup Athena, as usual; for example you can setup the latest build of the `24.0` release:

```bash
setupATLAS
asetup Athena,24.0,latest
```

## Run

`DumpGeo` has been migrated to the new Athena Component Accumulator (CA). A new `DumpGeoConfig.py` Python script configures the Athena algorithm.  

`DumpGeo` can be used both as a shell command or as an Athena jobOption, in command-line or embedded within your own jobOption.

### Basic use - Run as a terminal command

After having set Athena, at the prompt, run the command:


```sh
python -m DumpGeo.DumpGeoConfig
```

This uses the geometry tag from input metadata when available. If no tag is
available from the command line or metadata, DumpGeo uses the default Run-3
tag. The output filename reflects the geometry tag that was dumped; for
example, `geometry-ATLAS-R3S-2021-03-02-00.db`.

Optionally, you can specify which geometry tag to dump by using the
`--detDescr` option; for example:

```sh
python -m DumpGeo.DumpGeoConfig --detDescr=ATLAS-R2-2016-01-00-01
```

After issuing the command, a file named `geometry-ATLAS-R2-2016-01-00-01.db` will be created in the run folder.

The geometry tag is selected with the following precedence:

1. an explicit `--detDescr=TAG` argument;
2. an explicit generic `GeoModel.AtlasVersion=TAG` flag;
3. the geometry tag stored in the input-file metadata; and
4. the default Run-3 tag when none of the above provides a tag.

These commands can be used to check each explicit configuration path and its
precedence:

```sh
# Use the dedicated DumpGeo convenience alias.
python -m DumpGeo.DumpGeoConfig \
    --detDescr=ATLAS-R3S-2021-03-03-00

# Use the equivalent generic Athena flag syntax.
python -m DumpGeo.DumpGeoConfig \
    GeoModel.AtlasVersion=ATLAS-R3S-2021-03-03-00

# If both forms are supplied, the dedicated --detDescr alias takes precedence.
# This command therefore uses ATLAS-R3S-2021-03-03-00.
python -m DumpGeo.DumpGeoConfig \
    --detDescr=ATLAS-R3S-2021-03-03-00 \
    GeoModel.AtlasVersion=ATLAS-R2-2016-01-00-01
```


### Run it as an Athena jobOption

You can also run `dump-geo` as an Athena jobOption. For example:

```bash
athena DumpGeo/dump-geo.py -c "DetDescrVersion='ATLAS-R3-2021-01-00-00‘"
```

You can even embed it into your own workflow, within your own jobOption.


## Options

### Overwrite the Output File

By default, DumpGeo exits with an error when an output file with the same name is found.

You can force to overwrite the output file with the `--forceOverwrite` or `-f` CLI options:

```sh
python -m DumpGeo.DumpGeoConfig -f
```

Overwrite handling is non-destructive during Python configuration. When force
overwrite is enabled, the C++ algorithm makes one removal attempt during its
`initialize()` method, immediately before opening the output database. A
missing file is harmless; any other filesystem error causes initialization to
fail instead of continuing with a stale database.

### Filter DetectorManagers

The CLI option `--filterDetManagers` lets you filter over DetectorManagers.

DetectorManagers are “containers” for subsystems. The class [GeoVDetectorManager](https://gitlab.cern.ch/GeoModelDev/GeoModel/-/blob/main/GeoModelCore/GeoModelKernel/GeoModelKernel/GeoVDetectorManager.h?ref_type=heads) holds all the TreeTops (the top volumes) of a subsystem together.

By using the `--filterDetManagers` CLI option, you can dump the TreeTops belonging to a single DetectorManager:

```sh
python -m DumpGeo.DumpGeoConfig --filterDetManagers="InDetServMat"
```

Or you can dump all TreeTops from multiple managers, passing a comma-separated list:

```sh
python -m DumpGeo.DumpGeoConfig --filterDetManagers="BeamPipe,InDetServMat"
```

The output file will reflect the content of the GeoModel tree:

```sh
geometry-ATLAS-R3S-2021-03-02-00-BeamPipe-InDetServMat.db
```

When dumping the content of a DetectorManager, DumpGeo picks the TreeTop volumes and gets their default Transform (no alignments). 

It then it adds to the output “world” volume a GeoNameTag with the name of the DetectorManager, and then it adds the TreeTop volumes to it with their Transforms ahead of them.

```
World
|
|-- GeoNameTag("BeamPipe")
|
|-- GeoTransform -- TT1
|-- GeoVPhysVol -- TT1
|
|-- GeoTransform -- TT2
|-- GeoVPhysVol -- TT2
|
|-- GeoTransform -- TT3
|-- GeoVPhysVol -- TT3
```

In that way, we get meaningful, comprehensive checkboxes when visualizing the output tree from the SQLite file in [GMEX](https://gitlab.cern.ch/GeoModelDev/GeoModel/-/tree/master/GeoModelVisualization) ([GeoModelExplorer](https://gitlab.cern.ch/GeoModelDev/GeoModel/-/tree/master/GeoModelVisualization))

![a filtered GeoModel tree in GMEX](docs/img/gmex1.png)





### Additional Options

You can use all the common Athena flags to steer the dump mechanism. 

ZDC geometry compatibility is validated by `ZDC_DetTool` while GeoModel is
initialized. If geometry-service initialization fails before the DumpGeo
algorithm can run, standalone DumpGeo prints a final reminder to inspect the
preceding ZDC tool diagnostics.

With the new CA configuration, you can use the `--help` option to get the list of all available options. 

```bash
$ python -m DumpGeo.DumpGeoConfig —help
```


As soon as we add options to `DumpGeo`, you will get the new ones listed at the bottom of the “help” output, after the common Athena options

```sh
$ python -m DumpGeo.DumpGeoConfig —help

[...Athena options...]

--detDescr TAG                           The ATLAS geometry tag you want to dump (a convenience alias for the Athena flag 'GeoModel.AtlasVersion=TAG') (default: ATLAS-R3S-2021-03-02-00)

--filterDetManagers FILTERDETMANAGERS    Only output the GeoModel Detector Managers specified in the FILTER list; input is a comma-separated list (default: None)

-f, --forceOverwrite                     Force to overwrite an existing SQLite output file with the same name, if any (default: False)
```


## Unit tests

The unit tests in `python/DumpGeoConfig_test.py` provide a regression baseline
for the `DumpGeoCfg` ComponentAccumulator configuration. They exercise the
configuration without running an Athena event loop or creating a geometry
SQLite file.

The tests cover:

* the default `GeoModel.DumpGeo` flags;
* automatic and custom output file names;
* DetectorManager filtering and the corresponding automatic file name;
* the `ShowTreetopContent` property;
* explicit algorithm properties supplied through keyword arguments, including
  authoritative output-file validation and overwrite handling;
* propagation of `ForceOverwrite` without deleting files during configuration;
* rejection of an existing output file when overwrite is disabled;
* non-destructive output-file preflight when overwrite is enabled;
* suppression of configuration-flag dumps below the debug logging level;
* geometry-tag precedence and its use in automatic output filenames;
* the final ZDC diagnostic reminder after a failed standalone run; and
* the configurable algorithm name.

All tests are regular regression tests and are expected to pass.

After building the `DumpGeo` package and setting up the resulting Athena
runtime environment, run the tests directly with:

```sh
python -m unittest -v DumpGeo.DumpGeoConfig_test
```

The tests are also registered with CTest and can be run from the build
directory with:

```sh
ctest -R DumpGeoConfig --output-on-failure
```


## Documentation

You can get more information about the GeoModel tree and the content of the output SQLite file on the GeoModel documentation website: https://cern.ch/geomodel

 
