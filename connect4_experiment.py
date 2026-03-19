"""
Minimal Connect 4 experiment: Test if removal-aware heuristics improve performance.

This experiment compares two heuristics using identical search (negamax + alpha-beta):
- Heuristic A: Baseline (center preference, threat counting)
- Heuristic B: Removal-aware (adds removal mechanics awareness)
"""

import random
from typing import List, Tuple, Optional
from enum import IntEnum


class Player(IntEnum):
    NONE = 0
    PLAYER1 = 1
    PLAYER2 = -1


class ConnectFourGame:
    """Connect 4 with removal mechanic: 6 rows x 7 cols."""

    ROWS = 6
    COLS = 7
    WIN_LENGTH = 4
    MAX_MOVES = 80

    def __init__(self):
        self.board = [[Player.NONE for _ in range(self.COLS)] for _ in range(self.ROWS)]
        self.turn = Player.PLAYER1
        self.move_count = 0

    def get_legal_moves(self) -> List[Tuple[str, int]]:
        """Return list of (move_type, column) tuples.

        move_type: 'drop' or 'remove'
        column: 0-6
        """
        moves = []

        # DROP moves: piece enters at top of column
        for col in range(self.COLS):
            if self.board[0][col] == Player.NONE:
                moves.append(('drop', col))

        # REMOVE moves: can only remove bottom piece if it's ours
        for col in range(self.COLS):
            if self.board[self.ROWS - 1][col] == self.turn:
                moves.append(('remove', col))

        return moves

    def apply_move(self, move_type: str, col: int) -> None:
        """Apply a move to the board."""
        if move_type == 'drop':
            # Find lowest empty row in this column and place piece
            for row in range(self.ROWS - 1, -1, -1):
                if self.board[row][col] == Player.NONE:
                    self.board[row][col] = self.turn
                    break

        elif move_type == 'remove':
            # Remove bottom piece; shift all pieces above down by 1
            assert self.board[self.ROWS - 1][col] == self.turn, "Cannot remove opponent piece"
            for row in range(self.ROWS - 1, 0, -1):
                self.board[row][col] = self.board[row - 1][col]
            self.board[0][col] = Player.NONE

        self.move_count += 1
        self.turn = -self.turn

    def undo_move(self, move_type: str, col: int) -> None:
        """Undo a move (for search)."""
        self.turn = -self.turn
        self.move_count -= 1

        if move_type == 'drop':
            # Find and remove the piece we just dropped
            for row in range(self.ROWS):
                if self.board[row][col] == self.turn:
                    self.board[row][col] = Player.NONE
                    break

        elif move_type == 'remove':
            # Shift pieces back up
            for row in range(self.ROWS - 1):
                self.board[row][col] = self.board[row + 1][col]
            self.board[self.ROWS - 1][col] = self.turn

    def check_winner(self, player: int) -> bool:
        """Check if player has 4 in a row."""
        # Horizontal
        for row in range(self.ROWS):
            for col in range(self.COLS - self.WIN_LENGTH + 1):
                if all(self.board[row][col + i] == player for i in range(self.WIN_LENGTH)):
                    return True

        # Vertical
        for row in range(self.ROWS - self.WIN_LENGTH + 1):
            for col in range(self.COLS):
                if all(self.board[row + i][col] == player for i in range(self.WIN_LENGTH)):
                    return True

        # Diagonal (top-left to bottom-right)
        for row in range(self.ROWS - self.WIN_LENGTH + 1):
            for col in range(self.COLS - self.WIN_LENGTH + 1):
                if all(self.board[row + i][col + i] == player for i in range(self.WIN_LENGTH)):
                    return True

        # Diagonal (bottom-left to top-right)
        for row in range(self.ROWS - self.WIN_LENGTH + 1):
            for col in range(self.WIN_LENGTH - 1, self.COLS):
                if all(self.board[row + i][col - i] == player for i in range(self.WIN_LENGTH)):
                    return True

        return False

    def is_game_over(self) -> Tuple[bool, Optional[int]]:
        """Return (is_over, winner).

        winner: Player1 (1), Player2 (-1), or None (draw=0).
        """
        if self.check_winner(Player.PLAYER1):
            return True, Player.PLAYER1
        if self.check_winner(Player.PLAYER2):
            return True, Player.PLAYER2

        # Draw: move limit or no legal moves
        if self.move_count >= self.MAX_MOVES or len(self.get_legal_moves()) == 0:
            return True, None

        return False, None

    def copy(self) -> 'ConnectFourGame':
        """Return a deep copy."""
        new_game = ConnectFourGame()
        new_game.board = [row[:] for row in self.board]
        new_game.turn = self.turn
        new_game.move_count = self.move_count
        return new_game


