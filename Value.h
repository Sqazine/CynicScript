#pragma once
#include <string>
#include <unordered_map>
#include "Common.h"
namespace CynicScript
{
	enum ValueKind : uint8_t
	{
		NIL,
		INT,
		REAL,
		BOOL,
		OBJECT,
	};

	struct CYS_API Value
	{
		Value() noexcept;
		Value(int64_t integer) noexcept;
		Value(double number) noexcept;
		Value(bool boolean) noexcept;
		Value(struct Object *object) noexcept;
		~Value() noexcept = default;

		STRING ToString() const;
		void Mark() const;
		void UnMark() const;

		std::vector<uint8_t> Serialize() const;
		void Deserialize(const std::vector<uint8_t> &data);

		ValueKind kind;
		Permission permission = Permission::MUTABLE;

		union
		{
			int64_t integer;
			double realnum;
			bool boolean;
			struct Object *object;
		};
	};

	CYS_API bool operator==(const Value &left, const Value &right);
	CYS_API bool operator!=(const Value &left, const Value &right);

	struct CYS_API ValueHash
	{
		size_t operator()(const Value *v) const;
		size_t operator()(const Value &v) const;
	};

	using ValueUnorderedMap = std::unordered_map<Value, Value, ValueHash>;

	size_t HashValueList(Value *start, size_t count);
	size_t HashValueList(Value *start, Value *end);
}