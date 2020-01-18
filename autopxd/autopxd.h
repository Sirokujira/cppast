
class PxdNode
{
private:
    const std::string indent = '    ';

public:
    virtual std::string __str__(self)
    {
        return '\n'.join(this->lines())
    }
};

class IdentifierType : public PxdNode
{
private:
    std::string name;
    std::string type_name;

public:
    def IdentifierType(name, type_name)
    {
        this->name = name or ''
        this->type_name = type_name
    }

    std::string lines()
    {
        if this->name:
            return ['{0} {1}'.format(this->type_name, this->name)]
        else:
            return [this->type_name]
    }
};

class Function : public PxdNode
{
private:
    std::string return_type;
    std::string name;
    std::string args;

public:
    Function(std::string return_type, std::string name, std::string args)
    {
        this->return_type = return_type
        this->name = name
        this->args = args
    }

    std::string argstr()
    {
        l = []
        for arg in this->args:
            lines = arg.lines()
            assert len(lines) == 1
            l.append(lines[0])
        return ', '.join(l)
    }

    std::string lines()
    {
        return [
            '{0} {1}({2})'.format(this->return_type, this->name, this->argstr())
        ]
    }
};

class Ptr : public IdentifierType
{
private:
	auto node;

public:
    Ptr(auto node)
    {
        this->node = node;
    }

    // @property
    def name():
        return this->node.name;

    @property
	auto type_name()
	{
        return this->node.type_name + '*';
	}

    auto lines()
	{
        if isinstance(this->node, Function)
		{
            f = this->node
            args = f.argstr()
            return ['{0} (*{1})({2})'.format(f.return_type, f.name, args)]
		}
        else
		{
            return super(Ptr, self).lines()
		}
	}
};

/*!	@brief	
*/
class Array : public IdentifierType
{
private:
	auto node;

public:
    Array(auto node)
	{
        this->node = node
	}

    // @property
    def name(self):
        return this->node.name + '[1]'

    // @property
    def type_name(self):
        return this->node.type_name
};

/*!	@brief	
*/
class Type : public PxdNode
{
public:
	Type(node)
	{
        this->node = node
	}

    std::vector<std::string> lines(self)
	{
        lines = this->node.lines()
        lines[0] = 'ctypedef ' + lines[0]
        return lines
	}
};

/*!	@brief	
*/
class Block : public PxdNode
{
private:
    std::string name;
    std::string fields;
    std::string kind;

public:
    Block(std::string name, std::string fields, std::string kind)
    {
        this->name = name;
        this->fields = fields;
        this->kind = kind;
    }

    std::vector<double> lines()
    {
        rv = ['cdef {0} {1}:'.format(this->kind, this->name)]
        for field in this->fields:
            for line in field.lines():
                rv.append(this->indent + line)
        return rv
    }
}

/*!	@brief	
*/
class Enum : pubic PxdNode
{
private:
    std::string name;
    std::string items;
public:
    Enum(std::string name, std::string items)
    {
        this->name = name;
        this->items = items;
    }

    std::vector<std::string> lines()
    {
        rv = []
        if this->name:
            rv.append('cdef enum {0}:'.format(this->name))
        else:
            rv.append('cdef enum:')
        for item in this->items:
            rv.append(this->indent + item)
        return rv
    }
};

interface ClassDefine
{
};

/*!
*/
class AutoPxd : public PxdNode
{
private:
    // hファイル(root ファイル)
    // namespace(階層になるケースがあるため複数?)
    // class定義(階層になるケースがあるため複数?)
    // template付 class定義
    // template 定義
    // struct 定義
    // → 上記パラメータを同一で扱えるように基底クラスを用意する?

public:
    AutoPxd(auto hdrname)
    {
        this->hdrname = hdrname
        this->decl_stack = [[]]
        this->visit_stack = []
    }

    auto visit(auto node)
    {
        this->visit_stack.append(node)
        rv = super(AutoPxd, self).visit(node)
        n = this->visit_stack.pop()
        assert n == node
        return rv
    }

