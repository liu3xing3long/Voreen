/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#ifndef TGT_LOGMANAGER_H
#define TGT_LOGMANAGER_H

#include <string>
#include <sstream>
#include <vector>

#include "tgt/config.h"
#include "tgt/assert.h"
#include <stdarg.h>
#include "tgt/singleton.h"

namespace tgt {

/**
 * Specifies the severity of the log event.
 * Debug messages are not logged in release builds!
 */
enum LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

/**
 * Holds the information for filtering messages.
 */
struct LogFilter {
    std::string cat_;
    bool children_;
    LogLevel level_;
};

/**
 * Abstract basis class for logging messages.
 */
class Log {
public:
	virtual ~Log() {}

    /// Log a message in this Log (message is filtered based on cat and level)
	virtual void log(const std::string &cat, LogLevel level, const std::string &msg, const std::string &extendedInfo="");
    
    /**
     * Add a category that is accepted to this log.
     * @param cat All messages with category = cat are accepted and logged in this log.
     * @param Children If true all messages of subcategories of cat are also accepted.
     * example:
     * tgt.Texture.Reader is a subcategory of tgt.Texture
     * @param level All messages below this LogLevel are discarded, even if the category matches.
     */
	virtual void addCat(const std::string &cat, bool Children = true, LogLevel level = Debug);
	virtual bool isOpen() = 0;

    /// Returns if the messages are time-stamped.
	inline bool getTimeStamping() { return timeStamping_; }
	inline void setTimeStamping(const bool timeStamping) { timeStamping_ = timeStamping; }

    /// Returns if the messages are date-stamped.
	inline bool getDateStamping() { return dateStamping_; }
 	inline void setDateStamping(const bool dateStamping) { dateStamping_ = dateStamping; }

    /// Returns if the category is printed along with the messages
	inline bool getShowCat() { return showCat_; }
	inline void setShowCat(const bool showCat) { showCat_ = showCat; }

    /// Returns if the LogLevel is printed along with the messages.
	inline bool getShowLevel() { return showLevel_; }
	inline void setShowLevel(const bool showLevel) { showLevel_ = showLevel; }

protected:
	virtual bool testFilter(const std::string &cat, LogLevel level);
	virtual void logFiltered(const std::string &cat, LogLevel level, const std::string &msg,
                             const std::string &extendedInfo = "") = 0;
	virtual std::string getTimeString();
	virtual std::string getDateString();
	virtual std::string getLevelString(LogLevel level);

	bool timeStamping_, dateStamping_, showCat_, showLevel_;
	std::vector<LogFilter> filters_;
};


/// Implements logging to a plain Textfile
class TextLog : public Log {
public:
	TextLog(const std::string &filename, bool dateStamping = true, bool timeStamping= true,
            bool showCat = true, bool showLevel = true);
	virtual ~TextLog();
	bool isOpen();

protected:
	FILE* file_;
	void logFiltered(const std::string &cat, LogLevel level, const std::string &msg, const std::string &extendedInfo="");
    std::string getLevelColor(LogLevel level);
};

/**
 * Implements logging to stdout. Colored output on unix systems.
 * You can only have one ConsoleLog in the LogMgr
 */
class ConsoleLog : public Log {
public:
    ConsoleLog(bool dateStamping = false, bool timeStamping= false, bool showCat = true, bool showLevel = true);
    ~ConsoleLog() {};
    bool isOpen() { return true; }

protected:
    void logFiltered(const std::string &cat, LogLevel level, const std::string &msg, const std::string &extendedInfo="");
    std::string getLevelColor(LogLevel level);
};

///Implements a colored html log.
class HtmlLog : public Log {
public:
	HtmlLog(const std::string &filename, bool dateStamping = false, bool timeStamping= true,
            bool showCat = true, bool showLevel = true);
    ~HtmlLog();
	bool isOpen();

protected:
	void logFiltered(const std::string &cat, LogLevel level, const std::string &msg, const std::string &extendedInfo="");
    std::string getLevelString(LogLevel level);
    std::string getLevelColor(LogLevel level);

	FILE* file_;
};

/**
 * The Logmanager distributes logmessages to all Logs registered to the manager.
 * Logmessages consist of a message, a logging category and a loglevel.
 * See Log and LogLevel classes for further information.
 *
 * Usage:
 * Define "static const std::string loggerCat_;" in every class that logs messages and use the macros defined below:
 * LWARNING("Warning!");
 *
 * LDEBUG statements are removed if TGT_DEBUG is not defined!
 * @author Stefan Diepenbrock
 */
class LogManager {
public:
	LogManager(const std::string& logDir = "");
	~LogManager();

    /// Initialize logmanager, put all logfiles in logDir
	void reinit(const std::string& logDir);
	std::string getLogDir() { return logDir_; }

    /// Log message
	void log(const std::string &cat, LogLevel level, const std::string &msg, const std::string &extendedInfo="");

