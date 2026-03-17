#include <drogon/drogon.h>
#include <algorithm>
#include <cctype>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <thread>
#include <unistd.h>
#include <ctime>
#include "config/Config.hpp"
#include "controllers/Routes.hpp"

namespace {
    constexpr std::string_view kAnsiReset = "\033[0m";
    constexpr std::string_view kAnsiTime = "\033[38;5;45m";
    constexpr std::string_view kAnsiFile = "\033[38;5;246m";
    constexpr std::string_view kAnsiTrace = "\033[38;5;141m";
    constexpr std::string_view kAnsiDebug = "\033[38;5;75m";
    constexpr std::string_view kAnsiInfo = "\033[38;5;42m";
    constexpr std::string_view kAnsiWarn = "\033[38;5;220m";
    constexpr std::string_view kAnsiError = "\033[38;5;203m";
    constexpr std::string_view kAnsiFatal = "\033[1;37;41m";

    bool isTimeToken(const std::string_view token) {
        return token.size() >= 8 &&
               std::isdigit(token[0]) != 0 &&
               std::isdigit(token[1]) != 0 &&
               token[2] == ':' &&
               std::isdigit(token[3]) != 0 &&
               std::isdigit(token[4]) != 0 &&
               token[5] == ':' &&
               std::isdigit(token[6]) != 0 &&
               std::isdigit(token[7]) != 0;
    }

    std::string_view colorForLevel(const std::string_view token) {
        if (token == "TRACE") return kAnsiTrace;
        if (token == "DEBUG") return kAnsiDebug;
        if (token == "INFO") return kAnsiInfo;
        if (token == "WARN") return kAnsiWarn;
        if (token == "ERROR") return kAnsiError;
        if (token == "FATAL") return kAnsiFatal;
        return {};
    }

    std::string colorizeLogLine(const char* rawMessage, const uint64_t rawLength) {
        std::string_view message(rawMessage, static_cast<size_t>(rawLength));
        if (!::isatty(STDOUT_FILENO)) {
            return std::string(message);
        }

        const auto firstNewLine = message.find('\n');
        const std::string_view line = firstNewLine == std::string_view::npos
                                        ? message
                                        : message.substr(0, firstNewLine);
        const std::string_view suffix = firstNewLine == std::string_view::npos
                                          ? std::string_view{}
                                          : message.substr(firstNewLine);

        std::string output;
        output.reserve(message.size() + 64);

        size_t tokenStart = 0;
        std::string_view formattedTime;
        int tokenIndex = 0;

        while (tokenStart < line.size() && tokenIndex < 2) {
            const size_t tokenEnd = line.find(' ', tokenStart);
            const std::string_view token = line.substr(
                tokenStart,
                tokenEnd == std::string_view::npos ? line.size() - tokenStart
                                                   : tokenEnd - tokenStart);
            if (tokenIndex == 1 && isTimeToken(token)) {
                formattedTime = token.substr(0, 8);
                break;
            }

            if (tokenEnd == std::string_view::npos) {
                break;
            }

            tokenStart = line.find_first_not_of(' ', tokenEnd);
            ++tokenIndex;
        }

        if (!formattedTime.empty()) {
            output.append(kAnsiTime);
            output.append("[");
            output.append(formattedTime);
            output.append("]");
            output.append(kAnsiReset);
            output.push_back(' ');
        }

        tokenStart = 0;
        tokenIndex = 0;

        while (tokenStart < line.size()) {
            const size_t tokenEnd = line.find(' ', tokenStart);
            const std::string_view token = line.substr(
                tokenStart,
                tokenEnd == std::string_view::npos ? line.size() - tokenStart
                                                   : tokenEnd - tokenStart);

            if (tokenIndex < 3) {
                if (tokenEnd == std::string_view::npos) {
                    break;
                }

                tokenStart = line.find_first_not_of(' ', tokenEnd);
                ++tokenIndex;
                continue;
            }

            const std::string_view color = colorForLevel(token);

            if (!color.empty()) {
                output.append(color);
                output.append(token);
                output.append(kAnsiReset);
            } else {
                output.append(token);
            }

            if (tokenEnd == std::string_view::npos) {
                break;
            }

            const size_t nextTokenStart = line.find_first_not_of(' ', tokenEnd);
            output.append(line.substr(tokenEnd,
                                      (nextTokenStart == std::string_view::npos
                                           ? line.size()
                                           : nextTokenStart) - tokenEnd));
            tokenStart = nextTokenStart;
            ++tokenIndex;
        }

        const size_t locationSeparator = output.rfind(" - ");
        if (locationSeparator != std::string::npos) {
            const auto fileLocation = output.substr(locationSeparator + 3);
            output.erase(locationSeparator + 3);
            output.append(kAnsiFile);
            output.append(fileLocation);
            output.append(kAnsiReset);
        }

        output.append(suffix);
        return output;
    }

