#include "cfg.h"

ControlFlowGraph current_cfg;

static void format_quad_str(char *buf, size_t size, int q, bool html_mode) {
    const char *less = html_mode ? "&lt;" : "<";
    const char *greater = html_mode ? "&gt;" : ">";
    const char *term = html_mode ? "<br/>" : "";

    if (strcmp(quads[q].op, "=") == 0) {
        snprintf(buf, size, "%d: %s = %s%s", q, quads[q].result, quads[q].arg1, term);
    }
    else if (strcmp(quads[q].op, "goto") == 0) {
        snprintf(buf, size, "%d: goto %s%s", q, quads[q].result, term);
    }
    else if (strcmp(quads[q].op, "if") == 0) {
        /* Escape angle brackets in condition if html */
        char cond[128];
        if (html_mode && (strstr(quads[q].arg1, "<") || strstr(quads[q].arg1, ">"))) {
            char *in = quads[q].arg1;
            char *out = cond;
            while (*in && (size_t)(out - cond) < sizeof(cond) - 8) {
                if (*in == '<') { strcpy(out, "&lt;"); out += 4; }
                else if (*in == '>') { strcpy(out, "&gt;"); out += 4; }
                else { *out++ = *in; }
                in++;
            }
            *out = '\0';
            snprintf(buf, size, "%d: if %s goto %s%s", q, cond, quads[q].result, term);
        } else {
            snprintf(buf, size, "%d: if %s goto %s%s", q, quads[q].arg1, quads[q].result, term);
        }
    }
    else if (strcmp(quads[q].op, "end") == 0) {
        snprintf(buf, size, "%d: end%s", q, term);
    }
    else if (strcmp(quads[q].op, "func_begin") == 0) {
        snprintf(buf, size, "%d: func_begin %s%s", q, quads[q].arg1, term);
    }
    else if (strcmp(quads[q].op, "func_end") == 0) {
        snprintf(buf, size, "%d: func_end %s%s", q, quads[q].arg1, term);
    }
    else if (strcmp(quads[q].op, "return") == 0) {
        if (strcmp(quads[q].arg1, "-") == 0) {
            snprintf(buf, size, "%d: return%s", q, term);
        } else {
            snprintf(buf, size, "%d: return %s%s", q, quads[q].arg1, term);
        }
    }
    else if (strcmp(quads[q].op, "param") == 0) {
        snprintf(buf, size, "%d: param %s%s", q, quads[q].arg1, term);
    }
    else if (strcmp(quads[q].op, "call") == 0) {
        if (strcmp(quads[q].result, "-") == 0) {
            snprintf(buf, size, "%d: call %s, %s%s", q, quads[q].arg1, quads[q].arg2, term);
        } else {
            snprintf(buf, size, "%d: %s = call %s, %s%s", q, quads[q].result, quads[q].arg1, quads[q].arg2, term);
        }
    }
    else if (strcmp(quads[q].op, "uminus") == 0) {
        snprintf(buf, size, "%d: %s = -%s%s", q, quads[q].result, quads[q].arg1, term);
    }
    else if (strcmp(quads[q].op, "-") == 0) {
        if (strcmp(quads[q].arg2, "-") != 0) {
            snprintf(buf, size, "%d: %s = %s - %s%s", q, quads[q].result, quads[q].arg1, quads[q].arg2, term);
        } else {
            snprintf(buf, size, "%d: %s%s", q, quads[q].result, term);
        }
    }
    else {
        /* Normal binary operation */
        const char *op_display = quads[q].op;
        if (html_mode) {
            if (strcmp(quads[q].op, "<") == 0) op_display = less;
            else if (strcmp(quads[q].op, ">") == 0) op_display = greater;
            else if (strcmp(quads[q].op, "<=") == 0) op_display = "&lt;=";
            else if (strcmp(quads[q].op, ">=") == 0) op_display = "&gt;=";
        }
        snprintf(buf, size, "%d: %s = %s %s %s%s",
                 q, quads[q].result, quads[q].arg1, op_display, quads[q].arg2, term);
    }
}

