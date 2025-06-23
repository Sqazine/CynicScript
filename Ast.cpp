#include "Ast.h"

namespace CynicScript
{
	//----------------------Expressions-----------------------------

	LiteralExpr::LiteralExpr(Token *tagToken)
		: Expr(tagToken, AstKind::LITERAL)
	{
	}
	LiteralExpr::LiteralExpr(Token *tagToken, int64_t value)
		: Expr(tagToken, AstKind::LITERAL), i64Value(value)
	{
		type = Type(TypeKind::I64,tagToken->sourceLocation);
	}
	LiteralExpr::LiteralExpr(Token *tagToken, double value)
		: Expr(tagToken, AstKind::LITERAL), f64Value(value)
	{
		type = Type(TypeKind::F64,tagToken->sourceLocation);
	}
	LiteralExpr::LiteralExpr(Token *tagToken, bool value)
		: Expr(tagToken, AstKind::LITERAL), boolean(value)
	{
		type = Type(TypeKind::BOOL,tagToken->sourceLocation);
	}
	LiteralExpr::LiteralExpr(Token *tagToken, STRING_VIEW value)
		: Expr(tagToken, AstKind::LITERAL), str(value)
	{
		type = Type(TypeKind::STR,tagToken->sourceLocation);
	}
	LiteralExpr::~LiteralExpr()
	{
	}
#ifndef NDEBUG
	STRING LiteralExpr::ToString()
	{
		switch (type.GetKind())
		{
		case TypeKind::I8:
		case TypeKind::I16:
		case TypeKind::I32:
		case TypeKind::I64:
		case TypeKind::U8:
		case TypeKind::U16:
		case TypeKind::U32:
		case TypeKind::U64:
			return CYS_TO_STRING(i64Value);
		case TypeKind::F32:
		case TypeKind::F64:
			return CYS_TO_STRING(f64Value);
		case TypeKind::BOOL:
			return boolean ? TEXT("true") : TEXT("false");
		case TypeKind::CHAR:
			return STRING({character});
		case TypeKind::STR:
			return str;
		default:
			return TEXT("null");
		}
	}
#endif
	IdentifierExpr::IdentifierExpr(Token *tagToken)
		: Expr(tagToken, AstKind::IDENTIFIER)
	{
	}
	IdentifierExpr::IdentifierExpr(Token *tagToken, STRING_VIEW literal)
		: Expr(tagToken, AstKind::IDENTIFIER), literal(literal)
	{
	}
	IdentifierExpr::~IdentifierExpr()
	{
	}

#ifndef NDEBUG
	STRING IdentifierExpr::ToString()
	{
		return literal;
	}
#endif
	VarDescExpr::VarDescExpr(Token *tagToken)
		: Expr(tagToken, AstKind::VAR_DESC), name(nullptr)
	{
		type = Type(TypeKind::ANY,tagToken->sourceLocation);
	}
	VarDescExpr::VarDescExpr(Token *tagToken, const Type &type, Expr *name)
		: Expr(tagToken, AstKind::VAR_DESC), name(name)
	{
		this->type = type;
	}
	VarDescExpr::~VarDescExpr()
	{
		SAFE_DELETE(name);
	}
#ifndef NDEBUG
	STRING VarDescExpr::ToString()
	{
		return name->ToString() + TEXT(":") + STRING(type.GetName().data());
	}
#endif
	ArrayExpr::ArrayExpr(Token *tagToken)
		: Expr(tagToken, AstKind::ARRAY)
	{
	}
	ArrayExpr::ArrayExpr(Token *tagToken, const std::vector<Expr *> &elements) : Expr(tagToken, AstKind::ARRAY), elements(elements)
	{
	}
	ArrayExpr::~ArrayExpr()
	{
		std::vector<Expr *>().swap(elements);
	}
#ifndef NDEBUG
	STRING ArrayExpr::ToString()
	{
		STRING result = TEXT("[");

		if (!elements.empty())
		{
			for (auto e : elements)
				result += e->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT("]");
		return result;
	}
#endif

	DictExpr::DictExpr(Token *tagToken)
		: Expr(tagToken, AstKind::DICT)
	{
	}
	DictExpr::DictExpr(Token *tagToken, const std::vector<std::pair<Expr *, Expr *>> &elements)
		: Expr(tagToken, AstKind::DICT), elements(elements)
	{
	}
	DictExpr::~DictExpr()
	{
		std::vector<std::pair<Expr *, Expr *>>().swap(elements);
	}
#ifndef NDEBUG
	STRING DictExpr::ToString()
	{
		STRING result = TEXT("{");

		if (!elements.empty())
		{
			for (auto [key, value] : elements)
				result += key->ToString() + TEXT(":") + value->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT("}");
		return result;
	}
#endif

	GroupExpr::GroupExpr(Token *tagToken)
		: Expr(tagToken, AstKind::GROUP), expr(nullptr)
	{
	}
	GroupExpr::GroupExpr(Token *tagToken, Expr *expr)
		: Expr(tagToken, AstKind::GROUP), expr(expr)
	{
	}
	GroupExpr::~GroupExpr()
	{
		SAFE_DELETE(expr);
	}

#ifndef NDEBUG
	STRING GroupExpr::ToString()
	{
		return TEXT("(") + expr->ToString() + TEXT(")");
	}
#endif

	PrefixExpr::PrefixExpr(Token *tagToken)
		: Expr(tagToken, AstKind::PREFIX), right(nullptr)
	{
	}
	PrefixExpr::PrefixExpr(Token *tagToken, STRING_VIEW op, Expr *right)
		: Expr(tagToken, AstKind::PREFIX), op(op), right(right)
	{
	}
	PrefixExpr::~PrefixExpr()
	{
		SAFE_DELETE(right);
	}
#ifndef NDEBUG
	STRING PrefixExpr::ToString()
	{
		return op + right->ToString();
	}
#endif
	InfixExpr::InfixExpr(Token *tagToken)
		: Expr(tagToken, AstKind::INFIX), left(nullptr), right(nullptr)
	{
	}
	InfixExpr::InfixExpr(Token *tagToken, STRING_VIEW op, Expr *left, Expr *right)
		: Expr(tagToken, AstKind::INFIX), op(op), left(left), right(right)
	{
	}
	InfixExpr::~InfixExpr()
	{
		SAFE_DELETE(left);
		SAFE_DELETE(right);
	}
#ifndef NDEBUG
	STRING InfixExpr::ToString()
	{
		return left->ToString() + op + right->ToString();
	}
#endif

	PostfixExpr::PostfixExpr(Token *tagToken)
		: Expr(tagToken, AstKind::POSTFIX), left(nullptr)
	{
	}
	PostfixExpr::PostfixExpr(Token *tagToken, Expr *left, STRING_VIEW op)
		: Expr(tagToken, AstKind::POSTFIX), left(left), op(op)
	{
	}
	PostfixExpr::~PostfixExpr()
	{
		SAFE_DELETE(left);
	}

#ifndef NDEBUG
	STRING PostfixExpr::ToString()
	{
		return left->ToString() + op;
	}
#endif

	ConditionExpr::ConditionExpr(Token *tagToken)
		: Expr(tagToken, AstKind::CONDITION), condition(nullptr), trueBranch(nullptr), falseBranch(nullptr)
	{
	}
	ConditionExpr::ConditionExpr(Token *tagToken, Expr *condition, Expr *trueBranch, Expr *falseBranch)
		: Expr(tagToken, AstKind::CONDITION), condition(condition), trueBranch(trueBranch), falseBranch(falseBranch)
	{
	}
	ConditionExpr::~ConditionExpr()
	{
		SAFE_DELETE(condition);
		SAFE_DELETE(trueBranch);
		SAFE_DELETE(falseBranch);
	}
#ifndef NDEBUG
	STRING ConditionExpr::ToString()
	{
		return condition->ToString() + TEXT("?") + trueBranch->ToString() + TEXT(":") + falseBranch->ToString();
	}
#endif
	IndexExpr::IndexExpr(Token *tagToken)
		: Expr(tagToken, AstKind::INDEX), ds(nullptr), index(nullptr)
	{
	}
	IndexExpr::IndexExpr(Token *tagToken, Expr *ds, Expr *index)
		: Expr(tagToken, AstKind::INDEX), ds(ds), index(index)
	{
	}
	IndexExpr::~IndexExpr()
	{
		SAFE_DELETE(ds);
		SAFE_DELETE(index);
	}
#ifndef NDEBUG
	STRING IndexExpr::ToString()
	{
		return ds->ToString() + TEXT("[") + index->ToString() + TEXT("]");
	}
#endif

	RefExpr::RefExpr(Token *tagToken)
		: Expr(tagToken, AstKind::REF), refExpr(nullptr)
	{
	}
	RefExpr::RefExpr(Token *tagToken, Expr *refExpr)
		: Expr(tagToken, AstKind::REF), refExpr(refExpr)
	{
	}
	RefExpr::~RefExpr()
	{
		SAFE_DELETE(refExpr);
	}
#ifndef NDEBUG
	STRING RefExpr::ToString()
	{
		return TEXT("&") + refExpr->ToString();
	}
#endif
	LambdaExpr::LambdaExpr(Token *tagToken)
		: Expr(tagToken, AstKind::LAMBDA), body(nullptr)
	{
	}
	LambdaExpr::LambdaExpr(Token *tagToken, const std::vector<VarDescExpr *> &parameters, ScopeStmt *body)
		: Expr(tagToken, AstKind::LAMBDA), parameters(parameters), body(body)
	{
	}
	LambdaExpr::~LambdaExpr()
	{
		std::vector<VarDescExpr *>().swap(parameters);
		SAFE_DELETE(body);
	}
#ifndef NDEBUG
	STRING LambdaExpr::ToString()
	{
		STRING result = TEXT("fn(");
		if (!parameters.empty())
		{
			for (auto param : parameters)
				result += param->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT(")");
		result += body->ToString();
		return result;
	}
#endif

	CallExpr::CallExpr(Token *tagToken)
		: Expr(tagToken, AstKind::CALL), callee(nullptr)
	{
	}
	CallExpr::CallExpr(Token *tagToken, Expr *callee, const std::vector<Expr *> &arguments)
		: Expr(tagToken, AstKind::CALL), callee(callee), arguments(arguments)
	{
	}
	CallExpr::~CallExpr()
	{
		SAFE_DELETE(callee);
		std::vector<Expr *>().swap(arguments);
	}
#ifndef NDEBUG
	STRING CallExpr::ToString()
	{
		STRING result = callee->ToString() + TEXT("(");

		if (!arguments.empty())
		{
			for (const auto &arg : arguments)
				result += arg->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT(")");
		return result;
	}
#endif

	DotExpr::DotExpr(Token *tagToken)
		: Expr(tagToken, AstKind::DOT), callee(nullptr), callMember(nullptr)
	{
	}
	DotExpr::DotExpr(Token *tagToken, Expr *callee, IdentifierExpr *callMember)
		: Expr(tagToken, AstKind::DOT), callee(callee), callMember(callMember)
	{
	}
	DotExpr::~DotExpr()
	{
		SAFE_DELETE(callee);
		SAFE_DELETE(callMember);
	}
#ifndef NDEBUG
	STRING DotExpr::ToString()
	{
		return callee->ToString() + TEXT(".") + callMember->ToString();
	}
#endif
	NewExpr::NewExpr(Token *tagToken)
		: Expr(tagToken, AstKind::NEW), callee(nullptr)
	{
	}
	NewExpr::NewExpr(Token *tagToken, Expr *callee)
		: Expr(tagToken, AstKind::NEW), callee(callee)
	{
	}
	NewExpr::~NewExpr()
	{
		SAFE_DELETE(callee);
	}
#ifndef NDEBUG
	STRING NewExpr::ToString()
	{
		return TEXT("new ") + callee->ToString();
	}
#endif

	ThisExpr::ThisExpr(Token *tagToken)
		: Expr(tagToken, AstKind::THIS)
	{
	}

	ThisExpr::~ThisExpr()
	{
	}
#ifndef NDEBUG
	STRING ThisExpr::ToString()
	{
		return TEXT("this");
	}
#endif

	BaseExpr::BaseExpr(Token *tagToken, IdentifierExpr *callMember)
		: Expr(tagToken, AstKind::BASE), callMember(callMember)
	{
	}
	BaseExpr::~BaseExpr()
	{
		SAFE_DELETE(callMember);
	}
#ifndef NDEBUG
	STRING BaseExpr::ToString()
	{
		return TEXT("base.") + callMember->ToString();
	}
#endif

	CompoundExpr::CompoundExpr(Token *tagToken)
		: Expr(tagToken, AstKind::COMPOUND), endExpr(nullptr)
	{
	}
	CompoundExpr::CompoundExpr(Token *tagToken, const std::vector<Stmt *> &stmts, Expr *endExpr)
		: Expr(tagToken, AstKind::COMPOUND), stmts(stmts), endExpr(endExpr)
	{
	}
	CompoundExpr::~CompoundExpr()
	{
		SAFE_DELETE(endExpr);
		std::vector<Stmt *>().swap(stmts);
	}
#ifndef NDEBUG
	STRING CompoundExpr::ToString()
	{
		STRING result = TEXT("({");
		for (const auto &stmt : stmts)
			result += stmt->ToString();
		result += endExpr->ToString();
		result += TEXT("})");
		return result;
	}
#endif

	StructExpr::StructExpr(Token *tagToken)
		: Expr(tagToken, AstKind::STRUCT)
	{
	}
	StructExpr::StructExpr(Token *tagToken, const std::vector<std::pair<STRING, Expr *>> &elements)
		: Expr(tagToken, AstKind::STRUCT), elements(elements)
	{
	}
	StructExpr::~StructExpr()
	{
		std::vector<std::pair<STRING, Expr *>>().swap(elements);
	}
#ifndef NDEBUG
	STRING StructExpr::ToString()
	{
		STRING result = TEXT("{");
		if (!elements.empty())
		{
			for (auto [key, value] : elements)
				result += key + TEXT(":") + value->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT("}");
		return result;
	}
#endif
	VarArgExpr::VarArgExpr(Token *tagToken)
		: Expr(tagToken, AstKind::VAR_ARG), argName(nullptr)
	{
	}
	VarArgExpr::VarArgExpr(Token *tagToken, IdentifierExpr *argName)
		: Expr(tagToken, AstKind::VAR_ARG), argName(argName)
	{
	}
	VarArgExpr::~VarArgExpr()
	{
		SAFE_DELETE(argName);
	}
#ifndef NDEBUG
	STRING VarArgExpr::ToString()
	{
		return TEXT("...") + (argName ? argName->ToString() : TEXT(""));
	}
#endif
	FactorialExpr::FactorialExpr(Token *tagToken)
		: Expr(tagToken, AstKind::FACTORIAL), expr(nullptr)
	{
	}
	FactorialExpr::FactorialExpr(Token *tagToken, Expr *expr)
		: Expr(tagToken, AstKind::FACTORIAL), expr(expr)
	{
	}
	FactorialExpr::~FactorialExpr()
	{
		SAFE_DELETE(expr);
	}
#ifndef NDEBUG
	STRING FactorialExpr::ToString()
	{
		return expr->ToString() + TEXT("!");
	}
#endif

	AppregateExpr::AppregateExpr(Token *tagToken)
		: Expr(tagToken, AstKind::APPREGATE)
	{
	}
	AppregateExpr::AppregateExpr(Token *tagToken, const std::vector<Expr *> &exprs)
		: Expr(tagToken, AstKind::APPREGATE), exprs(exprs)
	{
	}
	AppregateExpr::~AppregateExpr()
	{
		std::vector<Expr *>().swap(exprs);
	}
#ifndef NDEBUG
	STRING AppregateExpr::ToString()
	{
		STRING result = TEXT("(");
		if (!exprs.empty())
		{
			for (const auto &expr : exprs)
				result += expr->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}

		return result + TEXT(")");
	}
#endif
	//----------------------Statements-----------------------------

	ExprStmt::ExprStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::EXPR), expr(nullptr)
	{
	}
	ExprStmt::ExprStmt(Token *tagToken, Expr *expr)
		: Stmt(tagToken, AstKind::EXPR), expr(expr)
	{
	}
	ExprStmt::~ExprStmt()
	{
		SAFE_DELETE(expr);
	}
#ifndef NDEBUG
	STRING ExprStmt::ToString()
	{
		return expr->ToString() + TEXT(";");
	}
#endif
	ReturnStmt::ReturnStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::RETURN), expr(nullptr)
	{
	}
	ReturnStmt::ReturnStmt(Token *tagToken, Expr *expr)
		: Stmt(tagToken, AstKind::RETURN), expr(expr)
	{
	}
	ReturnStmt::~ReturnStmt()
	{
		SAFE_DELETE(expr);
	}
#ifndef NDEBUG
	STRING ReturnStmt::ToString()
	{
		if (!expr)
			return TEXT("return;");
		else
			return TEXT("return ") + expr->ToString() + TEXT(";");
	}
#endif

	IfStmt::IfStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::IF), condition(nullptr), thenBranch(nullptr), elseBranch(nullptr)
	{
	}
	IfStmt::IfStmt(Token *tagToken, Expr *condition, Stmt *thenBranch, Stmt *elseBranch)
		: Stmt(tagToken, AstKind::IF),
		  condition(condition),
		  thenBranch(thenBranch),
		  elseBranch(elseBranch)
	{
	}
	IfStmt::~IfStmt()
	{
		SAFE_DELETE(condition);
		SAFE_DELETE(thenBranch);
		SAFE_DELETE(elseBranch);
	}
#ifndef NDEBUG
	STRING IfStmt::ToString()
	{
		STRING result;
		result = TEXT("if(") + condition->ToString() + TEXT(")") + thenBranch->ToString();
		if (elseBranch != nullptr)
			result += TEXT("else ") + elseBranch->ToString();
		return result;
	}
#endif

	ScopeStmt::ScopeStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::SCOPE)
	{
	}
	ScopeStmt::ScopeStmt(Token *tagToken, const std::vector<Stmt *> &stmts)
		: Stmt(tagToken, AstKind::SCOPE), stmts(stmts)
	{
	}
	ScopeStmt::~ScopeStmt()
	{
		std::vector<Stmt *>().swap(stmts);
	}
#ifndef NDEBUG
	STRING ScopeStmt::ToString()
	{
		STRING result = TEXT("{");
		for (const auto &stmt : stmts)
			result += stmt->ToString();
		result += TEXT("}");
		return result;
	}
#endif

	WhileStmt::WhileStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::WHILE), condition(nullptr), body(nullptr), increment(nullptr)
	{
	}
	WhileStmt::WhileStmt(Token *tagToken, Expr *condition, ScopeStmt *body, ScopeStmt *increment)
		: Stmt(tagToken, AstKind::WHILE), condition(condition), body(body), increment(increment)
	{
	}
	WhileStmt::~WhileStmt()
	{
		SAFE_DELETE(condition);
		SAFE_DELETE(body);
		SAFE_DELETE(increment);
	}
#ifndef NDEBUG
	STRING WhileStmt::ToString()
	{
		STRING result = TEXT("while(") + condition->ToString() + TEXT("){") + body->ToString();
		if (increment)
			result += increment->ToString();
		return result += TEXT("}");
	}
#endif

