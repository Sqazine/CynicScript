#pragma once
#include "Ast.h"
#include "Logger.h"
#include <vector>
#include <type_traits>
#include <memory>
namespace CynicScript
{
    class AstOptimizePass;
    template <typename T>
    concept IsChildOfAstOptimizePass = !std::is_same_v<T, void> &&
                                       !std::is_abstract_v<T> &&
                                       std::is_base_of_v<AstOptimizePass, T>;

    class CYS_API AstOptimizePass
    {
    public:
        AstOptimizePass() noexcept = default;
        virtual ~AstOptimizePass() = default;

        Stmt *Execute(Stmt *stmt)
        {
            return ExecuteStmt(stmt);
        }

    protected:
        Stmt *ExecuteStmt(Stmt *stmt)
        {
            switch (stmt->kind)
            {
            case AstKind::EXPR:
                return ExecuteExprStmt((ExprStmt *)stmt);
            case AstKind::RETURN:
                return ExecuteReturnStmt((ReturnStmt *)stmt);
            case AstKind::IF:
                return ExecuteIfStmt((IfStmt *)stmt);
            case AstKind::SCOPE:
                return ExecuteScopeStmt((ScopeStmt *)stmt);
            case AstKind::WHILE:
                return ExecuteWhileStmt((WhileStmt *)stmt);
            case AstKind::BREAK:
                return ExecuteBreakStmt((BreakStmt *)stmt);
            case AstKind::CONTINUE:
                return ExecuteContinueStmt((ContinueStmt *)stmt);
            case AstKind::VAR:
            case AstKind::ENUM:
            case AstKind::FUNCTION:
            case AstKind::CLASS:
            case AstKind::MODULE:
                return ExecuteDecl((Decl *)stmt);
            case AstKind::ASTSTMTS:
                return ExecuteAstStmts((AstStmts *)stmt);
            default:
                return stmt;
            }
        }

        Decl *ExecuteDecl(Decl *decl)
        {
            switch (decl->kind)
            {
            case AstKind::VAR:
                return ExecuteVarDecl((VarDecl *)decl);
            case AstKind::ENUM:
                return ExecuteEnumDecl((EnumDecl *)decl);
            case AstKind::FUNCTION:
                return ExecuteFunctionDecl((FunctionDecl *)decl);
            case AstKind::CLASS:
                return ExecuteClassDecl((ClassDecl *)decl);
            case AstKind::MODULE:
                return ExecuteModuleDecl((ModuleDecl *)decl);
            default:
                return decl;
            }
        }
        virtual Stmt *ExecuteAstStmts(AstStmts *stmt) { return stmt; }

        virtual Decl *ExecuteVarDecl(VarDecl *decl) { return decl; }
        virtual Decl *ExecuteEnumDecl(EnumDecl *decl) { return decl; }
        virtual Decl *ExecuteFunctionDecl(FunctionDecl *decl) { return decl; }
        virtual Decl *ExecuteModuleDecl(ModuleDecl *decl) { return decl; }
        virtual Decl *ExecuteClassDecl(ClassDecl *decl) { return decl; }

        virtual Stmt *ExecuteExprStmt(ExprStmt *stmt) { return stmt; }
        virtual Stmt *ExecuteReturnStmt(ReturnStmt *stmt) { return stmt; }
        virtual Stmt *ExecuteIfStmt(IfStmt *stmt) { return stmt; }
        virtual Stmt *ExecuteScopeStmt(ScopeStmt *stmt) { return stmt; }
        virtual Stmt *ExecuteWhileStmt(WhileStmt *stmt) { return stmt; }
        virtual Stmt *ExecuteBreakStmt(BreakStmt *stmt) { return stmt; }
        virtual Stmt *ExecuteContinueStmt(ContinueStmt *stmt) { return stmt; }

