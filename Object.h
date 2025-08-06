#pragma once
#include <string>
#include <functional>
#include <vector>
#include <unordered_map>
#include <map>
#include "Chunk.h"
#include "Token.h"
#include "Value.h"
namespace CynicScript
{
#define CYS_IS_STR_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::STR)
#define CYS_IS_ARRAY_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::ARRAY)
#define CYS_IS_TABLE_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::DICT)
#define CYS_IS_STRUCT_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::STRUCT)
#define CYS_IS_FUNCTION_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::FUNCTION)
#define CYS_IS_UPVALUE_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::UPVALUE)
#define CYS_IS_CLOSURE_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::CLOSURE)
#define CYS_IS_NATIVE_FUNCTION_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::NATIVE_FUNCTION)
#define CYS_IS_REF_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::REF)
#define CYS_IS_CLASS_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::CLASS)
#define CYS_IS_CLASS_INSTANCE_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::CLASS_INSTANCE)
#define CYS_IS_CLASS_CLOSURE_BIND_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::CLASS_CLOSURE_BIND)
#define CYS_IS_ENUM_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::ENUM)
#define CYS_IS_MODULE_OBJ(obj) ((obj)->kind == ::CynicScript::ObjectKind::MODULE)

#define CYS_TO_STR_OBJ(obj) ((::CynicScript::StrObject *)(obj))
#define CYS_TO_ARRAY_OBJ(obj) ((::CynicScript::ArrayObject *)(obj))
#define CYS_TO_TABLE_OBJ(obj) ((::CynicScript::DictObject *)(obj))
#define CYS_TO_STRUCT_OBJ(obj) ((::CynicScript::StructObject *)(obj))
#define CYS_TO_FUNCTION_OBJ(obj) ((::CynicScript::FunctionObject *)(obj))
#define CYS_TO_UPVALUE_OBJ(obj) ((::CynicScript::UpValueObject *)(obj))
#define CYS_TO_CLOSURE_OBJ(obj) ((::CynicScript::ClosureObject *)(obj))
#define CYS_TO_NATIVE_FUNCTION_OBJ(obj) ((::CynicScript::NativeFunctionObject *)(obj))
#define CYS_TO_REF_OBJ(obj) ((::CynicScript::RefObject *)(obj))
#define CYS_TO_CLASS_OBJ(obj) ((::CynicScript::ClassObject *)(obj))
#define CYS_TO_CLASS_INSTANCE_OBJ(obj) ((::CynicScript::ClassInstanceObject *)(obj))
#define CYS_TO_CLASS_CLOSURE_BIND_OBJ(obj) ((::CynicScript::ClassClosureBindObject *)(obj))
#define CYS_TO_ENUM_OBJ(obj) ((::CynicScript::EnumObject *)(obj))
#define CYS_TO_MODULE_OBJ(obj) ((::CynicScript::ModuleObject *)(obj))

#define CYS_IS_NULL_VALUE(v) ((v).kind == ::CynicScript::ValueKind::NIL)
#define CYS_IS_INT_VALUE(v) ((v).kind == ::CynicScript::ValueKind::INT)
#define CYS_IS_REAL_VALUE(v) ((v).kind == ::CynicScript::ValueKind::REAL)
#define CYS_IS_BOOL_VALUE(v) ((v).kind == ::CynicScript::ValueKind::BOOL)
#define CYS_IS_OBJECT_VALUE(v) ((v).kind == ::CynicScript::ValueKind::OBJECT)
#define CYS_IS_STR_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_STR_OBJ((v).object))
#define CYS_IS_ARRAY_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_ARRAY_OBJ((v).object))
#define CYS_IS_DICT_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_TABLE_OBJ((v).object))
#define CYS_IS_STRUCT_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_STRUCT_OBJ((v).object))
#define CYS_IS_FUNCTION_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_FUNCTION_OBJ((v).object))
#define CYS_IS_UPVALUE_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_UPVALUE_OBJ((v).object))
#define CYS_IS_CLOSURE_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_CLOSURE_OBJ((v).object))
#define CYS_IS_NATIVE_FUNCTION_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_NATIVE_FUNCTION_OBJ((v).object))
#define CYS_IS_REF_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_REF_OBJ((v).object))
#define CYS_IS_CLASS_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_CLASS_OBJ((v).object))
#define CYS_IS_CLASS_INSTANCE_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_CLASS_INSTANCE_OBJ((v).object))
#define CYS_IS_CLASS_CLOSURE_BIND_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_CLASS_CLOSURE_BIND_OBJ((v).object))
#define CYS_IS_ENUM_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_ENUM_OBJ((v).object))
#define CYS_IS_MODULE_VALUE(v) (CYS_IS_OBJECT_VALUE(v) && CYS_IS_MODULE_OBJ((v).object))

#define CYS_TO_INT_VALUE(v) ((v).integer)
#define CYS_TO_REAL_VALUE(v) ((v).realnum)
#define CYS_TO_BOOL_VALUE(v) ((v).boolean)
#define CYS_TO_OBJECT_VALUE(v) ((v).object)
#define CYS_TO_STR_VALUE(v) (CYS_TO_STR_OBJ((v).object))
#define CYS_TO_ARRAY_VALUE(v) (CYS_TO_ARRAY_OBJ((v).object))
#define CYS_TO_DICT_VALUE(v) (CYS_TO_TABLE_OBJ((v).object))
#define CYS_TO_STRUCT_VALUE(v) (CYS_TO_STRUCT_OBJ((v).object))
#define CYS_TO_FUNCTION_VALUE(v) (CYS_TO_FUNCTION_OBJ((v).object))
#define CYS_TO_UPVALUE_VALUE(v) (CYS_TO_UPVALUE_OBJ((v).object))
#define CYS_TO_CLOSURE_VALUE(v) (CYS_TO_CLOSURE_OBJ((v).object))
#define CYS_TO_NATIVE_FUNCTION_VALUE(v) (CYS_TO_NATIVE_FUNCTION_OBJ((v).object))
#define CYS_TO_REF_VALUE(v) (CYS_TO_REF_OBJ((v).object))
#define CYS_TO_CLASS_VALUE(v) (CYS_TO_CLASS_OBJ((v).object))
#define CYS_TO_CLASS_INSTANCE_VALUE(v) (CYS_TO_CLASS_INSTANCE_OBJ((v).object))
#define CYS_TO_CLASS_CLOSURE_BIND_VALUE(v) (CYS_TO_CLASS_CLOSURE_BIND_OBJ((v).object))
#define CYS_TO_ENUM_VALUE(v) (CYS_TO_ENUM_OBJ((v).object))
#define CYS_TO_MODULE_VALUE(v) (CYS_TO_MODULE_OBJ((v).object))

