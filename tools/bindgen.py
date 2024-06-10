import subprocess
import json
import code

def filter_types(str):
    return str.replace('_Bool', 'bool')

def parse_struct(decl):
    if decl.get("inner") == None:
        return None

    outp = {}
    outp['kind'] = 'struct'
    outp['struct_kind'] = decl["tagUsed"]
    outp['name'] = decl['name']
    outp['fields'] = []
    for item_decl in decl['inner']:
        if item_decl['kind'] == 'FullComment': continue
        if item_decl['kind'] != 'FieldDecl':
            print(f"// ERROR: Structs must only contain simple fields ({decl['name']})")
            print("/*", item_decl, "*/")
            return None
        item = {}
        if 'name' in item_decl:
            item['name'] = item_decl['name']
        item['type'] = filter_types(item_decl['type']['qualType'])
        outp['fields'].append(item)
    return outp

def parse_enum(decl):
    if decl.get("inner") == None:
        return None
    outp = {}
    if 'name' in decl:
        outp['kind'] = 'enum'
        outp['name'] = decl['name']
        needs_value = False
    else:
        outp['kind'] = 'consts'
        needs_value = True
    outp['items'] = []

    for item_decl in decl['inner']:
        if item_decl['kind'] == 'EnumConstantDecl':
            item = {}
            item['name'] = item_decl['name']
            if 'inner' in item_decl:
                const_expr = item_decl['inner'][0]
                if const_expr['kind'] != 'ConstantExpr':
                    print(f"// ERROR: Enum values must be a ConstantExpr ({item_decl['name']}), is '{const_expr['kind']}'")
                    return None
                if const_expr['valueCategory'] != 'rvalue' and const_expr['valueCategory'] != 'prvalue':
                    print(f"// ERROR: Enum value ConstantExpr must be 'rvalue' or 'prvalue' ({item_decl['name']}), is '{const_expr['valueCategory']}'")
                    return None
                if not ((len(const_expr['inner']) == 1) and (const_expr['inner'][0]['kind'] == 'IntegerLiteral')):
                    print(f"// ERROR: Enum value ConstantExpr must have exactly one IntegerLiteral ({item_decl['name']})")
                    return None
                item['value'] = const_expr['inner'][0]['value']
            if needs_value and 'value' not in item:
                print(f"// ERROR: anonymous enum items require an explicit value")
                return None
            outp['items'].append(item)
    return outp

def parse_func(decl):
    outp = {}
    outp['kind'] = 'func'
    outp['name'] = decl['name']
    outp['type'] = filter_types(decl['type']['qualType'])

    outp['variadic'] = False
    if decl.get("variadic"):
        outp['variadic'] = True
    outp['params'] = []
    if 'inner' in decl:
        for param in decl['inner']:
            if param['kind'] != 'ParmVarDecl':
                if param['kind'] == 'BuiltinAttr': continue
                if param['kind'] == 'NoThrowAttr': continue
                if param['kind'] == 'FullComment': continue
                if param['kind'] == 'PureAttr': continue
                if param['kind'] == 'DLLImportAttr': continue
                if param['kind'] == 'CompoundStmt': continue # function with body
                if param['kind'] == 'AlwaysInlineAttr': continue
                if param['kind'] == 'FormatAttr':
                    outp['variadic'] = True
                    continue
                print(f"// warning: ignoring func {decl['name']} (unsupported parameter type) {param['kind']}")
                return None
            if param.get('name') == None: return None
            outp_param = {}
            outp_param['name'] = param['name']
            outp_param['type'] = filter_types(param['type']['qualType'])
            outp['params'].append(outp_param)
    return outp

def parse_typedef(decl):
    if decl['type']['qualType'].startswith("struct") or\
        decl['type']['qualType'].startswith("union") or\
        decl['type']['qualType'].startswith("enum"):
        return None
    outp = {};
    outp['kind'] = 'typedef'
    outp['name'] = decl['name']
    outp['type'] = filter_types(decl['type']['qualType'])
    return outp

def parse_decl(decl):
    kind = decl['kind']
    if kind == 'RecordDecl':
        return parse_struct(decl)
    elif kind == 'EnumDecl':
        return parse_enum(decl)
    elif kind == 'FunctionDecl':
        return parse_func(decl)
    elif kind == "TypedefDecl":
        return parse_typedef(decl)
    else:
        return None

def bindgen_enum(decl):
    r = ""
    r += decl["name"] + " :: typedef int;\n"
    for item in decl["items"]:
        r += item["name"]
        if item.get("value"):
            r += " :: " + item["value"]
        r += ";\n"
    return r

