#pragma once

// #include <assert>

#include <string>
#include <vector>
// #include <format> // C++20

// code_generator
#include "generator/IGenerator.h"
#include "generator/CommentGenerator.h"
#include "generator/FloatliteralGenerator.h"
#include "generator/IdentifierGenerator.h"
#include "generator/IntliteralGenerator.h"
#include "generator/KeywordGenerator.h"
#include "generator/PreprocessorGenerator.h"
#include "generator/PunctuationGenerator.h"
#include "generator/ReferenceGenerator.h"
#include "generator/StrliteralGenerator.h"
#include "generator/TokenGenerator.h"

// http://pyopyopyo.hatenablog.com/entry/2019/02/08/102456
template <typename ... Args>
std::string format(const std::string& fmt, Args ... args )
{
    size_t len = std::snprintf( nullptr, 0, fmt.c_str(), args ... );
    std::vector<char> buf(len + 1);
    std::snprintf(&buf[0], len + 1, fmt.c_str(), args ... );
    return std::string(&buf[0], &buf[0] + len);
}


class PxdNode
{
public:
    //PxdNode()=default;
    //virtual ~PxdNode()=default;
    //PxdNode(const PxdNode &obj) {}
    //PxdNode(const PxdNode&) = delete;

public:
	// 
	// virtual void writeline(cppast::cpp_entity_kind kind, std::vector<IGenerator*> generators) = 0;
	virtual void writeline(std::vector<IGenerator*> generators) = 0;

	// 
	virtual void calc() = 0;

    virtual std::string lines() = 0;
    // {
    //     return std::string("");
    // }

    virtual std::vector<std::string> lines2() = 0;
    // {
    //     std::vector<std::string> param;
    //     return param;
    // }

    //virtual std::string __str__()
    //{
    //    return std::string("\n"); // + std::string(this->lines());
    //}
};

/*
class IdentifierType : public PxdNode
{
public:
    IdentifierType() = default;
    IdentifierType(const std::string& name, const std::string& type_name)
    {
        this->name = name;
        this->type_name = type_name;
    }
    virtual ~IdentifierType() = default;

    std::string lines() override
    {
        if(!this->name.empty())
        {
            return format("%s %s", this->type_name, this->name);
        }
        else
        {
            return this->type_name;
        }
    }

public:
    std::string name;
    std::string type_name;
};

class Function : public PxdNode
{
public:
    Function() = default;
    Function(const std::string& return_type, const std::string& name, const std::vector<PxdNode>& args)
    {
        this->return_type = return_type;
        this->name = name;
        this->args = args;
    }
    virtual ~Function() = default;

    std::string argstr()
    {
        // std::vector<std::string> l; // = new std::vector<std::string>();
        std::string l;
        // for(PxdNode arg : this->args)
        // {
        //     std::string lines = arg.lines();
        //     // assert(len(lines) == 1);
        //     // l.push_back(lines);
        //     l += lines;
        // }
        return ", " + l;
    }

    std::string lines() override
    {
        return format("{} {}({})", this->return_type, this->name, this->argstr());
    }

public:
    std::string return_type;
    std::string name;
    std::vector<PxdNode> args;

};

class Ptr : public IdentifierType
{
public:
    IdentifierType* node;

public:
    Ptr() = default;
    virtual ~Ptr() = default;
    Ptr(IdentifierType* node)
    {
        this->node = node;
    }

    std::string name()
    {
        return this->node->name;
    }

    std::string type_name()
    {
        return this->node->type_name + "*";
    }

    std::string lines() override
    {
        // if(typeid(this->node) == typeid(Function))
        // {
        //     auto f = this->node;
        //     auto args = f->argstr();
        //     return format("{0} (*{1})({2})", f->return_type, f->name, args);
        // }
        // else
        // {
            return ((PxdNode*)this)->lines();
        // }
    }
};

class Array : public IdentifierType
{
public:
    IdentifierType* node;

public:
    Array() = default;
    virtual ~Array() = default;

    Array(IdentifierType* node)
    {
        this->node = node;
    }

    // @property
    std::string name()
    {
        return this->node->name + "[1]";
    }

    // @property
    std::string type_name()
    {
        return this->node->type_name;
    }
};

class Type : public PxdNode
{
public:
    PxdNode* node;

public:
    Type() = default;
    virtual ~Type() = default;

    Type(PxdNode* node)
    {
        this->node = node;
    }

    // std::vector<std::string> lines()
    std::vector<std::string> lines2() override
    {
        std::vector<std::string> lines = this->node->lines2();
        lines[0] = "ctypedef " + lines[0];
        return lines;
    }
};

class Block : public PxdNode
{
public:
    std::string name;
    std::vector<PxdNode> fields;
    std::string kind;

public:
    const std::string indent = "    ";

public:
    Block() = default;
    virtual ~Block() = default;

    Block(const std::string& name, const std::vector<PxdNode>& fields, const std::string& kind)
    {
        this->name = name;
        this->fields = fields;
        this->kind = kind;
    }

    // std::vector<std::string> lines()
    std::vector<std::string> lines2() override
    {
        std::vector<std::string> rv;
        rv.push_back(format("cdef {0} {1}:",this->kind, this->name));
        // for(PxdNode field : this->fields)
        // {
        //     for(std::string strline : field.lines2())
        //     {
        //         rv.push_back(((PxdNode*)this)->indent + strline);
        //     }
        // }
        return rv;
    }
};
*/

