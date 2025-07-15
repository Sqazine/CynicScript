#pragma once
#include <iostream>
#include <sstream>
#include <string_view>
#include <string>
#include <cassert>
#include <cstdarg>
#include "Token.h"
#include "Common.h"

namespace CynicScript
{
    namespace Logger
    {
        enum class Kind
        {
            INFO,
            WARN,
            ERROR
        };
        namespace Record
        {
            inline STRING mCurFilePath = TEXT("repl");
            inline STRING mSourceCode = TEXT("");
        }

        inline void Output(OSTREAM &os, STRING s)
        {
            os << s;
        }

        template <typename T, typename... Args>
        inline void Output(OSTREAM &os, STRING s, const T &next, const Args &...args)
        {
            auto index = s.find_first_of(TEXT("{}"));
            if (index == STRING::npos)
                Output(os, s);
            else
            {
                STRING_STREAM sstr;
                sstr << next;
                s.replace(index, 2, sstr.str());
                sstr.clear();
                Output(os, s, args...);
            }
        }

        template <typename... Args>
        inline void Println(const STRING &s, const Args &...args)
        {
            Output(COUT, s + TEXT("\n"), args...);
        }

        template <typename... Args>
        inline void Print(const STRING &s, const Args &...args)
        {
            Output(COUT, s, args...);
        }

        inline void RecordSource(STRING_VIEW sourceCode)
        {
            Record::mSourceCode = sourceCode;
        }

        template <typename... Args>
        inline void AssemblyLogInfo(const STRING &headerHint, const STRING &colorHint, uint64_t lineNum, uint64_t column, uint64_t pos, const STRING &fmt, const Args &...args)
        {
            auto start = pos;
            auto end = pos;

            if (Record::mCurFilePath != TEXT("interpreter"))
            {
                while ((Record::mSourceCode[start - 1] != TCHAR('\n') && Record::mSourceCode[start - 1] != TCHAR('\r')) && start - 1 > 0)
                    start--;

                while ((Record::mSourceCode[end] != TCHAR('\n') && Record::mSourceCode[end] != TCHAR('\r')) && end < Record::mSourceCode.size())
                    end++;
            }
            else
            {
                start = 0;
                end = Record::mSourceCode.size();
            }

            auto startStr = headerHint + TEXT(":") + Record::mCurFilePath + TEXT("(line ") + CYS_TO_STRING(lineNum) + TEXT(",column ") + CYS_TO_STRING(column) + TEXT("): ");

            auto lineSrcCode = Record::mSourceCode.substr(start, end - start);

            Println(TEXT("\033[{}m{}{}\033[0m"), colorHint, startStr, lineSrcCode);

            auto blankSize = startStr.size() + pos - start;

            STRING errorHintStr;
            errorHintStr.insert(0, blankSize, TCHAR(' '));
            errorHintStr += TEXT("^ ") + STRING(fmt);

            Println(TEXT("\033[{}m") + errorHintStr + TEXT("\033[0m"), colorHint, args...);
        }

        template <typename... Args>
        void Log(Kind logKind, const STRING &fmt, const Args &...args)
        {
            switch (logKind)
            {
            case Kind::ERROR:
                Println(TEXT("\033[31m[ERROR]:") + fmt + TEXT("\033[0m"), args...);
                break;
            case Kind::WARN:
                Println(TEXT("\033[33m[WARN]:") + fmt + TEXT("\033[0m"), args...);
                break;
            case Kind::INFO:
                Println(TEXT("\033[32m[INFO]:") + fmt + TEXT("\033[0m"), args...);
                break;
            default:
                break;
            }
        }

