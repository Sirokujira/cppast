#include <fstream>

#include <cppast/libclang_parser.hpp>        // for libclang_parser, libclang_compile_config, cpp_entity,...
#include <cppast/visitor.hpp>                // for visit()
#include <cppast/code_generator.hpp>         // for generate_code()
#include <cppast/cpp_entity_kind.hpp>        // for the cpp_entity_kind definition
#include <cppast/cpp_forward_declarable.hpp> // for is_definition()
#include <cppast/cpp_namespace.hpp>          // for cpp_namespace

#include "autopxd.hpp"

/*! @brief  Pxd ファイル生成の大本クラス
    @remark cppast 側で対処可能な項目との切り分けを見ておくこと
            code_generator の定義はこちらに書くべき?
*/
class AutoPxdFactory
{
private:

public:
    AutoPxdFactory()
    {
    }

    virtual ~AutoPxdFactory()
    {
    }

    AutoPxd autopxd_entity(const cppast::cpp_entity& e)
    {
    	AutoPxd* retAutoPxd = nullptr;
        bool isDefine;
        isDefine = cppast::is_definition(e);

        // 種類
        // include directive -> 
        // macro definition -> 
        // namespace
        // class
        // class template
        // function
        // member variable
        // member function
        // access specifier
        switch(e.kind())
        {
            // cpp_entity_kind.hpp を参照
            case cppast::cpp_entity_kind::namespace_t:
                break;

            case cppast::cpp_entity_kind::macro_parameter_t:
            case cppast::cpp_entity_kind::macro_definition_t:
            case cppast::cpp_entity_kind::include_directive_t:
                break;

            case cppast::cpp_entity_kind::language_linkage_t:
                // fout_pxd << '\n';
                break;

            case cppast::cpp_entity_kind::namespace_alias_t:
            case cppast::cpp_entity_kind::using_directive_t:
            case cppast::cpp_entity_kind::using_declaration_t:
            case cppast::cpp_entity_kind::type_alias_t:
            case cppast::cpp_entity_kind::enum_t:
            case cppast::cpp_entity_kind::enum_value_t:
            case cppast::cpp_entity_kind::class_t:
            case cppast::cpp_entity_kind::access_specifier_t:
            case cppast::cpp_entity_kind::base_class_t:
            case cppast::cpp_entity_kind::variable_t:
            case cppast::cpp_entity_kind::member_variable_t:
            case cppast::cpp_entity_kind::bitfield_t:
            case cppast::cpp_entity_kind::function_parameter_t:
            case cppast::cpp_entity_kind::function_t:
            case cppast::cpp_entity_kind::member_function_t:
            case cppast::cpp_entity_kind::conversion_op_t:
            case cppast::cpp_entity_kind::constructor_t:
            case cppast::cpp_entity_kind::destructor_t:
            case cppast::cpp_entity_kind::friend_t:
            case cppast::cpp_entity_kind::template_type_parameter_t:
            case cppast::cpp_entity_kind::non_type_template_parameter_t:
            case cppast::cpp_entity_kind::template_template_parameter_t:
            case cppast::cpp_entity_kind::alias_template_t:
            case cppast::cpp_entity_kind::variable_template_t:
            case cppast::cpp_entity_kind::function_template_t:
            case cppast::cpp_entity_kind::function_template_specialization_t:
            case cppast::cpp_entity_kind::class_template_t:
            case cppast::cpp_entity_kind::class_template_specialization_t:
            case cppast::cpp_entity_kind::static_assert_t:
            case cppast::cpp_entity_kind::unexposed_t:
                break;

            // case cppast::cpp_entity_kind::count:
        }
        */

        // print whether or not it is a definition
        // if (cppast::is_definition(e))
        if (isDefine)
        {
            // define ?
            // 内部は?
            // fout_pxd << " [definition]";
            // pxd の記述として書き出す
            // e.kind() == "variant" + is_definition() == true
        }

        if (e.kind() == cppast::cpp_entity_kind::language_linkage_t)
        {
            // no need to print additional information for language linkages
            // fout_pxd << '\n';
        }
        else if (e.kind() == cppast::cpp_entity_kind::namespace_t)
        {
            // cast to cpp_namespace
            auto& ns = static_cast<const cppast::cpp_namespace&>(e);
            // print whether or not it is inline
            if (ns.is_inline())
            {
                // inline 関数?
                // fout_pxd << " [inline]";
            }
            
            if(!namespace_str.empty())
            {
                namespace_str += "::";
            }
            namespace_str += e.name();
            // fout_pxd << '\n';
        }
        else
        {
            // 細かい書き出しは、pxd_generator 内で対応する?
            // override で対応?
            // print the declaration of the entity
            // it will only use a single line
            // derive from code_generator and implement various callbacks for printing
            // it will print into a std::string
            class pxd_generator : public cppast::code_generator
            {
                std::string str_;                       // the result
                bool        was_newline_ = false;       // whether or not the last token was a newline
                // needed for lazily printing them

            public:
                pxd_generator(const cppast::cpp_entity& e)
                {
                    // kickoff code generation here
                    cppast::generate_code(*this, e);
                }

                // return the result
                const std::string& str() const noexcept
                {
                    return str_;
                }

            private:
                // called to retrieve the generation options of an entity
                generation_options do_get_options(const cppast::cpp_entity&,
                                                  cppast::cpp_access_specifier_kind) override
                {
                    // generate declaration only
                    return pxd_generator::declaration;
                }

                // no need to handle indentation, as only a single line is used
                void do_indent() override {}
                void do_unindent() override {}

            	/*
                void do_write_keyword(cppast::string_view str)
                {
                    str_ += "\n";
                    str_ += "keyword : ";
                    // const
                    // void
                    // int
                    // class
                    // public
                    // override
                    // 戻り値か、内部の引数か判断はどうやるか？
                    // 他のデータがないことから対応する？
                    str_ += "\n";
                    str_ += str.c_str();
                }

                void do_write_identifier(cppast::string_view str)
                {
                    str_ += "\n";
                    str_ += "identifier : ";
                    // AttributeOctahedronTransform::
                    // SetParameterspunctuation
                    str_ += "\n";
                    str_ += str.c_str();
                }

                void do_write_reference(cppast::string_view str)
                {
                    str_ += "\n";
                    str_ += "reference : ";
                    str_ += "\n";
                    str_ += str.c_str();
                }

                void do_write_punctuation(cppast::string_view str)
                {
                    str_ += "\n";
                    str_ += "punctuation : ";
                    // <PointIndex
                    // >
                    // &
                    // ,
                    // (draco::PointAttribute
                    // )
                    // ;
                    // *
                    str_ += "\n";
                    str_ += str.c_str();
                }

                void do_write_str_literal(cppast::string_view str)
                {
                    str_ += "\n";
                    str_ += "str : ";
                    str_ += "\n";
                    str_ += str.c_str();
                }

                void do_write_int_literal(cppast::string_view str)
                {
                    str_ += "\n";
                    str_ += "int : ";
                    str_ += "\n";
                    str_ += str.c_str();
                }

                void do_write_float_literal(cppast::string_view str)
                {
                    str_ += "\n";
                    str_ += "float : ";
                    str_ += "\n";
                    str_ += str.c_str();
                }

                void do_write_preprocessor(cppast::string_view str)
                {
                    str_ += "\n";
                    str_ += "preprocessor : ";
                    // 先頭 : #include/#define
                    // include の場合
                    // 次 : "file
                    // 次 : "
                    // define の場合
                    // 次 : identifier
                    str_ += "\n";
                    str_ += str.c_str();
                }

                void do_write_comment(cppast::string_view str)
                {
                    str_ += "\n";
                    str_ += "comment : ";
                    str_ += "\n";
                    str_ += str.c_str();
                }
				*/

                // called when a generic token sequence should be generated
                // there are specialized callbacks for various token kinds,
                // to e.g. implement syntax highlighting
                void do_write_token_seq(cppast::string_view tokens) override
                {
                    if (was_newline_)
                    {
                        // lazily append newline as space
                        str_ += ' ';
                        was_newline_ = false;
                    }
                    // append tokens
                    str_ += tokens.c_str();
                }

                // called when a newline should be generated
                // we're lazy as it will always generate a trailing newline,
                // we don't want
                void do_write_newline() override
                {
                    was_newline_ = true;
                }

            } generator(e);

            // fout_pxd << ": `" << generator.str() << '`' << '\n';
            // fout_pxd << '`' << generator.str() << '`' << '\n';

            // stack か何かに突っ込んでおいて、このタイミングで解析する?
            // namespace_str, 
        	fout_pxd << autopxd_generator(namespace_str, e, generator.str()) << '\n';
        }
    }

