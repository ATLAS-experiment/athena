# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""The multi-threaded RPC server job, and the fragments it offers.

The job is a menu of fragments under a parOR: the scheduler offers all of them
every event and the gates decide which one actually runs.

Adding a fragment is one ``menu.add(...)`` in RpcMenuCfg below: the payload is
an ordinary ComponentAccumulator, its wire inputs are read off its own handles,
and its boundaries say how those keys cross (see RpcFragments.py). Nothing in
the loop manager changes, and none of the payload algorithms know they are
being driven over RPC.

Every fragment here crosses on the ``protobuf`` encoding, against the schema in
proto/athexrpc_demo.proto -- which is the demonstration fragment's own, not the
framework's. Converting it is the boundary's business rather than the payload's,
so each payload below is nothing but unmodified Athena algorithms: the codec on
the gate turns the message into the keys they read, and the codec on the pack
algorithm turns what they wrote back into one.
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

from AthExRpcLoop.RpcDemoCodecs import DOUBLES, INTS, DemoDoubles, DemoInts
from AthExRpcLoop.RpcFragments import (
    RPC_REQUEST_KEY, RPC_TOP, RpcMenu, RpcPayloadCfg,
)

__all__ = ["RPC_REQUEST_KEY", "RPC_TOP", "RpcMenuCfg", "RpcHiveLoopCfg",
           "INTS", "DOUBLES"]


def RpcSumPayloadCfg(flags):
    """sum(a, b) -> sum. The minimal fragment.

    One algorithm. The message becomes two ordinary integers and the answer
    becomes a message again, but both conversions belong to the boundary rather
    than to the payload, so the fragment is exactly the work it does.
    """
    return RpcPayloadCfg(
        CompFactory.AthExRpc.SumAlg("RpcSeqSumAlg", A="a", B="b", Sum="sum"),
    )


def RpcChainPayloadCfg(flags, delay=0):
    """(p + q) + r -> chain.

    'pq' is real StoreGate data that never crosses the wire, which is why the
    output list is a choice rather than something to derive: everything here is
    written, only 'chainTotal' is wanted back.
    """
    return RpcPayloadCfg(
        CompFactory.AthExRpc.SumAlg("RpcSeqChainFirst", A="p", B="q", Sum="pq"),
        CompFactory.AthExRpc.DelayAlg("RpcSeqChainDelay", Milliseconds=delay),
        CompFactory.AthExRpc.SumAlg("RpcSeqChainSecond", A="pq", B="r",
                                    Sum="chain"),
    )


def RpcCondPayloadCfg(flags):
    """A payload that is more than a list of algorithms.

    It brings its own conditions algorithm, which ends up in AthCondSeq rather
    than in the fragment's sequence -- the thing that would be lost if
    RpcFragmentCfg took bare configurables. The scheduler works out that the
    CondAlg has to run before OffsetAlg.

    It is also the case that proves conditions reads are not mistaken for wire
    inputs: OffsetAlg reads 'RpcCondOffset' and nothing in this CA produces it
    into the event store, but it lives in ConditionStore, so the derivation
    leaves it alone and only 'value' is asked of the client.
    """
    cfg = ComponentAccumulator()
    cfg.addEventAlgo(CompFactory.AthExRpc.OffsetAlg("RpcSeqCondAlg",
                                                    Input="x",
                                                    Offset="RpcCondOffset",
                                                    Output="offsetted"))
    cfg.addCondAlgo(CompFactory.AthExRpc.RpcCondAlg("RpcCondAlg",
                                                    Offset="RpcCondOffset"))
    return cfg


def RpcScalePayloadCfg(flags, factor=2.0):
    """An array rather than a handful of scalars, on the same machinery."""
    return RpcPayloadCfg(
        CompFactory.AthExRpc.ScaleVectorAlg(
            "RpcSeqScaleAlg", Input="values", Output="scaled", Factor=factor),
    )


def RpcPingPayloadCfg(flags):
    """A fragment with no inputs at all.

    Worth having for two reasons. It is the only fragment a canned request can
    drive to a successful reply -- canned requests carry no payloads, because
    the loop manager has no way to build one without knowing a fragment's schema
    -- and it is the shape of a server that is asked to *produce* something
    rather than transform something.
    """
    return RpcPayloadCfg(
        CompFactory.AthExRpc.DemoNumbersAlg(
            "RpcSeqPingAlg", Values=["tick"], Offsets=[0]),
    )


