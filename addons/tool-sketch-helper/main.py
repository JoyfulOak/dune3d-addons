#!/usr/bin/env python3
import json
import os

command = os.environ.get("DUNE3D_ADDON_COMMAND", "")

response = {
    "title": "Tool Sketch Helper",
    "detail": "Pick a command.",
    "actions": []
}

if command == "show-sketch-tips":
    response["title"] = "Sketch Tips"
    response["detail"] = (
        "Start with a workplane, draw the base profile first, then add constraints before moving on to solid-model groups."
    )
    response["actions"] = [{
        "type": "show_message",
        "title": response["title"],
        "detail": response["detail"]
    }]
elif command == "show-constraint-tips":
    response["title"] = "Constraint Tips"
    response["detail"] = (
        "Apply a few high-value constraints early: coincident, horizontal/vertical, dimensions, then check underconstrained points."
    )
    response["actions"] = [{
        "type": "show_message",
        "title": response["title"],
        "detail": response["detail"]
    }]

print(json.dumps(response))
