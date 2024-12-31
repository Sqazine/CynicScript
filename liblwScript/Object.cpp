#include "Object.h"
#include "Chunk.h"
#include "Utils.h"
#include "Logger.h"
#include "Allocator.h"
namespace lwscript
{

	Object::Object(ObjectKind kind)
		: kind(kind), marked(false), next(nullptr)
	{
	}
	Object::~Object()
	{
	}

	void Object::Mark()
	{
		if (marked)
			return;
#ifdef GC_DEBUG
		Logger::Info(TEXT("(0x{}) mark: {}"), (void *)this, ToString());
#endif
		marked = true;
		Allocator::GetInstance()->mGrayObjects.emplace_back(this);
	}
	void Object::UnMark()
	{
		if (!marked)
			return;
#ifdef GC_DEBUG
		Logger::Info(TEXT("(0x{}) unMark: {}"), (void *)this, ToString());
#endif
		marked = false;
	}

	void Object::Blacken()
	{
#ifdef GC_DEBUG
		Logger::Info(TEXT("(0x{}) blacken: {}"), (void *)this, ToString());
#endif
	}

	StrObject::StrObject(STD_STRING_VIEW value)
		: Object(ObjectKind::STR), value(value)
	{
	}
	StrObject::~StrObject()
	{
	}
	STD_STRING StrObject::ToString() const
	{
		return value;
	}

	bool StrObject::IsEqualTo(Object *other)
	{
		if (!IS_STR_OBJ(other))
			return false;
		return value == TO_STR_OBJ(other)->value;
	}

	std::vector<uint8_t> StrObject::Serialize() const
	{
		return std::vector<uint8_t>();
	}

	ArrayObject::ArrayObject()
		: Object(ObjectKind::ARRAY)
	{
	}
	ArrayObject::ArrayObject(const std::vector<Value> &elements)
		: Object(ObjectKind::ARRAY), elements(elements)
	{
	}
	ArrayObject::~ArrayObject()
	{
	}