    /// Add a log to the manager, from now all messages recieved by the manager are also distributed to this log.
    /// All logs are deleted upon destruction of the manager!
    /// If a ConsoleLog is added it will replace an existing one, the old one will be deleted.
	void addLog(Log* log);

    /// Return the ConsoleLog (or 0 if there is none)
    ConsoleLog* getConsoleLog() { return consoleLog_; }

protected:
    std::string logDir_;
	std::vector<Log*> logs_;
    ConsoleLog* consoleLog_;
};

} // namespace

#define LogMgr tgt::Singleton<tgt::LogManager>::getRef()

// Use "do { ... } while (0)" to allow "if (foo) LINFO("bar"); else ...", which would fail
// otherwise.
// Compare: http://gcc.gnu.org/onlinedocs/cpp/Swallowing-the-Semicolon.html

#ifdef TGT_DEBUG
    #ifdef __GNUC__
        #define LDEBUG(msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __PRETTY_FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(loggerCat_, tgt::Debug, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LINFO(msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __PRETTY_FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(loggerCat_, tgt::Info, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LWARNING(msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __PRETTY_FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(loggerCat_, tgt::Warning, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LERROR(msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __PRETTY_FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(loggerCat_, tgt::Error, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LFATAL(msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __PRETTY_FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(loggerCat_, tgt::Fatal, _tmp.str(), _tmp2.str()); \
        } while (0)
        
        //with category parameter:
        #define LDEBUGC(cat, msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __PRETTY_FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(cat, tgt::Debug, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LINFOC(cat, msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __PRETTY_FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(cat, tgt::Info, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LWARNINGC(cat, msg) \
        { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __PRETTY_FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(cat, tgt::Warning, _tmp.str(), _tmp2.str()); \
        }

        #define LERRORC(cat, msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __PRETTY_FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(cat, tgt::Error, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LFATALC(cat, msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __PRETTY_FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(cat, tgt::Fatal, _tmp.str(), _tmp2.str()); \
        } while (0)
    #else
        #define LDEBUG(msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(loggerCat_, tgt::Debug, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LINFO(msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(loggerCat_, tgt::Info, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LWARNING(msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(loggerCat_, tgt::Warning, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LERROR(msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(loggerCat_, tgt::Error, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LFATAL(msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(loggerCat_, tgt::Fatal, _tmp.str(), _tmp2.str()); \
        } while (0)
        
        //with category parameter:
        #define LDEBUGC(cat, msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(cat, tgt::Debug, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LINFOC(cat, msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(cat, tgt::Info, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LWARNINGC(cat, msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(cat, tgt::Warning, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LERRORC(cat, msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(cat, tgt::Error, _tmp.str(), _tmp2.str()); \
        } while (0)

        #define LFATALC(cat, msg) \
        do { \
            std::ostringstream _tmp, _tmp2; \
            _tmp2 << __FUNCTION__  << " File: " << __FILE__ << "@" << __LINE__;\
            _tmp << msg; \
            LogMgr.log(cat, tgt::Fatal, _tmp.str(), _tmp2.str()); \
        } while (0)
     #endif
#else
    #define LDEBUG(msg)

    #define LINFO(msg) \
    do { \
        std::ostringstream _tmp; \
        _tmp << msg; \
        LogMgr.log(loggerCat_, tgt::Info, _tmp.str()); \
    } while (0)

    #define LWARNING(msg) \
    do { \
        std::ostringstream _tmp; \
        _tmp << msg; \
        LogMgr.log(loggerCat_, tgt::Warning, _tmp.str()); \
    } while (0)

    #define LERROR(msg) \
    do { \
        std::ostringstream _tmp; \
        _tmp << msg; \
        LogMgr.log(loggerCat_, tgt::Error, _tmp.str()); \
    } while (0)

    #define LFATAL(msg) \
    do { \
        std::ostringstream _tmp; \
        _tmp << msg; \
        LogMgr.log(loggerCat_, tgt::Fatal, _tmp.str()); \
    } while (0)

    //
    // with category parameter
    //

    #define LDEBUGC(cat, msg)

    #define LINFOC(cat, msg) \
    do { \
        std::ostringstream _tmp; \
        _tmp << msg; \
        LogMgr.log(cat, tgt::Info, _tmp.str()); \
    } while (0)

    #define LWARNINGC(cat, msg) \
    do { \
        std::ostringstream _tmp; \
        _tmp << msg; \
        LogMgr.log(cat, tgt::Warning, _tmp.str()); \
    } while (0)

    #define LERRORC(cat, msg) \
    do { \
        std::ostringstream _tmp; \
        _tmp << msg; \
        LogMgr.log(cat, tgt::Error, _tmp.str()); \
    } while (0)

    #define LFATALC(cat, msg) \
    do { \
        std::ostringstream _tmp; \
        _tmp << msg; \
        LogMgr.log(cat, tgt::Fatal, _tmp.str()); \
    } while (0)
    
#endif //TGT_DEBUG

#endif //TGT_LOGMANAGER_H