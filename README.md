# dune3d-addons

Private shared-library add-ons for Dune 3D.

This repo now targets the in-process private add-on runtime used by the current Dune 3D build. It no longer describes the old script/declarative addon manager model.

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
  mac-quit-cmd-q/
    addon.json
    addon.cpp
  constraint-context-delete/
    addon.json
    addon.cpp
```

`addons/index.json` is just a lightweight catalog of ids and folders.

## Manifest format

Example:

```json
{
  "id": "hello-tools",
  "name": "Hello Tools",
  "version": "1.0.0",
  "dune_version": "1.4.0",
  "entry": "hello_tools.dylib",
  "enabled": true
}
```

Expected fields:

- `id`
- `name`
- `version`
- `dune_version`
- `entry`
- `enabled`

The old fields are obsolete and should not be used for new add-ons:

- `entrypoint`
- `app_shortcuts`
- `window_controls`
- `commands`
- `api_version`
- `category`

## Current converted add-ons

### `dev.justin.native-mac-window-buttons`

Registers macOS-style titlebar controls through the app-owned add-on UI hook.

### `dev.justin.mac-quit-cmd-q`

Registers `app.quit` accelerators through the app-owned add-on shortcut hook.

### `dev.justin.constraint-context-delete`

Now loads as a shared-library add-on and exposes a status command. It does not yet inject constraint-list context-menu actions.

## Building an add-on

Add-ons compile against Dune 3D internal headers. A minimal macOS example:

```sh
/opt/homebrew/opt/llvm/bin/clang++ \
  -std=c++20 \
  -dynamiclib \
  -I/Users/justin/dune3d/src \
  -I/Users/justin/dune3d/3rd_party \
  addon.cpp \
  -o hello_tools.dylib \
  $(pkg-config --cflags --libs gtkmm-4.0)
```

Then copy the built library next to `addon.json` in `~/.config/dune3d/addons/<folder>/`.

## Notes

- Exact Dune version matching is expected.
- Add-ons may be rebuilt whenever Dune 3D changes.
- These add-ons are private and trusted, not sandboxed public plugins.
