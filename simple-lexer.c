#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//TokenType 用于说明当前的token是什么类型
enum TokenType
{
    TokenType_Plus,  // +
    TokenType_Minus, // -
    TokenType_Star,  // *
    TokenType_Slash, // /

    TokenType_GE, // >=
    TokenType_GT, // >
    TokenType_EQ, // ==
    TokenType_LE, // <=
    TokenType_LT, // <

    TokenType_SemiColon,  // ;
    TokenType_LeftParen,  // (
    TokenType_RightParen, // )

    TokenType_Assignment, // =

    TokenType_If,
    TokenType_Else,

    TokenType_Int,

    TokenType_Identifier, //标识符

    TokenType_IntLiteral,   //整型字面量
    TokenType_StringLiteral //字符串字面量
};

char *TokenTypeString[] = {"Plus", "Minus", "Star", "Slash", "GE", "GT", "EQ", "LE", "LT",
                           "SemiColon", "LeftParen", "RightParen", "Assignment", "If", "Else",
                           "Int", "Identifier", "IntLiteral", "StringLiteral"};

//Token结构体，包含类型和文本值
typedef struct
{
    enum TokenType token_type;
    char *text;
} Token;

//DFA状态，用于表示解析token过程中当前所处的状态
enum DFAState
{
    DFAState_Initial,

    DFAState_If,
    DFAState_Id_if1,
    DFAState_Id_if2,
    DFAState_Else,
    DFAState_Id_else1,
    DFAState_Id_else2,
    DFAState_Id_else3,
    DFAState_Id_else4,
    DFAState_Int,
    DFAState_Id_int1,
    DFAState_Id_int2,
    DFAState_Id_int3,
    DFAState_Id,
    DFAState_GT,
    DFAState_GE,

    DFAState_Assignment,

    DFAState_Plus,
    DFAState_Minus,
    DFAState_Star,
    DFAState_Slash,

    DFAState_SemiColon,
    DFAState_LeftParen,
    DFAState_RightParen,

    DFAState_IntLiteral
};

///////////////////////////////
//函数声明

// peek从标准输入流中查看下一个字符，不会引起下一个字符被读取
char peek();
// readToken从标准输入流中读取一个token
// 如果成功，返回0，如果读取错误，返回EOF
int readToken(Token *token);
//使用读取的字符来初始化token, 返回状态
enum DFAState initToken(Token *token, char ch);
//填充token文本，并初始化临时字符串缓存
void fillTokenText(Token *token);
// 解析并打印所有token
void dump();

//函数声明结束
///////////////////////////////////////

char *tmp_token_text;
int tmp_token_pos;
const int tmp_size = 1024;

int main(int argc, char *argv[])
{

    tmp_token_text = (char *)malloc(tmp_size * sizeof(char));
    tmp_token_pos = 0;

    dump();
}

char peek()
{
    char c = getc(stdin);
    if (c != EOF)
        return ungetc(c, stdin);
    return EOF;
}