class HeuristicA:
    """Baseline heuristic: no removal awareness."""

    @staticmethod
    def evaluate(game: ConnectFourGame, player: int) -> int:
        """Evaluate position from perspective of player."""
        score = 0

        # Prefer center columns
        center_col_order = [3, 2, 4, 1, 5, 0, 6]
        for col in center_col_order:
            for row in range(game.ROWS):
                if game.board[row][col] == player:
                    score += 3
                elif game.board[row][col] == -player:
                    score -= 3

        # Count threats (2-in-a-rows, 3-in-a-rows)
        threat_bonus = HeuristicA._count_threats(game, player)
        threat_penalty = HeuristicA._count_threats(game, -player)

        score += threat_bonus * 10
        score -= threat_penalty * 20

        return score

    @staticmethod
    def _count_threats(game: ConnectFourGame, player: int) -> int:
        """Count 2-in-a-rows and 3-in-a-rows."""
        count = 0

        # Horizontal
        for row in range(game.ROWS):
            for col in range(game.COLS - 2):
                segment = [game.board[row][col + i] for i in range(3)]
                if segment.count(player) == 3:
                    count += 4  # Very strong
                elif segment.count(player) == 2 and segment.count(Player.NONE) == 1:
                    count += 1

        # Vertical
        for row in range(game.ROWS - 2):
            for col in range(game.COLS):
                segment = [game.board[row + i][col] for i in range(3)]
                if segment.count(player) == 3:
                    count += 4
                elif segment.count(player) == 2 and segment.count(Player.NONE) == 1:
                    count += 1

        # Diagonal \
        for row in range(game.ROWS - 2):
            for col in range(game.COLS - 2):
                segment = [game.board[row + i][col + i] for i in range(3)]
                if segment.count(player) == 3:
                    count += 4
                elif segment.count(player) == 2 and segment.count(Player.NONE) == 1:
                    count += 1

        # Diagonal /
        for row in range(game.ROWS - 2):
            for col in range(2, game.COLS):
                segment = [game.board[row + i][col - i] for i in range(3)]
                if segment.count(player) == 3:
                    count += 4
                elif segment.count(player) == 2 and segment.count(Player.NONE) == 1:
                    count += 1

        return count


class HeuristicB(HeuristicA):
    """Removal-aware heuristic: extends A with removal mechanics."""

    @staticmethod
    def evaluate(game: ConnectFourGame, player: int) -> int:
        """Evaluate position, including removal mechanics awareness."""
        # Start with baseline
        score = HeuristicA.evaluate(game, player)

        # Bonus for having removal moves available
        moves = game.get_legal_moves()
        own_removals = sum(1 for mt, _ in moves if mt == 'remove')
        score += own_removals * 5

        # Penalty for opponent having many removal options
        game.turn = -game.turn
        opponent_moves = game.get_legal_moves()
        opponent_removals = sum(1 for mt, _ in opponent_moves if mt == 'remove')
        game.turn = -game.turn
        score -= opponent_removals * 8

        # Penalty for "fragile" structures (pieces depending on removable bottom pieces)
        score -= HeuristicB._fragility_score(game, player)
        score += HeuristicB._fragility_score(game, -player)

        return score

    @staticmethod
    def _fragility_score(game: ConnectFourGame, player: int) -> int:
        """Penalize structures vulnerable to opponent removals."""
        penalty = 0

        for col in range(game.COLS):
            # If opponent's piece is at bottom, our pieces above are fragile
            if game.board[game.ROWS - 1][col] == -player:
                for row in range(game.ROWS - 1):
                    if game.board[row][col] == player:
                        penalty += 2

        return penalty


