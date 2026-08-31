# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Declaring an RPC fragment in one place.

A fragment's boundary has to be described to several components: the gate
(which keys to unpack, with what encoding and schema), the pack algorithm
(which keys to read back, and where to stage the reply), and the loop manager's
menu. Writing those out by hand does not survive many fragments -- and the
failure mode is unpleasant, because a disagreement between the loop manager's
declared inputs and the gate's actual ones lets a request past the cheap
up-front check only to fail inside the scheduler.

So a fragment is declared once and everything else is derived from it. Two
parts of the declaration are derived, and one is not:

- the **input keys** are whatever the payload reads but does not produce. An
  unmet read has nowhere to come from except the client, so this is not a
  heuristic; it is the definition.
- the **StoreGate type** of every boundary comes from the handle that reads or
  writes it. ``ComponentAccumulator.getIO()`` reports type, key and mode for
  every handle in a CA, so the type is already stated in the one place that
  cannot drift from the code -- the algorithm's own handle declaration. It is
  needed for the scheduler dependencies, and for nothing else.
- the **encoding and schema** are stated by hand, per boundary, and there is no
  default. They are a wire contract, and deriving a wire format from a host
  type is how you get a boundary whose serialisation changes silently when
  somebody edits a handle. What a payload *is* on the wire is a decision, so
  the fragment makes it explicitly.

