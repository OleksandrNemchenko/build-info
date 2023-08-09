
#include <array>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std::string_literals;

extern const std::string _patternNotFoundInInputFile1 = "Pattern "s;
extern const std::string _patternNotFoundInInputFile2 = " has not been found in the input file "s;
extern const std::string _errorCommandExecuting = "Error executing command: "s;
extern const std::string _usage1 = "Usage: "s;
extern const std::string _usage2 = " --template-file <template_file> --build-time <build_time_template> "
    "--result-file <result_file> --git-commit <git_commit_template> "
    "--git-time <git_time_template> --build-number-pattern <build_number_pattern> --build-number-file <build_number_file>"
    "[--dont-increment-build-number]";
extern const std::string _invalidArgument = "Invalid argument: "s;
extern const std::string _notAGitDir = "Current directory seems to be not a git repository";
extern const std::string _errorInputFile = "Error opening input file "s;
extern const std::string _errorOutputFile = "Error opening output file "s;

std::string replaceAll(std::string& source, const std::string& search, const std::string& replace, const std::string& sourceFile) noexcept
{
    size_t pos = 0;
    bool replacedAtLeastOnce = false;
    while ((pos = source.find(search, pos)) != std::string::npos)
    {
        source.replace(pos, search.length(), replace);
        pos += replace.length();

        replacedAtLeastOnce = true;
    }

    if (!replacedAtLeastOnce)
        std::cerr << _patternNotFoundInInputFile1 << search << _patternNotFoundInInputFile2 << sourceFile << std::endl;

    return source;
}

class CFile
{
public:
    CFile(const std::string& command) : _pipe(
#ifdef _WIN32
        _popen(command.c_str(), "r")
#else
        popen(command.c_str(), "r")
#endif
    )
    {
        if (!_pipe)
            throw std::runtime_error(_errorCommandExecuting + command);
    }

    ~CFile() noexcept
    {
#ifdef _WIN32
        _pclose(_pipe);
#else
        pclose(_pipe);
#endif
    }

    operator FILE* () noexcept { return _pipe; }

private:
    FILE* _pipe = nullptr;
};

std::string executeCommand(const std::string& command, const bool removeLastEOL = true)
{
    std::array<char, 128> buffer;

    CFile pipe(command);

    std::string result;

    while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
        result += buffer.data();

    if (removeLastEOL && !result.empty() && result.back() == '\n')
        result.pop_back();

    return result;
}

std::string getCurrentCommit() noexcept
{
    static const std::string command = "git rev-parse HEAD";
    return executeCommand(command);
}

std::string getCurrentCommitDateUnixTime() noexcept
{
    static const std::string command = "git log -1 --format=%ct";
    return executeCommand(command);
}

bool isGitResource() noexcept
{
    static const std::string command = "git rev-parse --is-inside-work-tree";
    const std::string result = executeCommand(command);
    return result == "true"s;
}

size_t getBuildNumber(const std::string& buildNumberFile, bool incrementBuildNumber)
{
    size_t buildNumber;

    std::ifstream inputFile(buildNumberFile);
    if (!inputFile)
    {
        buildNumber = 1 - 1 /* wil lbe incremented later */;
        if (!incrementBuildNumber)
            return 1;
    }
    else
    {
        std::stringstream buffer;
        buffer << inputFile.rdbuf();
        inputFile.close();

        std::string content = buffer.str();
        buildNumber = static_cast<decltype(buildNumber)>(std::stoull(content));
    }

    if (!incrementBuildNumber)
        return buildNumber;
    ++buildNumber;

    std::ofstream outputFile(buildNumberFile, std::ios::trunc);
    if (!outputFile)
        throw std::runtime_error(_errorInputFile + buildNumberFile);

    outputFile << buildNumber;

    return buildNumber;
}

int main(int argc, char* argv[]) {
    
    try
    {
        constexpr size_t argumentsAmount = 14;
        if (argc < argumentsAmount)
            throw std::runtime_error(_usage1 + std::string(argv[0]) + _usage2);

        std::string templateFile, buildTimeTemplate, resultFile, gitCommitTemplate, gitTimeTemplate, buildNumberTemplate, buildNumberFile;
        bool incrementBuildNumber = true;

        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "--template-file")                    templateFile =         argv[++i];
            else if (arg == "--result-file")                 resultFile =           argv[++i];
            else if (arg == "--build-time")                  buildTimeTemplate =    argv[++i];
            else if (arg == "--git-commit")                  gitCommitTemplate =    argv[++i];
            else if (arg == "--git-time")                    gitTimeTemplate =      argv[++i];
            else if (arg == "--build-number-pattern")        buildNumberTemplate =  argv[++i];
            else if (arg == "--build-number-file")           buildNumberFile =      argv[++i];
            else if (arg == "--dont-increment-build-number") incrementBuildNumber = false;
            else                                             throw std::runtime_error(_invalidArgument + arg);
        }

        const bool isGitRepository = isGitResource();
        if (!isGitRepository)
            std::cerr << _notAGitDir << std::endl;

        std::ifstream inputFile(templateFile);
        if (!inputFile)
            throw std::runtime_error(_errorInputFile + std::string(templateFile));

        std::stringstream buffer;

        buffer << inputFile.rdbuf();
        std::string content = buffer.str();

        inputFile.close();

        const std::time_t currentTime = std::time(nullptr);
        std::stringstream unixTime;
        unixTime << currentTime;
        std::string buildTime = unixTime.str();
        content = replaceAll(content, buildTimeTemplate, buildTime, templateFile);

        const std::string gitCommit = getCurrentCommit();
        content = replaceAll(content, gitCommitTemplate, gitCommit, templateFile);

        const std::string gitTime = getCurrentCommitDateUnixTime();
        content = replaceAll(content, gitTimeTemplate, gitTime, templateFile);

        const std::string buildNumber = std::to_string(getBuildNumber(buildNumberFile, incrementBuildNumber));
        content = replaceAll(content, buildNumberTemplate, buildNumber, templateFile);

        std::ofstream outputFile(resultFile);
        if (!outputFile)
            throw std::runtime_error(_errorOutputFile + resultFile);

        outputFile << content;

        return 0;
    }
    catch (std::runtime_error err)
    {
        std::cerr << err.what() << std::endl;
        return -1;
    }
}
