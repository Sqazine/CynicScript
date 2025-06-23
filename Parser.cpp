#include "Parser.h"
#include "LibraryManager.h"
#include "Utils.h"
#include "Logger.h"
namespace CynicScript
{
	struct PrecedenceBinding
	{
		TokenKind kind;
		Precedence precedence;
	};

	constexpr PrecedenceBinding precedenceDict[] = {
		{TokenKind::EQUAL, Precedence::ASSIGN},
		{TokenKind::PLUS_EQUAL, Precedence::ASSIGN},
		{TokenKind::MINUS_EQUAL, Precedence::ASSIGN},
		{TokenKind::ASTERISK_EQUAL, Precedence::ASSIGN},
		{TokenKind::SLASH_EQUAL, Precedence::ASSIGN},
		{TokenKind::PERCENT_EQUAL, Precedence::ASSIGN},
		{TokenKind::AMPERSAND_EQUAL, Precedence::ASSIGN},
		{TokenKind::VBAR_EQUAL, Precedence::ASSIGN},
		{TokenKind::CARET_EQUAL, Precedence::ASSIGN},
		{TokenKind::LESS_LESS_EQUAL, Precedence::ASSIGN},
		{TokenKind::GREATER_GREATER_EQUAL, Precedence::ASSIGN},
		{TokenKind::VBAR_VBAR, Precedence::OR},
		{TokenKind::AMPERSAND_AMPERSAND, Precedence::AND},
		{TokenKind::QUESTION, Precedence::CONDITION},
		{TokenKind::VBAR, Precedence::BIT_OR},
		{TokenKind::CARET, Precedence::BIT_XOR},
		{TokenKind::AMPERSAND, Precedence::BIT_AND},
		{TokenKind::EQUAL_EQUAL, Precedence::EQUAL},
		{TokenKind::BANG_EQUAL, Precedence::EQUAL},
		{TokenKind::LESS, Precedence::COMPARE},
		{TokenKind::LESS_EQUAL, Precedence::COMPARE},
		{TokenKind::GREATER, Precedence::COMPARE},
		{TokenKind::GREATER_EQUAL, Precedence::COMPARE},
		{TokenKind::LESS_LESS, Precedence::BIT_SHIFT},
		{TokenKind::GREATER_GREATER, Precedence::BIT_SHIFT},
		{TokenKind::PLUS, Precedence::ADD_PLUS},
		{TokenKind::MINUS, Precedence::ADD_PLUS},
		{TokenKind::ASTERISK, Precedence::MUL_DIV_MOD},
		{TokenKind::SLASH, Precedence::MUL_DIV_MOD},
		{TokenKind::PERCENT, Precedence::MUL_DIV_MOD},
		{TokenKind::LBRACKET, Precedence::INFIX},
		{TokenKind::LPAREN, Precedence::INFIX},
		{TokenKind::DOT, Precedence::INFIX},
		{TokenKind::BANG, Precedence::INFIX},
		{TokenKind::PLUS_PLUS, Precedence::POSTFIX},
		{TokenKind::MINUS_MINUS, Precedence::POSTFIX},
	};

	struct AssociativityBinding
	{
		Precedence precedence;
		Associativity associativity;
	};

	constexpr AssociativityBinding associativityDict[] =
		{
			{Precedence::LOWEST, Associativity::L2R},
			{Precedence::ASSIGN, Associativity::R2L},
			{Precedence::CONDITION, Associativity::L2R},
			{Precedence::OR, Associativity::L2R},
			{Precedence::AND, Associativity::L2R},
			{Precedence::BIT_OR, Associativity::L2R},
			{Precedence::BIT_XOR, Associativity::L2R},
			{Precedence::BIT_AND, Associativity::L2R},
			{Precedence::EQUAL, Associativity::L2R},
			{Precedence::COMPARE, Associativity::L2R},
			{Precedence::BIT_SHIFT, Associativity::L2R},
			{Precedence::ADD_PLUS, Associativity::L2R},
			{Precedence::MUL_DIV_MOD, Associativity::L2R},
			{Precedence::PREFIX, Associativity::R2L},
			{Precedence::INFIX, Associativity::L2R},
			{Precedence::POSTFIX, Associativity::L2R},
	};

	std::unordered_map<TokenKind, PrefixFn> Parser::mPrefixFunctions =
		{
			{TokenKind::IDENTIFIER, &Parser::ParseIdentifierExpr},
			{TokenKind::NUMBER, &Parser::ParseLiteralExpr},
			{TokenKind::STR, &Parser::ParseLiteralExpr},
			{TokenKind::NIL, &Parser::ParseLiteralExpr},
			{TokenKind::TRUE, &Parser::ParseLiteralExpr},
			{TokenKind::FALSE, &Parser::ParseLiteralExpr},
			{TokenKind::CHAR, &Parser::ParseLiteralExpr},
			{TokenKind::MINUS, &Parser::ParsePrefixExpr},
			{TokenKind::TILDE, &Parser::ParsePrefixExpr},
			{TokenKind::BANG, &Parser::ParsePrefixExpr},
			{TokenKind::LPAREN, &Parser::ParseGroupExpr},
			{TokenKind::LBRACKET, &Parser::ParseArrayExpr},
			{TokenKind::LBRACE, &Parser::ParseDictExpr},
			{TokenKind::AMPERSAND, &Parser::ParseRefExpr},
			{TokenKind::FUNCTION, &Parser::ParseLambdaExpr},
			{TokenKind::PLUS_PLUS, &Parser::ParsePrefixExpr},
			{TokenKind::MINUS_MINUS, &Parser::ParsePrefixExpr},
			{TokenKind::NEW, &Parser::ParseNewExpr},
			{TokenKind::THIS, &Parser::ParseThisExpr},
			{TokenKind::BASE, &Parser::ParseBaseExpr},
			{TokenKind::MATCH, &Parser::ParseMatchExpr},
			{TokenKind::LPAREN_LBRACE, &Parser::ParseCompoundExpr},
			{TokenKind::ELLIPSIS, &Parser::ParseVarArgExpr},
			{TokenKind::STRUCT, &Parser::ParseStructExpr},
	};

