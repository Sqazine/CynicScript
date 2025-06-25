#pragma once
#include <memory>
#include <string>
#include "Utils.h"
#include "Config.h"

namespace CynicScript
{
    class CYS_API Config
    {
    public:
        SINGLETON_DECL(Config)

        void SetExecuteFilePath(std::string_view path);
        const std::string &GetExecuteFilePath() const;

        void SetUseFunctionCache(bool toggle);
        bool IsUseFunctionCache() const;

        void SetSerializeBinaryChunk(bool toggle);
        bool IsSerializeBinaryChunk() const;

        void SetSerializeBinaryFilePath(std::string_view path);
        std::string_view GetSerializeBinaryFilePath() const;

        std::string ToFullPath(std::string_view filePath);
    private:
        Config() = default;
        ~Config() = default;
        std::string mExecuteFilePath;

        bool mUseFunctionCache{false};

        bool mIsSerializeBinaryChunk{false};
        std::string_view mSerializeBinaryFilePath;

#ifndef NDEBUG
    public:
        void SetDebugGC(bool toggle);
        bool IsDebugGC() const;

        void SetStressGC(bool toggle);
        bool IsStressGC() const;

    private:
        bool mDebugGC{false};
        bool mStressGC{false};
#endif
    };
}