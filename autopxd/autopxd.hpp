#include <iostream>
#include <fstream>
#include <stack>

#include <cctype>

#include <cppast/libclang_parser.hpp>        // for libclang_parser, libclang_compile_config, cpp_entity,...
#include <cppast/visitor.hpp>                // for visit()
#include <cppast/code_generator.hpp>         // for generate_code()
// override parameter
//#include <type_safe/flag_set.hpp>            // ref
//#include <type_safe/index.hpp>
#include <cppast/cpp_entity.hpp>
#include <cppast/cpp_entity_ref.hpp>

#include <cppast/cpp_entity_kind.hpp>        // for the cpp_entity_kind definition
#include <cppast/cpp_forward_declarable.hpp> // for is_definition()
#include <cppast/cpp_namespace.hpp>          // for cpp_namespace

// Class 
#include "nodes.h"

/*! @brief  Pxd �t�@�C�������̑�{�N���X
    @remark cppast ���őΏ��\�ȍ��ڂƂ̐؂蕪�������Ă�������
            code_generator �̒�`�͂�����ɏ����ׂ�?
*/
class AutoPxd
{
private:
    // h�t�@�C��(root �t�@�C��)
    // namespace(�K�w�ɂȂ�P�[�X�����邽�ߕ���?)
    // class��`(�K�w�ɂȂ�P�[�X�����邽�ߕ���?)
    // template�t class��`
    // template ��`
    // struct ��`
    // �� ��L�p�����[�^�𓯈�ň�����悤�Ɋ��N���X��p�ӂ���?

    // pxd �t�@�C�������o���p
    std::string autopxd_file;
    std::ofstream fout_pxd;
    std::string base_filename;

    // �L�q���[����荞��?
    // std::vector<std::stack<PxdNode*>> declStack;
    std::vector<std::vector<PxdNode*>> declStack;

    // std::vector<std::string> pxd_lines;
    std::string namespace_str;
    std::string class_str;
    std::string class_access_str;
    std::string const_str;
    bool isClassAccessPublic;
    bool isEnumClassInFlag;
    int indentCount;
    bool isFileEnd;

