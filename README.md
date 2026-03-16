# dune3d-addons

Downloadable addons for Dune 3D.

This repo is an addon catalog for the Dune 3D Addons manager. Addons live here, Dune 3D syncs the repo into its managed cache, and users install addons from the Addons window inside the app.

## How Dune 3D uses this repo

Dune 3D expects this layout:

- `addons/index.json`
- `addons/<folder>/addon.json`
- `addons/<folder>/...`

The app reads `addons/index.json` as the preferred catalog. If `index.json` is missing, it falls back to scanning addon folders for `addon.json` files.

Normal user flow:

1. Open `Addons` in Dune 3D.
2. The app syncs this repo into its managed local checkout.
3. The app reads the addon catalog from `addons/index.json`.
4. The user clicks `Install` for the addon they want.
5. The addon is copied into Dune 3D's local addon data directory.
6. The user enables the addon and tests it.

Repo addons are not automatically active just because they exist here. They must still be installed from the Addons manager.

## Current Addons manager behavior

- the Addons window is filtered by `UI`, `Tools`, and `Models` tabs
- repo addons show `Install` when they are not installed yet
- installed addons show `Update` only when a newer repo version is available
- installed addons with declared `commands` show one button per command
- standalone script addons do not get a generic `Run` button in the installed-addon list

That means script addons should expose useful named commands instead of relying on a fallback run control.

## Repo structure

Example:

```text
addons/
  index.json
  native-mac-window-buttons/
    addon.json
  mac-quit-cmd-q/
    addon.json
  tool-sketch-helper/
    addon.json
    main.py
  model-demo-block/
    addon.json
    main.py
    demo-block.step
```

## Catalog file

`addons/index.json` lists addon ids and folder names.

Example:

```json
{
  "addons": [
    {
      "id": "dev.justin.native-mac-window-buttons",
      "folder": "native-mac-window-buttons"
    },
    {
      "id": "dev.justin.mac-quit-cmd-q",
      "folder": "mac-quit-cmd-q"
    },
    {
      "id": "dev.justin.tool-sketch-helper",
      "folder": "tool-sketch-helper"
    },
    {
      "id": "dev.justin.model-demo-block",
      "folder": "model-demo-block"
    }
  ]
}
```

Recommendations:

- Keep `id` stable once published.
- Keep `folder` stable once users install the addon.
- Add new addons to the catalog instead of relying on folder scanning.
- Use reverse-DNS ids such as `dev.yourname.my-addon`.

## Addon manifest

Every addon needs an `addon.json` manifest.

Common fields:

- `id`: globally unique addon id
- `name`: user-visible name
- `category`: addon tab/category shown in the Addons manager
- `version`: addon version string
- `author`: addon author
- `description`: short summary shown in the Addons window
- `api_version`: addon API version, currently `1`
- `entrypoint`: script file to run for script addons
- `commands`: addon commands shown in the Addons window
- `window_controls`: declarative titlebar behavior
- `app_shortcuts`: declarative app accelerators like `Cmd+Q`

Current supported categories:

- `UI`
- `Tools`
- `Models`

### Minimal declarative addon

```json
{
  "id": "dev.example.sample-addon",
  "name": "Sample Addon",
  "category": "UI",
  "version": "0.1.0",
  "author": "Example Author",
  "description": "A minimal addon manifest."
}
```

This installs cleanly but does nothing at runtime unless it declares a supported capability.

## Supported addon types

### 1. Declarative window-controls addon

Use this to change titlebar/button layout through supported app settings instead of direct GTK access.

```json
{
  "id": "dev.example.native-mac-window-buttons",
  "name": "Native Mac Window Buttons",
  "category": "UI",
  "version": "0.1.0",
  "author": "Example Author",
  "description": "Uses macOS-style traffic-light titlebar buttons.",
  "window_controls": {
    "use_native_controls": true,
    "decoration_layout": "close,minimize,maximize:"
  }
}
```

### 2. Declarative app-shortcut addon

Use this to register app shortcuts for built-in actions.

```json
{
  "id": "dev.example.mac-quit-cmd-q",
  "name": "Mac Quit CMD+Q",
  "category": "UI",
  "version": "0.1.0",
  "author": "Example Author",
  "description": "Adds a quit shortcut for macOS.",
  "app_shortcuts": [
    {
      "action": "app.quit",
      "accel": "<Meta>q"
    },
    {
      "action": "app.quit",
      "accel": "<Primary>q"
    },
    {
      "action": "app.quit",
      "accel": "<Super>q"
    }
  ]
}
```

Notes:

- Dune 3D expands common macOS modifier variants when registering accelerators.
- Multiple entries can target the same action.

