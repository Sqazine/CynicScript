#pragma once
#include <vector>
#include "Value.h"
#include "Token.h"
namespace CynicScript
{
    enum OpCode : uint8_t
    {
        OP_CONSTANT,
        OP_NULL,
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_MOD,
        OP_EQUAL,
        OP_GREATER,
        OP_LESS,
        OP_NOT,
        OP_MINUS,
        OP_BIT_AND,
        OP_BIT_OR,
        OP_BIT_XOR,
        OP_BIT_NOT,
        OP_BIT_LEFT_SHIFT,
        OP_BIT_RIGHT_SHIFT,
        OP_RETURN,
        OP_FACTORIAL,
        OP_ARRAY,
        OP_DICT,
        OP_GET_INDEX,
        OP_SET_INDEX,
        OP_JUMP_IF_FALSE,
        OP_JUMP,
        OP_LOOP,
        OP_POP,
        OP_SET_GLOBAL,
        OP_GET_GLOBAL,
        OP_SET_LOCAL,
        OP_GET_LOCAL,
        OP_GET_UPVALUE,
        OP_SET_UPVALUE,
        OP_CLOSE_UPVALUE,
        OP_REF_GLOBAL,
        OP_REF_LOCAL,
        OP_REF_INDEX_GLOBAL,
        OP_REF_INDEX_LOCAL,
        OP_REF_UPVALUE,
        OP_REF_INDEX_UPVALUE,
        OP_CALL,
        OP_CLASS,
        OP_CLASS_INSTANCE,
        OP_STRUCT,
        OP_SET_PROPERTY,
        OP_GET_PROPERTY,
        OP_GET_BASE,
        OP_CLOSURE,
        OP_APPREGATE_RESOLVE,
        OP_APPREGATE_RESOLVE_VAR_ARG,
        OP_MODULE,
        OP_INIT_VAR_ARG,
    };

    using OpCodeList = std::vector<uint8_t>;

    class CYS_API Chunk
    {
    public:
        Chunk() = default;
        Chunk(const OpCodeList &opcodes, const std::vector<Value> &constants);
        ~Chunk() = default;
#ifndef NDEBUG
        STRING ToString() const;
#endif
        std::vector<uint8_t> Serialize() const;
        void Deserialize(const std::vector<uint8_t> &data);

        OpCodeList opCodes;
        std::vector<Value> constants;
        std::vector<const Token *> opCodeRelatedTokens;

    private:
        STRING OpCodeToString(const OpCodeList &opcodes) const;
        uint32_t GetBiggestTokenLength() const;
    };

    bool operator==(const Chunk &left, const Chunk &right);
    bool operator!=(const Chunk &left, const Chunk &right);
}