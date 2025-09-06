#include "diagnostics.h"
#include <sstream>
#include <algorithm>

namespace ris {

void DiagnosticReporter::add_diagnostic(Severity severity, const std::string& message, 
                                       const SourcePos& position, const std::string& component) {
    diagnostics_.emplace_back(severity, message, position, component);
}

void DiagnosticReporter::add_error(const std::string& message, const SourcePos& position, 
                                  const std::string& component) {
    add_diagnostic(Severity::ERROR, message, position, component);
}

void DiagnosticReporter::add_warning(const std::string& message, const SourcePos& position, 
                                    const std::string& component) {
    add_diagnostic(Severity::WARNING, message, position, component);
}

bool DiagnosticReporter::has_errors() const {
    return std::any_of(diagnostics_.begin(), diagnostics_.end(),
                      [](const Diagnostic& diag) { return diag.severity == Severity::ERROR; });
}

std::vector<Diagnostic> DiagnosticReporter::get_errors() const {
    std::vector<Diagnostic> errors;
    std::copy_if(diagnostics_.begin(), diagnostics_.end(), std::back_inserter(errors),
                [](const Diagnostic& diag) { return diag.severity == Severity::ERROR; });
    return errors;
}

std::vector<Diagnostic> DiagnosticReporter::get_warnings() const {
    std::vector<Diagnostic> warnings;
    std::copy_if(diagnostics_.begin(), diagnostics_.end(), std::back_inserter(warnings),
                [](const Diagnostic& diag) { return diag.severity == Severity::WARNING; });
    return warnings;
}

void DiagnosticReporter::print_diagnostics() const {
    for (const auto& diag : diagnostics_) {
        std::cerr << format_diagnostic(diag) << std::endl;
    }
}

void DiagnosticReporter::print_diagnostics(const std::string& format) const {
    if (format == "gcc") {
        // GCC-style format: file:line:column: severity: message
        for (const auto& diag : diagnostics_) {
            std::cerr << "input.c:" << diag.position.line << ":" << diag.position.column 
                     << ": " << (diag.severity == Severity::ERROR ? "error" : "warning") 
                     << ": " << diag.message << std::endl;
        }
    } else {
        // Default format
        print_diagnostics();
    }
}

void DiagnosticReporter::clear() {
    diagnostics_.clear();
}

std::string DiagnosticReporter::get_error_message() const {
    if (diagnostics_.empty()) {
        return "";
    }
    
    // Return the first error message for compatibility
    for (const auto& diag : diagnostics_) {
        if (diag.severity == Severity::ERROR) {
            return format_diagnostic(diag);
        }
    }
    
    return "";
}

std::string DiagnosticReporter::format_diagnostic(const Diagnostic& diag) const {
    std::stringstream ss;
    
    // Format: [component] message at line:column
    ss << "[" << diag.component << "] " << diag.message 
       << " at " << diag.position.line << ":" << diag.position.column;
    
    return ss.str();
}

} // namespace ris
