# Security Policy

## Scope

The TSP Solver is an academic, educational project. It reads no network data, has no authentication, and processes only local `.dot` files and hardcoded graph data. The attack surface is minimal.

The most relevant security concerns are:

- **Memory safety** — buffer overflows, use-after-free, or dangling pointers in C code that could be triggered by crafted inputs.
- **Integer overflow** — in graph construction or tour indexing with very large N.

## Supported Versions

| Version | Supported |
|---------|-----------|
| `main` branch | YES |
| Older tags | NO |

Only the current `main` branch receives fixes.

## Reporting a Vulnerability

If you discover a security or memory-safety issue:

1. **Do not open a public issue.**
2. **Email** the maintainer directly: nelsonramos@ua.pt.
3. Include:
   - A description of the vulnerability.
   - Steps to reproduce (graph, N, exact command).
   - Valgrind output if applicable.

Optionally, you can use **GitHub Security Advisories:** [Report a vulnerability](https://github.com/nelsonramosua/TSP/security/advisories/new).
 
You will receive a response within 7 days. Once a fix is merged, you are welcome to disclose publicly.

## Memory Safety Baseline

This project is regularly tested with Valgrind:

```bash
make runvc N=1
```

All merges to `main` must pass `valgrind --leak-check=full --error-exitcode=1` on at least one graph.