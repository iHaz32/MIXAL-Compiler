%{
#include <stdio.h>
#include <string.h>
#include "sydc.tab.h"

extern FILE *logFile;  // external log file pointer
%}

%%

"read"              { fprintf(logFile, "Lex: Returning READ\n"); return READ; }
"write"             { fprintf(logFile, "Lex: Returning WRITE\n"); return WRITE; }
"if"                { fprintf(logFile, "Lex: Returning IF\n"); return IF; }
"then"              { fprintf(logFile, "Lex: Returning THEN\n"); return THEN; }
"else"              { fprintf(logFile, "Lex: Returning ELSE\n"); return ELSE; }
"repeat"            { fprintf(logFile, "Lex: Returning REPEAT\n"); return REPEAT; }
"until"             { fprintf(logFile, "Lex: Returning UNTIL\n"); return UNTIL; }
"end"               { fprintf(logFile, "Lex: Returning END\n"); return END; }
[0-9]+              { fprintf(logFile, "Lex: Returning DEC_CONST %s\n", yytext); yylval.yint = atoi(yytext); return DEC_CONST; }
"*"                 { fprintf(logFile, "Lex: Returning '*'\n"); return '*'; }
"/"                 { fprintf(logFile, "Lex: Returning '/'\n"); return '/'; }
"+"                 { fprintf(logFile, "Lex: Returning '+'\n"); return '+'; }
"-"                 { fprintf(logFile, "Lex: Returning '-'\n"); return '-'; }
":="                { fprintf(logFile, "Lex: Returning AS\n"); return AS; }
"<"                 { fprintf(logFile, "Lex: Returning '<'\n"); return '<'; }
"="                 { fprintf(logFile, "Lex: Returning '='\n"); return '='; }
"("                 { fprintf(logFile, "Lex: Returning '('\n"); return '('; }
")"                 { fprintf(logFile, "Lex: Returning ')'\n"); return ')'; }
";"                 { fprintf(logFile, "Lex: Returning ';'\n"); return ';'; }
[a-zA-Z][a-zA-Z0-9]* { fprintf(logFile, "Lex: Returning ID %s\n", yytext); strcpy(yylval.ystr, yytext); return ID; }
[ \t\n]+            {}   // ignore whitespaces
.                   { fprintf(logFile, "Lex: Unknown Character %s\n", yytext); }

%%