    enum CYS_API ObjectKind : uint8_t
    {
        STR,
        ARRAY,
        DICT,
        STRUCT,
        FUNCTION,
        UPVALUE,
        CLOSURE,
        NATIVE_FUNCTION,
        REF,
        CLASS,
        CLASS_INSTANCE,
        CLASS_CLOSURE_BIND,
        ENUM,
        MODULE
    };

    struct CYS_API Object
    {
        Object(ObjectKind kind);
        virtual ~Object() = default;

        void Mark();
        void UnMark();

        virtual STRING ToString() const = 0;
        virtual void Blacken();
        virtual bool IsEqualTo(Object *other) = 0;
        virtual std::vector<uint8_t> Serialize() const = 0;

        const ObjectKind kind;
        bool marked{false};
        Object *next{nullptr};
    };

    struct CYS_API StrObject : public Object
    {
        StrObject(STRING_VIEW value);
        ~StrObject() override = default;

        STRING ToString() const override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        STRING value{};
    };

    struct CYS_API ArrayObject : public Object
    {
        ArrayObject();
        ArrayObject(const std::vector<struct Value> &elements);
        ~ArrayObject() override = default;

        STRING ToString() const override;
        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        std::vector<struct Value> elements{};
    };

    struct CYS_API DictObject : public Object
    {
        DictObject();
        DictObject(const ValueUnorderedMap &elements);
        ~DictObject() override = default;

        STRING ToString() const override;
        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        ValueUnorderedMap elements{};
    };

    struct CYS_API StructObject : public Object
    {
        StructObject();
        StructObject(const std::unordered_map<STRING, Value> &elements);
        ~StructObject() override = default;

        STRING ToString() const override;
        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        std::unordered_map<STRING, Value> elements{};
    };

