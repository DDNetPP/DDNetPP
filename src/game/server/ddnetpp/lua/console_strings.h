#ifndef GAME_SERVER_DDNETPP_LUA_LUA_CONSOLE_STRINGS_H
#define GAME_SERVER_DDNETPP_LUA_LUA_CONSOLE_STRINGS_H

#include <game/server/ddnetpp/lua/lua_rcon_command.h>

#include <vector>

// returns true on success
// writes multiple teeworlds console statements (separated by semicolon)
// into the output array apStmts the amount of items is written to pNumStmts
//
// it strips the trailing semicolons
// and treats ;;; as an syntax error
bool SplitConsoleStatements(const char *apStmts[], size_t MaxStmts, size_t *pNumStmts, char *pLine, char *pError, size_t ErrorLen);

// returns true on success
// takes a mutable pInput string which it will also write to
// and writes the result to apArgs which is a user given array of size MaxArgs
// the amount of args it got it will write to pNumArgs
//
// on error it writes a reason to pError
bool SplitConsoleArgs(const char *apArgs[], size_t MaxArgs, size_t *pNumArgs, char *pInput, char *pError, size_t ErrorLen);

// Same as SplitConsoleArgs
// but also takes vParams into consideration
// and splitting "r" params differently
bool SplitConsoleArgsWithParams(const char *apArgs[], size_t MaxArgs, size_t *pNumArgs, char *pInput, const std::vector<CLuaRconCommand::CParam> &vParams, char *pError, size_t ErrorLen);

#endif
