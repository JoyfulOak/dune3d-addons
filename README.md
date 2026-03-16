# dune3d-addons

Downloadable addons for Dune 3D.

This repo is meant to be used as an addon catalog for the Dune 3D Addons manager. Addons live in this repo, Dune 3D syncs the repo into its managed cache, and users install addons from the Addons window inside the app.

## How Dune 3D uses this repo

Dune 3D expects this repo layout:

- `addons/index.json`
- `addons/<folder>/addon.json`
- `addons/<folder>/...`

The app reads `addons/index.json` as the preferred addon catalog. If `index.json` is missing, it falls back to scanning addon folders for `addon.json` files.

The normal user flow is:

1. Open the `Addons` window in Dune 3D.
2. The app syncs the addon repo automatically when the window opens.
3. The app reads this repo's catalog.
4. The user clicks `Install` on the addon they want.
5. The addon is copied into Dune 3D's local addon data directory.
6. The user enables the addon and tests it.

Do not assume addon folders in this repo are automatically active in the app. They must still be installed from the Addons manager.

## Current Addons manager behavior

The current Dune 3D Addons manager behaves like this:

- the Addons window is filtered by `UI`, `Tools`, and `Models` tabs
- repo addons show `Install` when they are not installed yet
- installed addons show `Update` only when a newer repo version is available
- installed addons with declared `commands` show one button per command
- standalone script addons do not get a generic `Run` button in the installed-addon list

That means addon authors should not rely on a fallback `Run` button being present for script entrypoints with no commands.

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
  model-workflow-notes/
    addon.json
    main.py
```

## Catalog file

`addons/index.json` is a simple catalog of addon ids and folder names.

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
      "id": "dev.justin.model-workflow-notes",
      "folder": "model-workflow-notes"
    }
  ]
}
```

Recommendations:

- Keep `id` stable forever once published.
- Keep `folder` stable if users are already installing the addon.
- Add new addons to the catalog rather than relying on folder scanning.
- Use reverse-DNS style ids like `dev.yourname.my-addon`.

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
- `window_controls`: declarative window-titlebar behavior
- `app_shortcuts`: declarative app accelerators like `Cmd+Q`

Current supported categories are:

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

This kind of addon can exist in the catalog and be installed, but it does not do anything at runtime unless it declares a supported capability.

## Supported addon types

### 1. Declarative window-controls addon

Use this when you want to change the app's titlebar/button layout through supported app settings instead of direct GTK access.

Example:

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

Fields:

- `use_native_controls`: ask the app to prefer native titlebar controls
- `decoration_layout`: GTK decoration layout string

### 2. Declarative app-shortcut addon

Use this when you want an addon to register app shortcuts for built-in actions.

Example:

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

Fields:

- `action`: the app action name, for example `app.quit`
- `accel`: GTK accelerator string

Notes:

- Dune 3D also expands common macOS modifier variants when it registers accelerators.
- Multiple entries can target the same action.
- Use fallback variants when platform key mapping is inconsistent.

### 3. Script addon

Use this when the addon needs logic at runtime.

Script addons declare an `entrypoint` and optionally `commands`.

Example manifest:

