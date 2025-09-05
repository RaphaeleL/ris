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

// ArrayType implementation
std::string ArrayType::to_string() const {
    std::stringstream ss;
    ss << element_type_->to_string();
    ss << "[";
    if (size_ == -1) {
        ss << "]";
    } else {
        ss << size_ << "]";
    }
    return ss.str();
}

bool ArrayType::is_assignable_from(const Type& other) const {
    if (const auto* other_array = dynamic_cast<const ArrayType*>(&other)) {
        return element_type_->equals(other_array->element_type()) && 
               (size_ == -1 || size_ == other_array->size());
    }
    return false;
}

bool ArrayType::is_comparable_with(const Type& other) const {
    // Arrays are not comparable
    return false;
}

bool ArrayType::is_arithmetic() const {
    return false;
}

bool ArrayType::is_boolean() const {
    return false;
}

bool ArrayType::is_void() const {
    return false;
}

bool ArrayType::equals(const Type& other) const {
    if (const auto* other_array = dynamic_cast<const ArrayType*>(&other)) {
        return element_type_->equals(other_array->element_type()) && 
               size_ == other_array->size();
    }
    return false;
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

bool FunctionType::is_assignable_from(const Type& other) const {
    // Functions are not assignable
    return false;
}

bool FunctionType::is_comparable_with(const Type& other) const {
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
    return nullptr;
}

std::unique_ptr<Type> create_array_type(std::unique_ptr<Type> element_type, int size) {
    return std::make_unique<ArrayType>(std::move(element_type), size);
}

std::unique_ptr<Type> create_function_type(std::unique_ptr<Type> return_type, 
                                          std::vector<std::unique_ptr<Type>> parameter_types) {
    return std::make_unique<FunctionType>(std::move(return_type), std::move(parameter_types));
}

} // namespace ris
