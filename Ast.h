#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Type.h"
#include "Token.h"
#include "Common.h"
namespace CynicScript
{
	enum class AstKind
	{
		// expr
		LITERAL,
		IDENTIFIER,
		VAR_DESC,
		GROUP,
		ARRAY,
		DICT,
		PREFIX,
		INFIX,
		POSTFIX,
		CONDITION,
		INDEX,
		REF,
		LAMBDA,
		DOT,
		CALL,
		NEW,
		THIS,
		BASE,
		COMPOUND,
		STRUCT,
		VAR_ARG,
		FACTORIAL,
		APPREGATE,
		// stmt
		VAR,
		EXPR,
		RETURN,
		IF,
		SCOPE,
		WHILE,
		BREAK,
		CONTINUE,
		ENUM,
		FUNCTION,
		CLASS,
		MODULE,
		ASTSTMTS,
	};

	struct AstNode
	{
		AstNode(Token *tagToken, AstKind kind) : tagToken(tagToken), kind(kind) {}
		virtual ~AstNode() {}
#ifndef NDEBUG
		virtual STRING ToString() = 0;
#endif
		const AstKind kind;
		Token *tagToken;
	};

	struct Expr : public AstNode
	{
		Expr(Token *tagToken, AstKind kind) : AstNode(tagToken, kind) {}
		virtual ~Expr() {}
#ifndef NDEBUG
		virtual STRING ToString() = 0;
#endif

		Type type;
	};

	struct LiteralExpr : public Expr
	{
		LiteralExpr(Token *tagToken);
		LiteralExpr(Token *tagToken, int64_t value);
		LiteralExpr(Token *tagToken, double value);
		LiteralExpr(Token *tagToken, bool value);
		LiteralExpr(Token *tagToken, STRING_VIEW value);
		~LiteralExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		union
		{
			int64_t i64Value;
			double f64Value;
			bool boolean;
			CHAR_T character;
			STRING str;
		};
	};

	struct IdentifierExpr : public Expr
	{
		IdentifierExpr(Token *tagToken);
		IdentifierExpr(Token *tagToken, STRING_VIEW literal);
		~IdentifierExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		STRING literal;
	};

	struct VarDescExpr : public Expr
	{
		VarDescExpr(Token *tagToken);
		VarDescExpr(Token *tagToken, const Type &type, Expr *name);
		~VarDescExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Expr *name;
	};

	struct ArrayExpr : public Expr
	{
		ArrayExpr(Token *tagToken);
		ArrayExpr(Token *tagToken, const std::vector<Expr *> &elements);
		~ArrayExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		std::vector<Expr *> elements;
	};

	struct DictExpr : public Expr
	{
		DictExpr(Token *tagToken);
		DictExpr(Token *tagToken, const std::vector<std::pair<Expr *, Expr *>> &elements);
		~DictExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		std::vector<std::pair<Expr *, Expr *>> elements;
	};

	struct GroupExpr : public Expr
	{
		GroupExpr(Token *tagToken);
		GroupExpr(Token *tagToken, Expr *expr);
		~GroupExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Expr *expr;
	};

	struct PrefixExpr : public Expr
	{
		PrefixExpr(Token *tagToken);
		PrefixExpr(Token *tagToken, STRING_VIEW op, Expr *right);
		~PrefixExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		STRING op;
		Expr *right;
	};

	struct InfixExpr : public Expr
	{
		InfixExpr(Token *tagToken);
		InfixExpr(Token *tagToken, STRING_VIEW op, Expr *left, Expr *right);
		~InfixExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		STRING op;
		Expr *left;
		Expr *right;
	};

	struct PostfixExpr : public Expr
	{
		PostfixExpr(Token *tagToken);
		PostfixExpr(Token *tagToken, Expr *left, STRING_VIEW op);
		~PostfixExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Expr *left;
		STRING op;
	};

	struct ConditionExpr : public Expr
	{
		ConditionExpr(Token *tagToken);
		ConditionExpr(Token *tagToken, Expr *condition, Expr *trueBranch, Expr *falseBranch);
		~ConditionExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Expr *condition;
		Expr *trueBranch;
		Expr *falseBranch;
	};

	struct IndexExpr : public Expr
	{
		IndexExpr(Token *tagToken);
		IndexExpr(Token *tagToken, Expr *ds, Expr *index);
		~IndexExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Expr *ds;
		Expr *index;
	};