    void configureLogger() {
        ::setenv("TZ", "America/Sao_Paulo", 1);
        ::tzset();
        trantor::Logger::setDisplayLocalTime(true);
        trantor::Logger::setOutputFunction(
            [](const char* message, const uint64_t length) {
                const auto colored = colorizeLogLine(message, length);
                std::fwrite(colored.data(), 1, colored.size(), stdout);
            },
            []() {
                std::fflush(stdout);
            });
    }

    double requestLatencyMs(const drogon::HttpRequestPtr& request) {
        const auto now = trantor::Date::now().microSecondsSinceEpoch();
        const auto startedAt = request->getCreationDate().microSecondsSinceEpoch();
        return static_cast<double>(now - startedAt) / 1000.0;
    }

    std::string formatLatencyMs(const double latencyMs) {
        std::ostringstream output;
        output << std::fixed << std::setprecision(2) << latencyMs;
        return output.str();
    }

    void configureRequestLogging(drogon::HttpAppFramework& app) {
        app.registerPreSendingAdvice(
            [](const drogon::HttpRequestPtr& request,
               const drogon::HttpResponsePtr& response) {
                if (request->path() == "/favicon.ico") {
                    return;
                }

                LOG_INFO << request->methodString()
                         << " " << request->path()
                         << " -> " << response->statusCode()
                         << " in " << formatLatencyMs(requestLatencyMs(request)) << "ms"
                         << " from " << request->peerAddr().toIpPort();
            });
    }

    void addCorsHeaders(const drogon::HttpResponsePtr& response) {
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
        response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }

    void configureCors(drogon::HttpAppFramework& app) {
        app.registerPreRoutingAdvice(
            [](const drogon::HttpRequestPtr& req,
            drogon::AdviceCallback&& callback,
            drogon::AdviceChainCallback&& next) {
                if (req->method() != drogon::Options) {
                    next();
                    return;
                }

                auto response = drogon::HttpResponse::newHttpResponse();
                addCorsHeaders(response);
                response->addHeader("Access-Control-Max-Age", "86400");
                callback(response);
            });

        app.registerPreSendingAdvice(
            [](const drogon::HttpRequestPtr&, const drogon::HttpResponsePtr& response) {
                addCorsHeaders(response);
            });
    }

    void configureUploadPath(drogon::HttpAppFramework& app) {
        app.setUploadPath(std::filesystem::temp_directory_path().string());
    }
}

int main() {
    auto config = Config::load();
    configureLogger();

    std::signal(SIGINT, [](int) {
        LOG_INFO << "Shutting down...";
        drogon::app().quit();
    });
    std::signal(SIGTERM, [](int) {
        LOG_INFO << "Shutting down...";
        drogon::app().quit();
    });

    registerAllRoutes();

    auto& app = drogon::app();
    app.addListener("0.0.0.0", config.port)
       .setThreadNum(std::thread::hardware_concurrency())
       .setLogLevel(trantor::Logger::kInfo);
    configureUploadPath(app);
    configureRequestLogging(app);

    if (config.corsEnabled) {
        configureCors(app);
    }

    LOG_INFO << "Server starting on port " << config.port;
    app.run();

    LOG_INFO << "Server stopped";
    return 0;
}
