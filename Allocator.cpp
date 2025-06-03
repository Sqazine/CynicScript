#include "Allocator.h"
#include "VM.h"
namespace CynicScript
{
    SINGLETON_IMPL(Allocator)

    Allocator::Allocator()
        : mObjectChain(nullptr)
    {
        ResetStatus();
    }

    Allocator::~Allocator()
    {
        FreeObjects();
    }

    void Allocator::ResetStatus()
    {
        if (mObjectChain)
            FreeObjects();

        mBytesAllocated = 0;
        mNextGCByteSize = 256;
        mObjectChain = nullptr;

        mCallFrameTop = mCallFrameStack;
        mStackTop = mValueStack;

        mOpenUpValues = nullptr;

        memset(mGlobalVariableList, 0, sizeof(Value) * GLOBAL_VARIABLE_MAX);

        for (int32_t i = 0; i < LibraryManager::GetInstance()->GetLibraries().size(); ++i)
            mGlobalVariableList[i] = LibraryManager::GetInstance()->GetLibraries()[i];
    }

    void Allocator::FreeObjects()
    {
        auto bytes = mBytesAllocated;
        Object *object = mObjectChain;
        while (object != nullptr)
        {
            Object *next = object->next;
            size_t objBytes = sizeof(*object);
            mBytesAllocated -= objBytes;
            SAFE_DELETE(object);
            object = next;
        }

#ifdef CYS_GC_DEBUG
        Logger::Info(TEXT("collected {} bytes (from {} to {}) next gc bytes {}"), bytes - mBytesAllocated, bytes, mNextGCByteSize);
#endif
    }

    void Allocator::PushStack(const Value &value)
    {
#ifndef NDEBUG
        if (mStackTop - mValueStack >= STACK_MAX)
            CYS_LOG_ERROR(TEXT("Stack overflow."));
#endif
        *(mStackTop++) = value;
    }
    Value Allocator::PopStack()
    {
#ifndef NDEBUG
        if (mStackTop - mValueStack <= 0)
            CYS_LOG_ERROR(TEXT("Stack underflow."));
#endif
        return *(--mStackTop);
    }
    Value Allocator::PeekStack(int32_t distance)
    {
        return *(mStackTop - distance - 1);
    }

    void Allocator::PushCallFrame(const CallFrame &callFrame)
    {
        *(mCallFrameTop++) = callFrame;
    }

    CallFrame *Allocator::PopCallFrame()
    {
        return --mCallFrameTop;
    }

    CallFrame *Allocator::PeekCallFrame(int32_t distance)
    {
        return mCallFrameTop - distance - 1;
    }

    bool Allocator::IsCallFrameStackEmpty()
    {
        return mCallFrameTop == mCallFrameStack;
    }

    size_t Allocator::CallFrameCount()
    {
        return mCallFrameTop - mCallFrameStack;
    }

    UpValueObject *Allocator::CaptureUpValue(Value *location)
    {
        UpValueObject *prevUpValue = nullptr;
        UpValueObject *upValue = mOpenUpValues;

        while (upValue != nullptr && upValue->location > location)
        {
            prevUpValue = upValue;
            upValue = upValue->nextUpValue;
        }

        if (upValue != nullptr && upValue->location == location)
            return upValue;

        auto createdUpValue = CreateObject<UpValueObject>(location);
        createdUpValue->nextUpValue = upValue;

        if (prevUpValue == nullptr)
            mOpenUpValues = createdUpValue;
        else
            prevUpValue->nextUpValue = createdUpValue;

        return createdUpValue;
    }
    void Allocator::ClosedUpValues(Value *end)
    {
        while (mOpenUpValues != nullptr && mOpenUpValues->location >= end)
        {
            UpValueObject *upvalue = mOpenUpValues;
            upvalue->closed = *upvalue->location;
            upvalue->location = &upvalue->closed;
            mOpenUpValues = upvalue->nextUpValue;
        }
    }

    void Allocator::SetStackTop(Value *top)
    {
        mStackTop = top;
    }

    Value *Allocator::StackTop() const
    {
        return mStackTop;
    }

    Value *Allocator::Stack()
    {
        return mValueStack;
    }

    void Allocator::MoveStackTop(int32_t offset)
    {
        mStackTop += offset;
    }

    void Allocator::SetValueFromStackTopOffset(int32_t offset, const Value &value)
    {
        mStackTop[offset] = value;
    }

    Value *Allocator::GetGlobalVariable(size_t idx)
    {
        return &mGlobalVariableList[idx];
    }

    void Allocator::SetGlobalVariable(size_t idx, const Value &v)
    {
        mGlobalVariableList[idx] = v;
    }

    void Allocator::GC()
    {
#ifdef CYS_GC_DEBUG
        Logger::Info(TEXT("begin gc"));
        size_t bytes = mBytesAllocated;
#endif

        MarkRootObjects();
        MarkGrayObjects();
        Sweep();
        mNextGCByteSize = mBytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef CYS_GC_DEBUG
        Logger::Info(TEXT("end gc"));
        Logger::Info(TEXT("    collected {} bytes (from {} to {}) next gc bytes {}"), bytes - mBytesAllocated, bytes, mNextGCByteSize);
#endif
    }

    void Allocator::MarkRootObjects()
    {
        for (Value *slot = mValueStack; slot < mStackTop; ++slot)
            slot->Mark();
        for (CallFrame *slot = mCallFrameStack; slot < mCallFrameTop; ++slot)
            slot->closure->Mark();
        for (UpValueObject *upvalue = mOpenUpValues; upvalue != nullptr; upvalue = upvalue->nextUpValue)
            upvalue->Mark();

        for (int32_t i = 0; i < GLOBAL_VARIABLE_MAX; ++i)
            if (mGlobalVariableList[i] != Value())
                mGlobalVariableList[i].Mark();
    }

    void Allocator::MarkGrayObjects()
    {
        while (mGrayObjects.size() > 0)
        {
            auto object = mGrayObjects.back();
            mGrayObjects.pop_back();
            object->Blacken();
        }
    }

    void Allocator::Sweep()
    {
        Object *previous = nullptr;
        Object *object = mObjectChain;
        while (object)
        {
            if (object->marked)
            {
                object->UnMark();
                previous = object;
                object = object->next;
            }
            else
            {
                Object *unreached = object;
                object = object->next;
                if (previous != nullptr)
                    previous->next = object;
                else
                    mObjectChain = object;

                FreeObject(unreached);
            }
        }
    }
}