	struct RefExpr : public Expr
	{
		RefExpr(Token *tagToken);
		RefExpr(Token *tagToken, Expr *refExpr);
		~RefExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Expr *refExpr;
	};

	struct LambdaExpr : public Expr
	{
		LambdaExpr(Token *tagToken);
		LambdaExpr(Token *tagToken, const std::vector<VarDescExpr *> &parameters, struct ScopeStmt *body);
		~LambdaExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		std::vector<VarDescExpr *> parameters;
		struct ScopeStmt *body;
	};

	struct CallExpr : public Expr
	{
		CallExpr(Token *tagToken);
		CallExpr(Token *tagToken, Expr *callee, const std::vector<Expr *> &arguments);
		~CallExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Expr *callee;
		std::vector<Expr *> arguments;
	};

	struct DotExpr : public Expr
	{
		DotExpr(Token *tagToken);
		DotExpr(Token *tagToken, Expr *callee, IdentifierExpr *callMember);
		~DotExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Expr *callee;
		IdentifierExpr *callMember;
	};

	struct NewExpr : public Expr
	{
		NewExpr(Token *tagToken);
		NewExpr(Token *tagToken, Expr *callee);
		~NewExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Expr *callee;
	};

	struct ThisExpr : public Expr
	{
		ThisExpr(Token *tagToken);
		~ThisExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif
	};

	struct BaseExpr : public Expr
	{
		BaseExpr(Token *tagToken, IdentifierExpr *callMember);
		~BaseExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		IdentifierExpr *callMember;
	};

	struct CompoundExpr : public Expr
	{
		CompoundExpr(Token *tagToken);
		CompoundExpr(Token *tagToken, const std::vector<struct Stmt *> &stmts, Expr *endExpr);
		~CompoundExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		std::vector<struct Stmt *> stmts;
		Expr *endExpr;
	};

	struct StructExpr : public Expr
	{
		StructExpr(Token *tagToken);
		StructExpr(Token *tagToken, const std::vector<std::pair<STRING, Expr *>> &elements);
		~StructExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		std::vector<std::pair<STRING, Expr *>> elements;
	};

	struct VarArgExpr : public Expr
	{
		VarArgExpr(Token *tagToken);
		VarArgExpr(Token *tagToken, IdentifierExpr *argName);
		~VarArgExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif
		IdentifierExpr *argName;
	};

	struct FactorialExpr : public Expr
	{
		FactorialExpr(Token *tagToken);
		FactorialExpr(Token *tagToken, Expr *expr);
		~FactorialExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif
		Expr *expr;
	};

	struct AppregateExpr : public Expr
	{
		AppregateExpr(Token *tagToken);
		AppregateExpr(Token *tagToken, const std::vector<Expr *> &exprs);
		~AppregateExpr() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif
		std::vector<Expr *> exprs;
	};

	struct Stmt : public AstNode
	{
		Stmt(Token *tagToken, AstKind kind) : AstNode(tagToken, kind) {}
		virtual ~Stmt() {}
		virtual STRING ToString() = 0;
	};

	struct ExprStmt : public Stmt
	{
		ExprStmt(Token *tagToken);
		ExprStmt(Token *tagToken, Expr *expr);
		~ExprStmt() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Expr *expr;
	};

	struct ReturnStmt : public Stmt
	{
		ReturnStmt(Token *tagToken);
		ReturnStmt(Token *tagToken, Expr *expr);
		~ReturnStmt() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Expr *expr;
	};

	struct IfStmt : public Stmt
	{
		IfStmt(Token *tagToken);
		IfStmt(Token *tagToken, Expr *condition, Stmt *thenBranch, Stmt *elseBranch);
		~IfStmt() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Expr *condition;
		Stmt *thenBranch;
		Stmt *elseBranch;
	};

	struct ScopeStmt : public Stmt
	{
		ScopeStmt(Token *tagToken);
		ScopeStmt(Token *tagToken, const std::vector<Stmt *> &stmts);
		~ScopeStmt() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		std::vector<Stmt *> stmts;
	};

	struct WhileStmt : public Stmt
	{
		WhileStmt(Token *tagToken);
		WhileStmt(Token *tagToken, Expr *condition, ScopeStmt *body, ScopeStmt *increment = nullptr);
		~WhileStmt() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Expr *condition;
		ScopeStmt *body;
		ScopeStmt *increment;
	};

	struct BreakStmt : public Stmt
	{
		BreakStmt(Token *tagToken);
		~BreakStmt() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif
	};

