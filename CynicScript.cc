#include <string>
#include <string_view>
#include "CynicScript.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)
#endif

#define CYS_REPL_CLEAR TEXT("clear")
#define CYS_REPL_EXIT TEXT("exit")

CynicScript::Lexer *gLexer{nullptr};
CynicScript::Parser *gParser{nullptr};

CynicScript::AstOptimizePassManager *gAstOptimizePassManager{nullptr};

CynicScript::Compiler *gCompiler{nullptr};
CynicScript::VM *gVm{nullptr};

int32_t PrintVersion()
{
	CYS_LOG_INFO(CYS_VERSION);
	return EXIT_FAILURE;
}

int32_t PrintUsage()
{
	CYS_LOG_INFO(TEXT("Usage: CynicScript [option]:"));
	CYS_LOG_INFO(TEXT("-h or --help:show usage info."));
	CYS_LOG_INFO(TEXT("-v or --version:show current CynicScript version"));
	CYS_LOG_INFO(TEXT("-s or --serialize: serialize source file as bytecode binary file"));
	CYS_LOG_INFO(TEXT("-f or --file:run source file with a valid file path,like : CynicScript -f examples/array.cd."));
	CYS_LOG_INFO(TEXT("--function-cache:use function cache optimize."));
#ifndef NDEBUG
	CYS_LOG_INFO(TEXT("--gc-debug:debug gc."));
	CYS_LOG_INFO(TEXT("--gc-stress:stressing gc."));
#endif
	CYS_LOG_INFO(TEXT("In REPL mode, you can input '{}' to clear the REPL history, and '{}' to exit the REPL."), CYS_REPL_CLEAR, CYS_REPL_EXIT);
	return EXIT_FAILURE;
}

void Run(STRING_VIEW content)
{
	auto tokens = gLexer->ScanTokens(content);
#ifndef NDEBUG
	for (const auto &token : tokens)
		CynicScript::Logger::Println(TEXT("{}"), *token);
#endif
	auto stmt = gParser->Parse(tokens);

#ifndef NDEBUG
	CynicScript::Logger::Println(TEXT("{}"), stmt->ToString());
#endif

	stmt = gAstOptimizePassManager->Execute(stmt);

#ifndef NDEBUG
	CynicScript::Logger::Println(TEXT("{}"), stmt->ToString());
#endif

	auto mainFunc = gCompiler->Compile(stmt);

#ifndef NDEBUG
	auto str = mainFunc->ToStringWithChunk();
	CynicScript::Logger::Println(TEXT("{}"), str);
#endif

	if (CynicScript::Config::GetInstance()->IsSerializeBinaryChunk())
	{
		auto data = mainFunc->chunk.Serialize();
		CynicScript::WriteBinaryFile(CynicScript::Config::GetInstance()->GetSerializeBinaryFilePath(), data);
	}
	else
	{
		gVm->Run(mainFunc);
	}
}

void Repl()
{
	STRING line;
	STRING allLines;

	PrintVersion();

	CynicScript::Logger::Print(TEXT(">> "));
	while (getline(CIN, line))
	{
		allLines += line;
		if (line == CYS_REPL_CLEAR)
		{
			allLines.clear();
		}
		else if (line == CYS_REPL_EXIT)
		{
			CynicScript::Logger::Println(TEXT("Bye!"));
			return;
		}
		else
		{
			Run(allLines);
		}

		CynicScript::Logger::Print(TEXT(">> "));
	}
}

void RunFile(std::string_view path)
{
	STRING content = CynicScript::ReadFile(path);
	Run(content);
}

int32_t ParseArgs(int32_t argc, const char *argv[])
{
	for (size_t i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0)
		{
			if (i + 1 < argc)
				CynicScript::Config::GetInstance()->SetExecuteFilePath(argv[++i]);
			else
				return PrintUsage();
		}

		if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--serialize") == 0)
		{
			if (i + 1 < argc)
			{
				CynicScript::Config::GetInstance()->SetSerializeBinaryChunk(true);
				CynicScript::Config::GetInstance()->SetSerializeBinaryFilePath(argv[++i]);
			}
			else
				return PrintUsage();
		}
		if (strcmp(argv[i], "--function-cache") == 0)
			CynicScript::Config::GetInstance()->SetUseFunctionCache(true);

		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			return PrintUsage();

		if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
			return PrintVersion();
	}

	return EXIT_SUCCESS;
}

int32_t main(int32_t argc, const char *argv[])
{
#if defined(_WIN32) || defined(_WIN64)
	system("chcp 65001");
#endif
	if (ParseArgs(argc, argv) == EXIT_FAILURE)
		return EXIT_FAILURE;

	gLexer = new CynicScript::Lexer();
	gParser = new CynicScript::Parser();
	gAstOptimizePassManager = new CynicScript::AstOptimizePassManager();
	gCompiler = new CynicScript::Compiler();
	gVm = new CynicScript::VM();

	gAstOptimizePassManager
		->Add<CynicScript::ConstantFoldPass>()
		->Add<CynicScript::SyntaxCheckPass>()
		->Add<CynicScript::TypeCheckAndResolvePass>();

	if (!CynicScript::Config::GetInstance()->GetExecuteFilePath().empty())
		RunFile(CynicScript::Config::GetInstance()->GetExecuteFilePath());
	else
		Repl();

	SAFE_DELETE(gLexer);
	SAFE_DELETE(gParser);
	SAFE_DELETE(gAstOptimizePassManager);
	SAFE_DELETE(gCompiler);
	SAFE_DELETE(gVm);

	return EXIT_SUCCESS;
}