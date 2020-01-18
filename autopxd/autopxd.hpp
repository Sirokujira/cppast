#include <iostream>
#include <fstream>
#include <stack>

#include <cppast/libclang_parser.hpp>        // for libclang_parser, libclang_compile_config, cpp_entity,...
#include <cppast/visitor.hpp>                // for visit()
#include <cppast/code_generator.hpp>         // for generate_code()
// override parameter
#include <cppast/cpp_entity.hpp>
#include <cppast/cpp_entity_ref.hpp>

#include <cppast/cpp_entity_kind.hpp>        // for the cpp_entity_kind definition
#include <cppast/cpp_forward_declarable.hpp> // for is_definition()
#include <cppast/cpp_namespace.hpp>          // for cpp_namespace

/*! @brief ベースとなる定義(indent の取り方など)
    @remark どれにとってのベース？(
*/
class PxdNode
{
private:
    const std::string indent = "    ";

public:
    PxdNode() {}
    virtual ~PxdNode() {}
    virtual std::string __str__()
    {
        return std::string("\n"); // + std::string(this->lines());
    }
};

class PxdLine
{
private:
    const std::string token = " ";

public:
    PxdLine() {}
    virtual ~PxdLine() {}
};

// Class 
#include <fstream>
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

/*! @brief  Pxd ファイル生成の大本クラス
    @remark cppast 側で対処可能な項目との切り分けを見ておくこと
            code_generator の定義はこちらに書くべき?
*/
// class AutoPxd : public PxdNode
class AutoPxd
{
private:
    // hファイル(root ファイル)
    // namespace(階層になるケースがあるため複数?)
    // class定義(階層になるケースがあるため複数?)
    // template付 class定義
    // template 定義
    // struct 定義
    // → 上記パラメータを同一で扱えるように基底クラスを用意する?

    // pxd ファイル書き出し用
    std::string autopxd_file;
    std::ofstream fout_pxd;
    std::string base_filename;

    // 記述ルール取り込み?
    // std::stack<std::stack<auto>> declStack;
    // std::stack<auto> visitStack;

    // std::vector<std::string> pxd_lines;
    std::string namespace_str;
    std::string class_access_str;
    std::stack<PxdNode> nodes;
    std::vector<std::string> define_lines;
    bool isClassAccessPublic;
    bool isEnumClassInFlag;
    int indendCount;

    // class 内の Template について格納
    // (Template 付関数呼び出し時の引数をチェックするために使用する。)
    // class に複数の Template が設定されている可能性があるため、以下の形にしている。
    std::vector<std::string> classTemplates;
    std::string rootPath = "/home/sirokujira/draco_py/draco/src/";

public:
    AutoPxd(const std::string& filename)
    {
        // そのまま設定すると、絶対パスになるため
        base_filename = filename;
        // 相対パスとして設定すること。
        // (後のファイル生成に影響を与えている?)
        // myReplace(base_filename, rootPath, "");

        // 拡張子取り除き
        // int path_i = base_filename.find_last_of("\\") + 1;
        int path_i = base_filename.find_last_of("/") + 1;
        int ext_i = base_filename.find_last_of(".");
        std::string filename_without_ext = base_filename.substr(path_i, ext_i - path_i);
        std::string extname = base_filename.substr(ext_i, base_filename.size() - ext_i);

        // this->declStack = new std::stack<std::stack<auto>>();
        // this->visitStack = new std::stack<auto>();
        // autopxd_file = base_filename + ".pxd";
        autopxd_file = filename_without_ext + ".pxd";
        namespace_str = "";
        class_access_str = "";
        isClassAccessPublic = false;
        isEnumClassInFlag = false;
        indendCount = 0;

        // open target write pxd file
        fout_pxd.open(autopxd_file, std::ios::out);
    }

    virtual ~AutoPxd()
    {
        // close target write pxd file
        fout_pxd.close();
    }

