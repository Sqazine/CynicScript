#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include "Token.h"
#include "Common.h"

namespace CynicScript
{
	class CYS_API Lexer
	{
	public:
		Lexer();
		~Lexer() = default;

		const std::vector<Token *> &ScanTokens(STRING_VIEW src);

	private:
		void ResetStatus();

		void ScanToken();

		bool IsMatchCurChar(CHAR_T c);
		bool IsMatchCurCharAndStepOnce(CHAR_T c);

		CHAR_T GetCurCharAndStepOnce();
		CHAR_T GetCurChar();

		void AddToken(TokenKind type);
		void AddToken(TokenKind type, STRING_VIEW literal);

		bool IsAtEnd();

		bool IsNumber(CHAR_T c);
		bool IsLetter(CHAR_T c, bool isAscii);
		bool IsLetterOrNumber(CHAR_T c, bool isAscii);

		void Number();
		void Identifier();
		void String();
		void Character();

		uint64_t mStartPos;
		uint64_t mCurPos;
		uint64_t mLine;
		uint64_t mColumn;
		STRING mSource;
		std::vector<Token *> mTokens;
	};
}