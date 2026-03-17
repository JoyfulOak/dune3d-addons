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

## Building the repo add-ons

These add-ons compile against the local Dune 3D source tree.

```sh
cd /Users/justin/Documents/GitHub/dune3d-addons
export DUNE3D_ROOT="/Users/justin/dune3d"
export CXX="/opt/homebrew/opt/llvm/bin/clang++"
bash build_addons.sh
```

That command writes the `.dylib` files directly into each `addons/<folder>/` directory next to `addon.json`, which makes the repo installable by Dune 3D's Addons window.

## Notes

- Exact Dune version matching is expected.
- Rebuild the add-ons whenever Dune 3D headers or behavior change.
- This repo is a private trusted add-on catalog, not a public plugin marketplace.
