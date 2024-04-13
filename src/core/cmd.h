
typedef enum {
    CmdDeclKind_Bool,
    CmdDeclKind_Int,
    CmdDeclKind_List,
    CmdDeclKind_Enum,
} CmdDeclKind;

typedef struct CmdDecl CmdDecl;
struct CmdDecl {
    CmdDecl    *next;
    CmdDeclKind kind;
    S8_String   name;
    S8_String   help;

    bool    *bool_result;
    S8_List *list_result;
    int     *int_result;

    int         *enum_result;
    const char **enum_options;
    int          enum_option_count;
};

typedef struct CmdParser CmdParser;
struct CmdParser {
    int         argc;
    char      **argv;
    MA_Arena   *arena;
    const char *custom_help;

    CmdDecl *fdecl;
    CmdDecl *ldecl;

    bool    require_one_standalone_arg;
    S8_List args;
};
