"""Local incident hold for interactive host tests; no environment-variable bypass."""
import json
from pathlib import Path


def require_desktop_runtime_available(root=None):
    root=Path(root) if root is not None else Path(__file__).resolve().parents[1]
    path=root/'.local/desktop-runtime-hold.json'
    if not path.exists():return
    try:hold=json.loads(path.read_text(encoding='utf-8'))
    except (OSError,ValueError) as error:
        raise RuntimeError(f'Cannot verify desktop runtime incident hold: {path}') from error
    if hold.get('active',True):
        raise RuntimeError('Interactive host game tests are held after a reported Windows freeze. '
            'Do not launch a game, capture the desktop, or inject input on this host. '
            f'See {path} and docs/HOST_FREEZE_INCIDENT.md; use an isolated guest for further runtime investigation.')