class HeuristicC(HeuristicA):
    """Threat-focused aggressive heuristic: no removal awareness."""

    @staticmethod
    def evaluate(game: ConnectFourGame, player: int) -> int:
        """Evaluate position with aggressive threat focus."""
        score = 0

        # Less center preference than A, more threat-focused
        threat_bonus = HeuristicC._count_threats(game, player)
        threat_penalty = HeuristicC._count_threats(game, -player)

        # Much heavier weighting on threat creation and blocking
        score += threat_bonus * 30  # vs A's 10
        score -= threat_penalty * 50  # vs A's 20

        # Board control: prefer higher rows (defensive/blocking)
        for row in range(game.ROWS):
            for col in range(game.COLS):
                if game.board[row][col] == player:
                    # Higher rows get more points (closer to opponent)
                    score += (game.ROWS - row)
                elif game.board[row][col] == -player:
                    score -= (game.ROWS - row)

        return score

    @staticmethod
    def _count_threats(game: ConnectFourGame, player: int) -> int:
        """Count threats with aggressive weighting."""
        count = 0

        # Horizontal
        for row in range(game.ROWS):
            for col in range(game.COLS - 2):
                segment = [game.board[row][col + i] for i in range(3)]
                if segment.count(player) == 3:
                    count += 10  # vs A's 4
                elif segment.count(player) == 2 and segment.count(Player.NONE) == 1:
                    count += 3  # vs A's 1

        # Vertical
        for row in range(game.ROWS - 2):
            for col in range(game.COLS):
                segment = [game.board[row + i][col] for i in range(3)]
                if segment.count(player) == 3:
                    count += 10
                elif segment.count(player) == 2 and segment.count(Player.NONE) == 1:
                    count += 3

        # Diagonal \
        for row in range(game.ROWS - 2):
            for col in range(game.COLS - 2):
                segment = [game.board[row + i][col + i] for i in range(3)]
                if segment.count(player) == 3:
                    count += 10
                elif segment.count(player) == 2 and segment.count(Player.NONE) == 1:
                    count += 3

        # Diagonal /
        for row in range(game.ROWS - 2):
            for col in range(2, game.COLS):
                segment = [game.board[row + i][col - i] for i in range(3)]
                if segment.count(player) == 3:
                    count += 10
                elif segment.count(player) == 2 and segment.count(Player.NONE) == 1:
                    count += 3

        return count


class HeuristicD(HeuristicB):
    """Removal chain-aware heuristic: extends B with strategic column control and lookahead."""

    @staticmethod
    def evaluate(game: ConnectFourGame, player: int) -> int:
        """Evaluate position with strategic removal cascade awareness."""
        # Start with B's evaluation
        score = HeuristicB.evaluate(game, player)

        # Strategic column control: which columns are winnable/holdable?
        score += HeuristicD._evaluate_win_paths(game, player) * 15

        # Look ahead: can we create winning removal cascades?
        score += HeuristicD._evaluate_removal_opportunities(game, player) * 10

        # Defense: prevent opponent from winning via removal
        score -= HeuristicD._evaluate_removal_opportunities(game, -player) * 12

        return score

    @staticmethod
    def _evaluate_win_paths(game: ConnectFourGame, player: int) -> int:
        """Identify columns that lead to wins or are being contested."""
        score = 0

        for col in range(game.COLS):
            bottom = game.board[game.ROWS - 1][col]
            our_count = sum(1 for row in range(game.ROWS) if game.board[row][col] == player)
            opp_count = sum(1 for row in range(game.ROWS) if game.board[row][col] == -player)

            if bottom == player:
                # We own the bottom - this column is "ours"
                # High value if we have multiple pieces here
                if our_count >= 3:
                    score += 20  # Almost full, very controllable
                elif our_count >= 2:
                    score += 10

            elif bottom == -player:
                # Opponent owns bottom - our pieces are at risk
                # But this column might still be useful if mostly empty
                if opp_count == 1 and our_count == 0:
                    score += 2  # Mostly empty, opponent has minimal control
                else:
                    score -= opp_count * 3

            else:  # bottom is empty
                # Neutral ground - value based on who can win it
                if our_count > opp_count:
                    score += (our_count - opp_count) * 3

        return score

    @staticmethod
    def _evaluate_removal_opportunities(game: ConnectFourGame, player: int) -> int:
        """Detect if this player can create/execute threatening removal chains."""
        score = 0

        # For each column, evaluate removal sequences
        for col in range(game.COLS):
            bottom = game.board[game.ROWS - 1][col]

            if bottom != player:
                continue  # Can't remove opponent pieces

            # How many own pieces are in this column?
            pieces = []
            for row in range(game.ROWS):
                if game.board[row][col] == player:
                    pieces.append(row)

            if len(pieces) == 0:
                continue

            # Score based on potential: removals create cascade opportunities
            # More pieces = more removal options
            score += len(pieces) * 2

            # Check if removing pieces would expose winning threats
            # (another piece adjacent horizontally becomes part of a 4-in-a-row)
            for piece_row in pieces:
                threat_value = HeuristicD._check_removal_threat(game, piece_row, col, player)
                score += threat_value * 5

        return score

    @staticmethod
    def _check_removal_threat(game: ConnectFourGame, row: int, col: int, player: int) -> int:
        """Check if removing piece at (row, col) would create winning opportunities."""
        threat_score = 0

        # Check horizontal neighbors (down-shifted piece might form 4-in-row)
        for dc in [-3, -2, -1, 0, 1, 2, 3]:
            if 0 <= col + dc < game.COLS:
                # After removal, piece at (row+1, col) shifts to (row, col)
                # Simulate: would pieces to the left/right form threats?
                count = 1  # The shifted piece
                for i in range(1, 4):
                    if col + dc + i < game.COLS:
                        if game.board[row][col + dc + i] == player:
                            count += 1
                    if col + dc - i >= 0:
                        if game.board[row][col + dc - i] == player:
                            count += 1

                if count >= 3:
                    threat_score += 1

        # Check vertical: removal opens up vertical threats
        vertical_potential = 0
        for row_offset in range(1, 3):
            if row + row_offset < game.ROWS:
                if game.board[row + row_offset][col] == player:
                    vertical_potential += 1

        if vertical_potential >= 2:
            threat_score += 2

        return threat_score



