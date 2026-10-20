Write-Host "=========================================================================" -ForegroundColor Cyan
Write-Host "             Building Intermediate Code Generator (ICG)                  " -ForegroundColor Cyan
Write-Host "=========================================================================" -ForegroundColor Cyan

$wslCmd = Get-Command wsl -ErrorAction SilentlyContinue
if ($wslCmd) {
    Write-Host "Building with WSL gcc, flex, bison..." -ForegroundColor Green
    wsl bash -c "cd /mnt/c/Users/Ashutosh/Downloads/Compiler_Project && make clean && make"
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`n[SUCCESS] Build completed successfully via WSL!" -ForegroundColor Green
        exit 0
    }
}

Write-Host "Attempting local build with native tools..." -ForegroundColor Yellow
& flex -o src/lex.yy.c src/lexer.l
& bison -d -o src/y.tab.c src/parser.y
& gcc -Wall -Isrc src/y.tab.c src/lex.yy.c src/quad.c src/symbol_table.c src/loop_stack.c src/cfg.c src/optimizer.c src/error_reporter.c src/main.c -o icg.exe