	STD_STRING ArrayObject::ToString() const
	{
		STD_STRING result = TEXT("[");
		if (!elements.empty())
		{
			for (const auto &e : elements)
				result += e.ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT("]");
		return result;
	}

	void ArrayObject::Blacken()
	{
		Object::Blacken();
		for (auto &e : elements)
			e.Mark();
	}

	bool ArrayObject::IsEqualTo(Object *other)
	{
		if (!IS_ARRAY_OBJ(other))
			return false;

		ArrayObject *arrayOther = TO_ARRAY_OBJ(other);

		if (arrayOther->elements.size() != elements.size())
			return false;

		for (size_t i = 0; i < elements.size(); ++i)
			if (elements[i] != arrayOther->elements[i])
				return false;

		return true;
	}

	std::vector<uint8_t> ArrayObject::Serialize() const
	{
		return std::vector<uint8_t>();
	}

	DictObject::DictObject()
		: Object(ObjectKind::DICT)
	{
	}
	DictObject::DictObject(const ValueUnorderedMap &elements)
		: Object(ObjectKind::DICT), elements(elements)
	{
	}
	DictObject::~DictObject()
	{
	}

	STD_STRING DictObject::ToString() const
	{
		STD_STRING result = TEXT("{");
		for (const auto &[k, v] : elements)
			result += k.ToString() + TEXT(":") + v.ToString() + TEXT(",");
		result = result.substr(0, result.size() - 1);
		result += TEXT("}");
		return result;
	}

	void DictObject::Blacken()
	{
		Object::Blacken();
		for (auto &[k, v] : elements)
		{
			k.Mark();
			v.Mark();
		}
	}

	bool DictObject::IsEqualTo(Object *other)
	{
		if (!IS_TABLE_OBJ(other))
			return false;

		DictObject *dictOther = TO_TABLE_OBJ(other);

		if (dictOther->elements.size() != elements.size())
			return false;

		for (const auto &[k1, v1] : elements)
		{
			bool isFound = false;
			for (const auto &[k2, v2] : dictOther->elements)
			{
				if (k1 == k2 && v1 == v2)
					isFound = true;
			}
			if (!isFound)
				return false;
		}

		return true;
	}

	std::vector<uint8_t> DictObject::Serialize() const
	{
		return std::vector<uint8_t>();
	}

	StructObject::StructObject()
		: Object(ObjectKind::STRUCT)
	{
	}
	StructObject::StructObject(const std::unordered_map<STD_STRING, Value> &elements)
		: Object(ObjectKind::STRUCT), elements(elements)
	{
	}
	StructObject::~StructObject()
	{
	}

	STD_STRING StructObject::ToString() const
	{
		STD_STRING result = TEXT("{");
		for (const auto &[k, v] : elements)
			result += k + TEXT(":") + v.ToString() + TEXT(",");
		result = result.substr(0, result.size() - 1);
		result += TEXT("}");
		return result;
	}

	void StructObject::Blacken()
	{
		Object::Blacken();
		for (auto &[k, v] : elements)
			v.Mark();
	}
	bool StructObject::IsEqualTo(Object *other)
	{
		if (!IS_STRUCT_OBJ(other))
			return false;

		StructObject *structOther = TO_STRUCT_OBJ(other);

		if (structOther->elements.size() != elements.size())
			return false;

		for (const auto &[k1, v1] : elements)
		{
			bool isFound = false;
			for (const auto &[k2, v2] : structOther->elements)
			{
				if (k1 == k2 && v1 == v2)
					isFound = true;
			}
			if (!isFound)
				return false;
		}

		return true;
	}
	std::vector<uint8_t> StructObject::Serialize() const
	{
		return std::vector<uint8_t>();
	}

	FunctionObject::FunctionObject()
		: Object(ObjectKind::FUNCTION), arity(0), upValueCount(0), varArg(VarArg::NONE)
	{
	}
	FunctionObject::FunctionObject(STD_STRING_VIEW name)
		: Object(ObjectKind::FUNCTION), arity(0), upValueCount(0), name(name), varArg(VarArg::NONE)
	{
	}
	FunctionObject::~FunctionObject()
	{
	}

	STD_STRING FunctionObject::ToString() const
	{
		return TEXT("<fn ") + name + TEXT(":0x") + PointerAddressToString((void *)this) + TEXT(">");
	}
#ifndef NDEBUG
	STD_STRING FunctionObject::ToStringWithChunk() const
	{
		return ToString() + TEXT("\n") + chunk.ToString();
	}
#endif

	void FunctionObject::Blacken()
	{
		Object::Blacken();
		for (auto &c : chunk.constants)
			c.Mark();
		if (Config::GetInstance()->IsUseFunctionCache())
		{
			for (auto &[key, value] : caches)
			{
				for (auto &valueElement : value)
					valueElement.Mark();
			}
		}
	}

	bool FunctionObject::IsEqualTo(Object *other)
	{
		if (!IS_FUNCTION_OBJ(other))
			return false;

		auto func = TO_FUNCTION_OBJ(other);
		if (arity != func->arity)
			return false;
		if (upValueCount != func->upValueCount)
			return false;
		if (chunk != func->chunk)
			return false;
		if (name != func->name)
			return false;
		return true;
	}

	std::vector<uint8_t> FunctionObject::Serialize() const
	{
		return std::vector<uint8_t>();
	}

	void FunctionObject::SetCache(size_t hash, const std::vector<Value> &result)
	{
		caches[hash] = result;
	}
	bool FunctionObject::GetCache(size_t hash,std::vector<Value> &result) const
	{
		auto iter2 = caches.find(hash);
		if (iter2 != caches.end())
		{
			result = iter2->second;
			return true;
		}

		return false;
	}

	UpValueObject::UpValueObject()
		: Object(ObjectKind::UPVALUE), location(nullptr), nextUpValue(nullptr)
	{
	}
	UpValueObject::UpValueObject(Value *location)
		: Object(ObjectKind::UPVALUE), location(location), nextUpValue(nullptr)
	{
	}
	UpValueObject::~UpValueObject()
	{
	}

	STD_STRING UpValueObject::ToString() const
	{
		return location->ToString();
	}

	void UpValueObject::Blacken()
	{
		Object::Blacken();
		closed.Mark();
	}

	bool UpValueObject::IsEqualTo(Object *other)
	{
		if (!IS_UPVALUE_OBJ(other))
			return false;

		auto upvo = TO_UPVALUE_OBJ(other);

		if (closed != upvo->closed)
			return false;
		if (*location != *upvo->location)
			return false;
		if (!nextUpValue->IsEqualTo(upvo->nextUpValue))
			return false;
		return true;
	}

	std::vector<uint8_t> UpValueObject::Serialize() const
	{
		return std::vector<uint8_t>();
	}

	ClosureObject::ClosureObject()
		: Object(ObjectKind::CLOSURE), function(nullptr)
	{
	}
	ClosureObject::ClosureObject(FunctionObject *function)
		: Object(ObjectKind::CLOSURE), function(function)
	{
		upvalues.resize(function->upValueCount);
	}
	ClosureObject::~ClosureObject()
	{
	}

	STD_STRING ClosureObject::ToString() const
	{
		return function->ToString();
	}

	void ClosureObject::Blacken()
	{
		Object::Blacken();
		function->Mark();
		for (int32_t i = 0; i < upvalues.size(); ++i)
			if (upvalues[i])
				upvalues[i]->Mark();
	}

	bool ClosureObject::IsEqualTo(Object *other)
	{
		if (!IS_CLOSURE_OBJ(other))
			return false;
		auto closure = TO_CLOSURE_OBJ(other);

		if (!function->IsEqualTo(closure->function))
			return false;
		if (upvalues.size() != closure->upvalues.size())
			return false;
		for (int32_t i = 0; i < upvalues.size(); ++i)
			if (!upvalues[i]->IsEqualTo(closure->upvalues[i]))
				return false;
		return true;
	}

	std::vector<uint8_t> ClosureObject::Serialize() const
	{
		return std::vector<uint8_t>();
	}

	NativeFunctionObject::NativeFunctionObject()
		: Object(ObjectKind::NATIVE_FUNCTION)
	{
	}
	NativeFunctionObject::NativeFunctionObject(NativeFunction f)
		: Object(ObjectKind::NATIVE_FUNCTION), fn(f)
	{
	}
	NativeFunctionObject::~NativeFunctionObject()
	{
	}

	STD_STRING NativeFunctionObject::ToString() const
	{
		return TEXT("<native function>");
	}

	bool NativeFunctionObject::IsEqualTo(Object *other)
	{
		if (!IS_NATIVE_FUNCTION_OBJ(other))
			return false;
		return true;
	}

	std::vector<uint8_t> NativeFunctionObject::Serialize() const
	{
		return std::vector<uint8_t>();
	}

	RefObject::RefObject(Value *pointer)
		: Object(ObjectKind::REF), pointer(pointer)
	{
	}
	RefObject::~RefObject()
	{
	}

	STD_STRING RefObject::ToString() const
	{
		return pointer->ToString();
	}

	bool RefObject::IsEqualTo(Object *other)
	{
		if (!IS_REF_OBJ(other))
			return false;
		return *pointer == *TO_REF_OBJ(other)->pointer;
	}

	std::vector<uint8_t> RefObject::Serialize() const
	{
		return std::vector<uint8_t>();
	}

	ClassObject::ClassObject()
		: Object(ObjectKind::CLASS)
	{
	}

	ClassObject::ClassObject(STD_STRING_VIEW name)
		: Object(ObjectKind::CLASS), name(name)
	{
	}

	ClassObject::~ClassObject()
	{
	}

	STD_STRING ClassObject::ToString() const
	{
		STD_STRING result = TEXT("class ") + name;
		if (!parents.empty())
		{
			result += TEXT(":");
			for (const auto &[k, v] : parents)
				result += k + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT("\n{\n");
		for (const auto &[k, v] : members)
			result += TEXT("  ") + k + TEXT(":") + v.ToString() + TEXT("\n");

		return result + TEXT("}\n");
	}

	void ClassObject::Blacken()
	{
		Object::Blacken();
		for (auto &[k, v] : members)
			v.Mark();
		for (auto &[k, v] : parents)
			v->Mark();
		for (auto &[k, v] : constructors)
			v->Mark();
	}

	bool ClassObject::IsEqualTo(Object *other)
	{
		if (!IS_CLASS_OBJ(other))
			return false;
		auto klass = TO_CLASS_OBJ(other);
		if (name != klass->name)
			return false;
		if (members != klass->members)
			return false;
		if (parents != klass->parents)
			return false;
		return true;
	}

	std::vector<uint8_t> ClassObject::Serialize() const
	{
		return std::vector<uint8_t>();
	}

	bool ClassObject::GetMember(const STD_STRING &name, Value &retV)
	{
		auto iter = members.find(name);
		if (iter != members.end())
		{
			retV = iter->second;
			return true;
		}
		else if (!parents.empty())
		{
			bool hasValue = false;
			for (const auto &[k, v] : parents)
			{
				if (name == k)
				{
					retV = v;
					hasValue = true;
				}
				else
				{
					hasValue = v->GetMember(name, retV);
				}
			}
			return hasValue;
		}
		return false;
	}

	bool ClassObject::GetParentMember(const STD_STRING &name, Value &retV)
	{
		if (!parents.empty())
		{
			bool hasValue = false;
			for (const auto &[k, v] : parents)
			{
				if (name == k)
				{
					retV = v;
					hasValue = true;
				}
				else
				{
					hasValue = v->GetMember(name, retV);
				}
			}
			return hasValue;
		}
		return false;
	}

	ClassClosureBindObject::ClassClosureBindObject()
		: Object(ObjectKind::CLASS_CLOSURE_BIND), closure(nullptr)
	{
	}
	ClassClosureBindObject::ClassClosureBindObject(const Value &receiver, ClosureObject *cl)
		: Object(ObjectKind::CLASS_CLOSURE_BIND), receiver(receiver), closure(cl)
	{
	}
	ClassClosureBindObject::~ClassClosureBindObject()
	{
	}
	STD_STRING ClassClosureBindObject::ToString() const
	{
		return closure->ToString();
	}

	void ClassClosureBindObject::Blacken()
	{
		Object::Blacken();
		receiver.Mark();
		closure->Mark();
	}

	bool ClassClosureBindObject::IsEqualTo(Object *other)
	{
		if (!IS_CLASS_CLOSURE_BIND_OBJ(other))
			return false;
		auto ccb = TO_CLASS_CLOSURE_BIND_OBJ(other);
		if (receiver != ccb->receiver)
			return false;
		if (!closure->IsEqualTo(ccb->closure))
			return false;
		return true;
	}

	std::vector<uint8_t> ClassClosureBindObject::Serialize() const
	{
		return std::vector<uint8_t>();
	}

	EnumObject::EnumObject()
		: Object(ObjectKind::ENUM)
	{
	}
	EnumObject::EnumObject(const STD_STRING &name, const std::unordered_map<STD_STRING, Value> &pairs)
		: Object(ObjectKind::ENUM), name(name), pairs(pairs)
	{
	}

	EnumObject::~EnumObject()
	{
	}

	STD_STRING EnumObject::ToString() const
	{
		STD_STRING result = TEXT("enum ") + name + TEXT("{");
		if (!pairs.empty())
		{
			for (const auto &[k, v] : pairs)
				result += k + TEXT("=") + v.ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		return result + TEXT("}");
	}

	void EnumObject::Blacken()
	{
		Object::Blacken();
		for (auto &[k, v] : pairs)
			v.Mark();
	}

	bool EnumObject::GetMember(const STD_STRING &name, Value &retV)
	{
		auto iter = pairs.find(name);
		if (iter != pairs.end())
		{
			retV = iter->second;
			return true;
		}
		return false;
	}

	bool EnumObject::IsEqualTo(Object *other)
	{
		if (!IS_ENUM_OBJ(other))
			return false;
		auto eo = TO_ENUM_OBJ(other);
		if (name != eo->name)
			return false;
		if (pairs != eo->pairs)
			return false;
		return true;
	}

	std::vector<uint8_t> EnumObject::Serialize() const
	{
		return std::vector<uint8_t>();
	}

	ModuleObject::ModuleObject()
		: Object(ObjectKind::MODULE)
	{
	}

	ModuleObject::ModuleObject(const STD_STRING &name, const std::unordered_map<STD_STRING, Value> &values)
		: Object(ObjectKind::MODULE), name(name), values(values)
	{
	}

	ModuleObject::~ModuleObject()
	{
	}

	STD_STRING ModuleObject::ToString() const
	{
		STD_STRING result = TEXT("module ") + name + TEXT("{");
		if (!values.empty())
		{
			for (const auto &[k, v] : values)
				result += k + TEXT("=") + v.ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		return result + TEXT("}");
	}

	void ModuleObject::Blacken()
	{
		Object::Blacken();
		for (auto &[k, v] : values)
			v.Mark();
	}

	bool ModuleObject::IsEqualTo(Object *other)
	{
		if (!IS_MODULE_OBJ(other))
			return false;
		auto eo = TO_MODULE_OBJ(other);
		if (name != eo->name)
			return false;
		if (values != eo->values)
			return false;
		return true;
	}

	std::vector<uint8_t> ModuleObject::Serialize() const
	{
		return std::vector<uint8_t>();
	}

	bool ModuleObject::GetMember(const STD_STRING &name, Value &retV)
	{
		auto iter = values.find(name);
		if (iter != values.end())
		{
			retV = iter->second;
			return true;
		}
		return false;
	}

}