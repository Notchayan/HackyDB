#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <readline/history.h>
#include <readline/readline.h>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "Interpreter/interpreter.h"

using namespace std;

// Prints a beautiful welcome message about HackyDB
void printWelcomeMessage() {
    // Clear the console (cross-platform approach)
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
    
    // Colors for terminal text (ANSI escape codes)
    const string RESET = "\033[0m";
    const string BOLD = "\033[1m";
    const string BLUE = "\033[34m";
    const string GREEN = "\033[32m";
    const string CYAN = "\033[36m";
    const string YELLOW = "\033[33m";
    const string MAGENTA = "\033[35m";
    
    // Animated typing effect for the ASCII art logo
    string logo[] = {
        "██╗  ██╗ █████╗  ██████╗██╗   ██╗██╗   ██╗ ",
        "██║  ██║██╔══██╗██╔════╝██║  ██║ ╚██╗ ██╔╝",
        "███████║███████║██║     ████║     ╚████╔╝ ",
        "██╔══██║██╔══██║██║     ██╔═ ██║   ╚██╔╝  ",
        "██║  ██║██║  ██║╚██████╗██║   ██║   ██║   ",
        "╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝   ╚═╝   ╚═╝   "
    };
    
    // Display logo with animation
    cout << endl;
    for (const string& line : logo) {
        cout << CYAN << BOLD << line << RESET << endl;
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    cout << endl;
    
    cout << BLUE << BOLD << "═══════════════════════════════════════════════════════════════════════════════" << RESET << endl;
    cout << GREEN << BOLD << "               Welcome to HackyDB - Your Very Own Database System!            " << RESET << endl;
    cout << BLUE << BOLD << "═══════════════════════════════════════════════════════════════════════════════" << RESET << endl;
    cout << CYAN << BOLD << "                                                                               " << RESET << endl;
    cout << CYAN << BOLD << "         HackyDB is a powerful, complete database system built in C++.         " << RESET << endl;
    cout << CYAN << BOLD << "    With support for all core database operations including system calls,      " << RESET << endl;
    cout << CYAN << BOLD << "     indexing, file handling, and record management, you're ready to go!       " << RESET << endl;
    cout << CYAN << BOLD << "    HackyDB integrates OS-level data management to bring you a truly robust     " << RESET << endl;
    cout << CYAN << BOLD << "    solution for managing data efficiently. Start exploring and building!      " << RESET << endl;
    cout << CYAN << BOLD << "                                                                               " << RESET << endl;
    cout << BLUE << BOLD << "═══════════════════════════════════════════════════════════════════════════════" << RESET << endl;
    cout << GREEN << BOLD << "         Ready to manage data with full control? Let's dive into HackyDB!      " << RESET << endl;
    cout << BLUE << BOLD << "═══════════════════════════════════════════════════════════════════════════════" << RESET << endl;

    // Project information
    cout << YELLOW << "🔹 " << BOLD << "Project: " << RESET << YELLOW << "HackyDB                                                           " << RESET << endl;
    cout << YELLOW << "🔹 " << BOLD << "Course: " << RESET << YELLOW << "CS315 - DATABASE SYSTEMS (6th Semester)                            " << RESET << endl;
    cout << YELLOW << "🔹 " << BOLD << "Institute: " << RESET << YELLOW << "Indian Institute of Technology Kanpur                           " << RESET << endl;
    cout << YELLOW << "🔹 " << BOLD << "Instructor: " << RESET << YELLOW << "Prof. Arnab Bhattacharya                                       " << RESET << endl;
    cout << "                                                                              " << endl;
    
    // Module information
    cout << CYAN << "🧩 " << BOLD << "Modules: " << RESET << CYAN << "Interpreter | API | Record Manager | Index Manager                " << RESET << endl;
    cout << CYAN << "           Catalog Manager | Buffer Manager (LRU Cache with 4KB Blocks)       " << RESET << endl;
    cout << "                                                                              " << endl;
    
    // Team information with subtle animation
    cout << MAGENTA << "👨‍💻 " << BOLD << "Crafted With Passion By Team HACKY:" << RESET << endl;
    
    string team[] = {
        "   ⚡ Harshit            ⚡ Aditya Gupta",
        "   ⚡ Chayan Kumawat     ⚡ Kushagra Singh",
        "   ⚡ Yash Pratap Singh"
    };
    
    for (const string& member : team) {
        cout << MAGENTA << member << RESET << endl;
        this_thread::sleep_for(chrono::milliseconds(150));
    }
    cout << "                                                                              " << endl;
    
    // Tips with highlighting
    cout << GREEN << "💡 " << BOLD << "Tips:" << RESET << endl;
    cout << GREEN << "   • End SQL statements with ';'" << RESET << endl;
    cout << GREEN << "   • Type 'help' to see available commands" << RESET << endl;
    cout << GREEN << "   • Type 'exit' or 'quit' to close HackyDB" << RESET << endl;
    
    cout << BLUE << BOLD << "═══════════════════════════════════════════════════════════════════════════════" << RESET << endl;
    cout << endl;
    
    // Ready prompt
    cout << YELLOW << BOLD << "[HackyDB]" << RESET << " Ready to execute your SQL commands. Type 'help' for assistance." << endl;
    cout << endl;
}

int main(int argc, const char *argv[]) {
    string sqlStatement;           
    Interpreter interpreter;      

    char *userInput;               
    size_t semicolonPosition;   

    using_history();              

    printWelcomeMessage();         

    while (true) {
        userInput = readline("HackyDB> ");

        sqlStatement = string(userInput);
        free(userInput);

        boost::algorithm::trim(sqlStatement);

        if (sqlStatement.compare(0, 4, "exit") == 0 || sqlStatement.compare(0, 4, "quit") == 0) {
            interpreter.ExecSQL("quit");
            break;
        }

        while ((semicolonPosition = sqlStatement.find(";")) == string::npos) {
            userInput = readline(""); 
            sqlStatement += "\n" + string(userInput);
            free(userInput);
        }

        if (!sqlStatement.empty()) {
            add_history(sqlStatement.c_str());
        }

        interpreter.ExecSQL(sqlStatement);
        cout << endl;
    }

    return 0;
}
