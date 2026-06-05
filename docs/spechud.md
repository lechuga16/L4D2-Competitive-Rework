# `spechud`

## Overview

`spechud` is a spectator-oriented HUD manager for Left 4 Dead 2. It renders two panel-based HUDs:

- `Spectator HUD`
- `Tank HUD`

The plugin is designed to provide a compact, continuously refreshed view of the current match state for:

- spectators
- SourceTV
- infected players using the tank HUD

The plugin does not own the game state it displays. Instead, it reads state from the engine, gamerules, and optional companion libraries, then formats that state into SourceMod panels.

## Main Commands

The plugin registers two commands:

- `sm_spechud`
- `sm_tankhud`

### `sm_spechud`

Toggles the `Spectator HUD`.

Behavior:

- only works for players on the `Spectator` team
- ignored for survivors and infected
- stores a per-client enabled/disabled flag

### `sm_tankhud`

Toggles the `Tank HUD`.

Behavior:

- works for spectators and infected players
- ignored for survivors
- stores a per-client enabled/disabled flag

## Draw Loop

The plugin updates its HUDs on a repeating timer:

- `SPECHUD_DRAW_INTERVAL = 0.5`

Every tick, it:

1. skips drawing while `readyup` or `pause` is active
2. classifies connected clients by role
3. builds the spectator HUD for all eligible spectators and SourceTV
4. builds the tank HUD for all eligible clients when a tank exists

The panel contents are rebuilt on each draw pass from cached snapshots rather than from persistent rendered strings.

## HUD Types

## Spectator HUD

The spectator HUD is a multi-section panel. Depending on the active game mode and available libraries, it can contain:

1. server header
2. survivor block
3. score block
4. infected block
5. game status or tank status block

### Header

The header shows:

- server name
- connected human player count
- maximum player slots
- tickrate

### Survivor Block

The survivor block shows one line per survivor, sorted by character identity.

It includes:

- player name
- current visible health state
- incap state
- hanging state
- active weapon / primary weapon / secondary shorthand

Examples of the survivor line format:

- alive: `Nick: 100HP [Mac 50/650 | P]`
- hanging: `Nick: <40HP@Hanging>`
- incapacitated: `Nick: <300HP@1st> [Pistol 15]`

Weapon display is built from a cached `WeaponSnapshot` and accounts for:

- primary weapon
- reserve ammo
- active secondary
- dual pistols
- melee / chainsaw special handling

### Score Block

The score block is shown in `Versus` when `l4d2_hybrid_scoremod` is available.

It reads a `KeyValues` snapshot from `SMPlus_FillSnapshot()` and formats the current score according to the active `SMPlusMode`.

Current behavior:

- `hybrid` / `zone`
  - shows `HB`, `DB`, and `Pills`
  - shows total bonus and total percent
  - shows map distance
- `legacy`
  - shows only `HB`
  - shows total bonus and total percent
  - shows map distance

The score block is display-only. `spechud` does not calculate score itself.

### Infected Block

The infected block shows one line per infected player except tanks.

It includes:

- player name
- zombie class name
- health state
- ghost state
- on-fire state
- ability cooldown, when relevant
- respawn countdown, when dead and waiting to spawn

If no special infected line is available, the panel prints the localized `No SI` text.

### Game Status / Tank Status Block

The last section depends on whether a live tank exists.

If a tank exists:

- the spectator HUD shows tank information instead of the generic game status section

If no tank exists:

- the plugin shows a game-status block for the active mode

In `Versus`, this can include:

- ready config name
- current round number
- tank flow
- witch flow
- current survivor progression
- tank selection, when the native exists

In `Scavenge`, it shows:

- ready config name
- round number
- best-of limit

## Tank HUD

The tank HUD is a dedicated panel rendered when a live tank exists.

It shows:

- title with current ready config name
- tank controller
- pass count
- current tank health
- frustration
- latency
- lerp, if `lerpmonitor` is available
- remaining burn time, when the tank is on fire

The tank HUD is shown to:

- spectators who enabled tank HUD
- infected players who enabled tank HUD
- the tank player, if enabled

## Supported Game Modes

The plugin has explicit handling for:

- `Versus`
- `Scavenge`

`g_iGamemode` is initialized from `L4D_GetGameModeType()` and drives the panel layout.

Mode-specific behavior includes:

- score block only in `Versus`
- scavenge timer and round information in `Scavenge`
- versus campaign score / progress distance in `Versus`

## Optional Libraries

`spechud` supports several optional libraries and tracks them through runtime flags.

Optional dependencies:

- `readyup`
- `pause`
- `l4d_boss_percent`
- `l4d2_hybrid_scoremod`
- `l4d_tank_control_eq`
- `lerpmonitor`
- `witch_and_tankifier`

It also checks for the native:

- `GetTankSelection`

These features are not all required for the plugin to load. Instead, `spechud` enables or disables blocks dynamically based on runtime availability.

## Runtime State

The plugin centralizes feature flags in `RuntimeState`.

Tracked runtime flags:

- `lateload`
- `readyUp`
- `pause`
- `l4dBossPercent`
- `hybridScoremod`
- `tankControlEq`
- `lerpMonitor`
- `witchAndTankifier`
- `tankSelection`

`RuntimeState.Refresh()` rebuilds the availability snapshot from loaded libraries and native presence.

## Cached State and Snapshots

The plugin uses typed snapshots to keep rendering code structured:

- `BossFlowState`
- `BossRoundState`
- `TankHudSnapshot`
- `WeaponSnapshot`
- `SurvivorSnapshot`
- `InfectedSnapshot`

Their purpose is:

- isolate engine reads from string formatting
- reduce repeated logic in panel builders
- keep rendering functions focused on presentation

## Boss Flow Handling

In `Versus`, the plugin caches boss-related state such as:

- current tank percent
- current witch percent
- whether the round has flow tank
- whether the round has flow witch
- number of remaining tanks
- number of remaining witches
- whether a custom boss system is active

The plugin uses:

- `l4d_boss_percent` when available
- map-specific exception tables
- finale scheme maps
- `witch_and_tankifier` helpers for static tank / witch maps

This state drives the third section of the spectator HUD when no live tank is active.

## ReadyUp and Pause Interaction

The plugin does not draw HUDs while:

- `readyup` reports the match is still in ready state
- `pause` reports the game is paused

This is enforced at the start of the draw timer and avoids presenting live-match information while the round is not actively being played.

## Translations

The plugin requires:

- `translations/spechud.phrases.txt`

It loads translations at startup and fails to load if the main translation file is missing.

The panel content is translated through `%T` lookups, which means the visible language depends on the target client receiving the panel.

## Plugin-Owned ConVars

`spechud` does not currently define its own plugin-specific convars.

Instead, it reads existing engine or companion-plugin convars such as:

- `survivor_limit`
- `versus_boss_buffer`
- `sv_maxplayers`
- `tank_burn_duration`

It caches these values and refreshes them through change hooks.

## Internal Organization

The plugin entry file is:

- `addons/sourcemod/scripting/spechud.sp`

It is split into four modules:

- `types.sp`
  - shared state, enums, structs, cached handles, runtime flags

- `helpers.sp`
  - reusable helper utilities, name normalization, percentages, flow helpers, sorting

- `runtime.sp`
  - event hooks, round-state resets, command handlers, lifecycle updates

- `render.sp`
  - snapshot builders, line formatting, panel rendering, HUD composition