	struct ContinueStmt : public Stmt
	{
		ContinueStmt(Token *tagToken);
		~ContinueStmt() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif
	};

	struct AstStmts : public Stmt
	{
		AstStmts(Token *tagToken);
		AstStmts(Token *tagToken, std::vector<Stmt *> stmts);
		~AstStmts() override;

#ifndef NDEBUG
		STRING ToString() override;
#endif

		std::vector<Stmt *> stmts;
	};

	struct Decl : public Stmt
	{
		Decl(Token *tagToken, AstKind kind) : Stmt(tagToken, kind) {}
		virtual ~Decl() {}
		virtual STRING ToString() = 0;
	};

	struct VarDecl : public Decl
	{
		VarDecl(Token *tagToken);
		VarDecl(Token *tagToken, Permission permission, const std::vector<std::pair<Expr *, Expr *>> &variables);
		~VarDecl() override;
#ifndef NDEBUG
		STRING ToString() override;
#endif

		Permission permission;
		std::vector<std::pair<Expr *, Expr *>> variables;
	};

	struct EnumDecl : public Decl
	{
		EnumDecl(Token *tagToken);
		EnumDecl(Token *tagToken, IdentifierExpr *name, const std::unordered_map<IdentifierExpr *, Expr *> &enumItems);
		~EnumDecl() override;

#ifndef NDEBUG
		STRING ToString() override;
#endif

		IdentifierExpr *name;
		std::unordered_map<IdentifierExpr *, Expr *> enumItems;
	};

	struct FunctionDecl : public Decl
	{
		FunctionDecl(Token *tagToken);
		FunctionDecl(Token *tagToken, IdentifierExpr *name, const std::vector<VarDescExpr *> &parameters, ScopeStmt *body);
		~FunctionDecl() override;

#ifndef NDEBUG
		STRING ToString() override;
#endif

		IdentifierExpr *name;
		std::vector<VarDescExpr *> parameters;
		ScopeStmt *body;
		std::vector<Type> returnTypes;
	};

	struct ClassDecl : public Decl
	{
		enum FunctionKind
		{
			NONE,
			CONSTRUCTOR,
			DESTRUCTOR,
			MEMBER,
		};

		enum MemberPrivilege
		{
			PUBLIC,
			PROTECTED,
			PRIVATE,
		};

		struct FunctionMember
		{
			FunctionMember() : kind(FunctionKind::NONE), decl(nullptr) {}
			FunctionMember(FunctionKind kind, FunctionDecl *functionDecl) : kind(kind), decl(functionDecl) {}
			FunctionKind kind{FunctionKind::NONE};
			FunctionDecl *decl{nullptr};
		};

		ClassDecl(Token *tagToken);
		ClassDecl(Token *tagToken,
				  STRING name,
				  const std::vector<std::pair<MemberPrivilege, IdentifierExpr *>> &parents,
				  const std::vector<std::pair<MemberPrivilege, VarDecl *>> &variables,
				  const std::vector<std::pair<MemberPrivilege, FunctionMember>> &functions,
				  const std::vector<std::pair<MemberPrivilege, EnumDecl *>> &enumerations = {});
		~ClassDecl() override;

#ifndef NDEBUG
		STRING ToString() override;
#endif

		STRING name;
		std::vector<std::pair<MemberPrivilege, IdentifierExpr *>> parents;
		std::vector<std::pair<MemberPrivilege, VarDecl *>> variables;
		std::vector<std::pair<MemberPrivilege, FunctionMember>> functions;
		std::vector<std::pair<MemberPrivilege, EnumDecl *>> enumerations;
	};

	struct ModuleDecl : public Decl
	{
		ModuleDecl(Token *tagToken);
		ModuleDecl(Token *tagToken,
				   IdentifierExpr *name,
				   const std::vector<VarDecl *> &varItems,
				   const std::vector<ClassDecl *> &classItems,
				   const std::vector<ModuleDecl *> &moduleItems,
				   const std::vector<EnumDecl *> &enumItems,
				   const std::vector<FunctionDecl *> &functionItems);
		~ModuleDecl() override;

#ifndef NDEBUG
		STRING ToString() override;
#endif

		IdentifierExpr *name;
		std::vector<VarDecl *> varItems;
		std::vector<ClassDecl *> classItems;
		std::vector<ModuleDecl *> moduleItems;
		std::vector<EnumDecl *> enumItems;
		std::vector<FunctionDecl *> functionItems;
	};
}