# Project Presentation: Intermediate Code Generator for Control-Flow Statements

This directory contains the final slide decks for the project review, defense, and technical evaluation.

---

## Deliverables

| File | Description | Slides | Format |
| :--- | :--- | :---: | :--- |
| [`Final_PDF_PPT_CD.pdf`](./Final_PDF_PPT_CD.pdf) | **Primary Final Presentation Slide Deck** (Full 23-Slide Comprehensive Presentation) | 23 | PDF (Vector) |
| [`Final_PDF_PPT_CD.pptx`](./Final_PDF_PPT_CD.pptx) | **Editable Presentation Source** (PowerPoint Presentation) | 23 | PPTX |
| [`archive/`](./archive/) | Historical slide deck revisions and review backups | 20–22 | PDF / PPTX |

---

## Slide Deck Overview (23 Slides)

1. **Slide 1: Title & Team Metadata**  
   * Project Title, Course Code, Team Members (`24BCE2983`, `24BCE2504`, `24BCE3001`), and Official GitHub Repository Link.
2. **Slide 2: Executive Summary & Motivation**  
   * AST Memory Overhead vs. Streaming Compilation.
3. **Slide 3: The Forward-Jump Problem**  
   * Why branch targets are unknown during single-pass bottom-up parsing.
4. **Slide 4: Project Objectives & Core Deliverables**  
   * Scope preservation, loop ordering, CFG generation, and safe optimization.
5. **Slide 5–7: Comprehensive Literature Survey**  
   * Survey 1/3: Linear IRs and MLIR (*Lattner et al., CGO 2021*).  
   * Survey 2/3: Flow-Typed CFGs (*FAIR, Niu et al., ICSE 2024*).  
   * Survey 3/3: Streaming Code Generation (*Google V8 Liftoff, CACM 2023*).
6. **Slide 8: System Architecture**  
   * End-to-end pipeline diagram (Flex $\to$ Bison $\to$ Quad Engine $\to$ CFG $\to$ Optimizer).
7. **Slide 9: Three-Address Code (3AC) Quadruple Formalism**  
   * Quadruple structure $\langle \mathit{op}, \mathit{arg1}, \mathit{arg2}, \mathit{result} \rangle$ and operations.
8. **Slide 10: Backpatching Algebra**  
   * Formal definition of `makelist`, `merge`, and `backpatch`.
9. **Slide 11: Scope-Aware Symbol Table & Dynamic Mangling**  
   * Resolving nested block shadowing without an AST ($x \to x\_s0, x\_s2$).
10. **Slide 12: Dual-Marker Linear Synchronization for `for` Loops**  
    * Solving parse order vs. execution order using $N_{\mathrm{skip}}$ and $N_{\mathrm{back}}$.
11. **Slide 13–15: Control Flow Lowering Demonstrations**  
    * `if-else` lowering, short-circuit Boolean evaluation (`&&`, `||`), and nested `while` loops.
12. **Slide 16: Loop Context Stack & Break/Continue Isolation**  
    * Stack isolation of jump targets across nested loops and switches.
13. **Slide 17: Leader Identification & Flow-Typed CFG Synthesis**  
    * 4-rule leader algorithm and 4-type edge classification (True, False, Jump, Fallthrough).
14. **Slide 18: Four-Pass Optimization Engine**  
    * Constant folding, jump chaining, and dead-code elimination.
15. **Slide 19: Address Displacement Vector Remapping**  
    * $R[i] = i - \sum \mathbb{I}(\text{dead})$ preserving jump-target integrity.
16. **Slide 20: Comprehensive Verification Matrix (14 Test Suites)**  
    * 100.0% pass rate across positive and negative test cases.
17. **Slide 21: Quantitative Evaluation & Asymptotic Complexity**  
    * 25% quad reduction and $O(N)$ time / $O(D)$ space comparison.
18. **Slide 22: Live Demonstration & Visual Artifacts**  
    * Interactive Mermaid HTML visualizer and Graphviz DOT output.
19. **Slide 23: Conclusion & Future Research Directions**  
    * LLVM-IR lowering, SSA $\phi$-construction, and WebAssembly target.

---

## How to Present

* For conference or project review presentations, use [`Final_PDF_PPT_CD.pdf`](./Final_PDF_PPT_CD.pdf) in full-screen presentation mode (`Ctrl+L` in Acrobat Reader / Evince).
* For editable changes, open [`Final_PDF_PPT_CD.pptx`](./Final_PDF_PPT_CD.pptx) in Microsoft PowerPoint or LibreOffice Impress.
