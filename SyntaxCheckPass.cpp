#include "SyntaxCheckPass.h"
#include "Common.h"
#include "Logger.h"

namespace CynicScript
{
    SyntaxCheckPass::SyntaxCheckPass() noexcept
        : mLoopDepth(0), mVarArgScopeDepth(false), mClassScopeDepth(0)
    {
    }

    Stmt *SyntaxCheckPass::ExecuteAstStmts(AstStmts *stmt)
    {
        for (auto &s : stmt->stmts)
            s = ExecuteStmt(s);
        return stmt;
    }

    Decl *SyntaxCheckPass::ExecuteVarDecl(VarDecl *decl)
    {
        for (auto &[k, v] : decl->variables)
        {
            if (k->kind == AstKind::ARRAY)
            {
                auto arrayK = (ArrayExpr *)k;
                for (int32_t i = 0; i < arrayK->elements.size(); ++i)
                {
                    auto e = arrayK->elements[i];
                    e = ExecuteExpr(e);
                    if (e->kind != AstKind::VAR_DESC && e->kind != AstKind::VAR_ARG)
                        CYS_LOG_ERROR_WITH_LOC(e->tagToken, TEXT("only variable description or variable argument is available in destructing assign expr."));

                    if (e->kind == AstKind::VAR_ARG && i != arrayK->elements.size() - 1)
                        CYS_LOG_ERROR_WITH_LOC(k->tagToken, TEXT("variable argument expr only available at the end of destructing assignment expr."));
                }
            }
            else if (k->kind == AstKind::VAR_DESC)
                k = ExecuteExpr(k);
            else
                CYS_LOG_ERROR_WITH_LOC(k->tagToken, TEXT("only destructing assign expr or variable description is available in let/const stmt's binding part."));

            v = ExecuteExpr(v);
        }
        return decl;
    }

    Stmt *SyntaxCheckPass::ExecuteExprStmt(ExprStmt *stmt)
    {
        stmt->expr = ExecuteExpr(stmt->expr);
        return stmt;
    }
    Stmt *SyntaxCheckPass::ExecuteIfStmt(IfStmt *stmt)
    {
        return stmt;
    }
    Stmt *SyntaxCheckPass::ExecuteScopeStmt(ScopeStmt *stmt)
    {
        for (auto &s : stmt->stmts)
            s = ExecuteStmt(s);
        return stmt;
    }
    Stmt *SyntaxCheckPass::ExecuteWhileStmt(WhileStmt *stmt)
    {
        stmt->condition = ExecuteExpr(stmt->condition);
        stmt->body = (ScopeStmt *)ExecuteScopeStmt(stmt->body);
        if (stmt->increment)
            stmt->increment = (ScopeStmt *)ExecuteScopeStmt(stmt->increment);
        return stmt;
    }
    Decl *SyntaxCheckPass::ExecuteEnumDecl(EnumDecl *decl)
    {
        decl->name = (IdentifierExpr *)ExecuteIdentifierExpr(decl->name);
        for (auto &[k, v] : decl->enumItems)
            v = ExecuteExpr(v);
        return decl;
    }
    Decl *SyntaxCheckPass::ExecuteFunctionDecl(FunctionDecl *decl)
    {
        for (int32_t i = 0; i < decl->parameters.size(); ++i)
        {
            auto e = decl->parameters[i];
            e = (VarDescExpr *)ExecuteVarDescExpr(e);

            if (e->name->kind == AstKind::VAR_ARG && i != decl->parameters.size() - 1)
                CYS_LOG_ERROR_WITH_LOC(e->tagToken, TEXT("variable argument expr only available at the end of function parameter list."));
        }

        decl->body = (ScopeStmt *)ExecuteScopeStmt(decl->body);
        return decl;
    }
    Decl *SyntaxCheckPass::ExecuteClassDecl(ClassDecl *decl)
    {
        return decl;
    }
    Stmt *SyntaxCheckPass::ExecuteReturnStmt(ReturnStmt *stmt)
    {
        if (stmt->expr)
            stmt->expr = ExecuteExpr(stmt->expr);
        return stmt;
    }
    Stmt *SyntaxCheckPass::ExecuteBreakStmt(BreakStmt *stmt)
    {
        return stmt;
    }
    Stmt *SyntaxCheckPass::ExecuteContinueStmt(ContinueStmt *stmt)
    {
        return stmt;
    }