    // class ���� Template �ɂ��Ċi�[
    // (Template �t�֐��Ăяo�����̈������`�F�b�N���邽�߂Ɏg�p����B)
    // class �ɕ����� Template ���ݒ肳��Ă���\�������邽�߁A�ȉ��̌`�ɂ��Ă���B
    std::vector<std::string> classTemplates;

public:
    AutoPxd(const std::string& filename)
    {
        // ���̂܂ܐݒ肷��ƁA��΃p�X�ɂȂ邽��
        // ���΃p�X�Ƃ��Đݒ肷�邱�ƁB
        base_filename = filename;

        // �g���q��菜��
        // win
        // int path_i = base_filename.find_last_of("\\") + 1;
        int path_i = base_filename.find_last_of("/") + 1;
        int ext_i = base_filename.find_last_of(".");
        std::string filename_without_ext = base_filename.substr(path_i, ext_i - path_i);
        std::string extname = base_filename.substr(ext_i, base_filename.size() - ext_i);

        // autopxd_file = base_filename + ".pxd";
        autopxd_file = filename_without_ext + ".pxd";
        namespace_str = "";
        class_str = "";
        class_access_str = "";
        isClassAccessPublic = false;
        isEnumClassInFlag = false;
        indentCount = 0;
        isFileEnd = false;

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
        std::vector<std::string> refLines = {};
        std::vector<PxdNode*> refNodes = {};
        PxdNode* tmpPxdNode = nullptr;
        bool node_continue = true;
    	bool container_start = true;
        bool isIndentCountUp = false;

        // recursively visit file and all children
        cppast::visit(file, [&](const cppast::cpp_entity& e, cppast::visitor_info info) {
            // std::cout << "is_new_entity: ";
            // std::cout << info.is_new_entity();
            // std::cout << "\n";
            
            // if(indentCount

            if (e.kind() == cppast::cpp_entity_kind::file_t)
            {
                // �t�@�C���擪/�I�[�Œʒm�����?
                std::cout << "file_t";
                std::cout << "\n";

                if(isFileEnd)
                {
                    // �t�@�C���I�[����
                    // enum ��`
                    // NodeType �̔��f���s���B
                    // container_entity_enter �̃^�C�~���O�Ŏ��s?
                    PxdNode* enumNode = new EnumNode("", refLines);
                    refNodes.push_back(enumNode);
                    //refNodes = {};
                    refLines = {};
                    node_continue = true;
                }
                else
                {
                    // �t�@�C���擪����
                    std::string headerRef = "\n";
                    headerRef += "cdef extern from \"" + base_filename + "\":" + "\n";
                    refLines.push_back(headerRef);
                    indentCount++;
                    isFileEnd = true;
                }

                // no need to do anything for a file,
                // templated and friended entities are just proxies, so skip those as well
                // return true to continue visit for children
                return true;
            }
            // else if (cppast::is_templated(e) || cppast::is_friended(e))
            // {
            //  std::cout << "is_templated";
            //     std::cout << "\n";
            //  
            //     // no need to do anything for a file,
            //     // templated and friended entities are just proxies, so skip those as well
            //     // return true to continue visit for children
            //     return true;
            // }
            else if (e.kind() == cppast::cpp_entity_kind::namespace_t)
            {
                std::cout << "namespace_t";
                std::cout << "\n";

                // cast to cpp_namespace
                auto& ns = static_cast<const cppast::cpp_namespace&>(e);
                // print whether or not it is inline
                if (ns.is_inline())
                {
                    // inline �֐�?
                    // fout_pxd << " [inline]";
                }

                if(!namespace_str.empty())
                {
                    namespace_str += "::";
                }

                namespace_str += e.name();
                // fout_pxd << '\n';
            }
            else if (info.event == cppast::visitor_info::container_entity_exit)
            {
                std::cout << "container_entity_exit";
                std::cout << "\n";

                // namespace ���ɕ����̃N���X/�\���̂���`����Ă���ꍇ���l��
                // �N���X/�\���̂��P���Ǘ�����H
                indentCount--;
                std::cout << "indentCount: ";
                std::cout << indentCount;
                std::cout << "\n";

                // �J�E���g�_�E��?
                node_continue = false;
            }
            // else if (info.event == cppast::visitor_info::container_entity_enter)
            // {
            //     std::cout << "container_entity_enter";
            //     std::cout << "\n";
            // }
            // �����́H
            // else if (info.event == cppast::visitor_info::leaf_entity)
            // {
            //    fout_pxd << "leaf_entity";
            // }
            else
            {
                // �K�w���̌���
                if (info.last_child)
                {
                    if (info.event == cppast::visitor_info::container_entity_enter)
                    {
                        // �K�w�ω�����(class ���� class/struct/union ��`�Ŕ���?)
                        std::cout << "(last_child)container_entity_enter";
                        std::cout << "\n";

                        // indentCount++;
                        isIndentCountUp = true;
                        std::cout << "indentCount: ";
                        std::cout << indentCount;
                        std::cout << "\n";

                        // �J�E���g�A�b�v?
                        node_continue = true;
                    }
                    else if(info.event == cppast::visitor_info::leaf_entity)
                    {
                        // �K�w�ω��Ȃ�(enum ���̏I�[?)
                        // fout_pxd << "leaf_entity";
                        std::cout << "(last_child)leaf_entity";
                        std::cout << "\n";
                        node_continue = false;
                    }

                    // �J�n�[?(�K�w+1)
                    // out << "+-";
                }
                else
                {
                    if (info.event == cppast::visitor_info::container_entity_enter)
                    {
                        // �K�w�ω�����(class ��`�Ƃ������C��?)
                        std::cout << "main? container_entity_enter";
                        std::cout << "\n";

                        // indentCount++;
                        isIndentCountUp = true;
                        std::cout << "indentCount: ";
                        std::cout << indentCount;
                        std::cout << "\n";

                        // base_filename
                        // first line
                        if(!namespace_str.empty() && container_start)
                        {
                            std::string headerRef = "\n";
                            headerRef += "cdef extern from \"" + base_filename + "\" namespace \"" + namespace_str + "\":" + "\n";
                            refLines.push_back(headerRef);
                        	container_start = false;
                        }

                        // �J�E���g�A�b�v?
                        node_continue = true;
                    }
                    else if(info.event == cppast::visitor_info::leaf_entity)
                    {
                        // �K�w�ω��Ȃ�(enum �r���o��? 1�s�`���s�P�ʂ̏����A�֐�/�ϐ���`���Ώ�?)
                        // �N���X���֐�/�ϐ������l?
                        std::cout << "main? leaf_entity";
                        std::cout << "\n";
                        // node_continue = true;
                    }
                    else
                    {
                        std::cout << "container other?";
                        std::cout << "\n";
                    }

                    // �K�w�p��?
                    // out << "|-";
                }

                // print_entity(out, e);
                // autopxd_entity(out, e);
                // AutoPxd �ɐ�p�̊֐���p�ӂ���B(�������̕����Ώ����y�Ȃ���)
                // autopxd_entity2(e, refLines, tmpPxdNode);
                autopxd_entity2(e, refLines);
            }

            if(node_continue == false)
            {
                // enum ��`
                // NodeType �̔��f���s���B
                // container_entity_enter �̃^�C�~���O�Ŏ��s?
                PxdNode* enumNode = new EnumNode("", refLines);
                refNodes.push_back(enumNode);
                //refNodes = {};
                refLines = {};
                node_continue = true;
            }

            if(isIndentCountUp)
            {
                indentCount++;
                isIndentCountUp = false;
            }

            return true;
        });
        declStack.push_back(refNodes);

        std::cout << "write pxd";
        std::cout << "\n";
        // stack ���� text �֏����o��?
        for(std::vector<PxdNode*>& stacksIt : declStack)
        // for (std::deque<PxdNode>::const_iterator stacksIt=declStack.begin(); stacksIt < declStack.end(); ++stacksIt)
        {
            std::cout << "stacksIt count : ";
            std::cout << stacksIt.size();
            std::cout << "\n";

            //while(!stacksIt.empty())
            for (PxdNode* stackIt : stacksIt)
            {
                //std::cout << "while";
                //std::cout << "\n";
                // for(auto lines: stacksIt.top()->lines2())
                // for(auto lines: stacksIt.top()->lines2())
                for(auto lines: stackIt->lines2())
                {
                    std::cout << lines;
                    fout_pxd << lines;
                }
                // stacksIt.pop();
            }
        }
    }

private:
    // void autopxd_entity2(const cppast::cpp_entity& e, std::vector<std::string>& refLines, PxdNode& pxd_node)
    void autopxd_entity2(const cppast::cpp_entity& e, std::vector<std::string>& refLines)
    {
        bool isDefine;
        isDefine = cppast::is_definition(e);

        /*
        // print name and the kind of the entity
        if (!e.name().empty())
        {
            // �Ώۍ���
            fout_pxd << e.name();
        }
        else
        {
            // �ϐ����悭�킩��Ȃ��Ƃ�(using �Ƃ��������ɓ���)
            fout_pxd << "<anonymous>";
        }
        */

        // ���
        // include directive -> 
        // macro definition -> 
        // namespace
        // class
        // class template
        // function
        // member variable
        // member function
        // access specifier
        // fout_pxd << " (" << cppast::to_string(e.kind()) << ")" << "\n";
        // std::string pxd_line = "";
        /*
        switch(e.kind())
        {
            // cpp_entity_kind.hpp ���Q��
            case cppast::cpp_entity_kind::namespace_t:
                // cast to cpp_namespace
                auto& ns = static_cast<const cppast::cpp_namespace&>(e);
                // print whether or not it is inline
                if (ns.is_inline())
                {
                    // inline �֐�?
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
            std::cout << "is_definition";
            std::cout << "\n";

            // define ?
            // ������?
            // fout_pxd << " [definition]";
            // pxd �̋L�q�Ƃ��ď����o��
            // e.kind() == "variant" + is_definition() == true
        }

        if (e.kind() == cppast::cpp_entity_kind::language_linkage_t)
        {
            std::cout << "language_linkage_t";
            std::cout << "\n";
            // pragma?

            // no need to print additional information for language linkages
        }
        else
        {
//#define GENERATOR_DEBUG
            // �ׂ��������o���́Apxd_generator ���őΉ�����?
            // override �őΉ�?
            // print the declaration of the entity
            // it will only use a single line
            // derive from code_generator and implement various callbacks for printing
            // it will print into a std::string
            class pxd_generator : public cppast::code_generator
            {
                std::string str_;                // the result
                bool was_newline_ = false;       // whether or not the last token was a newline
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
#ifdef GENERATOR_DEBUG
                    std::cout << "do_get_formatting";
                    std::cout << "\n";
#endif
                    // operator(+-/*)/comma(,)
                    // return formatting_flags::brace_nl | formatting_flags::operator_ws | formatting_flags::comma_ws;

                    return {};
                }
                // called to retrieve the generation options of an entity
                generation_options do_get_options(const cppast::cpp_entity& e,
                                                  cppast::cpp_access_specifier_kind kind) override
                {
#ifdef GENERATOR_DEBUG
                    std::cout << "do_get_options";
                    std::cout << "\n";
#endif // GENERATOR_DEBUG
                    //std::cout << (void)e;
                    //std::cout << (void)kind;

                    // generate declaration only
                    return pxd_generator::declaration;
                }

                /// \effects Will be invoked before code of an entity is generated.
                /// The base class version has no effect.
                void on_begin(const output& out, const cppast::cpp_entity& e) override
                {
                    // str_ += "on_begin";
                    // str_ += "\n";
                    // std::cout << "on_begin";
                    // std::cout << "\n";

                    //std::cout << out;
                    //std::cout << e;

                    (void)out;
                    (void)e;
                }

                /// \effects Will be invoked after all code of an entity has been generated.
                /// The base class version has no effect.
                void on_end(const output& out, const cppast::cpp_entity& e) override
                {
#ifdef GENERATOR_DEBUG
                    // str_ += "on_end";
                    // str_ += "\n";
                    std::cout << "on_end";
                    std::cout << "\n";
#endif // GENERATOR_DEBUG
                    (void)out;
                    (void)e;
                }

                void on_container_end(const output& out, const cppast::cpp_entity& e) override
                {
#ifdef GENERATOR_DEBUG
                    std::cout << "on_container_end";
                    std::cout << "\n";
#endif // GENERATOR_DEBUG
                    (void)out;
                    (void)e;
                }

                // no need to handle indentation, as only a single line is used
                void do_indent() override {}
                void do_unindent() override {}

                void do_write_keyword(cppast::string_view keyword) override
                {
#ifdef GENERATOR_DEBUG
                    std::cout << "do_write_keyword";
                    std::cout << "\n";
#endif // GENERATOR_DEBUG
                    str_ += "keyword : ";
                    // const
                    // void
                    // int
                    // class
                    // public
                    // override
                    // �߂�l���A�����̈��������f�͂ǂ���邩�H
                    // ���̃f�[�^���Ȃ����Ƃ���Ή�����H
                    str_ += keyword.c_str();
                    str_ += "\n";
                    // std::cout << str_;

                    // 2
                    IGenerator* generator = new KeywordGenerator();
                    generator->SetString(keyword.c_str());
                    generators.push_back(generator);
                }

                void do_write_identifier(cppast::string_view str) override
                {
#ifdef GENERATOR_DEBUG
                    std::cout << "do_write_identifier";
                    std::cout << "\n";
#endif // GENERATOR_DEBUG
                    str_ += "identifier : ";
                    // AttributeOctahedronTransform::
                    str_ += str.c_str();
                    str_ += "\n";
                    // std::cout << str_;

                    // 2
                    IGenerator* generator = new IdentifierGenerator();
                    generator->SetString(str.c_str());
                    generators.push_back(generator);
                }

                /*
                void do_write_reference(type_safe::array_ref<const cpp_entity_id> id, string_view name) override
                {
                    std::cout << "do_write_reference";
                    std::cout << "\n";

                    str_ += "reference : ";
                    str_ += name.c_str();
                    str_ += "\n";

                    // 2
                    IGenerator* generator = new ReferenceGenerator();
                    generator->SetString(name.c_str());
                    generators.push_back(generator);
                }
                */

                void do_write_punctuation(cppast::string_view punct) override
                {
#ifdef GENERATOR_DEBUG
                    std::cout << "do_write_punctuation";
                    std::cout << "\n";
#endif // GENERATOR_DEBUG
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
                    str_ += punct.c_str();
                    str_ += "\n";
                    // std::cout << str_;

                    // 2
                    IGenerator* generator = new PunctuationGenerator();
                    generator->SetString(punct.c_str());
                    generators.push_back(generator);
                }

                void do_write_str_literal(cppast::string_view str) override
                {
#ifdef GENERATOR_DEBUG
                    std::cout << "do_write_str_literal";
                    std::cout << "\n";
#endif // GENERATOR_DEBUG
                    str_ += "str : ";
                    str_ += "\n";
                    str_ += str.c_str();
                    str_ += "\n";

                    // 2
                    IGenerator* generator = new StrliteralGenerator();
                    generator->SetString(str.c_str());
                    generators.push_back(generator);
                }

                void do_write_int_literal(cppast::string_view str) override
                {
#ifdef GENERATOR_DEBUG
                    std::cout << "do_write_int_literal";
                    std::cout << "\n";
#endif // GENERATOR_DEBUG
                    str_ += "int : ";
                    str_ += str.c_str();
                    str_ += "\n";

                    // 2
                    IGenerator* generator = new IntliteralGenerator();
                    generator->SetString(str.c_str());
                    generators.push_back(generator);
                }

                void do_write_float_literal(cppast::string_view str) override
                {
#ifdef GENERATOR_DEBUG
                    std::cout << "do_write_float_literal";
                    std::cout << "\n";
#endif // GENERATOR_DEBUG
                    str_ += "float : ";
                    str_ += str.c_str();
                    str_ += "\n";
                    // std::cout << str_;

                    // 2
                    IGenerator* generator = new FloatliteralGenerator();
                    generator->SetString(str.c_str());
                    generators.push_back(generator);
                }

                void do_write_preprocessor(cppast::string_view punct) override
                {
                    str_ += "preprocessor : ";
                    // �擪 : #include/#define
                    // include �̏ꍇ
                    // �� : "file
                    // �� : "
                    // define �̏ꍇ
                    // �� : identifier
                    str_ += punct.c_str();
                    str_ += "\n";
                    // std::cout << str_;

                    // 2
                    IGenerator* generator = new PreprocessorGenerator();
                    generator->SetString(punct.c_str());
                    generators.push_back(generator);
                }

                void do_write_comment(cppast::string_view str) override
                {
#ifdef GENERATOR_DEBUG
                    std::cout << "do_write_comment";
                    std::cout << "\n";
#endif // GENERATOR_DEBUG
                    str_ += "comment : ";
                    str_ += str.c_str();
                    str_ += "\n";
                    // std::cout << str_;

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
#ifdef GENERATOR_DEBUG
                    std::cout << "do_write_token_seq";
                    std::cout << "\n";
#endif // GENERATOR_DEBUG
                    if (was_newline_)
                    {
                        // lazily append newline as space
                        str_ += ',';
                        was_newline_ = false;
                    }

                    // append tokens
                    str_ += "token : ";
                    str_ += tokens.c_str();
                    str_ += "\n";
                    // std::cout << str_;

                    // 2
                    IGenerator* generator = new TokenGenerator();
                    generator->SetString(tokens.c_str());
                    generators.push_back(generator);
                }

                // called when a newline should be generated
                // we're lazy as it will always generate a trailing newline,
                // we don't want
                void do_write_newline() override
                {
#ifdef GENERATOR_DEBUG
                    std::cout << "do_write_newline";
                    std::cout << "\n";
#endif // GENERATOR_DEBUG
                    was_newline_ = true;
                }
            } generator(e);

