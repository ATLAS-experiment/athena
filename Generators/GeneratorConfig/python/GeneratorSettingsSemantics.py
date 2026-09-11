# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from enum import Enum, IntEnum
import re

from GaudiConfig2.semantics import getSemanticsFor, SequenceSemantics
from AthenaCommon.Logging import logging

genSettingsLog = logging.getLogger("GeneratorSettingsSemantics")


class GeneratorSettingsKeep(str, Enum):
    """
    Which duplicate setting survives after layers are sorted by precedence.

    Layers are resolved in increasing precedence order: BASE, then TUNE, then
    USER. A duplicate is either a parsed command with the same normalized key,
    or an unparsed command with the same normalized full text.

    FIRST keeps the first duplicate encountered. This preserves lower-precedence
    defaults when later layers repeat the same setting.

    LAST keeps the last duplicate encountered. This is the normal generator
    behavior: tune settings override base settings, and user settings override
    both.
    """
    FIRST = "first"
    LAST = "last"


class GeneratorSettingsRecord(str, Enum):
    """
    How one command string is understood during merging.

    Each command is parsed with the separators from GeneratorSettingsLayer.
    Commands that contain one of those separators are treated as parsed
    key/value commands, e.g. "Main:timesAllowErrors = 500". These can override
    each other by key.

    Commands without any configured separator are unparsed commands. They have
    no key, so they can only be deduplicated when their normalized full text is
    identical.
    """
    PARSED_COMMAND = "parsed_command"
    UNPARSED_COMMAND = "unparsed_command"


class GeneratorSettingsPrecedence(IntEnum):
    """
    Precedence levels for generator settings layers.
    Lower precedence layers are overridden by higher ones during merging.
    """
    BASE = 10
    TUNE = 20
    MATCHING = 30
    WEIGHTS = 40
    USER = 100


class GeneratorSettingsLayer:
    """
    A layer contains all generator settings from a single source,
    e.g. a user fragment, tune, etc.

    Normal assignments still use a plain sequence of strings. Layers are only
    needed by configuration fragments that want to describe where commands came
    from and how they should be ordered during CA merging.
    """

    def __init__(self,
                 source,
                 values,
                 precedence,
                 separators="=",
                 keep=GeneratorSettingsKeep.LAST,
                 report_context=None):
        if isinstance(values, str):
            raise TypeError("GeneratorSettingsLayer values must be a sequence")

        # Label used in conflict/duplicate reports
        self.source = str(source)

        # Generator command strings belonging to this layer
        self.values = tuple(values or ())

        # Precedence used for deduplication and conflict resolution during merging
        self.precedence = GeneratorSettingsPrecedence(precedence)

        # Separators used to parse a generator setting command into a key/value
        # pair for duplicate detection
        if isinstance(separators, str):
            separators = [separators]
        self.separators = tuple(separators)

        # Which matching key wins after sorting by layer precedence
        self.keep = GeneratorSettingsKeep(keep)

        # Text that should be mentioned in the deduplication report, 
        # usually the property name from the fragment.
        self.report_context = report_context