    Decl *SyntaxCheckPass::ExecuteModuleDecl(ModuleDecl *decl)
    {
        return decl;
    }

    Expr *SyntaxCheckPass::ExecuteInfixExpr(InfixExpr *expr)
    {
        expr->left = ExecuteExpr(expr->left);
        expr->right = ExecuteExpr(expr->right);
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteLiteralExpr(LiteralExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxCheckPass::ExecutePrefixExpr(PrefixExpr *expr)
    {
        expr->right = ExecuteExpr(expr->right);
        return expr;
    }
    Expr *SyntaxCheckPass::ExecutePostfixExpr(PostfixExpr *expr)
    {
        expr->left = ExecuteExpr(expr->left);
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteConditionExpr(ConditionExpr *expr)
    {
        expr->condition = ExecuteExpr(expr->condition);
        expr->trueBranch = ExecuteExpr(expr->trueBranch);
        expr->falseBranch = ExecuteExpr(expr->falseBranch);
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteGroupExpr(GroupExpr *expr)
    {
        expr->expr = ExecuteExpr(expr->expr);
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteArrayExpr(ArrayExpr *expr)
    {
        for (auto &e : expr->elements)
            e = ExecuteExpr(e);
        return expr;
    }

    Expr *SyntaxCheckPass::ExecuteAppregateExpr(AppregateExpr *expr)
    {
        return expr;
    }

    Expr *SyntaxCheckPass::ExecuteDictExpr(DictExpr *expr)
    {
        for (auto &[k, v] : expr->elements)
        {
            if (!IsConstantLiteral(k))
                CYS_LOG_ERROR_WITH_LOC(k->tagToken, TEXT("Dict keys can only have constant literals (int num,real num,string,boolean,null or variable identifier)."));
            v = ExecuteExpr(v);
        }
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteIndexExpr(IndexExpr *expr)
    {
        expr->ds = ExecuteExpr(expr->ds);
        expr->index = ExecuteExpr(expr->index);
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteNewExpr(NewExpr *expr)
    {
        if (expr->callee->kind != AstKind::CALL)
            CYS_LOG_ERROR_WITH_LOC(expr->callee->tagToken, TEXT("Not a valid new expr,call expr is necessary followed 'new' keyword."));

        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteThisExpr(ThisExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteBaseExpr(BaseExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteIdentifierExpr(IdentifierExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteLambdaExpr(LambdaExpr *expr)
    {
        for (int32_t i = 0; i < expr->parameters.size(); ++i)
        {
            auto e = expr->parameters[i];
            e = (VarDescExpr *)ExecuteVarDescExpr(e);

            if (e->name->kind == AstKind::VAR_ARG && i != expr->parameters.size() - 1)
                CYS_LOG_ERROR_WITH_LOC(e->tagToken, TEXT("variable argument expr only available at the end of lambda parameter list."));
        }

        expr->body = (ScopeStmt *)ExecuteScopeStmt(expr->body);
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteCompoundExpr(CompoundExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteCallExpr(CallExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteDotExpr(DotExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteRefExpr(RefExpr *expr)
    {
        if (expr->refExpr->kind != AstKind::IDENTIFIER && expr->refExpr->kind != AstKind::INDEX)
            CYS_LOG_ERROR_WITH_LOC(expr->tagToken, TEXT("Only left value is available for reference."));
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteStructExpr(StructExpr *expr)
    {
        return expr;
    }
    Expr *SyntaxCheckPass::ExecuteVarArgExpr(VarArgExpr *expr)
    {
        return expr;
    }

    Expr *SyntaxCheckPass::ExecuteFactorialExpr(FactorialExpr *expr)
    {
        return expr;
    }

    Expr *SyntaxCheckPass::ExecuteVarDescExpr(VarDescExpr *expr)
    {
        if (expr->name->kind != AstKind::IDENTIFIER && expr->name->kind != AstKind::VAR_ARG)
            CYS_LOG_ERROR_WITH_LOC(expr->tagToken, TEXT("Only identifier or variable argument is available at variable declaration or param declaration"));
        return expr;
    }

    bool SyntaxCheckPass::IsConstantLiteral(Expr *expr)
    {
        switch (expr->kind)
        {
        case AstKind::LITERAL:
        case AstKind::IDENTIFIER:
            return true;
        default:
            return false;
        }
        return false;
    }
}