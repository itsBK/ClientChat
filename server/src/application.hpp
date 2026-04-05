#pragma once

class Application
{
    static void captureSignal(int sig);
public:
    ///
    /// @param argc
    /// @param argv
    /// @return 0 on success, -1 on error
    static int parseArgs(int argc, char **argv);
    static void captureExitSignals();
    static void cleanup();
};
