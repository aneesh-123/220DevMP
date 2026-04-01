$ErrorActionPreference = "Stop"

function New-BoardFromRows {
    param(
        [string[]]$Rows
    )

    if ($Rows.Count -ne 6) {
        throw "Each board must contain exactly 6 rows."
    }

    $board = @()
    foreach ($row in $Rows) {
        if ($row.Length -ne 7) {
            throw "Each row must contain exactly 7 cells."
        }

        $boardRow = @()
        foreach ($ch in $row.ToCharArray()) {
            switch ($ch) {
                '.' { $boardRow += $null }
                'O' { $boardRow += 0 }
                'X' { $boardRow += 1 }
                default { throw "Unsupported cell character '$ch'." }
            }
        }
        $board += ,$boardRow
    }

    return ,$board
}

function Get-PieceCounts {
    param(
        [object[][]]$Board
    )

    $counts = @(0, 0)
    foreach ($row in $Board) {
        foreach ($cell in $row) {
            if ($cell -eq 0) { $counts[0]++ }
            elseif ($cell -eq 1) { $counts[1]++ }
        }
    }

    return ,$counts
}

function Test-Gravity {
    param(
        [object[][]]$Board
    )

    for ($col = 0; $col -lt 7; $col++) {
        $foundPiece = $false
        for ($row = 0; $row -lt 6; $row++) {
            $cell = $Board[$row][$col]
            if ($cell -ne $null) {
                $foundPiece = $true
            } elseif ($foundPiece) {
                return $false
            }
        }
    }

    return $true
}

function Get-Winner {
    param(
        [object[][]]$Board
    )

    $directions = @(
        @(0, 1),
        @(1, 0),
        @(1, 1),
        @(-1, 1)
    )

    for ($row = 0; $row -lt 6; $row++) {
        for ($col = 0; $col -lt 7; $col++) {
            $cell = $Board[$row][$col]
            if ($cell -eq $null) {
                continue
            }

            foreach ($direction in $directions) {
                $dr = $direction[0]
                $dc = $direction[1]
                $ok = $true

                for ($step = 1; $step -lt 4; $step++) {
                    $r = $row + $dr * $step
                    $c = $col + $dc * $step
                    if ($r -lt 0 -or $r -ge 6 -or $c -lt 0 -or $c -ge 7) {
                        $ok = $false
                        break
                    }
                    if ($Board[$r][$c] -ne $cell) {
                        $ok = $false
                        break
                    }
                }

                if ($ok) {
                    return $cell
                }
            }
        }
    }

    return $null
}

function Reverse-String {
    param(
        [string]$Text
    )

    $chars = [char[]]$Text
    [array]::Reverse($chars)
    return -join $chars
}

function Mirror-RowsSafe {
    param(
        [string[]]$Rows
    )

    return @($Rows | ForEach-Object { Reverse-String $_ })
}

function Swap-Rows {
    param(
        [string[]]$Rows
    )

    return @($Rows | ForEach-Object {
        $_.Replace('O', 'o').Replace('X', 'O').Replace('o', 'X')
    })
}

function Write-StateFile {
    param(
        [string]$Path,
        [string]$Name,
        [int[]]$Removals,
        [string[]]$Rows
    )

    $board = New-BoardFromRows $Rows
    $counts = Get-PieceCounts $board

    if (-not (Test-Gravity $board)) {
        throw "Gravity check failed for $Path."
    }

    if ($counts[0] -ne $counts[1]) {
        throw "Expected equal piece counts for $Path, found O=$($counts[0]) X=$($counts[1])."
    }

    $winner = Get-Winner $board
    if ($winner -ne $null) {
        throw "Board already has a winner for $Path."
    }

    $payload = [ordered]@{
        name = $Name
        current_player = 0
        removals_remaining = $Removals
        board = $board
    }

    $json = $payload | ConvertTo-Json -Depth 10
    Set-Content -Path $Path -Value $json
}

$root = Split-Path -Parent $PSScriptRoot
$boardDir = Join-Path $root "board_states"