    // prints the AST of a file
    void autopxd_ast(const cppast::cpp_file& file)
    {
        // print file name
        // fout_pxd << "AST for '" << file.name() << "':\n";
        std::string prefix; // the current prefix string

        // recursively visit file and all children
        cppast::visit(file, [&](const cppast::cpp_entity& e, cppast::visitor_info info) {
            
            if (e.kind() == cppast::cpp_entity_kind::file_t 
                || cppast::is_templated(e)
                || cppast::is_friended(e))
                // no need to do anything for a file,
                // templated and friended entities are just proxies, so skip those as well
                // return true to continue visit for children
                return true;
            else if (info.event == cppast::visitor_info::container_entity_exit)
            {
                // namespace 内に複数のクラス/構造体が定義されている場合を考慮
                // クラス/構造体を１つずつ管理する？
                indendCount--;
                // fout_pxd << "container_entity_exit" << indendCount << '\n';
                // we have visited all children of a container,
                // remove prefix
                // 接頭文字 - 2文字削除
                prefix.pop_back();
                prefix.pop_back();
            }
            /*
            else if (info.event == cppast::visitor_info::container_entity_enter)
            {
                // fout_pxd << "container_entity_enter";
            }
            */
            // 役割は？
            // else if (info.event == cppast::visitor_info::leaf_entity)
            // {
            //    fout_pxd << "leaf_entity";
            // }
            else
            {
                // fout_pxd << info.event;
                
                // 階層情報の検索
                // out << prefix; // print prefix for previous entities
                // calculate next prefix
                if (info.last_child)
                {
                    if (info.event == cppast::visitor_info::container_entity_enter)
                    {
                        indendCount++;
                        // fout_pxd << "(lastChild)container_entity_enter" << indendCount << '\n';
                        prefix += "  ";
                    }
                    else if(info.event == cppast::visitor_info::leaf_entity)
                    {
                        fout_pxd << "leaf_entity";
                    }

                    // 開始端?(階層+1)
                    // out << "+-";
                }
                else
                {
                    if (info.event == cppast::visitor_info::container_entity_enter)
                    {
                        indendCount++;
                        // fout_pxd << "(not lastChild)container_entity_enter" << indendCount << '\n';
                        prefix += "| ";
                    }

                    // 階層継続?
                    // out << "|-";
                }

                // print_entity(out, e);
                // autopxd_entity(out, e);
                // AutoPxd に専用の関数を用意する。(そっちの方が対処が楽なため)
                autopxd_entity(e);
            }

            return true;
        });
    }