```json
{
  "id": "dev.example.tool-sketch-helper",
  "api_version": 1,
  "category": "Tools",
  "name": "Tool Sketch Helper",
  "version": "0.1.0",
  "author": "Example Author",
  "description": "Example multi-command tools addon.",
  "entrypoint": "main.py",
  "commands": [
    {
      "id": "show-sketch-tips",
      "label": "Sketch Tips",
      "description": "Shows a sketch workflow reminder.",
      "menu_label": "Sketch Tips"
    },
    {
      "id": "show-constraint-tips",
      "label": "Constraint Tips",
      "description": "Shows a constraint workflow reminder.",
      "menu_label": "Constraint Tips"
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
state_json = os.environ.get("DUNE3D_ADDON_STATE_JSON", "")

try:
    state = json.loads(state_json) if state_json else {}
except json.JSONDecodeError:
    state = {}

response = {
    "title": "Example Addon",
    "detail": "No command was provided.",
    "actions": [],
    "state": state,
}

if command == "show-sketch-tips":
    count = int(state.get("count", 0)) + 1
    state["count"] = count
    response["title"] = "Sketch Tips"
    response["detail"] = f"Sketch the base profile first, then constrain it before moving on. Shown {count} times."
    response["state"] = state
    response["actions"] = [
        {
            "type": "show_message",
            "title": response["title"],
            "detail": response["detail"]
        }
    ]
elif command == "show-constraint-tips":
    response["title"] = "Constraint Tips"
    response["detail"] = "Apply coincident, horizontal/vertical, and dimensions early to stabilize the sketch."
    response["state"] = state
    response["actions"] = [
        {
            "type": "show_message",
            "title": response["title"],
            "detail": response["detail"]
        }
    ]

print(json.dumps(response))
```

## Script addon environment

When Dune 3D runs a script addon, it provides these environment variables:

- `DUNE3D_ADDON_ID`
- `DUNE3D_ADDON_NAME`
- `DUNE3D_ADDON_DIR`
- `DUNE3D_CONFIG_DIR`
- `DUNE3D_ADDON_API_VERSION`
- `DUNE3D_ADDON_COMMAND`
- `DUNE3D_ADDON_STATE_JSON`
- `DUNE3D_ADDON_MANIFEST_PATH`

## Structured script output

Script addons can print JSON to stdout. Dune 3D reads that JSON and can use it to update addon state and trigger supported app actions.

Supported response fields:

- `title`
- `detail`
- `state`
- `actions`

Supported action types currently include:

- `show_message`
- `open_window`

Example response:

```json
{
  "title": "Addon Demo",
  "detail": "Everything worked.",
  "state": {
    "count": 3
  },
  "actions": [
    {
      "type": "show_message",
      "title": "Addon Demo",
      "detail": "Everything worked."
    },
    {
      "type": "open_window",
      "window": "addons"
    }
  ]
}
```

## How to create a new addon

1. Pick a stable addon id.
2. Create a new folder in `addons/`.
3. Create `addon.json` in that folder.
4. Add the addon to `addons/index.json`.
5. If it is a script addon, add the script file such as `main.py`.
6. Keep the addon focused on supported extension points.
7. Open Dune 3D, open `Addons`, let it sync, and install your addon from the UI.
8. Enable the addon and test it.
9. Bump the `version` when you ship changes.

## Recommended authoring rules

- Keep manifests small and explicit.
- Pick one of the current categories: `UI`, `Tools`, or `Models`.
- Prefer declarative addons when possible.
- Use script addons for behavior, not for arbitrary internal patching.
- Do not rely on direct access to Dune 3D internal C++ objects.
- Treat the addon API as a stable extension surface, not a private-engine hook.
- Keep command ids stable after release.
- Use clear names and descriptions because they appear in the Addons window.
- Add fallback accelerators for shortcuts on macOS.

## Versioning

Recommendations:

- Start with `0.1.0` for a first test addon.
- Bump patch version for small fixes.
- Bump minor version when adding new commands or capabilities.
- Keep the same `id` across versions of the same addon.

Dune 3D compares installed versions against repo versions to decide whether an update is available.

## Testing workflow

Recommended workflow:

1. Edit files in this repo.
2. Open Dune 3D.
3. Open the `Addons` window.
4. Let the repo auto-sync.
5. Click `Install` for a new addon, or `Update` for an existing one.
6. Enable it.
7. Test the behavior.
8. If needed, update the version and repeat.

## Current example addons

This repo currently includes examples for:

- `native-mac-window-buttons`
- `mac-quit-cmd-q`
- `tool-sketch-helper`
- `model-workflow-notes`

These are useful references when creating new addons.
