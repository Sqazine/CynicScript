#include "Chunk.h"
#include <iomanip>
#include <sstream>
#include "Version.h"
#include "Utils.h"
#include "Object.h"
#include "Logger.h"
namespace CynicScript
{
	Chunk::Chunk(const OpCodeList &opcodes, const std::vector<Value> &constants)
		: opCodes(opcodes), constants(constants)
	{
	}

#ifndef NDEBUG
	STRING Chunk::ToString() const
	{
		STRING result;
		result += OpCodeToString(opCodes);
		for (const auto &c : constants)
		{
			if (CYS_IS_FUNCTION_VALUE(c))
				result += CYS_TO_FUNCTION_VALUE(c)->ToStringWithChunk();
		}
		return result;
	}
#endif

	std::vector<uint8_t> Chunk::Serialize() const
	{
		std::vector<uint8_t> result;

		auto magNumberBytes = ByteConverter::ToU32ByteList(CYS_BINARY_FILE_MAGIC_NUMBER);
		result.insert(result.end(), magNumberBytes.begin(), magNumberBytes.end());

		auto versionBytes = ByteConverter::ToU32ByteList(CYS_VERSION_BINARY);
		result.insert(result.end(), versionBytes.begin(), versionBytes.end());

		auto opCodeCount = ByteConverter::ToU32ByteList(opCodes.size());
		result.insert(result.end(), opCodeCount.begin(), opCodeCount.end());

		result.insert(result.end(), opCodes.begin(), opCodes.end());

		auto constantsCount = ByteConverter::ToU32ByteList(constants.size());
		result.insert(result.end(), constantsCount.begin(), constantsCount.end());

		for (const auto &constant : constants)
		{
			auto bytes = constant.Serialize();

			auto size = ByteConverter::ToU32ByteList(bytes.size());
			result.insert(result.end(), size.begin(), size.end());

			result.insert(result.end(), bytes.begin(), bytes.end());
		}

		return result;
	}

	void Chunk::Deserialize(const std::vector<uint8_t> &data)
	{
		auto magicNumber = ByteConverter::GetU32Integer(data, 0);
		if (magicNumber != CYS_BINARY_FILE_MAGIC_NUMBER)
			CYS_LOG_ERROR(TEXT("Invalid CynicScript binary file,cannot deserialize from this file"));

		auto versionNumber = ByteConverter::GetU32Integer(data, 4);
		if (versionNumber != CYS_VERSION_BINARY)
			CYS_LOG_ERROR(TEXT("Invalid CynicScript binary file version of {},current version is {}"), versionNumber, CYS_VERSION_BINARY);

		auto opCodesCount = ByteConverter::GetU32Integer(data, 8);

		opCodes.insert(opCodes.end(), data.begin() + 12, data.begin() + 12 + opCodesCount);

		auto idx = 12 + opCodesCount;

		auto constantsCount = ByteConverter::GetU32Integer(data, idx);
		idx += 4;
		for (int32_t i = 0; i < constantsCount; ++i)
		{
			auto constantSize = ByteConverter::GetU32Integer(data, idx);
			idx += 4;

			std::vector<uint8_t> constantBytes(data.begin() + idx, data.begin() + idx + constantSize);

			Value v;
			v.Deserialize(constantBytes);

			constants.emplace_back(v);
		}
	}

	STRING Chunk::OpCodeToString(const OpCodeList &opcodes) const
	{
#define CASE(opCode)                                                                                                          \
	case opCode:                                                                                                              \
	{                                                                                                                         \
		auto instrLoc = i;                                                                                                    \
		auto tok = opCodeRelatedTokens[opcodes[++i]];                                                                         \
		auto tokStr = tok->ToString();                                                                                        \
		STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));                                                          \
		tokStr += tokGap;                                                                                                     \
		stream << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << instrLoc << TEXT("\t") << TEXT(#opCode) << std::endl; \
		break;                                                                                                                \
	}

#define CASE_JUMP(opCode, op)                                                                                                                                                      \
	case opCode:                                                                                                                                                                   \
	{                                                                                                                                                                              \
		auto instrLoc = i;                                                                                                                                                         \
		auto tok = opCodeRelatedTokens[opcodes[++i]];                                                                                                                              \
		uint16_t addressOffset = opcodes[++i] << 8 | opcodes[++i];                                                                                                                 \
		auto tokStr = tok->ToString();                                                                                                                                             \
		STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));                                                                                                               \
		tokStr += tokGap;                                                                                                                                                          \
		stream << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << instrLoc << TEXT("\t") << TEXT(#opCode) << TEXT("\t") << i << "->" << i op addressOffset + 3 << std::endl; \
		break;                                                                                                                                                                     \
	}

