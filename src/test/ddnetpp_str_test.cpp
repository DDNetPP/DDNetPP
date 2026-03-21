#include <base/mem.h>
#include <base/str.h>
#include <base/windows.h>

#include <engine/server/server.h>

#include <game/gamecore.h>
#include <game/server/ddnetpp/lua/console_strings.h>
#include <game/server/ddnetpp/lua/lua_plugin.h>

#include <gtest/gtest.h>

#include <vector>

TEST(DDNetPP, SplitStmts)
{
	const char *apStmts[16] = {};
	char aError[512] = "";
	size_t NumStmts = 0;
	char aLine[512];
	bool Ok;

	str_copy(aLine, "hello");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 1);
	EXPECT_STREQ(apStmts[0], "hello");

	str_copy(aLine, "hello world");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 1);
	EXPECT_STREQ(apStmts[0], "hello world");

	str_copy(aLine, "hello;");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 1);
	EXPECT_STREQ(apStmts[0], "hello");

	str_copy(aLine, "hello;world");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 2);
	EXPECT_STREQ(apStmts[0], "hello");
	EXPECT_STREQ(apStmts[1], "world");

	str_copy(aLine, "hello; world");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 2);
	EXPECT_STREQ(apStmts[0], "hello");
	EXPECT_STREQ(apStmts[1], " world");

	str_copy(aLine, "hello;;");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, false);
	EXPECT_STREQ(aError, "syntax error near unexpected token `;;'");

	str_copy(aLine, ";;");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, false);
	EXPECT_STREQ(aError, "syntax error near unexpected token `;;'");

	str_copy(aLine, "");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 0);

	str_copy(aLine, "hello # world");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 1);
	EXPECT_STREQ(apStmts[0], "hello ");

	str_copy(aLine, "echo \"#gaming\"");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 1);
	EXPECT_STREQ(apStmts[0], "echo \"#gaming\"");

	str_copy(aLine, "a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, false);
	EXPECT_STREQ(aError, "too many statements");

	str_copy(aLine, "a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, false);
	EXPECT_STREQ(aError, "too many statements");

	str_copy(aLine, "a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 16);

	str_copy(aLine, "a;a;a;a;a;a;a;a;a;a;a;a;a;a;a;");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 15);

	str_copy(aLine, "a;a;a;a;a;a;a;a;a;a;a;a;a;a;");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 14);

	str_copy(aLine, "echo \\\"escaped quote # real comment");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 1);
	EXPECT_STREQ(apStmts[0], "echo \\\"escaped quote ");

	str_copy(aLine, "hello # semi;nuts;comment;;;;");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 1);
	EXPECT_STREQ(apStmts[0], "hello ");

	str_copy(aLine, "echo \";_;\"");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 1);
	EXPECT_STREQ(apStmts[0], "echo \";_;\"");

	str_copy(aLine, "echo \";_;\";say a");
	Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumStmts, 2);
	EXPECT_STREQ(apStmts[0], "echo \";_;\"");
	EXPECT_STREQ(apStmts[1], "say a");
}

TEST(DDNetPP, ParseParams)
{
	std::vector<CLuaRconCommand::CParam> vParams;
	char aError[512];
	bool Ok;

	Ok = CLuaRconCommand::ParseParameters(vParams, "r", aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(vParams.size(), 1);

	vParams.clear();
	Ok = CLuaRconCommand::ParseParameters(vParams, "i?ii", aError, sizeof(aError));
	EXPECT_EQ(Ok, false);
	EXPECT_STREQ(aError, "got non optional param after optional one 'i?ii'");
}

TEST(DDNetPP, StrConsole)
{
	const char *apArgs[16] = {};
	char aError[512] = "";
	size_t NumArgs = 0;
	char aInput[512];
	bool Ok;

	str_copy(aInput, "hello world");
	Ok = SplitConsoleArgs(apArgs, (sizeof(apArgs) / sizeof(apArgs[0])), &NumArgs, aInput, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumArgs, 2);
	EXPECT_STREQ(apArgs[0], "hello");
	EXPECT_STREQ(apArgs[1], "world");

	str_copy(aInput, "hello");
	Ok = SplitConsoleArgs(apArgs, (sizeof(apArgs) / sizeof(apArgs[0])), &NumArgs, aInput, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumArgs, 1);
	EXPECT_STREQ(apArgs[0], "hello");

	str_copy(aInput, "hello \"world\"");
	Ok = SplitConsoleArgs(apArgs, (sizeof(apArgs) / sizeof(apArgs[0])), &NumArgs, aInput, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumArgs, 2);
	EXPECT_STREQ(apArgs[0], "hello");
	EXPECT_STREQ(apArgs[1], "world");

	str_copy(aInput, "hello \"world");
	Ok = SplitConsoleArgs(apArgs, (sizeof(apArgs) / sizeof(apArgs[0])), &NumArgs, aInput, aError, sizeof(aError));
	EXPECT_EQ(Ok, false);
	EXPECT_STREQ(aError, "missing closing quote");

	str_copy(aInput, "a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a");
	Ok = SplitConsoleArgs(apArgs, (sizeof(apArgs) / sizeof(apArgs[0])), &NumArgs, aInput, aError, sizeof(aError));
	EXPECT_EQ(Ok, false);
	EXPECT_STREQ(aError, "too many arguments");

	str_copy(aInput, "\"hello world\"");
	Ok = SplitConsoleArgs(apArgs, (sizeof(apArgs) / sizeof(apArgs[0])), &NumArgs, aInput, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumArgs, 1);
	EXPECT_STREQ(apArgs[0], "hello world");

	str_copy(aInput, "hello                   world");
	Ok = SplitConsoleArgs(apArgs, (sizeof(apArgs) / sizeof(apArgs[0])), &NumArgs, aInput, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumArgs, 2);
	EXPECT_STREQ(apArgs[0], "hello");
	EXPECT_STREQ(apArgs[1], "world");

	str_copy(aInput, "    hello      world   ");
	Ok = SplitConsoleArgs(apArgs, (sizeof(apArgs) / sizeof(apArgs[0])), &NumArgs, aInput, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumArgs, 2);
	EXPECT_STREQ(apArgs[0], "hello");
	EXPECT_STREQ(apArgs[1], "world");

	std::vector<CLuaRconCommand::CParam> vParams;
	CLuaRconCommand::ParseParameters(vParams, "r", aError, sizeof(aError));
	str_copy(aInput, "hello     world");
	Ok = SplitConsoleArgsWithParams(apArgs, (sizeof(apArgs) / sizeof(apArgs[0])), &NumArgs, aInput, vParams, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumArgs, 1);
	EXPECT_STREQ(apArgs[0], "hello     world");

	// more arguments than parameters
	vParams.clear();
	CLuaRconCommand::ParseParameters(vParams, "s", aError, sizeof(aError));
	str_copy(aInput, "hello world");
	Ok = SplitConsoleArgsWithParams(apArgs, (sizeof(apArgs) / sizeof(apArgs[0])), &NumArgs, aInput, vParams, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumArgs, 2);
	EXPECT_STREQ(apArgs[0], "hello");
	EXPECT_STREQ(apArgs[1], "world");
}
