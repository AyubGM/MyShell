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

		std::vector<std::wstring> args = TokenizeCommand(line);

		if (args.empty())
		{
			std::println("empty input");
			continue;
		}

        PrintTokens(args);

    }
    

} 

