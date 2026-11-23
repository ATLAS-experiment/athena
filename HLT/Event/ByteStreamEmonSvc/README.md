# ByteStream Event Monitoring Input

This package provides input services that read events from the online
event sampling instead of a file:

  * `ByteStreamWebdaqInputSvc` - the Phase 2 implementation. It requires
    only `tdaq-common`'s [webdaq](https://gitlab.cern.ch/atlas-tdaq-software/webdaq)
    package.
  * `ByteStreamEmonInputSvc` - the former implementation, which depends on the
    full TDAQ release. It will be removed, use `ByteStreamWebdaqInputSvc` instead.

The rest of this document describes `ByteStreamWebdaqInputSvc`.

## Athena Configuration

For a standalone setup use `WebdaqByteStreamCfg`:

```python
from ByteStreamEmonSvc.WebdaqByteStreamConfig import WebdaqByteStreamCfg
acc.merge(WebdaqByteStreamCfg(flags))
```

The following Gaudi properties are used to configure the service:

  * Partition - online partition name, if empty `$TDAQ_PARTITION` is used
  * SamplerType (default `eb`) - the source of events, typically you want the event builder
  * GroupID - a string specifying the group among which events are not shared
    It is recommended to set this explicitly, otherwise a generic and random name will be
    chosen.
  * SamplerNames- an explicit list of sampler names (usually not needed and empty)
  * ReadDetectorMaskFromIS - default: true
  * ProcessCorruptedEvents - default: false, if true, corrupted events are returned
  * Timeout - default: 60 (seconds), timeout when waiting for an event

The actual selection criteria are specified as Python dictionary and converted
to JSON to pass it as a string to the Gaudi property. You typically want
only "streams" and can leave out the others (every field is optional).

```python
Criteria = json.dumps({
  "l1_type": 32,
  "status_mask": 0xff,
  "streams": {
    "physics": [ "MinBias", "Muons" ],
    "calibration": [ "Calib1", "Calib2" ]
    }
})
```

## Testing

By design this package needs a running TDAQ partition and event sampler
to work. For internal testing there is a mock python server that
implements the correct API. It can be used without the need for
the full TDAQ release.

### Example Job

`WebdaqByteStreamJob.py` is an example job that is configured like a
normal job reading a raw data file, and then has the file input replaced
by the sampling service:

```sh
python -m ByteStreamEmonSvc.WebdaqByteStreamJob \
    --partition=test --groupID=myGroup --streams physics:Main --evtMax=10
```

  * No input file is needed. The run information (run number, detector
    mask, ...) comes from the running system via the webproxy. The flags
    that are normally autoconfigured from an input file are set to generic
    online defaults (run number -1, project `data_test`, Run 3) and can be
    overridden on the command line, e.g. `Input.RunNumbers=[123456]`.
  * For offline tests `--filesInput` can still be given to autoconfigure
    the flags from a file. No events are read from it.
  * `--streams` takes a list of `type:name`, `--l1Type` and `--statusMask`
    set the other selection criteria. The other service properties are
    available as `--samplerType`, `--samplerNames`, `--timeout`,
    `--noDetectorMaskFromIS` and `--processCorrupted`.
  * Geometry and conditions tags default to Run 3 and can be overridden
    with the usual flags, e.g. `IOVDb.GlobalTag=...`.
  * By default the job only prints the `xAOD::EventInfo` content of each
    event. Use `--reco` to run the standard reconstruction instead.
  * Run with `--help` for all options.

To use event sampling in your own job, configure it for file input as
usual and call `replaceByteStreamInputWithWebdaq()` from
`WebdaqByteStreamConfig` on the final `ComponentAccumulator`. Keyword
arguments are passed as properties to the service:

```python
from ByteStreamEmonSvc.WebdaqByteStreamConfig import replaceByteStreamInputWithWebdaq
replaceByteStreamInputWithWebdaq(flags, acc, Partition="test", GroupID="myGroup",
                                   Criteria=json.dumps({"streams": {"physics": ["Main"]}}))
```

### Mock Server Test

`test/dummy_webproxy.py` replaces the TDAQ partition, event sampler and
webproxy. It serves the events of a raw data file in a loop, for a
single partition (`--partition`, default `test`). It needs only `eformat`
and `EventStorage` from `tdaq-common` and runs in the athena environment:

```sh
python HLT/Event/ByteStreamEmonSvc/test/dummy_webproxy.py -f my.raw.data -p 8080 &
export TDAQ_WEBDAQ_BASE=http://localhost:8080
```

The data file (`-f`) is required. The RunParams published for the run are
taken from the file:

  * run number and run type from the first event (the run number is
    assumed to be the same for all events)
  * detector mask, beam type and energy, recording enabled flag, trigger
    type and T0 project tag from the file metadata
  * start of run from the time stamp of the first event

The RunParams are printed when the server starts. Then run the athena job
with the same partition name, e.g. the example job above with
`--partition=test`.

### Full TDAQ Test

For a proper test with TDAQ do the following:

  * Open a new terminal with a new session on the same host
  * Setup the TDAQ release of your choice, e.g. `cm_setup nightly`.
  * If you have no access to an initial partition, start one yourself:
    * `mkdir -p run`
    * `export TDAQ_IPC_INIT_REF=file:$(pwd)/run/init.ref`
    * `ipc_server &`
  * Start a test partition with a name of your choice (here `test`)
    * `ipc_server -p test &`
    * `is_server -p test -n Monitoring &`
  * Start an event sampler, preferably with a real data file
    * `event_sampler -p test -t eb -n eb-01 -f my.raw.data &`
  * Start the web proxy on a free port
    * `webproxy --port 8080 &`

Now you can go back to the athena session and configure webdaq:

  * `export TDAQ_WEBDAQ_BASE=http://localhost:8080`

Use the same port number as for the webproxy.

Now start the athena job to check if it is working. In this
example you have to set `Partition = "test"` and `SamplerType = "eb"`.

Note the test sampler in the TDAQ partition does not understand
streams, so use:

```sh
python -m ByteStreamEmonSvc.WebdaqByteStreamJob \
    --partition=test --groupID=myGroup --l1Type=0 --evtMax=10
```
