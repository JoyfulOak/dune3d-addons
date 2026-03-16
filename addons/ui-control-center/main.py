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
    "title": "UI Control Center",
    "detail": "No command was provided.",
    "actions": [],
    "state": state,
}

if command == "toggle-native-buttons":
    enabled = not state.get("native_window_buttons", False)
    state["native_window_buttons"] = enabled
    state["window_controls"] = {
        "use_native_controls": enabled,
        "decoration_layout": "close,minimize,maximize:" if enabled else ""
    }
    response["title"] = "Mac Window Buttons"
    response["detail"] = (
        "Enabled native macOS left-side traffic lights."
        if enabled else
        "Restored the default Dune 3D header-bar button layout."
    )
    response["state"] = state
    response["actions"] = [
        {
            "type": "show_message",
            "title": response["title"],
            "detail": response["detail"]
        }
    ]
elif command == "show-demo-message":
    count = int(state.get("demo_message_count", 0)) + 1
    state["demo_message_count"] = count
    response["title"] = "Addon API Demo"
    response["detail"] = f"Structured addon actions are working. Demo command run count: {count}."
    response["state"] = state
    response["actions"] = [
        {
            "type": "show_message",
            "title": response["title"],
            "detail": response["detail"]
        },
        {
            "type": "open_window",
            "window": "addons"
        }
    ]

print(json.dumps(response))
