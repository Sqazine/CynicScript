#include "VM.h"
#include <iostream>
#include "Allocator.h"
#include "Common.h"
#include "Object.h"
#include "Token.h"
#include "Logger.h"

namespace CynicScript
{
	std::vector<Value> VM::Run(FunctionObject *mainFunc) noexcept
	{
		Allocator::GetInstance()->StopGC();
		auto closure = Allocator::GetInstance()->CreateObject<ClosureObject>(mainFunc);
		Allocator::GetInstance()->RecoverGC();

		Allocator::GetInstance()->ResetStackPointer();
		Allocator::GetInstance()->ResetCallFramePointer();

		PUSH_STACK(closure);

		CallFrame mainCallFrame;
		mainCallFrame.closure = closure;
		mainCallFrame.ip = closure->function->chunk.opCodes.data();
		mainCallFrame.slots = STACK_TOP() - 1;

		PUSH_CALL_FRAME(mainCallFrame);

		Execute();

		std::vector<Value> returnValues;
#ifndef NDEBUG
		if (STACK_TOP() != STACK() + 1)
			CYS_LOG_ERROR_WITH_LOC(new Token(), TEXT("Stack occupancy exception."));
#endif

		while (STACK_TOP() != STACK() + 1)
			returnValues.emplace_back(POP_STACK());

		POP_STACK();

		return returnValues;
	}