	BreakStmt::BreakStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::BREAK)
	{
	}
	BreakStmt::~BreakStmt()
	{
	}
#ifndef NDEBUG
	STRING BreakStmt::ToString()
	{
		return TEXT("break;");
	}
#endif

	ContinueStmt::ContinueStmt(Token *tagToken)
		: Stmt(tagToken, AstKind::CONTINUE)
	{
	}
	ContinueStmt::~ContinueStmt()
	{
	}
#ifndef NDEBUG
	STRING ContinueStmt::ToString()
	{
		return TEXT("continue;");
	}
#endif

	AstStmts::AstStmts(Token *tagToken)
		: Stmt(tagToken, AstKind::ASTSTMTS)
	{
	}
	AstStmts::AstStmts(Token *tagToken, std::vector<Stmt *> stmts)
		: Stmt(tagToken, AstKind::ASTSTMTS), stmts(stmts)
	{
	}

	AstStmts::~AstStmts()
	{
		std::vector<Stmt *>().swap(stmts);
	}
#ifndef NDEBUG
	STRING AstStmts::ToString()
	{
		STRING result;
		for (const auto &stmt : stmts)
			result += stmt->ToString();
		return result;
	}
#endif
	//----------------------Declarations-----------------------------

	VarDecl::VarDecl(Token *tagToken)
		: Decl(tagToken, AstKind::VAR), permission(Permission::MUTABLE)
	{
	}
	VarDecl::VarDecl(Token *tagToken, Permission permission, const std::vector<std::pair<Expr *, Expr *>> &variables)
		: Decl(tagToken, AstKind::VAR), permission(permission), variables(variables)
	{
	}
	VarDecl::~VarDecl()
	{
		std::vector<std::pair<Expr *, Expr *>>().swap(variables);
	}
