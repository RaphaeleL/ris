#pragma once

#include "token.h"
#include <string>
#include <vector>
#include <iostream>

namespace ris {

// Severity levels for diagnostics
enum class Severity {
    ERROR,
    WARNING,
    INFO
};

// A diagnostic message with source position
struct Diagnostic {
    Severity severity;
    std::string message;
    SourcePos position;
    std::string component; // "lexer", "parser", "semantic", "codegen"
    
    Diagnostic(Severity s, const std::string& msg, const SourcePos& pos, const std::string& comp)
        : severity(s), message(msg), position(pos), component(comp) {}
};

// Centralized diagnostic reporter
class DiagnosticReporter {
public:
    // Add a diagnostic
    void add_diagnostic(Severity severity, const std::string& message, 
                       const SourcePos& position, const std::string& component);
    
    // Add an error (convenience method)
    void add_error(const std::string& message, const SourcePos& position, 
                  const std::string& component);
    
    // Add a warning (convenience method)
    void add_warning(const std::string& message, const SourcePos& position, 
                    const std::string& component);
    
    // Check if there are any errors
    bool has_errors() const;
    
    // Get all diagnostics
    const std::vector<Diagnostic>& get_diagnostics() const { return diagnostics_; }
    
    // Get all errors
    std::vector<Diagnostic> get_errors() const;
    
    // Get all warnings
    std::vector<Diagnostic> get_warnings() const;
    
    // Print all diagnostics to stderr
    void print_diagnostics() const;
    
    // Print diagnostics in a specific format
    void print_diagnostics(const std::string& format) const;
    
    // Clear all diagnostics
    void clear();
    
    // Get formatted error message for compatibility
    std::string get_error_message() const;

private:
    std::vector<Diagnostic> diagnostics_;
    
    // Format a single diagnostic
    std::string format_diagnostic(const Diagnostic& diag) const;
};

} // namespace ris
