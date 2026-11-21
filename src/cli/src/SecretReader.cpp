#include "SecretReader.hpp"

#include <openssl/crypto.h>
#include <termios.h>
#include <unistd.h>

#include <csignal>
#include <iostream>

namespace
{
// Terminal settings to restore if the program is interrupted while echo is off.
termios g_saved_terminal;

void restoreTerminalAndExit(int signal)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &g_saved_terminal);
    std::signal(signal, SIG_DFL);
    std::raise(signal);
}

// Turns terminal echo off for its lifetime and restores it afterwards, also on Ctrl+C.
class EchoOff
{
public:
    EchoOff()
    {
        if (tcgetattr(STDIN_FILENO, &g_saved_terminal) != 0)
        {
            return;  // not a real terminal: nothing to hide
        }

        m_active = true;
        m_previous_sigint = std::signal(SIGINT, restoreTerminalAndExit);
        m_previous_sigterm = std::signal(SIGTERM, restoreTerminalAndExit);

        termios hidden = g_saved_terminal;
        hidden.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &hidden);
    }

    ~EchoOff()
    {
        if (!m_active)
        {
            return;
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &g_saved_terminal);
        std::signal(SIGINT, m_previous_sigint);
        std::signal(SIGTERM, m_previous_sigterm);
    }

    EchoOff(const EchoOff&) = delete;
    EchoOff& operator=(const EchoOff&) = delete;

private:
    bool m_active = false;
    void (*m_previous_sigint)(int) = SIG_DFL;
    void (*m_previous_sigterm)(int) = SIG_DFL;
};
}  // namespace

void wipeSecret(std::string& secret)
{
    OPENSSL_cleanse(secret.data(), secret.size());
    secret.clear();
}

std::optional<std::string> readSecret(const std::string& prompt)
{
    return readSecret(prompt, std::cin, std::cerr, isatty(STDIN_FILENO));
}

std::optional<std::string> readSecret(const std::string& prompt, std::istream& in,
                                      std::ostream& prompt_out, bool interactive)
{
    std::string secret;
    bool got_line = false;

    if (interactive)
    {
        prompt_out << prompt << std::flush;

        EchoOff echo_off;
        got_line = static_cast<bool>(std::getline(in, secret));

        // The newline typed by the user was not echoed.
        prompt_out << "\n";
    }
    else
    {
        got_line = static_cast<bool>(std::getline(in, secret));
    }

    if (!secret.empty() && secret.back() == '\r')
    {
        secret.pop_back();
    }

    if (!got_line || secret.empty())
    {
        return std::nullopt;
    }

    return secret;
}
