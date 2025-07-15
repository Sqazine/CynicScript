#include "ConstantFoldPass.h"
#include "Common.h"

namespace CynicScript
{
	Stmt *ConstantFoldPass::ExecuteAstStmts(AstStmts *stmt)
	{
		for (auto &s : stmt->stmts)
			s = ExecuteStmt(s);
		return stmt;
	}
	Stmt *ConstantFoldPass::ExecuteExprStmt(ExprStmt *stmt)
	{
		stmt->expr = ExecuteExpr(stmt->expr);
		return stmt;
	}
	Stmt *ConstantFoldPass::ExecuteIfStmt(IfStmt *stmt)
	{
		stmt->condition = ExecuteExpr(stmt->condition);
		stmt->thenBranch = ExecuteStmt(stmt->thenBranch);
		if (stmt->elseBranch)
			stmt->elseBranch = ExecuteStmt(stmt->elseBranch);

		if (stmt->condition->kind == AstKind::LITERAL && ((LiteralExpr *)stmt->condition)->type.Is(TypeKind::BOOL))
		{
			if (((LiteralExpr *)stmt->condition)->boolean == true)
				return stmt->thenBranch;
			else
				return stmt->elseBranch;
		}

		return stmt;
	}
	Stmt *ConstantFoldPass::ExecuteScopeStmt(ScopeStmt *stmt)
	{
		for (auto &s : stmt->stmts)
			s = ExecuteStmt(s);
		return stmt;
	}
	Stmt *ConstantFoldPass::ExecuteWhileStmt(WhileStmt *stmt)
	{
		stmt->condition = ExecuteExpr(stmt->condition);
		stmt->body = (ScopeStmt *)ExecuteScopeStmt(stmt->body);
		return stmt;
	}
	Decl *ConstantFoldPass::ExecuteEnumDecl(EnumDecl *decl)
	{
		for (auto &[k, v] : decl->enumItems)
			v = ExecuteExpr(v);
		return decl;
	}
	Stmt *ConstantFoldPass::ExecuteReturnStmt(ReturnStmt *stmt)
	{
		return stmt;
	}
	Decl *ConstantFoldPass::ExecuteVarDecl(VarDecl *decl)
	{
		for (auto &[k, v] :decl->variables)
			v = ExecuteExpr(v);
		return decl;
	}

	Decl *ConstantFoldPass::ExecuteFunctionDecl(FunctionDecl *decl)
	{
		for (auto &e : decl->parameters)
			e = (VarDescExpr *)ExecuteVarDescExpr(e);

		decl->body = (ScopeStmt *)ExecuteScopeStmt(decl->body);

		return decl;
	}

	Decl *ConstantFoldPass::ExecuteClassDecl(ClassDecl *decl)
	{
		for (auto &varStmt : decl->variables)
			varStmt.second = (VarDecl *)ExecuteVarDecl(varStmt.second);

		for (auto &fnStmt : decl->functions)
			fnStmt.second.decl = (FunctionDecl *)ExecuteFunctionDecl(fnStmt.second.decl);

		return decl;
	}

	Stmt *ConstantFoldPass::ExecuteBreakStmt(BreakStmt *stmt)
	{
		return stmt;
	}

	Stmt *ConstantFoldPass::ExecuteContinueStmt(ContinueStmt *stmt)
	{
		return stmt;
	}

	Decl *ConstantFoldPass::ExecuteModuleDecl(ModuleDecl *decl)
	{
		return decl;
	}

