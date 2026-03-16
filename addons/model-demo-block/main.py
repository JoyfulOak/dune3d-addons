#!/usr/bin/env python3
import json
import os

command = os.environ.get("DUNE3D_ADDON_COMMAND", "")
response = {
    "title": "Model Demo Block",
    "detail": "Pick a command.",
    "actions": []
}

if command == "insert-demo-block":
    response["title"] = "Demo Block Inserted"
    response["detail"] = "Inserted the packaged STEP block into the current document. Saved projects receive a local project copy automatically."
    response["actions"] = [{
        "type": "insert_step_model",
        "path": "demo-block.step",
        "copy_to_project": True,
        "lock_rotation": True
    }]

print(json.dumps(response))
