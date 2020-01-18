/*
class PxdImport
{
private:
    std::string includeName;

public:
    PxdImport() {}
    virtual ~PxdImport() {}

};
*/
/*! @brief  変数などの定義？
*/
/*
class IdentifierType : public PxdNode
{
private:
    std::string name;
    std::string type_name;

public:
    IdentifierType(std::string name, std::string type_name)
    {
        this->name = name;
        this->type_name = type_name;
    }
    virtual ~IdentifierType() {}

    std::string lines()
    {
        if(this->name.empty())
        {
            return sprintf("%s %s", this->type_name.c_str(), this->name.c_str());
        }
        else
        {
            return this->type_name;
        }
    }
};
*/
/*! @brief  関数の定義
*/
/*
class Function : public PxdNode
{
private:
    std::string return_type;
    std::string name;
    // std::string args;
    std::vector<std::string> args;

public:
    // Function(std::string return_type, std::string name, std::string args)
    Function(std::string return_type, std::string name, std::vector<std::string> args)
    {
        this->return_type = return_type;
        this->name = name;
        this->args = args;
    }
    virtual ~Function() {}

    // @brief 関数内の引数の定義を文字列化
    std::string argstr()
    {
        std::vector<std::string> l;
        for(auto arg in this->args)
        {
            lines = arg.lines()
            assert(len(lines) == 1)
            l.append(lines[0])
        }
        return ', '.join(l)
    }

    std::string lines()
    {
        return sprintf("%s %s(%s)", this->return_type, this->name, this->argstr());
    }
};
*/
/*! @brief  ポインタ変数の定義
*/
/*
class Ptr : public IdentifierType
{
private:
    auto node;

public:
    Ptr(auto node)
    {
        this->node = node;
    }
    virtual ~Ptr() {}

    // @property
    std::string name()
    {
        return this->node.name;
    }

    // @property
    std::string type_name()
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
*/
/*! @brief  定数[const]変数の定義
    @remark Cython での対応
*/
/*
class ConstRef : public IdentifierType
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

    // @property
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
*/

/*! @brief  配列変数に関する対応をまとめたクラス
*/
/*
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
    auto name(self)
    {
        return this->node.name + '[1]'
    }

    // @property
    auto type_name(self)
    {
        return this->node.type_name
    }
};
*/

/*! @brief  typedef 定義に関する対応をまとめたクラス
*/
/*
class Type : public PxdNode
{
private:
    // node クラスを用意する?
    auto node
public:
    Type(auto node)
    {
        this->node = node;
    }

    std::vector<std::string> lines(self)
    {
        lines = this->node.lines()
        lines[0] = 'ctypedef ' + lines[0]
        return lines
    }
};
*/

/*! @brief  
*/
/*
class Block : public PxdNode
{
private:
    std::string name;
    std::vector<std::string> fields;
    std::string kind;

public:
    Block(std::string name, std::vector<std::string> fields, std::string kind)
    {
        this->name = name;
        this->fields = fields;
        this->kind = kind;
    }

    std::vector<double> lines()
    {
        // rv = ['cdef {0} {1}:'.format(this->kind, this->name)]
        for(auto field in this->fields)
        {
            for(line in field.lines() )
            {
                rv.append(this->indent + line);
            }
        }

        return rv
    }
}
*/
/*! @brief  Enum 定義の対応
    @remark not In Class
*/
/*
class Enum : pubic PxdNode
{
private:
    std::string name;
    std::vector<std::string> items;

public:
    Enum(std::string name, std::vector<std::string> items)
    {
        this->name = name;
        this->items = items;
    }

    std::vector<std::string> lines()
    {
        std::vector<std::string> rv;
        if(!this->name.empty())
        {
            rv.append('cdef enum {0}:'.format(this->name))
        }
        else
        {
            rv.append('cdef enum:')
        }

        for(auto item in this->items)
        {
            rv.append(this->indent + item);
        }

        return rv
    }
};
*/
/*! @brief  Enum 定義の対応
    @remark In Class define
*/
/*
class EnumInClass : pubic PxdNode
{
private:
    std::string name;
    std::vector<std::string> items;
    std::string class;  // ?

public:
    Enum(std::string name, std::vector<std::string> items)
    {
        this->name = name;
        this->items = items;
    }

    std::vector<std::string> lines()
    {
        std::vector<std::string> rv;
        if(!this->name.empty())
        {
            rv.append('cdef enum {0}:'.format(this->name))
        }
        else
        {
            rv.append('cdef enum:')
        }

        for(auto item in this->items)
        {
            rv.append(this->indent + item);
        }

        return rv
    }
};
*/

// interface ClassDefine
// {
// };
