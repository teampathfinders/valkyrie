#pragma once

#include <stacktrace>
#include <expected>

namespace valk {
    enum class ErrorKind {
        Unknown
    };

    class Error {
    public:
        Error(ErrorKind kind, std::string message) : m_kind(kind), m_message(std::move(message)) {}
        explicit Error(std::string message) : m_message(std::move(message)) {}
        explicit Error(ErrorKind kind) noexcept : m_kind(kind) {}

        const std::stacktrace& stacktrace() const noexcept {
            return m_trace;
        }

        const std::string& message() const noexcept {
            return m_message;
        }

        ErrorKind kind() const noexcept {
            return m_kind;
        }

    private:
        ErrorKind m_kind = ErrorKind::Unknown;
        std::string m_message = "unknown";
        std::stacktrace m_trace = std::stacktrace::current();
    };

    template<typename T>
    using ValkExpect = std::expected<T, Error>;
}