class EnumNode : public PxdNode
{
public:
    std::string name;
    // std::vector<PxdNode> items;
    std::vector<std::string> items;
    std::string statement="cdef";

public:
    const std::string indent = "    ";

public:
    EnumNode() = default;
    virtual ~EnumNode() = default;

    EnumNode(const std::string& name, const std::vector<std::string>& items)
    {
        this->name = name;
        this->items = items;
    }
	
	// C/C++ �̕������ʂ�ǉ�
	void writeline(std::vector<IGenerator*> generators) override
	{
		// 
	}

	void calc() override
	{
		// 
	}

    std::string lines() override
    {
        return std::string("");
    }

    std::vector<std::string> lines2() override
    {
        std::vector<std::string> rv = {};
        for (auto& item : this->items)
        {
            // rv.push_back(((PxdNode*)this)->indent + item);
            //rv.push_back(this->indent + item);
        	rv.push_back(item);
        }
        return rv;
    }
};

class ClassPxdNode : public PxdNode
{
public:
    std::string name;
    // std::vector<PxdNode> items;
    std::vector<std::string> items;
    std::string statement="cdef";

public:
    const std::string indent = "    ";

public:
    ClassPxdNode() = default;
    virtual ~ClassPxdNode() = default;

    ClassPxdNode(const std::string& name, const std::vector<std::string>& items)
    {
        this->name = name;
        this->items = items;
    }
	
	// C/C++ �̕������ʂ�ǉ�
	void writeline(std::vector<IGenerator*> generators) override
	{
		// 
	}

	void calc() override
	{
		// 
	}

    std::string lines() override
    {
        return std::string("");
    }

    std::vector<std::string> lines2() override
    {
        std::vector<std::string> rv = {};
        for (auto& item : this->items)
        {
            // rv.push_back(((PxdNode*)this)->indent + item);
            //rv.push_back(this->indent + item);
        	rv.push_back(item);
        }
        return rv;
    }
};

class TemplateClassPxdNode : public PxdNode
{
public:
    std::string name;
    // std::vector<PxdNode> items;
    std::vector<std::string> items;
    std::string statement="cdef";

public:
    const std::string indent = "    ";

public:
    TemplateClassPxdNode() = default;
    virtual ~TemplateClassPxdNode() = default;

    TemplateClassPxdNode(const std::string& name, const std::vector<std::string>& items)
    {
        this->name = name;
        this->items = items;
    }
	
	// C/C++ �̕������ʂ�ǉ�
	void writeline(std::vector<IGenerator*> generators) override
	{
		// 
	}

	void calc() override
	{
		// 
	}

    std::string lines() override
    {
        return std::string("");
    }

    std::vector<std::string> lines2() override
    {
        std::vector<std::string> rv = {};
        for (auto& item : this->items)
        {
            // rv.push_back(((PxdNode*)this)->indent + item);
            //rv.push_back(this->indent + item);
        	rv.push_back(item);
        }
        return rv;
    }
};

class TemplateFunctionPxdNode : public PxdNode
{
public:
    std::string name;
    // std::vector<PxdNode> items;
    std::vector<std::string> items;
    std::string statement="cdef";

public:
    const std::string indent = "    ";

public:
    TemplateFunctionPxdNode() = default;
    virtual ~TemplateFunctionPxdNode() = default;

    TemplateFunctionPxdNode(const std::string& name, const std::vector<std::string>& items)
    {
        this->name = name;
        this->items = items;
    }
	
	// C/C++ �̕������ʂ�ǉ�
	void writeline(std::vector<IGenerator*> generators) override
	{
		// 
	}

	void calc() override
	{
		// 
	}

    std::string lines() override
    {
        return std::string("");
    }

    std::vector<std::string> lines2() override
    {
        std::vector<std::string> rv = {};
        for (auto& item : this->items)
        {
            // rv.push_back(((PxdNode*)this)->indent + item);
            //rv.push_back(this->indent + item);
        	rv.push_back(item);
        }
        return rv;
    }
};

// TODO: �Ώۃ��W���[���̃C���N���[�h�p�X���擾
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