    struct CYS_API FunctionObject : public Object
    {
        FunctionObject();
        FunctionObject(STRING_VIEW name);
        ~FunctionObject() override = default;

        STRING ToString() const override;
#ifndef NDEBUG
        STRING ToStringWithChunk() const;
#endif
        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        // ++ Function cache relative
        void SetCache(size_t hash, const std::vector<Value> &result);
        bool GetCache(size_t hash, std::vector<Value> &result) const;
        std::unordered_map<size_t, std::vector<Value>> caches;
        // -- Function cache relative

        uint8_t arity{0};
        VarArg varArg{VarArg::NONE};
        int8_t upValueCount{0};
        Chunk chunk{};
        STRING name{};
    };

    struct CYS_API UpValueObject : public Object
    {
        UpValueObject();
        UpValueObject(Value *location);
        ~UpValueObject() override = default;

        STRING ToString() const override;
        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        Value *location{nullptr};
        Value closed{};
        UpValueObject *nextUpValue{nullptr};
    };

    struct CYS_API ClosureObject : public Object
    {
        ClosureObject();
        ClosureObject(FunctionObject *function);
        ~ClosureObject() override = default;

        STRING ToString() const override;
        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        FunctionObject *function{nullptr};
        std::vector<UpValueObject *> upvalues{};
    };

    using NativeFunction = std::function<bool(Value *, uint32_t, const Token *, Value &)>;

    struct CYS_API NativeFunctionObject : public Object
    {
        NativeFunctionObject();
        NativeFunctionObject(NativeFunction f);
        ~NativeFunctionObject() override = default;

        STRING ToString() const override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        NativeFunction fn{};
    };

    struct CYS_API RefObject : public Object
    {
        RefObject(Value *pointer);
        ~RefObject() override = default;

        STRING ToString() const override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        Value *pointer{nullptr};
    };

    struct CYS_API ClassObject : public Object
    {
        ClassObject();
        ClassObject(STRING_VIEW name);
        ~ClassObject() override = default;

        STRING ToString() const override;
        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        bool GetMember(const STRING &name, Value &retV);
        bool GetParentMember(const STRING &name, Value &retV);

        STRING name{};
        std::map<int32_t, ClosureObject *> constructors{}; // argument count as key for now
        std::unordered_map<STRING, Value> defaultMembers{};
        std::unordered_map<STRING, Value> functions{};
        std::unordered_map<STRING, Value> enums{};
        std::map<STRING, ClassObject *> parents{};
    };

    struct CYS_API ClassInstanceObject : public Object
    {
        ClassInstanceObject();
        ClassInstanceObject(ClassObject *klass);
        ~ClassInstanceObject() override = default;
        
        STRING ToString() const override;
        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        bool GetMember(const STRING &name, Value &retV);

        void ObtainParentMembers(ClassObject *classObj);

        ClassObject *klass;
        std::unordered_map<STRING, Value> members{};
        std::map<STRING, std::unordered_map<STRING,Value>> parentMembers{};//collect parent members
    };

    struct CYS_API ClassClosureBindObject : public Object
    {
        ClassClosureBindObject();
        ClassClosureBindObject(const Value &receiver, ClosureObject *cl);
        ~ClassClosureBindObject() override = default;

        STRING ToString() const override;
        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        Value receiver{};//ClassObject or CLassInstanceObject
        ClosureObject *closure{nullptr};
    };

    struct CYS_API EnumObject : public Object
    {
        EnumObject();
        EnumObject(const STRING &name, const std::unordered_map<STRING, Value> &pairs);
        ~EnumObject() override = default;

        STRING ToString() const override;
        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        bool GetMember(const STRING &name, Value &retV);

        STRING name{};
        std::unordered_map<STRING, Value> pairs{};
    };

    struct CYS_API ModuleObject : public Object
    {
        ModuleObject();
        ModuleObject(const STRING &name, const std::unordered_map<STRING, Value> &members={});
        ~ModuleObject() override = default;

        STRING ToString() const override;
        void Blacken() override;
        bool IsEqualTo(Object *other) override;
        std::vector<uint8_t> Serialize() const override;

        bool GetMember(const STRING &name, Value &retV);

        STRING name{};
        std::unordered_map<STRING, Value> members{};
    };
}