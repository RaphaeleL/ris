#include "symbol_table.h"
#include <sstream>

namespace ris {

// VariableSymbol implementation
std::string VariableSymbol::to_string() const {
    std::stringstream ss;
    ss << type_->to_string() << " " << name_;
    return ss.str();
}

// FunctionSymbol implementation
std::string FunctionSymbol::to_string() const {
    std::stringstream ss;
    ss << return_type_->to_string() << " " << name_ << "(";
    for (size_t i = 0; i < parameter_types_.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << parameter_types_[i]->to_string();
    }
    ss << ")";
    return ss.str();
}

// Scope implementation
bool Scope::add_symbol(std::unique_ptr<Symbol> symbol) {
    if (has_symbol(symbol->name())) {
        return false; // Symbol already exists
    }
    
    symbol_map_[symbol->name()] = symbols_.size();
    symbols_.push_back(std::move(symbol));
    return true;
}

Symbol* Scope::lookup_local(const std::string& name) {
    auto it = symbol_map_.find(name);
    if (it != symbol_map_.end()) {
        return symbols_[it->second].get();
    }
    return nullptr;
}

Symbol* Scope::lookup(const std::string& name) {
    // First check current scope
    Symbol* symbol = lookup_local(name);
    if (symbol) {
        return symbol;
    }
    
    // Then check parent scopes
    if (parent_) {
        return parent_->lookup(name);
    }
    
    return nullptr;
}

bool Scope::has_symbol(const std::string& name) const {
    return symbol_map_.find(name) != symbol_map_.end();
}

// SymbolTable implementation
SymbolTable::SymbolTable() {
    // Start with global scope
    enter_scope();
}

void SymbolTable::enter_scope() {
    Scope* parent = scopes_.empty() ? nullptr : scopes_.back().get();
    scopes_.push_back(std::make_unique<Scope>(parent));
}

void SymbolTable::exit_scope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

bool SymbolTable::add_symbol(std::unique_ptr<Symbol> symbol) {
    if (scopes_.empty()) {
        return false; // No scope to add to
    }
    
    return scopes_.back()->add_symbol(std::move(symbol));
}

Symbol* SymbolTable::lookup(const std::string& name) {
    if (scopes_.empty()) {
        return nullptr;
    }
    
    return scopes_.back()->lookup(name);
}

Symbol* SymbolTable::lookup_local(const std::string& name) {
    if (scopes_.empty()) {
        return nullptr;
    }
    
    return scopes_.back()->lookup_local(name);
}

bool SymbolTable::has_symbol(const std::string& name) {
    if (scopes_.empty()) {
        return false;
    }
    
    return scopes_.back()->has_symbol(name);
}

} // namespace ris