    void visit_IdentifierType(auto node)
    {
        this->append(' '.join(node.names))
    }

    void visit_Block(auto node, auto kind)
    {
        name = node.name
        if not name:
            name = this->path_name(kind[0])
        if not node.decls:
            if this->child_of(c_ast.TypeDecl, -2):
                # not a definition, must be a reference
                this->append(name)
            return
        fields = this->collect(node)
        # add the struct/union definition to the top level
        this->decl_stack[0].append(Block(name, fields, kind))
        if this->child_of(c_ast.TypeDecl, -2):
            # inline struct/union, add a reference to whatever name it was
            # defined on the top level
            this->append(name)
    }

    void visit_Enum(auto node)
    {
        items = []
        if node.values:
            for item in node.values.enumerators:
                items.append(item.name)
        name = node.name
        type_decl = this->child_of(c_ast.TypeDecl, -2)
        if not name and type_decl:
            name = this->path_name('e')
        # add the enum definition to the top level
        if len(items):
            this->decl_stack[0].append(Enum(name, items))
        if type_decl:
            this->append(name)
    }

    auto visit_Struct(auto node)
    {
        return this->visit_Block(node, 'struct')
    }

    auto visit_Union(auto node)
    {
        return this->visit_Block(node, 'union')
    }

    void visit_TypeDecl(auto node)
    {
        decls = this->collect(node)
        if not decls:
            return
        assert len(decls) == 1
        if isinstance(decls[0], six.string_types):
            this->append(IdentifierType(node.declname, decls[0]))
        else:
            this->append(decls[0])
    }

    void visit_Decl(auto node)
    {
        decls = this->collect(node)
        if not decls:
            return
        assert len(decls) == 1
        if isinstance(decls[0], six.string_types):
            this->append(IdentifierType(node.name, decls[0]))
        else:
            this->append(decls[0])
    }

    void visit_FuncDecl(auto node)
    {
        decls = this->collect(node)
        return_type = decls[-1].type_name
        fname = decls[-1].name
        args = decls[:-1]
        if (len(args) == 1 and isinstance(args[0], IdentifierType) and
            args[0].type_name == 'void'):
            args = []
        if (this->child_of(c_ast.PtrDecl, -2) and not
            this->child_of(c_ast.Typedef, -3)):
            # declaring a variable or parameter
            name = this->path_name('ft'.format(fname))
            this->decl_stack[0].append(Type(Ptr(Function(return_type, name, args))))
            this->append(name)
        else:
            this->append(Function(return_type, fname, args))
    }

    void visit_PtrDecl(auto node)
    {
        decls = this->collect(node)
        assert len(decls) == 1
        if isinstance(decls[0], six.string_types):
            this->append(decls[0])
        else:
            this->append(Ptr(decls[0]))
    }

    void visit_ArrayDecl(auto node)
    {
        decls = this->collect(node)
        assert len(decls) == 1
        this->append(Array(decls[0]))
    }

    void visit_Typedef(auto node)
    {
        decls = this->collect(node)
        if len(decls) != 1:
            return
        names = str(decls[0]).split()
        if names[0] != names[1]:
            this->decl_stack[0].append(Type(decls[0]))
    }

    auto collect(auto node)
    {
        decls = []
        this->decl_stack.append(decls)
        name = this->generic_visit(node)
        assert this->decl_stack.pop() == decls
        return decls
    }

    auto path_name(auto tag)
    {
        names = []
        for node in this->visit_stack[:-2]:
            if hasattr(node, 'declname') and node.declname:
                names.append(node.declname)
            elif hasattr(node, 'name') and node.name:
                names.append(node.name)
        return '_{0}_{1}'.format('_'.join(names), tag)
    }

    auto child_of(auto type, auto index=None)
    {
        if index is None:
            for node in reversed(this->visit_stack):
                if isinstance(node, type):
                    return True
            return False
        else:
            return isinstance(this->visit_stack[index], type)
    }