static void add_cfg_edge(BasicBlock *bb, int target_block, EdgeType type, const char *label, const char *color) {
    CFGEdge *curr = bb->successors;
    while (curr) {
        if (curr->target_block == target_block && curr->type == type) {
            return;
        }
        curr = curr->next;
    }

    CFGEdge *edge = (CFGEdge*)malloc(sizeof(CFGEdge));
    edge->target_block = target_block;
    edge->type = type;
    strncpy(edge->label, label ? label : "", sizeof(edge->label) - 1);
    edge->label[sizeof(edge->label) - 1] = '\0';
    strncpy(edge->color, color ? color : "black", sizeof(edge->color) - 1);
    edge->color[sizeof(edge->color) - 1] = '\0';
    edge->next = bb->successors;
    bb->successors = edge;
}

void cfg_build(void) {
    if (nextquad == 0) {
        current_cfg.num_blocks = 0;
        return;
    }

    bool is_leader[MAX_QUADS] = { false };
    int quad_to_block[MAX_QUADS];
    for (int i = 0; i < nextquad; i++) {
        quad_to_block[i] = -1;
    }

    /* Rule 1: The first instruction is a leader */
    is_leader[0] = true;

    /* Rule 2 & 3: Jump targets and instructions immediately following jumps/returns are leaders */
    for (int i = 0; i < nextquad; i++) {
        if (strcmp(quads[i].op, "goto") == 0) {
            int target = atoi(quads[i].result);
            if (target >= 0 && target < nextquad) {
                is_leader[target] = true;
            }
            if (i + 1 < nextquad) {
                is_leader[i + 1] = true;
            }
        }
        else if (strcmp(quads[i].op, "if") == 0) {
            int target = atoi(quads[i].result);
            if (target >= 0 && target < nextquad) {
                is_leader[target] = true;
            }
            if (i + 1 < nextquad) {
                is_leader[i + 1] = true;
            }
        }
        else if (strcmp(quads[i].op, "end") == 0 || strcmp(quads[i].op, "return") == 0) {
            if (i + 1 < nextquad) {
                is_leader[i + 1] = true;
            }
        }
        else if (strcmp(quads[i].op, "func_begin") == 0) {
            is_leader[i] = true;
        }
    }

    /* Form basic blocks */
    current_cfg.num_blocks = 0;
    int curr_start = 0;

    for (int i = 0; i < nextquad; i++) {
        if (i > 0 && is_leader[i]) {
            int b = current_cfg.num_blocks++;
            current_cfg.blocks[b].id = b;
            current_cfg.blocks[b].start_quad = curr_start;
            current_cfg.blocks[b].end_quad = i - 1;
            current_cfg.blocks[b].is_leader = true;
            current_cfg.blocks[b].successors = NULL;
            current_cfg.blocks[b].predecessors = NULL;
            current_cfg.blocks[b].is_reachable = false;
            for (int q = curr_start; q < i; q++) {
                quad_to_block[q] = b;
            }
            curr_start = i;
        }
    }

    /* Last block */
    if (curr_start < nextquad) {
        int b = current_cfg.num_blocks++;
        current_cfg.blocks[b].id = b;
        current_cfg.blocks[b].start_quad = curr_start;
        current_cfg.blocks[b].end_quad = nextquad - 1;
        current_cfg.blocks[b].is_leader = true;
        current_cfg.blocks[b].successors = NULL;
        current_cfg.blocks[b].predecessors = NULL;
        current_cfg.blocks[b].is_reachable = false;
        for (int q = curr_start; q < nextquad; q++) {
            quad_to_block[q] = b;
        }
    }

    /* Add edges */
    for (int b = 0; b < current_cfg.num_blocks; b++) {
        int last = current_cfg.blocks[b].end_quad;
        if (strcmp(quads[last].op, "goto") == 0) {
            int target = atoi(quads[last].result);
            if (target >= 0 && target < nextquad && quad_to_block[target] != -1) {
                add_cfg_edge(&current_cfg.blocks[b], quad_to_block[target], EDGE_JUMP, "jump", "#f39c12");
            }
        }
        else if (strcmp(quads[last].op, "if") == 0) {
            int target = atoi(quads[last].result);
            if (target >= 0 && target < nextquad && quad_to_block[target] != -1) {
                add_cfg_edge(&current_cfg.blocks[b], quad_to_block[target], EDGE_TRUE, "true", "#27ae60");
            }
            if (last + 1 < nextquad && quad_to_block[last + 1] != -1) {
                add_cfg_edge(&current_cfg.blocks[b], quad_to_block[last + 1], EDGE_FALSE, "false", "#c0392b");
            }
        }
        else if (strcmp(quads[last].op, "end") == 0 || strcmp(quads[last].op, "return") == 0) {
            /* Terminal block */
        }
        else {
            /* Normal fallthrough */
            if (b + 1 < current_cfg.num_blocks) {
                add_cfg_edge(&current_cfg.blocks[b], b + 1, EDGE_FALLTHROUGH, "fallthrough", "#2980b9");
            }
        }
    }

    /* Compute reachability via BFS: initialize with entry Block 0 and all function entry points */
    bool visited[MAX_BLOCKS] = { false };
    int queue[MAX_BLOCKS];
    int head = 0, tail = 0;

    if (current_cfg.num_blocks > 0) {
        for (int b = 0; b < current_cfg.num_blocks; b++) {
            int start = current_cfg.blocks[b].start_quad;
            if (b == 0 || strcmp(quads[start].op, "func_begin") == 0) {
                if (!visited[b]) {
                    visited[b] = true;
                    current_cfg.blocks[b].is_reachable = true;
                    queue[tail++] = b;
                }
            }
        }

        while (head < tail) {
            int curr = queue[head++];
            CFGEdge *e = current_cfg.blocks[curr].successors;
            while (e) {
                if (e->target_block >= 0 && e->target_block < current_cfg.num_blocks) {
                    if (!visited[e->target_block]) {
                        visited[e->target_block] = true;
                        current_cfg.blocks[e->target_block].is_reachable = true;
                        queue[tail++] = e->target_block;
                    }
                }
                e = e->next;
            }
        }
    }
}

