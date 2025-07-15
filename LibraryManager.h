#pragma once
#include <unordered_map>
#include <functional>
#include "Value.h"
#include "Object.h"
#include "Common.h"
namespace CynicScript
{
    class CYS_API LibraryManager:public Singleton<LibraryManager>
    {
    public:
        void Init() override;
        void Destroy() override;

        void RegisterLibrary(ClassObject *libraryClass);
        const std::vector<ClassObject *> &GetLibraries() const;

    private:
        std::vector<ClassObject *> mLibraries;
    };
}