## Description

<!-- Briefly describe what this PR does. For new algorithms, name it and state its type (exact / heuristic / meta-heuristic). -->

## Type of Change

- [ ] Bug fix
- [ ] New algorithm
- [ ] New graph / dataset
- [ ] Performance improvement (existing algorithm)
- [ ] Refactor / code quality
- [ ] Documentation update
- [ ] CI/CD change
- [ ] Other: <!-- describe -->

## Algorithm Details *(fill in if adding a new algorithm)*

| Field | Value |
|---|---|
| Algorithm name | |
| Type (exact / heuristic / meta-heuristic) | |
| Worst-case time complexity | |
| Where registered in `TSPTest.c` | |
| Header prototype added to `TravelingSalesmanProblem.h` | Yes / No |
| Source / Reference | if applicable |

## Graph Details *(fill in if adding a new graph)*

| Field | Value |
|---|---|
| Graph name / factory function | |
| N (number of vertices) | |
| Known optimal cost passed to test driver | |
| Source / reference | |

## Testing

- [ ] `make` builds without errors.
- [ ] `make CFLAGS="-Wall -Wextra -Werror -O3 -Iheaders"` passes with zero warnings.
- [ ] `./TSP_COMPARISON 3` runs successfully.
- [ ] `make runvc N=1` — Valgrind shows no leaks.
- [ ] Tested on the new graph/algorithm specifically (output looks correct).

## Checklist

- [ ] New algorithm / function is documented with a comment block in the `.c` file.
- [ ] Prototype added to the correct header (and to `TravelingSalesmanProblem.h` if public).
- [ ] Algorithm table in `README.md` updated (if new algorithm added).
- [ ] `CHANGELOG.md` updated under `[Unreleased]`.
- [ ] No new `TODO` / `FIXME` left unresolved.
- [ ] No unnecessary `printf` debug output left in code.

## Related Issues

Closes #<!-- issue number, if applicable -->