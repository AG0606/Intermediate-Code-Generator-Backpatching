# Research Manuscript: Intermediate Code Generator for Control Flow Statements

This directory contains the formal IEEE conference paper draft for the project, formatted in strict compliance with the **IEEE Transactions / Conference (IEEEtran)** two-column standard.

---

## Deliverables

| File | Description | Length | Format |
| :--- | :--- | :---: | :--- |
| [`IEEE_Paper_Draft.pdf`](./IEEE_Paper_Draft.pdf) | **Compiled Camera-Ready Conference Paper** | Exactly 6 Pages | PDF (Type 1 Fonts) |
| [`IEEE_Paper_Draft.tex`](./IEEE_Paper_Draft.tex) | **LaTeX Source Document** | 760+ Lines | LaTeX (`IEEEtran.cls`) |
| [`MANUSCRIPT.md`](./MANUSCRIPT.md) | **Markdown Manuscript Version** | Complete Text | GitHub Flavored Markdown |

---

## Paper Specifications

* **Document Class:** `\documentclass[conference]{IEEEtran}` (Standard 10pt, 2-column conference format).
* **Page Budget:** Strictly **6 pages** with balanced final columns on Page 6 (within 8.6 pt).
* **Typography:** IEEE standard `\footnotesize` (8 pt) across all tables; zero overfull `\hbox` warnings.
* **Citations:** 15 peer-reviewed academic citations covering classical compiler literature (Dragon Book, Muchnick, Allen, Cytron et al., DeRemer & Pennello) and contemporary 2021–2024 production architectures (V8 Liftoff CACM 2023, WebAssembly PLDI 2017, MLIR CGO 2021, FAIR ICSE 2024, Xdead ICSE-NIER 2024).
* **Authors:** Strictly Name and Registration Number (`Ashutosh Gupta - 24BCE2983`, `Dhruv Mathur - 24BCE2504`, `Somya Raj Singh Dalawat - 24BCE3001`).

---

## Compiling the LaTeX Source

To compile the LaTeX source into PDF on any system with a standard TeX Live / MiKTeX distribution:

```bash
# Directly from this directory:
pdflatex -interaction=nonstopmode IEEE_Paper_Draft.tex
pdflatex -interaction=nonstopmode IEEE_Paper_Draft.tex
```

*(Two passes ensure all cross-references `\ref` and citations `\cite` are resolved without warnings).*
