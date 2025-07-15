#pragma once
#include <vector>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "Token.h"
#include "Ast.h"
#include "Common.h"
#include "Type.h"
namespace CynicScript
{
	enum class Precedence
	{
		LOWEST = 0,	 // ,
		ASSIGN,		 // = += -= *= /= %= &= |= ^= <<= >>=
		CONDITION,	 // ?:
		OR,			 // ||
		AND,		 // &&
		BIT_OR,		 // |
		BIT_XOR,	 // ^
		BIT_AND,	 // &
		EQUAL,		 // == !=
		COMPARE,	 // < <= > >=
		BIT_SHIFT,	 // >> <<
		ADD_PLUS,	 // + -
		MUL_DIV_MOD, // * / %
		PREFIX,		 // ! ~ - & ++ --
		INFIX,		 // [] () . !
		POSTFIX,	 // ++ -- !
	};

	enum class Associativity
	{
		L2R, // left->right
		R2L	 // right->left
	};

	struct ClassInfo
	{
		ClassInfo *enclosing = nullptr;
		bool hasSuperClass = false;
		STRING name;
	};

	class Parser;

	typedef Expr *(Parser::*PrefixFn)();
	typedef Expr *(Parser::*InfixFn)(Expr *);
	typedef Expr *(Parser::*PostfixFn)(Expr *);

	class CYS_API Parser:public NonCopyable
	{
	public:
		Parser();
		~Parser();

		Stmt *Parse(const std::vector<Token *> &tokens);

	private:
		void ResetStatus();

		Decl *ParseDecl();
		Decl *ParseVarDecl();
		Decl *ParseFunctionDecl();
		Decl *ParseClassDecl();
		Decl *ParseEnumDecl();
		Decl *ParseModuleDecl();

		Stmt *ParseDeclAndStmt();

		Stmt *ParseStmt();
		Stmt *ParseExprStmt();
		Stmt *ParseReturnStmt();
		Stmt *ParseIfStmt();
		Stmt *ParseScopeStmt();
		Stmt *ParseWhileStmt();
		Stmt *ParseForStmt();
		Stmt *ParseBreakStmt();
		Stmt *ParseContinueStmt();
		Stmt *ParseSwitchStmt();

		Expr *ParseExpr(Precedence precedence = Precedence::LOWEST);
		Expr *ParseIdentifierExpr();
		Expr *ParseLiteralExpr();
		Expr *ParseGroupExpr();
		Expr *ParseArrayExpr();
		Expr *ParseDictExpr();
		Expr *ParseStructExpr();
		Expr *ParsePrefixExpr();
		Expr *ParseRefExpr();
		Expr *ParseLambdaExpr();
		Expr *ParseNewExpr();
		Expr *ParseThisExpr();
		Expr *ParseBaseExpr();
		Expr *ParseMatchExpr();
		Expr *ParseCompoundExpr();
		Expr *ParseInfixExpr(Expr *prefixExpr);
		Expr *ParsePostfixExpr(Expr *prefixExpr);
		Expr *ParseConditionExpr(Expr *prefixExpr);
		Expr *ParseIndexExpr(Expr *prefixExpr);
		Expr *ParseCallExpr(Expr *prefixExpr);
		Expr *ParseDotExpr(Expr *prefixExpr);
		Expr *ParseFactorialExpr(Expr *prefixExpr);
		Expr *ParseVarDescExpr();
		Expr *ParseVarArgExpr();
		std::pair<Expr *, Expr *> ParseDestructuringAssignmentExpr();

		Type ParseType();

		ClassDecl::MemberPrivilege ParseClassMemberPrivilege();

		Token *GetCurToken();
		Token *GetCurTokenAndStepOnce();
		Precedence GetCurTokenPrecedence();
		Associativity GetCurTokenAssociativity();

		Token *GetNextToken();
		Token *GetNextTokenAndStepOnce();
		Precedence GetNextTokenPrecedence();

		bool IsMatchCurToken(TokenKind kind);
		bool IsMatchCurTokenAndStepOnce(TokenKind kind);
		bool IsMatchNextToken(TokenKind kind);
		bool IsMatchNextTokenAndStepOnce(TokenKind kind);

		Token *Consume(TokenKind kind, STRING_VIEW errMsg);
		Token *Consume(const std::vector<TokenKind> &kinds, STRING_VIEW errMsg);

		bool IsAtEnd();

		ClassInfo *mCurClassInfo;

		std::vector<TokenKind> mSkippingConsumeTokenKindStack; // skip token while call consume function

		int64_t mCurPos;
		std::vector<Token *> mTokens;

		static std::unordered_map<TokenKind, PrefixFn> mPrefixFunctions;
		static std::unordered_map<TokenKind, InfixFn> mInfixFunctions;
		static std::unordered_map<TokenKind, PostfixFn> mPostfixFunctions;
	};
}