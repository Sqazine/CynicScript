#pragma once
#include "Token.h"
namespace CynicScript
{
    enum class TypeKind
    {
        UNDEFINED = 0,

        // Primitive Type
        I8,
        U8,
        I16,
        U16,
        I32,
        U32,
        I64,
        U64,

        F32,
        F64,

        BOOL,
        CHAR,
        ANY,
        STR,
        PRIMITIVE = STR, // This is a special type that represents all primitive types

        // Composite Type
        STRUCT,
        CLASS,
    };

    class Type
    {
    public:
        Type() noexcept;
        Type(TypeKind kind,const SourceLocation& sourceLocation) noexcept;
        Type(STRING_VIEW name,const SourceLocation& sourceLocation) noexcept;
        Type(TypeKind kind, STRING_VIEW name,const SourceLocation& sourceLocation) noexcept;
        ~Type() noexcept = default;

        TypeKind GetKind() const noexcept;

        bool IsNumeric() const noexcept;
        bool IsInteger() const noexcept;
        bool IsFloating() const noexcept;

        bool Is(TypeKind kind)
        {
            return mKind == kind;
        }

        STRING_VIEW GetName() const noexcept;

        bool IsPrimitiveType() const noexcept;
        bool IsCompositeType() const noexcept;

    private:
        void CheckIsValid() const;

        STRING_VIEW mName;
        TypeKind mKind;
        SourceLocation mSourceLocation;
    };
};