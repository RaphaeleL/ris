#pragma once

#include "types.h"
#include "ast.h"
#include <string>
#include <map>
#include <vector>
#include <memory>

namespace ris {

// Forward declarations
class Symbol;
class VariableSymbol;
class FunctionSymbol;

// Base symbol class
class Symbol {
public:
    enum class Kind {
        VARIABLE,
        FUNCTION
    };
    
    Symbol(const std::string& name, Kind kind, const SourcePos& position)
        : name_(name), kind_(kind), position_(position) {}
    
    virtual ~Symbol() = default;
    
    const std::string& name() const { return name_; }
    Kind kind() const { return kind_; }
    const SourcePos& position() const { return position_; }
    
    virtual std::string to_string() const = 0;

protected:
    std::string name_;
    Kind kind_;
    SourcePos position_;
};

// Variable symbol
class VariableSymbol : public Symbol {
public:
    VariableSymbol(const std::string& name, std::unique_ptr<Type> type, 
                   const SourcePos& position)
        : Symbol(name, Kind::VARIABLE, position), type_(std::move(type)) {}
    
    const Type& type() const { return *type_; }
    
    std::string to_string() const override;

private:
    std::unique_ptr<Type> type_;
};

// Function symbol
class FunctionSymbol : public Symbol {
public:
    FunctionSymbol(const std::string& name, std::unique_ptr<Type> return_type,
                   std::vector<std::unique_ptr<Type>> parameter_types,
                   const SourcePos& position)
        : Symbol(name, Kind::FUNCTION, position), return_type_(std::move(return_type)),
          parameter_types_(std::move(parameter_types)) {}
    
    const Type& return_type() const { return *return_type_; }
    const std::vector<std::unique_ptr<Type>>& parameter_types() const { return parameter_types_; }
    
    std::string to_string() const override;

private:
    std::unique_ptr<Type> return_type_;
    std::vector<std::unique_ptr<Type>> parameter_types_;
};

// Symbol table scope
class Scope {
public:
    Scope() = default;
    explicit Scope(Scope* parent) : parent_(parent) {}
    
    // Add a symbol to this scope
    bool add_symbol(std::unique_ptr<Symbol> symbol);
    
    // Look up a symbol in this scope only
    Symbol* lookup_local(const std::string& name);
    
    // Look up a symbol in this scope and all parent scopes
    Symbol* lookup(const std::string& name);
    
    // Get all symbols in this scope
    const std::vector<std::unique_ptr<Symbol>>& symbols() const { return symbols_; }
    
    // Get parent scope
    Scope* parent() const { return parent_; }
    
    // Check if a symbol exists in this scope
    bool has_symbol(const std::string& name) const;

private:
    std::vector<std::unique_ptr<Symbol>> symbols_;
    std::map<std::string, size_t> symbol_map_;
    Scope* parent_ = nullptr;
};

// Symbol table manager
class SymbolTable {
public:
    SymbolTable();
    
    // Enter a new scope
    void enter_scope();
    
    // Exit current scope
    void exit_scope();
    
    // Add a symbol to current scope
    bool add_symbol(std::unique_ptr<Symbol> symbol);
    
    // Look up a symbol
    Symbol* lookup(const std::string& name);
    
    // Look up a symbol in current scope only
    Symbol* lookup_local(const std::string& name);
    
    // Check if a symbol exists
    bool has_symbol(const std::string& name);
    
    // Get current scope depth
    size_t depth() const { return scopes_.size(); }
    
    // Get current scope
    Scope* current_scope() { return scopes_.empty() ? nullptr : scopes_.back().get(); }

private:
    std::vector<std::unique_ptr<Scope>> scopes_;
};

} // namespace ris
