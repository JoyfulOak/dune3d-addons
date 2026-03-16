#!/usr/bin/env python3
import json
import os

command = os.environ.get("DUNE3D_ADDON_COMMAND", "")

response = {
    "title": "Constraint Context Delete",
    "detail": "Pick a command.",
    "actions": []
}

if command == "show-api-status":
    response["title"] = "Constraint Delete Not Yet Available"
    response["detail"] = (
        "The current public Dune 3D addon API does not expose constraint-list right-click hooks, "
        "cursor-position popups, active constraint selection, or a delete-constraint action. "
        "This addon is a placeholder so the requested feature can be wired up once those hooks exist."
    )
    response["actions"] = [{
        "type": "show_message",
        "title": response["title"],
        "detail": response["detail"]
    }]

print(json.dumps(response))