        template <typename... Args>
        void Log(Kind logKind, int32_t pos, const STRING &fmt, const Args &...args)
        {
            auto lineNum = 1;
            for (int32_t i = 0; i < pos; ++i)
                if (Record::mSourceCode[i] == TCHAR('\n') || Record::mSourceCode[i] == TCHAR('\r'))
                    lineNum++;

            switch (logKind)
            {
            case Kind::INFO:
                AssemblyLogInfo(TEXT("[INFO]"), TEXT("32"), lineNum, 1, pos, fmt, args...);
                break;
            case Kind::WARN:
                AssemblyLogInfo(TEXT("[WARN]"), TEXT("33"), lineNum, 1, pos, fmt, args...);
                break;
            case Kind::ERROR:
                AssemblyLogInfo(TEXT("[ERROR]"), TEXT("31"), lineNum, 1, pos, fmt, args...);
                break;
            default:
                break;
            }
        }

        template <typename... Args>
        void Log(Kind logKind, const Token *tok, const STRING &fmt, const Args &...args)
        {
            switch (logKind)
            {
            case Kind::INFO:
                AssemblyLogInfo(TEXT("[INFO]"), TEXT("32"), tok->sourceLocation.line, tok->sourceLocation.column, tok->sourceLocation.pos, fmt, args...);
                break;
            case Kind::WARN:
                AssemblyLogInfo(TEXT("[WARN]"), TEXT("33"), tok->sourceLocation.line, tok->sourceLocation.column, tok->sourceLocation.pos, fmt, args...);
                break;
            case Kind::ERROR:
                AssemblyLogInfo(TEXT("[ERROR]"), TEXT("31"), tok->sourceLocation.line, tok->sourceLocation.column, tok->sourceLocation.pos, fmt, args...);
                break;
            default:
                break;
            }
        }
    }
#ifndef NDEBUG
#define CYS_LOG_ERROR(fmt, ...)                                                         \
    do                                                                                  \
    {                                                                                   \
        CynicScript::Logger::Log(CynicScript::Logger::Kind::ERROR, fmt, ##__VA_ARGS__); \
        assert(0);                                                                      \
    } while (false)

#define CYS_LOG_ERROR_WITH_LOC(tokOrPos, fmt, ...)                                                \
    do                                                                                            \
    {                                                                                             \
        CynicScript::Logger::Log(CynicScript::Logger::Kind::ERROR, tokOrPos, fmt, ##__VA_ARGS__); \
        assert(0);                                                                                \
    } while (false)

#else

#define CYS_LOG_ERROR(fmt, ...)                                                         \
    do                                                                                  \
    {                                                                                   \
        CynicScript::Logger::Log(CynicScript::Logger::Kind::ERROR, fmt, ##__VA_ARGS__); \
        exit(1);                                                                        \
    } while (false)

#define CYS_LOG_ERROR_WITH_LOC(tokOrPos, fmt, ...)                                                \
    do                                                                                            \
    {                                                                                             \
        CynicScript::Logger::Log(CynicScript::Logger::Kind::ERROR, tokOrPos, fmt, ##__VA_ARGS__); \
        exit(1);                                                                                  \
    } while (false)
#endif

#define CYS_LOG_WARN(fmt, ...)                                                         \
    do                                                                                 \
    {                                                                                  \
        CynicScript::Logger::Log(CynicScript::Logger::Kind::WARN, fmt, ##__VA_ARGS__); \
    } while (false)

#define CYS_LOG_WARN_WITH_LOC(tokOrPos, fmt, ...)                                                \
    do                                                                                           \
    {                                                                                            \
        CynicScript::Logger::Log(CynicScript::Logger::Kind::WARN, tokOrPos, fmt, ##__VA_ARGS__); \
    } while (false)

#define CYS_LOG_INFO(fmt, ...)                                                         \
    do                                                                                 \
    {                                                                                  \
        CynicScript::Logger::Log(CynicScript::Logger::Kind::INFO, fmt, ##__VA_ARGS__); \
    } while (false)

#define CYS_LOG_INFO_WITH_LOC(tokOrPos, fmt, ...)                                                \
    do                                                                                           \
    {                                                                                            \
        CynicScript::Logger::Log(CynicScript::Logger::Kind::INFO, tokOrPos, fmt, ##__VA_ARGS__); \
    } while (false)

}