def RpcMenuCfg(flags, delay=0, factor=2.0):
    """Every fragment this server offers.

    Note what is *not* written here: no input keys and no StoreGate types. Both
    come out of the payload accumulators' own handles. What is written is the
    wire contract -- which keys cross, on what encoding, against what schema --
    because that is a decision rather than a derivation.
    """
    menu = RpcMenu()

    # Only RpcSeqPing needs `produces=`, and for a real reason: DemoNumbersAlg
    # writes through a handle array, which ComponentAccumulator.getIO() cannot
    # see. Everywhere else the keys a message carries come from the boundary's
    # own codec, which says which they are, so the derivation needs no
    # exemption.
    menu.add("RpcSeqSum", RpcSumPayloadCfg(flags),
             outputs=["total"],
             boundaries={"addends": DemoInts(["a", "b"]),
                         "total": DemoInts(["sum"])})

    menu.add("RpcSeqChain", RpcChainPayloadCfg(flags, delay=delay),
             outputs=["chainTotal"],
             boundaries={"terms": DemoInts(["p", "q", "r"]),
                         "chainTotal": DemoInts(["chain"])})

    menu.add("RpcSeqCond", RpcCondPayloadCfg(flags),
             outputs=["offsetResult"],
             boundaries={"value": DemoInts(["x"]),
                         "offsetResult": DemoInts(["offsetted"])})

    menu.add("RpcSeqScale", RpcScalePayloadCfg(flags, factor=factor),
             outputs=["scaledPoints"],
             boundaries={"points": DemoDoubles("values"),
                         "scaledPoints": DemoDoubles("scaled")})

    menu.add("RpcSeqPing", RpcPingPayloadCfg(flags),
             outputs=["pong"], produces=["tick"],
             boundaries={"pong": DemoInts(["tick"])})

    # Always fails, to exercise the error path. No outputs, so also covers the
    # "fragment stages no reply" case.
    menu.add("RpcSeqFail",
             RpcPayloadCfg(CompFactory.AthExRpc.FailAlg("RpcSeqFailAlg")))
    return menu


def RpcHiveEventLoopMgrCfg(flags, menu, **kwargs):
    """The loop manager, configured with the menu it serves."""
    cfg = ComponentAccumulator()
    for key, value in menu.properties.items():
        kwargs.setdefault(key, value)
    kwargs.setdefault("SchedulerSvc", "AvalancheSchedulerSvc")
    cfg.addService(CompFactory.AthExRpc.RpcHiveEventLoopMgr(
        "RpcHiveEventLoopMgr", **kwargs))
    return cfg


def RpcHiveLoopCfg(flags, delay=0, **kwargs):
    """The whole job: Hive main services with our loop manager, plus the menu."""
    if flags.Concurrency.NumThreads < 1:
        raise ValueError("RpcHiveLoopCfg needs Concurrency.NumThreads >= 1")

    # MainServicesCfg picks the loop manager from the flags, so it configures
    # AthenaHiveEventLoopMgr here. That is what we want for everything *around*
    # the manager -- HiveMgrSvc, AlgResourcePool, AvalancheSchedulerSvc -- so we
    # take the whole component set and then point the application at ours
    # instead. The AthenaHiveEventLoopMgr entry stays in the configuration
    # unused; nothing instantiates it.
    cfg = MainServicesCfg(flags)
    cfg.setAppProperty(
        "EventLoop", "AthExRpc::RpcHiveEventLoopMgr/RpcHiveEventLoopMgr",
        overwrite=True)

    # The gates read the request through a ReadHandle, but its producer is the
    # loop manager -- a service, invisible to the dependency solver. Left alone,
    # AutoLoadUnmetDependencies hands the key to SGInputLoader, which then
    # reports it as an unprovided transient object once per event. Declaring it
    # as an extra output of SGInputLoader closes the graph honestly: it says
    # "this appears in the store before the algorithms run". Exactly what
    # SGInputLoaderCfg itself does for the EventInfo of inputless jobs.
    cfg.getEventAlgo("SGInputLoader").ExtraOutputs.add(
        ("AthExRpc::RpcRequestDescriptor", f"StoreGateSvc+{RPC_REQUEST_KEY}"))

    # One menu, used twice: once to build the sequences and once to tell the
    # loop manager what it serves. They cannot drift.
    menu = RpcMenuCfg(flags, delay=delay)
    cfg.merge(menu.build(flags))
    cfg.merge(RpcHiveEventLoopMgrCfg(flags, menu, **kwargs))
    return cfg


if __name__ == "__main__":
    import sys
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Exec.MaxEvents = -1          # serve until stopped
    flags.Concurrency.NumThreads = 2
    flags.Concurrency.NumConcurrentEvents = 2

    parser = flags.getArgumentParser()
    parser.add_argument("--port", type=int, default=0,
                        help="TCP port to bind; 0 lets the OS choose")
    parser.add_argument("--port-file", default="",
                        help="File to write the bound port to")
    parser.add_argument("--test-requests", default="",
                        help="Semicolon separated sequence names to serve with "
                             "no socket, e.g. 'RpcSeqPing;RpcSeqFail'. They "
                             "carry no payloads; see the TestRequests property")
    parser.add_argument("--delay", type=int, default=0,
                        help="Milliseconds RpcSeqChain sleeps, to make "
                             "concurrent requests overlap")
    parser.add_argument("--run-number", type=int, default=1,
                        help="Run number of the synthetic EventID; the "
                             "conditions fragment's answer depends on it")
    parser.add_argument("--queue-limit", type=int, default=128,
                        help="Queued requests before clients are refused with "
                             "RESOURCE_EXHAUSTED")
    parser.add_argument("--request-timeout", type=float, default=0.0,
                        help="Seconds before a request is answered TIMEOUT; "
                             "0 disables")
    args = flags.fillFromArgs(parser=parser)
    flags.lock()

    kwargs = {"Port": args.port, "PortFile": args.port_file,
              "QueueLimit": args.queue_limit,
              "RequestTimeout": args.request_timeout,
              "RunNumber": args.run_number}
    if args.test_requests:
        kwargs["TestRequests"] = args.test_requests.split(";")

    cfg = RpcHiveLoopCfg(flags, delay=args.delay, **kwargs)
    sys.exit(0 if cfg.run().isSuccess() else 1)