	Expr *ConstantFoldPass::ExecuteInfixExpr(InfixExpr *expr)
	{
		expr->left = ExecuteExpr(expr->left);
		expr->right = ExecuteExpr(expr->right);

		return ConstantFold(expr);
	}
	Expr *ConstantFoldPass::ExecutePostfixExpr(PostfixExpr *expr)
	{
		expr->left = ExecuteExpr(expr->left);
		return ConstantFold(expr);
	}
	Expr *ConstantFoldPass::ExecuteConditionExpr(ConditionExpr *expr)
	{
		expr->condition = ExecuteExpr(expr->condition);
		expr->trueBranch = ExecuteExpr(expr->trueBranch);
		expr->falseBranch = ExecuteExpr(expr->falseBranch);

		if (expr->condition->kind == AstKind::LITERAL && ((LiteralExpr *)expr->condition)->type.Is(TypeKind::BOOL))
		{
			if (((LiteralExpr *)expr->condition)->boolean == true)
				return expr->trueBranch;
			else
				return expr->falseBranch;
		}

		return expr;
	}
	Expr *ConstantFoldPass::ExecuteLiteralExpr(LiteralExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ExecutePrefixExpr(PrefixExpr *expr)
	{
		expr->right = ExecuteExpr(expr->right);
		return ConstantFold(expr);
	}

	Expr *ConstantFoldPass::ExecuteGroupExpr(GroupExpr *expr)
	{
		return ExecuteExpr(expr->expr);
	}
	Expr *ConstantFoldPass::ExecuteArrayExpr(ArrayExpr *expr)
	{
		for (auto &e : expr->elements)
			e = ExecuteExpr(e);
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteAppregateExpr(AppregateExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteDictExpr(DictExpr *expr)
	{
		for (auto &[k, v] : expr->elements)
		{
			k = ExecuteExpr(k);
			v = ExecuteExpr(v);
		}
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteIndexExpr(IndexExpr *expr)
	{
		expr->ds = ExecuteExpr(expr->ds);
		expr->index = ExecuteExpr(expr->index);
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteIdentifierExpr(IdentifierExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteLambdaExpr(LambdaExpr *expr)
	{
		for (auto &e : expr->parameters)
			e = (VarDescExpr *)ExecuteVarDescExpr(e);
		expr->body = (ScopeStmt *)ExecuteScopeStmt(expr->body);
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteCompoundExpr(CompoundExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteDotExpr(DotExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteCallExpr(CallExpr *expr)
	{
		expr->callee = ExecuteExpr(expr->callee);
		for (auto &arg : expr->arguments)
			arg = ExecuteExpr(arg);
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteNewExpr(NewExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteThisExpr(ThisExpr *expr)
	{
		return expr;
	}
	Expr *ConstantFoldPass::ExecuteBaseExpr(BaseExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteFactorialExpr(FactorialExpr *expr)
	{
		if (expr->expr->kind == AstKind::LITERAL && ((LiteralExpr *)expr->expr)->type.IsInteger())
		{
			auto literalExpr = (LiteralExpr *)expr->expr;
			auto intExpr = new LiteralExpr(expr->tagToken, Factorial(literalExpr->i64Value));
			SAFE_DELETE(expr);
			return intExpr;
		}
		else
		{
			expr->expr = ExecuteExpr(expr->expr);
			return expr;
		}
	}

	Expr *ConstantFoldPass::ExecuteVarDescExpr(VarDescExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteRefExpr(RefExpr *expr)
	{
		expr->refExpr = (IdentifierExpr *)ExecuteExpr(expr->refExpr);
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteStructExpr(StructExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ExecuteVarArgExpr(VarArgExpr *expr)
	{
		return expr;
	}

	Expr *ConstantFoldPass::ConstantFold(Expr *expr)
	{
		if (expr->kind == AstKind::INFIX)
		{
			auto infix = (InfixExpr *)expr;
			if (infix->left->kind == AstKind::LITERAL && infix->right->kind == AstKind::LITERAL)
			{
				Expr *newExpr = infix;
				auto tagToken = infix->tagToken;
				bool needToDelete = true;

				auto leftLiteral = ((LiteralExpr *)infix->left);
				auto rightLiteral = ((LiteralExpr *)infix->right);

				if (leftLiteral->type.IsFloating() && rightLiteral->type.IsFloating())
				{
#define BIN_EXPR(x)            \
	if (infix->op == TEXT(#x)) \
	newExpr = new LiteralExpr(tagToken, leftLiteral->f64Value x rightLiteral->f64Value)

					BIN_EXPR(+);
					else BIN_EXPR(-);
					else BIN_EXPR(*);
					else BIN_EXPR(/);
					else BIN_EXPR(==);
					else BIN_EXPR(!=);
					else BIN_EXPR(>);
					else BIN_EXPR(>=);
					else BIN_EXPR(<);
					else BIN_EXPR(<=);
					else needToDelete = false;
#undef BIN_EXPR
					if (needToDelete)
						SAFE_DELETE(infix);
					return newExpr;
				}
				else if (leftLiteral->type.IsInteger() && rightLiteral->type.IsInteger())
				{
#define BIN_EXPR(x)            \
	if (infix->op == TEXT(#x)) \
	newExpr = new LiteralExpr(tagToken, leftLiteral->i64Value x rightLiteral->i64Value)

					BIN_EXPR(+);
					else BIN_EXPR(-);
					else BIN_EXPR(*);
					else BIN_EXPR(/);
					else BIN_EXPR(%);
					else BIN_EXPR(&);
					else BIN_EXPR(|);
					else BIN_EXPR(^);
					else BIN_EXPR(==);
					else BIN_EXPR(!=);
					else BIN_EXPR(>);
					else BIN_EXPR(>=);
					else BIN_EXPR(<);
					else BIN_EXPR(<=);
					else BIN_EXPR(<<);
					else BIN_EXPR(>>);
					else needToDelete = false;
#undef BIN_EXPR
					if (needToDelete)
						SAFE_DELETE(infix);
					return newExpr;
				}
				else if (leftLiteral->type.IsInteger() && rightLiteral->type.IsFloating())
				{
#define BIN_EXPR(x)            \
	if (infix->op == TEXT(#x)) \
	newExpr = new LiteralExpr(tagToken, leftLiteral->i64Value x rightLiteral->f64Value)

					BIN_EXPR(+);
					else BIN_EXPR(-);
					else BIN_EXPR(*);
					else BIN_EXPR(/);
					else BIN_EXPR(==);
					else BIN_EXPR(!=);
					else BIN_EXPR(>);
					else BIN_EXPR(>=);
					else BIN_EXPR(<);
					else BIN_EXPR(<=);
					else needToDelete = false;
#undef BIN_EXPR
					if (needToDelete)
						SAFE_DELETE(infix);
					return newExpr;
				}
				else if (leftLiteral->type.IsFloating() && rightLiteral->type.IsInteger())
				{

#define BIN_EXPR(x)            \
	if (infix->op == TEXT(#x)) \
	newExpr = new LiteralExpr(tagToken, leftLiteral->f64Value x rightLiteral->i64Value)

					BIN_EXPR(+);
					else BIN_EXPR(-);
					else BIN_EXPR(*);
					else BIN_EXPR(/);
					else BIN_EXPR(==);
					else BIN_EXPR(!=);
					else BIN_EXPR(>);
					else BIN_EXPR(>=);
					else BIN_EXPR(<);
					else BIN_EXPR(<=);
					else needToDelete = false;
#undef BIN_EXPR
					if (needToDelete)
						SAFE_DELETE(infix);
					return newExpr;
				}
				else if (leftLiteral->type.Is(TypeKind::STR) && rightLiteral->type.Is(TypeKind::STR))
				{
					auto strExpr = new LiteralExpr(infix->tagToken, leftLiteral->str + rightLiteral->str);
					SAFE_DELETE(infix);
					return strExpr;
				}
			}
		}
		else if (expr->kind == AstKind::PREFIX)
		{
			auto prefix = (PrefixExpr *)expr;

			if (prefix->right->kind == AstKind::LITERAL)
			{
				auto rightLiteralExpr = ((LiteralExpr *)prefix->right);
				if (rightLiteralExpr->type.IsFloating() && prefix->op == TEXT("-"))
				{
					auto numExpr = new LiteralExpr(prefix->tagToken, -rightLiteralExpr->f64Value);
					SAFE_DELETE(prefix);
					return numExpr;
				}
				else if (rightLiteralExpr->type.IsInteger() && prefix->op == TEXT("-"))
				{
					auto numExpr = new LiteralExpr(prefix->tagToken, -rightLiteralExpr->i64Value);
					SAFE_DELETE(prefix);
					return numExpr;
				}
				else if (rightLiteralExpr->type.Is(TypeKind::BOOL) && prefix->op == TEXT("!"))
				{
					auto boolExpr = new LiteralExpr(prefix->tagToken, !rightLiteralExpr->boolean);
					SAFE_DELETE(prefix);
					return boolExpr;
				}
				else if (rightLiteralExpr->type.IsInteger() && prefix->op == TEXT("~"))
				{
					auto numExpr = new LiteralExpr(prefix->tagToken, ~rightLiteralExpr->i64Value);
					SAFE_DELETE(prefix);
					return numExpr;
				}
			}
		}
		else if (expr->kind == AstKind::POSTFIX)
		{
			auto postfix = (PostfixExpr *)expr;
			if (postfix->left->kind == AstKind::LITERAL)
			{
				auto leftLiteralExpr = (LiteralExpr *)postfix->left;
				if (postfix->op == TEXT("!"))
				{
					auto numExpr = new LiteralExpr(postfix->tagToken, Factorial(leftLiteralExpr->i64Value));
					SAFE_DELETE(postfix);
					return numExpr;
				}
			}
		}

		return expr;
	}
}