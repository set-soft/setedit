#ifndef STACKDBG_H_INCLUDED
#define STACKDBG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* Strategies */
/* Dump the stack, no symbol translations */
#define DBGST_DUMP_STACK       0
/* No action */
#define DBGST_DO_NOTHING       1
/* Try to get as many information as possible */
#define DBGST_INFORMATIVE      2
/* Start the debuger to get information about the crash */
#define DBGST_DEBUG            3

extern char DebugStackInstall(char aStrategy, const char *processname);
extern void DebugStack(const char *redirect);
extern int  DebugStackSeparateTerminalWillBeUsed();

#ifdef __cplusplus
}
#endif

#endif // STACKDBG_H_INCLUDED
