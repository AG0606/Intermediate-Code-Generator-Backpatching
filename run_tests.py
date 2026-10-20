#!/usr/bin/env python3
import os
import subprocess
import sys

GREEN = "\033[92m"
RED = "\033[91m"
BLUE = "\033[94m"
YELLOW = "\033[93m"
RESET = "\033[0m"

TESTS = [
    {
        "name": "Slide 13: if-else statement",
        "file": "tests/test1_if_else.txt",
        "args": [],
        "expected_substrings": [
            "0: if a < b goto 2",
            "1: goto 4",
            "2: x = 1",
            "3: goto 5",
            "4: x = 2",
            "5: end"
        ]
    },
    {
        "name": "Slide 14: Short-Circuit AND (&&)",
        "file": "tests/test2_short_circuit.txt",
        "args": [],
        "expected_substrings": [
            "0: if a < b goto 2",
            "1: goto 5",
            "2: if c < d goto 4",
            "3: goto 5",
            "4: x = 1",
            "5: end"
        ]
    },
    {
        "name": "Slide 15 & 19: Nested while + if-else",
        "file": "tests/test3_nested_while_if.txt",
        "args": [],
        "expected_substrings": [
            "0: if a < b goto 2",
            "1: goto 8",
            "2: if c < d goto 4",
            "3: goto 6",
            "4: x = 1",
            "5: goto 0",
            "6: x = 2",
            "7: goto 0",
            "8: end"
        ]
    },
    {
        "name": "For Loop with break and continue",
        "file": "tests/test4_for_loop.txt",
        "args": ["--quads"],
        "expected_substrings": [
            "QUADRUPLE TABLE",
            "goto",
            "end"
        ]
    },
    {
        "name": "Do-While Loop with break and continue",
        "file": "tests/test5_do_while.txt",
        "args": ["--quads"],
        "expected_substrings": [
            "QUADRUPLE TABLE",
            "goto",
            "end"
        ]
    },
    {
        "name": "Switch-Case with default and breaks",
        "file": "tests/test6_switch_case.txt",
        "args": ["--quads"],
        "expected_substrings": [
            "QUADRUPLE TABLE",
            "goto",
            "end"
        ]
    },
    {
        "name": "Complex Booleans (&&, ||, !)",
        "file": "tests/test7_complex_boolean.txt",
        "args": ["--cfg"],
        "expected_substrings": [
            "BASIC BLOCKS & CONTROL FLOW GRAPH",
            "[CFG] Exported Graphviz DOT",
            "[CFG] Exported Interactive HTML Visualizer"
        ]
    },
    {
        "name": "Deeply Nested Structures",
        "file": "tests/test8_deep_nesting.txt",
        "args": ["--all"],
        "expected_substrings": [
            "SYMBOL TABLE ENTRIES",
            "QUADRUPLE TABLE REPRESENTATION",
            "INTERMEDIATE CODE OPTIMIZATION REPORT"
        ]
    },
    {
        "name": "Variable Declarations and Type Checking",
        "file": "tests/test9_declarations_types.txt",
        "args": ["--symtab"],
        "expected_substrings": [
            "SYMBOL TABLE ENTRIES",
            "int",
            "inner"
        ]
    },
    {
        "name": "Semantic Errors Detection",
        "file": "tests/test10_semantic_errors.txt",
        "args": ["--strict"],
        "expected_exit_code": 1,
        "expected_substrings": [
            "[SEMANTIC ERROR]"
        ]
    },
    {
        "name": "Syntax Error Recovery",
        "file": "tests/test11_syntax_recovery.txt",
        "args": [],
        "expected_exit_code": 1,
        "expected_substrings": [
            "[SYNTAX ERROR]"
        ]
    },
    {
        "name": "Intermediate Code Optimization (Constant Folding)",
        "file": "tests/test12_optimization.txt",
        "args": ["--optimize"],
        "expected_substrings": [
            "INTERMEDIATE CODE OPTIMIZATION REPORT",
            "Constants Folded     : 2",
            "t1 = 30",
            "t2 = 300"
        ]
    },
    {
        "name": "Full C Syntax: Functions, Scopes { }, and Variable Shadowing",
        "file": "tests/test13_c_program_scopes.txt",
        "args": ["--symtab"],
        "expected_substrings": [
            "func_begin helper",
            "func_begin main",
            "x_s5",
            "return result"
        ]
    }
]

def run_tests():
    binary = "./icg"
    if not os.path.exists(binary):
        print(f"{RED}Error: {binary} does not exist. Run 'make' first.{RESET}")
        sys.exit(1)

    print("=" * 75)
    print("      INTERMEDIATE CODE GENERATOR (ICG) - AUTOMATED TEST SUITE        ")
    print("=" * 75)

    passed = 0
    total = len(TESTS)

    for i, test in enumerate(TESTS, 1):
        test_file = test["file"]
        cmd = [binary, test_file] + test.get("args", [])
        expected_exit = test.get("expected_exit_code", 0)

        res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        combined_output = res.stdout + "\n" + res.stderr

        all_matched = True
        missing = []
        for s in test.get("expected_substrings", []):
            if s not in combined_output:
                all_matched = False
                missing.append(s)

        exit_code_ok = (res.returncode == expected_exit)

        if all_matched and exit_code_ok:
            print(f"[{GREEN}PASS{RESET}] Test {i:2d}/{total:2d}: {test['name']}")
            passed += 1
        else:
            print(f"[{RED}FAIL{RESET}] Test {i:2d}/{total:2d}: {test['name']}")
            if not exit_code_ok:
                print(f"       Expected exit code {expected_exit}, got {res.returncode}")
            if missing:
                print(f"       Missing expected substrings: {missing}")
                print("--- Output was: ---")
                print(combined_output[:500])
                print("-------------------")

    print("=" * 75)
    print(f"Test Summary: {passed}/{total} tests passed ({passed/total*100:.1f}%)")
    print("=" * 75)

    if passed == total:
        print(f"{GREEN}ALL TESTS PASSED SUCCESSFULLY!{RESET}\n")
        return 0
    else:
        print(f"{RED}SOME TESTS FAILED!{RESET}\n")
        return 1

if __name__ == "__main__":
    sys.exit(run_tests())
