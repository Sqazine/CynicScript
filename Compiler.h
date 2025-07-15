#pragma once
#include "Chunk.h"
#include "Ast.h"
#include "Object.h"
#include "Common.h"
namespace CynicScript
{
	struct Symbol;
	class SymbolTable;
	class CYS_API Compiler:public NonCopyable
	{
	public:
		Compiler();
		~Compiler();

		FunctionObject *Compile(Stmt *stmt);

		void ResetStatus();

	private:
		enum class RWState // read write state
		{
			READ,
			WRITE,
		};

		void CompileDecl(Decl *decl);
		void CompileVarDecl(VarDecl *decl);
		void CompileFunctionDecl(FunctionDecl *decl);
		void CompileClassDecl(ClassDecl *decl);
		void CompileEnumDecl(EnumDecl *decl);
		void CompileModuleDecl(ModuleDecl *decl);

		void CompileDeclAndStmt(Stmt *stmt);

		void CompileStmt(Stmt *stmt);
		void CompileExprStmt(ExprStmt *stmt);
		void CompileIfStmt(IfStmt *stmt);
		void CompileScopeStmt(ScopeStmt *stmt);
		void CompileWhileStmt(WhileStmt *stmt);
		void CompileReturnStmt(ReturnStmt *stmt);
		void CompileBreakStmt(BreakStmt *stmt);
		void CompileContinueStmt(ContinueStmt *stmt);

		void CompileExpr(Expr *expr, const RWState &state = RWState::READ, int8_t paramCount = -1);
		void CompileInfixExpr(InfixExpr *expr);
		void CompileLiteralExpr(LiteralExpr *expr);
		void CompilePrefixExpr(PrefixExpr *expr);
		void CompilePostfixExpr(PostfixExpr *expr, const RWState &state = RWState::READ, bool isDelayCompile = true);
		void CompileConditionExpr(ConditionExpr *expr);
		void CompileGroupExpr(GroupExpr *expr);
		void CompileArrayExpr(ArrayExpr *expr);
		void CompileAppregateExpr(AppregateExpr *expr);
		void CompileDictExpr(DictExpr *expr);
		void CompileIndexExpr(IndexExpr *expr, const RWState &state = RWState::READ);
		void CompileNewExpr(NewExpr *expr);
		void CompileThisExpr(ThisExpr *expr);
		void CompileBaseExpr(BaseExpr *expr);
		void CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state, int8_t paramCount = -1);
		void CompileLambdaExpr(LambdaExpr *expr);
		void CompileCompoundExpr(CompoundExpr *expr);
		void CompileCallExpr(CallExpr *expr);
		void CompileDotExpr(DotExpr *expr, const RWState &state = RWState::READ);
		void CompileRefExpr(RefExpr *expr);
		void CompileStructExpr(StructExpr *expr);
		void CompileVarArgExpr(VarArgExpr *expr, const RWState &state = RWState::READ);
		void CompileFactorialExpr(FactorialExpr *expr, const RWState &state = RWState::READ);

		Symbol CompileFunction(FunctionDecl *decl, ClassDecl::FunctionKind kind = ClassDecl::FunctionKind::NONE);
		uint32_t CompileVars(VarDecl *decl, bool IsInClassOrModuleScope);
		Symbol CompileClass(ClassDecl *decl);

		uint64_t EmitOpCode(OpCode opCode, const Token *token);
		uint64_t Emit(uint8_t opcode);
		uint64_t EmitConstant(const Value &value, const Token *token);
		uint64_t EmitClosure(FunctionObject *function, const Token *token);
		uint64_t EmitReturn(uint8_t retCount, const Token *token);
		uint64_t EmitJump(OpCode opcode, const Token *token);
		void EmitLoop(uint16_t opcode, const Token *token);
		void PatchJump(uint64_t offset);
		uint8_t AddConstant(const Value &value);

		void EmitSymbol(const Symbol &symbol);

		void EnterScope();
		void ExitScope();

		Chunk &CurChunk();
		FunctionObject *CurFunction();
		OpCodeList &CurOpCodeList();

		VarArg GetVarArgFromParameterList(const std::vector<VarDescExpr *> &parameterList);

		std::vector<Expr *> StatsPostfixExprs(AstNode *astNode);

		void ClearStatus();

		std::vector<FunctionObject *> mFunctionList;
		SymbolTable *mSymbolTable;

		int64_t mCurBreakStmtAddress, mCurContinueStmtAddress;
	};
}