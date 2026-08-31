# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""An ordinary Athena job that calls the demonstration fragment.

This is the shape the whole exercise is about: a job with a normal event loop,
normal algorithms and normal typed handles, one of whose algorithms happens to
run somewhere else. Nothing in it is written twice -- the boundary is declared
once, by the server's fragment, and imported here.

The order is the mirror image of the server's::

    DemoNumbersAlg      writes ordinary integers          (knows nothing)
    DemoIntsPackAlg     integers   -> athexrpc.demo.v1.Ints
    RpcRequestAlg       sends the message, records the reply's bytes
    DemoIntsUnpackAlg   athexrpc.demo.v1.Ints -> integers
    DemoCheckAlg        reads ordinary integers, asserts the answer
                                                          (knows nothing)

The two adapter algorithms are the same C++ classes the server runs, configured
the other way round. That is what makes the two ends unable to disagree about
what the bytes mean, and it is the property a server whose code is a different
implementation cannot have.
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from McEventSelector.McEventSelectorConfig import McEventSelectorCfg

from AthExRpcLoop.RpcFragments import BLOB_TYPE, PROTOBUF, RpcRequestAlgCfg
from AthExRpcLoop.RpcHiveLoopConfig import INTS

__all__ = ["RpcDemoClientCfg"]


def RpcDemoClientCfg(flags, target, addends=(3, 4), **kwargs):
    """Call RpcSeqSum once per event and check what comes back.

    :param target: "host:port", or the path of a file the server writes its
        bound port into -- which is how a test can start the two in either
        order and let the OS choose the port.
    :param addends: what to add to the event number for each of the two
        numbers, so that no two events send the same request. A server that
        answered from a cache, or answered the wrong request, would be caught by
        DemoCheckAlg rather than by the job succeeding.
    """
    cfg = MainServicesCfg(flags)
    cfg.merge(McEventSelectorCfg(flags))

    cfg.addEventAlgo(CompFactory.AthExRpc.DemoNumbersAlg(
        "RpcDemoNumbers", Values=["a", "b"], Offsets=list(addends)))
    cfg.addEventAlgo(CompFactory.AthExRpc.DemoIntsPackAlg(
        "RpcDemoPack", Values=["a", "b"], Payload="addends"))

    # The declaration, in the four-tuple form RpcFragments uses everywhere:
    # (key, StoreGate type, encoding, schema). A client in another release, or
    # another language, gets the same three wire strings from ListSequences
    # instead of importing them.
    cfg.addEventAlgo(RpcRequestAlgCfg(
        "RpcDemoRequest",
        sequence="RpcSeqSum",
        target=target,
        inputs=[("addends", BLOB_TYPE, PROTOBUF, INTS)],
        outputs=[("total", BLOB_TYPE, PROTOBUF, INTS)],
        **kwargs))

    cfg.addEventAlgo(CompFactory.AthExRpc.DemoIntsUnpackAlg(
        "RpcDemoUnpack", Payload="total", Values=["sum"]))
    cfg.addEventAlgo(CompFactory.AthExRpc.DemoCheckAlg(
        "RpcDemoCheck", Values=["a", "b"], Result="sum"))
    return cfg


if __name__ == "__main__":
    import sys
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Exec.MaxEvents = 10
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1

    parser = flags.getArgumentParser()
    parser.add_argument("--target", required=True,
                        help="host:port, or a file the server writes its port "
                             "into")
    parser.add_argument("--ready-timeout", type=float, default=120.0,
                        help="Seconds to wait for the server to answer")
    args = flags.fillFromArgs(parser=parser)
    flags.lock()

    cfg = RpcDemoClientCfg(flags, args.target, ReadyTimeout=args.ready_timeout)
    sys.exit(0 if cfg.run().isSuccess() else 1)
