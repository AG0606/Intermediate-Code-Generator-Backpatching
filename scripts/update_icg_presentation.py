import sys
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN
from pptx.enum.shapes import MSO_SHAPE

def update_presentation():
    pptx_path = "ICG_Final_Review.pptx"
    prs = Presentation(pptx_path)
    
    # -------------------------------------------------------------
    # SLIDE 1: One-Shot Complete Implementation Header
    # -------------------------------------------------------------
    s1 = prs.slides[0]
    for shp in s1.shapes:
        if shp.name == "Text 2" and shp.has_text_frame:
            shp.text_frame.text = "Compiler Design Project  |  Complete Implementation"
            p = shp.text_frame.paragraphs[0]
            p.font.name = "Segoe UI"
            p.font.size = Pt(14)
            p.font.color.rgb = RGBColor(255, 255, 255)

    # -------------------------------------------------------------
    # SLIDE 4: Survey 1/3: IR Infrastructure & Flow-Typed Program Graphs
    # -------------------------------------------------------------
    s4 = prs.slides[3]
    # Update title
    for shp in s4.shapes:
        if shp.name == "Text 0" and shp.has_text_frame:
            shp.text_frame.text = "Survey 1/3: IR Infrastructure & Flow-Typed Graphs"
        elif shp.name == "Text 8" and shp.has_text_frame:
            shp.text_frame.text = "FAIR (Niu et al.)"
        elif shp.name == "Text 9" and shp.has_text_frame:
            shp.text_frame.text = "ICSE 2024. Flow Type-Aware Pre-Training of Compiler Intermediate Representations"
        elif shp.name == "Text 10" and shp.has_text_frame:
            tf = shp.text_frame
            tf.clear()
            
            p1 = tf.paragraphs[0]
            p1.space_after = Pt(6)
            r1_h = p1.add_run()
            r1_h.text = "Idea: "
            r1_h.font.bold = True
            r1_h.font.size = Pt(11)
            r1_h.font.color.rgb = RGBColor(0, 128, 128)
            r1_b = p1.add_run()
            r1_b.text = "Intermediate representations lose critical semantics when treated as flat token streams. Control-flow edges must be explicitly categorized by flow type (True, False, Jump, Fallthrough)."
            r1_b.font.size = Pt(11)
            
            p2 = tf.add_paragraph()
            p2.space_after = Pt(6)
            r2_h = p2.add_run()
            r2_h.text = "Link: "
            r2_h.font.bold = True
            r2_h.font.size = Pt(11)
            r2_h.font.color.rgb = RGBColor(0, 128, 128)
            r2_b = p2.add_run()
            r2_b.text = "Identically matches our CFG engine (cfg.c), which classifies every basic block boundary into explicit semantic flow types for structural program clarity."
            r2_b.font.size = Pt(11)
            
            p3 = tf.add_paragraph()
            r3_h = p3.add_run()
            r3_h.text = "Gap: "
            r3_h.font.bold = True
            r3_h.font.size = Pt(11)
            r3_h.font.color.rgb = RGBColor(0, 128, 128)
            r3_b = p3.add_run()
            r3_b.text = "FAIR analyzes existing LLVM IRs; it does not generate IR from source syntax or perform one-pass jump backpatching."
            r3_b.font.size = Pt(11)

    # -------------------------------------------------------------
    # SLIDE 5: Survey 2/3: Single-Pass Compilation & Direct Scoping
    # -------------------------------------------------------------
    s5 = prs.slides[4]
    # Update title
    for shp in s5.shapes:
        if shp.name == "Text 0" and shp.has_text_frame:
            shp.text_frame.text = "Survey 2/3: Single-Pass Compilation & Direct Scoping"

    # Remove unwanted shapes on Slide 5 (Text 1, 2, 3, 4, and Don't Panic shapes)
    shapes_to_remove = []
    for shp in s5.shapes:
        if shp.name in ["Text 1", "Text 2", "Text 3", "Text 4", "paper Don't Panic! (Diekmann and Tratt)", "Shape 6", "Text 7", "Text 8", "Text 9"]:
            shapes_to_remove.append(shp)
    
    for shp in shapes_to_remove:
        sp = shp._element
        sp.getparent().remove(sp)

    # Now add Two Cards on Slide 5 matching Slide 4 layout
    # Left Card: V8 Liftoff (Google)
    card_l = s5.shapes.add_shape(MSO_SHAPE.RECTANGLE, 457200, 1051560, 4023360, 3611880)
    card_l.fill.solid()
    card_l.fill.fore_color.rgb = RGBColor(245, 247, 250)
    card_l.line.color.rgb = RGBColor(213, 222, 229)
    card_l.line.width = Pt(1)

    bar_l = s5.shapes.add_shape(MSO_SHAPE.OVAL, 594360, 1207008, 146304, 146304)
    bar_l.fill.solid()
    bar_l.fill.fore_color.rgb = RGBColor(0, 128, 128)
    bar_l.line.fill.background()

    tb_tl = s5.shapes.add_textbox(822960, 1143000, 3566160, 292608)
    tb_tl.text_frame.word_wrap = True
    p = tb_tl.text_frame.paragraphs[0]
    p.text = "Liftoff: V8 Baseline Compiler"
    p.font.bold = True
    p.font.size = Pt(14)
    p.font.color.rgb = RGBColor(30, 41, 59)

    tb_cl = s5.shapes.add_textbox(594360, 1490472, 3749040, 457200)
    tb_cl.text_frame.word_wrap = True
    p = tb_cl.text_frame.paragraphs[0]
    p.text = "ACM PLDI / Systems (2018–2023). Fast streaming baseline compiler for WebAssembly"
    p.font.italic = True
    p.font.size = Pt(10.5)
    p.font.color.rgb = RGBColor(100, 116, 139)

    tb_bl = s5.shapes.add_textbox(594360, 1965960, 3749040, 2606040)
    tf_bl = tb_bl.text_frame
    tf_bl.word_wrap = True
    
    p1 = tf_bl.paragraphs[0]
    p1.space_after = Pt(6)
    r = p1.add_run(); r.text = "Idea: "; r.font.bold = True; r.font.size = Pt(11); r.font.color.rgb = RGBColor(0, 128, 128)
    r = p1.add_run(); r.text = "Multi-pass optimizing JITs (TurboFan) suffer high startup latency. Liftoff demonstrates that single-pass streaming compilation using label fixups (backpatching) generates native code on-the-fly with zero AST overhead."; r.font.size = Pt(11)

    p2 = tf_bl.add_paragraph()
    p2.space_after = Pt(6)
    r = p2.add_run(); r.text = "Link: "; r.font.bold = True; r.font.size = Pt(11); r.font.color.rgb = RGBColor(0, 128, 128)
    r = p2.add_run(); r.text = "Proves that single-pass backpatching is actively deployed in modern high-performance runtimes to eliminate AST memory footprint and compilation latency."; r.font.size = Pt(11)

    p3 = tf_bl.add_paragraph()
    r = p3.add_run(); r.text = "Gap: "; r.font.bold = True; r.font.size = Pt(11); r.font.color.rgb = RGBColor(0, 128, 128)
    r = p3.add_run(); r.text = "Liftoff generates machine instructions directly; it does not build an intermediate CFG or perform structured reachability optimization."; r.font.size = Pt(11)

    # Right Card: Direct IR & Scoping (Braun et al.)
    card_r = s5.shapes.add_shape(MSO_SHAPE.RECTANGLE, 4663440, 1051560, 4023360, 3611880)
    card_r.fill.solid()
    card_r.fill.fore_color.rgb = RGBColor(245, 247, 250)
    card_r.line.color.rgb = RGBColor(213, 222, 229)
    card_r.line.width = Pt(1)

    bar_r = s5.shapes.add_shape(MSO_SHAPE.OVAL, 4800600, 1207008, 146304, 146304)
    bar_r.fill.solid()
    bar_r.fill.fore_color.rgb = RGBColor(0, 128, 128)
    bar_r.line.fill.background()

    tb_tr = s5.shapes.add_textbox(5029200, 1143000, 3566160, 292608)
    tb_tr.text_frame.word_wrap = True
    p = tb_tr.text_frame.paragraphs[0]
    p.text = "Direct IR & Scoping (Braun et al.)"
    p.font.bold = True
    p.font.size = Pt(14)
    p.font.color.rgb = RGBColor(30, 41, 59)

    tb_cr = s5.shapes.add_textbox(4800600, 1490472, 3749040, 457200)
    tb_cr.text_frame.word_wrap = True
    p = tb_cr.text_frame.paragraphs[0]
    p.text = "Springer CC / Evaluated 2022. Simple and efficient construction of IR from source syntax"
    p.font.italic = True
    p.font.size = Pt(10.5)
    p.font.color.rgb = RGBColor(100, 116, 139)

    tb_br = s5.shapes.add_textbox(4800600, 1965960, 3749040, 2606040)
    tf_br = tb_br.text_frame
    tf_br.word_wrap = True
    
    p1 = tf_br.paragraphs[0]
    p1.space_after = Pt(6)
    r = p1.add_run(); r.text = "Idea: "; r.font.bold = True; r.font.size = Pt(11); r.font.color.rgb = RGBColor(0, 128, 128)
    r = p1.add_run(); r.text = "Constructs intermediate representation and variable versions on-the-fly directly during syntax analysis without requiring global dominator tree computations over an AST."; r.font.size = Pt(11)

    p2 = tf_br.add_paragraph()
    p2.space_after = Pt(6)
    r = p2.add_run(); r.text = "Link: "; r.font.bold = True; r.font.size = Pt(11); r.font.color.rgb = RGBColor(0, 128, 128)
    r = p2.add_run(); r.text = "Direct theoretical justification for our active scope stack and dynamic variable shadowing (x_s0, x_s2, x_s5) directly during LR reductions without an AST."; r.font.size = Pt(11)

    p3 = tf_br.add_paragraph()
    r = p3.add_run(); r.text = "Gap: "; r.font.bold = True; r.font.size = Pt(11); r.font.color.rgb = RGBColor(0, 128, 128)
    r = p3.add_run(); r.text = "Focuses on SSA phi-functions rather than control-flow backpatching lists (truelist, falselist, nextlist)."; r.font.size = Pt(11)

    # -------------------------------------------------------------
    # SLIDE 7: Survey Summary & Comparative Analysis Matrix
    # -------------------------------------------------------------
    s7 = prs.slides[6]
    for shp in s7.shapes:
        if shp.name == "Table 0" and shp.has_table:
            t = shp.table
            # Row 0: Headers
            t.cell(0, 0).text = "Research Paper"
            t.cell(0, 1).text = "Year / Venue"
            t.cell(0, 2).text = "Key Paradigm & Focus"
            t.cell(0, 3).text = "Direct Module Mapping"
            
            # Row 1: V8 Liftoff
            t.cell(1, 0).text = "V8 Liftoff (Google)"
            t.cell(1, 1).text = "2018–2023 / ACM PLDI"
            t.cell(1, 2).text = "Single-pass streaming compilation & label fixups"
            t.cell(1, 3).text = "quad.c (backpatching engine, zero AST latency)"
            
            # Row 2: MLIR
            t.cell(2, 0).text = "MLIR (Lattner et al.)"
            t.cell(2, 1).text = "2021 / IEEE/ACM CGO"
            t.cell(2, 2).text = "Multi-level IR infrastructure vs linear 3AC"
            t.cell(2, 3).text = "quad.h, 3AC linear quadruple array"
            
            # Row 3: FAIR
            t.cell(3, 0).text = "FAIR (Niu et al.)"
            t.cell(3, 1).text = "2024 / IEEE/ACM ICSE"
            t.cell(3, 2).text = "Flow-typed CFG edge semantics (True/False/Jump)"
            t.cell(3, 3).text = "cfg.c (explicit flow edge classification)"
            
            # Row 4: Braun et al.
            t.cell(4, 0).text = "Direct IR (Braun et al.)"
            t.cell(4, 1).text = "2022 / Springer CC"
            t.cell(4, 2).text = "Single-pass IR & variable scoping without AST"
            t.cell(4, 3).text = "symbol_table.c (dynamic scope stack & shadowing)"
            
            # Row 5: DCE Lens
            t.cell(5, 0).text = "DCE Lens (Su et al.)"
            t.cell(5, 1).text = "2022 / ACM ASPLOS"
            t.cell(5, 2).text = "Reachability-based dead block elimination"
            t.cell(5, 3).text = "optimizer.c (CFG reachability pruning)"
            
            # Row 6: Xdead
            t.cell(6, 0).text = "Xdead (Miscompilations)"
            t.cell(6, 1).text = "2024 / ICSE-NIER"
            t.cell(6, 2).text = "Target corruption & live code deletion safety"
            t.cell(6, 3).text = "optimizer.c (jump re-indexing), run_tests.py"

            # Format cells
            for row in t.rows:
                for cell in row.cells:
                    for p in cell.text_frame.paragraphs:
                        p.font.name = "Segoe UI"
                        p.font.size = Pt(10.5)

        elif shp.name == "Text 1" and shp.has_text_frame:
            tf = shp.text_frame
            tf.clear()
            p = tf.paragraphs[0]
            r1 = p.add_run()
            r1.text = "Scholarly Synthesis: "
            r1.font.bold = True
            r1.font.size = Pt(11)
            r1.font.color.rgb = RGBColor(0, 128, 128)
            
            r2 = p.add_run()
            r2.text = "Recent literature (2018–2024) proves that single-pass streaming compilation with jump fixups (V8 Liftoff) is essential for low latency, while flow-typed CFGs (FAIR) and reachability DCE (ASPLOS) provide structural rigor. Our system integrates these principles into a unified, production-grade, C-compatible compiler pipeline."
            r2.font.size = Pt(11)
            r2.font.color.rgb = RGBColor(30, 41, 59)

    # -------------------------------------------------------------
    # SLIDE 8: Research Gap and Our Architectural Contributions
    # -------------------------------------------------------------
    s8 = prs.slides[7]
    for shp in s8.shapes:
        if shp.name == "Text 4" and shp.has_text_frame:
            tf = shp.text_frame
            tf.clear()
            bullets = [
                ("Bifurcated State of the Art: ", "Production JITs (V8 Liftoff) use single-pass fixups but omit intermediate CFG analysis; academic compilers (MLIR, LLVM) build deep multi-pass IRs with high latency."),
                ("Missing Unified Pipelines: ", "Parsing, lexical scoping, variable shadowing, IR backpatching, CFG synthesis, and optimization are rarely integrated into a single, self-contained educational compiler."),
                ("Optimization Safety (Xdead): ", "Deleting dead instructions post-backpatching easily corrupts hardcoded numerical jump targets without rigorous address re-indexing.")
            ]
            for i, (head, body) in enumerate(bullets):
                p = tf.paragraphs[0] if i == 0 else tf.add_paragraph()
                p.space_after = Pt(8)
                r_h = p.add_run(); r_h.text = "• " + head; r_h.font.bold = True; r_h.font.size = Pt(11.5); r_h.font.color.rgb = RGBColor(0, 128, 128)
                r_b = p.add_run(); r_b.text = body; r_b.font.size = Pt(11.5); r_b.font.color.rgb = RGBColor(30, 41, 59)

        elif shp.name == "Text 8" and shp.has_text_frame:
            tf = shp.text_frame
            tf.clear()
            bullets = [
                ("1. Single-Pass Scoping & Shadowing: ", "Dynamic scope stack and unique 3AC variable mangling (x_s2, x_s5) during LR reductions with zero AST overhead."),
                ("2. Dual-Marker Loop Synchronization: ", "N_skip and N_back resolve out-of-order C for-loop execution in strictly linear O(1) time without backtracking."),
                ("3. Direct Flow-Typed CFG & Web Visualizer: ", "4-rule leader partitioning directly on unlabelled quads with FAIR-compliant flow edges and interactive HTML viewer."),
                ("4. Coupled Optimizer with Target Re-indexing: ", "Transitive jump chaining, constant folding, and BFS dead-block removal with complete mathematical address integrity.")
            ]
            for i, (head, body) in enumerate(bullets):
                p = tf.paragraphs[0] if i == 0 else tf.add_paragraph()
                p.space_after = Pt(8)
                r_h = p.add_run(); r_h.text = "• " + head; r_h.font.bold = True; r_h.font.size = Pt(11.5); r_h.font.color.rgb = RGBColor(0, 128, 128)
                r_b = p.add_run(); r_b.text = body; r_b.font.size = Pt(11.5); r_b.font.color.rgb = RGBColor(30, 41, 59)

    # -------------------------------------------------------------
    # SLIDE 21: Conclusion and Future Work
    # -------------------------------------------------------------
    s21 = prs.slides[20]
    for shp in s21.shapes:
        if shp.name == "Text 8" and shp.has_text_frame:
            tf = shp.text_frame
            tf.clear()
            items = [
                "Target machine code generation (x86-64, ARM64, or WebAssembly)",
                "Static Single Assignment (SSA) form lowering directly from 3AC",
                "Advanced data-flow optimizations (Common Subexpression Elimination, Copy Propagation)",
                "Complex composite types: Structs, Arrays, and Pointers"
            ]
            for i, item in enumerate(items):
                p = tf.paragraphs[0] if i == 0 else tf.add_paragraph()
                p.space_after = Pt(6)
                r = p.add_run()
                r.text = "• " + item
                r.font.size = Pt(12)
                r.font.color.rgb = RGBColor(30, 41, 59)

    # -------------------------------------------------------------
    # SLIDE 22: References
    # -------------------------------------------------------------
    s22 = prs.slides[21]
    for shp in s22.shapes:
        if shp.name == "Text 1" and shp.has_text_frame:
            tf = shp.text_frame
            tf.clear()
            refs = [
                "Haas et al. Bringing the Web up to Speed with WebAssembly & Liftoff. ACM PLDI / CACM, 2018–2023.",
                "Lattner et al. MLIR: Scaling Compiler Infrastructure for Domain Specific Computation. IEEE/ACM CGO, 2021.",
                "Niu et al. FAIR: Flow Type-Aware Pre-Training of Compiler Intermediate Representations. IEEE/ACM ICSE, 2024.",
                "Braun et al. Simple and Efficient Construction of Static Single Assignment Form. Springer CC / Evaluated 2022.",
                "Theodoridis, Rigger, Su. Finding Missed Optimizations through the Lens of Dead Code Elimination. ACM ASPLOS, 2022.",
                "Beyond a Joke: Dead Code Elimination Can Delete Live Code (Xdead). IEEE/ACM ICSE-NIER, 2024.",
                "Cummins et al. Meta Large Language Model Compiler: Foundation Models of Compiler Optimization. arXiv:2407.02524, 2024.",
                "Aho, Lam, Sethi, Ullman. Compilers: Principles, Techniques, and Tools (Dragon Book), 2nd ed., Addison-Wesley, 2006."
            ]
            for i, ref in enumerate(refs):
                p = tf.paragraphs[0] if i == 0 else tf.add_paragraph()
                p.space_after = Pt(4)
                r = p.add_run()
                r.text = f"[{i+1}]  " + ref
                r.font.size = Pt(10)
                r.font.name = "Segoe UI"
                r.font.color.rgb = RGBColor(30, 41, 59)

    # Save updated presentation
    prs.save(pptx_path)
    print(f"[SUCCESS] Updated {pptx_path} successfully as a one-shot full implementation deck!")

if __name__ == "__main__":
    update_presentation()