What is left to state besides that is the fragment's name and which of the keys
it produces should go back to the client. Neither is derivable: the second is a
choice, and an intermediate that stays in the store is a legitimate one.
"""

from AthenaCommon.CFElements import parOR, seqAND
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging

_msg = logging.getLogger("RpcFragments")

#: The encoding the framework itself implements: opaque bytes, recorded under
#: the boundary's key as an ``AthExRpc::RpcBlob`` and converted by the fragment
#: that published the schema. This is a convenience spelling, not an
#: enumeration the system depends on -- an encoding is a string resolved against
#: a codec registry at run time, and adding one costs nothing here at all.
PROTOBUF = "protobuf"

#: The StoreGate type a ``protobuf`` boundary materialises as.
BLOB_TYPE = "AthExRpc::RpcBlob"

#: Stores whose contents a client never supplies: a conditions read is served
#: by AthCondSeq and a detector-store read by the geometry, so neither is an
#: unmet dependency in the sense that matters here.
_NON_EVENT_STORES = frozenset(("ConditionStore+", "DetectorStore+"))


def handlePath(key):
    """The full path of a key as getIO() reports it, e.g. 'StoreGateSvc+a'.

    getIO() gives a plain string for a handle left at its C++ default and a
    DataHandle for one the configuration set (ComponentAccumulator.py:305), so
    both have to be accepted.
    """
    return str(getattr(key, "Path", key))


def _split_store(path):
    """'StoreGateSvc+a' -> ('StoreGateSvc+', 'a'); 'a' -> ('', 'a')."""
    store, plus, key = path.rpartition("+")
    return store + plus, key


def _is_conditions(key, store):
    """Is this handle reading conditions rather than event data?

    Two ways to tell, and both are needed. A key can carry its store in the
    path, which is the easy case. But a ReadCondHandleKey whose configured key
    is a COOL folder -- '/Indet/Align' -- has no prefix at all, and taking it
    for event data is not a harmless mistake: it becomes a wire input, and the
    client is asked to send the alignment. The handle itself knows:
    AtlasSemantics.VarHandleKeySemantics sets IsCondition from the C++ handle
    type, which is the authority here rather than the shape of the string.
    """
    return store in _NON_EVENT_STORES or getattr(key, "IsCondition", False)


def deriveBoundaries(payload, outputs, request_key, extra_inputs=(),
                     ignore_inputs=(), produces=()):
    """Work out which keys cross a fragment's boundary, and their types.

    :param payload: the payload ComponentAccumulator
    :param outputs: keys to return to the client
    :param request_key: the request descriptor's key, which every gate reads and
        which must never be mistaken for a wire input
    :param extra_inputs: keys to treat as client inputs even though nothing in
        the payload reads them -- for a fragment whose consumer is added later
    :param ignore_inputs: unmet reads that are *not* wire inputs, because the
        payload tolerates their absence. "Unmet read" is otherwise a sound
        definition of a wire input, and this is the one thing it cannot see --
        an optional dependency looks exactly like a required one from the
        outside. Naming them here is a claim that the payload copes, so it is
        deliberately explicit rather than a heuristic about which types look
        optional.
    :param produces: keys the payload writes that ``getIO()`` cannot see.
        It reports single handles, ExtraInputs and ExtraOutputs, but not
        ``SG::VarHandleKeyArray`` properties -- so an algorithm writing through
        a WriteHandleKeyArray looks, from Python, as though it writes nothing,
        and its keys would be charged to the client as unmet reads. The
        scheduler is not fooled (AthCommonDataStore registers array handles as
        real dependencies after initialize), which is exactly why this shows up
        as a wrong boundary rather than as a broken job.
    :return: ``(inputs, types)``, where inputs is the derived list of client
        input keys and types maps every boundary key to its StoreGate type

    Raises ValueError if a declared output is not produced, if two consumers of
    one key disagree about its type, or if an input's type cannot be determined.
    """
    reads, writes, types = {}, set(), {}
    for handle in payload.getIO():
        store, key = _split_store(handlePath(handle["key"]))
        if not key or _is_conditions(handle["key"], store):
            continue
        if handle["mode"] == "W":
            writes.add(key)
            # A writer is authoritative: it names the concrete type, where a
            # reader may name a base class.
            types[key] = handle["type"]
        else:
            reads.setdefault(key, set()).add(handle["type"])

    stale_writes = [k for k in produces if k in writes]
    if stale_writes:
        raise ValueError(
            f"produces names {sorted(stale_writes)}, which the payload already "
            "declares in a way the derivation can see. Either the handle "
            "stopped being an array or the key is wrong; either way the "
            "exemption is now hiding nothing")
    writes.update(produces)

    unmet = set(k for k in reads if k not in writes)
    stale = [k for k in ignore_inputs if k not in unmet]
    if stale:
        raise ValueError(
            f"ignore_inputs names {sorted(stale)}, which the payload does not "
            "leave unmet. Either the key is wrong or the payload changed; "
            "either way the exemption is now hiding nothing")
    inputs = sorted(unmet - {request_key} - set(ignore_inputs))
    inputs += [k for k in extra_inputs if k not in inputs]

    for key in inputs:
        if key in types:
            continue
        candidates = reads.get(key, set())
        if len(candidates) != 1:
            raise ValueError(
                f"cannot derive the type of RPC input '{key}': "
                + (f"its consumers disagree ({sorted(candidates)})"
                   if candidates else "nothing in the payload reads it")
                + ". Declare it explicitly with types={'" + key + "': ...}")
        types[key] = next(iter(candidates))

    missing = [key for key in outputs if key not in writes]
    if missing:
        raise ValueError(
            f"the payload does not produce {missing}; it writes "
            f"{sorted(writes)}")
    return inputs, types


def boundarySpecs(declarations):
    """Boundaries as the strings the algorithms parse: ``key#encoding#schema``.

    '#' rather than ':' or ',' because a schema may be a C++ type name, and
    those contain both.
    """
    return [f"{key}#{encoding}#{schema}"
            for key, _, encoding, schema in declarations]


def boundaryDeps(declarations):
    """Boundaries as scheduler dependencies.

    A boundary key cannot be a typed handle key -- the type is not known at
    compile time, which is the whole point -- so the gate cannot declare itself
    its producer the usual way, and the pack algorithm cannot declare itself its
    consumer. Saying it here keeps the data-flow graph closed.

    Note this is the *StoreGate* type, not the schema: what the scheduler orders
    is the appearance of an object under a key, and for a protobuf boundary that
    object is a blob whatever message is inside it.
    """
    return {(sg_type, f"StoreGateSvc+{key}")
            for key, sg_type, _, _ in declarations}


def RpcRequestAlgCfg(name, sequence, target, inputs=(), outputs=(), **kwargs):
    """The client-side algorithm, from the same declarations the fragment makes.

    A client that imports its server's fragment declaration and passes it here
    cannot disagree with the server about what crosses -- which is the whole
    argument for both ends being Athena. A client that cannot import it (a
    different release, a different language) discovers the same three strings
    from ListSequences instead.
    """
    alg = CompFactory.AthExRpc.RpcRequestAlg(
        name,
        SequenceName=sequence,
        Target=target,
        Inputs=boundarySpecs(inputs),
        Outputs=boundarySpecs(outputs),
        **kwargs)
    alg.ExtraInputs = boundaryDeps(inputs)
    alg.ExtraOutputs = boundaryDeps(outputs)
    return alg


class RpcFragment:
    """The declaration of one entry on the server's menu.

    Usually built for you by :class:`RpcMenu`; construct one directly only to
    state a boundary that cannot be derived.

    :param name: the sequence name, which is also what a request asks for
    :param inputs: ``(key, sg_type, encoding, schema)`` the client must supply
    :param outputs: the same, returned to the client
    :param reply_key: where the pack algorithm stages the reply; derived from
        the name unless given. A fragment with no outputs needs none, and gets
        no pack algorithm at all.

    Note there is no list of supported types anywhere: a boundary is a key, an
    encoding and a schema name, all strings resolved at run time.
    """

    def __init__(self, name, inputs=(), outputs=(), reply_key=None):
        self.name = name
        self.inputs = [tuple(d) for d in inputs]
        self.outputs = [tuple(d) for d in outputs]
        if outputs and reply_key is None:
            reply_key = "RpcReply" + name.removeprefix("RpcSeq")
        self.reply_key = reply_key

    @property
    def input_specs(self):
        return boundarySpecs(self.inputs)

    @property
    def output_specs(self):
        return boundarySpecs(self.outputs)

    def gate(self, request_key):
        """The algorithm that selects this fragment and unpacks its inputs."""
        alg = CompFactory.AthExRpc.RpcGateAlg(
            f"{self.name}Gate",
            SequenceName=self.name,
            Request=request_key,
            Inputs=self.input_specs,
        )
        alg.ExtraOutputs = boundaryDeps(self.inputs)
        return alg

    def pack(self, request_key):
        """The algorithm that reads this fragment's outputs back, if any."""
        if not self.outputs:
            return None
        alg = CompFactory.AthExRpc.RpcPackAlg(
            f"{self.name}Pack",
            Request=request_key,
            Reply=self.reply_key,
            Outputs=self.output_specs,
        )
        # Only the outputs. What orders this algorithm after the whole payload
        # is that a fragment is a seqAND, which CFElements builds with
        # Sequential=True and to which the pack algorithm is appended last.
        alg.ExtraInputs = boundaryDeps(self.outputs)
        return alg


