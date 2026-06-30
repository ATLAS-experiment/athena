# Phase 2 Event Sampling

This package replaces BytestreamEmonInputSvc for Phase 2.

It removes the dependencies on the full TDAQ release that the
former implementation had, and requires only `tdaq-common`'s
[webdaq](https://gitlab.cern.ch/atlas-tdaq-software/webdaq) package.

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
Criteria = json.load({
  "l1_type": 32,
  "status_mask": 0xff
  "streams": {
    "physics": [ "MinBias", "Muons" ],
    "calibration": [ "Calib1", "Calib2" ]
    }
})
```