class Negamax:
    """Negamax with alpha-beta pruning."""

    def __init__(self, depth: int, heuristic):
        self.depth = depth
        self.heuristic = heuristic
        self.nodes_evaluated = 0

    def search(self, game: ConnectFourGame, player: int) -> Tuple[Tuple[str, int], int]:
        """Find best move for player.

        Returns: ((move_type, col), score)
        """
        best_move = None
        best_score = float('-inf')
        alpha = float('-inf')
        beta = float('inf')

        moves = game.get_legal_moves()
        # Simple move ordering: prioritize drops over removes
        moves.sort(key=lambda m: (m[0] == 'remove', m[1]))

        for move_type, col in moves:
            game.apply_move(move_type, col)
            score = -self._negamax(game, self.depth - 1, -beta, -alpha, -player)
            game.undo_move(move_type, col)

            if score > best_score:
                best_score = score
                best_move = (move_type, col)

            alpha = max(alpha, score)
            if alpha >= beta:
                break

        return best_move, best_score

    def _negamax(self, game: ConnectFourGame, depth: int, alpha: float, beta: float, player: int) -> int:
        """Negamax recursion."""
        # Terminal states
        is_over, winner = game.is_game_over()
        if is_over:
            if winner == player:
                return 1000000  # Win
            elif winner is None:
                return 0  # Draw
            else:
                return -1000000  # Loss

        # Leaf nodes
        if depth == 0:
            self.nodes_evaluated += 1
            return self.heuristic.evaluate(game, player)

        max_score = float('-inf')

        moves = game.get_legal_moves()
        moves.sort(key=lambda m: (m[0] == 'remove', m[1]))

        for move_type, col in moves:
            game.apply_move(move_type, col)
            score = -self._negamax(game, depth - 1, -beta, -alpha, -player)
            game.undo_move(move_type, col)

            max_score = max(max_score, score)
            alpha = max(alpha, score)
            if alpha >= beta:
                break

        return max_score