    void autopxd_entity(const cppast::cpp_entity& e)
    {
        bool isDefine;
        isDefine = cppast::is_definition(e);

        /*
        // print name and the kind of the entity
        if (!e.name().empty())
        {
            // 対象項目
            fout_pxd << e.name();
        }
        else
        {
            // 変数がよくわからないとき(using とかもここに入る)
            fout_pxd << "<anonymous>";
        }
        */

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
        fout_pxd << " (" << cppast::to_string(e.kind()) << ")" << "\n";
        // std::string pxd_line = "";
        /*
        switch(e.kind())
        {
            // cpp_entity_kind.hpp を参照
            case cppast::cpp_entity_kind::namespace_t:
                // cast to cpp_namespace
                auto& ns = static_cast<const cppast::cpp_namespace&>(e);
                // print whether or not it is inline
                if (ns.is_inline())
                {
                    // inline 関数?
                    // fout_pxd << " [inline]";
                }
                // fout_pxd << '\n';

                if(!namespace_str.empty())
                {
                    namespace_str += "::";
                }
                namespace_str += e.name();
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
                // IGenetator?
                std::vector<IGenerator*> generators;
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

                std::vector<IGenerator*>& generatorLists()
                {
                    return generators;
                }

            private:
                cppast::formatting do_get_formatting() const override
                {
                    std::cout << "do_get_formatting";
                    std::cout << "\n";

                    return {};
                }
                // called to retrieve the generation options of an entity
                generation_options do_get_options(const cppast::cpp_entity&,
                                                  cppast::cpp_access_specifier_kind) override
                {
                    std::cout << "do_get_options";
                    std::cout << "\n";

                    // generate declaration only
                    return pxd_generator::declaration;
                }

                /// \effects Will be invoked before code of an entity is generated.
                /// The base class version has no effect.
                void on_begin(const output& out, const cppast::cpp_entity& e) override
                {
                    // str_ += "on_begin";
                    // str_ += "\n";
                    std::cout << "on_begin";
                    std::cout << "\n";

                    (void)out;
                    (void)e;
                }

                /// \effects Will be invoked after all code of an entity has been generated.
                /// The base class version has no effect.
                void on_end(const output& out, const cppast::cpp_entity& e) override
                {
                    // str_ += "on_end";
                    // str_ += "\n";
                    std::cout << "on_end";
                    std::cout << "\n";

                    (void)out;
                    (void)e;
                }

                void on_container_end(const output& out, const cppast::cpp_entity& e) override
                {
                    std::cout << "on_container_end";
                    std::cout << "\n";

                    (void)out;
                    (void)e;
                }

                // no need to handle indentation, as only a single line is used
                void do_indent() override {}
                void do_unindent() override {}

                void do_write_keyword(cppast::string_view str)
                {
                    // std::cout << "do_write_keyword";
                    // std::cout << "\n";

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

                    // 2
                    IGenerator* generator = new KeywordGenerator();
                    generator->SetString(str.c_str());
                    generators.push_back(generator);
                }

                void do_write_identifier(cppast::string_view str)
                {
                    // std::cout << "do_write_identifier";
                    // std::cout << "\n";

                    str_ += "\n";
                    str_ += "identifier : ";
                    // AttributeOctahedronTransform::
                    str_ += "\n";
                    str_ += str.c_str();

                    // 2
                    IGenerator* generator = new IdentifierGenerator();
                    generator->SetString(str.c_str());
                    generators.push_back(generator);
                }

                void do_write_reference(cppast::string_view str)
                {
                    // std::cout << "do_write_reference";
                    // std::cout << "\n";

                    str_ += "\n";
                    str_ += "reference : ";
                    str_ += "\n";
                    str_ += str.c_str();

                    // 2
                    IGenerator* generator = new ReferenceGenerator();
                    generator->SetString(str.c_str());
                    generators.push_back(generator);
                }

                void do_write_punctuation(cppast::string_view str)
                {
                    // std::cout << "do_write_punctuation";
                    // std::cout << "\n";

                    str_ += "\n";
                    str_ += "punctuation : ";
                    // <PointIndex
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

                    // 2
                    IGenerator* generator = new PunctuationGenerator();
                    generator->SetString(str.c_str());
                    generators.push_back(generator);
                }

                void do_write_str_literal(cppast::string_view str)
                {
                    // std::cout << "do_write_str_literal";
                    // std::cout << "\n";

                    str_ += "\n";
                    str_ += "str : ";
                    str_ += "\n";
                    str_ += str.c_str();

                    // 2
                    IGenerator* generator = new StrliteralGenerator();
                    generator->SetString(str.c_str());
                    generators.push_back(generator);
                }

                void do_write_int_literal(cppast::string_view str)
                {
                    // std::cout << "do_write_int_literal";
                    // std::cout << "\n";
                    str_ += "\n";
                    str_ += "int : ";
                    str_ += "\n";
                    str_ += str.c_str();

                    // 2
                    IGenerator* generator = new IntliteralGenerator();
                    generator->SetString(str.c_str());
                    generators.push_back(generator);
                }

                void do_write_float_literal(cppast::string_view str)
                {
                    // std::cout << "do_write_float_literal";
                    // std::cout << "\n";

                    str_ += "\n";
                    str_ += "float : ";
                    str_ += "\n";
                    str_ += str.c_str();

                    // 2
                    IGenerator* generator = new FloatliteralGenerator();
                    generator->SetString(str.c_str());
                    generators.push_back(generator);
                }

                void do_write_preprocessor(cppast::string_view str)
                {
                    // std::cout << "do_write_preprocessor";
                    // std::cout << "\n";

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

                    // 2
                    IGenerator* generator = new PreprocessorGenerator();
                    generator->SetString(str.c_str());
                    generators.push_back(generator);
                }

                void do_write_comment(cppast::string_view str)
                {
                    // std::cout << "do_write_comment";
                    // std::cout << "\n";

                    str_ += "\n";
                    str_ += "comment : ";
                    str_ += "\n";
                    str_ += str.c_str();

                    // 2
                    IGenerator* generator = new CommentGenerator();
                    generator->SetString(str.c_str());
                    generators.push_back(generator);
                }

                // called when a generic token sequence should be generated
                // there are specialized callbacks for various token kinds,
                // to e.g. implement syntax highlighting
                void do_write_token_seq(cppast::string_view tokens) override
                {
                    // std::cout << "do_write_token_seq";
                    // std::cout << "\n";

                    if (was_newline_)
                    {
                        // lazily append newline as space
                        str_ += ',';
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
            // fout_pxd << autopxd_generator2(namespace_str, e, generator.generatorLists()) << '\n';
        }
    }

    // 特殊対応
    // header
    // standard c header list
    std::vector<std::string> c_header_lists { "<errno.h>", "<float.h>", "<limits.h>", "<locale.h>", "<math.h>", "<setjmp.h>", "<signal.h>", "<stddef.h>", "<stdint.h>", "<stdio.h>", "<stdlib.h>", "<string.h>", "<time.h>" };
    // standard c++ header list
    std::vector<std::string> cpp_header_lists { "<algorithm>", "<cast>", "<complex>", "<deque>", "<forward_list>", "<functional>", "<iterator>", "<limits>", "<list>", "<map>", "<memory>", "<pair>", "<queue>", "<set>", "<stack>", "<string>", "<typeindex>", "<typeinfo>", "<unordered_map>", "<unordered_set>", "<utility>", "<vector>" };
    // posix header list
    std::vector<std::string> posix_header_lists { "<dlfcn.h>", "<fcntl.h>", "<sys/ioctl.h>", "<sys/mman.h>", "<sys/resource.h>", "<sys/select.h>", "<signal.h>", "<sys/stat.h>", "<stdio.h>", "<stdlib.h>", "<strings.h>", "<sys/time.h>", "<sys/types.h>", "<unistd.h>", "<sys/wait.h>" };
    // reject header list
    // std::vector<std::string> reject_header_lists { "<windows.h>" };
    // cimport <cinttypes>
    // cimport <cstddef>
    // cimport <cstring>
    // cimport <ostream>

    // c header 読み出し+
    // c - stdint 読み込み時に展開する定義
    // cython が使用する標準で定義されている型情報リスト
    // uint32_t 等が使用されている時の対応はどうするか
    // from libc.stdint cimport int8_t
    // from libc.stdint cimport uint32_t
    // :
    std::vector<std::string> cstd_type_lists { "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "size_t" };

    // cpp header 読み出し+
    // memory 呼び出し時の
    // cpp ファイルの定義として使用する型定義を追加
    std::vector<std::string> cpp_type_lists { "bool", "nullptr_t", "nullptr" };
    std::vector<std::string> cpp_memory_import_lists { "unique_ptr", "shared_ptr", "weak_ptr" };

    // class 内の Enum 値を使用する場合は?(定義が後になるケース?[あるかどうかも調査]の対応)
    // operator function の対応についてどうするか。

    // 文字列置き換え
    void myReplace(std::string& str,
               const std::string& oldStr,
               const std::string& newStr)
    {
        std::string::size_type pos = 0u;
        while((pos = str.find(oldStr, pos)) != std::string::npos)
        {
            str.replace(pos, oldStr.length(), newStr);
            pos += newStr.length();
        }
    }

    std::string autopxd_generator(const std::string& ns_str, const cppast::cpp_entity& e, const std::string& line_generator)
    {
        std::string retStr = "";
        if(e.kind() == cppast::cpp_entity_kind::include_directive_t)
        {
            // TODO : Cython が標準で対応できないヘッダファイルに関しては、再帰的にヘッダを解析していく仕組みを実装する?
            // 現時点での対応は面倒なため、対応しないことにする
            std::string importDef = "";
            importDef = line_generator;
            bool isStandard = false;

            // standard header list
            // start
            // 標準 C ヘッダファイルかどうかチェックする。
            for(auto itr = c_header_lists.begin(); itr != c_header_lists.end(); ++itr)
            {
                // '標準C header check'
                if(std::string::npos != importDef.find(*itr))
                {
                    std::string repace_c_import_header = *itr;
                    myReplace(repace_c_import_header, "<", "");
                    myReplace(repace_c_import_header, ">", "");

                    // cimport の対応
                    std::string repace_c_import = "";
                    // TODO : Cython として取り出したい import 内容と ヘッダファイル名が一致していないケースもあるので注意して実装する。
                    if (repace_c_import_header == "stdint.h")
                    {
                        // stdint 読み出し
                        // repace_c_import = "from libc." + repace_c_import_header + " " + "cimport " + repace_c_import_header;
                        // memory なら unique_ptr とか?
                        for(auto itr2 = cstd_type_lists.begin(); itr2 != cstd_type_lists.end(); ++itr2)
                        {
                            repace_c_import += "from libc.";
                            repace_c_import += repace_c_import_header;
                            repace_c_import += " ";
                            repace_c_import += "cimport ";
                            repace_c_import += *itr2;
                            repace_c_import += "\n";
                        }
                    }
                    else
                    {
                        repace_c_import = "from libc." + repace_c_import_header + " " + "cimport " + repace_c_import_header;
                    }

                    myReplace(importDef, *itr, repace_c_import);
                    isStandard = true;
                    break;
                }
            }

            for(auto itr = cpp_header_lists.begin(); itr != cpp_header_lists.end(); ++itr)
            {
                // '標準 C++ header check'
                if(std::string::npos != importDef.find(*itr))
                {
                    std::string repace_cpp_import_header = *itr;
                    myReplace(repace_cpp_import_header, "<", "");
                    myReplace(repace_cpp_import_header, ">", "");

                    // cimport の対応
                    std::string repace_cpp_import = "";
                    if(repace_cpp_import_header == "memory")
                    {
                        // memory なら unique_ptr とか?
                        for(auto itr2 = cpp_memory_import_lists.begin(); itr2 != cpp_memory_import_lists.end(); ++itr2)
                        {
                            repace_cpp_import += "from libcpp.";
                            repace_cpp_import += repace_cpp_import_header;
                            repace_cpp_import += " ";
                            repace_cpp_import += "cimport ";
                            repace_cpp_import += *itr2;
                            repace_cpp_import += "\n";
                        }
                    }
                    else
                    {
                        // TODO : 取り出したい内容が ファイル名と一致していないケースもあるので注意して実装する。
                        repace_cpp_import = "from libcpp." + repace_cpp_import_header + " " + "cimport " + repace_cpp_import_header;
                    }

                    myReplace(importDef, *itr, repace_cpp_import);
                    isStandard = true;
                    break;
                }
            }

            for(auto itr = posix_header_lists.begin(); itr != posix_header_lists.end(); ++itr)
            {
                // 'posix header check'
                if(std::string::npos != importDef.find(*itr))
                {
                    std::string repace_posix_import_header = *itr;
                    myReplace(repace_posix_import_header, "<", "");
                    myReplace(repace_posix_import_header, ">", "");

                    std::string repace_posix_import = "from posix." + repace_posix_import_header + " " + "cimport " + repace_posix_import_header;
                    myReplace(importDef, *itr, repace_posix_import);
                    isStandard = true;
                    break;
                }
            }
            // end

            // custom header list?

            // ".h" remove
            // importDef.erase(std::find(line_generator.begin(), line_generator.end(), ".h"));
            myReplace(importDef, ".h", "");
            // #include -> import
            // std::replace(importDef.begin(), importDef.end(), "#include", "cimport");
            if(isStandard)
            {
                myReplace(importDef, "#include", "");
            }
            else
            {
                // 
                myReplace(importDef, "#include", "cimport");
            }

            // "/" to "."
            std::replace(importDef.begin(), importDef.end(), '/', '.');
            // "\"" remove
            myReplace(importDef, "\"", "");

            retStr = importDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::class_t)
        {
            std::string classDef = "";
            std::string indentSpace = "    ";

            // base_filename
            // first line
            classDef = "cdef extern from \"" + base_filename + "\" namespace \"" + ns_str + "\":" + "\n";
            // second line
            classDef += indentSpace;
            // classDef += "cdef cppclass ";
            // classDef += "className";
            classDef += "cdef cpp";
            classDef += line_generator;
            classDef += ":";

            // ベースクラス継承する場合
            // (
            // Action
            // )

            // Debug
            // classDef += "\n";
            // classDef += "Debug Str : ";
            // classDef += line_generator;

            retStr = classDef;
            isClassAccessPublic = false;
        }
        else if (e.kind() == cppast::cpp_entity_kind::class_template_t)
        {
            std::string classTemplateDef = "";
            std::string indentSpace = "    ";

            // first line
            classTemplateDef = "cdef extern from \"" + base_filename + "\" namespace \"" + ns_str + "\":" + "\n";
            // second line
            classTemplateDef += indentSpace;
            classTemplateDef += "cdef cppclass ";
            classTemplateDef += "className";
            // template 付の class の場合
            classTemplateDef += "[";
                classTemplateDef += "Test";
                // classTemplateDef += ", "
            classTemplateDef += "]";
            classTemplateDef += ":";

            // ベースクラス継承する場合
            // (
            // Action
            // )

            // Debug
            classTemplateDef += "\n";
            classTemplateDef += "Debug Str : ";
            classTemplateDef += line_generator;

            retStr = classTemplateDef;
            isClassAccessPublic = false;
        }
        else if (e.kind() == cppast::cpp_entity_kind::access_specifier_t)
        {
            // 権限が public 以外は、書き出さない。
            // case cppast::cpp_entity_kind::access_specifier_t:
            if(line_generator.compare("public:") == 0)
            {
                isClassAccessPublic = true;
            }
            else
            {
                isClassAccessPublic = false;
            }
        }
        else if(e.kind() == cppast::cpp_entity_kind::constructor_t)
        {
            std::string constructorTemplateDef = "";
            std::string indentSpace = "    ";

            // construct
            if(isClassAccessPublic)
            {
                constructorTemplateDef += "Debug Str : ";
                constructorTemplateDef += line_generator;
            }

            // public 以外での対応をどうするか?
            // 現状 : 書かない
            retStr = constructorTemplateDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::destructor_t)
        {
            std::string destructorTemplateDef = "";
            std::string indentSpace = "    ";
            
            // destruct
            if(isClassAccessPublic)
            {
                destructorTemplateDef += "Debug Str : ";
                destructorTemplateDef += line_generator;
            }
            // public 以外での対応をどうするか?
            // 現状 : 書かない
            retStr = destructorTemplateDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::member_function_t)
        {
            // class 内の Function
            std::string classFunctionDef = "";
            std::string indentSpace = "        ";
            std::string memberFuncDef = "";
            memberFuncDef = line_generator;
            std::string nsStr2 = ns_str + "::";

            classFunctionDef += indentSpace;

            // remove namespace?
            myReplace(memberFuncDef, nsStr2, "");
            classFunctionDef += line_generator;
            retStr = classFunctionDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::enum_t)
        {
            std::string enumTemplateDef = "";
            std::string indentSpace = "    ";

            // first line
            enumTemplateDef = "cdef extern from \"" + base_filename + "\" namespace \"" + ns_str + "\":" + "\n";
            // second line
            enumTemplateDef += indentSpace;

            if(isClassAccessPublic)
            {
                // class 内定義
                enumTemplateDef += "ctypedef enum ";
                enumTemplateDef += "EnumTypeNameReplace";
                enumTemplateDef +=  " ";
                enumTemplateDef +=  ns_str;
                enumTemplateDef +=  "::";
                enumTemplateDef +=  "ClassName";
                enumTemplateDef +=  "::"; 
                enumTemplateDef +=  "EnumTypeName";
                enumTemplateDef +=  ":";
                // enum in flag on?
                // Enum の値を設定する際の判断となるフラグを on にする?
                isEnumClassInFlag = true;
            }
            else
            {
                // class 外定義
                enumTemplateDef += "cdef enum ";
                enumTemplateDef += "EnumTypeName";
                enumTemplateDef += ":";
                // enum in flag off?
                // Enum の値を設定する際の判断となるフラグを off にする?
                isEnumClassInFlag = false;
            }

            // Debug
            enumTemplateDef += "\n";
            enumTemplateDef += "Debug Str : ";
            enumTemplateDef += line_generator;
            retStr = enumTemplateDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::enum_value_t)
        {
            std::string enumTemplateValueDef = "";
            std::string indentSpace = "        ";
            enumTemplateValueDef += indentSpace;

            if(isEnumClassInFlag)
            {
                // class 内定義
                // Enum 定義 + "_" + Enum 値名称
                enumTemplateValueDef += "EnumDef";
                enumTemplateValueDef += "_";
                enumTemplateValueDef += "EnumValueName";
                enumTemplateValueDef += " ";

                // 
                enumTemplateValueDef += "\"";
                enumTemplateValueDef += ns_str;
                enumTemplateValueDef += "::";
                enumTemplateValueDef += "ClassName";
                enumTemplateValueDef += "::";
                enumTemplateValueDef += "EnumValueName";
                enumTemplateValueDef += "\"";
            }
            else
            {
                // class 外定義
                enumTemplateValueDef += "EnumValueName";
                enumTemplateValueDef += " ";

                enumTemplateValueDef += "EnumValue";
                enumTemplateValueDef += ",";
            }

            // Debug
            enumTemplateValueDef += "\n";
            enumTemplateValueDef += "Debug Str : ";
            enumTemplateValueDef += line_generator;
            retStr = enumTemplateValueDef;
        }
        else
        {
            std::string otherDef = "";
            
            // Debug
            otherDef += "\n";
            otherDef += "Debug Str : ";
            otherDef += line_generator;
            retStr = otherDef;
        }

        return retStr;
    }

    // IGenerator 継承
    std::string autopxd_generator2(const std::string& ns_str, const cppast::cpp_entity& e, const std::vector<IGenerator*>& generatorLists)
    {
        std::string retStr = "";
        if(e.kind() == cppast::cpp_entity_kind::include_directive_t)
        {
            // TODO : Cython が標準で対応できないヘッダファイルに関しては、再帰的にヘッダを解析していく仕組みを実装する?
            // 現時点での対応は面倒なため、対応しないことにする
            std::string importDef = "";
            std::string line_generator = "";

            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                // line_generator += (*itr)->GetType();
                line_generator += (*itr)->GetString();
            }

            importDef = line_generator;
            bool isStandard = false;

            // standard header list
            // start
            // 標準 C/C++ ヘッダファイルかどうかチェックする。
            for(auto itr = c_header_lists.begin(); itr != c_header_lists.end(); ++itr)
            {
                // '標準C header check'
                if(std::string::npos != importDef.find(*itr))
                {
                    std::string repace_c_import_header = *itr;
                    myReplace(repace_c_import_header, "<", "");
                    myReplace(repace_c_import_header, ">", "");

                    // cimport の対応
                    std::string repace_c_import = "";
                    // TODO : 取り出したい内容が ファイル名と一致していないケースもあるので注意して実装する。
                    if (repace_c_import_header == "stdint.h")
                    {
                        // stdint 読み出し
                        // repace_c_import = "from libc." + repace_c_import_header + " " + "cimport " + repace_c_import_header;
                        // memory なら unique_ptr とか?
                        for(auto itr2 = cstd_type_lists.begin(); itr2 != cstd_type_lists.end(); ++itr2)
                        {
                            repace_c_import += "from libc.";
                            repace_c_import += repace_c_import_header;
                            repace_c_import += " ";
                            repace_c_import += "cimport ";
                            repace_c_import += *itr2;
                            repace_c_import += "\n";
                        }
                    }
                    else
                    {
                        repace_c_import = "from libc." + repace_c_import_header + " " + "cimport " + repace_c_import_header;
                    }

                    myReplace(importDef, *itr, repace_c_import);
                    isStandard = true;
                    break;
                }
            }

            for(auto itr = cpp_header_lists.begin(); itr != cpp_header_lists.end(); ++itr)
            {
                // '標準 C++ header check'
                if(std::string::npos != importDef.find(*itr))
                {
                    std::string repace_cpp_import_header = *itr;
                    myReplace(repace_cpp_import_header, "<", "");
                    myReplace(repace_cpp_import_header, ">", "");

                    // cimport の対応
                    std::string repace_cpp_import = "";
                    if(repace_cpp_import_header == "memory")
                    {
                        // memory なら unique_ptr とか?
                        for(auto itr2 = cpp_memory_import_lists.begin(); itr2 != cpp_memory_import_lists.end(); ++itr2)
                        {
                            repace_cpp_import += "from libcpp.";
                            repace_cpp_import += repace_cpp_import_header;
                            repace_cpp_import += " ";
                            repace_cpp_import += "cimport ";
                            repace_cpp_import += *itr2;
                            repace_cpp_import += "\n";
                        }
                    }
                    else
                    {
                        // TODO : 取り出したい内容が ファイル名と一致していないケースもあるので注意して実装する。
                        repace_cpp_import = "from libcpp." + repace_cpp_import_header + " " + "cimport " + repace_cpp_import_header;
                    }

                    myReplace(importDef, *itr, repace_cpp_import);
                    isStandard = true;
                    break;
                }
            }

            for(auto itr = posix_header_lists.begin(); itr != posix_header_lists.end(); ++itr)
            {
                // 'posix header check'
                if(std::string::npos != importDef.find(*itr))
                {
                    std::string repace_posix_import_header = *itr;
                    myReplace(repace_posix_import_header, "<", "");
                    myReplace(repace_posix_import_header, ">", "");

                    std::string repace_posix_import = "from posix." + repace_posix_import_header + " " + "cimport " + repace_posix_import_header;
                    myReplace(importDef, *itr, repace_posix_import);
                    isStandard = true;
                    break;
                }
            }
            // end

            // custom header list?

            // ".h" remove
            myReplace(importDef, ".h", "");
            // #include -> import
            // std::replace(importDef.begin(), importDef.end(), "#include", "cimport");
            if(isStandard)
            {
                myReplace(importDef, "#include", "");
            }
            else
            {
                // 
                myReplace(importDef, "#include", "cimport");
            }

            // "/" to "."
            std::replace(importDef.begin(), importDef.end(), '/', '.');
            // "\"" remove
            myReplace(importDef, "\"", "");

            retStr = importDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::class_t)
        {
            std::string classDef = "";
            std::string indentSpace = "    ";
            std::string line_generator = "";
            
            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                line_generator += (*itr)->GetType();
                line_generator += (*itr)->GetString();
            }

            // base_filename
            // first line
            classDef = "cdef extern from \"" + base_filename + "\" namespace \"" + ns_str + "\":" + "\n";
            // second line
            classDef += indentSpace;
            // classDef += "cdef cppclass ";
            // classDef += "className";
            classDef += "cdef cpp";

            classDef += line_generator;
            classDef += ":";

            // ベースクラス継承する場合
            // (
            // Action
            // )

            // Debug
            // classDef += "\n";
            // classDef += "Debug Str : ";
            // classDef += line_generator;

            retStr = classDef;
            isClassAccessPublic = false;
        }
        else if (e.kind() == cppast::cpp_entity_kind::class_template_t)
        {
            std::string classTemplateDef = "";
            std::string indentSpace = "    ";
            std::string line_generator = "";
            
            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                line_generator += (*itr)->GetType();
                line_generator += (*itr)->GetString();
            }

            // first line
            classTemplateDef = "cdef extern from \"" + base_filename + "\" namespace \"" + ns_str + "\":" + "\n";
            // second line
            classTemplateDef += indentSpace;
            classTemplateDef += "cdef cppclass ";
            classTemplateDef += "className";
            // template 付の class の場合
            classTemplateDef += "[";
                classTemplateDef += "Test";
                // classTemplateDef += ", "
            classTemplateDef += "]";
            classTemplateDef += ":";

            // ベースクラス継承する場合
            // (
            // Action
            // )

            // Debug
            classTemplateDef += "\n";
            classTemplateDef += "Debug Str : ";
            classTemplateDef += line_generator;

            retStr = classTemplateDef;
            isClassAccessPublic = false;
        }
        else if (e.kind() == cppast::cpp_entity_kind::access_specifier_t)
        {
            std::string line_generator = "";
            
            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                line_generator += (*itr)->GetType();
                line_generator += (*itr)->GetString();
            }

            // 権限が public 以外は、書き出さない。
            // case cppast::cpp_entity_kind::access_specifier_t:
            if(line_generator.compare("public:") == 0)
            {
                isClassAccessPublic = true;
            }
            else
            {
                isClassAccessPublic = false;
            }
        }
        else if(e.kind() == cppast::cpp_entity_kind::constructor_t)
        {
            std::string constructorTemplateDef = "";
            std::string indentSpace = "    ";
            std::string line_generator = "";
            
            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                line_generator += (*itr)->GetType();
                line_generator += (*itr)->GetString();
            }

            // construct
            if(isClassAccessPublic)
            {
                constructorTemplateDef += "Debug Str : ";
                constructorTemplateDef += line_generator;
            }

            // public 以外での対応をどうするか?
            // 現状 : 書かない
            retStr = constructorTemplateDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::destructor_t)
        {
            std::string destructorTemplateDef = "";
            std::string indentSpace = "    ";
            std::string line_generator = "";

            // destruct
            if(isClassAccessPublic)
            {
                destructorTemplateDef += "Debug Str : ";
                destructorTemplateDef += line_generator;
            }
            // public 以外での対応をどうするか?
            // 現状 : 書かない
            retStr = destructorTemplateDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::member_function_t)
        {
            // class 内の Function
            std::string classFunctionDef = "";
            std::string indentSpace = "        ";
            std::string memberFuncDef = "";
            std::string line_generator = "";
            
            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                line_generator += (*itr)->GetType();
                line_generator += (*itr)->GetString();
            }

            memberFuncDef = line_generator;
            std::string nsStr2 = ns_str + "::";

            classFunctionDef += indentSpace;

            // remove namespace?
            myReplace(memberFuncDef, nsStr2, "");
            classFunctionDef += line_generator;
            retStr = classFunctionDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::enum_t)
        {
            std::string enumTemplateDef = "";
            std::string indentSpace = "    ";
            std::string line_generator = "";
            
            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                line_generator += (*itr)->GetType();
                line_generator += (*itr)->GetString();
            }

            // first line
            enumTemplateDef = "cdef extern from \"" + base_filename + "\" namespace \"" + ns_str + "\":" + "\n";
            // second line
            enumTemplateDef += indentSpace;

            if(isClassAccessPublic)
            {
                // class 内定義
                enumTemplateDef += "ctypedef enum ";
                enumTemplateDef += "EnumTypeNameReplace";
                enumTemplateDef +=  " ";
                enumTemplateDef +=  ns_str;
                enumTemplateDef +=  "::";
                enumTemplateDef +=  "ClassName";
                enumTemplateDef +=  "::"; 
                enumTemplateDef +=  "EnumTypeName";
                enumTemplateDef +=  ":";
                // enum in flag on?
                // Enum の値を設定する際の判断となるフラグを on にする?
                isEnumClassInFlag = true;
            }
            else
            {
                // class 外定義
                enumTemplateDef += "cdef enum ";
                enumTemplateDef += "EnumTypeName";
                enumTemplateDef += ":";
                // enum in flag off?
                // Enum の値を設定する際の判断となるフラグを off にする?
                isEnumClassInFlag = false;
            }

            // Debug
            enumTemplateDef += "\n";
            enumTemplateDef += "Debug Str : ";
            enumTemplateDef += line_generator;
            retStr = enumTemplateDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::enum_value_t)
        {
            std::string enumTemplateValueDef = "";
            std::string indentSpace = "        ";
            std::string line_generator = "";
            
            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                line_generator += (*itr)->GetType();
                line_generator += (*itr)->GetString();
            }

            enumTemplateValueDef += indentSpace;

            if(isEnumClassInFlag)
            {
                // class 内定義
                // Enum 定義 + "_" + Enum 値名称
                enumTemplateValueDef += "EnumDef";
                enumTemplateValueDef += "_";
                enumTemplateValueDef += "EnumValueName";
                enumTemplateValueDef += " ";

                // 
                enumTemplateValueDef += "\"";
                enumTemplateValueDef += ns_str;
                enumTemplateValueDef += "::";
                enumTemplateValueDef += "ClassName";
                enumTemplateValueDef += "::";
                enumTemplateValueDef += "EnumValueName";
                enumTemplateValueDef += "\"";
            }
            else
            {
                // class 外定義
                enumTemplateValueDef += "EnumValueName";
                enumTemplateValueDef += " ";

                enumTemplateValueDef += "EnumValue";
                enumTemplateValueDef += ",";
            }

            // Debug
            enumTemplateValueDef += "\n";
            enumTemplateValueDef += "Debug Str : ";
            enumTemplateValueDef += line_generator;
            retStr = enumTemplateValueDef;
        }
        else
        {
            std::string otherDef = "";
            std::string line_generator = "";

            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                line_generator += (*itr)->GetType();
                line_generator += (*itr)->GetString();
            }

            // Debug
            otherDef += "\n";
            otherDef += "Debug Str : ";
            otherDef += line_generator;
            retStr = otherDef;
        }

        return retStr;
    }

};

// TODO: 対象モジュールのインクルードパスを取得
// # BUILTIN_HEADERS_DIR = os.path.join(os.path.dirname(__file__), 'include')
// # Types declared by pycparser fake headers that we should ignore
