#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H

#include "Python3BaseVisitor.h"
#include <string>
#include <iostream>
#include "Bigint.h"
#include <map>
#include <algorithm>
#include <vector>
#include <iomanip>

std::vector<std::map<std::string, antlrcpp::Any>> glb_map;
std::map<std::string, Python3Parser::FuncdefContext *> func_map;

class EvalVisitor : public Python3BaseVisitor
{
    //flow_control
    class BREAK_STMT{ };
    class CONTINUE_STMT{ };
    class RETURN_STMT
    {
    public:
        antlrcpp::Any ret;
        RETURN_STMT(const antlrcpp::Any &tmp) { ret = tmp;};
    };

    //判断any 的类型
    int Find_type(antlrcpp::Any val)
    {
        int flag = 0;
        if (val.is<std::string>())
            flag = 1;
        else if (val.is<Bigint>())
            flag = 2;
        else if (val.is<bool>())
            flag = 3;
        else if (val.is<double>())
            flag = 4;
        return flag;
    }

    //判断变量名是否在map中
    bool Find_map_key(std::map<std::string, antlrcpp::Any> my_map, std::string key)
    {
        std::map<std::string, antlrcpp::Any>::iterator iter = my_map.begin();
        iter = my_map.find(key);
        if (iter != my_map.end())
            return true;
        else
            return false;
    }

    void Is_Val_Name(antlrcpp::Any &ret)
    {
        int map_size = glb_map.size();
        string s1 = ret.as<std::string>();
        if (s1.find("\"") == std::string::npos)
        {
            for (int i = map_size - 1; i >= 0; i--)
            {
                if (Find_map_key(glb_map[i], ret.as<std::string>()))
                {
                    ret = glb_map[i][ret.as<std::string>()];
                    break;
                }
            }
        }
    }

    antlrcpp::Any visitFile_input(Python3Parser::File_inputContext *ctx) override
    {
        std::map<std::string, antlrcpp::Any> my_map;
        glb_map.push_back(my_map);
        return visitChildren(ctx);
    }

    antlrcpp::Any visitFuncdef(Python3Parser::FuncdefContext *ctx) override
    {
        //用map 记录funcname
        std::string func_name = ctx->NAME()->toString();
        func_map[func_name] = ctx;
        return nullptr;
    }

    antlrcpp::Any visitParameters(Python3Parser::ParametersContext *ctx) override
    {
        if (ctx->typedargslist() != nullptr)
        {
            return visitTypedargslist(ctx->typedargslist());
        }
        else
            return visitChildren(ctx);
    }

    antlrcpp::Any visitTypedargslist(Python3Parser::TypedargslistContext *ctx) override
    {

        return visitChildren(ctx);
    }

    antlrcpp::Any visitTfpdef(Python3Parser::TfpdefContext *ctx) override
    {

        return visitChildren(ctx);
    }

    antlrcpp::Any visitStmt(Python3Parser::StmtContext *ctx) override
    {
        if (ctx->simple_stmt() != nullptr)
        {
            return visitSimple_stmt(ctx->simple_stmt());
        }
        else
            return visitCompound_stmt(ctx->compound_stmt());
    }