    append(node)
    {
        this->decl_stack[-1].append(node)
    }

    auto lines()
    {
        rv = ['cdef extern from "{0}":'.format(this->hdrname), '']
        for decl in this->decl_stack[0]:
            for line in decl.lines():
                rv.append(this->indent + line)
            rv.append('')
        return rv
    }
};

// TODO: 対象モジュールのインクルードパスを取得
// # BUILTIN_HEADERS_DIR = os.path.join(os.path.dirname(__file__), 'include')
// # Types declared by pycparser fake headers that we should ignore
/*
IGNORE_DECLARATIONS = set((
    'size_t', '__builtin_va_list', '__gnuc_va_list', '__int8_t', '__uint8_t',
    '__int16_t', '__uint16_t', '__int_least16_t', '__uint_least16_t',
    '__int32_t', '__uint32_t', '__int64_t', '__uint64_t', '__int_least32_t',
    '__uint_least32_t', '__s8', '__u8', '__s16', '__u16', '__s32', '__u32',
    '__s64', '__u64', '_LOCK_T', '_LOCK_RECURSIVE_T', '_off_t', '__dev_t',
    '__uid_t', '__gid_t', '_off64_t', '_fpos_t', '_ssize_t', 'wint_t',
    '_mbstate_t', '_flock_t', '_iconv_t', '__ULong', '__FILE', 'ptrdiff_t',
    'wchar_t', '__off_t', '__pid_t', '__loff_t', 'u_char', 'u_short', 'u_int',
    'u_long', 'ushort', 'uint', 'clock_t', 'time_t', 'daddr_t', 'caddr_t',
    'ino_t', 'off_t', 'dev_t', 'uid_t', 'gid_t', 'pid_t', 'key_t', 'ssize_t',
    'mode_t', 'nlink_t', 'fd_mask', '_types_fd_set', 'clockid_t', 'timer_t',
    'useconds_t', 'suseconds_t', 'FILE', 'fpos_t', 'cookie_read_function_t',
    'cookie_write_function_t', 'cookie_seek_function_t',
    'cookie_close_function_t', 'cookie_io_functions_t', 'div_t', 'ldiv_t',
    'lldiv_t', 'sigset_t', '__sigset_t', '_sig_func_ptr', 'sig_atomic_t',
    '__tzrule_type', '__tzinfo_type', 'mbstate_t', 'sem_t', 'pthread_t',
    'pthread_attr_t', 'pthread_mutex_t', 'pthread_mutexattr_t',
    'pthread_cond_t', 'pthread_condattr_t', 'pthread_key_t', 'pthread_once_t',
    'pthread_rwlock_t', 'pthread_rwlockattr_t', 'pthread_spinlock_t',
    'pthread_barrier_t', 'pthread_barrierattr_t', 'jmp_buf', 'rlim_t',
    'sa_family_t', 'sigjmp_buf', 'stack_t', 'siginfo_t', 'z_stream', 'int8_t',
    'uint8_t', 'int16_t', 'uint16_t', 'int32_t', 'uint32_t', 'int64_t',
    'uint64_t', 'int_least8_t', 'uint_least8_t', 'int_least16_t',
    'uint_least16_t', 'int_least32_t', 'uint_least32_t', 'int_least64_t',
    'uint_least64_t', 'int_fast8_t', 'uint_fast8_t', 'int_fast16_t',
    'uint_fast16_t', 'int_fast32_t', 'uint_fast32_t', 'int_fast64_t',
    'uint_fast64_t', 'intptr_t', 'uintptr_t', 'intmax_t', 'uintmax_t', 'bool',
    'va_list',
))


WHITELIST = []

@click.command()
@click.argument('infile', type=click.File('rb'), default=sys.stdin)
@click.argument('outfile', type=click.File('wb'), default=sys.stdout)
def cli(infile, outfile):
    outfile.write(translate(infile.read(), infile.name))
*/