def RpcPayloadCfg(*algorithms):
    """Convenience for the common case: a payload that is just some algorithms.

    Only for fragments simple enough not to need anything else. Anything real
    should pass its own ``SomethingCfg(flags)`` accumulator straight to
    :func:`RpcFragmentCfg`.
    """
    cfg = ComponentAccumulator()
    for algorithm in algorithms:
        cfg.addEventAlgo(algorithm)
    return cfg


def RpcFragmentCfg(flags, fragment, payload, parent, request_key):
    """Assemble one fragment: gate, the payload, then pack.

    ``payload`` is a ComponentAccumulator, not a list of algorithms, so that an
    existing subsystem configuration can be dropped in whole -- with its
    services, conditions algorithms, private tools and any internal sequence
    structure intact. Taking a list of configurables instead would force every
    caller to decompose a perfectly good CA into raw components and lose
    everything that is not an event algorithm.

    Its algorithms are merged into this fragment's sequence, between the gate
    and the pack algorithm; everything else in it merges normally. They stay
    ordinary Athena algorithms with ordinary typed handles, and the same
    configuration would work in a normal job -- which is the property the
    gate/pack pair exists to preserve.

    ``parent`` is the sequence the fragment hangs under, passed as a
    configurable rather than a name so that each fragment's accumulator is
    self-contained and they can be merged in any order.
    """
    cfg = ComponentAccumulator()
    cfg.addSequence(parent, parentName="AthAlgSeq")
    cfg.addSequence(seqAND(fragment.name), parentName=parent.name)
    cfg.addEventAlgo(fragment.gate(request_key), sequenceName=fragment.name)
    cfg.merge(payload, sequenceName=fragment.name)
    pack = fragment.pack(request_key)
    if pack is not None:
        cfg.addEventAlgo(pack, sequenceName=fragment.name)
    return cfg


#: Defaults for a job that serves one menu, which is every job so far.
RPC_TOP = "RpcTop"              #: the parOR every fragment hangs under
RPC_REQUEST_KEY = "RpcRequest"  #: where the loop manager stages the request


