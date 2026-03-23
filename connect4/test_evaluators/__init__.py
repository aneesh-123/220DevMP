"""
Test evaluator discovery for Connect 4 experiments.

Each .py file in this folder defines an EVALUATOR instance used as
an opponent when benchmarking student evaluators.
"""

import importlib
import importlib.util
import sys
from pathlib import Path
from typing import Dict


def discover_test_evaluators() -> Dict[str, object]:
    """
    Dynamically load all test evaluators from this folder.

    Returns:
        Dictionary of {display_name: evaluator_instance}.
    """
    evaluators = {}
    eval_dir = Path(__file__).parent

    project_root = Path(__file__).parent.parent.parent
    if str(project_root) not in sys.path:
        sys.path.insert(0, str(project_root))

    for eval_file in sorted(eval_dir.glob("*.py")):
        if eval_file.name.startswith("_"):
            continue

        module_name = eval_file.stem
        try:
            spec = importlib.util.spec_from_file_location(
                f"connect4.test_evaluators.{module_name}",
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
            print(f"Warning: Could not load test evaluator from {eval_file.name}: {e}")

    return evaluators