#define CASE_1(opCode)                                                                                                                             \
	case opCode:                                                                                                                                   \
	{                                                                                                                                              \
		auto instrLoc = i;                                                                                                                         \
		auto tok = opCodeRelatedTokens[opcodes[++i]];                                                                                              \
		auto pos = opcodes[++i];                                                                                                                   \
		auto tokStr = tok->ToString();                                                                                                             \
		STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));                                                                               \
		tokStr += tokGap;                                                                                                                          \
		stream << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << instrLoc << TEXT("\t") << TEXT(#opCode) << TEXT("\t") << pos << std::endl; \
		break;                                                                                                                                     \
	}

		const uint32_t maxTokenShowSize = GetBiggestTokenLength() + 4; // 4 for a gap "    "
		STRING_STREAM stream;
		for (int32_t i = 0; i < opcodes.size(); ++i)
		{
			switch (opcodes[i])
			{
				CASE(OP_NULL)
				CASE(OP_ADD)
				CASE(OP_SUB)
				CASE(OP_MUL)
				CASE(OP_DIV)
				CASE(OP_LESS)
				CASE(OP_GREATER)
				CASE(OP_NOT)
				CASE(OP_MINUS)
				CASE(OP_BIT_AND)
				CASE(OP_BIT_OR)
				CASE(OP_BIT_XOR)
				CASE(OP_BIT_NOT)
				CASE(OP_BIT_LEFT_SHIFT)
				CASE(OP_BIT_RIGHT_SHIFT)
				CASE(OP_EQUAL)
				CASE(OP_FACTORIAL)
				CASE(OP_CLOSE_UPVALUE)
				CASE(OP_GET_INDEX)
				CASE(OP_SET_INDEX)
				CASE(OP_POP)
				CASE(OP_GET_BASE)
				CASE(OP_SET_PROPERTY)
				CASE(OP_GET_PROPERTY)
				CASE_JUMP(OP_JUMP_IF_FALSE, +)
				CASE_JUMP(OP_JUMP, +)
				CASE_JUMP(OP_LOOP, -)
				CASE_1(OP_RETURN)
				CASE_1(OP_ARRAY)
				CASE_1(OP_DICT)
				CASE_1(OP_SET_GLOBAL)
				CASE_1(OP_GET_GLOBAL)
				CASE_1(OP_SET_LOCAL)
				CASE_1(OP_GET_LOCAL)
				CASE_1(OP_SET_UPVALUE)
				CASE_1(OP_GET_UPVALUE)
				CASE_1(OP_REF_GLOBAL)
				CASE_1(OP_REF_LOCAL)
				CASE_1(OP_REF_INDEX_GLOBAL)
				CASE_1(OP_REF_INDEX_LOCAL)
				CASE_1(OP_REF_UPVALUE)
				CASE_1(OP_REF_INDEX_UPVALUE)
				CASE_1(OP_CALL)
				CASE_1(OP_STRUCT)
				CASE_1(OP_APPREGATE_RESOLVE)
				CASE_1(OP_APPREGATE_RESOLVE_VAR_ARG)
				CASE_1(OP_INIT_VAR_ARG)
			case OP_CONSTANT:
			{
				auto instrLoc = i;
				auto tok = opCodeRelatedTokens[opcodes[++i]];
				auto pos = opcodes[++i];
				STRING constantStr = constants[pos].ToString();

				auto tokStr = tok->ToString();
				STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));
				tokStr += tokGap;
				stream << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << instrLoc << TEXT("\tOP_CONSTANT\t") << pos << TEXT("\t'") << constantStr << TEXT("'") << std::endl;
				break;
			}
			case OP_CLASS:
			{
				auto instrLoc = i;
				auto tok = opCodeRelatedTokens[opcodes[++i]];
				auto constructorCount = opcodes[++i];
				auto parentClassCount = opcodes[++i];
				auto varCount = opcodes[++i];
				auto constCount = opcodes[++i];
				auto tokStr = tok->ToString();
				STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));
				tokStr += tokGap;
				stream << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\tOP_CLASS\t") << constructorCount << TEXT("\t") << parentClassCount << TEXT("\t") << varCount << TEXT("\t") << constCount << TEXT("\t") << std::endl;
				break;
			}
			case OP_CLOSURE:
			{
				auto instrLoc = i;
				auto tok = opCodeRelatedTokens[opcodes[++i]];
				auto pos = opcodes[++i];
				STRING funcStr = (TEXT("<fn ") + CYS_TO_FUNCTION_VALUE(constants[pos])->name + TEXT(":0x") + PointerAddressToString((void *)CYS_TO_FUNCTION_VALUE(constants[pos])) + TEXT(">"));

				auto tokStr = tok->ToString();
				STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));
				tokStr += tokGap;

				stream << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\tOP_CLOSURE\t") << pos << TEXT("\t") << funcStr << std::endl;

				auto upvalueCount = CYS_TO_FUNCTION_VALUE(constants[pos])->upValueCount;
				if (upvalueCount > 0)
				{
					stream << TEXT("        upvalues:") << std::endl;
					for (auto j = 0; j < upvalueCount; ++j)
					{
						stream << TEXT("                 location  ") << opcodes[++i];
						stream << TEXT(" | ");
						stream << TEXT("depth  ") << opcodes[++i] << std::endl;
					}
				}
				break;
			}
			case OP_MODULE:
			{
				auto instrLoc = i;
				auto tok = opCodeRelatedTokens[opcodes[++i]];
				auto varCount = opcodes[++i];
				auto constCount = opcodes[++i];
				auto tokStr = tok->ToString();
				STRING tokGap(maxTokenShowSize - tokStr.size(), TCHAR(' '));
				tokStr += tokGap;
				stream << tokStr << std::setfill(TCHAR('0')) << std::setw(8) << i << TEXT("\tOP_MODULE\t") << varCount << TEXT("\t") << constCount << std::endl;
				break;
			}
			default:
				break;
			}
		}

		return stream.str();
	}

	uint32_t Chunk::GetBiggestTokenLength() const
	{
		uint32_t length = 0;
		for (const auto &t : opCodeRelatedTokens)
		{
			auto l = (uint32_t)t->ToString().size();
			if (length < l)
				length = l;
		}
		return length;
	}

	bool operator==(const Chunk &left, const Chunk &right)
	{
		if (left.opCodes != right.opCodes)
			return false;
		if (left.constants.size() != right.constants.size())
			return false;
		for (int32_t i = 0; i < left.constants.size(); ++i)
			if (left.constants[i] != right.constants[i])
				return false;
		return true;
	}

	bool operator!=(const Chunk &left, const Chunk &right)
	{
		return !(left == right);
	}
}