void cfg_print_blocks(FILE *fp) {
    fprintf(fp, "\n=========================================================================\n");
    fprintf(fp, "                 BASIC BLOCKS & CONTROL FLOW GRAPH (CFG)                 \n");
    fprintf(fp, "=========================================================================\n");
    char line_buf[256];
    for (int b = 0; b < current_cfg.num_blocks; b++) {
        BasicBlock *bb = &current_cfg.blocks[b];
        fprintf(fp, "Block B%-2d [Quads %2d..%-2d] (Reachable: %s):\n",
                bb->id, bb->start_quad, bb->end_quad, bb->is_reachable ? "YES" : "NO (Dead Code)");
        for (int q = bb->start_quad; q <= bb->end_quad; q++) {
            format_quad_str(line_buf, sizeof(line_buf), q, false);
            fprintf(fp, "    %s\n", line_buf);
        }
        fprintf(fp, "  Successors: ");
        CFGEdge *e = bb->successors;
        if (!e) {
            fprintf(fp, "(None - Exit/Halt)\n");
        } else {
            while (e) {
                fprintf(fp, "-> B%d [%s] ", e->target_block, e->label);
                e = e->next;
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "-------------------------------------------------------------------------\n");
    }
    fprintf(fp, "\n");
}

void cfg_export_dot(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error opening %s for DOT export\n", filename);
        return;
    }
    fprintf(fp, "digraph ControlFlowGraph {\n");
    fprintf(fp, "    node [shape=record, fontname=\"Consolas, monospace\", style=filled];\n");
    fprintf(fp, "    edge [fontname=\"Consolas, monospace\", fontsize=10];\n\n");

    char line_buf[256];
    for (int b = 0; b < current_cfg.num_blocks; b++) {
        BasicBlock *bb = &current_cfg.blocks[b];
        const char *fill = bb->is_reachable ? "#e8f4f8" : "#f8d7da";
        const char *color = bb->is_reachable ? "#3498db" : "#e74c3c";

        fprintf(fp, "    B%d [fillcolor=\"%s\", color=\"%s\", label=\"{B%d (Quads %d..%d)%s|",
                bb->id, fill, color, bb->id, bb->start_quad, bb->end_quad,
                bb->is_reachable ? "" : " [DEAD]");

        for (int q = bb->start_quad; q <= bb->end_quad; q++) {
            format_quad_str(line_buf, sizeof(line_buf), q, false);
            /* Escape quotes or special chars if needed for DOT */
            fprintf(fp, "%s\\l", line_buf);
        }
        fprintf(fp, "}\"];\n");
    }

    fprintf(fp, "\n");
    for (int b = 0; b < current_cfg.num_blocks; b++) {
        BasicBlock *bb = &current_cfg.blocks[b];
        CFGEdge *e = bb->successors;
        while (e) {
            fprintf(fp, "    B%d -> B%d [label=\"%s\", color=\"%s\", fontcolor=\"%s\"];\n",
                    bb->id, e->target_block, e->label, e->color, e->color);
            e = e->next;
        }
    }
    fprintf(fp, "}\n");
    fclose(fp);
}

