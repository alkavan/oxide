/*
 *  Copyright (C) 2025-2026 Igal Alkon and ALKONTEK
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */
#include <oxide/assert.hpp>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <sys/wait.h>
#  include <unistd.h>
#  include <fcntl.h>
#endif

namespace {

constexpr int kDidNotAbortExitCode = 64;
constexpr std::string_view kAbortFlag = "--oxide-expect-abort";

// MSVC abort()/fail-fast commonly reports as:
//   3           - older CRT abort
//   0xC0000409  - STATUS_STACK_BUFFER_OVERRUN (fast fail); decimal 3221226505
constexpr std::uint32_t kWindowsAbortExitCode = 3;
constexpr std::uint32_t kWindowsFastFailCode = 0xC0000409u;

void banner(const std::string_view title) {
    std::cout << "\n=== " << title << " ===\n";
}

using CaseFn = void (*)();

std::unordered_map<std::string_view, CaseFn>& abort_cases() {
    static std::unordered_map<std::string_view, CaseFn> cases;
    return cases;
}

#if defined(_WIN32)
void enable_utf8_console() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

std::wstring wide_from_utf8(const std::string& s) {
    if (s.empty()) {
        return {};
    }
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (n <= 0) {
        return {};
    }
    std::wstring out(static_cast<std::size_t>(n - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, out.data(), n);
    return out;
}

std::wstring current_executable_path_w() {
    std::wstring path(MAX_PATH, L'\0');
    DWORD n = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    if (n == 0) {
        return {};
    }
    if (n >= path.size()) {
        path.resize(n + 1);
        n = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
        if (n == 0 || n >= path.size()) {
            return {};
        }
    }
    path.resize(n);
    return path;
}

bool is_abortish_exit_code(const DWORD code) {
    return code == kWindowsAbortExitCode
        || code == kWindowsFastFailCode
        || (code != 0 && code != static_cast<DWORD>(kDidNotAbortExitCode));
}

// Run child, capture combined stdout/stderr, print it as one block.
bool run_child_reexec(const std::string_view label) {
    const std::wstring exe = current_executable_path_w();
    if (exe.empty()) {
        std::cout << "FAILED (exe path)\n";
        return false;
    }

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE read_pipe = nullptr;
    HANDLE write_pipe = nullptr;
    if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) {
        std::cout << "FAILED (CreatePipe)\n";
        return false;
    }
    // Parent side must NOT be inheritable.
    SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);

    std::wstring cmd = L"\"" + exe + L"\" ";
    cmd += wide_from_utf8(std::string(kAbortFlag));
    cmd += L" ";
    cmd += wide_from_utf8(std::string(label));

    std::vector<wchar_t> cmdline(cmd.begin(), cmd.end());
    cmdline.push_back(L'\0');

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = write_pipe;
    si.hStdError = write_pipe;

    PROCESS_INFORMATION pi{};

    // Avoid parent/child console races: flush before spawn.
    std::cout.flush();
    std::cerr.flush();

    const BOOL created = CreateProcessW(
        exe.c_str(),
        cmdline.data(),
        nullptr,
        nullptr,
        TRUE,  // inherit pipe write end
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    // Parent must close its copy of the write end before reading.
    CloseHandle(write_pipe);
    write_pipe = nullptr;

    if (!created) {
        CloseHandle(read_pipe);
        std::cout << "FAILED (CreateProcess)\n";
        return false;
    }

    std::string child_out;
    char buf[4096];
    for (;;) {
        DWORD got = 0;
        if (!ReadFile(read_pipe, buf, sizeof(buf), &got, nullptr) || got == 0) {
            break;
        }
        child_out.append(buf, buf + got);
    }
    CloseHandle(read_pipe);

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    if (!child_out.empty()) {
        // Print child panic output as its own block (no interleaving).
        if (child_out.back() != '\n') {
            child_out.push_back('\n');
        }
        std::cerr << child_out << std::flush;
    }

    if (code == static_cast<DWORD>(kDidNotAbortExitCode)) {
        std::cout << "FAILED (code ran but did not abort)\n" << std::flush;
        return false;
    }

    if (is_abortish_exit_code(code)) {
        std::cout << "ok (child exited with code " << code << ")\n" << std::flush;
        return true;
    }

    std::cout << "FAILED (child exited 0 unexpectedly)\n" << std::flush;
    return false;
}
#endif  // _WIN32

bool run_expecting_abort(const std::string_view label, CaseFn body) {
    std::cout << "[expect abort] " << label << " ... " << std::flush;

#if defined(_WIN32)
    (void)body;
    return run_child_reexec(label);
#else
    const pid_t pid = fork();
    if (pid < 0) {
        std::cout << "FAILED (fork)\n";
        return false;
    }

    if (pid == 0) {
        body();
        std::_Exit(kDidNotAbortExitCode);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        std::cout << "FAILED (waitpid)\n";
        return false;
    }

    if (WIFSIGNALED(status)) {
        std::cout << "ok (child terminated by signal " << WTERMSIG(status);
#  ifdef SIGABRT
        if (WTERMSIG(status) == SIGABRT) {
            std::cout << " / SIGABRT";
        }
#  endif
        std::cout << ")\n" << std::flush;
        return true;
    }

    if (WIFEXITED(status)) {
        const int code = WEXITSTATUS(status);
        if (code == kDidNotAbortExitCode) {
            std::cout << "FAILED (code ran but did not abort)\n";
        } else {
            std::cout << "FAILED (child exited with code " << code << ")\n";
        }
        return false;
    }

    std::cout << "FAILED (unexpected child status)\n";
    return false;
#endif
}

int run_abort_child(const std::string_view label) {
    const auto it = abort_cases().find(label);
    if (it == abort_cases().end()) {
        std::cerr << "unknown abort case: " << label << '\n';
        return 2;
    }
    it->second();
    std::_Exit(kDidNotAbortExitCode);
}

void demo_passing_checks() {
    using namespace oxide;

    banner("passing function API");

    constexpr int size = 4;
    constexpr int index = 2;
    const int* ptr = &size;

    check(index < size);
    check(ptr != nullptr, "ptr must not be null");
    assert_that(size > 0);
    assert_that(index >= 0, "index must be non-negative");

    std::cout << "check/assert_that (functions): ok\n";
}

void case_check_false() {
    oxide::check(false, "manual check failure");
}

void case_assert_false() {
    oxide::assert_that(false, "manual assertion failure");
}

void demo_failing_checks() {
    banner("failing checks (child process should abort)");

    run_expecting_abort("oxide::check(false)", &case_check_false);
    run_expecting_abort("oxide::assert_that(false)", &case_assert_false);
}

}  // namespace

int main(int argc, char** argv) {
#if defined(_WIN32)
    enable_utf8_console();
#endif

    // Fixed registry: child must resolve labels without parent heap state.
    abort_cases().emplace("oxide::check(false)", &case_check_false);
    abort_cases().emplace("oxide::assert_that(false)", &case_assert_false);

    if (argc >= 3 && std::string_view(argv[1]) == kAbortFlag) {
        return run_abort_child(argv[2]);
    }

    std::cout << "Note: panic uses std::abort(); it cannot be caught with try/catch.\n";
    std::cout << "      Aborts are isolated in a child process "
                 "(fork on POSIX, re-exec on Windows).\n";

    demo_passing_checks();
    demo_failing_checks();

    banner("done");
    std::cout << "Passing checks ran in-process.\n";
    std::cout << "Failing checks were isolated so the demo can keep going.\n";
    return 0;
}
