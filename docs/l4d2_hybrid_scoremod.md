# `l4d2_hybrid_scoremod`

## Overview

`l4d2_hybrid_scoremod` is a unified Left 4 Dead 2 scoremod that concentrates three scoring models in a single plugin:

- `legacy`
- `hybrid`
- `zone`

The active model is controlled by the `smplus_mode` convar.

In addition to internal bonus calculation, the plugin exposes a public API for:

- reading the active mode
- reading full `KeyValues` snapshots
- applying external bonuses at team or survivor scope
- receiving forwards when score state changes

The plugin is intended to be consumed by other plugins, especially HUDs, without depending on rigid component-specific natives.

## Score Models

The main convar is:

- `smplus_mode`

Values:

- `0`: `legacy`
- `1`: `hybrid`
- `2`: `zone`

### `legacy`

Reproduces a classic single-value `health bonus` model.

Characteristics:

- uses `smplus_legacy_*` convars
- calculates a final round bonus
- does not conceptually break score into `health / damage / pills`
- in the public API, legacy bonus is mapped as:
  - `health = legacy total`
  - `damage = 0`
  - `pills = 0`
  - `total = legacy total`

### `hybrid`

Uses a three-part model:

- `health bonus`
- `damage bonus`
- `pills bonus`

Characteristics:

- uses non-legacy `smplus_*` convars
- calculates both current bonus and map maximum bonus
- exposes a full component breakdown in snapshots

### `zone`

Uses the same base model as `hybrid`, but adds extra penalties associated with the `zone` ruleset.

Characteristics:

- shares almost all logic with `hybrid`
- applies additional penalties for states such as incap/death according to the current internal implementation
- is enabled with `smplus_mode 2`

## Game Mode Scope

The plugin understands the lifecycle of multiple game modes, but its internal score calculation is designed for `Versus`.

Current behavior:

- in `Versus`, the plugin calculates internal score according to the configured model
- in other modes, the internal base score remains `0`
- even outside `Versus`, the external bonus API remains available

This allows other plugins to use:

- round lifecycle
- snapshots
- external bonuses

without requiring the internal score calculation to be active.

## Bonus Components

In `hybrid` and `zone`, team score is composed of:

- `health bonus`
- `damage bonus`
- `pills bonus`
- `total bonus`

The plugin also maintains:

- `bonus_max`
- `bonus_external`
- `bonus_effective`

Definitions:

- `bonus`: base score calculated by the plugin
- `bonus_external`: score injected by other plugins
- `bonus_effective`: `bonus + bonus_external`

## External Bonuses

The plugin allows external bonus injection through its public API.

External bonus can be applied:

- at team scope
- per individual survivor

Rule:

- `client = 0`: team-level external bonus
- `client > 0`: survivor-level external bonus

External total bonus is not directly writable as an independent category. It is derived from:

- `Health`
- `Damage`
- `Pills`

This avoids inconsistencies between the total and its component breakdown.

## Public API

The public include is:

- `addons/sourcemod/scripting/include/l4d2_hybrid_scoremod.inc`

### Enums

#### `SMPlusMode`

- `SMPlusMode_Legacy`
- `SMPlusMode_Hybrid`
- `SMPlusMode_Zone`

#### `SMPlusBonusType`

- `SMPlusBonusType_Total`
- `SMPlusBonusType_Health`
- `SMPlusBonusType_Damage`
- `SMPlusBonusType_Pills`

`SMPlusBonusType_Total` is read-only for writes. The total is derived from the other categories.

### Natives

#### `SMPlusMode SMPlus_GetMode()`

Returns the active score model.

#### `void SMPlus_AddExternalBonus(SMPlusBonusType type, float value, int client = 0)`

Adds an external bonus delta.

#### `void SMPlus_SetExternalBonus(SMPlusBonusType type, float value, int client = 0)`

Overwrites an external bonus value.

#### `float SMPlus_GetExternalBonus(SMPlusBonusType type, int client = 0)`

Reads the current external bonus value.

#### `void SMPlus_ResetExternalBonus(int client = 0)`

Resets external bonus state.

#### `void SMPlus_FillSnapshot(KeyValues kv)`

Fills a complete snapshot of the current score state.

#### `void SMPlus_FillClientSnapshot(int client, KeyValues kv)`

Fills a score snapshot for a specific survivor.

### Forwards

#### `SMPlus_OnScoreUpdated()`

Fired when score-relevant state changes.

#### `SMPlus_OnRoundFinalized(int round)`

Fired when a round bonus is finalized.

#### `SMPlus_OnMatchFinalized(int winningTeam)`

Fired when the match is finalized.

## Snapshot Structure

`SMPlus_FillSnapshot` writes a `KeyValues` tree intended for plugin consumption. A representative example is:

