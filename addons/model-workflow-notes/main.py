#!/usr/bin/env python3
import json
import os

command = os.environ.get("DUNE3D_ADDON_COMMAND", "")

response = {
    "title": "Model Workflow Notes",
    "detail": "Pick a command.",
    "actions": []
}

if command == "show-extrude-checklist":
    response["title"] = "Extrude Checklist"
    response["detail"] = (
        "Confirm the sketch is constrained, the profile is closed, and the target body/group is correct before extruding."
    )
    response["actions"] = [{
        "type": "show_message",
        "title": response["title"],
        "detail": response["detail"]
    }]
elif command == "show-export-checklist":
    response["title"] = "Export Checklist"
    response["detail"] = (
        "Rebuild the model, verify the intended body is visible, then export STL or STEP from the final solid-model group."
    )
    response["actions"] = [{
        "type": "show_message",
        "title": response["title"],
        "detail": response["detail"]
    }]

print(json.dumps(response))
