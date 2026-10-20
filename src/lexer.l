%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "quad.h"
#include "y.tab.h"
#include "common.h"

int yycolumn = 1;

#define YY_USER_ACTION yycolumn += yyleng;
%}

%option yylineno
%option noinput
%option nounput

%%
[ \t\r]+                    { /* Skip whitespace */ }
\n                          { yycolumn = 1; }
"//".*                      { /* Single-line comment */ }
"/*"([^*]|\*+[^*/])*\*+"/"  {
                                for (int i = 0; yytext[i]; i++) {
                                    if (yytext[i] == '\n') {
                                        yycolumn = 1;
                                    }
                                }
                            }

"if"                        { return IF; }
"else"                      { return ELSE; }
"while"                     { return WHILE; }
"for"                       { return FOR; }
"do"                        { return DO; }
"switch"                    { return SWITCH; }
"case"                      { return CASE; }
"default"                   { return DEFAULT; }
"break"                     { return BREAK; }
"continue"                  { return CONTINUE; }
"return"                    { return RETURN; }

"int"                       { strcpy(yylval.str, yytext); return TYPE_INT_KW; }
"short"                     { strcpy(yylval.str, yytext); return TYPE_INT_KW; }
"long"                      { strcpy(yylval.str, yytext); return TYPE_INT_KW; }
"float"                     { strcpy(yylval.str, yytext); return TYPE_FLOAT_KW; }
"double"                    { strcpy(yylval.str, yytext); return TYPE_FLOAT_KW; }
"char"                      { strcpy(yylval.str, yytext); return TYPE_CHAR_KW; }
"bool"                      { strcpy(yylval.str, yytext); return TYPE_BOOL_KW; }
"void"                      { strcpy(yylval.str, yytext); return TYPE_VOID_KW; }

"true"                      { return TRUE_KW; }
"false"                     { return FALSE_KW; }

"++"                        { return INC_OP; }
"--"                        { return DEC_OP; }

"+="                        { return ADD_ASSIGN; }
"-="                        { return SUB_ASSIGN; }
"*="                        { return MUL_ASSIGN; }
"/="                        { return DIV_ASSIGN; }
"%="                        { return MOD_ASSIGN; }

"<<"                        { return SHL_OP; }
">>"                        { return SHR_OP; }

"&&"                        { return AND; }
"||"                        { return OR; }
"!"                         { return NOT; }

"<="                        { strcpy(yylval.str, "<="); return RELOP; }
">="                        { strcpy(yylval.str, ">="); return RELOP; }
"=="                        { strcpy(yylval.str, "=="); return RELOP; }
"!="                        { strcpy(yylval.str, "!="); return RELOP; }
"<"                         { strcpy(yylval.str, "<"); return RELOP; }
">"                         { strcpy(yylval.str, ">"); return RELOP; }

";"                         { return ';'; }
":"                         { return ':'; }
","                         { return ','; }
"{"                         { return '{'; }
"}"                         { return '}'; }
"("                         { return '('; }
")"                         { return ')'; }
"+"                         { return '+'; }
"-"                         { return '-'; }
"*"                         { return '*'; }
"/"                         { return '/'; }
"%"                         { return '%'; }
"="                         { return '='; }
"&"                         { return '&'; }
"|"                         { return '|'; }
"^"                         { return '^'; }
"~"                         { return '~'; }
"?"                         { return '?'; }

[0-9]+\.[0-9]+              { strcpy(yylval.str, yytext); return FLOAT_NUM; }
[0-9]+                      { strcpy(yylval.str, yytext); return NUM; }
[a-zA-Z_][a-zA-Z0-9_]*      { strcpy(yylval.str, yytext); return ID; }

.                           { return yytext[0]; }

%%

int yywrap(void) {
    return 1;
}