	std::unordered_map<TokenKind, InfixFn> Parser::mInfixFunctions =
		{
			{TokenKind::EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::PLUS_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::MINUS_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::ASTERISK_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::SLASH_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::PERCENT_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::AMPERSAND_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::VBAR_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::CARET_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::LESS_LESS_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::GREATER_GREATER_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::QUESTION, &Parser::ParseConditionExpr},
			{TokenKind::VBAR_VBAR, &Parser::ParseInfixExpr},
			{TokenKind::AMPERSAND_AMPERSAND, &Parser::ParseInfixExpr},
			{TokenKind::VBAR, &Parser::ParseInfixExpr},
			{TokenKind::CARET, &Parser::ParseInfixExpr},
			{TokenKind::AMPERSAND, &Parser::ParseInfixExpr},
			{TokenKind::LESS_LESS, &Parser::ParseInfixExpr},
			{TokenKind::GREATER_GREATER, &Parser::ParseInfixExpr},
			{TokenKind::EQUAL_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::BANG_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::LESS, &Parser::ParseInfixExpr},
			{TokenKind::LESS_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::GREATER, &Parser::ParseInfixExpr},
			{TokenKind::GREATER_EQUAL, &Parser::ParseInfixExpr},
			{TokenKind::PLUS, &Parser::ParseInfixExpr},
			{TokenKind::MINUS, &Parser::ParseInfixExpr},
			{TokenKind::ASTERISK, &Parser::ParseInfixExpr},
			{TokenKind::SLASH, &Parser::ParseInfixExpr},
			{TokenKind::PERCENT, &Parser::ParseInfixExpr},
			{TokenKind::LPAREN, &Parser::ParseCallExpr},
			{TokenKind::LBRACKET, &Parser::ParseIndexExpr},
			{TokenKind::DOT, &Parser::ParseDotExpr},
			{TokenKind::BANG, &Parser::ParseFactorialExpr},
	};

	std::unordered_map<TokenKind, PostfixFn> Parser::mPostfixFunctions =
		{
			{TokenKind::PLUS_PLUS, &Parser::ParsePostfixExpr},
			{TokenKind::MINUS_MINUS, &Parser::ParsePostfixExpr},
	};

	Parser::Parser()
		: mCurClassInfo(nullptr), mCurPos(0)
	{
	}

	Parser::~Parser()
	{
		std::unordered_map<TokenKind, PrefixFn>().swap(mPrefixFunctions);
		std::unordered_map<TokenKind, InfixFn>().swap(mInfixFunctions);
	}

	Stmt *Parser::Parse(const std::vector<Token *> &tokens)
	{
		ResetStatus();
		mTokens = tokens;

		AstStmts *astStmts = new AstStmts(GetCurToken());

		while (!IsMatchCurToken(TokenKind::END))
			astStmts->stmts.emplace_back(ParseDeclAndStmt());

		return astStmts;
	}

	void Parser::ResetStatus()
	{
		mCurPos = 0;

		mCurClassInfo = nullptr;
	}

	Decl *Parser::ParseDecl()
	{
		switch (GetCurToken()->kind)
		{
		case TokenKind::LET:
		case TokenKind::CONST:
			return ParseVarDecl();
		case TokenKind::FUNCTION:
			GetCurTokenAndStepOnce();
			return ParseFunctionDecl();
		case TokenKind::CLASS:
			return ParseClassDecl();
		case TokenKind::ENUM:
			return ParseEnumDecl();
		case TokenKind::MODULE:
			return ParseModuleDecl();
		default:
			return nullptr;
		}
	}