    antlrcpp::Any visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) override
    {
        return visitSmall_stmt(ctx->small_stmt());
    }

    antlrcpp::Any visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) override
    {
        if (ctx->expr_stmt() != nullptr)
            return visitExpr_stmt(ctx->expr_stmt());
        else
            return visitFlow_stmt(ctx->flow_stmt());
    }
    // flag = 1 表示字符串，2 表示 bigint  3 表示 bool   4表示double  5 none(待补充)
    antlrcpp::Any visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) override
    {
        if (ctx->testlist().size() == 1)
            return visit(ctx->testlist(0));
        int sign_num = ctx->ASSIGN().size();
        if (sign_num > 1)
        {
            antlrcpp::Any R_val = visit(ctx->testlist().back());
            int map_size = glb_map.size(); //  返回此时全局有几张图
            if (R_val.is<std::string>())
                Is_Val_Name(R_val);
            int flag = Find_type(R_val);
            for (int i = 0; i < sign_num; i++)
            {
                antlrcpp::Any name_tmp = visit(ctx->testlist(i)->test(0));
                std::string name_str = name_tmp.as<std::string>();
                switch (flag)
                {
                case 1:
                {
                    std::string tmp_Str = R_val.as<std::string>();
                    glb_map[map_size - 1][name_str] = tmp_Str;
                    break;
                }
                case 2:
                {
                    Bigint tmp_Int = R_val.as<Bigint>();
                    glb_map[map_size - 1][name_str] = tmp_Int;
                    break;
                }
                case 3:
                {
                    bool tmp_bool = R_val.as<bool>();
                    if (tmp_bool)
                    {
                        std::string true_str = "True";
                        glb_map[map_size - 1][name_str] = true_str;
                    }
                    else
                    {
                        std::string false_str = "False";
                        glb_map[map_size - 1][name_str] = false_str;
                    }
                    break;
                }
                case 4:
                {
                    double tmp_double = R_val.as<double>();
                    glb_map[map_size - 1][name_str] = tmp_double;
                    break;
                }
                default:
                {
                    std::cout << "error : Invalid type！" << std::endl;
                    break;
                }
                }
            }
            return R_val;
        }
        antlrcpp::Any ret = visit(ctx->testlist(1));
        if (sign_num == 1 && (!ret.is<std::vector<antlrcpp::Any>>()))
        {
            //总共 1 个值需要赋
            int map_size = glb_map.size();
            antlrcpp::Any le_val = visit(ctx->testlist(0)->test(0));
            antlrcpp::Any ri_val = visit(ctx->testlist(1)->test(0));
            if (ri_val.is<std::string>())
                Is_Val_Name(ri_val);
            int flag = Find_type(ri_val);
            std::string name_str = le_val.as<std::string>();
            switch (flag)
            {
            case 1:
            {
                std::string tmp_Str = ri_val.as<std::string>();
                glb_map[map_size - 1][name_str] = tmp_Str;
                break;
            }
            case 2:
            {
                Bigint tmp_Int = ri_val.as<Bigint>();
                glb_map[map_size - 1][name_str] = tmp_Int;
                break;
            }
            case 3:
            {
                bool tmp_bool = ri_val.as<bool>();
                if (tmp_bool)
                {
                    glb_map[map_size - 1][name_str] = true;
                }
                else
                {
                    glb_map[map_size - 1][name_str] = false;
                }
                break;
            }
            case 4:
            {
                double tmp_double = ri_val.as<double>();
                glb_map[map_size - 1][name_str] = tmp_double;
                break;
            }
            default:
            {
                std::cout << "error : Invalid type！" << std::endl;
                break;
            }
            }
            return 1;
        }
        else if (sign_num == 1 && ret.is<std::vector<antlrcpp::Any>>())
        {
            int val_num = ctx->testlist(0)->test().size();
            int right_num = ret.as<std::vector<antlrcpp::Any>>().size();
            int map_size = glb_map.size();
            if (val_num != right_num)
            {
                std::cerr << "Invalid assign !\n";
                exit(1);
            }
            std::vector<antlrcpp::Any> right_val = ret.as<std::vector<antlrcpp::Any>>();
            std::vector<antlrcpp::Any> right_rec;
            for (int i = 0; i < val_num; i++)
            {
                antlrcpp::Any tmp = right_val[i];
                if (tmp.is<std::string>())
                    Is_Val_Name(tmp);
                right_rec.push_back(tmp);
            }
            for (int i = 0; i < val_num; i++)
            {
                antlrcpp::Any le_val = visit(ctx->testlist(0)->test(i));
                antlrcpp::Any ri_val = right_rec[i];
                int flag = Find_type(ri_val);
                std::string name_str = le_val.as<std::string>();
                switch (flag)
                {
                case 1:
                {
                    std::string tmp_Str = ri_val.as<std::string>();
                    glb_map[map_size - 1][name_str] = tmp_Str;
                    break;
                }
                case 2:
                {
                    Bigint tmp_Int = ri_val.as<Bigint>();
                    glb_map[map_size - 1][name_str] = tmp_Int;
                    break;
                }
                case 3:
                {
                    bool tmp_bool = ri_val.as<bool>();
                    if (tmp_bool)
                    {
                        glb_map[map_size - 1][name_str] = true;
                    }
                    else
                    {
                        glb_map[map_size - 1][name_str] = false;
                    }
                    break;
                }
                case 4:
                {
                    double tmp_double = ri_val.as<double>();
                    glb_map[map_size - 1][name_str] = tmp_double;
                    break;
                }
                default:
                {
                    std::cout << "error : Invalid type！" << std::endl;
                    break;
                }
                }
            }
            return 1;
        }
        // augassign
        else
        {
            antlrcpp::Any key = visit(ctx->testlist(0));
            std::string map_key = key.as<std::string>();
            antlrcpp::Any left_val = visit(ctx->testlist(0));
            int map_size = glb_map.size(); //  返回此时全局有几张图
            int val_pos = -1;              // 记录最后边变量第一次出现实在哪张图
            for (int i = map_size - 1; i >= 0; i--)
            {
                if (Find_map_key(glb_map[i], left_val.as<std::string>()))
                {
                    val_pos = i;
                    break;
                }
            }
            if (left_val.is<std::string>())
                Is_Val_Name(left_val);
            antlrcpp::Any right_val = visit(ctx->testlist(1));
            if (right_val.is<std::string>())
                Is_Val_Name(right_val);
            antlrcpp::Any any_sign_num = visit(ctx->augassign());
            double sign_num = any_sign_num.as<double>();
            switch ((int)sign_num)
            {
            case 1: // +=
            {
                if (left_val.is<Bigint>() && right_val.is<Bigint>())
                    glb_map[val_pos][map_key] = left_val.as<Bigint>() + right_val.as<Bigint>();

                else if (left_val.is<bool>() && right_val.is<Bigint>())
                    glb_map[val_pos][map_key] = Bigint(left_val.as<bool>()) + right_val.as<Bigint>();

                else if (left_val.is<Bigint>() && right_val.is<bool>())
                    glb_map[val_pos][map_key] = left_val.as<Bigint>() + Bigint(right_val.as<bool>());

                else if (left_val.is<bool>() && right_val.is<bool>())
                    glb_map[val_pos][map_key] = left_val.as<bool>() + right_val.as<bool>();

                else if (left_val.is<double>() && right_val.is<double>())
                    glb_map[val_pos][map_key] = left_val.as<double>() + right_val.as<double>();

                else if (left_val.is<double>() && right_val.is<bool>())
                    glb_map[val_pos][map_key] = left_val.as<double>() + right_val.as<bool>();

                else if (left_val.is<bool>() && right_val.is<double>())
                    glb_map[val_pos][map_key] = left_val.as<bool>() + right_val.as<double>();

                else if (left_val.is<double>() && right_val.is<Bigint>())
                    glb_map[val_pos][map_key] = left_val.as<double>() + double(right_val.as<Bigint>());

                else if (left_val.is<Bigint>() && right_val.is<double>())
                    glb_map[val_pos][map_key] = double(left_val.as<Bigint>()) + right_val.as<double>();

                else if (left_val.is<std::string>() && right_val.is<std::string>())
                {
                    std::string s1 = left_val.as<std::string>();
                    string s2 = right_val.as<std::string>();
                    s1 = s1.substr(0, s1.size() - 1);
                    s2 = s2.substr(1, s2.size() - 1);
                    glb_map[val_pos][map_key] = s1 + s2;
                }
                else
                {
                    std::cerr << "Error: Undefined operation!\n";
                    exit(0);
                }
                break;
            }
            case 2: // -=
            {
                if (left_val.is<Bigint>() && right_val.is<Bigint>())
                    glb_map[val_pos][map_key] = left_val.as<Bigint>() - right_val.as<Bigint>();

                else if (left_val.is<bool>() && right_val.is<Bigint>())
                    glb_map[val_pos][map_key] = Bigint(left_val.as<bool>()) - right_val.as<Bigint>();

                else if (left_val.is<Bigint>() && right_val.is<bool>())
                    glb_map[val_pos][map_key] = left_val.as<Bigint>() - Bigint(right_val.as<bool>());

                else if (left_val.is<bool>() && right_val.is<bool>())
                    glb_map[val_pos][map_key] = left_val.as<bool>() - right_val.as<bool>();

                else if (left_val.is<double>() && right_val.is<double>())
                    glb_map[val_pos][map_key] = left_val.as<double>() - right_val.as<double>();

                else if (left_val.is<double>() && right_val.is<bool>())
                    glb_map[val_pos][map_key] = left_val.as<double>() - right_val.as<bool>();

                else if (left_val.is<bool>() && right_val.is<double>())
                    glb_map[val_pos][map_key] = left_val.as<bool>() - right_val.as<double>();

                else if (left_val.is<double>() && right_val.is<Bigint>())
                    glb_map[val_pos][map_key] = left_val.as<double>() - double(right_val.as<Bigint>());

                else if (left_val.is<Bigint>() && right_val.is<double>())
                    glb_map[val_pos][map_key] = double(left_val.as<Bigint>()) - right_val.as<double>();

                else
                {
                    std::cerr << "Error: Undefined operation!\n";
                    exit(0);
                }
                break;
            }
            case 3: // *=
            {
                if (left_val.is<Bigint>() && right_val.is<Bigint>())
                    glb_map[val_pos][map_key] = left_val.as<Bigint>() * right_val.as<Bigint>();

                else if (left_val.is<bool>() && right_val.is<Bigint>())
                    glb_map[val_pos][map_key] = Bigint(left_val.as<bool>()) * right_val.as<Bigint>();

                else if (left_val.is<Bigint>() && right_val.is<bool>())
                    glb_map[val_pos][map_key] = left_val.as<Bigint>() * Bigint(right_val.as<bool>());

                else if (left_val.is<bool>() && right_val.is<bool>())
                    glb_map[val_pos][map_key] = left_val.as<bool>() * right_val.as<bool>();

                else if (left_val.is<double>() && right_val.is<double>())
                    glb_map[val_pos][map_key] = left_val.as<double>() * right_val.as<double>();

                else if (left_val.is<double>() && right_val.is<bool>())
                    glb_map[val_pos][map_key] = left_val.as<double>() * right_val.as<bool>();

                else if (left_val.is<bool>() && right_val.is<double>())
                    glb_map[val_pos][map_key] = left_val.as<bool>() * right_val.as<double>();

                else if (left_val.is<double>() && right_val.is<Bigint>())
                    glb_map[val_pos][map_key] = left_val.as<double>() * double(right_val.as<Bigint>());

                else if (left_val.is<Bigint>() && right_val.is<double>())
                    glb_map[val_pos][map_key] = double(left_val.as<Bigint>()) * right_val.as<double>();

                else if (left_val.is<Bigint>() && right_val.is<std::string>())
                {
                    if (left_val.as<Bigint>() == Bigint(0))
                    {
                        glb_map[val_pos][map_key] = "";
                    }
                    else
                    {
                        std::string str = right_val.as<std::string>();
                        std::string str_right = str.substr(1, str.size() - 2);
                        str = str.substr(0, str.size() - 1);
                        double cnt = double(left_val.as<Bigint>());
                        for (double i = 1; i < cnt; i++)
                        {
                            str = str + str_right;
                        }
                        str += "\"";
                        glb_map[val_pos][map_key] = str;
                    }
                }
                else if (left_val.is<std::string>() && right_val.is<Bigint>())
                {
                    if (right_val.as<Bigint>() == Bigint(0))
                    {
                        glb_map[val_pos][map_key] = "";
                    }
                    else
                    {
                        std::string str = left_val.as<std::string>();
                        std::string str_right = str.substr(1, str.size() - 2);
                        str = str.substr(0, str.size() - 1);
                        double cnt = double(right_val.as<Bigint>()); // 可能会出事
                        for (double i = 1; i < cnt; i++)
                        {
                            str = str + str_right;
                        }
                        str += "\"";
                        glb_map[val_pos][map_key] = str;
                    }
                }
                else if (left_val.is<bool>() && right_val.is<std::string>())
                {
                    if (left_val.as<bool>())
                        glb_map[val_pos][map_key] = right_val.as<std::string>();
                    else
                        glb_map[val_pos][map_key] = "";
                }
                else if (left_val.is<std::string>() && right_val.is<bool>())
                {
                    if (right_val.as<bool>())
                        glb_map[val_pos][map_key] = left_val.as<std::string>();
                    else
                        glb_map[val_pos][map_key] = "";
                }
                else
                {
                    std::cerr << "Error: Undefined operation!\n";
                    exit(0);
                }
                break;
            }
            case 4: //    /=
            {
                if (left_val.is<Bigint>() && right_val.is<Bigint>())
                {
                    if (right_val.as<Bigint>() == (Bigint)0)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    else
                    {
                        antlrcpp::Any ans = (double)left_val.as<Bigint>() / (double)right_val.as<Bigint>();
                        glb_map[val_pos][map_key] = ans;
                    }
                }
                else if (left_val.is<bool>() && right_val.is<Bigint>())
                {
                    if (right_val.as<Bigint>() == Bigint(0))
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    glb_map[val_pos][map_key] = double(left_val.as<bool>()) / (double)right_val.as<Bigint>();
                }
                else if (left_val.is<Bigint>() && right_val.is<bool>())
                {
                    if (right_val.as<bool>() == false)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    glb_map[val_pos][map_key] = (double)left_val.as<Bigint>() / (double)(right_val.as<bool>());
                }
                else if (left_val.is<bool>() && right_val.is<bool>())
                {
                    if (right_val.as<bool>() == false)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    glb_map[val_pos][map_key] = (double)left_val.as<bool>() / (double)right_val.as<bool>();
                }
                else if (left_val.is<double>() && right_val.is<Bigint>())
                {
                    if (right_val.as<Bigint>() == (Bigint)0)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    else
                        glb_map[val_pos][map_key] = left_val.as<double>() / (double)right_val.as<Bigint>();
                }
                else if (left_val.is<Bigint>() && right_val.is<double>())
                {
                    if (right_val.as<double>() == 0)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    else
                        glb_map[val_pos][map_key] = (double)left_val.as<Bigint>() / right_val.as<double>();
                }
                else if (left_val.is<double>() && right_val.is<bool>())
                {
                    if (right_val.as<bool>() == false)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    else
                        glb_map[val_pos][map_key] = left_val.as<double>() / (double)right_val.as<bool>();
                }
                else if (left_val.is<bool>() && right_val.is<double>())
                {
                    if (right_val.as<double>() == 0)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    else
                        glb_map[val_pos][map_key] = (double)left_val.as<bool>() / right_val.as<double>();
                }
                else if (left_val.is<double>() && right_val.is<double>())
                {
                    if (right_val.as<double>() == 0)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    else
                        glb_map[val_pos][map_key] = left_val.as<double>() / right_val.as<double>();
                }
                else
                {
                    std::cerr << "Error: Undefined operation!\n";
                    exit(0);
                }
                break;
            }
            case 5: //   //=
            {
                if (left_val.is<Bigint>() && right_val.is<Bigint>())
                {
                    if (right_val.as<Bigint>() == (Bigint)0)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    else
                        glb_map[val_pos][map_key] = left_val.as<Bigint>() / right_val.as<Bigint>();
                }

                else if (left_val.is<bool>() && right_val.is<Bigint>())
                {
                    if (right_val.as<Bigint>() == Bigint(0))
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    glb_map[val_pos][map_key] = Bigint(left_val.as<bool>()) / right_val.as<Bigint>();
                }
                else if (left_val.is<Bigint>() && right_val.is<bool>())
                {
                    if (right_val.as<bool>() == false)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    glb_map[val_pos][map_key] = left_val.as<Bigint>() / Bigint(right_val.as<bool>());
                }
                else if (left_val.is<bool>() && right_val.is<bool>())
                {
                    if (right_val.as<bool>() == false)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    glb_map[val_pos][map_key] = left_val.as<bool>() / right_val.as<bool>();
                }
                else if (left_val.is<double>() || right_val.is<double>())
                {
                    std::cerr << "Undefined behavior!\n";
                    exit(0);
                }
                else
                {
                    std::cerr << "Error: Undefined operation!\n";
                    exit(0);
                }
                break;
            }
            case 6:
            {
                if (left_val.is<Bigint>() && right_val.is<Bigint>())
                {
                    if (right_val.as<Bigint>() == (Bigint)0)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    else
                        glb_map[val_pos][map_key] = left_val.as<Bigint>() % right_val.as<Bigint>();
                }

                else if (left_val.is<bool>() && right_val.is<Bigint>())
                {
                    if (right_val.as<Bigint>() == Bigint(0))
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    glb_map[val_pos][map_key] = Bigint(left_val.as<bool>()) % right_val.as<Bigint>();
                }
                else if (left_val.is<Bigint>() && right_val.is<bool>())
                {
                    if (right_val.as<bool>() == false)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    glb_map[val_pos][map_key] = left_val.as<Bigint>() % Bigint(right_val.as<bool>());
                }
                else if (left_val.is<bool>() && right_val.is<bool>())
                {
                    if (right_val.as<bool>() == false)
                    {
                        std::cerr << "Error : The divisor cannot be zero!\n";
                        exit(0);
                    }
                    glb_map[val_pos][map_key] = left_val.as<bool>() % right_val.as<bool>();
                }
                else if (left_val.is<double>() || right_val.is<double>())
                {
                    std::cerr << "Undefined behavior!\n";
                    exit(0);
                }
                else
                {
                    std::cerr << "Error: Undefined operation!\n";
                    exit(0);
                }
                break;
            }
            default:
            {
                std::cerr << "Error : Undefined operator\n";
                exit(0);
            }
            }
            return left_val;
        }
    }

    antlrcpp::Any visitAugassign(Python3Parser::AugassignContext *ctx) override
    {
        if (ctx->ADD_ASSIGN() != nullptr)
        {
            return 1.0;
        }
        else if (ctx->SUB_ASSIGN() != nullptr)
        {
            return 2.0;
        }
        else if (ctx->MULT_ASSIGN() != nullptr)
        {
            return 3.0;
        }
        else if (ctx->DIV_ASSIGN() != nullptr)
        {
            return 4.0;
        }
        else if (ctx->IDIV_ASSIGN() != nullptr)
        {
            return 5.0;
        }
        else if (ctx->MULT_ASSIGN() != nullptr)
        {
            return 6.0;
        }
    }

    antlrcpp::Any visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) override
    {
        return visitChildren(ctx);
    }

    antlrcpp::Any visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) override
    {
        if (ctx->BREAK() != nullptr)
        {
            BREAK_STMT x;
            return x;
        }
        return visitChildren(ctx);
    }

    antlrcpp::Any visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) override
    {
        if (ctx->CONTINUE() != nullptr)
        {
            CONTINUE_STMT x;
            return x;
        }
        return visitChildren(ctx);
    }

    antlrcpp::Any visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) override
    {
        std::vector<antlrcpp::Any> tmp;
        if (ctx->testlist() != nullptr)
        {
            antlrcpp::Any ret = visit(ctx->testlist());
            if (ret.is<std::vector<antlrcpp::Any>>())
            {
                tmp = ret.as<std::vector<antlrcpp::Any>>();
                return RETURN_STMT(tmp);
            }
            else
                return RETURN_STMT(ret);
        }
        else
            return RETURN_STMT(tmp);
    }

    antlrcpp::Any visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) override
    {
        if (ctx->if_stmt() != nullptr)
            return visitIf_stmt(ctx->if_stmt());
        else if (ctx->while_stmt() != nullptr)
            return visitWhile_stmt(ctx->while_stmt());
        else
            return visitFuncdef(ctx->funcdef());
    }

    antlrcpp::Any visitIf_stmt(Python3Parser::If_stmtContext *ctx) override
    {
        int flag = 0;
        int num = ctx->test().size();
        int map_size = glb_map.size();
        for (int i = 0; i < num; i++)
        {
            antlrcpp::Any judge = visit(ctx->test(i));
            if (judge.is<std::string>())
                Is_Val_Name(judge);
            if (judge.is<bool>())
            {
                if (judge.as<bool>())
                    flag = 1;
            }
            else if (judge.is<std::string>())
            {
                if (!judge.as<std::string>().empty())
                    flag = 1;
            }
            else if (judge.is<Bigint>())
            {
                if (judge.as<Bigint>() != Bigint(0))
                    flag = 1;
            }
            else if (judge.is<double>())
            {
                if (judge.as<double>() != 0)
                    flag = 1;
            }
            else
            {
                std::cerr << "Error : Invalid judgment conditions !\n";
                exit(0);
            }
            if (flag)
            {
                return visitSuite(ctx->suite(i));
                break;
            }
        }
        if (ctx->ELSE() != nullptr && flag == 0)
        {
            return visitSuite(ctx->suite(num));
        }
        return nullptr;
    }

    antlrcpp::Any visitWhile_stmt(Python3Parser::While_stmtContext *ctx) override
    {
        antlrcpp::Any judge = visit(ctx->test());
        antlrcpp::Any ret;
        int flag = 0;
        int map_size = glb_map.size();
        if (judge.is<std::string>())
            Is_Val_Name(judge);
        if (judge.is<bool>())
        {
            if (judge.as<bool>())
                flag = 1;
        }
        else if (judge.is<std::string>())
        {
            if (!judge.as<std::string>().empty())
                flag = 1;
        }
        else if (judge.is<Bigint>())
        {
            if (judge.as<Bigint>() != Bigint(0))
                flag = 1;
        }
        else if (judge.is<double>())
        {
            if (judge.as<double>() != 0)
                flag = 1;
        }
        else
        {
            std::cerr << "Error : Invalid judgment conditions !\n";
            exit(0);
        }
        while (flag)
        {
            ret = visitSuite(ctx->suite());
            judge = visit(ctx->test());
            flag = 0;
            if (ret.is<BREAK_STMT>())
            {
                ret = nullptr;
                break;
            }
            else if (ret.is<CONTINUE_STMT>())
            {
                ret = nullptr;
                continue;
            }
            else if (ret.is<RETURN_STMT>())
                break;
            if (judge.is<std::string>())
                Is_Val_Name(judge);
            if (judge.is<bool>())
            {
                if (judge.as<bool>())
                    flag = 1;
            }
            else if (judge.is<std::string>())
            {
                if (!judge.as<std::string>().empty())
                    flag = 1;
            }
            else if (judge.is<Bigint>())
            {
                if (judge.as<Bigint>() != Bigint(0))
                    flag = 1;
            }
            else if (judge.is<double>())
            {
                if (judge.as<double>() != 0)
                    flag = 1;
            }
            else
            {
                std::cerr << "Error : Invalid judgment conditions !\n";
                exit(0);
            }
        }
        return ret;
    }

    antlrcpp::Any visitSuite(Python3Parser::SuiteContext *ctx) override
    {
        antlrcpp::Any ret = nullptr;
        if (ctx->simple_stmt() != nullptr)
        {
            ret = visit(ctx->simple_stmt());
            return ret;
        }
        else
        {
            int num = ctx->stmt().size();
            for (int i = 0; i < num; i++)
            {
                ret = visit(ctx->stmt(i));
                if (ret.is<BREAK_STMT>() || ret.is<CONTINUE_STMT>() || ret.is<RETURN_STMT>())
                    break;
            }
            return ret;
        }
    }

    antlrcpp::Any visitTest(Python3Parser::TestContext *ctx) override
    {
        return visitOr_test(ctx->or_test());
    }

    antlrcpp::Any visitOr_test(Python3Parser::Or_testContext *ctx) override
    {
        if (ctx->and_test().size() == 1)
            return visitAnd_test(ctx->and_test(0));
        else
        {
            int x = ctx->and_test().size();
            int map_size = glb_map.size();
            for (int i = 0; i < x; i++)
            {
                antlrcpp::Any tmp_and = visit(ctx->and_test(i));
                if (tmp_and.is<std::string>())
                    Is_Val_Name(tmp_and);
                if (tmp_and.is<Bigint>())
                {
                    if (tmp_and.as<Bigint>())
                        return true;
                    else
                        continue;
                }
                else if (tmp_and.is<double>())
                {
                    if (tmp_and.as<double>())
                        return true;
                    else
                        continue;
                }
                else if (tmp_and.is<bool>())
                {
                    if (tmp_and.as<bool>())
                        return true;
                    else
                        continue;
                }
                else if (tmp_and.is<std::string>())
                {
                    std::string str = tmp_and.as<std::string>();
                    str = str.substr(1, str.size() - 2);
                    if (!str.empty())
                        return true;
                    else
                        continue;
                }
                else
                {
                    std::cerr << "Errortype ! \n";
                    exit(0);
                }
            }
            return false;
        }
    }

    antlrcpp::Any visitAnd_test(Python3Parser::And_testContext *ctx) override
    {
        if (ctx->not_test().size() == 1)
            return visitNot_test(ctx->not_test(0));
        else
        {
            int x = ctx->not_test().size();
            for (int i = 0; i < x; i++)
            {
                antlrcpp::Any tmp_not = visit(ctx->not_test(i));
                int map_size = glb_map.size();
                if (tmp_not.is<std::string>())
                    Is_Val_Name(tmp_not);
                if (tmp_not.is<Bigint>())
                {
                    if (tmp_not.as<Bigint>())
                        continue;
                    else
                        return false;
                }
                else if (tmp_not.is<double>())
                {
                    if (tmp_not.as<double>())
                        continue;
                    else
                        return false;
                }
                else if (tmp_not.is<bool>())
                {
                    if (tmp_not.as<bool>())
                        continue;
                    else
                        return false;
                }
                else if (tmp_not.is<std::string>())
                {
                    std::string s1 = tmp_not.as<std::string>();
                    s1 = s1.substr(1, s1.size() - 2);
                    if (!s1.empty())
                        continue;
                    else
                        return false;
                }
                else
                {
                    std::cerr << "Errortype ! \n";
                    exit(0);
                }
            }
            return true;
        }
    }

    antlrcpp::Any visitNot_test(Python3Parser::Not_testContext *ctx) override
    {
        if (ctx->comparison() != nullptr)
            return visitComparison(ctx->comparison());
        else
        {
            antlrcpp::Any tmp_end_not = visit(ctx->not_test());
            int map_size = glb_map.size();
            if (tmp_end_not.is<std::string>())
                Is_Val_Name(tmp_end_not);
            if (tmp_end_not.is<Bigint>())
            {
                if (tmp_end_not.as<Bigint>())
                    return false;
                else
                    return true;
            }
            else if (tmp_end_not.is<double>())
            {
                if (tmp_end_not.as<double>())
                    return false;
                else
                    return true;
            }
            else if (tmp_end_not.is<bool>())
            {
                if (tmp_end_not.as<bool>())
                    return false;
                else
                    return true;
            }
            else if (tmp_end_not.is<std::string>())
            {
                std::string s1 = tmp_end_not.as<std::string>();
                s1 = s1.substr(1, s1.size() - 2);
                if (s1.empty())
                    return true;
                else
                    return false;
            }
            else
            {
                std::cerr << "Errortype ! \n";
                exit(0);
            }
        }
    }

    antlrcpp::Any visitComparison(Python3Parser::ComparisonContext *ctx) override
    {
        if (ctx->comp_op().empty())
            return visitArith_expr(ctx->arith_expr(0));
        else
        {
            int x = ctx->comp_op().size();
            bool flag = 1;
            int map_size = glb_map.size();
            antlrcpp::Any left_val = visit(ctx->arith_expr(0));
            if (left_val.is<std::string>())
                Is_Val_Name(left_val);
            std::vector<antlrcpp::Any> rec;
            rec.push_back(left_val);
            for (int i = 0; i < x; i++)
            {
                left_val = rec[0];
                antlrcpp::Any comp_token_enum = visit(ctx->comp_op(i));
                double comp_token = comp_token_enum.as<double>();
                antlrcpp::Any right_val = visit(ctx->arith_expr(i + 1));
                if (right_val.is<std::string>())
                    Is_Val_Name(right_val);
                rec.pop_back();
                rec.push_back(right_val);
                switch ((int)comp_token)
                {
                case 1: // <
                {
                    if (left_val.is<Bigint>() && right_val.is<Bigint>())
                    {
                        if (left_val.as<Bigint>() >= right_val.as<Bigint>())
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<Bigint>())
                    {
                        if (Bigint(left_val.as<bool>()) >= right_val.as<Bigint>())
                            flag = 0;
                    }
                    else if (left_val.is<Bigint>() && right_val.is<bool>())
                    {
                        if (left_val.as<Bigint>() >= Bigint(right_val.as<bool>()))
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<bool>())
                    {
                        if (left_val.as<bool>() >= right_val.as<bool>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<double>())
                    {
                        if (left_val.as<double>() >= right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<bool>())
                    {
                        if (left_val.as<double>() >= right_val.as<bool>())
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<double>())
                    {
                        if (left_val.as<bool>() >= right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<Bigint>())
                    {
                        // 可能会出问题   Bigint 转 double 精度丢失
                        if (left_val.as<double>() >= double(right_val.as<Bigint>()))
                            flag = 0;
                    }
                    else if (left_val.is<Bigint>() && right_val.is<double>())
                    {
                        // 可能会出问题   Bigint 转 double 精度丢失
                        if (double(left_val.as<Bigint>()) >= right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<std::string>() && right_val.is<std::string>())
                    {
                        if (left_val.as<std::string>() == "None" || right_val.as<std::string>() == "None")
                            flag = 0;
                        else 
                        {
                            std::string s1 = left_val.as<std::string>();
                            std::string s2 = right_val.as<std::string>();
                            s1 = s1.substr(1, s1.size() - 2);
                            s2 = s2.substr(1, s2.size() - 2);
                            if (s1 >= s2)
                                flag = 0;
                        }
                    }
                    else
                    {
                        std::cerr << "TypeError: '<' not supported between these two instances !";
                        exit(0);
                    }
                    break;
                }
                case 2: // >
                {
                    if (left_val.is<Bigint>() && right_val.is<Bigint>())
                    {
                        if (left_val.as<Bigint>() <= right_val.as<Bigint>())
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<Bigint>())
                    {
                        if (Bigint(left_val.as<bool>()) <= right_val.as<Bigint>())
                            flag = 0;
                    }
                    else if (left_val.is<Bigint>() && right_val.is<bool>())
                    {
                        if (left_val.as<Bigint>() <= Bigint(right_val.as<bool>()))
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<bool>())
                    {
                        if (left_val.as<bool>() <= right_val.as<bool>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<double>())
                    {
                        if (left_val.as<double>() <= right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<bool>())
                    {
                        if (left_val.as<double>() <= right_val.as<bool>())
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<double>())
                    {
                        if (left_val.as<bool>() <= right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<Bigint>())
                    {
                        // 可能会出问题   Bigint 转 double 精度丢失
                        if (left_val.as<double>() <= double(right_val.as<Bigint>()))
                            flag = 0;
                    }
                    else if (left_val.is<Bigint>() && right_val.is<double>())
                    {
                        // 可能会出问题   Bigint 转 double 精度丢失
                        if (double(left_val.as<Bigint>()) <= right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<std::string>() && right_val.is<std::string>())
                    {
                        if (left_val.as<std::string>() == "None" || right_val.as<std::string>() == "None")
                            flag = 0;
                        else
                        {
                            std::string s1 = left_val.as<std::string>();
                            std::string s2 = right_val.as<std::string>();
                            s1 = s1.substr(1, s1.size() - 2);
                            s2 = s2.substr(1, s2.size() - 2);
                            if (s1 <= s2)
                                flag = 0;
                        }
                    }
                    else
                    {
                        std::cerr << "TypeError: '>' not supported between these two instances !";
                        exit(0);
                    }
                    break;
                }
                case 3: // ==
                {
                    if (left_val.is<Bigint>() && right_val.is<Bigint>())
                    {
                        if (left_val.as<Bigint>() != right_val.as<Bigint>())
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<Bigint>())
                    {
                        if (Bigint(left_val.as<bool>()) != right_val.as<Bigint>())
                            flag = 0;
                    }
                    else if (left_val.is<Bigint>() && right_val.is<bool>())
                    {
                        if (left_val.as<Bigint>() != Bigint(right_val.as<bool>()))
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<bool>())
                    {
                        if (left_val.as<bool>() != right_val.as<bool>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<double>())
                    {
                        if (left_val.as<double>() != right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<bool>())
                    {
                        if (left_val.as<double>() != right_val.as<bool>())
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<double>())
                    {
                        if (left_val.as<bool>() != right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<Bigint>())
                    {
                        // 可能会出问题   Bigint 转 double 精度丢失
                        if (left_val.as<double>() != double(right_val.as<Bigint>()))
                            flag = 0;
                    }
                    else if (left_val.is<Bigint>() && right_val.is<double>())
                    {
                        // 可能会出问题   Bigint 转 double 精度丢失
                        if (double(left_val.as<Bigint>()) != right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<std::string>() && right_val.is<std::string>())
                    {
                        if (left_val.as<std::string>() != right_val.as<std::string>())
                            flag = 0;
                    }
                    else if (left_val.is<std::string>() && right_val.is<std::string>())
                    {
                        if (left_val.as<std::string>() == "None" || right_val.as<std::string>() == "None")
                            flag = 0;
                        else
                        {
                            std::string s1 = left_val.as<std::string>();
                            std::string s2 = right_val.as<std::string>();
                            s1 = s1.substr(1, s1.size() - 2);
                            s2 = s2.substr(1, s2.size() - 2);
                            if (s1 != s2)
                                flag = 0;
                        }
                    }
                    else
                    {
                        std::cerr << "TypeError: '==' not supported between these two instances !";
                        exit(0);
                    }
                    break;
                }
                case 4: // >=
                {
                    if (left_val.is<Bigint>() && right_val.is<Bigint>())
                    {
                        if (left_val.as<Bigint>() < right_val.as<Bigint>())
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<Bigint>())
                    {
                        if (Bigint(left_val.as<bool>()) < right_val.as<Bigint>())
                            flag = 0;
                    }
                    else if (left_val.is<Bigint>() && right_val.is<bool>())
                    {
                        if (left_val.as<Bigint>() < Bigint(right_val.as<bool>()))
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<bool>())
                    {
                        if (left_val.as<bool>() < right_val.as<bool>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<double>())
                    {
                        if (left_val.as<double>() < right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<bool>())
                    {
                        if (left_val.as<double>() < right_val.as<bool>())
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<double>())
                    {
                        if (left_val.as<bool>() < right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<Bigint>())
                    {
                        // 可能会出问题   Bigint 转 double 精度丢失
                        if (left_val.as<double>() < double(right_val.as<Bigint>()))
                            flag = 0;
                    }
                    else if (left_val.is<Bigint>() && right_val.is<double>())
                    {
                        // 可能会出问题   Bigint 转 double 精度丢失
                        if (double(left_val.as<Bigint>()) < right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<std::string>() && right_val.is<std::string>())
                    {
                        if (left_val.as<std::string>() == "None" || right_val.as<std::string>() == "None")
                            flag = 0;
                        else
                        {
                            std::string s1 = left_val.as<std::string>();
                            std::string s2 = right_val.as<std::string>();
                            s1 = s1.substr(1, s1.size() - 2);
                            s2 = s2.substr(1, s2.size() - 2);
                            if (s1 < s2)
                                flag = 0;
                        }
                    }
                    else
                    {
                        std::cerr << "TypeError: '>=' not supported between these two instances !";
                        exit(0);
                    }
                    break;
                }
                case 5: // <=
                {
                    if (left_val.is<Bigint>() && right_val.is<Bigint>())
                    {
                        if (left_val.as<Bigint>() > right_val.as<Bigint>())
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<Bigint>())
                    {
                        if (Bigint(left_val.as<bool>()) > right_val.as<Bigint>())
                            flag = 0;
                    }
                    else if (left_val.is<Bigint>() && right_val.is<bool>())
                    {
                        if (left_val.as<Bigint>() > Bigint(right_val.as<bool>()))
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<bool>())
                    {
                        if (left_val.as<bool>() > right_val.as<bool>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<double>())
                    {
                        if (left_val.as<double>() > right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<bool>())
                    {
                        if (left_val.as<double>() > right_val.as<bool>())
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<double>())
                    {
                        if (left_val.as<bool>() > right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<Bigint>())
                    {
                        // 可能会出问题   Bigint 转 double 精度丢失
                        if (left_val.as<double>() > double(right_val.as<Bigint>()))
                            flag = 0;
                    }
                    else if (left_val.is<Bigint>() && right_val.is<double>())
                    {
                        // 可能会出问题   Bigint 转 double 精度丢失
                        if (double(left_val.as<Bigint>()) > right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<std::string>() && right_val.is<std::string>())
                    {
                        if (left_val.as<std::string>() == "None" || right_val.as<std::string>() == "None")
                            flag = 0;
                        else
                        {
                            std::string s1 = left_val.as<std::string>();
                            std::string s2 = right_val.as<std::string>();
                            s1 = s1.substr(1, s1.size() - 2);
                            s2 = s2.substr(1, s2.size() - 2);
                            if (s1 > s2)
                                flag = 0;
                        }
                    }
                    else
                    {
                        std::cerr << "TypeError: '<=' not supported between these two instances !";
                        exit(0);
                    }
                    break;
                }
                case 6: // !=
                {
                    if (left_val.is<Bigint>() && right_val.is<Bigint>())
                    {
                        if (left_val.as<Bigint>() == right_val.as<Bigint>())
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<Bigint>())
                    {
                        if (Bigint(left_val.as<bool>()) == right_val.as<Bigint>())
                            flag = 0;
                    }
                    else if (left_val.is<Bigint>() && right_val.is<bool>())
                    {
                        if (left_val.as<Bigint>() == Bigint(right_val.as<bool>()))
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<bool>())
                    {
                        if (left_val.as<bool>() == right_val.as<bool>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<double>())
                    {
                        if (left_val.as<double>() == right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<bool>())
                    {
                        if (left_val.as<double>() == right_val.as<bool>())
                            flag = 0;
                    }
                    else if (left_val.is<bool>() && right_val.is<double>())
                    {
                        if (left_val.as<bool>() == right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<double>() && right_val.is<Bigint>())
                    {
                        // 可能会出问题   Bigint 转 double 精度丢失
                        if (left_val.as<double>() == double(right_val.as<Bigint>()))
                            flag = 0;
                    }
                    else if (left_val.is<Bigint>() && right_val.is<double>())
                    {
                        // 可能会出问题   Bigint 转 double 精度丢失
                        if (double(left_val.as<Bigint>()) == right_val.as<double>())
                            flag = 0;
                    }
                    else if (left_val.is<std::string>() && right_val.is<std::string>())
                    {
                        if (left_val.as<std::string>() == "None" || right_val.as<std::string>() == "None")
                            flag = 0;
                        else
                        {
                            std::string s1 = left_val.as<std::string>();
                            std::string s2 = right_val.as<std::string>();
                            s1 = s1.substr(1, s1.size() - 2);
                            s2 = s2.substr(1, s2.size() - 2);
                            if (s1 == s2)
                                flag = 0;
                        }
                    }
                    else
                    {
                        std::cerr << "TypeError: '!=' not supported between these two instances !";
                        exit(0);
                    }
                    break;
                }
                default:
                {
                    std::cerr << "Error : Undefined comparison symbol ! \n";
                    exit(0);
                }
                }
                if (flag == 0)
                {
                    return false;
                }
            }
            return true;
        }
    }

    antlrcpp::Any visitComp_op(Python3Parser::Comp_opContext *ctx) override
    {
        if (ctx->LESS_THAN() != nullptr)
        {
            return 1.0;
        }
        else if (ctx->GREATER_THAN() != nullptr)
        {
            return 2.0;
        }
        else if (ctx->EQUALS() != nullptr)
        {
            return 3.0;
        }
        else if (ctx->GT_EQ() != nullptr)
        {
            return 4.0;
        }
        else if (ctx->LT_EQ() != nullptr)
        {
            return 5.0;
        }
        else if (ctx->NOT_EQ_2() != nullptr)
        {
            return 6.0;
        }
    }

    antlrcpp::Any visitArith_expr(Python3Parser::Arith_exprContext *ctx) override
    {
        if (ctx->term().size() == 1)
            return visitTerm(ctx->term(0));
        else
        {
            int token_num = ctx->term().size() - 1;
            int add_rec = 0, minus_rec = 0;
            int add_size = ctx->ADD().size();
            int minus_size = ctx->MINUS().size();
            antlrcpp::Any ans = visit(ctx->term(0));
            int map_size = glb_map.size();
            //判断左值是否为变量名
            if (ans.is<std::string>())
                Is_Val_Name(ans);
            int x = Find_type(ans);
            int add_index = 1e9;
            int minus_index = 1e9;
            for (int i = 0; i < token_num; i++)
            {
                antlrcpp::Any tmp = visit(ctx->term(i + 1));
                //判断右值是否为变量名
                if (tmp.is<std::string>())
                    Is_Val_Name(tmp);
                if (add_size > 0 && add_rec != add_size)
                {
                    add_index = ctx->ADD()[add_rec]->getSymbol()->getTokenIndex();
                }
                else
                {
                    add_index = 1e9;
                }
                if (minus_size > 0 && minus_rec != minus_size)
                {
                    minus_index = ctx->MINUS()[minus_rec]->getSymbol()->getTokenIndex();
                }
                else
                {
                    minus_index = 1e9;
                }
                if (add_index < minus_index)
                {
                    // Bigint + Bigint
                    if (ans.is<Bigint>() && tmp.is<Bigint>())
                    {
                        ans = ans.as<Bigint>() + tmp.as<Bigint>();
                    }
                    //double + Bigint
                    else if (ans.is<double>() && tmp.is<Bigint>())
                    {
                        ans = ans.as<double>() + double(tmp.as<Bigint>());
                    }
                    //Bigint + double
                    else if (ans.is<Bigint>() && tmp.is<double>())
                    {
                        ans = double(ans.as<Bigint>()) + tmp.as<double>();
                    }
                    //double + double
                    else if (ans.is<double>() && tmp.is<double>())
                    {
                        ans = ans.as<double>() + tmp.as<double>();
                    }
                    //Bigint + bool
                    else if (ans.is<Bigint>() && tmp.is<bool>())
                    {
                        ans = ans.as<Bigint>() + Bigint(tmp.as<bool>());
                    }
                    //bool + bigint
                    else if (ans.is<bool>() && tmp.is<Bigint>())
                    {
                        ans = Bigint(ans.as<bool>()) + tmp.as<Bigint>();
                    }
                    //bool + bool
                    else if (ans.is<bool>() && tmp.is<bool>())
                    {
                        ans = (Bigint)ans.as<bool>() + (Bigint)tmp.as<bool>();
                    }
                    //bool + double
                    else if (ans.is<bool>() && tmp.is<double>())
                    {
                        ans = double(ans.as<bool>()) + tmp.as<double>();
                    }
                    //double + bool
                    else if (ans.is<double>() && tmp.is<bool>())
                    {
                        ans = ans.as<double>() + double(tmp.as<bool>());
                    }
                    //string + string
                    else if (ans.is<std::string>() && tmp.is<std::string>())
                    {
                        std::string s1 = ans.as<std::string>();
                        std::string s2 = tmp.as<std::string>();
                        s1 = s1.substr(0, s1.size() - 1);
                        s2 = s2.substr(1, s2.size() - 1);
                        ans = s1 + s2;
                    }
                    else
                    {
                        std::cerr << "Error operation type! \n ";
                        exit(0);
                    }
                    add_rec += 1;
                }
                else if (add_index > minus_index)
                {
                    //Bigint - Bigint
                    if (ans.is<Bigint>() && tmp.is<Bigint>())
                    {
                        ans = ans.as<Bigint>() - tmp.as<Bigint>();
                    }
                    //double - Bigint
                    else if (ans.is<double>() && tmp.is<Bigint>())
                    {
                        ans = ans.as<double>() - double(tmp.as<Bigint>());
                    }
                    //Bigint - double
                    else if (ans.is<Bigint>() && tmp.is<double>())
                    {
                        ans = double(ans.as<Bigint>()) - tmp.as<double>();
                    }
                    //double - double
                    else if (ans.is<double>() && tmp.is<double>())
                    {
                        ans = ans.as<double>() - tmp.as<double>();
                    }
                    //Bigint - bool
                    else if (ans.is<Bigint>() && tmp.is<bool>())
                    {
                        ans = ans.as<Bigint>() - Bigint(tmp.as<bool>());
                    }
                    //bool - Bigint
                    else if (ans.is<bool>() && tmp.is<Bigint>())
                    {
                        ans = Bigint((ans.as<bool>())) - tmp.as<Bigint>();
                    }
                    //bool - bool
                    else if (ans.is<bool>() && tmp.is<bool>())
                    {
                        ans = Bigint((ans.as<bool>())) - Bigint((tmp.as<bool>()));
                    }
                    //bool - double
                    else if (ans.is<bool>() && tmp.is<double>())
                    {
                        ans = double(ans.as<bool>()) - tmp.as<double>();
                    }
                    //double - bool
                    else if (ans.is<double>() && tmp.is<bool>())
                    {
                        ans = ans.as<double>() - double(tmp.as<bool>());
                    }
                    else
                    {
                        std::cerr << "Error operation type! \n ";
                        exit(0);
                    }
                    minus_rec += 1;
                }
            }
            return ans;
        }
    }

    antlrcpp::Any visitTerm(Python3Parser::TermContext *ctx) override
    {
        if (ctx->factor().size() == 1)
            return visitFactor(ctx->factor(0));
        else
        {
            // star *   div /   idiv // mod %
            int token_num = ctx->factor().size() - 1;
            int star_rec = 0, div_rec = 0, idiv_rec = 0, mod_rec = 0;
            int star_size = ctx->STAR().size();
            int div_size = ctx->DIV().size();
            int idiv_size = ctx->IDIV().size();
            int mod_size = ctx->MOD().size();
            int star_index = 1e9;
            int div_index = 1e9;
            int idiv_index = 1e9;
            int mod_index = 1e9;
            int map_size = glb_map.size();
            antlrcpp::Any ans = visit(ctx->factor(0));
            if (ans.is<std::string>())
                Is_Val_Name(ans);
            for (int i = 1; i < token_num + 1; i++)
            {
                antlrcpp::Any tmp = visit(ctx->factor(i));
                if (tmp.is<std::string>())
                    Is_Val_Name(tmp);
                if (star_size > 0 && star_rec != star_size)
                {
                    star_index = ctx->STAR(star_rec)->getSymbol()->getTokenIndex();
                }
                else
                {
                    star_index = 1e9;
                }
                if (div_size > 0 && div_rec != div_size)
                {
                    div_index = ctx->DIV(div_rec)->getSymbol()->getTokenIndex();
                }
                else
                {
                    div_index = 1e9;
                }
                if (idiv_size > 0 && idiv_size != idiv_rec)
                {
                    idiv_index = ctx->IDIV(idiv_rec)->getSymbol()->getTokenIndex();
                }
                else
                {
                    idiv_index = 1e9;
                }
                if (mod_size > 0 && mod_size != mod_rec)
                {
                    mod_index = ctx->MOD(mod_rec)->getSymbol()->getTokenIndex();
                }
                else
                {
                    mod_index = 1e9;
                }

                int tmp_index = min(min(star_index, div_index), min(idiv_index, mod_index));

                if (tmp_index == star_index)
                {
                    if (ans.is<Bigint>() && tmp.is<Bigint>())
                    {
                        ans = ans.as<Bigint>() * tmp.as<Bigint>();
                    }
                    else if (ans.is<Bigint>() && tmp.is<double>())
                    {
                        ans = double(ans.as<Bigint>()) * tmp.as<double>();
                    }
                    else if ((ans.is<double>() && tmp.is<Bigint>()))
                    {
                        ans = ans.as<double>() * double(tmp.as<Bigint>());
                    }
                    else if (ans.is<double>() && tmp.is<double>())
                    {
                        ans = ans.as<double>() * tmp.as<double>();
                    }
                    else if (ans.is<Bigint>() && tmp.is<std::string>())
                    {
                        if (ans.as<Bigint>() == (Bigint)0 || ans.as<Bigint>() < (Bigint)0)
                        {
                            std::string s1 = "\"\"";
                            ans = s1;
                            continue;
                        }
                        std::string str = tmp.as<std::string>();
                        std::string s1 = str.substr(0, str.size() - 1);
                        str = str.substr(1, str.size() - 2);
                        double cnt = double(ans.as<Bigint>());
                        for (double i = 1; i < cnt; i++)
                        {
                            s1 = s1 + str;
                        }
                        s1 += "\"";
                        ans = s1;
                    }
                    else if (ans.is<std::string>() && tmp.is<Bigint>())
                    {
                        if (tmp.as<Bigint>() == Bigint(0) || tmp.as<Bigint>() < (Bigint)0)
                        {
                            std::string s1 = "\"\"";
                            ans = s1;
                            continue;
                        }
                        std::string str = ans.as<std::string>();
                        std::string s1 = str.substr(0, str.size() - 1);
                        str = str.substr(1, str.size() - 2);
                        double cnt = double(tmp.as<Bigint>()); //可能会出事
                        //  for (Bigint i(0); i < tmp.as<Bigint>(); i++)  //WHY？？
                        for (double i = 1; i < cnt; i++)
                        {
                            s1 = s1 + str;
                        }
                        s1 += "\"";
                        ans = s1;
                    }
                    else if (ans.is<bool>() && tmp.is<bool>())
                    {
                        ans = Bigint(ans.as<bool>()) * Bigint(tmp.as<bool>());
                    }
                    else if (ans.is<bool>() && tmp.is<Bigint>())
                    {
                        ans = Bigint(ans.as<bool>()) * tmp.as<Bigint>();
                    }
                    else if (ans.is<Bigint>() && tmp.is<bool>())
                    {
                        ans = ans.as<Bigint>() * Bigint(tmp.as<bool>());
                    }
                    else if (ans.is<bool>() && tmp.is<std::string>())
                    {
                        if (ans.as<bool>())
                            ans = tmp.as<std::string>();
                        else
                        {
                            std::string s1 = "\"\"";
                            ans = s1;
                        }
                    }
                    else if (ans.is<std::string>() && tmp.is<bool>())
                    {
                        if (!tmp.as<bool>())
                        {
                            std::string s1 = "\"\"";
                            ans = s1;
                        }
                    }
                    else if (ans.is<bool>() && tmp.is<double>())
                    {
                        ans = double(ans.as<bool>()) * tmp.as<double>();
                    }
                    else if (ans.is<double>() && tmp.is<bool>())
                    {
                        ans = double(tmp.as<bool>()) * ans.as<double>();
                    }
                    else
                    {
                        std::cerr << "Error operation type!\n ";
                        exit(0);
                    }
                    star_rec += 1;
                }
                //   /
                else if (div_index == tmp_index)
                {
                    if (ans.is<Bigint>() && tmp.is<Bigint>())
                    {
                        if (tmp.as<Bigint>() == Bigint(0))
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = double(ans.as<Bigint>()) / double(tmp.as<Bigint>());
                    }
                    else if (ans.is<bool>() && tmp.is<bool>())
                    {
                        if (tmp.as<bool>() == false)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = double(ans.as<bool>()) / double(tmp.as<bool>());
                    }
                    else if (ans.is<bool>() && tmp.is<Bigint>())
                    {
                        if (tmp.as<Bigint>() == (Bigint)0)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = (double)ans.as<bool>() / (double)tmp.as<Bigint>();
                    }
                    else if (ans.is<Bigint>() && tmp.is<bool>())
                    {
                        if (tmp.as<bool>() == false)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = double(ans.as<Bigint>()) / double((tmp.as<bool>()));
                    }
                    else if (ans.is<Bigint>() && tmp.is<double>())
                    {
                        if (tmp.as<double>() == 0)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = double(ans.as<Bigint>()) / tmp.as<double>();
                    }
                    else if (ans.is<double>() && tmp.is<double>())
                    {
                        if (tmp.as<double>() == 0)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = ans.as<double>() / tmp.as<double>();
                    }
                    else if (ans.is<double>() && tmp.is<Bigint>())
                    {
                        if (tmp.as<Bigint>() == (Bigint)0)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = ans.as<double>() / (double)tmp.as<Bigint>();
                    }
                    else if (ans.is<double>() && tmp.is<bool>())
                    {
                        if (tmp.as<bool>() == false)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = ans.as<double>() / (double)tmp.as<bool>();
                    }
                    else if (ans.is<bool>() && tmp.as<double>())
                    {
                        if (tmp.as<double>() == 0)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = (double)ans.as<bool>() / tmp.as<double>();
                    }
                    else
                    {
                        std::cerr << "Error operation type!\n ";
                        exit(0);
                    }
                    div_rec += 1;
                }
                //  //
                else if (idiv_index == tmp_index)
                {
                    if (ans.is<Bigint>() && tmp.is<Bigint>())
                    {
                        double x = 1;
                        if (tmp.as<Bigint>() == (Bigint)0)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        antlrcpp::Any tmp_ans;
                        antlrcpp::Any judge_ans;
                        judge_ans = double(ans.as<Bigint>()) / double(tmp.as<Bigint>());
                        tmp_ans = ans.as<Bigint>() / tmp.as<Bigint>();
                        if ((ans.as<Bigint>() >= Bigint(0) && tmp.as<Bigint>() >= (Bigint)0) || (ans.as<Bigint>() < Bigint(0) && tmp.as<Bigint>() < (Bigint)0) || ((double)tmp_ans.as<Bigint>() == judge_ans.as<double>()))
                        {
                            ans = tmp_ans.as<Bigint>();
                        }
                        else
                        {
                            ans = tmp_ans.as<Bigint>() - Bigint(x);
                        }
                    }
                    else if (ans.is<bool>() && tmp.is<bool>())
                    {
                        double x = 1;
                        if (tmp.as<bool>() == false)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = (Bigint)ans.as<bool>() / (Bigint)tmp.as<bool>();
                    }
                    else if (ans.is<bool>() && tmp.is<Bigint>())
                    {
                        double x = 1;
                        if (tmp.as<Bigint>() == (Bigint)0)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = (Bigint)ans.as<bool>() / tmp.as<Bigint>();
                        if (tmp.as<Bigint>() < Bigint(0))
                            ans = Bigint(ans.as<bool>()) - Bigint(x);
                    }
                    else if (ans.is<Bigint>() && tmp.is<bool>())
                    {
                        double x = 1;
                        if (tmp.as<bool>() == false)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = ans.as<Bigint>() / (Bigint)tmp.as<bool>();
                    }
                    else if (ans.is<double>() || tmp.is<double>())
                    {
                        std::cerr << "Undefined behavior\n ";
                        exit(0);
                    }
                    else
                    {
                        std::cerr << "Error operation type!\n ";
                        exit(0);
                    }
                    idiv_rec += 1;
                }

                else if (mod_index == tmp_index)
                {
                    if (ans.is<Bigint>() && tmp.is<Bigint>())
                    {
                        if (tmp.as<Bigint>() == Bigint(0))
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = ans.as<Bigint>() % tmp.as<Bigint>();
                    }
                    else if (ans.is<bool>() && tmp.is<bool>())
                    {
                        if (tmp.as<bool>() == false)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = Bigint((ans.as<bool>())) % Bigint((tmp.as<bool>()));
                    }
                    else if (ans.is<bool>() && tmp.is<Bigint>())
                    {
                        if (tmp.as<Bigint>() == Bigint(0))
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = Bigint((ans.as<bool>())) % tmp.as<Bigint>();
                    }
                    else if (ans.is<Bigint>() && tmp.is<bool>())
                    {
                        if (tmp.as<bool>() == false)
                        {
                            std::cerr << "Error : The divisor cannot be zero!\n";
                            exit(0);
                        }
                        ans = ans.as<Bigint>() % Bigint((tmp.as<bool>()));
                    }
                    else if (ans.is<double>() || tmp.is<double>())
                    {
                        std::cerr << "Undefined behavior\n ";
                        exit(0);
                    }
                    else
                    {
                        std::cerr << "Error operation type!\n ";
                        exit(0);
                    }
                    mod_rec += 1;
                }
            }
            return ans;
        }
    }

    antlrcpp::Any visitFactor(Python3Parser::FactorContext *ctx) override
    {
        if (ctx->factor() != nullptr)
        {
            antlrcpp::Any ans = visit(ctx->factor());
            // 判断ans 是否为 变量名
            int map_size = glb_map.size();
            if (ans.is<std::string>())
                Is_Val_Name(ans);
            if (ctx->ADD() != nullptr)
            {
                if (ans.is<std::string>())
                {
                    std::cerr << "Invalid operator !\n";
                    exit(0);
                }
                return ans;
            }
            else
            {
                if (ans.is<Bigint>())
                {
                    Bigint x(-1.0);
                    ans = ans.as<Bigint>() * x;
                }
                else if (ans.is<double>())
                {
                    ans = ans.as<double>() * -1;
                }
                else if (ans.is<bool>())
                {
                    ans = (bool)(ans.as<bool>() * -1);
                }
                else if (ans.is<std::string>())
                {
                    std::cerr << "Invalid operator !\n";
                    exit(0);
                }
                return ans;
            }
        }
        else
            return visitAtom_expr(ctx->atom_expr());
    }

    antlrcpp::Any visitAtom_expr(Python3Parser::Atom_exprContext *ctx) override
    {
        if (ctx->trailer() == nullptr)
            return visitAtom(ctx->atom());
        std::string ret = ctx->atom()->NAME()->toString();
        if (ret == "print")
        {
            if(ctx -> trailer() -> arglist() == nullptr)
            {
                std::cout << '\n';
                return nullptr;
            }
            else
            {
                int argument_size = ctx->trailer()->arglist()->argument().size();
                for (int i = 0; i < argument_size; i++)
                {
                    if (i)
                        std::cout << " ";
                    antlrcpp::Any tmp = visit(ctx->trailer()->arglist()->argument(i)->test());
                    if (tmp.is<std::string>())
                        Is_Val_Name(tmp);
                    if (tmp.is<std::string>())
                    {
                        std::string ans = tmp.as<std::string>();
                        if (ans == "None")
                            std::cout << "None";
                        else 
                        {
                            int len = ans.size();
                            ans = ans.substr(1, len - 2);
                            std::cout << ans;
                        }
                    }
                    else if (tmp.is<double>())
                    {
                        std::cout << setiosflags(ios::fixed | ios::showpoint) << setprecision(6) << tmp.as<double>();
                    }
                    else if (tmp.is<Bigint>())
                    {
                        std::cout << tmp.as<Bigint>();
                    }
                    else if (tmp.is<bool>())
                    {
                        bool x = tmp.as<bool>();
                        if (x)
                            std::cout << "True";
                        else
                            std::cout << "False";
                    }
                }
                std::cout << std::endl;
                return nullptr;
            }
        }
        else if (ret == "int")
        {
            antlrcpp::Any r_val = visit(ctx->trailer()->arglist()->argument(0)->test());
            int map_size = glb_map.size();
            if (r_val.is<std::string>())
                Is_Val_Name(r_val);
            if (r_val.is<Bigint>())
            {
                return r_val;
            }
            else if (r_val.is<double>())
            {
                return (Bigint)r_val.as<double>();
            }
            else if (r_val.is<bool>())
            {
                return (Bigint)r_val.as<bool>();
            }
            else if (r_val.is<std::string>())
            {
                std::string s1 = r_val.as<std::string>();
                s1 = s1.substr(1, s1.size() - 2);
                return (Bigint)s1;
            }
            else
            {
                std::cerr << "Invalid type conversion !\n";
                exit(0);
            }
        }
        else if (ret == "float")
        {
            antlrcpp::Any r_val = visit(ctx->trailer()->arglist()->argument(0)->test());
            int map_size = glb_map.size();
            if (r_val.is<std::string>())
                Is_Val_Name(r_val);
            if (r_val.is<Bigint>())
            {
                return (double)r_val.as<Bigint>();
            }
            else if (r_val.is<double>())
            {
                return r_val;
            }
            else if (r_val.is<bool>())
            {
                return (double)r_val.as<bool>();
            }
            else if (r_val.is<std::string>())
            {
                char c[1000];
                std::string s1 = r_val.as<std::string>();
                s1 = s1.substr(1, s1.size() - 2);
                strcpy(c, s1.c_str());
                double x = atof(c);
                return x;
            }
            else
            {
                std::cerr << "Invalid type conversion !\n";
                exit(0);
            }
        }
        else if (ret == "bool")
        {
            antlrcpp::Any r_val = visit(ctx->trailer()->arglist()->argument(0)->test());
            int map_size = glb_map.size();
            if (r_val.is<std::string>())
                Is_Val_Name(r_val);
            if (r_val.is<Bigint>())
            {
                return (r_val.as<Bigint>() == Bigint(0)) ? false : true;
            }
            else if (r_val.is<double>())
            {
                return (r_val.as<double>() == 0) ? false : true;
            }
            else if (r_val.is<bool>())
            {
                return r_val.as<bool>();
            }
            else if (r_val.is<std::string>())
            {
                std::string ans = r_val.as<std::string>();
                ans = ans.substr(1, ans.size() - 2);
                return (ans.empty()) ? false : true;
            }
            else
            {
                std::cerr << "Invalid type conversion !\n";
                exit(0);
            }
        }
        else if (ret == "str")
        {
            antlrcpp::Any r_val = visit(ctx->trailer()->arglist()->argument(0)->test());
            int map_size = glb_map.size();
            if (r_val.is<std::string>())
                Is_Val_Name(r_val);
            if (r_val.is<Bigint>())
            {
                std::string tmp_str = (std::string)r_val.as<Bigint>();
                std::string ans = "\"";
                ans += tmp_str;
                ans += "\"";
                return ans;
            }
            else if (r_val.is<double>())
            {
                std::string tmp_str = std::to_string(r_val.as<double>());
                std::string ans = "\"";
                ans += tmp_str;
                ans += "\"";
                return ans;
            }
            else if (r_val.is<bool>())
            {
                if (r_val.as<bool>() == true)
                {
                    std::string ans = "\"True\"";
                    return ans;
                }
                else
                {
                    std::string ans = "\"False\"";
                    return ans;
                }
            }
            else if (r_val.is<std::string>())
            {
                return r_val.as<std::string>();
            }
            else
            {
                std::cerr << "Invalid type conversion !\n";
                exit(0);
            }
        }
        else
        {
            Python3Parser::FuncdefContext *func_node = func_map[ret];
            std::map<std::string, antlrcpp::Any> inner_map;
            glb_map.push_back(inner_map);
            if (ctx->trailer()->arglist() != nullptr)
            {
                Python3Parser::TypedargslistContext *Typedargslist_node = func_node->parameters()->typedargslist();
                int incoming_val_num = ctx->trailer()->arglist()->argument().size(); // 传入的变量数量
                int contain_null_val_num = Typedargslist_node->tfpdef().size();      //所有的变量数量
                // 总共有 argument_num 个参数
                int map_size = glb_map.size();
                std::vector<antlrcpp::Any> rec;
                for (int i = 0; i < incoming_val_num; i++)
                {
                    antlrcpp::Any val = visit(ctx->trailer()->arglist()->argument(i)->test());
                    if (val.is<std::string>())
                    {
                        string s1 = val.as<std::string>();
                        if (s1.find("\"") == std::string::npos)
                        {
                            for (int i = map_size - 2; i >= 0; i--)
                            {
                                if (Find_map_key(glb_map[i], val.as<std::string>()))
                                {
                                    val = glb_map[i][val.as<std::string>()];
                                    break;
                                }
                            }
                        }
                    }
                    rec.push_back(val);
                }
                for (int i = 0; i < incoming_val_num; i++)
                {
                    if (ctx->trailer()->arglist()->argument(i)->ASSIGN() == nullptr)
                    {
                        // postion 传参
                        std::string val_name = Typedargslist_node->tfpdef(i)->NAME()->toString();
                        antlrcpp::Any val = rec[i];
                        if (val.is<std::string>())
                            glb_map[map_size - 1][val_name] = val.as<std::string>();
                        else if (val.is<Bigint>())
                            glb_map[map_size - 1][val_name] = val.as<Bigint>();
                        else if (val.is<double>())
                            glb_map[map_size - 1][val_name] = val.as<double>();
                        else if (val.is<bool>())
                            glb_map[map_size - 1][val_name] = val.as<bool>();
                    }
                    else // keyword 传参
                    {
                        std::string val_name = ctx->trailer()->arglist()->argument(i)->NAME()->toString();
                        antlrcpp::Any val = rec[i];
                        if (val.is<std::string>())
                        {
                            glb_map[map_size - 1][val_name] = val.as<std::string>();
                        }
                        else if (val.is<Bigint>())
                            glb_map[map_size - 1][val_name] = val.as<Bigint>();
                        else if (val.is<double>())
                            glb_map[map_size - 1][val_name] = val.as<double>();
                        else if (val.is<bool>())
                            glb_map[map_size - 1][val_name] = val.as<bool>();
                    }
                }
                for (int i = 0, j = 0; i < contain_null_val_num; i++)
                {
                    std::string val_name = Typedargslist_node->tfpdef(i)->NAME()->toString();
                    if (!Find_map_key(glb_map[map_size - 1], val_name))
                    {
                        glb_map[map_size - 1][val_name] = visit(Typedargslist_node->test(j));
                        j++;
                    }
                }
            }
            // 无参，或不传参的情况
            else
            {
                if (func_node->parameters()->typedargslist() != nullptr)
                {
                    Python3Parser::TypedargslistContext *Typedargslist_node = func_node->parameters()->typedargslist();
                    int map_size = glb_map.size();
                    for (int i = 0; i < Typedargslist_node->tfpdef().size(); i++)
                    {
                        std::string val_name = Typedargslist_node->tfpdef(i)->NAME()->toString();
                        glb_map[map_size - 1][val_name] = visit(Typedargslist_node->test(i));
                    }
                }
            }
            antlrcpp::Any tmp = visitSuite(func_node->suite());
            if (!tmp.is<RETURN_STMT>())
                return nullptr; //     函数中不写return 的情况
            else if (tmp.as<RETURN_STMT>().ret.is<std::vector<antlrcpp::Any>>())
            {
                std::vector<antlrcpp::Any> ans = tmp.as<RETURN_STMT>().ret;
                if (ans.empty())
                    tmp = nullptr;
                else
                {
                    for (int j = 0; j < ans.size(); j++)
                    {
                        antlrcpp::Any val = ans[j];
                        if (val.is<std::string>())
                            Is_Val_Name(val);
                        ans[j] = val;
                    }
                    tmp = ans;
                }
            }
            else
            {
                antlrcpp::Any x = tmp.as<RETURN_STMT>().ret;
                if (x.is<std::string>())
                    Is_Val_Name(x);
                tmp = x;
            }
            glb_map.pop_back();
            return tmp;
        }
    }

    antlrcpp::Any visitTrailer(Python3Parser::TrailerContext *ctx) override
    {
        return visitChildren(ctx);
    }

    antlrcpp::Any visitAtom(Python3Parser::AtomContext *ctx) override
    {
        if (ctx->NAME() != nullptr)
        {
            std::string str = ctx->NAME()->toString();
            return str;
        }
        else if (ctx->STRING().size() != 0)
        {
            std::string tmpStr;
            tmpStr = ctx->STRING(0)->toString();
            tmpStr = tmpStr.substr(0, tmpStr.size() - 1);
            for (int i = 1; i < ctx->STRING().size(); i++)
            {
                std::string str = ctx->STRING(i)->toString();
                int len = str.size();
                str = str.substr(1, len - 2);
                tmpStr += str;
            }
            tmpStr += "\"";
            return tmpStr;
        }
        else if (ctx->NUMBER() != nullptr)
        {
            char c[5000];
            std::string s1 = ctx->NUMBER()->toString();
            strcpy(c, s1.c_str());
            if (s1.find(".") == std::string::npos)
            {
                Bigint x(s1);
                antlrcpp::Any tmp = x;
                return tmp;
            }
            else
            {
                double x = atof(c);
                return x;
            }
        }
        else if (ctx->TRUE() != nullptr)
        {
            bool flag = true;
            return flag;
        }
        else if (ctx->FALSE() != nullptr)
        {
            bool flag = false;
            return flag;
        }
        else if (ctx->NONE() != nullptr)
        {
            std::string str = "None";
            return str;
        }
        else if (ctx->test() != nullptr)
        {
            return visitTest(ctx->test());
        }
    }

    antlrcpp::Any visitTestlist(Python3Parser::TestlistContext *ctx) override
    {
        if (ctx->test().size() == 1)
            return visitTest(ctx->test(0));
        else
        {
            std::vector<antlrcpp::Any> ret;
            for (int i = 0; i < ctx->test().size(); i++)
            {
                antlrcpp::Any ans = visit(ctx->test(i));
                if(ans.is<std::string>())
                    Is_Val_Name(ans);
                ret.push_back(ans);
            }
            return ret;
        }
    }

    antlrcpp::Any visitArglist(Python3Parser::ArglistContext *ctx) override
    {
        return visitArgument(ctx->argument(0));
    }

    antlrcpp::Any visitArgument(Python3Parser::ArgumentContext *ctx) override
    {
        return visitTest(ctx->test());
    }

    //todo:override all methods of Python3BaseVisitor
};

#endif //PYTHON_INTERPRETER_EVALVISITOR_H