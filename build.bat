@echo off
echo =========================================================================
echo              Building Intermediate Code Generator (ICG)
echo =========================================================================

where wsl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Building via WSL environment...
    wsl bash -c "cd /mnt/c/Users/Ashutosh/Downloads/Compiler_Project && make clean && make"
    if %ERRORLEVEL% EQU 0 (
        echo [SUCCESS] icg build succeeded!
        goto done
    )
)

echo Fallback: Attempting native compilation if flex/bison/gcc exist in PATH...
flex -o src/lex.yy.c src/lexer.l
bison -d -o src/y.tab.c src/parser.y
gcc -Wall -Isrc src/y.tab.c src/lex.yy.c src/quad.c src/symbol_table.c src/loop_stack.c src/cfg.c src/optimizer.c src/error_reporter.c src/main.c -o icg.exe

:done
echo Build finished.
