#pragma once

#include "Version.h"
#include "Utils.h"
#include "Logger.h"
#include "Token.h"
#include "Ast.h"
#include "Config.h"
#include "Lexer.h"
#include "Parser.h"
#include "AstOptimizePass.h"
#include "ConstantFoldPass.h"
#include "TypeCheckAndResolvePass.h"
#include "SyntaxCheckPass.h"
#include "Compiler.h"
#include "VM.h"