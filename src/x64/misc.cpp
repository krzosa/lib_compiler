S8_String GetCodeLine(LC_AST *n) {
    LC_Token    *token   = n->pos;
    LC_Lex      *x       = n->file->afile.x;
    S8_String content = S8_MakeFromChar(x->begin);
    S8_List   lines   = S8_Split(L->arena, content, S8_Lit("\n"), 0);

    int line = 1;
    S8_For(it, lines) {
        if (token->line == line) return it->string;
        line += 1;
    }
    return S8_MakeEmpty();
}

void OutASTLine(LC_AST *n) {
    S8_String line = GetCodeLine(n);
    LC_GenLinef(";%.*s", S8_Expand(line));
}
