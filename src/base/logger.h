#ifndef BASE_LOGGER_H
#define BASE_LOGGER_H

#include "log.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

typedef struct IOINTERNAL *IOHANDLE;

/**
 * @ingroup Log
 *
 * Metadata and actual content of a log message.
 */
class CLogMessage
{
public:
	/**
	 * Severity
	 */
	LEVEL m_Level;
	bool m_HaveColor;
	/**
	 * The requested color of the log message. Only useful if `m_HaveColor`
	 * is set.
	 */
	LOG_COLOR m_Color;
	char m_aTimestamp[80];
	char m_aSystem[32];
	/**
	 * The actual log message including the timestamp and the system.
	 */
	char m_aLine[4096];
	int m_TimestampLength;
	int m_SystemLength;
	/**
	 * Length of the log message including timestamp and the system.
	 */
	int m_LineLength;
	int m_LineMessageOffset;

	/**
	 * The actual log message excluding timestamp and the system.
	 */
	const char *Message() const
	{
		return m_aLine + m_LineMessageOffset;
	}
};

class ILogger
{
public:
	virtual ~ILogger() {}

	/**
	 * Send the specified message to the logging backend.
	 *
	 * @param pMessage Struct describing the log message.
	 */
	virtual void Log(const CLogMessage *pMessage) = 0;
	/**
	 * Flushes output buffers and shuts down.
	 * Global loggers cannot be destroyed because they might be accessed
	 * from multiple threads concurrently.
	 *
	 * This function is called on the global logger by
	 * `log_global_logger_finish` when the program is about to shut down
	 * and loggers are supposed to finish writing the log messages they
	 * have received so far.
	 *
	 * The destructor of this `ILogger` instance will not be called if this
	 * function is called.
	 *
	 * @see log_global_logger_finish
	 */
	virtual void GlobalFinish() {}
};

/**
 * @ingroup Log
 *
 * Sets the global loglevel to the given value. Log messages with a less severe
 * loglevel will not be forwarded to loggers.
 *
 * @param level The loglevel.
 */
void log_set_loglevel(LEVEL level);

/**
 * @ingroup Log
 *
 * Registers a logger instance as the default logger for all current and future
 * threads. It will only be used if no thread-local logger is set via
 * `log_set_scope_logger`.
 *
 * This function can only be called once. The passed logger instance will never
 * be freed.
 *
 * @param logger The global logger default.
 */
void log_set_global_logger(ILogger *logger);

/**
 * @ingroup Log
 *
 * Registers a sane default as the default logger for all current and future
 * threads.
 *
 * This is logging to stdout on most platforms and to the system log on
 * Android.
 *
 * @see log_set_global_logger
 */
void log_set_global_logger_default();

/**
 * @ingroup Log
 *
 * Notify global loggers of impending abnormal exit.
 *
 * This function is automatically called on normal exit. It notifies the global
 * logger of the impending shutdown via `GlobalFinish`, the logger is supposed
 * to flush its buffers and shut down.
 *
 * Don't call this except right before an abnormal exit.
 */
void log_global_logger_finish();

/**
 * @ingroup Log
 *
 * Get the logger active in the current scope. This might be the global default
 * logger or some other logger set via `log_set_scope_logger`.
 */
ILogger *log_get_scope_logger();

/**
 * @ingroup Log
 *
 * Set the logger for the current thread. The logger isn't managed by the
 * logging system, it still needs to be kept alive or freed by the caller.
 *
 * Consider using `CLogScope` if you only want to set the logger temporarily.
 *
 * @see CLogScope
 */
void log_set_scope_logger(ILogger *logger);

/**
 * @ingroup Log
 *
 * Logger for sending logs to the Android system log.
 *
 * Should only be called when targeting the Android platform.
 */
std::unique_ptr<ILogger> log_logger_android();

/**
 * @ingroup Log
 *
 * Logger combining a vector of other loggers.
 */
std::unique_ptr<ILogger> log_logger_collection(std::vector<std::shared_ptr<ILogger>> &&vpLoggers);

/**
 * @ingroup Log
 *
 * Logger for writing logs to the given file.
 *
 * @param file File to write to, must be opened for writing.
 */
std::unique_ptr<ILogger> log_logger_file(IOHANDLE file);

/**
 * @ingroup Log
 *
 * Logger for writing logs to the standard output (stdout).
 */
std::unique_ptr<ILogger> log_logger_stdout();

/**
 * @ingroup Log
 *
 * Logger for sending logs to the debugger on Windows via `OutputDebugStringW`.
 *
 * Should only be called when targeting the Windows platform.
 */
std::unique_ptr<ILogger> log_logger_windows_debugger();

/**
 * @ingroup Log
 *
 * Logger that collects log messages in memory until it is replaced by another
 * logger.
 *
 * Useful when you want to set a global logger without all logging targets
 * being configured.
 */
class CFutureLogger : public ILogger
{
private:
	std::atomic<ILogger *> m_pLogger;
	std::vector<CLogMessage> m_vPending;
	std::mutex m_PendingLock;

public:
	/**
	 * Replace the `CFutureLogger` instance with the given logger. It'll
	 * receive all log messages sent to the `CFutureLogger` so far.
	 */
	void Set(std::unique_ptr<ILogger> &&pLogger);
	void Log(const CLogMessage *pMessage) override;
	void GlobalFinish() override;
};

/**
 * @ingroup Log
 *
 * RAII guard for temporarily changing the logger via `log_set_scope_logger`.
 *
 * @see log_set_scope_logger
 */
class CLogScope
{
	ILogger *old_scope_logger;
	ILogger *new_scope_logger;

public:
	CLogScope(ILogger *logger) :
		old_scope_logger(log_get_scope_logger()),
		new_scope_logger(logger)
	{
		log_set_scope_logger(new_scope_logger);
	}
	~CLogScope()
	{
		//dbg_assert(log_get_scope_logger() == new_scope_logger, "loggers weren't properly scoped");
		log_set_scope_logger(old_scope_logger);
	}
};
#endif // BASE_LOGGER_H