int readToken(Token *token)
{

    char ch = getc(stdin);
    if (ch == EOF)
        return EOF;
    enum DFAState state = initToken(token, ch);

    //第一个字符已经吃入，状态也已经初始化好，根据第一个字符得到的状态，从第二个字符开始检测
    while ((ch = getc(stdin)) != EOF)
    {
        switch (state)
        {
        case DFAState_Initial:
            if (tmp_token_pos > 0)
            {
                //将临时保存的数据存储到token中，然后初始化pos并返回
                fillTokenText(token);
                return 0;
            }
            //忽略其他非法字符, 直接使用第二个字符重新初始化
            state = initToken(token, ch);
            break;
        case DFAState_Id:
            if (isalpha(ch) || isdigit(ch))
            {
                //后续字符正常，吃入，继续循环
                tmp_token_text[tmp_token_pos++] = ch;
            }
            else
            {
                //后续字符不正常（可能属于下个token），吐出，结束当前循环
                ungetc(ch, stdin);
                fillTokenText(token);
                return 0;
            }
            break;
        case DFAState_GT:
            if (ch == '=')
            {
                //吃入，同时转换状态
                token->token_type = TokenType_GE; //转换成GE
                state = DFAState_GE;
                tmp_token_text[tmp_token_pos++] = ch;
            }
            else
            {
                //吐出并返回
                ungetc(ch, stdin);
                fillTokenText(token);
                return 0;
            }
            break;
        case DFAState_GE:
        case DFAState_Assignment:
        case DFAState_Plus:
        case DFAState_Minus:
        case DFAState_Star:
        case DFAState_Slash:
        case DFAState_SemiColon:
        case DFAState_LeftParen:
        case DFAState_RightParen:
            //前方状态已经是结束状态，吃入的字符没有意义，吐出然后返回
            ungetc(ch, stdin);
            fillTokenText(token);
            return 0;
        case DFAState_IntLiteral:
            if (isdigit(ch))
            {
                //后续字符正常，吃入，继续循环
                tmp_token_text[tmp_token_pos++] = ch;
            }
            else
            {
                //吐出并返回
                ungetc(ch, stdin);
                fillTokenText(token);
                return 0;
            }
            break;
        case DFAState_Id_int1:
            if (ch == 'n')
            {
                //吃入，改变状态，继续循环
                state = DFAState_Id_int2;
                tmp_token_text[tmp_token_pos++] = ch;
            }
            else if (isdigit(ch) || isalpha(ch))
            {
                //吃入，改变状态，继续循环
                state = DFAState_Id; //切换回Id状态
                tmp_token_text[tmp_token_pos++] = ch;
            }
            else
            {
                //吐出，返回
                ungetc(ch, stdin);
                fillTokenText(token);
                return 0;
            }
            break;
        case DFAState_Id_int2:
            if (ch == 't')
            {
                //吃入，改变状态，继续循环
                state = DFAState_Id_int3;
                tmp_token_text[tmp_token_pos++] = ch;
            }
            else if (isdigit(ch) || isalpha(ch))
            {
                //吃入，改变状态，继续循环
                state = DFAState_Id; //切换回id状态
                tmp_token_text[tmp_token_pos++] = ch;
            }
            else
            {
                //吐出，返回
                ungetc(ch, stdin);
                fillTokenText(token);
                return 0;
            }
            break;
        case DFAState_Id_int3:
            if (isblank(ch))
            {
                //在int状态结束后同时跟随一个空白字符，此时int类型确定，直接返回
                //由于空白字符下个token也不会处理，吐出没有意义，所以直接返回
                token->token_type = TokenType_Int;
                fillTokenText(token);
                return 0;
            }
            else if (isprint(ch))
            {
                //当前切换为id，吃入，继续循环直到id状态结束
                state = DFAState_Id; //切换回Id状态
                tmp_token_text[tmp_token_pos++] = ch;
            }
            else
            {
                //吐出，返回
                ungetc(ch, stdin);
                fillTokenText(token);
                return 0;
            }
            break;
        default:
            //忽略其他非法状态
            return EOF;
        }
    }

    //到达输入结尾，返回最后一个token
    if (tmp_token_pos > 0)
    {
        fillTokenText(token);
        return 0;
    }
    return EOF;
}

enum DFAState initToken(Token *token, char ch)
{
    enum DFAState state = DFAState_Initial;
    if (isalpha(ch))
    {
        if (ch == 'i')
        {
            state = DFAState_Id_int1;
        }
        else
        {
            state = DFAState_Id; //进入Id状态
        }
        token->token_type = TokenType_Identifier;
        tmp_token_text[tmp_token_pos++] = ch;
    }
    else if (isdigit(ch))
    {
        state = DFAState_IntLiteral;
        token->token_type = TokenType_IntLiteral;
        tmp_token_text[tmp_token_pos++] = ch;
    }
    else if (ch == '>')
    {
        state = DFAState_GT;
        token->token_type = TokenType_GT;
        tmp_token_text[tmp_token_pos++] = ch;
    }
    else if (ch == '+')
    {
        state = DFAState_Plus;
        token->token_type = TokenType_Plus;
        tmp_token_text[tmp_token_pos++] = ch;
    }
    else if (ch == '-')
    {
        state = DFAState_Minus;
        token->token_type = TokenType_Minus;
        tmp_token_text[tmp_token_pos++] = ch;
    }
    else if (ch == '*')
    {
        state = DFAState_Star;
        token->token_type = TokenType_Star;
        tmp_token_text[tmp_token_pos++] = ch;
    }
    else if (ch == '/')
    {
        state = DFAState_Slash;
        token->token_type = TokenType_Slash;
        tmp_token_text[tmp_token_pos++] = ch;
    }
    else if (ch == ';')
    {
        state = DFAState_SemiColon;
        token->token_type = TokenType_SemiColon;
        tmp_token_text[tmp_token_pos++] = ch;
    }
    else if (ch == '(')
    {
        state = DFAState_LeftParen;
        token->token_type = TokenType_LeftParen;
        tmp_token_text[tmp_token_pos++] = ch;
    }
    else if (ch == ')')
    {
        state = DFAState_RightParen;
        token->token_type = TokenType_RightParen;
        tmp_token_text[tmp_token_pos++] = ch;
    }
    else if (ch == '=')
    {
        state = DFAState_Assignment;
        token->token_type = TokenType_Assignment;
        tmp_token_text[tmp_token_pos++] = ch;
    }
    else
    {
        state = DFAState_Initial; // skip all unknown patterns
    }
    return state;
}

void fillTokenText(Token *token)
{
    token->text = (char *)malloc(tmp_token_pos * sizeof(char) + 1);
    strncpy(token->text, tmp_token_text, tmp_token_pos);
    token->text[tmp_token_pos] = '\0';
    tmp_token_pos = 0;
}

void dump()
{
    struct Token *token;
    while (1)
    {
        Token token;
        if (readToken(&token) == EOF)
            return;
        printf("%s\t\t%s\n", token.text, TokenTypeString[token.token_type]);
    }
}