#pragma once

#include "token.h"
#include <string>
#include <vector>
#include <memory>

namespace ris {

// Forward declarations
class Type;
class PrimitiveType;
class ArrayType;
class FunctionType;

// Base type class
class Type {
public:
    virtual ~Type() = default;
    virtual std::string to_string() const = 0;
    virtual bool is_assignable_from(const Type& other) const = 0;
    virtual bool is_comparable_with(const Type& other) const = 0;
    virtual bool is_arithmetic() const = 0;
    virtual bool is_boolean() const = 0;
    virtual bool is_void() const = 0;
    virtual bool equals(const Type& other) const = 0;
};

// Primitive types (int, float, bool, char, string, void)
class PrimitiveType : public Type {
public:
    enum class Kind {
        INT,
        FLOAT,
        BOOL,
        CHAR,
        STRING,
        VOID
    };
    
    explicit PrimitiveType(Kind kind) : kind_(kind) {}
    
    Kind kind() const { return kind_; }
    
    std::string to_string() const override;
    bool is_assignable_from(const Type& other) const override;
    bool is_comparable_with(const Type& other) const override;
    bool is_arithmetic() const override;
    bool is_boolean() const override;
    bool is_void() const override;
    bool equals(const Type& other) const override;
    
    static std::unique_ptr<PrimitiveType> create(Kind kind);
    static std::unique_ptr<PrimitiveType> from_string(const std::string& type_name);

private:
    Kind kind_;
};

// Array types
class ArrayType : public Type {
public:
    ArrayType(std::unique_ptr<Type> element_type, int size = -1) 
        : element_type_(std::move(element_type)), size_(size) {}
    
    const Type& element_type() const { return *element_type_; }
    int size() const { return size_; }
    bool is_dynamic() const { return size_ == -1; }
    
    std::string to_string() const override;
    bool is_assignable_from(const Type& other) const override;
    bool is_comparable_with(const Type& other) const override;
    bool is_arithmetic() const override;
    bool is_boolean() const override;
    bool is_void() const override;
    bool equals(const Type& other) const override;

private:
    std::unique_ptr<Type> element_type_;
    int size_; // -1 for dynamic arrays
};

// Function types
class FunctionType : public Type {
public:
    FunctionType(std::unique_ptr<Type> return_type, 
                 std::vector<std::unique_ptr<Type>> parameter_types)
        : return_type_(std::move(return_type)), 
          parameter_types_(std::move(parameter_types)) {}
    
    const Type& return_type() const { return *return_type_; }
    const std::vector<std::unique_ptr<Type>>& parameter_types() const { return parameter_types_; }
    
    std::string to_string() const override;
    bool is_assignable_from(const Type& other) const override;
    bool is_comparable_with(const Type& other) const override;
    bool is_arithmetic() const override;
    bool is_boolean() const override;
    bool is_void() const override;
    bool equals(const Type& other) const override;

private:
    std::unique_ptr<Type> return_type_;
    std::vector<std::unique_ptr<Type>> parameter_types_;
};

// Type factory functions
std::unique_ptr<Type> create_type(const std::string& type_name);
std::unique_ptr<Type> create_array_type(std::unique_ptr<Type> element_type, int size = -1);
std::unique_ptr<Type> create_function_type(std::unique_ptr<Type> return_type, 
                                          std::vector<std::unique_ptr<Type>> parameter_types);

} // namespace ris
