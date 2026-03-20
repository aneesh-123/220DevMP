"""
Connect 4 with Removals: AI Heuristic Design Project

A modified Connect 4 game where players can remove pieces and test heuristic
evaluation functions with a built-in minimax search engine.
"""

__version__ = "0.1.0"

from .board import GameState
from .moves import PlacementMove, RemovalMove
from .bots import MinimaxBot
from .evaluators import BasicEvaluator

__all__ = [
    "GameState",
    "PlacementMove",
    "RemovalMove",
    "MinimaxBot",
    "BasicEvaluator",
]