```text
"scoremod_snapshot"
{
    "mode"                  "1"
    "score_model"           "hybrid"
    "current_round"         "1"
    "round_finalized"       "0"
    "match_finalized"       "0"
    "team_size"             "4"
    "map_distance"          "400"
    "alive_survivors"       "4"
    "upright_survivors"     "4"
    "pill_worth"            "30"
    "adrenaline_worth"      "0"

    "bonus"
    {
        "health"            "400"
        "damage"            "400"
        "pills"             "80"
        "total"             "880"
    }

    "bonus_max"
    {
        "health"            "400"
        "damage"            "400"
        "pills"             "80"
        "total"             "880"
    }

    "bonus_external"
    {
        "health"            "0"
        "damage"            "0"
        "pills"             "0"
        "total"             "0"
    }

    "bonus_effective"
    {
        "health"            "400"
        "damage"            "400"
        "pills"             "80"
        "total"             "880"
    }

    "rounds"
    {
        "round1_bonus"      "880"
        "round2_bonus"      "0"
        "round1_state"      "4/4"
        "round2_state"      "0/0"
        "round1_si_damage"  "0"
        "round2_si_damage"  "0"
    }

    "hybrid"
    {
        "bonus_per_survivor_multiplier" "0.5"
        "permanent_health_proportion"   "0.75"
        "pills_hp_factor"               "6.0"
        "pills_max_bonus"               "30"
        "zone_penalties_enabled"        "0"
    }

    "clients"
    {
        "2"
        {
            "client"                    "2"
            "userid"                    "2"
            "alive"                     "1"
            "incapped"                  "0"
            "ledged"                    "0"
            "has_pills"                 "1"
            "has_adrenaline"            "0"
            "permanent_health"          "100"
            "temporary_health"          "0"
            "revive_count"              "0"
            "health_bonus"              "100"
            "damage_bonus"              "100"
            "pills_bonus"               "20"
            "total_bonus"               "220"
            "external_health_bonus"     "0"
            "external_damage_bonus"     "0"
            "external_pills_bonus"      "0"
            "external_total_bonus"      "0"
            "effective_health_bonus"    "100"
            "effective_damage_bonus"    "100"
            "effective_pills_bonus"     "20"
            "effective_total_bonus"     "220"
        }
    }
}
```

Notes about the structure:

- `bonus` contains the plugin-calculated base score
- `bonus_max` contains the theoretical maximum for the same breakdown
- `bonus_external` contains score injected by other plugins
- `bonus_effective` contains the final visible value, that is `base + external`
- `rounds` stores half-by-half state
- `legacy` or `hybrid` appears depending on the active model
- `clients` contains per-survivor snapshots indexed by `userid`

## Commands

The plugin registers these commands:

- `sm_bonus`
- `sm_health`
- `sm_damage`
- `sm_mapinfo`

### `sm_bonus`

Shows the current bonus state.

Supported variants:

- `!bonus`
- `!bonus lite`
- `!bonus full`

Behavior:

- in `legacy`, prints the format associated with that model
- in `hybrid/zone`, prints the component breakdown
- if external bonus exists, it is included in the total and a `Custom` block is appended

### `sm_health`

Alias of the same bonus handler.

### `sm_damage`

Alias of the same bonus handler.

### `sm_mapinfo`

Shows structural map score information:

- team size
- distance
- maximum total bonus
- maximum component breakdown
- tiebreaker

It does not add bonus. It is informational only.

## Team Bonus Print Behavior

The convar:

- `smplus_bonus_team_print`

controls how `!bonus` is printed.

Values:

- `0`: private reply to the calling player
- `1`: if executed by a human on `Survivor` or `Infected`, the output is replicated to that player's team

It does not replicate to spectators.

## ConVars

### General

- `smplus_debug`
  - `0/1`
  - enables debug output

- `smplus_mode`
  - `0..2`
  - defines the active score model

- `smplus_bonus_team_print`
  - `0/1`
  - replicates `!bonus` to the caller's team

### Hybrid / Zone

- `smplus_bonus_per_survivor_multiplier`
  - base per-survivor bonus multiplier

- `smplus_permanent_health_proportion`
  - share of map bonus allocated to permanent health

- `smplus_pills_hp_factor`
  - relative HP value used for pills scoring

- `smplus_pills_max_bonus`
  - ceiling for unused pills bonus

### Legacy

- `smplus_legacy_enable`
  - `0/1`
  - enables legacy logic

- `smplus_legacy_health_bonus_ratio`
  - legacy health bonus multiplier

- `smplus_legacy_survival_bonus_ratio`
  - legacy static survival bonus ratio

- `smplus_legacy_temp_multi_incap_0`
  - temp health multiplier with `0` incaps

- `smplus_legacy_temp_multi_incap_1`
  - temp health multiplier with `1` incap

- `smplus_legacy_temp_multi_incap_2`
  - temp health multiplier with `2` incaps

- `smplus_legacy_first_aid_heal_percent`
  - legacy heal percent for first aid

- `smplus_legacy_pain_pills_health_value`
  - legacy pills buffer value

- `smplus_legacy_adrenaline_health_buffer`
  - legacy adrenaline buffer value

- `smplus_legacy_map_multi`
  - `0/1`
  - determines whether legacy maximum scales with map distance

- `smplus_legacy_custom_max_distance`
  - custom legacy maximum distance, if used

## Internal Lifecycle

The plugin maintains lifecycle context by game mode:

- round start signals
- round end signals
- live signal

This context is used to:

- know when to reset state
- fire forwards
- support future extensibility in other modes

Currently, even though the lifecycle context exists for multiple modes, internal score calculation remains focused on `Versus`.

## Code Organization

The plugin is divided into modules under:

- `addons/sourcemod/scripting/l4d2_hybrid_scoremod`

Main files:

- `shared.sp`
  - general helpers, mode handling, lifecycle, and shared utilities

- `events.sp`
  - event hooks and round/player flow

- `commands.sp`
  - `sm_bonus`, `sm_health`, `sm_damage`, `sm_mapinfo`

- `snapshot.sp`
  - snapshot construction and `KeyValues` serialization

- `external_bonus.sp`
  - external bonus state and forwards

- `legacy.sp`
  - legacy model logic

- `hybrid.sp`
  - hybrid / zone model logic

- `natives.sp`
  - public native implementations
