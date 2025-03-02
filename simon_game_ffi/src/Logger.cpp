#include "Logger.hpp"

std::ofstream Logger::logFile;
std::mutex Logger::logMutex;
std::atomic<bool> Logger::fileLoggingEnabled(false);
std::atomic<Logger::LogLevel> Logger::minLogLevel(Logger::LogLevel::INFO);
std::string Logger::logFilePath;

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    std::tm tm_buf;
    localtime_s(&tm_buf, &now_c);
    
    ss << std::setfill('0') << std::setw(2) << tm_buf.tm_hour << "."
       << std::setfill('0') << std::setw(2) << tm_buf.tm_min << "."
       << std::setfill('0') << std::setw(2) << tm_buf.tm_sec << "."
       << std::setfill('0') << std::setw(3) << now_ms.count();
       
    return ss.str();
}

std::string Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:    return "[DEBUG]";
        case LogLevel::INFO:     return "[INFO]";
        case LogLevel::MAIN:     return "[MAIN]";
        case LogLevel::WARNING:  return "[WARNING]";
        case LogLevel::ERROR_LEVEL: return "[ERROR]";
        case LogLevel::CRITICAL: return "[CRITICAL]";
        default:                 return "[UNKNOWN]";
    }
}

void Logger::setConsoleColor(LogLevel level) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    switch (level) {
        case LogLevel::DEBUG:
            SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
            break;
        case LogLevel::INFO:
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE);
            break;
        case LogLevel::MAIN:
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
            break;
        case LogLevel::WARNING:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
            break;
        case LogLevel::ERROR_LEVEL:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
            break;
        case LogLevel::CRITICAL:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            break;
        default:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            break;
    }
}

void Logger::rotateLogFileIfNeeded() {
    if (!fileLoggingEnabled) return;
    
    std::lock_guard<std::mutex> lock(logMutex);
    
    if (!logFile.is_open()) return;
    
    logFile.flush();
    if (std::filesystem::file_size(logFilePath) > MAX_LOG_SIZE) {
        logFile.close();
        
        std::string backupPath = logFilePath + ".old";
        if (std::filesystem::exists(backupPath)) {
            std::filesystem::remove(backupPath);
        }
        std::filesystem::rename(logFilePath, backupPath);
        
        logFile.open(logFilePath, std::ios::out | std::ios::app);
        if (logFile.is_open()) {
            logFile << "[" << getCurrentTimestamp() << "] " << "[INFO]: " << "Log file rotated" << std::endl;
        }
    }
}

bool Logger::initialize(const std::string& filePath, LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    minLogLevel = level;
    
    if (filePath.empty()) {
        fileLoggingEnabled = false;
        return true;
    }
    
    logFilePath = filePath;
    fileLoggingEnabled = true;
    
    if (logFile.is_open()) {
        logFile.close();
    }
    
    logFile.open(filePath, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        fileLoggingEnabled = false;
        return false;
    }
    
    logFile << "[" << getCurrentTimestamp() << "] " << "[INFO]: " << "Logging initialized" << std::endl;
    return true;
}

void Logger::setLogLevel(LogLevel level) {
    minLogLevel = level;
}

void Logger::log(LogLevel level, const std::string& message, const char* file, int line) {
    if (level < minLogLevel) return;
    
    std::lock_guard<std::mutex> lock(logMutex);
    
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = getLevelString(level);
    std::thread::id threadId = std::this_thread::get_id();
    std::stringstream threadIdStr;
    threadIdStr << threadId;
    
    std::ostringstream logMsg;
    logMsg << "[" << timestamp << "] " << levelStr << " <" << threadIdStr.str() << "> ";
    
    if (file != nullptr) {
        std::string filename(file);
        auto pos = filename.find_last_of("/\\");
        if (pos != std::string::npos) {
            filename = filename.substr(pos + 1);
        }
        
        logMsg << "(" << filename << ":" << line << ") ";
    }
    
    logMsg << message;
    
    setConsoleColor(level);
    std::cout << logMsg.str() << std::endl;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    
    OutputDebugStringA((logMsg.str() + "\n").c_str());
    
    if (fileLoggingEnabled && logFile.is_open()) {
        logFile << logMsg.str() << std::endl;
        logFile.flush();
        
        rotateLogFileIfNeeded();
    }
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile << "[" << getCurrentTimestamp() << "] " << "[INFO]: " << "Logging shutdown" << std::endl;
        logFile.close();
    }
}