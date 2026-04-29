# Shakashaka Solver (PUZ-PRE v3)

This project contains two C programs for solving Shakashaka puzzles in PUZ-PRE v3 format:

- `logical_solver.c`: logical-only solver (no guessing/backtracking)
- `solver.c`: full solver (logic + backtracking)

## Features

- Reads puzzle input in PUZ-PRE v3 (`pzprv3`, `shakashaka`) format
- Prints board states in terminal with visual symbols during solving
- Writes solved result as PUZ-PRE v3 output file
- Reports elapsed solving time in seconds
- Reports iteration count and remaining unsolved cells

## Build

Compile with `gcc`:

```bash
gcc -O2 -std=c11 -o logical_solver logical_solver.c
gcc -O2 -std=c11 -o solver solver.c
```

## Run

```bash
./logical_solver <input_file>
./solver <input_file>
```

Example:

```bash
./logical_solver problem001.txt
./solver problem001.txt
```

## Input Format (PUZ-PRE v3)

Input must follow this structure:

1. `pzprv3`
2. `shakashaka`
3. `<height>`
4. `<width>`
5. `height` lines of puzzle cells
6. `height` lines of initial solution state (must be dots)

Cell tokens in puzzle section:

- `0` to `4`: numbered black clue cell
- `5`: black cell without number
- `.`: undecided white-area cell

Initial solution rows should contain `.` values (space-separated), matching width.

## Output Format (PUZ-PRE v3)

Both programs output PUZ-PRE v3 files with the same header and board size.

Output file naming:

- `logical_solver.c`: `<input_basename>_logic_solved.txt`
- `solver.c`: `<input_basename>_solved.txt`

Solution layer tokens:

- `.`: white cell
- `2`: triangle down-left
- `3`: triangle down-right
- `4`: triangle up-right
- `5`: triangle up-left

## Terminal Visualization

During execution, programs print board snapshots in terminal:

- black cell: solid block
- white cell: blank space
- triangles: quarter-triangle symbols

Printed stages include:

- Problem Board
- Logical Solved Board
- (for `solver.c`) final Solution Board

## Timing and Stats

At the end of solving, programs print:

- `unsolved`: remaining unresolved cells
- `iterations`: logic iteration count
- `elapsed time: <seconds> sec`

## Notes

- PUZ-PRE v3 Shakashaka puzzles can be found at https://puzz.link/db/?type=shakashaka.
- `logical_solver.c` may finish with unresolved cells if pure logic is insufficient.
- `solver.c` continues with backtracking to find a complete valid solution when needed.
