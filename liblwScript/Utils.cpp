#include "Utils.h"
#include "Logger.h"
#include <locale>
#include <codecvt>
#include <fstream>
#include <sstream>
namespace lwscript
{
    STD_STRING ReadFile(std::string_view path)
    {
#ifdef UTF8_ENCODE
        auto utf8Path = Utf8::Decode(path.data());
        Logger::Record::mCurFilePath = utf8Path;
#else
        Logger::Record::mCurFilePath = path;
#endif

        STD_IFSTREAM file;
        file.open(Logger::Record::mCurFilePath.c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open())
            Logger::Error(TEXT("Failed to open file:{}"), Logger::Record::mCurFilePath);

        STD_STRING_STREAM sstream;
        sstream << file.rdbuf();
        file.close();
        return sstream.str();
    }

    void WriteBinaryFile(std::string_view path, const std::vector<uint8_t> &content)
    {
        std::ofstream file;
        file.open(path.data(), std::ios::out | std::ios::binary);
        if (!file.is_open())
#ifdef UTF8_ENCODE
        {
            auto utf8Path = Utf8::Decode(path.data());
            Logger::Error(TEXT("Failed to open file:{}"), utf8Path);
        }
#else
            Logger::Error(TEXT("Failed to open file:{}"), path);
#endif
        file.write((const char *)content.data(), content.size());
        file.close();
    }

    std::vector<uint8_t> LWSCRIPT_API ReadBinaryFile(std::string_view path)
    {
        std::ifstream file;
        file.open(path.data(), std::ios::in | std::ios::binary);
        if (!file.is_open())
#ifdef UTF8_ENCODE
        {
            auto utf8Path = Utf8::Decode(path.data());
            Logger::Error(TEXT("Failed to open file:{}"), utf8Path);
        }
#else
            Logger::Error(TEXT("Failed to open file:{}"), path);
#endif

        std::stringstream sstream;
        sstream << file.rdbuf();
        file.close();
        auto str = sstream.str();

        std::vector<uint8_t> result;
        result.assign(str.begin(), str.end());

        return result;
    }

    STD_STRING PointerAddressToString(void *pointer)
    {
        STD_STRING_STREAM sstr;
        sstr << pointer;
        STD_STRING address = sstr.str();
        return address;
    }

    int64_t Factorial(int64_t v, int64_t tmp)
    {
        if (v == 0)
            return tmp;
        return Factorial(v - 1, v * tmp);
    }

    uint64_t NormalizeIdx(int64_t idx, size_t dsSize)
    {
        bool isNeg = false;
        if (idx < 0)
        {
            isNeg = true;
            idx = abs(idx);
        }

        idx = idx % (int64_t)dsSize;

        if (isNeg)
            idx = dsSize - idx;

        return idx;
    }

    namespace Utf8
    {

        std::string Encode(const std::wstring &str)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            return converter.to_bytes(str);
        }

        std::wstring Decode(const std::string &str)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            return converter.from_bytes(str);
        }
    }
    namespace ByteConverter
    {
        std::array<uint8_t, 8> ToU64ByteList(int64_t integer)
        {
            std::array<uint8_t, 8> result{};
            result[0] = (uint8_t((integer & 0xFF00000000000000) >> 56));
            result[1] = (uint8_t((integer & 0x00FF000000000000) >> 48));
            result[2] = (uint8_t((integer & 0x0000FF0000000000) >> 40));
            result[3] = (uint8_t((integer & 0x000000FF00000000) >> 32));
            result[4] = (uint8_t((integer & 0x00000000FF000000) >> 24));
            result[5] = (uint8_t((integer & 0x0000000000FF0000) >> 16));
            result[6] = (uint8_t((integer & 0x000000000000FF00) >> 8));
            result[7] = (uint8_t((integer & 0x00000000000000FF) >> 0));
            return result;
        }

        uint64_t GetU64Integer(const std::vector<uint8_t> data, uint32_t start)
        {
            uint64_t v{0};
            for (int32_t i = 0; i < 8; ++i)
                v |= ((uint64_t)(data[start + i] & 0x00000000000000FF) << ((7 - i) * 8));
            return v;
        }

        std::array<uint8_t, 4> ToU32ByteList(int32_t integer)
        {
            std::array<uint8_t, 4> result{};
            result[0] = (uint8_t((integer & 0xFF000000) >> 24));
            result[1] = (uint8_t((integer & 0x00FF0000) >> 16));
            result[2] = (uint8_t((integer & 0x0000FF00) >> 8));
            result[3] = (uint8_t((integer & 0x000000FF) >> 0));
            return result;
        }

        uint32_t GetU32Integer(const std::vector<uint8_t> data, uint32_t start)
        {
            uint32_t v{0};
            for (int32_t i = 0; i < 4; ++i)
                v |= ((uint32_t)(data[start + i] & 0x000000FF) << ((3 - i) * 8));
            return v;
        }
    }
}