class GeneratorSettingsValue:
    """
    Stored property value used by GeneratorSettingsSemantics.

    It keeps the unresolved layers rather than a pre-merged list so that
    CA merges remain associative: base.merge(user) and user.merge(base) 
    yield the same result.
    """
    def __init__(self, layers=None):
        self.layers = []
        self._reported_contexts = set()
        seen = set()

        for layer in layers or ():
            if not isinstance(layer, GeneratorSettingsLayer):
                layer_type = type(layer).__name__
                raise TypeError(
                    f"GeneratorSettingsValue layers must be "
                    f"GeneratorSettingsLayer instances, got {layer_type}"
                )

            # Deduplicate identical layers explicitly instead of relying on
            # object identity. This is what makes repeated symmetric CA merges
            # yield the same result.
            key = (
                layer.source,
                layer.values,
                layer.precedence,
                layer.separators,
                layer.keep,
                layer.report_context,
            )
            if key not in seen:
                self.layers.append(layer)
                seen.add(key)

    @property
    def data(self):
        return self.resolve()

    def resolve(self, report_context=None, emit_report=False):
        """
        Resolve all layers to the final settings list.

        If emit_report is True, print duplicate/conflict warnings once using
        report_context (or a layer-provided report_context if available).
        """
        settings, report = self._resolve()

        if emit_report:
            for settings_layer in _ordered_layers(self.layers):
                if settings_layer.report_context:
                    report_context = settings_layer.report_context
                    break

            if report_context and report_context not in self._reported_contexts:
                _log_report(report_context, report)
                self._reported_contexts.add(report_context)

        return settings

    def __str__(self):
        """
        Return the resolved list as a string.
        ComponentAccumulator.gatherProps() converts stored property values
        to strings before handing them to Gaudi.
        """
        return str(self.resolve(emit_report=True))

    def _resolve(self):
        """Return the final command list plus the duplicate/conflict report."""
        layers = _ordered_layers(self.layers)
        separators = layers[0].separators if layers else ("=",)
        keep = layers[0].keep if layers else GeneratorSettingsKeep.LAST
        precedences_seen = {}
        for layer in layers:
            if (
                layer.separators == separators
                and layer.keep == keep
            ):
                if layer.precedence in precedences_seen:
                    previous_layer = precedences_seen[layer.precedence]
                    raise ValueError(
                        f"cannot merge generator settings with duplicate "
                        f"precedence {layer.precedence.name}: "
                        f"{previous_layer.source} and {layer.source}"
                    )
                precedences_seen[layer.precedence] = layer
                continue

            summary = ", ".join(
                f"{entry.source}: separators={entry.separators}, "
                f"keep={entry.keep.value}"
                for entry in layers
            )
            raise ValueError(
                f"cannot merge generator settings with different parsing "
                f"settings: "
                f"{summary}"
            )

        records = _build_records(layers, separators)
        kept_records, removed_records = _deduplicate_records(records, keep)
        return (
            [record["original_setting"] for record in kept_records],
            _build_report(records, kept_records, removed_records),
        )


class GeneratorSettingsSemantics(SequenceSemantics):
    """
    Semantics for generator settings properties.

    Gaudi sees the property as a sequence of strings, but CA merging receives
    GeneratorSettingsLayer objects so fragments can be combined by precedence
    before the final list is handed to the C++ component.
    """
    __handled_types__ = [re.compile(r"^GeneratorSettings<.*>$")]

    def __init__(self, cpp_type):
        # SequenceSemantics needs the element semantics even though we store a
        # custom helper value instead of Gaudi's normal list helper.
        super().__init__(cpp_type, valueSem=getSemanticsFor("std::string"))

    def store(self, assigned_value):
        """Convert user input into the internal unresolved layer container."""
        if isinstance(assigned_value, GeneratorSettingsValue):
            return assigned_value
        if isinstance(assigned_value, GeneratorSettingsLayer):
            return GeneratorSettingsValue([assigned_value])
        if isinstance(assigned_value, str):
            raise TypeError("generator settings must be assigned from a sequence")
        return GeneratorSettingsValue([
            GeneratorSettingsLayer(
                source=self.name or "<unknown>",
                values=tuple(assigned_value or ()),
                precedence=GeneratorSettingsPrecedence.USER,
                report_context=self.name,
            )
        ])

    def default(self, default_commands):
        """Store C++ defaults as the lowest-precedence command layer."""
        if not default_commands:
            return GeneratorSettingsValue()
        if isinstance(default_commands, str):
            raise TypeError("generator settings must be assigned from a sequence")
        return GeneratorSettingsValue([
            GeneratorSettingsLayer(
                source=self.name or "<default>",
                values=tuple(default_commands or ()),
                precedence=GeneratorSettingsPrecedence.BASE,
                report_context=self.name,
            )
        ])

    def merge(self, current_value, incoming_value):
        """Merge two unresolved layer sets during CA merging."""
        current_settings = self.store(current_value)
        incoming_settings = self.store(incoming_value)
        return GeneratorSettingsValue(
            current_settings.layers + incoming_settings.layers
        )

    def opt_value(self, stored_value):
        """
        Return the final plain command list.
        This is the point where all layers are resolved. 
        Print duplicate and conflict warnings here.
        """
        settings_value = self.store(stored_value)
        return settings_value.resolve(report_context=self.name, emit_report=True)


def _ordered_layers(layers):
    """Apply precedence before deduplication."""
    return sorted(
        layers,
        key=lambda layer: (
            int(layer.precedence),
            layer.source,
            repr(layer.values),
        ),
    )