	Decl *Parser::ParseVarDecl()
	{
		auto curToken = GetCurToken();
		auto kind = curToken->kind;

		auto varStmt = new VarDecl(curToken);

		if (kind == TokenKind::LET)
			varStmt->permission = Permission::MUTABLE;
		else if (kind == TokenKind::CONST)
			varStmt->permission = Permission::IMMUTABLE;

		Consume(kind, TEXT("Expect 'let' or 'const' key word"));

		do
		{
			if (IsMatchCurToken(TokenKind::LBRACKET))
				varStmt->variables.emplace_back(ParseDestructuringAssignmentExpr());
			else
			{
				auto varDescExpr = ParseVarDescExpr();

				Expr *value = new LiteralExpr(varDescExpr->tagToken);
				if (IsMatchCurTokenAndStepOnce(TokenKind::EQUAL))
					value = ParseExpr();
				varStmt->variables.emplace_back(varDescExpr, value);
			}

		} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));

		Consume(TokenKind::SEMICOLON, TEXT("Expect ';' after let or const declaration."));

		return varStmt;
	}

	Decl *Parser::ParseFunctionDecl()
	{
		auto funcStmt = new FunctionDecl(GetCurToken());

		{
			funcStmt->name = (IdentifierExpr *)ParseIdentifierExpr();

			Consume(TokenKind::LPAREN, TEXT("Expect '(' after 'fn' keyword"));

			if (!IsMatchCurToken(TokenKind::RPAREN)) // has parameter
			{
				do
				{
					funcStmt->parameters.emplace_back((VarDescExpr *)ParseVarDescExpr());
				} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));
			}

			Consume(TokenKind::RPAREN, TEXT("Expect ')' after function stmt's '('"));

			std::vector<Type> functionReturnTypes;
			{
				// TODO: parse function return type
				if (IsMatchCurToken(TokenKind::COLON))
				{
					GetCurTokenAndStepOnce();
					do
					{
						auto type = ParseType();
						functionReturnTypes.emplace_back(type);
					} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));
				}
			}

			funcStmt->returnTypes = functionReturnTypes;
			funcStmt->body = (ScopeStmt *)ParseScopeStmt();

			// if (funcStmt->body->stmts.back()->kind != AstKind::RETURN && funcStmt->functionKind != FunctionDecl::Kind::CLASS_CONSTRUCTOR)
			// {
			// 	auto tmpReturn = new ReturnStmt(GetCurToken());
			// 	funcStmt->body->stmts.emplace_back(tmpReturn);
			// }
		}

		return funcStmt;
	}

	Decl *Parser::ParseClassDecl()
	{
		auto classStmt = new ClassDecl(GetCurToken());

		Consume(TokenKind::CLASS, TEXT("Expect 'class' keyword"));

		classStmt->name = ((IdentifierExpr *)ParseIdentifierExpr())->literal;

		ClassInfo classInfo;
		classInfo.hasSuperClass = false;
		classInfo.enclosing = mCurClassInfo;
		classInfo.name = classStmt->name;
		mCurClassInfo = &classInfo;

		if (IsMatchCurTokenAndStepOnce(TokenKind::COLON))
		{
			do
			{
				auto privilege = ParseClassMemberPrivilege();
				classStmt->parents.emplace_back(std::make_pair(privilege, (IdentifierExpr *)ParseIdentifierExpr()));
			} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));

			mCurClassInfo->hasSuperClass = true;
		}

		Consume(TokenKind::LBRACE, TEXT("Expect '{' after class name or parent class name"));

		while (!IsMatchCurToken(TokenKind::RBRACE))
		{
			auto privilege = ParseClassMemberPrivilege();

			if (IsMatchCurToken(TokenKind::LET) || IsMatchCurToken(TokenKind::CONST))
				classStmt->variables.emplace_back(privilege, (VarDecl *)ParseVarDecl());
			else if (IsMatchCurToken(TokenKind::ENUM))
				classStmt->enumerations.emplace_back(privilege, (EnumDecl *)ParseEnumDecl());
			else if (IsMatchCurTokenAndStepOnce(TokenKind::FUNCTION))
			{
				auto fn = (FunctionDecl *)ParseFunctionDecl();
				if (fn->name->literal == classStmt->name)
					CYS_LOG_ERROR_WITH_LOC(fn->name->tagToken, TEXT("The class member function name :{} conflicts with its class:{}, only constructor function name is allowed to same with its class's name"), fn->name->literal);
				classStmt->functions.emplace_back(privilege, std::move(ClassDecl::FunctionMember(ClassDecl::FunctionKind::MEMBER, fn)));
			}
			else if (GetCurToken()->literal == classStmt->name) // constructor
			{
				auto fn = (FunctionDecl *)ParseFunctionDecl();
				classStmt->functions.emplace_back(privilege, std::move(ClassDecl::FunctionMember(ClassDecl::FunctionKind::CONSTRUCTOR, fn)));
			}
			else
				Consume({TokenKind::LET, TokenKind::FUNCTION, TokenKind::CONST}, TEXT("UnExpect identifier '") + GetCurToken()->literal + TEXT("'."));
		}

		Consume(TokenKind::RBRACE, TEXT("Expect '}' after class stmt's '{'"));

		mCurClassInfo = mCurClassInfo->enclosing;

		return classStmt;
	}
	Decl *Parser::ParseEnumDecl()
	{
		auto enumStmt = new EnumDecl(GetCurToken());

		Consume(TokenKind::ENUM, TEXT("Expect 'enum' keyword."));
		enumStmt->name = (IdentifierExpr *)ParseIdentifierExpr();
		Consume(TokenKind::LBRACE, TEXT("Expect '{' after 'enum' keyword."));

		std::unordered_map<IdentifierExpr *, Expr *> items;

		while (!IsMatchCurToken(TokenKind::RBRACE))
		{
			auto name = (IdentifierExpr *)ParseIdentifierExpr();
			Expr *value = new LiteralExpr(name->tagToken, name->literal);
			if (IsMatchCurTokenAndStepOnce(TokenKind::EQUAL))
			{
				SAFE_DELETE(value);
				value = ParseExpr();
			}

			items[name] = value;

			if (IsMatchCurToken(TokenKind::COMMA))
				Consume(TokenKind::COMMA, TEXT("Expect ',' after enum item."));
		}

		Consume(TokenKind::RBRACE, TEXT("Expect '}' at the end of the 'enum' stmt."));

		enumStmt->enumItems = items;

		return enumStmt;
	}

	Decl *Parser::ParseModuleDecl()
	{
		auto token = Consume(TokenKind::MODULE, TEXT("Expect 'module' keyword."));

		auto moduleDecl = new ModuleDecl(token);

		moduleDecl->name = (IdentifierExpr *)ParseIdentifierExpr();

		Consume(TokenKind::LBRACE, TEXT("Expect '{' after module name."));

		while (!IsMatchCurToken(TokenKind::RBRACE))
		{
			switch (GetCurToken()->kind)
			{
			case TokenKind::LET:
			case TokenKind::CONST:
				moduleDecl->varItems.emplace_back((VarDecl *)ParseVarDecl());
				break;
			case TokenKind::FUNCTION:
				GetCurTokenAndStepOnce();
				moduleDecl->functionItems.emplace_back((FunctionDecl *)ParseFunctionDecl());
				break;
			case TokenKind::CLASS:
				moduleDecl->classItems.emplace_back((ClassDecl *)ParseClassDecl());
				break;
			case TokenKind::ENUM:
				moduleDecl->enumItems.emplace_back((EnumDecl *)ParseEnumDecl());
				break;
			case TokenKind::MODULE:
				moduleDecl->moduleItems.emplace_back((ModuleDecl *)ParseModuleDecl());
				break;
			default:
				CYS_LOG_ERROR_WITH_LOC(GetCurToken(), TEXT("Only let,const,function,class,enum and module is available in module scope"));
			}
		}

		Consume(TokenKind::RBRACE, TEXT("Expect '}'."));

		return moduleDecl;
	}

	Stmt *Parser::ParseDeclAndStmt()
	{
		Stmt *result = ParseDecl();
		if (!result)
			result = ParseStmt();
		return result;
	}

	Stmt *Parser::ParseStmt()
	{
		switch (GetCurToken()->kind)
		{
		case TokenKind::RETURN:
			return ParseReturnStmt();
		case TokenKind::IF:
			return ParseIfStmt();
		case TokenKind::LBRACE:
			return ParseScopeStmt();
		case TokenKind::WHILE:
			return ParseWhileStmt();
		case TokenKind::FOR:
			return ParseForStmt();
		case TokenKind::BREAK:
			return ParseBreakStmt();
		case TokenKind::CONTINUE:
			return ParseContinueStmt();
		case TokenKind::SWITCH:
			return ParseSwitchStmt();
		default:
			return ParseExprStmt();
		}
	}

	Stmt *Parser::ParseExprStmt()
	{
		auto exprStmt = new ExprStmt(GetCurToken());
		exprStmt->expr = ParseExpr();

		Consume(TokenKind::SEMICOLON, TEXT("Expect ';' after expr stmt."));
		return exprStmt;
	}

	Stmt *Parser::ParseReturnStmt()
	{
		auto returnStmt = new ReturnStmt(GetCurToken());

		Consume(TokenKind::RETURN, TEXT("Expect 'return' key word."));

		if (!IsMatchCurToken(TokenKind::SEMICOLON))
		{
			std::vector<Expr *> returnExprs;
			do
			{
				if (IsMatchCurToken(TokenKind::SEMICOLON))
					break;

				returnExprs.emplace_back(ParseExpr());

			} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));

			if (returnExprs.size() > 1)
				returnStmt->expr = new AppregateExpr(returnExprs[0]->tagToken, returnExprs);
			else if (returnExprs.size() == 1)
				returnStmt->expr = returnExprs[0];
		}

		Consume(TokenKind::SEMICOLON, TEXT("Expect ';' after return stmt"));
		return returnStmt;
	}

	Stmt *Parser::ParseIfStmt()
	{
		auto ifStmt = new IfStmt(GetCurToken());

		Consume(TokenKind::IF, TEXT("Expect 'if' key word."));
		Consume(TokenKind::LPAREN, TEXT("Expect '(' after 'if'."));

		ifStmt->condition = ParseExpr();

		Consume(TokenKind::RPAREN, TEXT("Expect ')' after if condition"));

		ifStmt->thenBranch = ParseStmt();

		if (IsMatchCurTokenAndStepOnce(TokenKind::ELSE))
			ifStmt->elseBranch = ParseStmt();

		return ifStmt;
	}

	Stmt *Parser::ParseScopeStmt()
	{
		auto scopeStmt = new ScopeStmt(GetCurToken());

		Consume(TokenKind::LBRACE, TEXT("Expect '{'."));
		while (!IsMatchCurToken(TokenKind::RBRACE))
			scopeStmt->stmts.emplace_back(ParseDeclAndStmt());
		Consume(TokenKind::RBRACE, TEXT("Expect '}'."));
		return scopeStmt;
	}

	Stmt *Parser::ParseWhileStmt()
	{
		auto whileStmt = new WhileStmt(GetCurToken());

		Consume(TokenKind::WHILE, TEXT("Expect 'while' keyword."));
		Consume(TokenKind::LPAREN, TEXT("Expect '(' after 'while'."));

		whileStmt->condition = ParseExpr();

		Consume(TokenKind::RPAREN, TEXT("Expect ')' after while stmt's condition."));

		if (IsMatchCurToken(TokenKind::LBRACE)) // scope stmt:while(a<10){a=a+1;}
			whileStmt->body = (ScopeStmt *)ParseScopeStmt();
		else // single stmt:while(a<10) a=a+1;
		{
			auto scopeStmt = new ScopeStmt(GetCurToken());
			scopeStmt->stmts.emplace_back(ParseStmt());
			whileStmt->body = scopeStmt;
		}

		return whileStmt;
	}

	Stmt *Parser::ParseForStmt()
	{
		// for(let i=0,j=0;i<10&&j<10;i=i+1,j=j+1)
		//{
		//	...
		// }
		//  |
		//  |
		//  v
		//{
		//		let i=0,j=0;
		//		while(i<10&&j<10)
		//		{
		//			{
		//				...
		//			}
		//			increment part:
		//			{
		//				i=i+1;
		//				j=j+1;
		//			}
		//		}
		// }

		auto token = GetCurToken();

		auto scopeStmt = new ScopeStmt(token);

		Consume(TokenKind::FOR, TEXT("Expect 'for' keyword."));
		Consume(TokenKind::LPAREN, TEXT("Expect '(' after 'for'."));

		// initializer
		if (IsMatchCurToken(TokenKind::LET) || IsMatchCurToken(TokenKind::CONST))
			scopeStmt->stmts.emplace_back(ParseVarDecl());
		else
		{
			if (!IsMatchCurToken(TokenKind::SEMICOLON))
			{
				do
				{
					scopeStmt->stmts.emplace_back(new ExprStmt(GetCurToken(), ParseExpr()));
				} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));
			}
			Consume(TokenKind::SEMICOLON, TEXT("Expect ';' after for stmt's initializer stmt"));
		}

		Expr *condition = nullptr;
		if (!IsMatchCurToken(TokenKind::SEMICOLON))
			condition = ParseExpr();
		Consume(TokenKind::SEMICOLON, TEXT("Expect ';' after for stmt's condition expr."));

		std::vector<Expr *> increment;
		auto incrementToken = GetCurToken();
		if (!IsMatchCurToken(TokenKind::RPAREN))
		{
			do
			{
				increment.emplace_back(ParseExpr());
			} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));
		}
		Consume(TokenKind::RPAREN, TEXT("Expect ')' after for stmt's increment expr(s)"));
		auto scopeToken = GetCurToken();
		std::vector<Stmt *> whileBodyStmts;
		if (IsMatchCurToken(TokenKind::LBRACE)) // scope stmt for 'for' stmt:like for(a=0;a<10;a=a+1){println("{}",a);}
			whileBodyStmts.emplace_back(ParseScopeStmt());
		else // single stmt for 'for' stmt:like for(a=0;a<10;a=a+1) println("{}",a);
		{
			auto scopeStmt = new ScopeStmt(scopeToken);
			scopeStmt->stmts.emplace_back(ParseStmt());
			whileBodyStmts.emplace_back(scopeStmt);
		}

		auto whileStmt = new WhileStmt(token);
		whileStmt->condition = condition;
		whileStmt->body = new ScopeStmt(scopeToken, whileBodyStmts);
		std::vector<Stmt *> incrementStmts;
		for (const auto expr : increment)
			incrementStmts.emplace_back(new ExprStmt(expr->tagToken, expr));
		whileStmt->increment = new ScopeStmt(incrementToken, incrementStmts);
		scopeStmt->stmts.emplace_back(whileStmt);

		return scopeStmt;
	}

	Stmt *Parser::ParseBreakStmt()
	{
		auto breakStmt = new BreakStmt(GetCurToken());

		Consume(TokenKind::BREAK, TEXT("Expect 'break' keyword."));
		Consume(TokenKind::SEMICOLON, TEXT("Expect ';' after 'break' keyword."));
		return breakStmt;
	}

	Stmt *Parser::ParseContinueStmt()
	{
		auto continueStmt = new ContinueStmt(GetCurToken());

		Consume(TokenKind::CONTINUE, TEXT("Expect 'continue' keyword"));
		Consume(TokenKind::SEMICOLON, TEXT("Expect ';' after 'continue' keyword."));
		return continueStmt;
	}

	Stmt *Parser::ParseSwitchStmt()
	{
		auto ifStmt = new IfStmt(GetCurToken());

		Consume(TokenKind::SWITCH, TEXT("Expect 'switch' keyword."));
		Consume(TokenKind::LPAREN, TEXT("Expect '(' after 'switch' keyword."));

		auto switchExpr = ParseIdentifierExpr();

		Consume(TokenKind::RPAREN, TEXT("Expect ')' after switch's expression."));
		Consume(TokenKind::LBRACE, TEXT("Expect '{' after 'switch' keyword."));

		using Item = std::pair<std::vector<Expr *>, ScopeStmt *>;

		std::vector<Item> items;
		ScopeStmt *defaultScopeStmt = nullptr;
		while (!IsMatchCurToken(TokenKind::RBRACE))
		{
			if (IsMatchCurTokenAndStepOnce(TokenKind::DEFAULT))
			{
				Consume(TokenKind::COLON, TEXT("Expect ':' after condition expr."));
				defaultScopeStmt = new ScopeStmt(GetCurToken());

				if (IsMatchCurTokenAndStepOnce(TokenKind::LBRACE))
				{
					while (!IsMatchCurToken(TokenKind::RBRACE))
						defaultScopeStmt->stmts.emplace_back(ParseStmt());
					Consume(TokenKind::RBRACE, TEXT("Expect '}' at the end of default block while has multiple statement"));
				}
				else
					defaultScopeStmt->stmts.emplace_back(ParseStmt());
			}
			else
			{
				Item item;

				std::vector<Expr *> conditions;
				do
				{
					auto valueCompareExpr = ParseExpr();
					auto caseCondition = new InfixExpr(GetCurToken(), TEXT("=="), switchExpr, valueCompareExpr);
					conditions.emplace_back(caseCondition);
				} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));
				Consume(TokenKind::COLON, TEXT("Expect ':' after condition expr."));

				item.first = conditions;
				item.second = new ScopeStmt(GetCurToken());

				if (IsMatchCurTokenAndStepOnce(TokenKind::LBRACE))
				{
					while (!IsMatchCurToken(TokenKind::RBRACE))
						item.second->stmts.emplace_back(ParseStmt());
					Consume(TokenKind::RBRACE, TEXT("Expect '}' at the end of block while has multiple statements."));
				}
				else
					item.second->stmts.emplace_back(ParseStmt());

				items.emplace_back(item);
			}
		}

		Consume(TokenKind::RBRACE, TEXT("Expect '}' after switch stmt"));

		if (items.empty() && defaultScopeStmt != nullptr)
			return defaultScopeStmt;
		else
		{
			auto loopIfStmt = ifStmt;
			for (size_t i = 0; i < items.size(); ++i)
			{
				if (items[i].first.size() == 1)
					loopIfStmt->condition = items[i].first[0];
				else
				{
					Expr *condition = items[i].first[0];
					for (size_t j = 1; j < items[i].first.size(); ++j)
						condition = new InfixExpr(condition->tagToken, TEXT("||"), condition, items[i].first[j]);
					loopIfStmt->condition = condition;
				}
				loopIfStmt->thenBranch = items[i].second;
				if (i + 1 < items.size())
				{
					loopIfStmt->elseBranch = new IfStmt(ifStmt->tagToken);
					loopIfStmt = (IfStmt *)loopIfStmt->elseBranch;
				}
			}

			if (defaultScopeStmt)
				loopIfStmt->elseBranch = defaultScopeStmt;
		}
		return ifStmt;
	}

	Expr *Parser::ParseLambdaExpr()
	{
		auto lambdaExpr = new LambdaExpr(GetCurToken());

		Consume(TokenKind::FUNCTION, TEXT("Expect 'fn' keyword"));
		Consume(TokenKind::LPAREN, TEXT("Expect '(' after 'fn' keyword"));

		if (!IsMatchCurToken(TokenKind::RPAREN)) // has parameter
		{
			do
			{
				lambdaExpr->parameters.emplace_back((VarDescExpr *)ParseVarDescExpr());
			} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));
		}
		Consume(TokenKind::RPAREN, TEXT("Expect ')' after lambda expr's '('"));

		lambdaExpr->body = (ScopeStmt *)ParseScopeStmt();

		return lambdaExpr;
	}

	Expr *Parser::ParseNewExpr()
	{
		auto newExpr = new NewExpr(GetCurToken());

		Consume(TokenKind::NEW, TEXT("Expect 'new' keyword"));
		Expr *callee = ParseExpr();
		newExpr->callee = callee;
		return newExpr;
	}

	Expr *Parser::ParseThisExpr()
	{
		auto token = Consume(TokenKind::THIS, TEXT("Expect 'this' keyword"));

		auto thisExpr = new ThisExpr(token);

		if (!mCurClassInfo)
			CYS_LOG_ERROR_WITH_LOC(thisExpr->tagToken, TEXT("Invalid 'this' keyword:Cannot use 'this' outside class."));

		return thisExpr;
	}

	Expr *Parser::ParseBaseExpr()
	{
		auto token = Consume(TokenKind::BASE, TEXT("Expect 'base' keyword"));

		Consume(TokenKind::DOT, TEXT("Expect '.' after base keyword"));

		auto baseExpr = new BaseExpr(token, (IdentifierExpr *)ParseIdentifierExpr());

		if (!mCurClassInfo)
			CYS_LOG_ERROR_WITH_LOC(baseExpr->tagToken, TEXT("Invalid 'base' keyword:Cannot use 'base' outside class."));

		return baseExpr;
	}

	Expr *Parser::ParseMatchExpr()
	{
		Expr *defaultBranch = nullptr;

		auto matchToken = Consume(TokenKind::MATCH, TEXT("Expect 'match' keyword."));
		Consume(TokenKind::LPAREN, TEXT("Expect '(' after 'match' keyword."));

		auto judgeExpr = ParseExpr();

		Consume(TokenKind::RPAREN, TEXT("Expect ')' after match's expression."));
		Consume(TokenKind::LBRACE, TEXT("Expect '{' after 'match' keyword."));

		using Item = std::pair<std::vector<Expr *>, Expr *>;

		std::vector<Item> items;
		bool hasDefaultBranch = false;
		if (!IsMatchCurToken(TokenKind::RBRACE))
		{
			do
			{
				if (IsMatchCurToken(TokenKind::DEFAULT))
				{
					if (hasDefaultBranch)
						CYS_LOG_ERROR_WITH_LOC(GetCurToken(), TEXT("Already exists a default branch.only a default branch is available in a match expr."));

					GetCurTokenAndStepOnce();

					Consume(TokenKind::COLON, TEXT("Expect ':' after default's condition expr."));
					defaultBranch = ParseExpr();
					hasDefaultBranch = true;
				}
				else
				{
					Item item;

					std::vector<Expr *> conditions;

					do
					{
						auto valueCompareExpr = ParseExpr();
						auto caseCondition = new InfixExpr(GetCurToken(), TEXT("=="), judgeExpr, valueCompareExpr);
						conditions.emplace_back(caseCondition);
					} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));
					Consume(TokenKind::COLON, TEXT("Expect ':' after match item's condition expr."));

					item.first = conditions;
					item.second = ParseExpr();

					items.emplace_back(item);
				}
			} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));
		}

		Consume(TokenKind::RBRACE, TEXT("Expect '}' after match expr"));

		auto conditionExpr = new ConditionExpr(matchToken);
		auto loopConditionExpr = conditionExpr;
		for (int32_t i = 0; i < items.size(); ++i)
		{
			if (items[i].first.size() == 1)
				loopConditionExpr->condition = items[i].first[0];
			else
			{
				Expr *fullCondition = items[i].first[0];
				for (size_t j = 1; j < items[i].first.size(); ++j)
					fullCondition = new InfixExpr(fullCondition->tagToken, TEXT("||"), fullCondition, items[i].first[j]);
				loopConditionExpr->condition = fullCondition;
			}

			loopConditionExpr->trueBranch = items[i].second;
			if (i + 1 < items.size())
			{
				loopConditionExpr->falseBranch = new ConditionExpr(items[i].second->tagToken);
				loopConditionExpr = (ConditionExpr *)loopConditionExpr->falseBranch;
			}
		}

		if (defaultBranch)
			loopConditionExpr->falseBranch = defaultBranch;

		return conditionExpr;
	}

	Expr *Parser::ParseCompoundExpr()
	{
		auto token = Consume(TokenKind::LPAREN_LBRACE, TEXT("Expect '({'."));

		mSkippingConsumeTokenKindStack.emplace_back(TokenKind::SEMICOLON);

		auto blockExpr = new CompoundExpr(token);

		std::vector<struct Stmt *> stmts;
		do
		{
			if (IsMatchCurToken(TokenKind::RBRACE_RPAREN))
				CYS_LOG_ERROR_WITH_LOC(GetCurToken(), TEXT("Expr required at the end of block expression."));
			stmts.emplace_back(ParseDeclAndStmt());
		} while (IsMatchCurTokenAndStepOnce(TokenKind::SEMICOLON));

		mSkippingConsumeTokenKindStack.pop_back();

		if (stmts.back()->kind != AstKind::EXPR)
			CYS_LOG_ERROR_WITH_LOC(stmts.back()->tagToken, TEXT("Expr required at the end of block expression."));

		auto expr = ((ExprStmt *)stmts.back())->expr;
		stmts.pop_back();

		blockExpr->stmts = stmts;
		blockExpr->endExpr = expr;

		Consume(TokenKind::RBRACE_RPAREN, TEXT("Expect '})'."));

		return blockExpr;
	}

	Expr *Parser::ParseExpr(Precedence precedence)
	{
		if (mPrefixFunctions.find(GetCurToken()->kind) == mPrefixFunctions.end())
		{
			auto token = GetCurTokenAndStepOnce();
			CYS_LOG_ERROR_WITH_LOC(token, TEXT("no prefix definition for:{}"), token->literal);

			auto nullExpr = new LiteralExpr(token);

			return nullExpr;
		}

		auto prefixFn = mPrefixFunctions[GetCurToken()->kind];

		auto leftExpr = (this->*prefixFn)();

		while (!IsMatchCurToken(TokenKind::SEMICOLON) && (GetCurTokenAssociativity() == Associativity::L2R ? precedence < GetCurTokenPrecedence() : precedence <= GetCurTokenPrecedence()))
		{
			if (mPostfixFunctions.find(GetCurToken()->kind) != mPostfixFunctions.end())
			{
				auto postfixFn = mPostfixFunctions[GetCurToken()->kind];
				leftExpr = (this->*postfixFn)(leftExpr);
			}
			else if (mInfixFunctions.find(GetCurToken()->kind) != mInfixFunctions.end())
			{
				auto infixFn = mInfixFunctions[GetCurToken()->kind];
				leftExpr = (this->*infixFn)(leftExpr);
			}
			else
				break;
		}

		return leftExpr;
	}

	Expr *Parser::ParseIdentifierExpr()
	{
		auto token = Consume(TokenKind::IDENTIFIER, TEXT("Unexpect Identifier'") + GetCurToken()->literal + TEXT("'."));
		auto identifierExpr = new IdentifierExpr(token);
		identifierExpr->literal = token->literal;
		return identifierExpr;
	}

	Expr *Parser::ParseLiteralExpr()
	{
		auto token = GetCurTokenAndStepOnce();
		if (token->kind == TokenKind::NUMBER)
		{
			auto literal = token->literal;
			LiteralExpr *numExpr = new LiteralExpr(token);
			if (literal.find('.') != STRING::npos)
			{
				auto v = std::stod(literal);
				numExpr->f64Value = v;

				if (v < std::numeric_limits<float>::max())
					numExpr->type = Type(TypeKind::F32, token->sourceLocation);
				else if (v < std::numeric_limits<double>::max())
					numExpr->type = Type(TypeKind::F64, token->sourceLocation);
			}
			else
			{
				size_t v = std::stoll(literal);
				numExpr->i64Value = v;

				if (v < std::numeric_limits<uint8_t>::max())
					numExpr->type = Type(TypeKind::U8, token->sourceLocation);
				else if (v < std::numeric_limits<uint16_t>::max())
					numExpr->type = Type(TypeKind::U16, token->sourceLocation);
				else if (v < std::numeric_limits<uint32_t>::max())
					numExpr->type = Type(TypeKind::U32, token->sourceLocation);
				else if (v < std::numeric_limits<uint64_t>::max())
					numExpr->type = Type(TypeKind::U64, token->sourceLocation);
			}
			return numExpr;
		}
		else if (token->kind == TokenKind::STR)
			return new LiteralExpr(token, token->literal);
		else if (token->kind == TokenKind::NIL)
			return new LiteralExpr(token);
		else if (token->kind == TokenKind::TRUE)
			return new LiteralExpr(token, true);
		else if (token->kind == TokenKind::FALSE)
			return new LiteralExpr(token, false);

		CYS_LOG_ERROR_WITH_LOC(token, TEXT("Unknown Literal."));
		return nullptr;
	}

	Expr *Parser::ParseGroupExpr()
	{
		auto token = Consume(TokenKind::LPAREN, TEXT("Expect '('."));
		auto groupExpr = new GroupExpr(token);
		groupExpr->expr = ParseExpr();
		Consume(TokenKind::RPAREN, TEXT("Expect ')'."));
		return groupExpr;
	}

	Expr *Parser::ParseArrayExpr()
	{
		auto token = Consume(TokenKind::LBRACKET, TEXT("Expect '['."));
		auto arrayExpr = new ArrayExpr(token);
		if (!IsMatchCurToken(TokenKind::RBRACKET))
		{
			do
			{
				if (IsMatchCurToken(TokenKind::RBRACKET))
					break;

				arrayExpr->elements.emplace_back(ParseExpr());
			} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));
		}

		Consume(TokenKind::RBRACKET, TEXT("Expect ']'."));
		return arrayExpr;
	}

	Expr *Parser::ParseDictExpr()
	{
		auto token = Consume(TokenKind::LBRACE, TEXT("Expect '{'."));
		auto dictExpr = new DictExpr(token);

		if (!IsMatchCurToken(TokenKind::RBRACE))
		{
			std::vector<std::pair<Expr *, Expr *>> elements;
			do
			{
				if (IsMatchCurToken(TokenKind::RBRACE))
					break;

				Expr *key = ParseExpr();
				Consume(TokenKind::COLON, TEXT("Expect ':' after dict key."));
				Expr *value = ParseExpr();
				elements.emplace_back(std::make_pair(key, value));
			} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));

			dictExpr->elements = elements;
		}
		Consume(TokenKind::RBRACE, TEXT("Expect '}' after dict."));
		return dictExpr;
	}

	Expr *Parser::ParseStructExpr()
	{
		auto token = Consume(TokenKind::STRUCT, TEXT("Expect 'struct' keyword."));
		Consume(TokenKind::LBRACE, TEXT("Expect '{' after 'struct' keyword."));
		auto structExpr = new StructExpr(token);

		if (!IsMatchCurToken(TokenKind::RBRACE))
		{
			std::vector<std::pair<STRING, Expr *>> elements;
			do
			{
				if (IsMatchCurToken(TokenKind::RBRACE))
					break;

				Expr *key = ParseExpr();

				if (key->kind != AstKind::IDENTIFIER)
					CYS_LOG_ERROR_WITH_LOC(key->tagToken, TEXT("Struct object require key must be a valid identifier."));

				Consume(TokenKind::COLON, TEXT("Expect ':' after struct object's key."));
				Expr *value = ParseExpr();
				elements.emplace_back(((IdentifierExpr *)key)->literal, value);
			} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));

			structExpr->elements = elements;
		}
		Consume(TokenKind::RBRACE, TEXT("Expect '}' after struct object."));
		return structExpr;
	}

	Expr *Parser::ParsePrefixExpr()
	{
		auto prefixExpr = new PrefixExpr(GetCurToken());
		prefixExpr->op = GetCurTokenAndStepOnce()->literal;
		prefixExpr->right = ParseExpr(Precedence::PREFIX);
		return prefixExpr;
	}

	Expr *Parser::ParseRefExpr()
	{
		auto token = Consume(TokenKind::AMPERSAND, TEXT("Expect '&'."));
		auto refExpr = new RefExpr(token);
		refExpr->refExpr = ParseExpr(Precedence::PREFIX);
		return refExpr;
	}

	Expr *Parser::ParseInfixExpr(Expr *prefixExpr)
	{
		auto infixExpr = new InfixExpr(GetCurToken());
		infixExpr->left = prefixExpr;
		Precedence opPrece = GetCurTokenPrecedence();
		infixExpr->op = GetCurTokenAndStepOnce()->literal;
		infixExpr->right = ParseExpr(opPrece);
		return infixExpr;
	}

	Expr *Parser::ParsePostfixExpr(Expr *prefixExpr)
	{
		auto postfixExpr = new PostfixExpr(GetCurToken());
		postfixExpr->op = GetCurTokenAndStepOnce()->literal;
		postfixExpr->left = prefixExpr;
		return postfixExpr;
	}

	Expr *Parser::ParseConditionExpr(Expr *prefixExpr)
	{
		ConditionExpr *conditionExpr = new ConditionExpr(GetCurToken());
		conditionExpr->condition = prefixExpr;
		Consume(TokenKind::QUESTION, TEXT("Expect '?'."));
		conditionExpr->trueBranch = ParseExpr(Precedence::CONDITION);
		Consume(TokenKind::COLON, TEXT("Expect ':' in condition expr"));
		conditionExpr->falseBranch = ParseExpr(Precedence::CONDITION);
		return conditionExpr;
	}

	Expr *Parser::ParseIndexExpr(Expr *prefixExpr)
	{
		auto indexExpr = new IndexExpr(GetCurToken());
		Consume(TokenKind::LBRACKET, TEXT("Expect '['."));
		indexExpr->ds = prefixExpr;
		indexExpr->index = ParseExpr();
		Consume(TokenKind::RBRACKET, TEXT("Expect ']'."));
		return indexExpr;
	}

	Expr *Parser::ParseCallExpr(Expr *prefixExpr)
	{
		auto callExpr = new CallExpr(GetCurToken());
		callExpr->callee = prefixExpr;
		Consume(TokenKind::LPAREN, TEXT("Expect '('."));
		if (!IsMatchCurToken(TokenKind::RPAREN)) // has arguments
		{
			do
			{
				callExpr->arguments.emplace_back(ParseExpr());
			} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));
		}
		Consume(TokenKind::RPAREN, TEXT("Expect ')'."));

		return callExpr;
	}

	Expr *Parser::ParseDotExpr(Expr *prefixExpr)
	{
		auto dotExpr = new DotExpr(GetCurToken());
		Consume(TokenKind::DOT, TEXT("Expect '.'."));
		dotExpr->callee = prefixExpr;
		dotExpr->callMember = (IdentifierExpr *)ParseIdentifierExpr();
		return dotExpr;
	}

	Expr *Parser::ParseFactorialExpr(Expr *prefixExpr)
	{
		auto token = Consume(TokenKind::BANG, TEXT("Expect '!'"));
		return new FactorialExpr(token, prefixExpr);
	}

	Expr *Parser::ParseVarDescExpr()
	{
		Expr *expr = nullptr;
		if (IsMatchCurToken(TokenKind::ELLIPSIS))
			expr = ParseVarArgExpr();
		else
			expr = (IdentifierExpr *)ParseIdentifierExpr(); // variable name

		// variable type
		auto type = Type();
		if (IsMatchCurToken(TokenKind::COLON))
		{
			GetCurTokenAndStepOnce();
			type = ParseType();
		}

		auto varDescExpr = new VarDescExpr(expr->tagToken, type, expr);
		return varDescExpr;
	}

	Expr *Parser::ParseVarArgExpr()
	{
		Consume(TokenKind::ELLIPSIS, TEXT("Expect '...'"));

		auto varArgExpr = new VarArgExpr(GetCurToken());

		if (IsMatchCurToken(TokenKind::IDENTIFIER))
		{
			varArgExpr->argName = (IdentifierExpr *)ParseIdentifierExpr();
			return varArgExpr;
		}
		return varArgExpr;
	}

	std::pair<Expr *, Expr *> Parser::ParseDestructuringAssignmentExpr()
	{
		Consume(TokenKind::LBRACKET, TEXT("Expect '['"));

		auto arrayExpr = new ArrayExpr(GetCurToken());

		int8_t varArgCount = 0;
		do
		{
			if (IsMatchCurToken(TokenKind::RBRACKET))
				break;

			auto varDescExpr = (VarDescExpr *)ParseVarDescExpr();
			if (varDescExpr->name->kind == AstKind::VAR_ARG) // check if has var arg
			{
				varArgCount++;
				if (varArgCount > 1)
					CYS_LOG_ERROR_WITH_LOC(varDescExpr->tagToken, TEXT("only 1 variable arg decl is available in var declaration."));
			}

			arrayExpr->elements.emplace_back(varDescExpr);

		} while (IsMatchCurTokenAndStepOnce(TokenKind::COMMA));

		Consume(TokenKind::RBRACKET, TEXT("Expect ']' destructuring assignment expr."));

		if (varArgCount == 1)
		{
			for (int32_t i = 0; i < arrayExpr->elements.size(); ++i)
			{
				if (((VarDescExpr *)arrayExpr->elements[i])->name->kind == AstKind::VAR_ARG)
				{
					if (i < arrayExpr->elements.size() - 1)
						CYS_LOG_ERROR_WITH_LOC(arrayExpr->elements[i]->tagToken, TEXT("variable arg decl must be located at the end of destructing assignment declaration."));
				}
			}
		}

		ArrayExpr *initializeList = new ArrayExpr(GetCurToken());

		if (IsMatchCurTokenAndStepOnce(TokenKind::EQUAL))
		{
			Expr *value = ParseExpr();

			if (value->kind == AstKind::ARRAY)
			{
				int32_t grad = (int32_t)arrayExpr->elements.size() - (int32_t)((ArrayExpr *)value)->elements.size();
				if (grad == 0)
					return std::make_pair(arrayExpr, value);
				else if (grad > 0)
				{
					initializeList->elements = ((ArrayExpr *)value)->elements;
					for (int32_t i = 0; i < grad; ++i)
						initializeList->elements.emplace_back(new LiteralExpr(GetCurToken()));
				}
				else if (((VarDescExpr *)arrayExpr->elements.back())->name->kind == AstKind::VAR_ARG)
					initializeList->elements = ((ArrayExpr *)value)->elements;
				else
					CYS_LOG_ERROR_WITH_LOC(GetCurToken(), TEXT("variable less than value."));
			}
			else if (value->kind == AstKind::CALL)
				return std::make_pair(arrayExpr, value);
			else
				initializeList->elements.resize(arrayExpr->elements.size(), value);
		}
		else
		{
			Expr *nullExpr = new LiteralExpr(GetCurToken());
			initializeList->elements.resize(arrayExpr->elements.size(), nullExpr);
		}

		return std::make_pair(arrayExpr, initializeList);
	}

	Type Parser::ParseType()
	{
		//TODO:only support basic single word type
		auto token = GetCurTokenAndStepOnce();
		return Type(token->literal, token->sourceLocation);
	}

	ClassDecl::MemberPrivilege Parser::ParseClassMemberPrivilege()
	{
		switch (GetCurToken()->kind)
		{
		case TokenKind::PRIVATE:
			return ClassDecl::MemberPrivilege::PRIVATE;
		case TokenKind::PROTECTED:
			return ClassDecl::MemberPrivilege::PROTECTED;
		case TokenKind::PUBLIC:
			return ClassDecl::MemberPrivilege::PUBLIC;
		default:
			return ClassDecl::MemberPrivilege::PRIVATE;
		}
	}

	Token *Parser::GetCurToken()
	{
		if (!IsAtEnd())
			return mTokens[mCurPos];
		return mTokens.back();
	}
	Token *Parser::GetCurTokenAndStepOnce()
	{
		if (!IsAtEnd())
			return mTokens[mCurPos++];
		return mTokens.back();
	}

	Precedence Parser::GetCurTokenPrecedence()
	{
		for (const auto &precedenceBinding : precedenceDict)
			if (precedenceBinding.kind == GetCurToken()->kind)
				return precedenceBinding.precedence;
		return Precedence::LOWEST;
	}

	Associativity Parser::GetCurTokenAssociativity()
	{
		for (const auto &associativityBinding : associativityDict)
			if (associativityBinding.precedence == GetCurTokenPrecedence())
				return associativityBinding.associativity;
		return Associativity::L2R;
	}

	Token *Parser::GetNextToken()
	{
		if (mCurPos + 1 < (int32_t)mTokens.size())
			return mTokens[mCurPos + 1];
		return mTokens.back();
	}
	Token *Parser::GetNextTokenAndStepOnce()
	{
		if (mCurPos + 1 < (int32_t)mTokens.size())
			return mTokens[++mCurPos];
		return mTokens.back();
	}

	Precedence Parser::GetNextTokenPrecedence()
	{
		for (const auto &precedenceBinding : precedenceDict)
			if (precedenceBinding.kind == GetNextToken()->kind)
				return precedenceBinding.precedence;
		return Precedence::LOWEST;
	}

	bool Parser::IsMatchCurToken(TokenKind kind)
	{
		return GetCurToken()->kind == kind;
	}

	bool Parser::IsMatchCurTokenAndStepOnce(TokenKind kind)
	{
		if (IsMatchCurToken(kind))
		{
			mCurPos++;
			return true;
		}
		return false;
	}

	bool Parser::IsMatchNextToken(TokenKind kind)
	{
		return GetNextToken()->kind == kind;
	}

	bool Parser::IsMatchNextTokenAndStepOnce(TokenKind kind)
	{
		if (IsMatchNextToken(kind))
		{
			mCurPos++;
			return true;
		}
		return false;
	}

	Token *Parser::Consume(TokenKind kind, STRING_VIEW errMsg)
	{
		if (mSkippingConsumeTokenKindStack.empty() || kind != mSkippingConsumeTokenKindStack.back())
		{
			if (IsMatchCurToken(kind))
				return GetCurTokenAndStepOnce();
			Token *token = GetCurToken();
			CYS_LOG_ERROR_WITH_LOC(token, TEXT("{}"), errMsg);
		}
		return nullptr;
	}

	Token *Parser::Consume(const std::vector<TokenKind> &kinds, STRING_VIEW errMsg)
	{
		if (!mSkippingConsumeTokenKindStack.empty())
			for (const auto &kind : kinds)
				if (kind == mSkippingConsumeTokenKindStack.back())
					return nullptr;

		for (const auto &kind : kinds)
			if (IsMatchCurToken(kind))
				return GetCurTokenAndStepOnce();
		Token *token = GetCurToken();
		CYS_LOG_ERROR_WITH_LOC(token, TEXT("{}"), errMsg);
		return nullptr;
	}

	bool Parser::IsAtEnd()
	{
		return mCurPos >= (int32_t)mTokens.size();
	}
}