$baseStates = @(
    [ordered]@{
        Id = "opening_center_01"
        Name = "Opening Center Tension 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            ".......",
            "..OXOX."
        )
    },
    [ordered]@{
        Id = "opening_split_01"
        Name = "Opening Split Pressure 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            ".......",
            "O..XXO."
        )
    },
    [ordered]@{
        Id = "opening_wide_01"
        Name = "Opening Wide Wings 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            ".......",
            ".OX..XO"
        )
    },
    [ordered]@{
        Id = "mid_center_stack_01"
        Name = "Midgame Center Stack 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "...X...",
            "...O...",
            "..OXOX."
        )
    },
    [ordered]@{
        Id = "mid_wing_pressure_01"
        Name = "Midgame Wing Pressure 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            ".X...O.",
            "OOX.XXO"
        )
    },
    [ordered]@{
        Id = "mid_balance_01"
        Name = "Midgame Balanced Shape 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            "..XO...",
            ".OXOX.."
        )
    },
    [ordered]@{
        Id = "horizontal_threat_01"
        Name = "Horizontal Threat Race 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            ".......",
            "XX.OOOX"
        )
    },
    [ordered]@{
        Id = "horizontal_block_01"
        Name = "Horizontal Block Test 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            ".......",
            "OO.XXXO"
        )
    },
    [ordered]@{
        Id = "vertical_threat_01"
        Name = "Vertical Threat Tower 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "...O...",
            "X..O...",
            "XO.OXX."
        )
    },
    [ordered]@{
        Id = "vertical_block_01"
        Name = "Vertical Block Tower 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "...X...",
            "O..X...",
            "OX.XOO."
        )
    },
    [ordered]@{
        Id = "diagonal_rise_01"
        Name = "Diagonal Rise Threat 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..OX...",
            ".OXO...",
            "OXXOX.."
        )
    },
    [ordered]@{
        Id = "diagonal_fall_01"
        Name = "Diagonal Fall Threat 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "...XO..",
            "...OX..",
            ".XOOXXO"
        )
    },
    [ordered]@{
        Id = "fork_setup_01"
        Name = "Fork Setup Puzzle 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "...O...",
            "..XO...",
            "XXOOX.."
        )
    },
    [ordered]@{
        Id = "fork_setup_02"
        Name = "Fork Setup Puzzle 02"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "...X...",
            "...O...",
            "OOXXO.X"
        )
    },
    [ordered]@{
        Id = "removal_tension_01"
        Name = "Removal Tension 01"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..XO...",
            ".OXOX..",
            "XOXXOO."
        )
    },
    [ordered]@{
        Id = "removal_tension_02"
        Name = "Removal Tension 02"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "...X...",
            "..OXO..",
            ".XOOXXO"
        )
    },
    [ordered]@{
        Id = "late_game_01"
        Name = "Late Game Grind 01"
        Removals = @(0, 0)
        Rows = @(
            ".......",
            ".......",
            "..XO...",
            ".OXOX..",
            "XOXXOOX",
            "OXOOXXO"
        )
    },
    [ordered]@{
        Id = "late_game_02"
        Name = "Late Game Grind 02"
        Removals = @(0, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..OX...",
            "XOXXO..",
            "OXOOXXO"
        )
    },
    [ordered]@{
        Id = "defense_crunch_01"
        Name = "Defense Crunch 01"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..OX...",
            ".XOXXO.",
            "OOXXOOX"
        )
    },
    [ordered]@{
        Id = "defense_crunch_02"
        Name = "Defense Crunch 02"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..OXO..",
            "..OXXO.",
            "XXOOOXX"
        )
    },

    # ── CATEGORY 1: Opening positions (4-6 pieces) ─────────────────────────────
    [ordered]@{
        Id = "opening_corner_01"
        Name = "Opening Corner Grab 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            ".......",
            "OX...XO"
        )
    },
    [ordered]@{
        Id = "opening_corner_02"
        Name = "Opening Corner Grab 02"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            ".......",
            "XO...OX"
        )
    },
    [ordered]@{
        Id = "opening_gap_01"
        Name = "Opening Gap Center 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            ".......",
            ".OX.XO."
        )
    },
    [ordered]@{
        Id = "opening_gap_02"
        Name = "Opening Gap Center 02"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            ".......",
            ".XO.OX."
        )
    },
    [ordered]@{
        Id = "opening_stack_01"
        Name = "Opening Stack Early 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            "..X.O..",
            "..OXOX."
        )
    },
    [ordered]@{
        Id = "opening_stack_02"
        Name = "Opening Stack Early 02"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            "..O.X..",
            "..XOXO."
        )
    },
    [ordered]@{
        Id = "opening_pressure_01"
        Name = "Opening Early Pressure 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            ".XO....",
            "OXO.X.."
        )
    },
    [ordered]@{
        Id = "opening_pressure_02"
        Name = "Opening Early Pressure 02"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            ".OX....",
            "XOX.O.."
        )
    },
    [ordered]@{
        Id = "opening_flank_01"
        Name = "Opening Flank Control 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            "X......",
            "OXO...."
        )
    },
    [ordered]@{
        Id = "opening_flank_02"
        Name = "Opening Flank Control 02"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            "O......",
            "XOX...."
        )
    },
    [ordered]@{
        Id = "opening_early_fight_01"
        Name = "Opening Early Fight 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            "..XO...",
            ".OXOX.."
        )
    },
    [ordered]@{
        Id = "opening_early_fight_02"
        Name = "Opening Early Fight 02"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            "..OX...",
            ".XOXO.."
        )
    },

    # ── CATEGORY 2: Early-midgame positions (8-10 pieces per side) ─────────────
    [ordered]@{
        Id = "early_mid_01"
        Name = "Early Midgame Center Dominant 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "...O...",
            "..XOX..",
            "XOXOOX."
        )
    },
    [ordered]@{
        Id = "early_mid_02"
        Name = "Early Midgame Center Dominant 02"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "...X...",
            "..OXO..",
            "OXOXXO."
        )
    },
    [ordered]@{
        Id = "early_mid_03"
        Name = "Early Midgame Wing Pressure 03"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "XO.....",
            "OX.....",
            "OOXO.XX"
        )
    },
    [ordered]@{
        Id = "early_mid_04"
        Name = "Early Midgame Wing Pressure 04"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "OX.....",
            "XO.....",
            "XXOO.XO"
        )
    },
    [ordered]@{
        Id = "early_mid_05"
        Name = "Early Midgame One Sided Threat 05"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..O....",
            "..OX...",
            ".XOOXX."
        )
    },
    [ordered]@{
        Id = "early_mid_06"
        Name = "Early Midgame One Sided Threat 06"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..X....",
            "..XO...",
            ".OXXOO."
        )
    },
    [ordered]@{
        Id = "early_mid_07"
        Name = "Early Midgame Symmetric 07"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".X...O.",
            ".XO.OX.",
            "XOXOOX."
        )
    },
    [ordered]@{
        Id = "early_mid_08"
        Name = "Early Midgame Symmetric 08"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".O...X.",
            ".OX.XO.",
            "OXOXXO."
        )
    },
    [ordered]@{
        Id = "early_mid_09"
        Name = "Early Midgame Removal Asymmetry 09"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..X....",
            ".OX.O..",
            "XOOXOX."
        )
    },
    [ordered]@{
        Id = "early_mid_10"
        Name = "Early Midgame Removal Asymmetry 10"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..O....",
            ".XO.X..",
            "OXXOXO."
        )
    },
    [ordered]@{
        Id = "early_mid_11"
        Name = "Early Midgame Dense Center 11"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..XO...",
            "..OX...",
            ".XOXO.."
        )
    },
    [ordered]@{
        Id = "early_mid_12"
        Name = "Early Midgame Dense Center 12"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..OX...",
            "..XO...",
            ".OXOX.."
        )
    },
    [ordered]@{
        Id = "early_mid_13"
        Name = "Early Midgame Spread Board 13"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "O......",
            "XO...X.",
            "OXOX.OX"
        )
    },
    [ordered]@{
        Id = "early_mid_14"
        Name = "Early Midgame Spread Board 14"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "X......",
            "OX...O.",
            "XOXO.XO"
        )
    },
    [ordered]@{
        Id = "early_mid_15"
        Name = "Early Midgame Left Heavy 15"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "XO.....",
            "OX.X...",
            "XOOX.O."
        )
    },
    [ordered]@{
        Id = "early_mid_16"
        Name = "Early Midgame Left Heavy 16"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "OX.....",
            "XO.O...",
            "OXXO.X."
        )
    },
    [ordered]@{
        Id = "early_mid_17"
        Name = "Early Midgame Right Heavy 17"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "......X",
            "...O.XX",
            ".XOOOXO"
        )
    },
    [ordered]@{
        Id = "early_mid_18"
        Name = "Early Midgame Right Heavy 18"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "......O",
            "...X.OO",
            ".OXXXOX"
        )
    },
    [ordered]@{
        Id = "early_mid_19"
        Name = "Early Midgame Battle Line 19"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            "XOXOX..",
            "OXXOO.."
        )
    },
    [ordered]@{
        Id = "early_mid_20"
        Name = "Early Midgame Battle Line 20"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            "OXOXO..",
            "XOXXO.."
        )
    },

    # ── CATEGORY 3: Midgame positions (12-16 pieces) ───────────────────────────
    [ordered]@{
        Id = "midgame_01"
        Name = "Midgame Horizontal Threat 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..OX...",
            ".XOO...",
            "OXXOX.."
        )
    },
    [ordered]@{
        Id = "midgame_02"
        Name = "Midgame Horizontal Threat 02"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..XO...",
            ".OXX...",
            "XOOXO.."
        )
    },
    [ordered]@{
        Id = "midgame_03"
        Name = "Midgame Vertical Threat 03"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            "...O...",
            "...X...",
            "..XOX..",
            "XOOOXXO"
        )
    },
    [ordered]@{
        Id = "midgame_04"
        Name = "Midgame Vertical Threat 04"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            "...X...",
            "...O...",
            "..OXO..",
            "OXXXOOX"
        )
    },
    [ordered]@{
        Id = "midgame_05"
        Name = "Midgame Fork Setup 05"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".OXO...",
            ".OXX...",
            "XOOXOX."
        )
    },
    [ordered]@{
        Id = "midgame_06"
        Name = "Midgame Fork Setup 06"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".XOX...",
            ".XOO...",
            "OXXOXO."
        )
    },
    [ordered]@{
        Id = "midgame_07"
        Name = "Midgame Double Threat 07"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "X.O....",
            "XOOX...",
            "OXOXXO."
        )
    },
    [ordered]@{
        Id = "midgame_08"
        Name = "Midgame Double Threat 08"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "O.X....",
            "OXXO...",
            "XOOOXX."
        )
    },
    [ordered]@{
        Id = "midgame_09"
        Name = "Midgame Blocked Initiative 09"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..OX...",
            "XOXOO..",
            "OXXOXXO"
        )
    },
    [ordered]@{
        Id = "midgame_10"
        Name = "Midgame Blocked Initiative 10"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..XO...",
            "OXOXX..",
            "XOOXOOX"
        )
    },
    [ordered]@{
        Id = "midgame_11"
        Name = "Midgame Dense Left 11"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            "XO.....",
            "XO.....",
            "OXXO...",
            "OXOXXO."
        )
    },
    [ordered]@{
        Id = "midgame_12"
        Name = "Midgame Dense Left 12"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            "OX.....",
            "OX.....",
            "XOOX...",
            "XOXOOX."
        )
    },
    [ordered]@{
        Id = "midgame_13"
        Name = "Midgame Dense Right 13"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".....OX",
            ".....OX",
            "...XOOX",
            ".OXOXXO"
        )
    },
    [ordered]@{
        Id = "midgame_14"
        Name = "Midgame Dense Right 14"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            ".....XO",
            ".....OX",
            "...XOOX",
            "...XOOX"
        )
    },
    [ordered]@{
        Id = "midgame_15"
        Name = "Midgame Center Pillar 15"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            "...X...",
            "...O...",
            "..XOX..",
            "OOXOXXO"
        )
    },
    [ordered]@{
        Id = "midgame_16"
        Name = "Midgame Center Pillar 16"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            "...O...",
            "...X...",
            "..OXO..",
            "XXOOXXO"
        )
    },
    [ordered]@{
        Id = "midgame_17"
        Name = "Midgame Open Diagonal 17"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            "....X..",
            "...OX..",
            "..XOX..",
            "OXOOOX."
        )
    },
    [ordered]@{
        Id = "midgame_18"
        Name = "Midgame Open Diagonal 18"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            "....O..",
            "...XO..",
            "..OXO..",
            ".XXOXOX"
        )
    },
    [ordered]@{
        Id = "midgame_19"
        Name = "Midgame Scattered Fight 19"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            "XOXOXOX",
            "OXOXOXO"
        )
    },
    [ordered]@{
        Id = "midgame_20"
        Name = "Midgame Scattered Fight 20"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            "OXOXOXO",
            "XOXOXOX"
        )
    },

    # ── CATEGORY 4: Fork/threat puzzle positions ───────────────────────────────
    [ordered]@{
        Id = "fork_setup_03"
        Name = "Fork Setup Puzzle 03"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..O....",
            ".OX....",
            "OXXOX.."
        )
    },
    [ordered]@{
        Id = "fork_setup_04"
        Name = "Fork Setup Puzzle 04"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..X....",
            ".XO....",
            "XOOXO.."
        )
    },
    [ordered]@{
        Id = "fork_setup_05"
        Name = "Fork Setup Puzzle 05"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "...O...",
            ".OXO...",
            "XOOXXX."
        )
    },
    [ordered]@{
        Id = "fork_setup_06"
        Name = "Fork Setup Puzzle 06"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "...X...",
            ".XOX...",
            "OXXOOO."
        )
    },
    [ordered]@{
        Id = "fork_setup_07"
        Name = "Fork Setup Puzzle 07"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..OX...",
            ".OOX...",
            "XOXXOXO"
        )
    },
    [ordered]@{
        Id = "fork_setup_08"
        Name = "Fork Setup Puzzle 08"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..XO...",
            ".XXO...",
            "OXOOXOX"
        )
    },
    [ordered]@{
        Id = "fork_setup_09"
        Name = "Fork Setup Puzzle 09"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "O......",
            "OX.X...",
            "OXOOXX."
        )
    },
    [ordered]@{
        Id = "fork_setup_10"
        Name = "Fork Setup Puzzle 10"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "X......",
            "XO.O...",
            "XOXXOO."
        )
    },
    [ordered]@{
        Id = "threat_race_01"
        Name = "Threat Race 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            "XXO....",
            "XXOOOXO"
        )
    },
    [ordered]@{
        Id = "threat_race_02"
        Name = "Threat Race 02"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".......",
            "OOX....",
            "OOXXXOX"
        )
    },
    [ordered]@{
        Id = "threat_race_03"
        Name = "Threat Race 03"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "...O...",
            "..OX...",
            "XXOOXOX"
        )
    },
    [ordered]@{
        Id = "threat_race_04"
        Name = "Threat Race 04"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "...X...",
            "..XO...",
            "OOXXOXO"
        )
    },
    [ordered]@{
        Id = "threat_race_05"
        Name = "Threat Race 05"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "X......",
            "XO.....",
            "XOOOOXX"
        )
    },
    [ordered]@{
        Id = "threat_race_06"
        Name = "Threat Race 06"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "O......",
            "OX.....",
            "OXXXXOO"
        )
    },
    [ordered]@{
        Id = "double_threat_01"
        Name = "Double Threat 01"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..O....",
            ".XOX...",
            "OOXOXX."
        )
    },
    [ordered]@{
        Id = "double_threat_02"
        Name = "Double Threat 02"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..X....",
            ".OXO...",
            "XXOXOO."
        )
    },
    [ordered]@{
        Id = "double_threat_03"
        Name = "Double Threat 03"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".O.....",
            ".OX.X..",
            "XOOOXX."
        )
    },
    [ordered]@{
        Id = "double_threat_04"
        Name = "Double Threat 04"
        Removals = @(1, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            ".X.....",
            ".XO.O..",
            "OXXXOO."
        )
    },

    # ── CATEGORY 5: Removal-tension positions ──────────────────────────────────
    [ordered]@{
        Id = "removal_tension_03"
        Name = "Removal Tension 03"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..XO...",
            ".OXOX..",
            "XOXXOO."
        )
    },
    [ordered]@{
        Id = "removal_tension_04"
        Name = "Removal Tension 04"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..OX...",
            ".XOXO..",
            "OXOOX.X"
        )
    },
    [ordered]@{
        Id = "removal_tension_05"
        Name = "Removal Tension 05"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "X.O....",
            "XOOX...",
            "OXXOOX."
        )
    },
    [ordered]@{
        Id = "removal_tension_06"
        Name = "Removal Tension 06"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "O.X....",
            "OXXO...",
            "XOOXXO."
        )
    },
    [ordered]@{
        Id = "removal_tension_07"
        Name = "Removal Tension 07"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            "..O....",
            "..OX...",
            ".XOOX..",
            "OXXXOOX"
        )
    },
    [ordered]@{
        Id = "removal_tension_08"
        Name = "Removal Tension 08"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            "..X....",
            "..XO...",
            ".OXXO..",
            "XOOOXXO"
        )
    },
    [ordered]@{
        Id = "removal_tension_09"
        Name = "Removal Tension 09"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            "....X..",
            "...OX..",
            ".XOOX..",
            "OOXOXOX"
        )
    },
    [ordered]@{
        Id = "removal_tension_10"
        Name = "Removal Tension 10"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            "....O..",
            "...XO..",
            ".OXXO..",
            "XXOXOXO"
        )
    },
    [ordered]@{
        Id = "removal_tension_11"
        Name = "Removal Tension 11"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "XOOX...",
            "OXXO...",
            "OXOXXO."
        )
    },
    [ordered]@{
        Id = "removal_tension_12"
        Name = "Removal Tension 12"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "OXXO...",
            "XOOX...",
            "XOXXOO."
        )
    },
    [ordered]@{
        Id = "removal_tension_13"
        Name = "Removal Tension 13"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            "...X...",
            "..OX...",
            ".XOOX..",
            "OOXOXXO"
        )
    },
    [ordered]@{
        Id = "removal_tension_14"
        Name = "Removal Tension 14"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            "...O...",
            "..XO...",
            ".OXXO..",
            "XXOXOOX"
        )
    },
    [ordered]@{
        Id = "removal_tension_15"
        Name = "Removal Tension 15"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..XO...",
            "XOOX...",
            "OXOXXO."
        )
    },
    [ordered]@{
        Id = "removal_tension_16"
        Name = "Removal Tension 16"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "..OX...",
            "OXXO...",
            "XOXOOX."
        )
    },
    [ordered]@{
        Id = "removal_tension_17"
        Name = "Removal Tension 17"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "X.O.X..",
            "OXOO...",
            "XOXXOOX"
        )
    },

    # ── CATEGORY 6: Late-game positions (20-28 pieces) ─────────────────────────
    [ordered]@{
        Id = "late_game_03"
        Name = "Late Game Grind 03"
        Removals = @(0, 0)
        Rows = @(
            ".......",
            "..XO...",
            "..XO...",
            ".OXOX..",
            "XOXXOOX",
            "OXOOXXO"
        )
    },
    [ordered]@{
        Id = "late_game_04"
        Name = "Late Game Grind 04"
        Removals = @(0, 0)
        Rows = @(
            ".......",
            "..OX...",
            "..OX...",
            ".XOXO..",
            "OXOOXOX",
            "XOXXOOX"
        )
    },
    [ordered]@{
        Id = "late_game_05"
        Name = "Late Game Grind 05"
        Removals = @(0, 0)
        Rows = @(
            ".......",
            ".......",
            "XOOXX..",
            "OXXOO..",
            "OXOXXOX",
            "XOXOOXO"
        )
    },
    [ordered]@{
        Id = "late_game_06"
        Name = "Late Game Grind 06"
        Removals = @(0, 0)
        Rows = @(
            ".......",
            ".......",
            "..XOXO.",
            "..OXOX.",
            "XOXXOOX",
            "OXOOXOX"
        )
    },
    [ordered]@{
        Id = "late_game_07"
        Name = "Late Game Grind 07"
        Removals = @(0, 0)
        Rows = @(
            ".......",
            "..XOXO.",
            "XOOXOX.",
            "OXOXOOX",
            "XOXXOOX",
            "OXXOOXO"
        )
    },
    [ordered]@{
        Id = "late_game_08"
        Name = "Late Game Grind 08"
        Removals = @(0, 0)
        Rows = @(
            ".......",
            "O.XO...",
            "OXXO...",
            "XOOO...",
            "OOOXXXX",
            "XOOXOXO"
        )
    },
    [ordered]@{
        Id = "late_game_09"
        Name = "Late Game Grind 09"
        Removals = @(0, 0)
        Rows = @(
            ".......",
            "XOOX...",
            "OXOX...",
            "OXXOO..",
            "XOXXOOX",
            "OXOOOXX"
        )
    },
    [ordered]@{
        Id = "late_game_10"
        Name = "Late Game Grind 10"
        Removals = @(0, 0)
        Rows = @(
            ".......",
            "OXXO...",
            "XOXO...",
            "XOOX...",
            "OXOOXXO",
            "XOXXXOO"
        )
    },
    [ordered]@{
        Id = "late_game_11"
        Name = "Late Game Grind 11"
        Removals = @(0, 0)
        Rows = @(
            ".......",
            "..XOXO.",
            "..OXOX.",
            "XOOXOX.",
            "OXXOOXO",
            "XOOXOXX"
        )
    },
    [ordered]@{
        Id = "late_game_12"
        Name = "Late Game Grind 12"
        Removals = @(0, 0)
        Rows = @(
            ".......",
            "..OXOX.",
            "..XOXO.",
            "OXXOXO.",
            "XOOXOXO",
            "OXXOXOO"
        )
    },

    # ── CATEGORY 7: Defense crunch and endgame squeeze ─────────────────────────
    [ordered]@{
        Id = "defense_crunch_03"
        Name = "Defense Crunch 03"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            "..OX...",
            ".OXX...",
            "OOXOO..",
            "XXOOOXX"
        )
    },
    [ordered]@{
        Id = "defense_crunch_04"
        Name = "Defense Crunch 04"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            "..XO...",
            ".XOO...",
            "XXOXX..",
            "OOXOOXX"
        )
    },
    [ordered]@{
        Id = "defense_crunch_05"
        Name = "Defense Crunch 05"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "X.OX...",
            "OXOXX..",
            "OOXOOXX"
        )
    },
    [ordered]@{
        Id = "defense_crunch_06"
        Name = "Defense Crunch 06"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            ".......",
            "O.XO...",
            "XOXOO..",
            "XXOXXOO"
        )
    },
    [ordered]@{
        Id = "defense_crunch_07"
        Name = "Defense Crunch 07"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            "...O...",
            "..OX...",
            "XOOXO..",
            "OXXOXXO"
        )
    },
    [ordered]@{
        Id = "defense_crunch_08"
        Name = "Defense Crunch 08"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            "...X...",
            "..XO...",
            "OXXOX..",
            "XOOXOOX"
        )
    },
    [ordered]@{
        Id = "defense_crunch_09"
        Name = "Defense Crunch 09"
        Removals = @(0, 1)
        Rows = @(
            ".......",
            ".......",
            "..O....",
            "XOOX...",
            "OXXO...",
            "OOXOOXX"
        )
    },
    [ordered]@{
        Id = "defense_crunch_10"
        Name = "Defense Crunch 10"
        Removals = @(1, 0)
        Rows = @(
            ".......",
            ".......",
            "..X....",
            "OXXO...",
            "XOOX...",
            "XXOXXOO"
        )
    }
)

$variants = @(
    @{ Suffix = ""; NameSuffix = ""; Mirror = $false; Swap = $false },
    @{ Suffix = "_mirror"; NameSuffix = " Mirror"; Mirror = $true; Swap = $false },
    @{ Suffix = "_swap"; NameSuffix = " Player Swap"; Mirror = $false; Swap = $true },
    @{ Suffix = "_mirror_swap"; NameSuffix = " Mirror Swap"; Mirror = $true; Swap = $true }
)

$written = 0
foreach ($state in $baseStates) {
    foreach ($variant in $variants) {
        $rows = $state.Rows
        $removals = @($state.Removals[0], $state.Removals[1])

        if ($variant.Mirror) {
            $rows = Mirror-RowsSafe $rows
        }
        if ($variant.Swap) {
            $rows = Swap-Rows $rows
            $removals = @($removals[1], $removals[0])
        }

        $path = Join-Path $boardDir ("suite_{0}{1}.json" -f $state.Id, $variant.Suffix)
        $name = "{0}{1}" -f $state.Name, $variant.NameSuffix
        Write-StateFile -Path $path -Name $name -Removals $removals -Rows $rows
        $written++
    }
}

Write-Host "Wrote $written board states to $boardDir"