class Experiment:
    """Run controlled experiment comparing two heuristics."""

    def __init__(self, depth: int, num_games: int, heuristic1_name: str, heuristic1_class,
                 heuristic2_name: str, heuristic2_class):
        self.depth = depth
        self.num_games = num_games
        self.heuristic1_name = heuristic1_name
        self.heuristic2_name = heuristic2_name
        self.bot_1 = Negamax(depth, heuristic1_class)
        self.bot_2 = Negamax(depth, heuristic2_class)

        self.wins_1 = 0
        self.wins_2 = 0
        self.draws = 0
        self.divergence_examples = []  # Track games where heuristics differ

    def run(self):
        """Play num_games games, alternating starting player."""
        print(f"\nStarting {self.num_games} games: {self.heuristic1_name} vs {self.heuristic2_name} (depth {self.depth})...")

        for game_num in range(self.num_games):
            # Alternate which bot goes first
            if game_num % 2 == 0:
                # Bot 1 is Player1, Bot 2 is Player2
                winner = self._play_game(self.bot_1, self.bot_2)
                if winner == Player.PLAYER1:
                    self.wins_1 += 1
                elif winner == Player.PLAYER2:
                    self.wins_2 += 1
                else:
                    self.draws += 1
            else:
                # Bot 2 is Player1, Bot 1 is Player2
                winner = self._play_game(self.bot_2, self.bot_1)
                if winner == Player.PLAYER1:
                    self.wins_2 += 1
                elif winner == Player.PLAYER2:
                    self.wins_1 += 1
                else:
                    self.draws += 1

            if (game_num + 1) % 10 == 0:
                print(f"  Completed {game_num + 1} games...")

    def _play_game(self, bot1, bot2) -> Optional[int]:
        """Play one game: bot1 is Player1, bot2 is Player2.

        Returns: winning player or None (draw).
        """
        game = ConnectFourGame()

        while True:
            is_over, winner = game.is_game_over()
            if is_over:
                return winner

            if game.turn == Player.PLAYER1:
                move, _ = bot1.search(game, game.turn)
            else:
                move, _ = bot2.search(game, game.turn)

            # Track divergence: check if opposite heuristic would choose differently
            if len(self.divergence_examples) < 3:
                self._check_divergence(game, bot1, bot2)

            game.apply_move(*move)

    def _check_divergence(self, game: ConnectFourGame, bot1, bot2) -> None:
        """Check if bots would make different moves from current position."""
        move1, score1 = bot1.search(game, game.turn)
        move2, score2 = bot2.search(game, game.turn)

        if move1 != move2:
            # Only record first few divergences
            self.divergence_examples.append({
                'move_1': move1,
                'move_2': move2,
                'score_1': score1,
                'score_2': score2,
                'depth': self.depth,
            })

    def print_results(self):
        """Print experiment results."""
        total = self.wins_1 + self.wins_2 + self.draws
        win_rate_2 = (self.wins_2 / total * 100) if total > 0 else 0.0

        print(f"\n{'='*70}")
        print(f"{self.heuristic1_name} vs {self.heuristic2_name} ({total} games, depth={self.depth})")
        print(f"{'='*70}")
        print(f"{self.heuristic1_name} wins: {self.wins_1}")
        print(f"{self.heuristic2_name} wins: {self.wins_2}")
        print(f"Draws: {self.draws}")
        print(f"Win rate for {self.heuristic2_name}: {win_rate_2:.1f}%")

        # Show divergence examples
        if self.divergence_examples:
            print(f"\n{'-'*70}")
            print(f"Examples where heuristics diverged:")
            print(f"{'-'*70}")
            for i, ex in enumerate(self.divergence_examples, 1):
                print(f"\nExample {i}:")
                print(f"  {self.heuristic1_name} chooses: {ex['move_1']} (score: {ex['score_1']})")
                print(f"  {self.heuristic2_name} chooses: {ex['move_2']} (score: {ex['score_2']})")
        print()


def main():
    """Run experiments comparing all heuristic pairs."""
    random.seed(42)

    print("\n" + "="*70)
    print("Connect 4 Heuristic Experiment")
    print("Comparing four heuristics to test if removal awareness matters")
    print("="*70)
    print("\nHeuristic A (Baseline): Center preference + threat counting")
    print("Heuristic B (Removal-aware): A + removal mechanics awareness")
    print("Heuristic C (Threat-focused): Aggressive threat focus, no removal awareness")
    print("Heuristic D (Chain-aware): B + column control + removal cascades")
    print("="*70)

    depth = 3
    num_games = 50

    # Run all pairwise comparisons involving D
    comparisons = [
        ("Baseline (A)", HeuristicA, "Removal-aware (B)", HeuristicB),
        ("Removal-aware (B)", HeuristicB, "Chain-aware (D)", HeuristicD),
        ("Baseline (A)", HeuristicA, "Chain-aware (D)", HeuristicD),
        ("Chain-aware (D)", HeuristicD, "Threat-focused (C)", HeuristicC),
    ]

    for name1, heur1, name2, heur2 in comparisons:
        exp = Experiment(depth=depth, num_games=num_games,
                        heuristic1_name=name1, heuristic1_class=heur1,
                        heuristic2_name=name2, heuristic2_class=heur2)
        exp.run()
        exp.print_results()

    print("="*70)
    print("Experiment complete.")


if __name__ == '__main__':
    main()