            // fout_pxd << ": `" << generator.str() << '`' << '\n';
            // fout_pxd << '`' << generator.str() << '`' << '\n';
            // std::unique_ptr<pxd_generator> tmp_generator = new pxd_generator(e);

            // stack �������ɓ˂�����ł����āA���̃^�C�~���O�ŉ�͂���?
            // �s�P�ʂ̉��
            // namespace_str, 
            // std::cout << "call autopxd_generator2 start.";
            // std::cout << "\n";
            refLines.push_back(autopxd_generator2(namespace_str, e, generator.generatorLists(), indentCount));
            // TODO: Node ��ނ͓����Ō��߂�?
            // �s�P�ʂł̉�͂̂��߁A��؂肪�s��
            // declStack.push_back(autopxd_generator3(namespace_str, e, generator.generatorLists()));
            // autopxd_generator3(namespace_str, e, generator.generatorLists(), refNodes);
            // std::cout << "call autopxd_generator2 end.";
            // std::cout << "\n";

            // Node �N���X���ɕ�����������������?
            // pxd_node.writeline(generator.generatorLists());
        }
    }

    // ����Ή�
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

    // c header �ǂݏo��+
    // c - stdint �ǂݍ��ݎ��ɓW�J�����`
    // cython ���g�p����W���Œ�`����Ă���^��񃊃X�g
    // uint32_t �����g�p����Ă��鎞�̑Ή��͂ǂ����邩
    // from libc.stdint cimport int8_t
    // from libc.stdint cimport uint32_t
    // :
    std::vector<std::string> cstd_type_lists { "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "size_t" };

    // cpp header �ǂݏo��+
    // memory �Ăяo������
    // cpp �t�@�C���̒�`�Ƃ��Ďg�p����^��`��ǉ�
    std::vector<std::string> cpp_type_lists { "bool", "nullptr_t", "nullptr" };
    std::vector<std::string> cpp_memory_import_lists { "unique_ptr", "shared_ptr", "weak_ptr" };

    // class ���� Enum �l���g�p����ꍇ��?(��`����ɂȂ�P�[�X?[���邩�ǂ���������]�̑Ή�)
    // operator function �̑Ή��ɂ��Ăǂ����邩�B

    // ������u������
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

    // IGenerator �p��
    std::string autopxd_generator2(const std::string& ns_str, const cppast::cpp_entity& e, const std::vector<IGenerator*>& generatorLists, int indentCount)
    {
        std::string retStr = "";
        std::string indentSpace = "";
        const std::string indentBaseSpace = "    ";
        for(int i = 0; i < indentCount;i++)
        {
            indentSpace += indentBaseSpace;
        }

        if(e.kind() == cppast::cpp_entity_kind::include_directive_t)
        {
            std::cout << "include_directive_t";
            std::cout << "\n";

            // TODO : Cython ���W���őΉ��ł��Ȃ��w�b�_�t�@�C���Ɋւ��ẮA�ċA�I�Ƀw�b�_����͂��Ă����d�g�݂���������?
            // �����_�ł̑Ή��͖ʓ|�Ȃ��߁A�Ή����Ȃ����Ƃɂ���
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
            // �W�� C/C++ �w�b�_�t�@�C�����ǂ����`�F�b�N����B
            for(auto itr = c_header_lists.begin(); itr != c_header_lists.end(); ++itr)
            {
                // '�W��C header check'
                if(std::string::npos != importDef.find(*itr))
                {
                    std::string repace_c_import_header = *itr;
                    myReplace(repace_c_import_header, "<", "");
                    myReplace(repace_c_import_header, ">", "");

                    // cimport �̑Ή�
                    std::string repace_c_import = "";
                    // TODO : ���o���������e�� �t�@�C�����ƈ�v���Ă��Ȃ��P�[�X������̂Œ��ӂ��Ď�������B
                    if (repace_c_import_header == "stdint.h")
                    {
                        // stdint �ǂݏo��
                        // repace_c_import = "from libc." + repace_c_import_header + " " + "cimport " + repace_c_import_header;
                        // memory �Ȃ� unique_ptr �Ƃ�?
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
                // '�W�� C++ header check'
                if(std::string::npos != importDef.find(*itr))
                {
                    std::string repace_cpp_import_header = *itr;
                    myReplace(repace_cpp_import_header, "<", "");
                    myReplace(repace_cpp_import_header, ">", "");

                    // cimport �̑Ή�
                    std::string repace_cpp_import = "";
                    if(repace_cpp_import_header == "memory")
                    {
                        // memory �Ȃ� unique_ptr �Ƃ�?
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
                        // TODO : ���o���������e�� �t�@�C�����ƈ�v���Ă��Ȃ��P�[�X������̂Œ��ӂ��Ď�������B
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
            importDef += "\n";

            retStr = importDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::type_alias_t)
        {
            // typedef
            std::cout << "type_alias_t";
            std::cout << "\n";

            std::string typeAliasDef   = "";
            std::string typeAliasName  = "";
            std::string typeAliasValue = "";

            typeAliasDef += indentSpace;

            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";

                IdentifierGenerator *sa = dynamic_cast<IdentifierGenerator*>(*itr);
                if(sa != nullptr)
                {
                    typeAliasName = sa->GetString();
                    continue;
                }

                KeywordGenerator *sc = dynamic_cast<KeywordGenerator*>(*itr);
                if(sc != nullptr)
                {
                    typeAliasValue = sc->GetString();
                    continue;
                }

                TokenGenerator *sb = dynamic_cast<TokenGenerator*>(*itr);
                if(sb != nullptr)
                {
                    typeAliasValue = sb->GetString();
                    continue;
                }
            }

            typeAliasDef += "ctypedef ";
            // typeAliasDef += "my_union_u";
            // typeAliasDef += "hogehoge ";
            typeAliasDef += typeAliasValue;
            typeAliasDef += " ";
            typeAliasDef += typeAliasName;
            typeAliasDef += "\n";
            retStr = typeAliasDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::macro_definition_t)
        {
            std::cout << "macro_definition_t";
            std::cout << "\n";

            // �s�v?
            /*
            std::string macroDefineDef   = "";
            std::string macroDefineName  = "";
            std::string macroDefineValue = "";

            macroDefineDef += indentSpace;

            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";

                IdentifierGenerator *sa = dynamic_cast<IdentifierGenerator*>(*itr);
                if(sa != nullptr)
                {
                    macroDefineName = sa->GetString();
                }
                //IdentifierGenerator *sa = dynamic_cast<IdentifierGenerator*>(*itr);
                //if(sa != nullptr)
                //{
            }

            macroDefineDef += "DEF ";
            macroDefineDef += macroDefineName;
            macroDefineDef += macroDefineValue;
            macroDefineDef += "\n";
            retStr = macroDefineDef;
            */
        }
        else if(e.kind() == cppast::cpp_entity_kind::class_t)
        {
            std::cout << "class_t";
            std::cout << "\n";

            std::string classDef = "";
            std::string classKeyworkName = "";
            std::string className = "";

            classDef += indentSpace;

            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";

                KeywordGenerator *sa = dynamic_cast<KeywordGenerator*>(*itr);
                if(sa != nullptr)
                {
                    classKeyworkName = sa->GetString();
                }
                IdentifierGenerator *sb = dynamic_cast<IdentifierGenerator*>(*itr);
                if(sb != nullptr)
                {
                    std::string tmpClassName= sb->GetString();
                    if(!tmpClassName.empty())
                    {
                        className = tmpClassName;
                        class_str = className;
                    }
                }
                PunctuationGenerator *sc = dynamic_cast<PunctuationGenerator*>(*itr);
                if(sc != nullptr)
                {
                    std::string tmpPunctuation = sc->GetString();
                    if(tmpPunctuation == ";")
                    {
                        // ���`�Ƃ��Ĉ���
                        // className = tmpClassName;
                    }
                }
            }

            // second line
            // classDef += "cdef cppclass ";
            // classDef += "className";
            classDef += "cdef ";
            classDef += classKeyworkName;
            classDef += " ";
            classDef += className;
            classDef += ":";

            // �x�[�X�N���X�p������ꍇ
            // (
            // Action
            // )

            classDef += "\n";

            retStr = classDef;
            isClassAccessPublic = false;
        }
        else if (e.kind() == cppast::cpp_entity_kind::class_template_t)
        {
            std::cout << "class_template_t";
            std::cout << "\n";

            std::string classTemplateDef = "";
            std::string classTemplateKeyworkName = "";
            std::string classTemplateName = "";

            classTemplateDef += indentSpace;

            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";

                KeywordGenerator *sa = dynamic_cast<KeywordGenerator*>(*itr);
                if(sa != nullptr)
                {
                    classTemplateKeyworkName = sa->GetString();
                }
                IdentifierGenerator *sb = dynamic_cast<IdentifierGenerator*>(*itr);
                if(sb != nullptr)
                {
                    std::string tmpClassName= sb->GetString();
                    if(!tmpClassName.empty())
                    {
                        classTemplateName = tmpClassName;
                    }
                }
                PunctuationGenerator *sc = dynamic_cast<PunctuationGenerator*>(*itr);
                if(sc != nullptr)
                {
                    std::string tmpPunctuation = sc->GetString();
                    if(tmpPunctuation == ";")
                    {
                        // ���`�Ƃ��Ĉ���
                        // classTemplateName = tmpClassName;
                    }
                }
            }

            // first line
            // second line
            classTemplateDef += "cdef cppclass ";
            // classTemplateDef += "className";
            classTemplateDef += classTemplateName;
            // template �t�� class �̏ꍇ
            classTemplateDef += "[";
            classTemplateDef += "Test";
            // classTemplateDef += ", "
            classTemplateDef += "]";
            classTemplateDef += ":";

            // �x�[�X�N���X�p������ꍇ
            // (
            // Action
            // )

            classTemplateDef += "\n";

            // Debug
            // classTemplateDef += line_generator;

            retStr = classTemplateDef;
            isClassAccessPublic = false;
        }
        else if (e.kind() == cppast::cpp_entity_kind::access_specifier_t)
        {
            std::cout << "access_specifier_t";
            std::cout << "\n";

        	std::string accessParam = "";
            // std::string line_generator = "";

            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";

            	KeywordGenerator *sa = dynamic_cast<KeywordGenerator*>(*itr);
                if(sa != nullptr)
                {
                    accessParam = sa->GetString();
                	continue;
                }
                // line_generator += (*itr)->GetString();
            }

            // ������ public �ȊO�́A�����o���Ȃ��B
            // case cppast::cpp_entity_kind::access_specifier_t:
            if(accessParam.compare("public") == 0)
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
            std::cout << "constructor_t";
            std::cout << "\n";

            std::string constructorTemplateDef = "";
            constructorTemplateDef += indentSpace;

            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";
                constructorTemplateDef += (*itr)->GetString();
            }

            // def __init__(self):

            // construct
            if(isClassAccessPublic)
            {
                // constructorTemplateDef += ;
            }

            // public �ȊO�ł̑Ή����ǂ����邩?
            // ���� : �����Ȃ�
            constructorTemplateDef += "\n";
            retStr = constructorTemplateDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::destructor_t)
        {
            std::cout << "destructor_t";
            std::cout << "\n";

            std::string destructorTemplateDef = "";
            std::string line_generator = "";

            destructorTemplateDef += indentSpace;

            // destruct
            if(isClassAccessPublic)
            {
                destructorTemplateDef += line_generator;
            }
            // public �ȊO�ł̑Ή����ǂ����邩?
            // ���� : �����Ȃ�
            
            destructorTemplateDef += "\n";
            retStr = destructorTemplateDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::member_function_t)
        {
            std::cout << "member_function_t";
            std::cout << "\n";

            // class ���� Function
            std::string classFunctionDef = "";
            std::string memberFuncDef = "";
            std::string line_generator = "";
            std::string nsStr2 = "";

            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";
            }

            memberFuncDef = "@cython.boundscheck(False) # turn off boundscheck for this function";
            //memberFuncDef = line_generator;
            if(ns_str.empty())
            {
            }
            else
            {
                nsStr2 = ns_str + "::";
            }
            classFunctionDef += indentSpace;

            // remove namespace?
            myReplace(memberFuncDef, nsStr2, "");
            // Debug?
            // classFunctionDef += line_generator;
            retStr = classFunctionDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::enum_t)
        {
            std::cout << "enum_t";
            std::cout << "\n";

            std::string enumTemplateDef = "";
            std::string generatorParam = "";
            std::string enumTypeName = "";

            enumTemplateDef += indentSpace;

            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";

                IdentifierGenerator *sa = dynamic_cast<IdentifierGenerator*>(*itr);
                if(sa != nullptr)
                {
                    enumTypeName = sa->GetString();
                }
            }

            // first line
            // second line

            if(isClassAccessPublic)
            {
                // class ����`
                enumTemplateDef += "ctypedef enum ";
                enumTemplateDef += "EnumTypeNameReplace";
                enumTemplateDef +=  " ";
                enumTemplateDef +=  ns_str;
                enumTemplateDef +=  "::";
                // enumTemplateDef += "ClassName";
                enumTemplateDef += class_str;
                enumTemplateDef +=  "::"; 
                // enumTemplateDef +=  "EnumTypeName";
                enumTemplateDef += enumTypeName;
                enumTemplateDef +=  ":";
                // enum in flag on?
                // Enum �̒l��ݒ肷��ۂ̔��f�ƂȂ�t���O�� on �ɂ���?
                isEnumClassInFlag = true;
            }
            else
            {
                // class �O��`
                enumTemplateDef += "cdef enum";
                if(enumTypeName.empty())
                {
                    // None
                }
                else
                {
                    enumTemplateDef += " ";
                    // enumTemplateDef += "EnumTypeName";
                    enumTemplateDef += enumTypeName;
                }
                enumTemplateDef += ":";
                // enum in flag off?
                // Enum �̒l��ݒ肷��ۂ̔��f�ƂȂ�t���O�� off �ɂ���?
                isEnumClassInFlag = false;
            }

            enumTemplateDef += "\n";
            retStr = enumTemplateDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::enum_value_t)
        {
            std::cout << "enum_value_t";
            std::cout << "\n";

            std::string enumValueDef = "";
            std::string line_generator = "";

            std::string enumValueName = "";
            std::string enumValue = "";
            enumValueDef += indentSpace;

            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                //std::cout << typeid(*itr).name();
                //std::cout << "\n";
                //std::cout << typeid(IdentifierGenerator).name();
                //std::cout << "\n";
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";

                IdentifierGenerator *sa = dynamic_cast<IdentifierGenerator*>(*itr);
                if(sa != nullptr)
                {
                    enumValueName = sa->GetString();
                    continue;
                }

                IntliteralGenerator *sb = dynamic_cast<IntliteralGenerator*>(*itr);
                if(sb != nullptr)
                {
                	// ���l�ȊO�̕�����͎�菜��
                    std::string tmpValue = sb->GetString();

                	// https://qiita.com/edo1z/items/da66e28e206d2b01157e
                	// if(!isdigit(tmpValue.c_str()))
                	// {
                	if (!std::all_of(tmpValue.cbegin(), tmpValue.cend(), isdigit))
					{
                		// ������1�����]���ɂ��Ă���P�[�X
                		// std::regex regex(KEYWORD_LINE_REGEX);
                		tmpValue.erase(tmpValue.size() -1);
                		// tmpValue 
                	}
                	
                	enumValue = tmpValue;
                    continue;
                }
            }

            if(isEnumClassInFlag)
            {
                // class �� enum ��`
                // Enum ��` + "_" + Enum �l����
                enumValueDef += "EnumDef";
                enumValueDef += "_";
                // enumValueDef += "EnumValueName";
                enumValueDef += enumValueName;
                enumValueDef += " ";

                // 
                enumValueDef += "\"";
                enumValueDef += ns_str;
                enumValueDef += "::";
                enumValueDef += "ClassName";
                enumValueDef += "::";
                // enumTemplateValueDef += "EnumValueName";
                if(enumValueName.empty())
                {
                    // None
                    // error?
                }
                else
                {
                    enumValueDef += enumValueName;
                }
            }
            else
            {
                // enum �P�̒�`
                // Error:
                // auto result = std::find(generatorLists.begin(), generatorLists.end(), typeid(Identifier));
                // result.
                if(enumValueName.empty())
                {
                    // None
                }
                else
                {
                    // enumValueDef += "EnumValueName";
                    enumValueDef += enumValueName;
                    enumValueDef += " ";
                }

                // generatorLists -> intliteral
                if(!enumValue.empty())
                {
                    // enumValueDef += "EnumValue";
                    enumValueDef += enumValue;
                    // enumValueDef += ",";
                }
            }

            enumValueDef += "\n";
            retStr = enumValueDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::variable_t)
        {
            std::cout << "variable_t";
            std::cout << "\n";

            std::string variableDef = "";
            std::string variableName = "";
            variableName += indentSpace;
            // variableName += indentBaseSpace;

            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";
                variableName += (*itr)->GetString();

                //IdentifierGenerator *sa = dynamic_cast<IdentifierGenerator*>(*itr);
                //if(sa != nullptr)
                //{
                //    enumValueName = sa->GetString();
                //}
                //if(typeid(*itr) == typeid(IdentifierGenerator))
                //{
                //    enumValueName = (*itr)->GetString();
                //}
            }

            variableDef = variableName;
            variableDef += "\n";
            retStr = variableDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::member_variable_t)
        {
            std::cout << "member_variable_t";
            std::cout << "\n";

            std::string memberVariableDef = "";
            std::string memberVariableName = "";
            memberVariableName += indentSpace;
            // memberVariableName += indentBaseSpace;

            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";

                // IdentifierGenerator *sa = dynamic_cast<IdentifierGenerator*>(*itr);
                // if(sa != nullptr)
                // {
                //     memberVariableName = sa->GetString();
                // }

                PunctuationGenerator *sb = dynamic_cast<PunctuationGenerator*>(*itr);
                if(sb != nullptr)
                {
                    std::string tmpPunctuation = sb->GetString();
                    if(tmpPunctuation == ";")
                    {
                    }
                    else
                    {
                        memberVariableName += tmpPunctuation;
                    }
                    continue;
                }

                memberVariableName += (*itr)->GetString();
            }
            
            memberVariableDef = memberVariableName;
            memberVariableDef += "\n";
            retStr = memberVariableDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::function_t)
        {
            std::cout << "function_t";
            std::cout << "\n";

            std::string functionDef = "";
            std::string functionName = "";
            functionName += indentSpace;
            // functionName += indentBaseSpace;
            bool isPunctuation = false;

            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";
                // functionName += (*itr)->GetString();
                KeywordGenerator *sa = dynamic_cast<KeywordGenerator*>(*itr);
                if(sa != nullptr)
                {
                    std::string tmpKeyword = sa->GetString();
                    // �ړ���
                    if(tmpKeyword == "static")
                    {
                        functionName += "@staticmethod";
                        functionName += "\n";
                        functionName += indentSpace;
                        continue;
                    }

                    // �ϐ��̖߂�l��`
                    if(isPunctuation == false)
                    {
                        /*
                        if(tmpKeyword == "int")
                        {
                            functionName += "@cython.returns(cython.int)";
                            functionName += "\n";
                        }
                        else if(tmpKeyword == "float")
                        {
                            functionName += "@cython.returns(cython.float)";
                            functionName += "\n";
                        }
                        else if(tmpKeyword == "double")
                        {
                            functionName += "@cython.returns(cython.double)";
                            functionName += "\n";
                        }
                        else
                        {
                            // ��`���Ȃ��B(�s���Ȃ���)
                        }
                        */
                        // int
                        functionName += "def";
                        // functionName += "cdef";
                    }
                    else
                    {
                        // �֐��̌Ăяo���ϐ��̌^���
                        if(tmpKeyword == "int")
                        {
                        }
                        else if(tmpKeyword == "float")
                        {
                        }
                        else if(tmpKeyword == "double")
                        {
                        }
                        functionName += tmpKeyword;
                    }
                    continue;
                }
                
                PunctuationGenerator *sb = dynamic_cast<PunctuationGenerator*>(*itr);
                if(sb != nullptr)
                {
                    std::string tmpPunctuation = sb->GetString();
                    if(tmpPunctuation == "(")
                    {
                        isPunctuation = true;
                    }
                    else if(tmpPunctuation == ")")
                    {
                        isPunctuation = false;
                    }
                    else if(tmpPunctuation == ";")
                    {
                        continue;
                    }
                    functionName += tmpPunctuation;
                    continue;
                }
                functionName += (*itr)->GetString();

                // IdentifierGenerator *sb = dynamic_cast<IdentifierGenerator*>(*itr);
                // if(sa != nullptr)
                // {
                //     functionName = sa->GetString();
                // }
            }

            functionDef = functionName;
            functionDef += "\n";
            retStr = functionDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::template_type_parameter_t)
        {
            std::cout << "template_type_parameter_t";
            std::cout << "\n";
        }
        else if(e.kind() == cppast::cpp_entity_kind::non_type_template_parameter_t)
        {
            std::cout << "non_type_template_parameter_t";
            std::cout << "\n";
        }
        else if(e.kind() == cppast::cpp_entity_kind::template_template_parameter_t)
        {
            std::cout << "template_template_parameter_t";
            std::cout << "\n";
        }
        else if(e.kind() == cppast::cpp_entity_kind::alias_template_t)
        {
            std::cout << "alias_template_t";
            std::cout << "\n";
        }
        else if(e.kind() == cppast::cpp_entity_kind::variable_template_t)
        {
            std::cout << "variable_template_t";
            std::cout << "\n";
        }
        else if(e.kind() == cppast::cpp_entity_kind::function_template_t)
        {
            std::cout << "function_template_t";
            std::cout << "\n";
        }
        else if(e.kind() == cppast::cpp_entity_kind::function_template_specialization_t)
        {
            std::cout << "function_template_specialization_t";
            std::cout << "\n";
        }
        else if(e.kind() == cppast::cpp_entity_kind::static_assert_t)
        {
            std::cout << "static_assert_t";
            std::cout << "\n";
        }
        else if(e.kind() == cppast::cpp_entity_kind::unexposed_t)
        {
        	std::cout << "unexposed_t";
            std::cout << "\n";
        }
        else
        {
            // cpp_entity_kind.hpp
            std::cout << "Unknown Param";
            std::cout << "\n";
            std::cout << (int)e.kind();
            std::cout << "\n";
            
            std::string otherDef = "";

            // class �O��`
            // generatorLists -> Identifier
            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";
            }
            otherDef += "\n";

            retStr = otherDef;
        }

        return retStr;
    }

    /*
    // IGenerator �p��
    // ��ɒ��g�̊֐��Ƃ���ݒ肷��B
    // PxdNode ��n��?
    // ���[���B�������ԋp����B
     autopxd_generator3(const std::string& ns_str, const cppast::cpp_entity& e, const std::vector<IGenerator*>& generatorLists)
    {
        bool isClose = false;
        std::string retStr = "";
        const std::string indentSpace = "    ";

        if(e.kind() == cppast::cpp_entity_kind::include_directive_t)
        {
            // TODO : Cython ���W���őΉ��ł��Ȃ��w�b�_�t�@�C���Ɋւ��ẮA�ċA�I�Ƀw�b�_����͂��Ă����d�g�݂���������?
            // �����_�ł̑Ή��͖ʓ|�Ȃ��߁A�Ή����Ȃ����Ƃɂ���
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
            // �W�� C/C++ �w�b�_�t�@�C�����ǂ����`�F�b�N����B
            for(auto itr = c_header_lists.begin(); itr != c_header_lists.end(); ++itr)
            {
                // '�W��C header check'
                if(std::string::npos != importDef.find(*itr))
                {
                    std::string repace_c_import_header = *itr;
                    myReplace(repace_c_import_header, "<", "");
                    myReplace(repace_c_import_header, ">", "");

                    // cimport �̑Ή�
                    std::string repace_c_import = "";
                    // TODO : ���o���������e�� �t�@�C�����ƈ�v���Ă��Ȃ��P�[�X������̂Œ��ӂ��Ď�������B
                    if (repace_c_import_header == "stdint.h")
                    {
                        // stdint �ǂݏo��
                        // repace_c_import = "from libc." + repace_c_import_header + " " + "cimport " + repace_c_import_header;
                        // memory �Ȃ� unique_ptr �Ƃ�?
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
                // '�W�� C++ header check'
                if(std::string::npos != importDef.find(*itr))
                {
                    std::string repace_cpp_import_header = *itr;
                    myReplace(repace_cpp_import_header, "<", "");
                    myReplace(repace_cpp_import_header, ">", "");

                    // cimport �̑Ή�
                    std::string repace_cpp_import = "";
                    if(repace_cpp_import_header == "memory")
                    {
                        // memory �Ȃ� unique_ptr �Ƃ�?
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
                        // TODO : ���o���������e�� �t�@�C�����ƈ�v���Ă��Ȃ��P�[�X������̂Œ��ӂ��Ď�������B
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
            std::string line_generator = "";
            
            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                line_generator += (*itr)->GetType();
                line_generator += (*itr)->GetString();
            }

            // base_filename
            // first line
            // second line
            classDef += indentSpace;
            // classDef += "cdef cppclass ";
            // classDef += "className";
            classDef += "cdef cpp";

            classDef += line_generator;
            classDef += ":";

            // �x�[�X�N���X�p������ꍇ
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
            std::string line_generator = "";
            
            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                line_generator += (*itr)->GetType();
                line_generator += (*itr)->GetString();
            }

            // first line
            // second line
            classTemplateDef += "cdef cppclass ";
            classTemplateDef += "className";
            // template �t�� class �̏ꍇ
            classTemplateDef += "[";
                classTemplateDef += "Test";
                // classTemplateDef += ", "
            classTemplateDef += "]";
            classTemplateDef += ":";

            // �x�[�X�N���X�p������ꍇ
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

            // ������ public �ȊO�́A�����o���Ȃ��B
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

            // public �ȊO�ł̑Ή����ǂ����邩?
            // ���� : �����Ȃ�
            retStr = constructorTemplateDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::destructor_t)
        {
            std::string destructorTemplateDef = "";
            std::string line_generator = "";

            // destruct
            if(isClassAccessPublic)
            {
                destructorTemplateDef += "Debug Str : ";
                destructorTemplateDef += line_generator;
            }
            // public �ȊO�ł̑Ή����ǂ����邩?
            // ���� : �����Ȃ�
            retStr = destructorTemplateDef;
        }
        else if(e.kind() == cppast::cpp_entity_kind::member_function_t)
        {
            // class ���� Function
            std::string classFunctionDef = "";
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
            std::cout << "enum_t";
            std::cout << "\n";

            // visit_Enum
            std::string enumTemplateDef = "";
            std::string line_generator = "";

            std::vector<std::string> items;

            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                line_generator += (*itr)->GetType();
                line_generator += (*itr)->GetString();

                items.push_back((*itr)->GetString());
            }

            // first line
            // second line
            enumTemplateDef += indentSpace;

            if(isClassAccessPublic)
            {
                // class ����`
                enumTemplateDef += "ctypedef enum ";
                enumTemplateDef += "EnumTypeNameReplace";
                enumTemplateDef +=  " ";
                enumTemplateDef +=  ns_str;
                enumTemplateDef +=  "::";
                enumTemplateDef +=  "ClassName";
                enumTemplateDef +=  "::"; 
                enumTemplateDef +=  "EnumTypeName"; // base_filename
                enumTemplateDef +=  ":";
                // enum in flag on?
                // Enum �̒l��ݒ肷��ۂ̔��f�ƂȂ�t���O�� on �ɂ���?
                isEnumClassInFlag = true;
                // self.decl_stack[0].append(Enum(escape(name, True), items, 'ctypedef'))
            }
            else
            {
                // class �O��`
                enumTemplateDef += "cdef enum ";
                enumTemplateDef += "EnumTypeName";  // base_filename
                enumTemplateDef += ":";
                // enum in flag off?
                // Enum �̒l��ݒ肷��ۂ̔��f�ƂȂ�t���O�� off �ɂ���?
                isEnumClassInFlag = false;
            }

            // Debug
            enumTemplateDef += "\n";
            enumTemplateDef += "Debug Str : ";
            enumTemplateDef += line_generator;
            retStr = enumTemplateDef;

            PxdNode* enumNode = new EnumNode(base_filename, items);
            refNodes.push(enumNode);
        }
        else if(e.kind() == cppast::cpp_entity_kind::enum_value_t)
        {
            std::cout << "enum_value_t";
            std::cout << "\n";

            std::string name;
            std::vector<std::string> items;

            std::string enumTemplateValueDef = "";
            std::string line_generator = "";
            
            // lineGeneratorStack Iterator
            for(auto itr = generatorLists.begin(); itr != generatorLists.end(); ++itr)
            {
                std::cout << (*itr)->GetType();
                std::cout << "\n";
                std::cout << (*itr)->GetString();
                std::cout << "\n";

                line_generator += (*itr)->GetType();
                line_generator += (*itr)->GetString();
                items.push_back((*itr)->GetString());
            }

            enumTemplateValueDef += indentSpace;

            if(isEnumClassInFlag)
            {
                // class ����`
                // Enum ��` + "_" + Enum �l����
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
                // class �O��`
                enumTemplateValueDef += "EnumValueName";
                enumTemplateValueDef += " ";

                enumTemplateValueDef += "EnumValue";
                enumTemplateValueDef += ",";
            }

            // Debug
            enumTemplateValueDef += "\n";
            // enumTemplateValueDef += "Debug Str : ";
            // enumTemplateValueDef += line_generator;
            // retStr = enumTemplateValueDef;

            std::cout << enumTemplateValueDef;
            std::cout << "\n";

            std::cout << items.size();
            std::cout << "\n";

            //PxdNode* enumNode = new EnumNode("hogehoge", items);
            //refNodes.push(enumNode);
            PxdNode* enumNode = refNodes.top();
            enumNode

            std::cout << refNodes.size();
            std::cout << "\n";
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

        return isClose;
    }
    */
};

// TODO: �Ώۃ��W���[���̃C���N���[�h�p�X���擾
// # BUILTIN_HEADERS_DIR = os.path.join(os.path.dirname(__file__), 'include')
// # Types declared by pycparser fake headers that we should ignore
