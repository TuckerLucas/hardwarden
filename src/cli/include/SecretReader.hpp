#pragma once

#include <istream>
#include <optional>
#include <ostream>
#include <string>

// Reads a secret, such as a PIN, from standard input.
// On a terminal, the prompt is shown on stderr and the typed characters are not echoed.
// Otherwise, one line is read without a prompt.
// Returns nothing if the input is empty or ends before a secret is given.
std::optional<std::string> readSecret(const std::string& prompt);

// Overwrites the secret in memory, so it does not linger after use.
void wipeSecret(std::string& secret);

// Same as above, with the input, the prompt output and the terminal check passed in.
std::optional<std::string> readSecret(const std::string& prompt, std::istream& in,
                                      std::ostream& prompt_out, bool interactive);