void cfg_export_html(const char *filename, const char *source_title) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error opening %s for HTML export\n", filename);
        return;
    }

    fprintf(fp, "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n");
    fprintf(fp, "<meta charset=\"UTF-8\">\n");
    fprintf(fp, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    fprintf(fp, "<title>CFG Visualizer - %s</title>\n", source_title ? source_title : "Intermediate Code");
    fprintf(fp, "<script src=\"https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.min.js\"></script>\n");
    fprintf(fp, "<style>\n");
    fprintf(fp, "  body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; background: #0f172a; color: #f8fafc; padding: 24px; }\n");
    fprintf(fp, "  .container { max-width: 1200px; margin: 0 auto; }\n");
    fprintf(fp, "  header { border-bottom: 2px solid #334155; padding-bottom: 16px; margin-bottom: 24px; }\n");
    fprintf(fp, "  h1 { margin: 0 0 8px 0; color: #38bdf8; font-size: 28px; }\n");
    fprintf(fp, "  .badge { display: inline-block; padding: 4px 10px; border-radius: 9999px; font-size: 12px; font-weight: 600; background: #1e293b; color: #94a3b8; margin-right: 8px; }\n");
    fprintf(fp, "  .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 24px; }\n");
    fprintf(fp, "  .card { background: #1e293b; border-radius: 12px; border: 1px solid #334155; padding: 20px; box-shadow: 0 10px 15px -3px rgba(0,0,0,0.3); }\n");
    fprintf(fp, "  .card h2 { margin-top: 0; color: #e2e8f0; font-size: 20px; border-bottom: 1px solid #334155; padding-bottom: 10px; }\n");
    fprintf(fp, "  pre { background: #090d16; padding: 14px; border-radius: 8px; overflow-x: auto; font-family: 'Consolas', monospace; font-size: 14px; color: #a5f3fc; line-height: 1.5; }\n");
    fprintf(fp, "  .mermaid { background: #090d16; padding: 20px; border-radius: 8px; overflow: auto; text-align: center; }\n");
    fprintf(fp, "  .legend { display: flex; gap: 16px; margin-top: 12px; flex-wrap: wrap; }\n");
    fprintf(fp, "  .legend-item { display: flex; align-items: center; gap: 6px; font-size: 13px; color: #cbd5e1; }\n");
    fprintf(fp, "  .dot { width: 12px; height: 12px; border-radius: 50%%; display: inline-block; }\n");
    fprintf(fp, "  table { width: 100%%; border-collapse: collapse; margin-top: 12px; font-size: 13px; }\n");
    fprintf(fp, "  th, td { border: 1px solid #334155; padding: 8px 12px; text-align: left; }\n");
    fprintf(fp, "  th { background: #0f172a; color: #38bdf8; }\n");
    fprintf(fp, "</style>\n</head>\n<body>\n");
    fprintf(fp, "<div class=\"container\">\n");
    fprintf(fp, "  <header>\n");
    fprintf(fp, "    <h1>Control Flow Graph & Basic Blocks</h1>\n");
    fprintf(fp, "    <div><span class=\"badge\">Compiler Design Project</span><span class=\"badge\">Total Quads: %d</span><span class=\"badge\">Basic Blocks: %d</span></div>\n",
            nextquad, current_cfg.num_blocks);
    fprintf(fp, "  </header>\n");

    fprintf(fp, "  <div class=\"card\" style=\"margin-bottom: 24px;\">\n");
    fprintf(fp, "    <h2>Interactive Control Flow Graph</h2>\n");
    fprintf(fp, "    <div class=\"legend\">\n");
    fprintf(fp, "      <div class=\"legend-item\"><span class=\"dot\" style=\"background: #27ae60;\"></span> True Branch</div>\n");
    fprintf(fp, "      <div class=\"legend-item\"><span class=\"dot\" style=\"background: #c0392b;\"></span> False Branch</div>\n");
    fprintf(fp, "      <div class=\"legend-item\"><span class=\"dot\" style=\"background: #f39c12;\"></span> Unconditional Jump</div>\n");
    fprintf(fp, "      <div class=\"legend-item\"><span class=\"dot\" style=\"background: #2980b9;\"></span> Fallthrough</div>\n");
    fprintf(fp, "    </div>\n");
    fprintf(fp, "    <div class=\"mermaid\">\n");
    fprintf(fp, "graph TD;\n");

    char q_buf[256];
    for (int b = 0; b < current_cfg.num_blocks; b++) {
        BasicBlock *bb = &current_cfg.blocks[b];
        fprintf(fp, "    B%d[\"<b>Block B%d</b> [Quads %d..%d]<br/>", bb->id, bb->id, bb->start_quad, bb->end_quad);
        for (int q = bb->start_quad; q <= bb->end_quad; q++) {
            format_quad_str(q_buf, sizeof(q_buf), q, true);
            fprintf(fp, "%s", q_buf);
        }
        fprintf(fp, "\"]\n");
    }

    for (int b = 0; b < current_cfg.num_blocks; b++) {
        BasicBlock *bb = &current_cfg.blocks[b];
        CFGEdge *e = bb->successors;
        while (e) {
            fprintf(fp, "    B%d -->|%s| B%d\n", bb->id, e->label, e->target_block);
            e = e->next;
        }
    }
    fprintf(fp, "    </div>\n");
    fprintf(fp, "  </div>\n");

    fprintf(fp, "  <div class=\"grid\">\n");
    fprintf(fp, "    <div class=\"card\">\n");
    fprintf(fp, "      <h2>Three-Address Code (Quadruples)</h2>\n");
    fprintf(fp, "      <pre>");
    for (int i = 0; i < nextquad; i++) {
        format_quad_str(q_buf, sizeof(q_buf), i, false);
        fprintf(fp, "%s\n", q_buf);
    }
    fprintf(fp, "</pre>\n    </div>\n");

    fprintf(fp, "    <div class=\"card\">\n");
    fprintf(fp, "      <h2>Basic Block Summary</h2>\n");
    fprintf(fp, "      <table>\n");
    fprintf(fp, "        <tr><th>Block</th><th>Quads</th><th>Reachable</th><th>Successors</th></tr>\n");
    for (int b = 0; b < current_cfg.num_blocks; b++) {
        BasicBlock *bb = &current_cfg.blocks[b];
        fprintf(fp, "        <tr><td><b>B%d</b></td><td>%d..%d</td><td style=\"color:%s\">%s</td><td>",
                bb->id, bb->start_quad, bb->end_quad,
                bb->is_reachable ? "#4ade80" : "#f87171",
                bb->is_reachable ? "YES" : "NO");
        CFGEdge *e = bb->successors;
        if (!e) fprintf(fp, "<i>Exit</i>");
        while (e) {
            fprintf(fp, "B%d (%s) ", e->target_block, e->label);
            e = e->next;
        }
        fprintf(fp, "</td></tr>\n");
    }
    fprintf(fp, "      </table>\n    </div>\n");
    fprintf(fp, "  </div>\n");
    fprintf(fp, "</div>\n");
    fprintf(fp, "<script>mermaid.initialize({startOnLoad:true, theme:'dark'});</script>\n");
    fprintf(fp, "</body>\n</html>\n");
    fclose(fp);
}