        virtual Expr *ExecuteExpr(Expr *expr)
        {
            switch (expr->kind)
            {
            case AstKind::LITERAL:
                return ExecuteLiteralExpr((LiteralExpr *)expr);
            case AstKind::IDENTIFIER:
                return ExecuteIdentifierExpr((IdentifierExpr *)expr);
            case AstKind::VAR_DESC:
                return ExecuteVarArgExpr((VarArgExpr *)expr);
            case AstKind::GROUP:
                return ExecuteGroupExpr((GroupExpr *)expr);
            case AstKind::ARRAY:
                return ExecuteArrayExpr((ArrayExpr *)expr);
            case AstKind::DICT:
                return ExecuteDictExpr((DictExpr *)expr);
            case AstKind::PREFIX:
                return ExecutePrefixExpr((PrefixExpr *)expr);
            case AstKind::INFIX:
                return ExecuteInfixExpr((InfixExpr *)expr);
            case AstKind::POSTFIX:
                return ExecutePostfixExpr((PostfixExpr *)expr);
            case AstKind::CONDITION:
                return ExecuteConditionExpr((ConditionExpr *)expr);
            case AstKind::REF:
                return ExecuteRefExpr((RefExpr *)expr);
            case AstKind::LAMBDA:
                return ExecuteLambdaExpr((LambdaExpr *)expr);
            case AstKind::DOT:
                return ExecuteDotExpr((DotExpr *)expr);
            case AstKind::CALL:
                return ExecuteCallExpr((CallExpr *)expr);
            case AstKind::NEW:
                return ExecuteNewExpr((NewExpr *)expr);
            case AstKind::THIS:
                return ExecuteThisExpr((ThisExpr *)expr);
            case AstKind::BASE:
                return ExecuteBaseExpr((BaseExpr *)expr);
            case AstKind::COMPOUND:
                return ExecuteCompoundExpr((CompoundExpr *)expr);
            case AstKind::STRUCT:
                return ExecuteStructExpr((StructExpr *)expr);
            case AstKind::VAR_ARG:
                return ExecuteVarArgExpr((VarArgExpr *)expr);
            case AstKind::FACTORIAL:
                return ExecuteFactorialExpr((FactorialExpr *)expr);
            case AstKind::APPREGATE:
                return ExecuteAppregateExpr((AppregateExpr *)expr);
            default:
                return expr;
            }
        }
        virtual Expr *ExecuteLiteralExpr(LiteralExpr *expr) { return expr; }
        virtual Expr *ExecuteIdentifierExpr(IdentifierExpr *expr) { return expr; }
        virtual Expr *ExecuteGroupExpr(GroupExpr *expr) { return expr; }
        virtual Expr *ExecuteArrayExpr(ArrayExpr *expr) { return expr; }
        virtual Expr *ExecuteAppregateExpr(AppregateExpr *expr) { return expr; }
        virtual Expr *ExecuteDictExpr(DictExpr *expr) { return expr; }
        virtual Expr *ExecutePrefixExpr(PrefixExpr *expr) { return expr; }
        virtual Expr *ExecuteInfixExpr(InfixExpr *expr) { return expr; }
        virtual Expr *ExecutePostfixExpr(PostfixExpr *expr) { return expr; }
        virtual Expr *ExecuteConditionExpr(ConditionExpr *expr) { return expr; }
        virtual Expr *ExecuteIndexExpr(IndexExpr *expr) { return expr; }
        virtual Expr *ExecuteRefExpr(RefExpr *expr) { return expr; }
        virtual Expr *ExecuteStructExpr(StructExpr *expr) { return expr; }
        virtual Expr *ExecuteVarArgExpr(VarArgExpr *expr) { return expr; }
        virtual Expr *ExecuteLambdaExpr(LambdaExpr *expr) { return expr; }
        virtual Expr *ExecuteCompoundExpr(CompoundExpr *expr) { return expr; }
        virtual Expr *ExecuteDotExpr(DotExpr *expr) { return expr; }
        virtual Expr *ExecuteCallExpr(CallExpr *expr) { return expr; }
        virtual Expr *ExecuteNewExpr(NewExpr *expr) { return expr; }
        virtual Expr *ExecuteThisExpr(ThisExpr *expr) { return expr; }
        virtual Expr *ExecuteBaseExpr(BaseExpr *expr) { return expr; }
        virtual Expr *ExecuteFactorialExpr(FactorialExpr *expr) { return expr; }
        virtual Expr *ExecuteVarDescExpr(VarDescExpr *expr) { return expr; }

    protected:
        template <typename T>
        requires IsChildOfAstOptimizePass<T>
        void RequirePass();

    private:
        friend class AstOptimizePassManager;
        AstOptimizePassManager *mOwner;
    };

    class AstOptimizePassManager
    {
    public:
        constexpr AstOptimizePassManager() noexcept = default;
        constexpr ~AstOptimizePassManager() noexcept = default;

        template <typename T, typename... Args>
        requires IsChildOfAstOptimizePass<T>
            AstOptimizePassManager *Add(Args &&...params) noexcept
        {
            for (size_t pos = 0; pos < mPasses.size(); ++pos)
            {
                if (dynamic_cast<T *>(mPasses[pos].get())) // ignore already exists pass
                    return this;
            }

            std::unique_ptr<T> pass = std::make_unique<T>(std::forward<Args>(params)...);
            pass->mOwner = this;

            mPasses.emplace_back(std::move(pass));
            return this;
        }

        Stmt* Execute(Stmt *stmt)
        {
            Stmt* result = stmt;
            for (auto &pass : mPasses)
                pass->Execute(result);
            return result;
        }

    private:
        std::vector<std::unique_ptr<AstOptimizePass>> mPasses;

        friend class AstOptimizePass;

        template <typename T>
        requires IsChildOfAstOptimizePass<T>
        bool HasPass()
        {
            for (int32_t pos; pos < mPasses.size(); ++pos)
            {
                if (dynamic_cast<T *>(mPasses[pos].get()))
                    return true;
            }
            return false;
        }
    };

    template <typename T>
    requires IsChildOfAstOptimizePass<T>
    inline void AstOptimizePass::RequirePass()
    {
        if (!mOwner->HasPass<T>())
            mOwner->Add<T>();
    }
}