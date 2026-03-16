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
        {
            "type": "add_line3d",
            "label": "addon axis x",
            "point1": [0, 0, 0],
            "point2": [40, 0, 0]
        },
        {
            "type": "add_line3d",
            "label": "addon axis y",
            "point1": [0, 0, 0],
            "point2": [0, 40, 0]
        },
        {
            "type": "add_line3d",
            "label": "addon axis z",
            "point1": [0, 0, 0],
            "point2": [0, 0, 40]
        }
    ]
elif command == "show-sketch-tips":
    response["title"] = "Sketch Tips"
    response["detail"] = (
        "Start with a workplane, lock down the base profile, then switch to solid-model groups once the sketch is constrained."
    )
    response["actions"] = [{
        "type": "show_message",
        "title": response["title"],
        "detail": response["detail"]
    }]

print(json.dumps(response))