class RpcMenu:
    """Every fragment a server offers, and the configuration that follows.

    One call per fragment, and the menu the loop manager is told about and the
    sequences it drives come out of the same object -- so they cannot disagree,
    which is the failure this module exists to prevent.

    Typical use::

        menu = RpcMenu()
        menu.add("RpcSeqSum", RpcSumPayloadCfg(flags),
                 outputs=["total"],
                 boundaries={"addends": (PROTOBUF, "athexrpc.demo.v1.Ints"),
                             "total": (PROTOBUF, "athexrpc.demo.v1.Ints")})
        cfg.merge(menu.build(flags))
    """

    def __init__(self, request_key=RPC_REQUEST_KEY, top=RPC_TOP):
        self.request_key = request_key
        self.top = top
        self.fragments = []
        self._payloads = {}

    def add(self, name, payload, outputs=(), boundaries=None, types=None,
            inputs=None, ignore_inputs=(), produces=(), reply_key=None):
        """Offer one fragment.

        :param name: the sequence name, which is what a request asks for
        :param payload: the payload ComponentAccumulator, configured exactly as
            it would be for a normal job
        :param outputs: the keys to send back to the client. Not derivable: an
            intermediate that stays in the store is a legitimate choice.
        :param boundaries: ``{key: (encoding, schema)}`` for every key that
            crosses, inputs and outputs alike. Required, and a key without an
            entry is an error rather than a default -- see the module docstring.
        :param types: overrides for derived StoreGate types,
            ``{key: type_name}``. Needed when a consumer reads a base class
            rather than the concrete type, which is the one case the derivation
            gets wrong rather than refusing.
        :param inputs: if given, an assertion rather than a declaration: these
            keys exactly, and a mismatch with what the payload actually leaves
            unmet is an error. Use it on a fragment whose contract matters.
        :param ignore_inputs: unmet reads the payload tolerates being absent,
            so they are not asked of the client. See deriveBoundaries.
        :param produces: keys the payload writes through a handle array, which
            Python cannot see. See deriveBoundaries.
        """
        types = dict(types or {})
        boundaries = dict(boundaries or {})
        derived, derived_types = deriveBoundaries(
            payload, outputs, self.request_key,
            extra_inputs=[k for k in (inputs or ()) if k in types],
            ignore_inputs=ignore_inputs, produces=produces)
        derived_types.update(types)

        if inputs is not None and sorted(inputs) != sorted(derived):
            raise ValueError(
                f"fragment '{name}' declares inputs {sorted(inputs)} but its "
                f"payload leaves {sorted(derived)} unmet")

        crossing = list(derived) + list(outputs)
        undeclared = [k for k in crossing if k not in boundaries]
        if undeclared:
            raise ValueError(
                f"fragment '{name}' does not say how {sorted(undeclared)} "
                "cross: give each an (encoding, schema) in boundaries=. There "
                "is no default; a wire format is not derivable from a C++ type")
        unused = [k for k in boundaries if k not in crossing]
        if unused:
            # A boundary declared for a key that does not cross is a rename
            # somebody half-finished. Left alone it is silent, and the key it
            # was meant to describe falls back to... nothing, above.
            raise ValueError(
                f"fragment '{name}' declares boundaries for {sorted(unused)}, "
                f"which do not cross; it crosses {sorted(crossing)}")

        def declare(keys):
            return [(key, derived_types[key]) + tuple(boundaries[key])
                    for key in keys]

        fragment = RpcFragment(name,
                               inputs=declare(derived),
                               outputs=declare(list(outputs)),
                               reply_key=reply_key)
        # Said out loud because a derived boundary is otherwise invisible: a
        # forgotten addCondAlgo turns an internal read into a mandatory client
        # input, and the first sign of it would be every request failing.
        _msg.info("RPC fragment %s: in %s, out %s", name,
                  fragment.input_specs or "-", fragment.output_specs or "-")
        self.fragments.append(fragment)
        self._payloads[name] = payload
        return fragment

    def build(self, flags):
        """The parOR, every fragment's sequence, and the algorithms between."""
        cfg = ComponentAccumulator()
        cfg.addSequence(parOR(self.top), parentName="AthAlgSeq")
        for fragment in self.fragments:
            cfg.merge(RpcFragmentCfg(flags, fragment,
                                     self._payloads[fragment.name],
                                     parOR(self.top), self.request_key))
        return cfg

    @property
    def properties(self):
        """The loop manager's view, from the same declarations."""
        props = RpcMenuProperties(self.fragments)
        props["RequestKey"] = self.request_key
        return props


def RpcMenuProperties(fragments):
    """The loop manager's view of the menu, derived from the same declarations."""
    return {
        "Sequences": [f.name for f in fragments],
        "Inputs": {f.name: f.input_specs for f in fragments if f.inputs},
        "Outputs": {f.name: f.output_specs for f in fragments if f.outputs},
        "ReplyKeys": {f.name: f.reply_key for f in fragments if f.reply_key},
    }