	void VM::Execute()
	{
		//  - * /
#define COMMON_BINARY(op)                                                                                                                                                                                                     \
	do                                                                                                                                                                                                                        \
	{                                                                                                                                                                                                                         \
		Value right = POP_STACK();                                                                                                                                                                                            \
		Value left = POP_STACK();                                                                                                                                                                                             \
		if (CYS_IS_REF_VALUE(left))                                                                                                                                                                                           \
			left = *CYS_TO_REF_VALUE(left)->pointer;                                                                                                                                                                          \
		if (CYS_IS_REF_VALUE(right))                                                                                                                                                                                          \
			right = *CYS_TO_REF_VALUE(right)->pointer;                                                                                                                                                                        \
		if (CYS_IS_INT_VALUE(left) && CYS_IS_INT_VALUE(right))                                                                                                                                                                \
			PUSH_STACK(CYS_TO_INT_VALUE(left) op CYS_TO_INT_VALUE(right));                                                                                                                                                    \
		else if (CYS_IS_REAL_VALUE(left) && CYS_IS_REAL_VALUE(right))                                                                                                                                                         \
			PUSH_STACK(CYS_TO_REAL_VALUE(left) op CYS_TO_REAL_VALUE(right));                                                                                                                                                  \
		else if (CYS_IS_INT_VALUE(left) && CYS_IS_REAL_VALUE(right))                                                                                                                                                          \
			PUSH_STACK(CYS_TO_INT_VALUE(left) op CYS_TO_REAL_VALUE(right));                                                                                                                                                   \
		else if (CYS_IS_REAL_VALUE(left) && CYS_IS_INT_VALUE(right))                                                                                                                                                          \
			PUSH_STACK(CYS_TO_REAL_VALUE(left) op CYS_TO_INT_VALUE(right));                                                                                                                                                   \
		else                                                                                                                                                                                                                  \
			CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid binary op:{}{}{},only (&)int-(&)int,(&)real-(&)real,(&)int-(&)real or (&)real-(&)int type pair is available."), left.ToString(), TEXT(#op), right.ToString()); \
	} while (0);

// & | << >>
#define INTEGER_BINARY(op)                                                                                                                                                   \
	do                                                                                                                                                                       \
	{                                                                                                                                                                        \
		Value right = POP_STACK();                                                                                                                                           \
		Value left = POP_STACK();                                                                                                                                            \
		if (CYS_IS_REF_VALUE(left))                                                                                                                                          \
			left = *CYS_TO_REF_VALUE(left)->pointer;                                                                                                                         \
		if (CYS_IS_REF_VALUE(right))                                                                                                                                         \
			right = *CYS_TO_REF_VALUE(right)->pointer;                                                                                                                       \
		if (CYS_IS_INT_VALUE(left) && CYS_IS_INT_VALUE(right))                                                                                                               \
			PUSH_STACK(CYS_TO_INT_VALUE(left) op CYS_TO_INT_VALUE(right));                                                                                                   \
		else                                                                                                                                                                 \
			CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid binary op:{}{}{},only (&)int-(&)int type pair is available."), left.ToString(), TEXT(#op), right.ToString()); \
	} while (0);

// > <
#define COMPARE_BINARY(op)                                                                  \
	do                                                                                      \
	{                                                                                       \
		Value right = POP_STACK();                                                          \
		Value left = POP_STACK();                                                           \
		if (CYS_IS_REF_VALUE(left))                                                         \
			left = *CYS_TO_REF_VALUE(left)->pointer;                                        \
		if (CYS_IS_REF_VALUE(right))                                                        \
			right = *CYS_TO_REF_VALUE(right)->pointer;                                      \
		if (CYS_IS_INT_VALUE(left) && CYS_IS_INT_VALUE(right))                              \
			PUSH_STACK(CYS_TO_INT_VALUE(left) op CYS_TO_INT_VALUE(right) ? true : false);   \
		else if (CYS_IS_REAL_VALUE(left) && CYS_IS_REAL_VALUE(right))                       \
			PUSH_STACK(CYS_TO_REAL_VALUE(left) op CYS_TO_REAL_VALUE(right) ? true : false); \
		else if (CYS_IS_INT_VALUE(left) && CYS_IS_REAL_VALUE(right))                        \
			PUSH_STACK(CYS_TO_INT_VALUE(left) op CYS_TO_REAL_VALUE(right) ? true : false);  \
		else if (CYS_IS_REAL_VALUE(left) && CYS_IS_INT_VALUE(right))                        \
			PUSH_STACK(CYS_TO_REAL_VALUE(left) op CYS_TO_INT_VALUE(right) ? true : false);  \
		else                                                                                \
			PUSH_STACK(false);                                                              \
	} while (0);

// && ||
#define LOGIC_BINARY(op)                                                                                                                                                       \
	do                                                                                                                                                                         \
	{                                                                                                                                                                          \
		Value right = POP_STACK();                                                                                                                                             \
		Value left = POP_STACK();                                                                                                                                              \
		if (CYS_IS_REF_VALUE(left))                                                                                                                                            \
			left = *CYS_TO_REF_VALUE(left)->pointer;                                                                                                                           \
		if (CYS_IS_REF_VALUE(right))                                                                                                                                           \
			right = *CYS_TO_REF_VALUE(right)->pointer;                                                                                                                         \
		if (CYS_IS_BOOL_VALUE(left) && CYS_IS_BOOL_VALUE(right))                                                                                                               \
			PUSH_STACK(CYS_TO_BOOL_VALUE(left) op CYS_TO_BOOL_VALUE(right) ? Value(true) : Value(false));                                                                      \
		else                                                                                                                                                                   \
			CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid binary op:{}{}{},only (&)bool-(&)bool type pair is available."), left.ToString(), TEXT(#op), right.ToString()); \
	} while (0);

#define READ_INS() (*frame->ip++)

#define CHECK_IDX_RANGE(v, idx)                 \
	if (idx < 0 || idx >= (uint64_t)(v).size()) \
		CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Idx out of range."));

#define CHECK_IDX_VALID(idxValue)    \
	if (!CYS_IS_INT_VALUE(idxValue)) \
		CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid idx type for array or string,only integer is available."));

		while (1)
		{
			if (CYS_IS_CALL_FRAME_STACK_EMPTY())
				return;
			CallFrame *frame = PEEK_CALL_FRAME(0);

			auto instruction = READ_INS();
			auto relatedToken = frame->closure->function->chunk.opCodeRelatedTokens[READ_INS()];
			switch (instruction)
			{
			case OP_RETURN:
			{
				auto retCount = READ_INS();
				Value *retValues = STACK_TOP() - retCount;

				CLOSED_UPVALUES(frame->slots);

				if (CYS_IS_CALL_FRAME_STACK_EMPTY())
					return;

				SET_STACK_TOP(frame->slots);

				if (retCount == 0)
				{
					// ++ Function cache relative
					if (Config::GetInstance()->IsUseFunctionCache())
					{
						auto callFrameTop = PEEK_CALL_FRAME(0);
						callFrameTop->closure->function->SetCache(callFrameTop->argumentsHash, {Value()});
					}
					// -- Function cache relative
					PUSH_STACK(Value());
				}
				else
				{
					// ++ Function cache relative
					if (Config::GetInstance()->IsUseFunctionCache())
					{
						auto callFrameTop = PEEK_CALL_FRAME(0);
						std::vector<Value> rets(retValues, retValues + retCount);
						callFrameTop->closure->function->SetCache(callFrameTop->argumentsHash, rets);
					}
					// ++ Function cache relative
					for (uint8_t i = 0; i < retCount; ++i)
					{
						auto value = *(retValues + i);
						PUSH_STACK(value);
					}
				}

				frame = POP_CALL_FRAME();
				break;
			}
			case OP_CONSTANT:
			{
				auto pos = READ_INS();
				auto v = frame->closure->function->chunk.constants[pos];
				PUSH_STACK(v);
				break;
			}
			case OP_NULL:
			{
				PUSH_STACK(Value());
				break;
			}
			case OP_SET_GLOBAL:
			{
				auto pos = READ_INS();
				auto v = PEEK_STACK(0);

				auto globalValue = GET_GLOBAL_VARIABLE(pos);

				if (CYS_IS_REF_VALUE(*globalValue))
					*CYS_TO_REF_VALUE(*globalValue)->pointer = v;
				else
					*globalValue = v;
				break;
			}
			case OP_GET_GLOBAL:
			{
				auto pos = READ_INS();
				PUSH_STACK(*GET_GLOBAL_VARIABLE(pos));
				break;
			}
			case OP_SET_LOCAL:
			{
				auto pos = READ_INS();
				auto value = PEEK_STACK(0);

				auto slot = frame->slots + pos;

				if (CYS_IS_REF_VALUE((*slot)))
					*CYS_TO_REF_VALUE((*slot))->pointer = value;
				else
					*slot = value; // now assume base ptr on the stack bottom
				break;
			}
			case OP_GET_LOCAL:
			{
				auto pos = READ_INS();
				PUSH_STACK(frame->slots[pos]); // now assume base ptr on the stack bottom
				break;
			}
			case OP_SET_UPVALUE:
			{
				auto pos = READ_INS();
				auto v = PEEK_STACK(0);
				*frame->closure->upvalues[pos]->location = PEEK_STACK(0);
				break;
			}
			case OP_GET_UPVALUE:
			{
				auto pos = READ_INS();
				PUSH_STACK(*frame->closure->upvalues[pos]->location);
				break;
			}
			case OP_CLOSE_UPVALUE:
			{
				CLOSED_UPVALUES(STACK_TOP() - 1);
				POP_STACK();
				break;
			}
			case OP_ADD:
			{
				Value left = PEEK_STACK(0);
				Value right = PEEK_STACK(1);
				Value result;
				if (CYS_IS_REF_VALUE(left))
					left = *CYS_TO_REF_VALUE(left)->pointer;
				if (CYS_IS_REF_VALUE(right))
					right = *CYS_TO_REF_VALUE(right)->pointer;
				if (CYS_IS_INT_VALUE(left) && CYS_IS_INT_VALUE(right))
					result = CYS_TO_INT_VALUE(left) + CYS_TO_INT_VALUE(right);
				else if (CYS_IS_REAL_VALUE(left) && CYS_IS_REAL_VALUE(right))
					result = CYS_TO_REAL_VALUE(left) + CYS_TO_REAL_VALUE(right);
				else if (CYS_IS_INT_VALUE(left) && CYS_IS_REAL_VALUE(right))
					result = CYS_TO_INT_VALUE(left) + CYS_TO_REAL_VALUE(right);
				else if (CYS_IS_REAL_VALUE(left) && CYS_IS_INT_VALUE(right))
					result = CYS_TO_REAL_VALUE(left) + CYS_TO_INT_VALUE(right);
				else if (CYS_IS_STR_VALUE(left) && CYS_IS_STR_VALUE(right))
					result = Allocator::GetInstance()->CreateObject<StrObject>(CYS_TO_STR_VALUE(left)->value + CYS_TO_STR_VALUE(right)->value);
				else
					CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid binary op:{}+{},only (&)int-(&)int,(&)real-(&)real,(&)int-(&)real or (&)real-(&)int type pair is available."), left.ToString(), right.ToString());

				MOVE_STACK_TOP(-2);
				PUSH_STACK(result);

				break;
			}
			case OP_SUB:
			{
				COMMON_BINARY(-);
				break;
			}
			case OP_MUL:
			{
				COMMON_BINARY(*);
				break;
			}
			case OP_DIV:
			{
				COMMON_BINARY(/);
				break;
			}
			case OP_MOD:
			{
				INTEGER_BINARY(%);
				break;
			}
			case OP_BIT_AND:
			{
				INTEGER_BINARY(&);
				break;
			}
			case OP_BIT_OR:
			{
				INTEGER_BINARY(|);
				break;
			}
			case OP_BIT_LEFT_SHIFT:
			{
				INTEGER_BINARY(<<);
				break;
			}
			case OP_BIT_RIGHT_SHIFT:
			{
				INTEGER_BINARY(>>);
				break;
			}
			case OP_LESS:
			{
				COMPARE_BINARY(<);
				break;
			}
			case OP_GREATER:
			{
				COMPARE_BINARY(>);
				break;
			}
			case OP_NOT:
			{
				auto value = POP_STACK();
				if (CYS_IS_REF_VALUE(value))
					value = *CYS_TO_REF_VALUE(value)->pointer;
				if (!CYS_IS_BOOL_VALUE(value))
					CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid op:!{}, only bool type is available."), value.ToString());
				PUSH_STACK(!CYS_TO_BOOL_VALUE(value));
				break;
			}
			case OP_EQUAL:
			{
				Value left = POP_STACK();
				Value right = POP_STACK();
				if (CYS_IS_REF_VALUE(left))
					left = *CYS_TO_REF_VALUE(left)->pointer;
				if (CYS_IS_REF_VALUE(right))
					right = *CYS_TO_REF_VALUE(right)->pointer;
				PUSH_STACK(left == right);
				break;
			}
			case OP_MINUS:
			{
				auto value = POP_STACK();
				if (CYS_IS_REF_VALUE(value))
					value = *CYS_TO_REF_VALUE(value)->pointer;
				if (CYS_IS_INT_VALUE(value))
					PUSH_STACK(-CYS_TO_INT_VALUE(value));
				else if (CYS_IS_REAL_VALUE(value))
					PUSH_STACK(-CYS_TO_REAL_VALUE(value));
				else
					CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid op:-{}, only -(int||real expr) is available."), value.ToString());
				break;
			}
			case OP_FACTORIAL:
			{
				auto value = POP_STACK();
				if (CYS_IS_REF_VALUE(value))
					value = *CYS_TO_REF_VALUE(value)->pointer;
				if (CYS_IS_INT_VALUE(value))
					PUSH_STACK(Factorial(CYS_TO_INT_VALUE(value)));
				else
					CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid op:{}!, only (int expr)! is available."), value.ToString());
				break;
			}
			case OP_ARRAY:
			{
				auto count = READ_INS();

				std::vector<Value> elements(count);
				size_t i = 0;
				for (auto e = STACK_TOP() - count; e < STACK_TOP(); ++e, ++i)
					elements[i] = *e;

				auto arrayObject = Allocator::GetInstance()->CreateObject<ArrayObject>(elements);

				MOVE_STACK_TOP(-count);

				PUSH_STACK(arrayObject);
				break;
			}
			case OP_DICT:
			{
				auto count = READ_INS();
				ValueUnorderedMap elements;

				auto dict = Allocator::GetInstance()->CreateObject<DictObject>(elements);

				for (auto e = STACK_TOP() - count * 2; e < STACK_TOP(); e += 2)
				{
					auto key = *e;
					auto value = *(e + 1);
					dict->elements[key] = value;
				}

				MOVE_STACK_TOP(-count * 2);

				PUSH_STACK(dict);
				break;
			}
			case OP_GET_INDEX:
			{
				auto idxValue = POP_STACK();
				auto dsValue = POP_STACK();
				if (CYS_IS_ARRAY_VALUE(dsValue))
				{
					auto array = CYS_TO_ARRAY_VALUE(dsValue);
					CHECK_IDX_VALID(idxValue);

					auto intIdx = NormalizeIdx(CYS_TO_INT_VALUE(idxValue), array->elements.size());
					CHECK_IDX_RANGE(array->elements, intIdx);

					PUSH_STACK(array->elements[intIdx]);
				}
				else if (CYS_IS_STR_VALUE(dsValue))
				{
					auto strObj = CYS_TO_STR_VALUE(dsValue);
					CHECK_IDX_VALID(idxValue)
					auto intIdx = NormalizeIdx(CYS_TO_INT_VALUE(idxValue), strObj->value.size());
					CHECK_IDX_RANGE(strObj->value, intIdx);
					PUSH_STACK(Allocator::GetInstance()->CreateObject<StrObject>(strObj->value.substr(intIdx, 1)));
				}
				else if (CYS_IS_DICT_VALUE(dsValue))
				{
					auto dict = CYS_TO_DICT_VALUE(dsValue);

					auto iter = dict->elements.find(idxValue);

					if (iter != dict->elements.end())
						PUSH_STACK(iter->second);
					else
						CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("No key in dict"));
				}
				break;
			}
			case OP_SET_INDEX:
			{
				auto idxValue = POP_STACK();
				auto dsValue = POP_STACK();
				auto newValue = PEEK_STACK(0);
				if (CYS_IS_ARRAY_VALUE(dsValue))
				{
					auto array = CYS_TO_ARRAY_VALUE(dsValue);
					CHECK_IDX_VALID(idxValue);
					auto intIdx = NormalizeIdx(CYS_TO_INT_VALUE(idxValue), array->elements.size());
					CHECK_IDX_RANGE(array->elements, intIdx);
					array->elements[intIdx] = newValue;
				}
				else if (CYS_IS_STR_VALUE(dsValue))
				{
					auto strObj = CYS_TO_STR_VALUE(dsValue);
					CHECK_IDX_VALID(idxValue)
					auto intIdx = NormalizeIdx(CYS_TO_INT_VALUE(idxValue), strObj->value.size());
					CHECK_IDX_RANGE(strObj->value, intIdx)

					if (!CYS_IS_STR_VALUE(newValue))
						CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Cannot insert a non string clip:{} to string:{}"), newValue.ToString(), strObj->value);

					strObj->value.append(CYS_TO_STR_VALUE(newValue)->value, intIdx, CYS_TO_STR_VALUE(newValue)->value.size());
				}
				else if (CYS_IS_DICT_VALUE(dsValue))
				{
					auto dict = CYS_TO_DICT_VALUE(dsValue);
					dict->elements[idxValue] = newValue;
				}
				break;
			}
			case OP_POP:
			{
				POP_STACK();
				break;
			}
			case OP_JUMP_IF_FALSE:
			{
				uint16_t address = (*(frame->ip++) << 8) | (*(frame->ip++));
				if (IsFalsey(PEEK_STACK(0)))
					frame->ip += address;
				break;
			}
			case OP_JUMP:
			{
				uint16_t address = (*(frame->ip++) << 8) | (*(frame->ip++));
				frame->ip += address;
				break;
			}
			case OP_LOOP:
			{
				uint16_t address = (*(frame->ip++) << 8) | (*(frame->ip++));
				frame->ip -= address;
				break;
			}
			case OP_REF_GLOBAL:
			{
				auto index = READ_INS();
				PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(GET_GLOBAL_VARIABLE(index)));
				break;
			}
			case OP_REF_LOCAL:
			{
				auto index = READ_INS();
				PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(frame->slots + index));
				break;
			}
			case OP_REF_UPVALUE:
			{
				auto index = READ_INS();
				PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(frame->closure->upvalues[index]->location));
				break;
			}
			case OP_REF_INDEX_GLOBAL:
			{
				auto index = READ_INS();
				auto idxValue = POP_STACK();

				auto globalValue = GET_GLOBAL_VARIABLE(index);

				if (CYS_IS_DICT_VALUE(*globalValue))
					PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(&CYS_TO_DICT_VALUE(*globalValue)->elements[idxValue]));
				else if (CYS_IS_ARRAY_VALUE(*globalValue))
				{
					auto array = CYS_TO_ARRAY_VALUE(*globalValue);
					CHECK_IDX_VALID(idxValue)
					auto intIdx = NormalizeIdx(CYS_TO_INT_VALUE(idxValue), array->elements.size());
					CHECK_IDX_RANGE(array->elements, intIdx);
					PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(&(array->elements[intIdx])));
				}
				else
					CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid indexed reference type:{} not a dict or array value."), globalValue->ToString());
				break;
			}
			case OP_REF_INDEX_LOCAL:
			{
				auto index = READ_INS();
				auto idxValue = POP_STACK();
				Value *v = frame->slots + index;
				if (CYS_IS_DICT_VALUE((*v)))
					PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(&CYS_TO_DICT_VALUE((*v))->elements[idxValue]));
				else if (CYS_IS_ARRAY_VALUE((*v)))
				{
					auto array = CYS_TO_ARRAY_VALUE((*v));
					CHECK_IDX_VALID(idxValue)
					auto intIdx = NormalizeIdx(CYS_TO_INT_VALUE(idxValue), array->elements.size());
					CHECK_IDX_RANGE(array->elements, intIdx);
					PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(&array->elements[intIdx]));
				}
				else
					CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid indexed reference type:{} not a dict or array value."), v->ToString());
				break;
			}
			case OP_REF_INDEX_UPVALUE:
			{
				auto index = READ_INS();
				auto idxValue = POP_STACK();
				Value *v = frame->closure->upvalues[index]->location;
				if (CYS_IS_DICT_VALUE((*v)))
					PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(&CYS_TO_DICT_VALUE((*v))->elements[idxValue]));
				else if (CYS_IS_ARRAY_VALUE((*v)))
				{
					auto array = CYS_TO_ARRAY_VALUE((*v));
					CHECK_IDX_VALID(idxValue)
					auto intIdx = NormalizeIdx(CYS_TO_INT_VALUE(idxValue), array->elements.size());
					CHECK_IDX_RANGE(array->elements, intIdx)
					PUSH_STACK(Allocator::GetInstance()->CreateObject<RefObject>(&array->elements[intIdx]));
				}
				else
					CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid indexed reference type: {}  not a dict or array value."), v->ToString());
				break;
			}
			case OP_CALL:
			{
				auto argCount = READ_INS();
				auto callee = PEEK_STACK(argCount);
				if (CYS_IS_CLOSURE_VALUE(callee) || CYS_IS_CLASS_CLOSURE_BIND_VALUE(callee)) // normal function or class member function
				{
					if (CYS_IS_CLASS_CLOSURE_BIND_VALUE(callee))
					{
						auto binding = CYS_TO_CLASS_CLOSURE_BIND_VALUE(callee);

						SET_VALUE_FROM_STACK_TOP_OFFSET(-(argCount + 1), binding->receiver);
						callee = binding->closure;
					}

					if (CYS_TO_CLOSURE_VALUE(callee)->function->varArg > VarArg::NONE)
					{
						auto arity = CYS_TO_CLOSURE_VALUE(callee)->function->arity;
						if (argCount < arity)
						{
							if (argCount == arity - 1)
							{
								if (CYS_TO_CLOSURE_VALUE(callee)->function->varArg == VarArg::WITH_NAME)
								{
									PUSH_STACK(new ArrayObject());
									argCount = arity;
								}
								else
									argCount = arity - 1;
							}
							else
								CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("No matching argument count."));
						}
						else if (argCount >= arity)
						{
							auto diff = argCount - arity + 1;
							if (CYS_TO_CLOSURE_VALUE(callee)->function->varArg == VarArg::WITH_NAME)
							{
								std::vector<Value> varArgs;
								for (int32_t i = 0; i < diff; ++i)
									varArgs.insert(varArgs.begin(), POP_STACK());
								PUSH_STACK(new ArrayObject(varArgs));
								argCount = arity;
							}
							else
							{
								for (int32_t i = 0; i < diff; ++i)
									POP_STACK();
								argCount = arity - 1;
							}
						}
					}
					else if (argCount != CYS_TO_CLOSURE_VALUE(callee)->function->arity)
						CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("No matching argument count."));

					auto argsHash = HashValueList(STACK_TOP() - argCount, STACK_TOP());
					std::vector<Value> rets;
					// ++ Function cache relative
					if (Config::GetInstance()->IsUseFunctionCache() && CYS_TO_CLOSURE_VALUE(callee)->function->GetCache(argsHash, rets))
					{
						MOVE_STACK_TOP(-(argCount + 1));
						for (int32_t i = 0; i < rets.size(); ++i)
							PUSH_STACK(rets[i]);
					}
					else
					// -- Function cache relative
					{
						// init a new frame
						CallFrame newframe;
						newframe.closure = CYS_TO_CLOSURE_VALUE(callee);
						newframe.ip = newframe.closure->function->chunk.opCodes.data();
						newframe.slots = STACK_TOP() - argCount - 1;
						// ++ Function cache relative
						if (Config::GetInstance()->IsUseFunctionCache())
						{
							newframe.argumentsHash = argsHash;
						}
						// -- Function cache relative
						PUSH_CALL_FRAME(newframe);
					}
				}
				else if (CYS_IS_CLASS_INSTANCE_VALUE(callee)) // class constructor(for initializing class instance)
				{
					auto classInstance = CYS_TO_CLASS_INSTANCE_VALUE(callee);
					auto klass = classInstance->klass;

					auto iter = klass->constructors.find(argCount);
					if (iter == klass->constructors.end())
						CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Not matching argument count of class: {}'s constructors."), klass->name);

					auto ctor = iter->second;
					// init a new frame
					CallFrame newframe;
					newframe.closure = ctor;
					newframe.ip = newframe.closure->function->chunk.opCodes.data();
					newframe.slots = STACK_TOP() - argCount - 1;

					PUSH_CALL_FRAME(newframe);
				}
				else if (CYS_IS_NATIVE_FUNCTION_VALUE(callee)) // native function
				{

					Value result;
					auto hasRetV = CYS_TO_NATIVE_FUNCTION_VALUE(callee)->fn(STACK_TOP() - argCount, argCount, relatedToken, result);

					MOVE_STACK_TOP(-(argCount + 1));

					if (hasRetV)
						PUSH_STACK(result);
					else
						PUSH_STACK(Value());
				}
				else
					CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid callee,Only function is available: {}"), callee.ToString());
				break;
			}
			case OP_CLASS:
			{
				auto name = PEEK_STACK(0);
				auto constructorCount = READ_INS();
				auto parentClassCount = READ_INS();
				auto varCount = READ_INS();
				auto constCount = READ_INS();
				auto fnCount = READ_INS();
				auto enumCount = READ_INS();

				auto classObj = Allocator::GetInstance()->CreateObject<ClassObject>();

				classObj->name = CYS_TO_STR_VALUE(name)->value;
				POP_STACK(); // pop name strobject

				for (int32_t i = 0; i < constructorCount; ++i)
				{
					auto v = CYS_TO_CLOSURE_VALUE(POP_STACK());
					classObj->constructors[v->function->arity] = v;
				}

				for (int32_t i = 0; i < parentClassCount; ++i)
				{
					name = POP_STACK();
					auto parentClass = POP_STACK();
					classObj->parents[CYS_TO_STR_VALUE(name)->value] = CYS_TO_CLASS_VALUE(parentClass);
				}

				for (int32_t i = 0; i < varCount; ++i)
				{
					name = POP_STACK();
					auto v = POP_STACK();
					v.permission = Permission::MUTABLE;
					classObj->defaultMembers[CYS_TO_STR_VALUE(name)->value] = v;
				}

				for (int32_t i = 0; i < constCount; ++i)
				{
					name = POP_STACK();
					auto v = POP_STACK();
					v.permission = Permission::IMMUTABLE;
					classObj->defaultMembers[CYS_TO_STR_VALUE(name)->value] = v;
				}

				for (int32_t i = 0; i < fnCount; ++i)
				{
					name = POP_STACK();
					auto v = POP_STACK();
					v.permission = Permission::IMMUTABLE;
					classObj->functions[CYS_TO_STR_VALUE(name)->value] = v;
				}

				for (int32_t i = 0; i < enumCount; ++i)
				{
					name = POP_STACK();
					auto v = POP_STACK();
					v.permission = Permission::IMMUTABLE;
					classObj->enums[CYS_TO_STR_VALUE(name)->value] = v;
				}

				PUSH_STACK(classObj);
				break;
			}
			case OP_CLASS_INSTANCE:
			{
				auto classObject = CYS_TO_CLASS_VALUE(POP_STACK());

				auto instance = Allocator::GetInstance()->CreateObject<ClassInstanceObject>(classObject);

				PUSH_STACK(instance);
				break;
			}
			case OP_STRUCT:
			{
				auto eCount = READ_INS();
				auto structObj = Allocator::GetInstance()->CreateObject<StructObject>();
				for (int64_t i = 0; i < (int64_t)eCount; ++i)
				{
					auto key = CYS_TO_STR_VALUE(POP_STACK())->value;
					auto value = POP_STACK();
					structObj->elements[key] = value;
				}
				PUSH_STACK(structObj);
				break;
			}
			case OP_GET_PROPERTY:
			{
				auto peekValue = PEEK_STACK(1);

				if (CYS_IS_REF_VALUE(peekValue))
					peekValue = *(CYS_TO_REF_VALUE(peekValue)->pointer);

				auto propName = CYS_TO_STR_VALUE(POP_STACK())->value;
				if (CYS_IS_CLASS_VALUE(peekValue))
				{
					ClassObject *klass = CYS_TO_CLASS_VALUE(peekValue);

					Value member;
					if (klass->GetMember(propName, member))
					{
						POP_STACK(); // pop class object
						if (CYS_IS_CLOSURE_VALUE(member))
							member = Allocator::GetInstance()->CreateObject<ClassClosureBindObject>(klass, CYS_TO_CLOSURE_VALUE(member));

						PUSH_STACK(member);
						break;
					}
					else
						CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("No member: {} in class object:{}"), propName, klass->name);
				}
				else if (CYS_IS_CLASS_INSTANCE_VALUE(peekValue))
				{
					ClassInstanceObject *classInstance = CYS_TO_CLASS_INSTANCE_VALUE(peekValue);

					Value member;
					if (classInstance->GetMember(propName, member))
					{
						POP_STACK(); // pop class object
						if (CYS_IS_CLOSURE_VALUE(member))
							member = Allocator::GetInstance()->CreateObject<ClassClosureBindObject>(classInstance, CYS_TO_CLOSURE_VALUE(member));

						PUSH_STACK(member);
						break;
					}
					else
						CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("No member: {} in class object:{}"), propName, classInstance->klass->name);
				}
				else if (CYS_IS_ENUM_VALUE(peekValue))
				{
					EnumObject *enumObj = CYS_TO_ENUM_VALUE(peekValue);

					Value member;
					if (enumObj->GetMember(propName, member))
					{
						POP_STACK(); // pop enum object
						PUSH_STACK(member);
						break;
					}
					else
						CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("No member: {} in enum object: {}"), propName, enumObj->name);
				}
				else if (CYS_IS_STRUCT_VALUE(peekValue))
				{
					auto structObj = CYS_TO_STRUCT_VALUE(peekValue);
					auto iter = structObj->elements.find(propName);
					if (iter == structObj->elements.end())
						CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("No property: {} in struct object:{}."), propName, structObj->ToString());
					POP_STACK(); // pop struct object
					PUSH_STACK(iter->second);
					break;
				}
				else if (CYS_IS_MODULE_VALUE(peekValue))
				{
					auto moduleObj = CYS_TO_MODULE_VALUE(peekValue);
					Value member;
					if (moduleObj->GetMember(propName, member))
					{
						POP_STACK(); // pop module object
						PUSH_STACK(member);
						break;
					}
					else
						CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("No member: {} in module: {}"), propName, moduleObj->name);
				}
				else
					CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid call:not a valid class,enum or struct object instance: {}"), peekValue.ToString());

				break;
			}
			case OP_SET_PROPERTY:
			{
				auto peekValue = PEEK_STACK(1);

				if (CYS_IS_REF_VALUE(peekValue))
					peekValue = *(CYS_TO_REF_VALUE(peekValue)->pointer);

				auto propName = CYS_TO_STR_VALUE(POP_STACK())->value;
				if (CYS_IS_CLASS_VALUE(peekValue))
				{
					auto klass = CYS_TO_CLASS_VALUE(peekValue);
					POP_STACK(); // pop class value

					Value member;
					if (klass->GetMember(propName, member))
					{
						if (member.permission == Permission::IMMUTABLE)
							CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Constant cannot be assigned twice: {}'s member: {} is a constant value"), klass->name, propName);
						else
							klass->defaultMembers[propName] = PEEK_STACK(0);
					}
					else
						CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("No member named: {} in class: {}"), propName, klass->name);
				}
				else if (CYS_IS_CLASS_INSTANCE_VALUE(peekValue))
				{
					auto classInstance = CYS_TO_CLASS_INSTANCE_VALUE(peekValue);
					POP_STACK(); // pop class value

					Value member;
					if (classInstance->GetMember(propName, member))
					{
						if (member.permission == Permission::IMMUTABLE)
							CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Constant cannot be assigned twice: {}'s member: {} is a constant value"), classInstance->klass->name, propName);
						else
							classInstance->members[propName] = PEEK_STACK(0);
					}
					else
						CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("No member named: {} in class: {}"), propName, classInstance->klass->name);
				}
				else if (CYS_IS_STRUCT_VALUE(peekValue))
				{
					auto structObj = CYS_TO_STRUCT_VALUE(peekValue);
					auto iter = structObj->elements.find(propName);
					if (iter == structObj->elements.end())
						CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("No property: {} in struct object:{}"), propName, structObj->ToString());
					POP_STACK(); // pop struct object
					structObj->elements[iter->first] = PEEK_STACK(0);
					break;
				}
				else if (CYS_IS_ENUM_VALUE(peekValue))
					CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid call:cannot assign value to a enum object member."));
				else
					CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid call:not a valid class or struct object instance."));
				break;
			}
			case OP_GET_BASE:
			{
				if (!CYS_IS_CLASS_VALUE(PEEK_STACK(1)))
					CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("Invalid class call:not a valid class instance."));
				auto propName = CYS_TO_STR_VALUE(POP_STACK())->value;
				auto klass = CYS_TO_CLASS_VALUE(POP_STACK());
				Value member;
				bool hasValue = klass->GetParentMember(propName, member);
				if (!hasValue)
					CYS_LOG_ERROR_WITH_LOC(relatedToken, TEXT("No member: {} in class: {}'s parent class(es)."), propName, klass->name);
				PUSH_STACK(member);
				break;
			}
			case OP_CLOSURE:
			{
				auto pos = READ_INS();
				auto func = CYS_TO_FUNCTION_VALUE(frame->closure->function->chunk.constants[pos]);

				PUSH_STACK(func); // push function object for avoiding gc
				auto closure = Allocator::GetInstance()->CreateObject<ClosureObject>(func);
				POP_STACK(); // pop function object

				PUSH_STACK(closure);

				for (int32_t i = 0; i < closure->upvalues.size(); ++i)
				{
					auto index = READ_INS();
					auto depth = READ_INS();
					if (depth == CALL_FRAME_COUNT() - 1)
					{
						auto captured = CAPTURE_UPVALUE(frame->slots + index);
						closure->upvalues[i] = captured;
					}
					else
						closure->upvalues[i] = frame->closure->upvalues[index];
				}

				break;
			}
			case OP_APPREGATE_RESOLVE:
			{
				auto count = READ_INS();
				auto value = POP_STACK();
				if (CYS_IS_ARRAY_VALUE(value))
				{
					auto arrayObj = CYS_TO_ARRAY_VALUE(value);
					if (count >= arrayObj->elements.size())
					{
						auto diff = count - arrayObj->elements.size();
						while (diff > 0)
						{
							PUSH_STACK(Value());
							diff--;
						}
						for (int32_t i = static_cast<int32_t>(arrayObj->elements.size() - 1); i >= 0; --i)
							PUSH_STACK(arrayObj->elements[i]);
					}
					else
					{
						for (int32_t i = count - 1; i >= 0; --i)
							PUSH_STACK(arrayObj->elements[i]);
					}
				}
				else
				{
					auto diff = count - 1;
					while (diff > 0)
					{
						PUSH_STACK(Value());
						diff--;
					}

					PUSH_STACK(value);
				}
				break;
			}
			case OP_APPREGATE_RESOLVE_VAR_ARG:
			{
				auto count = READ_INS();
				auto value = PEEK_STACK(0);
				if (CYS_IS_ARRAY_VALUE(value))
				{
					auto arrayObj = CYS_TO_ARRAY_VALUE(value);
					if (count >= arrayObj->elements.size())
					{
						ArrayObject *varArgArray = Allocator::GetInstance()->CreateObject<ArrayObject>();

						POP_STACK(); // pop value object

						auto diff = count - arrayObj->elements.size();
						for (int32_t i = static_cast<int32_t>(diff); i > 0; --i)
						{
							if (i == diff)
								PUSH_STACK(varArgArray);
							else
								PUSH_STACK(Value());
						}

						for (int32_t i = static_cast<int32_t>(arrayObj->elements.size() - 1); i >= 0; --i)
							PUSH_STACK(arrayObj->elements[i]);
					}
					else
					{
						ArrayObject *varArgArray = Allocator::GetInstance()->CreateObject<ArrayObject>();

						POP_STACK(); // pop value object

						for (int32_t i = count - 1; i < arrayObj->elements.size(); ++i)
							varArgArray->elements.emplace_back(arrayObj->elements[i]);
						PUSH_STACK(varArgArray);

						for (int32_t i = count - 2; i >= 0; --i)
							PUSH_STACK(arrayObj->elements[i]);
					}
				}
				else
				{
					auto arrayObj = Allocator::GetInstance()->CreateObject<ArrayObject>();

					POP_STACK(); // pop value object

					auto diff = count - 2;
					while (diff > 0)
					{
						PUSH_STACK(Value());
						diff--;
					}

					PUSH_STACK(arrayObj);
					PUSH_STACK(value);
				}
				break;
			}
			case OP_MODULE:
			{
				auto name = PEEK_STACK(0);
				auto nameStr = CYS_TO_STR_VALUE(name)->value;

				auto varCount = READ_INS();
				auto constCount = READ_INS();

				auto moduleObj = Allocator::GetInstance()->CreateObject<ModuleObject>();
				moduleObj->name = nameStr;
				POP_STACK(); // pop name strobject

				for (int32_t i = 0; i < constCount; ++i)
				{
					name = POP_STACK();
					nameStr = CYS_TO_STR_VALUE(name)->value;
					auto v = POP_STACK();
					v.permission = Permission::IMMUTABLE;
					moduleObj->members[nameStr] = v;
				}

				for (int32_t i = 0; i < varCount; ++i)
				{
					name = POP_STACK();
					nameStr = CYS_TO_STR_VALUE(name)->value;
					auto v = POP_STACK();
					v.permission = Permission::MUTABLE;
					moduleObj->members[nameStr] = v;
				}

				PUSH_STACK(moduleObj);

				break;
			}
			case OP_INIT_VAR_ARG:
			{
				auto count = READ_INS();
				std::vector<Value> values(count);
				std::vector<Value> keys(count);
				for (int32_t i = count - 1; i >= 0; --i)
					keys[i] = POP_STACK();
				for (uint32_t i = 0; i < count; ++i)
					values[i] = POP_STACK();
				for (uint32_t i = 0; i < count; ++i)
				{
					PUSH_STACK(values[i]);
					PUSH_STACK(keys[i]);
				}
				break;
			}
			default:
				break;
			}
		}
	}

	bool VM::IsFalsey(const Value &v) noexcept
	{
		return CYS_IS_NULL_VALUE(v) || (CYS_IS_BOOL_VALUE(v) && !CYS_TO_BOOL_VALUE(v));
	}
}