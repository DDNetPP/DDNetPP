#include <base/mem.h>
#include <base/str.h>
#include <base/windows.h>

#include <engine/server/server.h>

#include <game/gamecore.h>
#include <game/server/ddnetpp/lua/lua_plugin.h>

#include <gtest/gtest.h>

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
	Ok = SplitConsoleArgs(apArgs, (sizeof(apArgs)/sizeof(apArgs[0])), &NumArgs, aInput, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumArgs, 2);
	EXPECT_STREQ(apArgs[0], "hello");
	EXPECT_STREQ(apArgs[1], "world");

	str_copy(aInput, "    hello      world   ");
	Ok = SplitConsoleArgs(apArgs, (sizeof(apArgs)/sizeof(apArgs[0])), &NumArgs, aInput, aError, sizeof(aError));
	EXPECT_EQ(Ok, true);
	EXPECT_EQ(NumArgs, 2);
	EXPECT_STREQ(apArgs[0], "hello");
	EXPECT_STREQ(apArgs[1], "world");
}