def _build_records(layers, separators):
    """
    Convert raw command strings into normalized records.
    Parsed commands are deduplicated by key. Commands that cannot be parsed as
    key/value records are deduplicated by their normalized full text.
    """
    records = []
    for layer in layers:
        for raw_setting in layer.values:
            key_text, value_text = _parse_assignment(raw_setting, separators)
            if key_text is None:
                records.append({
                    "source_name": layer.source,
                    "record_kind": GeneratorSettingsRecord.UNPARSED_COMMAND,
                    "normalized_key": None,
                    "normalized_value": None,
                    "original_setting": raw_setting,
                    "dedup_signature": (
                        GeneratorSettingsRecord.UNPARSED_COMMAND,
                        _normalize_text(raw_setting),
                    ),
                })
                continue

            normalized_key = _normalize_text(key_text)
            records.append({
                "source_name": layer.source,
                "record_kind": GeneratorSettingsRecord.PARSED_COMMAND,
                "normalized_key": normalized_key,
                "normalized_value": _normalize_text(value_text),
                "original_setting": raw_setting,
                "dedup_signature": (
                    GeneratorSettingsRecord.PARSED_COMMAND,
                    normalized_key,
                ),
            })
    return records


def _parse_assignment(setting_text, separators):
    text = str(setting_text).strip()
    for separator in separators:
        if separator in text:
            key_text, value_text = text.split(separator, 1)
            return key_text.strip(), value_text.strip()
    return None, None


def _normalize_text(value):
    text = str(value).strip()
    return " ".join(text.split())


def _deduplicate_records(records, keep):
    """Keep the first or last record for each normalized setting key."""
    keep_last = keep == GeneratorSettingsKeep.LAST
    records_to_scan = reversed(records) if keep_last else records

    kept_records = []
    removed_records = []
    first_seen_by_signature = {}
    for record in records_to_scan:
        signature = record["dedup_signature"]
        if signature in first_seen_by_signature:
            kept_record = first_seen_by_signature[signature]
            removed_record = dict(record)
            removed_record["duplicate_of_source_name"] = kept_record["source_name"]
            removed_record["kept_normalized_value"] = kept_record["normalized_value"]
            removed_record["kept_original_setting"] = kept_record["original_setting"]
            removed_records.append(removed_record)
            continue

        first_seen_by_signature[signature] = record
        kept_records.append(record)

    if keep_last:
        kept_records.reverse()
        removed_records.reverse()
    return kept_records, removed_records


def _build_report(records, kept_records, removed_records):
    """Collect duplicate and conflict information for logging/tests."""
    removed_duplicates = [
        {
            "source": record["source_name"],
            "duplicate_of_source": record.get("duplicate_of_source_name"),
            "setting": record["original_setting"],
            "kept_setting": record.get("kept_original_setting"),
            "normalized_value": record["normalized_value"],
            "kept_normalized_value": record.get("kept_normalized_value"),
        }
        for record in removed_records
    ]

    removed_identical = [
        {
            "source": record["source"],
            "duplicate_of_source": record.get("duplicate_of_source"),
            "setting": record["setting"],
            "kept_setting": record.get("kept_setting"),
        }
        for record in removed_duplicates
        if record["normalized_value"] == record.get("kept_normalized_value")
    ]
    duplicates_in_source = {}
    duplicates_across_sources = {}

    for record in removed_identical:
        source_name = record.get("source", "<unknown>")
        duplicate_of_source = record.get("duplicate_of_source", "<unknown>")
        if duplicate_of_source == source_name:
            duplicates_in_source[source_name] = (
                duplicates_in_source.get(source_name, 0) + 1
            )
            continue

        source_pair = (source_name, duplicate_of_source)
        duplicates_across_sources[source_pair] = (
            duplicates_across_sources.get(source_pair, 0) + 1
        )

    conflict_details = _build_conflict_details(records, kept_records)

    return {
        "removed_duplicates": removed_duplicates,
        "removed_identical": removed_identical,
        "conflicting_reassignments": [
            record
            for record in removed_duplicates
            if record["normalized_value"] != record.get("kept_normalized_value")
        ],
        "removed_overridden": [
            record
            for record in removed_duplicates
            if record["normalized_value"] != record.get("kept_normalized_value")
        ],
        "conflict_details": conflict_details,
        "conflicts": _find_conflicts(records),
        "duplicates_in_source": duplicates_in_source,
        "duplicates_across_sources": [
            {
                "source": source_name,
                "duplicate_of_source": duplicate_of_source,
                "count": duplicate_count,
            }
            for (source_name, duplicate_of_source), duplicate_count
            in sorted(duplicates_across_sources.items())
        ],
    }