types = {
    "unsigned int": "uint",
    "unsigned long": "ulong",
    "unsigned long long": "ullong",
    "long long": "llong",
    "unsigned int": "uint",
    "unsigned": "uint",
    "unsigned short": "ushort",
    "unsigned char": "uchar",
    "uint64_t": "u64",
    "uint32_t": "u32",
    "uint16_t": "u16",
    "uint8_t": "u8",
    "int64_t": "i64",
    "int32_t": "i32",
    "int16_t": "i16",
    "int8_t": "i8",
    "size_t": "usize",
}

def bindgen_type(type):
    type = type.replace("const ", "")
    type = type.strip()
    if types.get(type): return types[type]
    if type[-1] == '*': return "*" + bindgen_type(type[:-1])
    if type[-1] == ' ': return bindgen_type(type[:-1])
    if type[-1] == ']':
        i = -1
        while type[i] != '[': i += 1
        array = type[i:]
        return array + bindgen_type(type[:i])

    func_ptr_idx = type.find('(*)')
    if func_ptr_idx != -1:
        args = type[func_ptr_idx+4:-1]
        args = args.split(",")
        new_args = []

        name = 'a'
        for it in args:
            new_args.append(name + ": " + bindgen_type(it))
            name = chr(ord(name[0]) + 1)
        new_args = ', '.join(new_args)

        ret = type[:func_ptr_idx]
        return "proc(" + new_args + "): " + bindgen_type(ret)

    func_idx = type.find('(')
    if func_idx != -1:
        args = type[func_idx+1:-1]
        args = args.split(",")
        new_args = []

        name = 'a'
        for it in args:
            new_args.append(name + ": " + bindgen_type(it))
            name = chr(ord(name[0]) + 1)
        new_args = ', '.join(new_args)

        ret = type[:func_idx]
        return "proc(" + new_args + "): " + bindgen_type(ret)

    return type

def bindgen_struct(decl):
    r = ""
    r += decl["name"] + " :: " + decl["struct_kind"] + " {\n"
    for field in decl["fields"]:
        r += "    " + field["name"] + ": " + bindgen_type(field["type"]) + ";\n"
    r += "}\n"
    return r

def bindgen_func(decl):
    r = ""
    vargs = decl['variadic']
    r += decl["name"] + " :: proc("
    i = 0
    for param in decl["params"]:
        r += param["name"] + ": " + bindgen_type(param["type"])
        if i != len(decl["params"]) - 1 or vargs:
            r += ", "
        i += 1
    if vargs: r += "..."
    r += ")"
    decl_type = decl["type"]
    return_type = decl_type[:decl_type.index('(')].strip()
    return_type = bindgen_type(return_type)
    if return_type != "void":
        r += ": " + return_type
    r += ";"
    return r

def bindgen_typedef(decl):
    return decl["name"] + " :: typedef " + bindgen_type(decl["type"]) + ";"


def get_clang_ast(file):
    cmd = ['clang', '-IC:/Work/', '-Xclang', '-ast-dump=json']
    cmd.append(file)
    out = subprocess.run(cmd, stdout=subprocess.PIPE)

    if out.returncode == 0:
        tu = json.loads(out.stdout)
        return tu
    return None

def has_prefix(decl, prefixes):
    if prefixes == None or len(prefixes) == 0: return True

    for it in prefixes:
        if decl["name"].startswith(it):
            return True
    return False

def bindgen_to_stdout(file, prefixes):
    tu = get_clang_ast(file)
    if tu == None: return

    decls = []
    for decl in tu["inner"]:
        has_name = decl.get("name")
        if has_name == None: continue

        hp = has_prefix(decl, prefixes)
        if hp:
            out_decl = parse_decl(decl)
            if out_decl:
                decls.append(out_decl)

    for decl in decls:
        try:
            if decl["kind"] == "func":      out = bindgen_func(decl)
            elif decl["kind"] == "struct":  out = bindgen_struct(decl)
            elif decl["kind"] == "enum":    out = bindgen_enum(decl)
            elif decl["kind"] == "typedef": out = bindgen_typedef(decl)
            else: sys.exit("invalid decl kind", decl)
        except IndexError:
            print(f"// ERROR: generating {decl['name']} {decl['type']}")
            continue

        print(out)


if __name__ == "__main__":
    import argparse
    import sys

    parser = argparse.ArgumentParser(description='generate bindings for a c file using clang')
    parser.add_argument('filename')
    parser.add_argument('--prefixes', nargs='+', help = 'generate declarations with these prefixes')
    args = parser.parse_args()

    if args.filename.endswith(".c") or args.filename.endswith(".cpp"):
        print("warning: for some reason clang doesn't like c and c++ files, the file to generate bindings for need to be a header")

    bindgen_to_stdout(args.filename, args.prefixes)