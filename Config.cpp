#include "Config.h"
#include <filesystem>

namespace CynicScript
{
    SINGLETON_IMPL(Config)

    void Config::SetExecuteFilePath(std::string_view path)
    {
        mExecuteFilePath = path;
    }

    const std::string &Config::GetExecuteFilePath() const
    {
        return mExecuteFilePath;
    }

    void Config::SetUseFunctionCache(bool toggle)
    {
        mUseFunctionCache = toggle;
    }

    bool Config::IsUseFunctionCache() const
    {
        return mUseFunctionCache;
    }

    void Config::SetSerializeBinaryChunk(bool toggle)
    {
        mIsSerializeBinaryChunk = toggle;
    }

    bool Config::IsSerializeBinaryChunk() const
    {
        return mIsSerializeBinaryChunk;
    }

    void Config::SetSerializeBinaryFilePath(std::string_view path)
    {
        mSerializeBinaryFilePath = path;
    }

    std::string_view Config::GetSerializeBinaryFilePath() const
    {
        return mSerializeBinaryFilePath;
    }

    std::string Config::ToFullPath(std::string_view filePath)
    {
        std::filesystem::path filesysPath = filePath;
        std::string fullPath = filesysPath.string();
        if (!filesysPath.is_absolute())
            fullPath = mExecuteFilePath + fullPath;
        return fullPath;
    }
    void Config::SetDebugGC(bool toggle)
    {
        mDebugGC = toggle;
    }
    bool Config::IsDebugGC() const
    {
        return mDebugGC;
    }
    void Config::SetStressGC(bool toggle)
    {
        mStressGC = toggle;
    }
    bool Config::IsStressGC() const
    {
        return mStressGC;
    }
}