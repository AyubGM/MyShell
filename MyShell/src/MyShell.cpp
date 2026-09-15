#include <iostream>
#include <string>
#include <print>
#include <filesystem>
#include <format>
#define NOMINMAX
#include <windows.h>

void PrintTokens(const std::vector<std::wstring>& tokens)
{
    for (size_t i{}; i < tokens.size(); i++)
    {
      std::wcout << std::format(L"Token{}: {}", i, tokens[i]) << L'\n';
    }
}

std::vector<std::wstring> TokenizeCommand(const std::wstring& line)
{
    std::vector<std::wstring> tokens;
    std::wstring currentToken;

    bool inQuotes = false;

    for (wchar_t ch : line)
    {
        if (ch == L'"')
        {
            inQuotes = !inQuotes; // Toggle state when encountering quotes
        }
        else if (ch == L' ' && !inQuotes)
        {
            if (!currentToken.empty())
            {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        }
        else
        {
            currentToken += ch;
        }
    }

    if (!currentToken.empty())
    {
        tokens.push_back(currentToken);
    }

    return tokens;
}

std::wstring ExpandEnvironmentVariables(const std::wstring& input)
{
    DWORD size = ExpandEnvironmentStringsW(input.c_str(), nullptr, 0);
    if (size == 0) return input;

    std::wstring expanded(size, L'\0');
    ExpandEnvironmentStringsW(input.c_str(), expanded.data(), size);
    expanded.resize(size - 1); // Remove null-terminator padding
    return expanded;
}

bool HandelBuiltIns(const std::wstring& command, const std::vector<std::wstring>& args)
{
    if (command == L"exit")
    {
        std::exit(0);
    }
    else if (command == L"cd")
    {
        if (args.size() < 2)
        {
            std::println("cd: missing path argument");
            return true;
        }

        std::error_code ec;
        std::filesystem::current_path(args[1], ec);
        if (ec)
        {
            std::println("cd: failed to change directory: {}", ec.message());
        }
        return true;
    }
    return false;
}
void ExecuteCommand(const std::wstring& commandLine, const std::vector<std::wstring>& args) 
{
    STARTUPINFO si{ .cb = sizeof(si) };
    PROCESS_INFORMATION pi{};

    std::wstring finalCommandLine = commandLine;

    std::filesystem::path exePath(args[0]);
    std::wstring ext = exePath.extension().wstring();

    if (ext == L".bat" || ext == L".cmd")
    {
        finalCommandLine = L"cmd.exe /c " + commandLine;
    }
    else if (ext == L".ps1")
    {
        finalCommandLine = L"powershell.exe -ExecutionPolicy Bypass -File " + commandLine;
    }
    else if (ext == L".sh")
    {
        finalCommandLine = L"wsl.exe bash -c \"" + commandLine + L"\"";
    }

    BOOL success = CreateProcess(
        nullptr,
        finalCommandLine.data(),
        nullptr, nullptr,
        FALSE, 0, nullptr, nullptr,
        &si, &pi
    );

    if (success)
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else
    {
        /// Fallback for built-ins like 'dir', 'cls', or extensionless batch scripts
        std::wstring cmdFallback = L"cmd.exe /c " + commandLine;
        success = CreateProcess(
            nullptr, cmdFallback.data(),
            nullptr, nullptr, FALSE, 0, nullptr, nullptr,
            &si, &pi
        );

        if (success)
        {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else
        {
            std::println("Command failed or not found. Error code: {}", GetLastError());
        }
    }
}


int main(int argc, char* argv[])
{
    while (true)
    {
        std::print("{}>", std::filesystem::current_path().string());

        std::wstring line;
        if (!std::getline(std::wcin, line) || line.empty())
        {
            std::println("empty input");
            continue;
        }

        line = ExpandEnvironmentVariables(line);


		std::vector<std::wstring> args = TokenizeCommand(line);

		if (args.empty())
		{
			std::println("empty input");
			continue;
		}

        PrintTokens(args);

        if (!HandelBuiltIns(args[0], args))
        {
            ExecuteCommand(line, args);
        }

    }
    

} 