	// standard c header list
	std::vector<std::string> c_header_lists { "errno.h", "float.h", "limits.h", "locale.h", "math.h", "setjmp.h", "signal.h", "stddef.h", "stdint.h", "stdio.h", "stdlib.h", "string.h", "time.h" };
	// standard c++ header list
	std::vector<std::string> cpp_header_lists { "algorithm", "cast", "complex", "deque", "forward_list", "functional", "iterator", "limits", "list", "map", "memory", "pair", "queue", "set", "stack", "string", "typeindex", "typeinfo", "unordered_map", "unordered_set", "utility", "vector" };
	// posix header list
	std::vector<std::string> posix_header_lists { "dlfcn.h", "fcntl.h", "sys/ioctl.h", "sys/mman.h", "sys/resource.h", "sys/select.h", "signal.h", "sys/stat.h", "stdio.h", "stdlib.h", "strings.h", "sys/time.h", "sys/types.h", "unistd.h", "sys/wait.h" };

    // private std::string autopxd_generator(const std::string& ns_str, const cppast::cpp_entity& e, const std::stack<PxdLine>& linePxdStack)
    std::string autopxd_generator(const std::string& ns_str, const cppast::cpp_entity& e, const std::string& line_generator)
	{
		std::string retStr = "";
		if(e.kind() == cppast::cpp_entity_kind::include_directive_t)
		{
			std::string importDef = "";
			importDef = line_generator;

			// 標準C/C++ ヘッダファイルかどうかチェックする。
				// std::find(c_header_list.begin(), c_header_list.end(), '標準C header')
					// repace_c_import = "libc." + c_header_list.erase(-2)
			    	// importDef.replace(c_header_list, repace_c_import);

				// std::find(cpp_header_list.begin(), cpp_header_list.end(), '標準C++ header')
					// repace_cpp_import = "libcpp." + cpp_header_list;
			    	// importDef.replace(c_header_list, repace_cpp_import);

			// ".h" remove
			importDef.erase(std::find(s.begin(), s.end(), ".h"));
			// #include -> import
			// 
			importDef.replace("#include", "import");

			// "/" to "."
			importDef.replace('/', '.');
			retStr = importDef;
		}
		else if(e.kind() == cppast::cpp_entity_kind::class_t)
		{
			std::string classDef = "";
			std::string indentSpace = "    ";

			// base_filename
			// first line
			classDef = "cdef extern from \"" + base_filename + "\" namespace \"" + ns_str + "\":" + "\n"
			// second line
			classDef += indentSpace;
			classDef += "cdef cppclass ";
			classDef += "className";
				// typedef 型の場合
				// classDef += "[";
					// classDef += 
					// classDef += ", "
				// classDef += "]";
			classDef += ":";
			
			retStr = classDef;
		}
		else
		{
		}

		return retStr;
	}

/*
    // ファイルに依存する別ファイル？といった認識でOK? == cppast の visit
    auto visit(auto node)
    {
        this->visitStack.push(node);
        rv = super(AutoPxd, self).visit(node);
        n = this->visitStack.pop();
        assert(n == node);
        return rv;
    }

    // 変数の種類?を設定
    void visit_IdentifierType(auto node)
    {
        this->append(' '.join(node.names))
    }

    // 
    void visit_Block(auto node, auto kind)
    {
        name = node.name;
        if not name)
        {
            name = this->path_name(kind[0]);
        }

        if(not node.decls)
        {
            if this->child_of(c_ast.TypeDecl, -2):
                # not a definition, must be a reference
                this->append(name)
            return;

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
        string::vector<std::string> items;
        if node.values:
            for item in node.values.enumerators:
                items.append(item.name)
        name = node.name
        type_decl = this->child_of(c_ast.TypeDecl, -2)

        if not name and type_decl
        {
            name = this->path_name('e')
        }

        # add the enum definition to the top level
        if len(items)
        {
            this->decl_stack[0].append(Enum(name, items))
        }
        
        if type_decl
        {
            this->append(name)
        }
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
        std::vector<auto> decls;
        this->decl_stack.append(decls)
        name = this->generic_visit(node)
        assert this->decl_stack.pop() == decls
        return decls
    }

    auto path_name(auto tag)
    {
        std::vector<std::string> names;
        for node in this->visitStack[:-2]:
            if hasattr(node, 'declname') and node.declname:
                names.append(node.declname)
            elif hasattr(node, 'name') and node.name:
                names.append(node.name)
        return '_{0}_{1}'.format('_'.join(names), tag)
    }

    auto child_of(auto type, auto index=None)
    {
        if index is None
        {
            for node in reversed(this->visitStack)
            {
                if isinstance(node, type)
                {
                    return True
                }

                return False
            }
        }
        else
        {
            return isinstance(this->visitStack[index], type)
        }
    }

    void append(node)
    {
        this->decl_stack[-1].append(node)
    }

    std::vector<std::string> lines()
    {
        rv = ['cdef extern from "{0}":'.format(this->hdrname), ''];
        for decl in this->decl_stack[0];
        {
            for line in decl.lines()
            {
                rv.append(this->indent + line);
            }

            rv.append('');
        }

        return rv;
    }
*/
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

*/