### 3. Script addon

Use this when the addon needs runtime logic.

Script addons declare an `entrypoint` and usually one or more `commands`.

Example manifest:

```json
{
  "id": "dev.example.tool-sketch-helper",
  "api_version": 1,
  "category": "Tools",
  "name": "Tool Sketch Helper",
  "version": "0.2.0",
  "author": "Example Author",
  "description": "Example tools addon with live document actions.",
  "entrypoint": "main.py",
  "commands": [
    {
      "id": "add-axis-guide",
      "label": "Add Axis Guide",
      "description": "Adds three 3D guide lines to the current group.",
      "menu_label": "Add Axis Guide"
    },
    {
      "id": "show-sketch-tips",
      "label": "Sketch Tips",
      "description": "Shows a sketch workflow reminder.",
      "menu_label": "Sketch Tips"
    }
  ]
}
```

Example `main.py`:

```python
#!/usr/bin/env python3
import json
import os

command = os.environ.get("DUNE3D_ADDON_COMMAND", "")
response = {
    "title": "Tool Sketch Helper",
    "detail": "Pick a command.",
    "actions": []
}

if command == "add-axis-guide":
    response["title"] = "Axis Guide Added"
    response["detail"] = "Added X, Y, and Z guide lines to the current group."
    response["actions"] = [
        {"type": "add_line3d", "label": "addon axis x", "point1": [0, 0, 0], "point2": [40, 0, 0]},
        {"type": "add_line3d", "label": "addon axis y", "point1": [0, 0, 0], "point2": [0, 40, 0]},
        {"type": "add_line3d", "label": "addon axis z", "point1": [0, 0, 0], "point2": [0, 0, 40]}
    ]
elif command == "show-sketch-tips":
    response["title"] = "Sketch Tips"
    response["detail"] = "Start with a workplane, lock down the base profile, then switch to solid-model groups once the sketch is constrained."
    response["actions"] = [{
        "type": "show_message",
        "title": response["title"],
        "detail": response["detail"]
    }]

print(json.dumps(response))
```

## Script addon environment

When Dune 3D runs a script addon, it provides:

- `DUNE3D_ADDON_ID`
- `DUNE3D_ADDON_NAME`
- `DUNE3D_ADDON_DIR`
- `DUNE3D_CONFIG_DIR`
- `DUNE3D_ADDON_API_VERSION`
- `DUNE3D_ADDON_COMMAND`
- `DUNE3D_ADDON_STATE_JSON`
- `DUNE3D_ADDON_MANIFEST_PATH`

## Structured script output

Script addons can print JSON to stdout. Dune 3D reads that JSON and can update addon state and trigger supported app actions.

Supported response fields:

- `title`
- `detail`
- `state`
- `actions`

Supported action types:

- `show_message`
- `open_window`
- `add_line3d`
- `insert_step_model`

### `add_line3d`

Fields:

- `point1`
- `point2`
- `label`

This creates one 3D line entity in the active document group.

### `insert_step_model`

Fields:

- `path`
- `copy_to_project`
- `lock_rotation`

Behavior:

- relative `path` values are resolved from the addon folder
- if `copy_to_project` is `true` and the document already has a saved path, Dune 3D copies the STEP file into a project addon-assets folder before inserting it
- if `lock_rotation` is `true`, the inserted STEP entity gets a lock-rotation constraint

Example response:

```json
{
  "title": "Demo Block Inserted",
  "detail": "Inserted the packaged STEP block into the current document.",
  "actions": [
    {
      "type": "insert_step_model",
      "path": "demo-block.step",
      "copy_to_project": true,
      "lock_rotation": true
    }
  ]
}
```

## How to create a new addon

1. Pick one of the current categories: `UI`, `Tools`, or `Models`.
2. Create a folder under `addons/`.
3. Add an `addon.json` manifest.
4. If the addon is scripted, add `main.py` or another entrypoint file.
5. If the addon ships assets, place them in the same addon folder.
6. Add the addon to `addons/index.json`.
7. Open Dune 3D, open `Addons`, let it sync, and install the addon from the UI.
8. Enable the addon and test its commands.
9. Bump the addon `version` whenever you publish a new repo update.

## Authoring tips

- Prefer named commands over one generic script entrypoint.
- Keep addon ids and folder names stable.
- Use declarative fields when the feature already exists, such as `window_controls` or `app_shortcuts`.
- Use script actions when you need runtime behavior.
- Keep script output small and deterministic.
- Treat the addon API as a supported surface, not as direct access to internal dune3d C++ objects.

## Current example addons

- `native-mac-window-buttons`
- `mac-quit-cmd-q`
- `tool-sketch-helper`
- `model-demo-block`
