#pragma once
#include <iostream>
#include <vector>
#include "Chunk.h"
#include "Object.h"
#include "LibraryManager.h"
namespace CynicScript
{
    class CYS_API VM
    {
    public:
        constexpr VM() noexcept = default;
        constexpr ~VM() noexcept = default;

        std::vector<Value> Run(FunctionObject *mainFunc) noexcept;

    private:
        void Execute();

        bool IsFalsey(const Value &v) noexcept;
    };
}