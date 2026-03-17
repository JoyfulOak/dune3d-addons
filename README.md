# dune3d-addons

Private shared-library add-ons for Dune 3D.

This repo targets the in-process private add-on runtime used by the current Dune 3D build.

## Runtime model

Dune 3D loads trusted add-ons from:

- `~/.config/dune3d/addons/<folder>/addon.json`
- `~/.config/dune3d/addons/<folder>/<shared-library>`

Each add-on is:

- owner-built
- version-locked to the app
- loaded in-process
- registered through a single C entrypoint

Required entrypoint:

```cpp
extern "C" bool dune_register_addon(dune3d::AddonContext& ctx);
```

## Repo layout

```text
addons/
  index.json
  native-mac-window-buttons/
    addon.json
    addon.cpp
    native_mac_window_buttons.dylib
  mac-quit-cmd-q/
    addon.json
    addon.cpp
    mac_quit_cmd_q.dylib
  constraint-context-delete/
    addon.json
    addon.cpp
    constraint_context_delete.dylib
build_addons.sh
```

## addon.json fields

Typical manifest:

```json
{
  "id": "dev.justin.example-addon",
  "name": "Example Add-on",
  "version": "0.1.0",
  "dune_version": "1.4.0",
  "author": "Justin",
  "description": "One-line summary shown in the Addons window.",
  "entry": "example_addon.dylib",
  "enabled": true
}
```

Field purpose:

- `id`: stable unique add-on id
- `name`: user-visible add-on name
- `version`: add-on version
- `dune_version`: exact Dune 3D app version the add-on targets
- `author`: optional author label
- `description`: one-line summary shown in Dune 3D's Addons window
- `entry`: compiled shared library filename that Dune 3D loads
- `enabled`: optional default enabled state

`dune_version` is important because the private add-on loader uses exact version matching. If Dune 3D is running as `1.4.0`, then the add-on manifest must also say `"dune_version": "1.4.0"` or the add-on will be rejected as incompatible. When the app version changes, rebuild the add-on and update `dune_version` to match.

## Building the repo add-ons

These add-ons compile against the local Dune 3D source tree.

```sh
cd /Users/justin/Documents/GitHub/dune3d-addons
export DUNE3D_ROOT="/Users/justin/dune3d"
export CXX="/opt/homebrew/opt/llvm/bin/clang++"
bash build_addons.sh
```

That command writes the `.dylib` files directly into each `addons/<folder>/` directory next to `addon.json`, which makes the repo installable by Dune 3D's Addons window.

## Creating a new add-on

1. Create a new folder under `addons/<your-addon>/`.
2. Add an `addon.json` manifest.
3. Add an `addon.cpp` file that exports `dune_register_addon`.
4. Build the add-on so the compiled `.dylib` sits next to `addon.json`.
5. Update `index.json` if you use it as a catalog listing.
6. Push the repo changes so Dune 3D can see the updated package online.

Minimal entrypoint:

```cpp
#include "addons/addon_context.hpp"

extern "C" bool dune_register_addon(dune3d::AddonContext &ctx)
{
    (void)ctx;
    return true;
}
```

## Notes

- Exact Dune version matching is expected.
- Rebuild the add-ons whenever Dune 3D headers or behavior change.
- This repo is a private trusted add-on catalog, not a public plugin marketplace.