def _build_conflict_details(records, kept_records):
    """
    Build one reporting entry per conflicting parsed key.

    The source and value order follows the original record order, and the kept
    value is marked explicitly in the value list.
    """
    kept_value_by_key = {}
    for record in kept_records:
        if record["record_kind"] != GeneratorSettingsRecord.PARSED_COMMAND:
            continue
        kept_value_by_key[record["normalized_key"]] = record["normalized_value"]

    values_by_key = {}
    for record in records:
        if record["record_kind"] != GeneratorSettingsRecord.PARSED_COMMAND:
            continue

        key = record["normalized_key"]
        source_name = record["source_name"]
        value = record["normalized_value"]

        key_entry = values_by_key.setdefault(
            key,
            {
                "sources": [],
                "source_set": set(),
                "values": [],
                "value_set": set(),
                "source_to_values": {},
            },
        )
        if source_name not in key_entry["source_set"]:
            key_entry["source_set"].add(source_name)
            key_entry["sources"].append(source_name)
        if value not in key_entry["value_set"]:
            key_entry["value_set"].add(value)
            key_entry["values"].append(value)
        key_entry["source_to_values"].setdefault(source_name, set()).add(value)

    conflict_details = []
    for key, key_entry in values_by_key.items():
        merged_values = set()
        for values in key_entry["source_to_values"].values():
            merged_values.update(values)
        if len(merged_values) <= 1 or len(key_entry["source_to_values"]) <= 1:
            continue

        kept_value = kept_value_by_key.get(key)
        marked_values = []
        for value in key_entry["values"]:
            if value == kept_value:
                marked_values.append(f"{value} (kept)")
                continue
            marked_values.append(value)

        conflict_details.append({
            "key": key,
            "sources": key_entry["sources"],
            "values": marked_values,
        })
    return conflict_details


def _find_conflicts(records):
    """Find keys assigned to multiple values within or across sources."""
    values_by_key_and_source = {}
    for record in records:
        if record["record_kind"] != GeneratorSettingsRecord.PARSED_COMMAND:
            continue

        key = record["normalized_key"]
        source_name = record["source_name"]
        value = record["normalized_value"]
        values_by_key_and_source.setdefault(key, {}).setdefault(
            source_name,
            set(),
        ).add(value)

    conflicts = []
    for key, source_to_values in values_by_key_and_source.items():
        for source_name, values in source_to_values.items():
            if len(values) > 1:
                conflicts.append({
                    "type": "intra_source_conflict",
                    "key": key,
                    "source": source_name,
                    "values": sorted(values),
                })

        merged_values = set()
        for values in source_to_values.values():
            merged_values.update(values)
        if len(merged_values) > 1 and len(source_to_values) > 1:
            conflicts.append({
                "type": "inter_source_conflict",
                "key": key,
                "sources": sorted(source_to_values.keys()),
                "values": sorted(merged_values),
            })
    return conflicts


def _log_report(context, report):
    """Print warnings from the structured report."""
    issue_prefix = "Potential issue with generator settings"

    duplicates = report.get("removed_identical", [])
    conflict_details = report.get("conflict_details", [])
    if not duplicates and not conflict_details:
        return

    genSettingsLog.warning(
        f"{issue_prefix} [{context}]: found {len(duplicates)} duplicate "
        f"setting(s) across sources and {len(conflict_details)} conflicting "
        f"setting key(s)"
    )

    for entry in sorted(
        duplicates,
        key=lambda record: (
            record.get("source", ""),
            record.get("duplicate_of_source", ""),
            record.get("setting", ""),
        ),
    ):
        source_name = entry.get("source", "<unknown>")
        duplicate_of_source = entry.get("duplicate_of_source", "<unknown>")
        setting = entry.get("setting", "<unknown>")
        source_list = [source_name]
        if duplicate_of_source != source_name:
            source_list.append(duplicate_of_source)
        genSettingsLog.warning(
            f"{issue_prefix} [{context}]: duplicate setting from sources "
            f"[{', '.join(source_list)}]: {setting}"
        )

    for entry in conflict_details:
        key = entry.get("key", "<unknown>")
        sources = ", ".join(entry.get("sources", []))
        values = ", ".join(entry.get("values", []))
        genSettingsLog.warning(
            f"{issue_prefix} [{context}]: conflicting setting '{key}' across "
            f"sources [{sources}] -> [{values}]"
        )
