"""
Evaluator discovery for Connect 4.

Each .py file in this folder defines an EVALUATOR instance.
They are automatically discovered and made available in the CLI.
"""

import importlib
import importlib.util
import sys
from pathlib import Path
from typing import Dict


def discover_evaluators() -> Dict[str, object]:
    """
    Dynamically load all evaluators from this folder.

    Each evaluator file must define an EVALUATOR variable (an instance
    with an evaluate(state, player) method and a __repr__ for display).

    Returns:
        Dictionary of {display_name: evaluator_instance}.
    """
    evaluators = {}
    eval_dir = Path(__file__).parent

    # Ensure project root is in sys.path for imports
    project_root = Path(__file__).parent.parent.parent
    if str(project_root) not in sys.path:
        sys.path.insert(0, str(project_root))

    for eval_file in sorted(eval_dir.glob("*.py")):
        if eval_file.name.startswith("_"):
            continue

        module_name = eval_file.stem
        try:
            spec = importlib.util.spec_from_file_location(
                f"connect4.evaluators.{module_name}",
                eval_file,
            )
            if spec and spec.loader:
                module = importlib.util.module_from_spec(spec)
                sys.modules[spec.name] = module
                spec.loader.exec_module(module)

                if hasattr(module, "EVALUATOR"):
                    evaluator = module.EVALUATOR
                    name = repr(evaluator)
                    evaluators[name] = evaluator
        except Exception as e:
            print(f"Warning: Could not load evaluator from {eval_file.name}: {e}")

    return evaluators