#ifndef NDEBUG
	STRING VarDecl::ToString()
	{
		STRING result;

		if (permission == Permission::IMMUTABLE)
			result += TEXT("const ");
		else
			result += TEXT("let ");

		if (!variables.empty())
		{
			for (auto [key, value] : variables)
				result += key->ToString() + TEXT("=") + value->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		return result + TEXT(";");
	}
#endif

	EnumDecl::EnumDecl(Token *tagToken)
		: Decl(tagToken, AstKind::ENUM), name(nullptr)
	{
	}
	EnumDecl::EnumDecl(Token *tagToken, IdentifierExpr *name, const std::unordered_map<IdentifierExpr *, Expr *> &enumItems)
		: Decl(tagToken, AstKind::ENUM), name(name), enumItems(enumItems)
	{
	}
	EnumDecl::~EnumDecl()
	{
		SAFE_DELETE(name);
		std::unordered_map<IdentifierExpr *, Expr *>().swap(enumItems);
	}

#ifndef NDEBUG
	STRING EnumDecl::ToString()
	{
		STRING result = TEXT("enum ") + name->ToString() + TEXT("{");

		if (!enumItems.empty())
		{
			for (auto [key, value] : enumItems)
				result += key->ToString() + TEXT("=") + value->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		return result + TEXT("}");
	}
#endif
	ModuleDecl::ModuleDecl(Token *tagToken)
		: Decl(tagToken, AstKind::MODULE), name(nullptr)
	{
	}
	ModuleDecl::ModuleDecl(Token *tagToken,
						   IdentifierExpr *name,
						   const std::vector<VarDecl *> &varItems,
						   const std::vector<ClassDecl *> &classItems,
						   const std::vector<ModuleDecl *> &moduleItems,
						   const std::vector<EnumDecl *> &enumItems,
						   const std::vector<FunctionDecl *> &functionItems)
		: Decl(tagToken, AstKind::MODULE), name(name),
		  varItems(varItems),
		  classItems(classItems),
		  moduleItems(moduleItems),
		  enumItems(enumItems),
		  functionItems(functionItems)
	{
	}
	ModuleDecl::~ModuleDecl()
	{
		SAFE_DELETE(name);
		std::vector<VarDecl *>().swap(varItems);
		std::vector<ClassDecl *>().swap(classItems);
		std::vector<ModuleDecl *>().swap(moduleItems);
		std::vector<EnumDecl *>().swap(enumItems);
		std::vector<FunctionDecl *>().swap(functionItems);
	}
#ifndef NDEBUG
	STRING ModuleDecl::ToString()
	{
		STRING result = TEXT("module ") + name->ToString() + TEXT("\n{\n");
		for (const auto &item : varItems)
			result += item->ToString() + TEXT("\n");
		for (const auto &item : classItems)
			result += item->ToString() + TEXT("\n");
		for (const auto &item : moduleItems)
			result += item->ToString() + TEXT("\n");
		for (const auto &item : enumItems)
			result += item->ToString() + TEXT("\n");
		for (const auto &item : functionItems)
			result += item->ToString() + TEXT("\n");
		return result + TEXT("}\n");
	}
#endif

	FunctionDecl::FunctionDecl(Token *tagToken)
		: Decl(tagToken, AstKind::FUNCTION), name(nullptr), body(nullptr)
	{
	}

	FunctionDecl::FunctionDecl(Token *tagToken, IdentifierExpr *name, const std::vector<VarDescExpr *> &parameters, ScopeStmt *body)
		: Decl(tagToken, AstKind::FUNCTION), name(name), parameters(parameters), body(body)
	{
	}

	FunctionDecl::~FunctionDecl()
	{
		SAFE_DELETE(name);
		std::vector<VarDescExpr *>().swap(parameters);
		SAFE_DELETE(body);
	}
#ifndef NDEBUG
	STRING FunctionDecl::ToString()
	{
		STRING result = TEXT("fn ") + name->ToString() + TEXT("(");
		if (!parameters.empty())
		{
			for (auto param : parameters)
				result += param->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT(")");
		result += body->ToString();
		return result;
	}
#endif

	ClassDecl::ClassDecl(Token *tagToken)
		: Decl(tagToken, AstKind::CLASS)
	{
	}

	ClassDecl::ClassDecl(Token *tagToken,
						 STRING name,
						 const std::vector<std::pair<MemberPrivilege, IdentifierExpr *>> &parents,
						 const std::vector<std::pair<MemberPrivilege, VarDecl *>> &variables,
						 const std::vector<std::pair<MemberPrivilege, FunctionMember>> &functions,
						 const std::vector<std::pair<MemberPrivilege, EnumDecl *>> &enumerations)
		: Decl(tagToken, AstKind::CLASS),
		  name(name),
		  variables(variables),
		  functions(functions),
		  enumerations(enumerations),
		  parents(parents)
	{
	}

	ClassDecl::~ClassDecl()
	{
		std::vector<std::pair<MemberPrivilege, IdentifierExpr *>>().swap(parents);
		std::vector<std::pair<MemberPrivilege, VarDecl *>>().swap(variables);
		std::vector<std::pair<MemberPrivilege, FunctionMember>>().swap(functions);
		std::vector<std::pair<MemberPrivilege, EnumDecl *>>().swap(enumerations);
	}
#ifndef NDEBUG
	STRING ClassDecl::ToString()
	{
		STRING result = TEXT("class ") + name;
		if (!parents.empty())
		{
			result += TEXT(":");
			for (const auto &parent : parents)
				result += parent.second->ToString() + TEXT(",");
			result = result.substr(0, result.size() - 1);
		}
		result += TEXT("{");
		for (auto enumeration : enumerations)
			result += enumeration.second->ToString();
		for (auto variable : variables)
			result += variable.second->ToString();
		for (auto fn : functions)
			result += fn.second.decl->ToString();
		return result + TEXT("}");
	}
#endif
}
