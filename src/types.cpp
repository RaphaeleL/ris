#include "types.h"
#include <sstream>

namespace ris {

// PrimitiveType implementation
std::string PrimitiveType::to_string() const {
    switch (kind_) {
        case Kind::INT: return "int";
        case Kind::FLOAT: return "float";
        case Kind::BOOL: return "bool";
        case Kind::CHAR: return "char";
        case Kind::STRING: return "string";
        case Kind::VOID: return "void";
        default: return "unknown";
    }
}

bool PrimitiveType::is_assignable_from(const Type& other) const {
    if (const auto* other_prim = dynamic_cast<const PrimitiveType*>(&other)) {
        // Allow implicit conversions: int -> float, char -> int
        if (kind_ == Kind::FLOAT && other_prim->kind_ == Kind::INT) {
            return true;
        }
        if (kind_ == Kind::INT && other_prim->kind_ == Kind::CHAR) {
            return true;
        }
        // Exact type match
        return kind_ == other_prim->kind_;
    }
    return false;
}

bool PrimitiveType::is_comparable_with(const Type& other) const {
    if (const auto* other_prim = dynamic_cast<const PrimitiveType*>(&other)) {
        // Numeric types can be compared
        if ((kind_ == Kind::INT || kind_ == Kind::FLOAT || kind_ == Kind::CHAR) &&
            (other_prim->kind_ == Kind::INT || other_prim->kind_ == Kind::FLOAT || other_prim->kind_ == Kind::CHAR)) {
            return true;
        }
        // Boolean types can be compared
        if (kind_ == Kind::BOOL && other_prim->kind_ == Kind::BOOL) {
            return true;
        }
        // String types can be compared
        if (kind_ == Kind::STRING && other_prim->kind_ == Kind::STRING) {
            return true;
        }
    }
    return false;
}

bool PrimitiveType::is_arithmetic() const {
    return kind_ == Kind::INT || kind_ == Kind::FLOAT || kind_ == Kind::CHAR;
}

bool PrimitiveType::is_boolean() const {
    return kind_ == Kind::BOOL;
}

bool PrimitiveType::is_void() const {
    return kind_ == Kind::VOID;
}

bool PrimitiveType::equals(const Type& other) const {
    if (const auto* other_prim = dynamic_cast<const PrimitiveType*>(&other)) {
        return kind_ == other_prim->kind_;
    }
    return false;
}

std::unique_ptr<PrimitiveType> PrimitiveType::create(Kind kind) {
    return std::make_unique<PrimitiveType>(kind);
}

std::unique_ptr<PrimitiveType> PrimitiveType::from_string(const std::string& type_name) {
    if (type_name == "int") return create(Kind::INT);
    if (type_name == "float") return create(Kind::FLOAT);
    if (type_name == "bool") return create(Kind::BOOL);
    if (type_name == "char") return create(Kind::CHAR);
    if (type_name == "string") return create(Kind::STRING);
    if (type_name == "void") return create(Kind::VOID);
    return nullptr;
}


// FunctionType implementation
std::string FunctionType::to_string() const {
    std::stringstream ss;
    ss << return_type_->to_string() << "(";
    for (size_t i = 0; i < parameter_types_.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << parameter_types_[i]->to_string();
    }
    ss << ")";
    return ss.str();
}

bool FunctionType::is_assignable_from(const Type& /* other */) const {
    // Functions are not assignable
    return false;
}

bool FunctionType::is_comparable_with(const Type& /* other */) const {
    // Functions are not comparable
    return false;
}

bool FunctionType::is_arithmetic() const {
    return false;
}

bool FunctionType::is_boolean() const {
    return false;
}

bool FunctionType::is_void() const {
    return false;
}

bool FunctionType::equals(const Type& other) const {
    if (const auto* other_func = dynamic_cast<const FunctionType*>(&other)) {
        if (!return_type_->equals(other_func->return_type())) {
            return false;
        }
        if (parameter_types_.size() != other_func->parameter_types().size()) {
            return false;
        }
        for (size_t i = 0; i < parameter_types_.size(); ++i) {
            if (!parameter_types_[i]->equals(*other_func->parameter_types()[i])) {
                return false;
            }
        }
        return true;
    }
    return false;
}

// Type factory functions
std::unique_ptr<Type> create_type(const std::string& type_name) {
    auto prim_type = PrimitiveType::from_string(type_name);
    if (prim_type) {
        return prim_type;
    }
    
    // Try to create a list type
    auto list_type = ListType::from_string(type_name);
    if (list_type) {
        return list_type;
    }
    
    return nullptr;
}


std::unique_ptr<Type> create_function_type(std::unique_ptr<Type> return_type, 
                                          std::vector<std::unique_ptr<Type>> parameter_types) {
    return std::make_unique<FunctionType>(std::move(return_type), std::move(parameter_types));
}

// ListType implementation
std::string ListType::to_string() const {
    return "list<" + element_type_->to_string() + ">";
}

bool ListType::is_assignable_from(const Type& other) const {
    if (const auto* other_list = dynamic_cast<const ListType*>(&other)) {
        // Lists are assignable if their element types are assignable
        return element_type_->is_assignable_from(other_list->element_type());
    }
    return false;
}

bool ListType::is_comparable_with(const Type& other) const {
    if (const auto* other_list = dynamic_cast<const ListType*>(&other)) {
        // Lists are comparable if their element types are comparable
        return element_type_->is_comparable_with(other_list->element_type());
    }
    return false;
}

bool ListType::is_arithmetic() const {
    return false;
}

bool ListType::is_boolean() const {
    return false;
}

bool ListType::is_void() const {
    return false;
}

bool ListType::equals(const Type& other) const {
    if (const auto* other_list = dynamic_cast<const ListType*>(&other)) {
        return element_type_->equals(other_list->element_type());
    }
    return false;
}

std::unique_ptr<ListType> ListType::create(std::unique_ptr<Type> element_type) {
    return std::make_unique<ListType>(std::move(element_type));
}

std::unique_ptr<ListType> ListType::from_string(const std::string& type_name) {
    // Parse "list<element_type>" format
    if (type_name.substr(0, 5) == "list<" && type_name.back() == '>') {
        std::string element_type_name = type_name.substr(5, type_name.length() - 6);
        auto element_type = create_type(element_type_name);
        if (element_type) {
            return create(std::move(element_type));
        }
    }
    return nullptr;
}

std::unique_ptr<Type> create_list_type(std::unique_ptr<Type> element_type) {
    return ListType::create(std::move(element_type));
}

} // namespace ris
