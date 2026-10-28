# Sample Compiler Outputs & Visualizations

This directory contains pre-generated sample compilation artifacts and Control Flow Graph (CFG) visualizations produced by the Intermediate Code Generator (`icg`).

---

## Files

| File | Type | Description |
| :--- | :---: | :--- |
| [`cfg.dot`](./cfg.dot) | Graphviz DOT | Raw synthesized Control Flow Graph with basic blocks, 3AC instructions, and flow-typed edges. Unreachable dead blocks are highlighted in red (`[DEAD]`). |
| [`cfg.html`](./cfg.html) | Interactive HTML | Standalone, interactive browser-based visualization rendered with Mermaid.js. Open in any modern web browser to zoom, pan, and inspect blocks and typed edges. |
| [`cfg_optimized.dot`](./cfg_optimized.dot) | Graphviz DOT | Post-optimization Control Flow Graph after applying 4-pass optimization (constant folding, jump chaining, and dead-code elimination with address remapping). |
| [`cfg_optimized.html`](./cfg_optimized.html) | Interactive HTML | Interactive browser-based visualization of the optimized CFG, demonstrating compact control flow with dead blocks pruned and branch targets safely re-indexed. |

---

## Regenerating Outputs

You can regenerate these files from any input program using the compiler:

```bash
# Generate unoptimized CFG (DOT + HTML):
./icg --cfg tests/test8_deep_nesting.txt

# Generate optimized CFG (DOT + HTML):
./icg -O tests/test8_deep_nesting.txt

# Generate all artifacts (table, symbol table, CFG, JSON):
./icg --all tests/test8_deep_nesting.txt
```
