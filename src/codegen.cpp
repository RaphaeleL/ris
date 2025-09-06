#include "codegen.h"
#include "runtime.h"
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <iostream>
#include <fstream>

namespace ris {

CodeGenerator::CodeGenerator() 
    : has_error_(false), error_message_("") {
    // Initialize LLVM
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    
    context_ = std::make_unique<llvm::LLVMContext>();
    module_ = std::make_unique<llvm::Module>("ris_module", *context_);
    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);
}

CodeGenerator::~CodeGenerator() = default;

bool CodeGenerator::generate(std::unique_ptr<Program> program, const std::string& output_file) {
    generate_program(*program);
    
    if (has_error_) {
        return false;
    }
    
    // Verify the module
    if (llvm::verifyModule(*module_, &llvm::errs())) {
        error("Module verification failed");
        return false;
    }
    
    // Write LLVM IR to file
    std::error_code ec;
    llvm::raw_fd_ostream out(output_file, ec, llvm::sys::fs::OF_None);
    if (ec) {
        error("Failed to open output file: " + ec.message());
        return false;
    }
    
    module_->print(out, nullptr);
    return true;
}

void CodeGenerator::error(const std::string& message) {
    has_error_ = true;
    error_message_ = message;
    std::cerr << "CodeGen error: " << message << std::endl;
}

llvm::Type* CodeGenerator::get_llvm_type(const Type& type) {
    if (auto* primitive = dynamic_cast<const PrimitiveType*>(&type)) {
        switch (primitive->kind()) {
            case PrimitiveType::Kind::INT:
                return llvm::Type::getInt64Ty(*context_);
            case PrimitiveType::Kind::FLOAT:
                return llvm::Type::getDoubleTy(*context_);
            case PrimitiveType::Kind::BOOL:
                return llvm::Type::getInt8Ty(*context_);
            case PrimitiveType::Kind::CHAR:
                return llvm::Type::getInt8Ty(*context_);
            case PrimitiveType::Kind::STRING:
                return llvm::PointerType::get(*context_, 0);
            case PrimitiveType::Kind::VOID:
                return llvm::Type::getVoidTy(*context_);
            default:
                return llvm::Type::getInt64Ty(*context_);
        }
    } else if (auto* array = dynamic_cast<const ArrayType*>(&type)) {
        auto element_type = get_llvm_type(array->element_type());
        return llvm::ArrayType::get(element_type, array->size());
    }
    
    // Default to int64
    return llvm::Type::getInt64Ty(*context_);
}

llvm::Type* CodeGenerator::get_llvm_type(const std::string& type_name) {
    if (type_name == "int") {
        return llvm::Type::getInt64Ty(*context_);
    } else if (type_name == "float") {
        return llvm::Type::getDoubleTy(*context_);
    } else if (type_name == "bool") {
        return llvm::Type::getInt8Ty(*context_);
    } else if (type_name == "char") {
        return llvm::Type::getInt8Ty(*context_);
    } else if (type_name == "string") {
        return llvm::PointerType::get(*context_, 0);
    } else if (type_name == "void") {
        return llvm::Type::getVoidTy(*context_);
    }
    
    // Default to int64
    return llvm::Type::getInt64Ty(*context_);
}

void CodeGenerator::generate_program(Program& program) {
    // Declare runtime functions
    declare_runtime_functions();
    
    // Generate global variables
    for (auto& global : program.globals) {
        generate_variable_declaration(*global, true);
    }
    
    // Generate functions
    for (auto& func : program.functions) {
        generate_function(*func);
    }
    
    // Create main function if it doesn't exist
    if (functions_.find("main") == functions_.end()) {
        create_main_function();
    }
}

void CodeGenerator::generate_function(FuncDecl& func) {
    // Get parameter types
    std::vector<llvm::Type*> param_types;
    for (const auto& param : func.parameters) {
        param_types.push_back(get_llvm_type(param.first));
    }
    
    // Get return type
    llvm::Type* return_type = get_llvm_type(func.return_type);
    
    // Create function type
    llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, param_types, false);
    
    // Create function
    llvm::Function* llvm_func = llvm::Function::Create(
        func_type, 
        llvm::Function::ExternalLinkage, 
        func.name, 
        module_.get()
    );
    
    functions_[func.name] = llvm_func;
    
    // Create basic block for function body
    llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(*context_, "entry", llvm_func);
    builder_->SetInsertPoint(entry_block);
    
    // Add parameters to named values
    auto arg_it = llvm_func->arg_begin();
    for (size_t i = 0; i < func.parameters.size(); ++i) {
        if (arg_it != llvm_func->arg_end()) {
            arg_it->setName(func.parameters[i].second);
            named_values_[func.parameters[i].second] = &*arg_it;
            ++arg_it;
        }
    }
    
    // Generate function body
    if (func.body) {
        generate_block(*func.body);
    }
    
    // Add return statement if function doesn't have one and return type is void
    // Only add if the current block doesn't already have a terminator
    if (func.return_type == "void" && !builder_->GetInsertBlock()->getTerminator()) {
        builder_->CreateRetVoid();
    }
}

void CodeGenerator::generate_variable_declaration(VarDecl& var, bool is_global) {
    llvm::Type* var_type;
    
    // Handle array types
    if (var.is_array) {
        llvm::Type* element_type = get_llvm_type(var.type);
        if (var.array_size == -1) {
            // Dynamic array - not supported yet
            error("Dynamic arrays not yet implemented");
            return;
        } else {
            // Fixed-size array
            var_type = llvm::ArrayType::get(element_type, var.array_size);
        }
    } else {
        // Regular variable
        var_type = get_llvm_type(var.type);
    }
    
    llvm::Value* initial_value = nullptr;
    
    // Generate initializer if present
    if (var.initializer) {
        if (var.is_array) {
            // Array initialization not supported yet
            error("Array initialization not yet implemented");
            return;
        } else {
        initial_value = generate_expression(*var.initializer);
        }
    } else {
        // Create default initial value
        if (var.is_array) {
            // Arrays are zero-initialized by default
            initial_value = llvm::ConstantAggregateZero::get(var_type);
        } else {
        if (var.type == "int") {
            initial_value = llvm::ConstantInt::get(var_type, 0);
        } else if (var.type == "float") {
            initial_value = llvm::ConstantFP::get(var_type, 0.0);
        } else if (var.type == "bool") {
            initial_value = llvm::ConstantInt::get(var_type, 0);
        } else if (var.type == "char") {
            initial_value = llvm::ConstantInt::get(var_type, 0);
        } else if (var.type == "string") {
            initial_value = llvm::ConstantPointerNull::get(llvm::PointerType::get(*context_, 0));
            }
        }
    }
    
    if (is_global) {
        // Create global variable
        module_->getOrInsertGlobal(var.name, var_type);
        llvm::GlobalVariable* global_var = module_->getNamedGlobal(var.name);
        global_var->setLinkage(llvm::GlobalValue::InternalLinkage);
        if (initial_value && llvm::isa<llvm::Constant>(initial_value)) {
            global_var->setInitializer(static_cast<llvm::Constant*>(initial_value));
        }
        named_values_[var.name] = global_var;
    } else {
        // Create local variable
        llvm::AllocaInst* alloca = builder_->CreateAlloca(var_type, nullptr, var.name);
        if (initial_value) {
            builder_->CreateStore(initial_value, alloca);
        }
        named_values_[var.name] = alloca;
    }
}

void CodeGenerator::generate_statement(Stmt& stmt) {
    if (auto* block = dynamic_cast<BlockStmt*>(&stmt)) {
        generate_block(*block);
    } else if (auto* if_stmt = dynamic_cast<IfStmt*>(&stmt)) {
        generate_if_statement(*if_stmt);
    } else if (auto* while_stmt = dynamic_cast<WhileStmt*>(&stmt)) {
        generate_while_statement(*while_stmt);
    } else if (auto* for_stmt = dynamic_cast<ForStmt*>(&stmt)) {
        generate_for_statement(*for_stmt);
    } else if (auto* return_stmt = dynamic_cast<ReturnStmt*>(&stmt)) {
        generate_return_statement(*return_stmt);
    } else if (auto* var_decl = dynamic_cast<VarDecl*>(&stmt)) {
        generate_variable_declaration(*var_decl, false);
    } else if (auto* expr_stmt = dynamic_cast<ExprStmt*>(&stmt)) {
        if (expr_stmt->expression) {
            generate_expression(*expr_stmt->expression);
        }
    }
}

void CodeGenerator::generate_block(BlockStmt& block) {
    for (auto& stmt : block.statements) {
        generate_statement(*stmt);
    }
}

llvm::Value* CodeGenerator::generate_expression(Expr& expr) {
    if (auto* literal = dynamic_cast<LiteralExpr*>(&expr)) {
        return generate_literal_expression(*literal);
    } else if (auto* identifier = dynamic_cast<IdentifierExpr*>(&expr)) {
        return generate_identifier_expression(*identifier);
    } else if (auto* binary = dynamic_cast<BinaryExpr*>(&expr)) {
        return generate_binary_expression(*binary);
    } else if (auto* unary = dynamic_cast<UnaryExpr*>(&expr)) {
        return generate_unary_expression(*unary);
    } else if (auto* call = dynamic_cast<CallExpr*>(&expr)) {
        return generate_call_expression(*call);
    } else if (auto* array_access = dynamic_cast<ArrayIndexExpr*>(&expr)) {
        return generate_array_access_expression(*array_access);
    } else if (auto* struct_access = dynamic_cast<StructAccessExpr*>(&expr)) {
        return generate_struct_access_expression(*struct_access);
    }
    
    return nullptr;
}

llvm::Value* CodeGenerator::generate_literal_expression(LiteralExpr& expr) {
    switch (expr.type) {
        case TokenType::INTEGER_LITERAL: {
            int64_t value = std::stoll(expr.value);
            return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), value);
        }
        case TokenType::FLOAT_LITERAL: {
            double value = std::stod(expr.value);
            return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context_), value);
        }
        case TokenType::CHAR_LITERAL: {
            char value = expr.value[0];
            return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context_), value);
        }
        case TokenType::STRING_LITERAL: {
            return builder_->CreateGlobalString(expr.value);
        }
        case TokenType::TRUE:
            return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context_), 1);
        case TokenType::FALSE:
            return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context_), 0);
        default:
            return nullptr;
    }
}

llvm::Value* CodeGenerator::generate_identifier_expression(IdentifierExpr& expr) {
    auto it = named_values_.find(expr.name);
    if (it == named_values_.end()) {
        error("Undefined variable: " + expr.name);
        return nullptr;
    }
    
    llvm::Value* var = it->second;
    
    // Check if this is an array variable
    llvm::Type* var_type = nullptr;
    if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(var)) {
        var_type = alloca->getAllocatedType();
    } else if (auto* global = llvm::dyn_cast<llvm::GlobalVariable>(var)) {
        var_type = global->getValueType();
    }
    
    // If it's an array type, return the pointer directly (don't load)
    if (var_type && llvm::isa<llvm::ArrayType>(var_type)) {
        return var;
    }
    
    // For scalar variables, load the value from memory
    if (llvm::isa<llvm::AllocaInst>(var) || llvm::isa<llvm::GlobalVariable>(var)) {
        // Get the element type from the variable type
        llvm::Type* element_type = nullptr;
        if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(var)) {
            element_type = alloca->getAllocatedType();
        } else if (auto* global = llvm::dyn_cast<llvm::GlobalVariable>(var)) {
            element_type = global->getValueType();
        }
        return builder_->CreateLoad(element_type, var, expr.name);
    }
    
    // For function parameters, return the value directly
    return var;
}

llvm::Value* CodeGenerator::generate_binary_expression(BinaryExpr& expr) {
    if (!expr.left || !expr.right) {
        return nullptr;
    }
    
    llvm::Value* left = generate_expression(*expr.left);
    llvm::Value* right = generate_expression(*expr.right);
    
    if (!left || !right) {
        return nullptr;
    }
    
    switch (expr.op) {
        case TokenType::ASSIGN: {
            // Handle assignment: left must be an identifier or array access
            if (auto* identifier = dynamic_cast<IdentifierExpr*>(expr.left.get())) {
                auto it = named_values_.find(identifier->name);
                if (it == named_values_.end()) {
                    error("Undefined variable: " + identifier->name);
                    return nullptr;
                }
                
                llvm::Value* var = it->second;
                // Store the right value into the variable
                builder_->CreateStore(right, var);
                return right; // Return the assigned value
            } else if (auto* array_access = dynamic_cast<ArrayIndexExpr*>(expr.left.get())) {
                // Handle array assignment
                return generate_array_assignment(*array_access, right);
            } else {
                error("Left side of assignment must be a variable or array access");
                return nullptr;
            }
        }
        case TokenType::PLUS:
            // Check if this is string concatenation
            if (left->getType()->isPointerTy() && right->getType()->isPointerTy()) {
                // String concatenation using ris_string_concat
                auto concat_func = functions_.find("ris_string_concat");
                if (concat_func != functions_.end()) {
                    std::vector<llvm::Value*> args = {left, right};
                    return builder_->CreateCall(concat_func->second, args, "concat");
                } else {
                    error("String concatenation function not found");
                    return nullptr;
                }
            } else if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFAdd(left, right, "addtmp");
            } else {
                return builder_->CreateAdd(left, right, "addtmp");
            }
        case TokenType::MINUS:
            if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFSub(left, right, "subtmp");
            } else {
                return builder_->CreateSub(left, right, "subtmp");
            }
        case TokenType::MULTIPLY:
            if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFMul(left, right, "multmp");
            } else {
                return builder_->CreateMul(left, right, "multmp");
            }
        case TokenType::DIVIDE:
            if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFDiv(left, right, "divtmp");
            } else {
                return builder_->CreateSDiv(left, right, "divtmp");
            }
        case TokenType::EQUAL:
            if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFCmpOEQ(left, right, "eqtmp");
            } else {
                return builder_->CreateICmpEQ(left, right, "eqtmp");
            }
        case TokenType::GREATER:
            if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFCmpOGT(left, right, "gttmp");
            } else {
                return builder_->CreateICmpSGT(left, right, "gttmp");
            }
        case TokenType::LESS:
            if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFCmpOLT(left, right, "lttmp");
            } else {
                return builder_->CreateICmpSLT(left, right, "lttmp");
            }
        case TokenType::GREATER_EQUAL:
            if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFCmpOGE(left, right, "getmp");
            } else {
                return builder_->CreateICmpSGE(left, right, "getmp");
            }
        case TokenType::LESS_EQUAL:
            if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFCmpOLE(left, right, "letmp");
            } else {
                return builder_->CreateICmpSLE(left, right, "letmp");
            }
        case TokenType::NOT_EQUAL:
            if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFCmpONE(left, right, "netmp");
            } else {
                return builder_->CreateICmpNE(left, right, "netmp");
            }
        case TokenType::AND:
            return builder_->CreateAnd(left, right, "andtmp");
        case TokenType::OR:
            return builder_->CreateOr(left, right, "ortmp");
        default:
            return nullptr;
    }
}

llvm::Value* CodeGenerator::generate_unary_expression(UnaryExpr& expr) {
    if (!expr.operand) {
        return nullptr;
    }
    
    llvm::Value* operand = generate_expression(*expr.operand);
    if (!operand) {
        return nullptr;
    }
    
    switch (expr.op) {
        case TokenType::NOT:
            return builder_->CreateNot(operand, "nottmp");
        case TokenType::MINUS:
            if (operand->getType()->isFloatingPointTy()) {
                return builder_->CreateFNeg(operand, "negtmp");
            } else {
                return builder_->CreateNeg(operand, "negtmp");
            }
        default:
            return nullptr;
    }
}

llvm::Value* CodeGenerator::generate_call_expression(CallExpr& expr) {
    // Handle generic print functions specially
    if (expr.function_name == "print" || expr.function_name == "println") {
        return generate_generic_print_call(expr);
    }
    
    auto it = functions_.find(expr.function_name);
    if (it == functions_.end()) {
        error("Undefined function: " + expr.function_name);
        return nullptr;
    }
    
    llvm::Function* func = it->second;
    
    // Generate arguments
    std::vector<llvm::Value*> args;
    for (auto& arg : expr.arguments) {
        args.push_back(generate_expression(*arg));
    }
    
    return builder_->CreateCall(func, args);
}

llvm::Value* CodeGenerator::generate_generic_print_call(CallExpr& expr) {
    if (expr.arguments.empty()) {
        // Handle println() with no arguments - just print a newline
        if (expr.function_name == "println") {
            // Just print a newline by calling print with a newline string
            auto type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 4); // TYPE_STRING
            auto newline_str = builder_->CreateGlobalStringPtr("\n");
            auto it = functions_.find("print");
            if (it != functions_.end()) {
                std::vector<llvm::Value*> args = {type_tag, newline_str};
                return builder_->CreateCall(it->second, args);
            }
        }
        error("print() requires at least one argument");
        return nullptr;
    }
    
    // Get the print functions
    auto print_it = functions_.find("print");
    auto print_with_space_it = functions_.find("print_with_space");
    if (print_it == functions_.end() || print_with_space_it == functions_.end()) {
        error("Print functions not found");
        return nullptr;
    }
    llvm::Function* print_func = print_it->second;
    llvm::Function* print_with_space_func = print_with_space_it->second;
    
    // Process each argument with appropriate print function
    for (size_t i = 0; i < expr.arguments.size(); ++i) {
        auto& arg_expr = expr.arguments[i];
        auto arg_value = generate_expression(*arg_expr);
        if (!arg_value) {
            error("Failed to generate argument for print");
            return nullptr;
        }
        
        // Determine the type tag based on the argument type
        llvm::Value* type_tag = nullptr;
        llvm::Value* value_ptr = nullptr;
        
        // Get the type of the argument by checking the expression type
        if (auto* literal = dynamic_cast<LiteralExpr*>(arg_expr.get())) {
            switch (literal->type) {
                case TokenType::INTEGER_LITERAL:
                    type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 0); // TYPE_INT
                    // Allocate space for the int value
                    value_ptr = builder_->CreateAlloca(llvm::Type::getInt64Ty(*context_));
                    builder_->CreateStore(arg_value, value_ptr);
                    break;
                case TokenType::FLOAT_LITERAL:
                    type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 1); // TYPE_FLOAT
                    // Allocate space for the float value
                    value_ptr = builder_->CreateAlloca(llvm::Type::getDoubleTy(*context_));
                    builder_->CreateStore(arg_value, value_ptr);
                    break;
                case TokenType::TRUE:
                case TokenType::FALSE:
                    type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 2); // TYPE_BOOL
                    // Allocate space for the bool value
                    value_ptr = builder_->CreateAlloca(llvm::Type::getInt8Ty(*context_));
                    builder_->CreateStore(arg_value, value_ptr);
                    break;
                case TokenType::CHAR_LITERAL:
                    type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 3); // TYPE_CHAR
                    // Allocate space for the char value
                    value_ptr = builder_->CreateAlloca(llvm::Type::getInt8Ty(*context_));
                    builder_->CreateStore(arg_value, value_ptr);
                    break;
                case TokenType::STRING_LITERAL:
                    type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 4); // TYPE_STRING
                    value_ptr = arg_value; // String literals are already pointers
                    break;
                default:
                    error("Unsupported literal type for print");
                    return nullptr;
            }
        } else {
            // For non-literal expressions, determine type based on the LLVM type of the generated value
            if (arg_value->getType()->isIntegerTy(64)) {
                // int type
                type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 0); // TYPE_INT
                value_ptr = builder_->CreateAlloca(llvm::Type::getInt64Ty(*context_));
                builder_->CreateStore(arg_value, value_ptr);
            } else if (arg_value->getType()->isDoubleTy()) {
                // float type
                type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 1); // TYPE_FLOAT
                value_ptr = builder_->CreateAlloca(llvm::Type::getDoubleTy(*context_));
                builder_->CreateStore(arg_value, value_ptr);
            } else if (arg_value->getType()->isIntegerTy(8)) {
                // bool or char type - need to determine which
                if (auto* literal = dynamic_cast<LiteralExpr*>(arg_expr.get())) {
                    if (literal->type == TokenType::TRUE || literal->type == TokenType::FALSE) {
                        // bool type
                        type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 2); // TYPE_BOOL
                    } else {
                        // char type
                        type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 3); // TYPE_CHAR
                    }
                } else {
                    // For non-literal expressions, assume char if it's i8
                    type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 3); // TYPE_CHAR
                }
                value_ptr = builder_->CreateAlloca(llvm::Type::getInt8Ty(*context_));
                builder_->CreateStore(arg_value, value_ptr);
            } else if (arg_value->getType()->isPointerTy()) {
                // string type (pointer to char)
                type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 4); // TYPE_STRING
                value_ptr = arg_value; // String values are already pointers
            } else {
                error("Unsupported type for print: " + std::to_string(arg_value->getType()->getTypeID()));
                return nullptr;
            }
        }
        
        // Choose the appropriate print function
        llvm::Function* func_to_call = (i < expr.arguments.size() - 1) ? print_with_space_func : print_func;
        
        // Call the appropriate print function
        std::vector<llvm::Value*> args = {type_tag, value_ptr};
        builder_->CreateCall(func_to_call, args);
    }
    
    // If this is println, add a newline at the end
    if (expr.function_name == "println") {
        // Just print a newline using the print function
        auto print_it = functions_.find("print");
        if (print_it != functions_.end()) {
            auto type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 4); // TYPE_STRING
            auto newline_str = builder_->CreateGlobalStringPtr("\n");
            std::vector<llvm::Value*> args = {type_tag, newline_str};
            builder_->CreateCall(print_it->second, args);
        }
    }
    
    // Return void for print functions
    return llvm::Constant::getNullValue(llvm::Type::getVoidTy(*context_));
}

llvm::Value* CodeGenerator::generate_array_access_expression(ArrayIndexExpr& expr) {
    if (!expr.array || !expr.index) {
        error("Array access missing array or index");
        return nullptr;
    }
    
    // Generate array reference
    llvm::Value* array_value = generate_expression(*expr.array);
    if (!array_value) {
        error("Failed to generate array reference");
        return nullptr;
    }
    
    // Generate index expression
    llvm::Value* index_value = generate_expression(*expr.index);
    if (!index_value) {
        error("Failed to generate index expression");
        return nullptr;
    }
    
    // Convert index to i64 if needed
    if (!index_value->getType()->isIntegerTy(64)) {
        index_value = builder_->CreateIntCast(index_value, 
            llvm::Type::getInt64Ty(*context_), true, "indexcast");
    }
    
    // Get array type information
    llvm::Type* array_type = nullptr;
    if (llvm::isa<llvm::AllocaInst>(array_value)) {
        array_type = llvm::dyn_cast<llvm::AllocaInst>(array_value)->getAllocatedType();
    } else if (llvm::isa<llvm::GlobalVariable>(array_value)) {
        array_type = llvm::dyn_cast<llvm::GlobalVariable>(array_value)->getValueType();
    } else {
        error("Array access on non-array variable");
        return nullptr;
    }
    
    // Check if it's an array type
    if (!llvm::isa<llvm::ArrayType>(array_type)) {
        error("Variable is not an array");
        return nullptr;
    }
    
    auto* array_llvm_type = llvm::dyn_cast<llvm::ArrayType>(array_type);
    llvm::Type* element_type = array_llvm_type->getElementType();
    uint64_t array_size = array_llvm_type->getNumElements();
    
    // Add bounds checking
    llvm::Function* current_func = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* bounds_ok = llvm::BasicBlock::Create(*context_, "bounds_ok", current_func);
    llvm::BasicBlock* bounds_fail = llvm::BasicBlock::Create(*context_, "bounds_fail", current_func);
    
    // Check if index >= 0
    llvm::Value* index_ge_zero = builder_->CreateICmpSGE(index_value, 
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 0), "index_ge_zero");
    
    // Check if index < array_size
    llvm::Value* index_lt_size = builder_->CreateICmpSLT(index_value, 
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), array_size), "index_lt_size");
    
    // Combine both conditions
    llvm::Value* bounds_ok_cond = builder_->CreateAnd(index_ge_zero, index_lt_size, "bounds_ok");
    
    builder_->CreateCondBr(bounds_ok_cond, bounds_ok, bounds_fail);
    
    // Bounds check failed - call runtime error function
    builder_->SetInsertPoint(bounds_fail);
    // Call ris_exit with error code
    auto it = functions_.find("ris_exit");
    if (it != functions_.end()) {
        builder_->CreateCall(it->second, {llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 1)});
    } else {
        // Fallback: create function if not found
        llvm::Function* exit_func = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getVoidTy(*context_), 
                                   {llvm::Type::getInt32Ty(*context_)}, false),
            llvm::Function::ExternalLinkage, "ris_exit", module_.get());
        builder_->CreateCall(exit_func, {llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 1)});
    }
    builder_->CreateUnreachable();
    
    // Bounds check passed - generate array access
    builder_->SetInsertPoint(bounds_ok);
    
    // Generate GEP (GetElementPtr) for array access
    std::vector<llvm::Value*> indices = {
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 0), // First index (0 for array)
        index_value  // Second index (element index)
    };
    
    llvm::Value* element_ptr = builder_->CreateGEP(array_type, array_value, indices, "array_element_ptr");
    
    // Load the element value
    return builder_->CreateLoad(element_type, element_ptr, "array_element");
}

llvm::Value* CodeGenerator::generate_array_assignment(ArrayIndexExpr& expr, llvm::Value* value) {
    if (!expr.array || !expr.index) {
        error("Array assignment missing array or index");
        return nullptr;
    }
    
    // Generate array reference
    llvm::Value* array_value = generate_expression(*expr.array);
    if (!array_value) {
        error("Failed to generate array reference");
        return nullptr;
    }
    
    // Generate index expression
    llvm::Value* index_value = generate_expression(*expr.index);
    if (!index_value) {
        error("Failed to generate index expression");
        return nullptr;
    }
    
    // Convert index to i64 if needed
    if (!index_value->getType()->isIntegerTy(64)) {
        index_value = builder_->CreateIntCast(index_value, 
            llvm::Type::getInt64Ty(*context_), true, "indexcast");
    }
    
    // Get array type information
    llvm::Type* array_type = nullptr;
    if (llvm::isa<llvm::AllocaInst>(array_value)) {
        array_type = llvm::dyn_cast<llvm::AllocaInst>(array_value)->getAllocatedType();
    } else if (llvm::isa<llvm::GlobalVariable>(array_value)) {
        array_type = llvm::dyn_cast<llvm::GlobalVariable>(array_value)->getValueType();
    } else {
        error("Array assignment on non-array variable");
        return nullptr;
    }
    
    // Check if it's an array type
    if (!llvm::isa<llvm::ArrayType>(array_type)) {
        error("Variable is not an array");
    return nullptr;
    }
    
    auto* array_llvm_type = llvm::dyn_cast<llvm::ArrayType>(array_type);
    llvm::Type* element_type = array_llvm_type->getElementType();
    uint64_t array_size = array_llvm_type->getNumElements();
    
    // Add bounds checking
    llvm::Function* current_func = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* bounds_ok = llvm::BasicBlock::Create(*context_, "bounds_ok", current_func);
    llvm::BasicBlock* bounds_fail = llvm::BasicBlock::Create(*context_, "bounds_fail", current_func);
    
    // Check if index >= 0
    llvm::Value* index_ge_zero = builder_->CreateICmpSGE(index_value, 
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 0), "index_ge_zero");
    
    // Check if index < array_size
    llvm::Value* index_lt_size = builder_->CreateICmpSLT(index_value, 
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), array_size), "index_lt_size");
    
    // Combine both conditions
    llvm::Value* bounds_ok_cond = builder_->CreateAnd(index_ge_zero, index_lt_size, "bounds_ok");
    
    builder_->CreateCondBr(bounds_ok_cond, bounds_ok, bounds_fail);
    
    // Bounds check failed - call runtime error function
    builder_->SetInsertPoint(bounds_fail);
    // Call ris_exit with error code
    auto it = functions_.find("ris_exit");
    if (it != functions_.end()) {
        builder_->CreateCall(it->second, {llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 1)});
    } else {
        // Fallback: create function if not found
        llvm::Function* exit_func = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getVoidTy(*context_), 
                                   {llvm::Type::getInt32Ty(*context_)}, false),
            llvm::Function::ExternalLinkage, "ris_exit", module_.get());
        builder_->CreateCall(exit_func, {llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 1)});
    }
    builder_->CreateUnreachable();
    
    // Bounds check passed - generate array assignment
    builder_->SetInsertPoint(bounds_ok);
    
    // Generate GEP (GetElementPtr) for array access
    std::vector<llvm::Value*> indices = {
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 0), // First index (0 for array)
        index_value  // Second index (element index)
    };
    
    llvm::Value* element_ptr = builder_->CreateGEP(array_type, array_value, indices, "array_element_ptr");
    
    // Convert value to element type if needed
    if (value->getType() != element_type) {
        if (element_type->isIntegerTy() && value->getType()->isIntegerTy()) {
            value = builder_->CreateIntCast(value, element_type, true, "valuecast");
        } else if (element_type->isFloatingPointTy() && value->getType()->isFloatingPointTy()) {
            value = builder_->CreateFPCast(value, element_type, "valuecast");
        } else if (element_type->isIntegerTy() && value->getType()->isFloatingPointTy()) {
            value = builder_->CreateFPToSI(value, element_type, "valuecast");
        } else if (element_type->isFloatingPointTy() && value->getType()->isIntegerTy()) {
            value = builder_->CreateSIToFP(value, element_type, "valuecast");
        }
    }
    
    // Store the value into the array element
    builder_->CreateStore(value, element_ptr);
    return value; // Return the assigned value
}

llvm::Value* CodeGenerator::generate_struct_access_expression(StructAccessExpr& /* expr */) {
    // Simplified implementation
    error("Struct access not yet implemented");
    return nullptr;
}

void CodeGenerator::generate_if_statement(IfStmt& stmt) {
    if (!stmt.condition) {
        error("If statement missing condition");
        return;
    }
    
    // Generate condition
    llvm::Value* cond_value = generate_expression(*stmt.condition);
    if (!cond_value) {
        error("Failed to generate if condition");
        return;
    }
    
    // Convert condition to boolean if needed
    if (!cond_value->getType()->isIntegerTy(1)) {
        cond_value = builder_->CreateICmpNE(cond_value, 
            llvm::ConstantInt::get(cond_value->getType(), 0), "ifcond");
    }
    
    // Get current function and create basic blocks
    llvm::Function* func = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* then_block = llvm::BasicBlock::Create(*context_, "then", func);
    llvm::BasicBlock* else_block = stmt.else_branch ? 
        llvm::BasicBlock::Create(*context_, "else", func) : nullptr;
    llvm::BasicBlock* merge_block = nullptr;
    
    // Create conditional branch
    if (else_block) {
        // Check if both branches return (no merge block needed)
        // We need to check if the statements are return statements
        bool then_returns = false;
        bool else_returns = false;
        
        if (stmt.then_branch) {
            if (dynamic_cast<ReturnStmt*>(stmt.then_branch.get())) {
                then_returns = true;
            } else if (auto* block_stmt = dynamic_cast<BlockStmt*>(stmt.then_branch.get())) {
                // Check if block contains only a return statement
                if (block_stmt->statements.size() == 1) {
                    then_returns = dynamic_cast<ReturnStmt*>(block_stmt->statements[0].get()) != nullptr;
                }
            }
        }
        
        if (stmt.else_branch) {
            if (dynamic_cast<ReturnStmt*>(stmt.else_branch.get())) {
                else_returns = true;
            } else if (auto* block_stmt = dynamic_cast<BlockStmt*>(stmt.else_branch.get())) {
                // Check if block contains only a return statement
                if (block_stmt->statements.size() == 1) {
                    else_returns = dynamic_cast<ReturnStmt*>(block_stmt->statements[0].get()) != nullptr;
                }
            }
        }
        
        if (then_returns && else_returns) {
            // Both branches return, no merge block needed
            builder_->CreateCondBr(cond_value, then_block, else_block);
        } else {
            // Need merge block
            merge_block = llvm::BasicBlock::Create(*context_, "ifcont", func);
            builder_->CreateCondBr(cond_value, then_block, else_block);
        }
    } else {
        // For if without else, create merge block and branch to it
        merge_block = llvm::BasicBlock::Create(*context_, "ifcont", func);
        builder_->CreateCondBr(cond_value, then_block, merge_block);
    }
    
    // Generate then branch
    builder_->SetInsertPoint(then_block);
    if (stmt.then_branch) {
        generate_statement(*stmt.then_branch);
    }
    // Only add branch if the block doesn't already have a terminator and merge block exists
    if (!builder_->GetInsertBlock()->getTerminator() && merge_block) {
        builder_->CreateBr(merge_block);
    }
    
    // Generate else branch if it exists
    if (else_block) {
        builder_->SetInsertPoint(else_block);
        if (stmt.else_branch) {
            generate_statement(*stmt.else_branch);
        }
        // Only add branch if the block doesn't already have a terminator and merge block exists
        if (!builder_->GetInsertBlock()->getTerminator() && merge_block) {
            builder_->CreateBr(merge_block);
        }
    }
    
    // Set insertion point to merge block only if it exists
    if (merge_block) {
        builder_->SetInsertPoint(merge_block);
    }
}

void CodeGenerator::generate_while_statement(WhileStmt& stmt) {
    if (!stmt.condition) {
        error("While statement missing condition");
        return;
    }
    
    // Get current function and create basic blocks
    llvm::Function* func = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* cond_block = llvm::BasicBlock::Create(*context_, "while.cond", func);
    llvm::BasicBlock* body_block = llvm::BasicBlock::Create(*context_, "while.body", func);
    llvm::BasicBlock* end_block = llvm::BasicBlock::Create(*context_, "while.end", func);
    
    // Branch to condition block
    builder_->CreateBr(cond_block);
    
    // Generate condition block
    builder_->SetInsertPoint(cond_block);
    llvm::Value* cond_value = generate_expression(*stmt.condition);
    if (!cond_value) {
        error("Failed to generate while condition");
        return;
    }
    
    // Convert condition to boolean if needed
    if (!cond_value->getType()->isIntegerTy(1)) {
        cond_value = builder_->CreateICmpNE(cond_value, 
            llvm::ConstantInt::get(cond_value->getType(), 0), "whilecond");
    }
    
    // Create conditional branch
    builder_->CreateCondBr(cond_value, body_block, end_block);
    
    // Generate body block
    builder_->SetInsertPoint(body_block);
    if (stmt.body) {
        generate_statement(*stmt.body);
    }
    // Branch back to condition
    builder_->CreateBr(cond_block);
    
    // Set insertion point to end block
    builder_->SetInsertPoint(end_block);
}

void CodeGenerator::generate_for_statement(ForStmt& stmt) {
    // Get current function and create basic blocks
    llvm::Function* func = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* init_block = llvm::BasicBlock::Create(*context_, "for.init", func);
    llvm::BasicBlock* cond_block = llvm::BasicBlock::Create(*context_, "for.cond", func);
    llvm::BasicBlock* body_block = llvm::BasicBlock::Create(*context_, "for.body", func);
    llvm::BasicBlock* update_block = llvm::BasicBlock::Create(*context_, "for.update", func);
    llvm::BasicBlock* end_block = llvm::BasicBlock::Create(*context_, "for.end", func);
    
    // Generate initialization
    builder_->CreateBr(init_block);
    builder_->SetInsertPoint(init_block);
    if (stmt.init) {
        generate_statement(*stmt.init);
    }
    builder_->CreateBr(cond_block);
    
    // Generate condition block
    builder_->SetInsertPoint(cond_block);
    if (stmt.condition) {
        llvm::Value* cond_value = generate_expression(*stmt.condition);
        if (!cond_value) {
            error("Failed to generate for condition");
            return;
        }
        
        // Convert condition to boolean if needed
        if (!cond_value->getType()->isIntegerTy(1)) {
            cond_value = builder_->CreateICmpNE(cond_value, 
                llvm::ConstantInt::get(cond_value->getType(), 0), "forcond");
        }
        
        // Create conditional branch
        builder_->CreateCondBr(cond_value, body_block, end_block);
    } else {
        // No condition means infinite loop, branch to body
        builder_->CreateBr(body_block);
    }
    
    // Generate body block
    builder_->SetInsertPoint(body_block);
    if (stmt.body) {
        generate_statement(*stmt.body);
    }
    // Branch to update block
    builder_->CreateBr(update_block);
    
    // Generate update block
    builder_->SetInsertPoint(update_block);
    if (stmt.update) {
        generate_expression(*stmt.update);
    }
    // Branch back to condition
    builder_->CreateBr(cond_block);
    
    // Set insertion point to end block
    builder_->SetInsertPoint(end_block);
}

void CodeGenerator::generate_return_statement(ReturnStmt& stmt) {
    if (stmt.value) {
        llvm::Value* ret_value = generate_expression(*stmt.value);
        if (!ret_value) {
            error("Failed to generate return value");
            return;
        }
        builder_->CreateRet(ret_value);
    } else {
        builder_->CreateRetVoid();
    }
}

void CodeGenerator::create_main_function() {
    // Create main function
    llvm::FunctionType* main_type = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context_),
        std::vector<llvm::Type*>(),
        false
    );
    
    llvm::Function* main_func = llvm::Function::Create(
        main_type,
        llvm::Function::ExternalLinkage,
        "main",
        module_.get()
    );
    
    llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(*context_, "entry", main_func);
    builder_->SetInsertPoint(entry_block);
    
    // Return 0
    builder_->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 0));
    
    functions_["main"] = main_func;
}

void CodeGenerator::declare_runtime_functions() {
    // Declare print functions
    auto void_type = llvm::Type::getVoidTy(*context_);
    auto string_type = llvm::PointerType::get(*context_, 0);
    auto size_t_type = llvm::Type::getInt64Ty(*context_);
    auto int32_type = llvm::Type::getInt32Ty(*context_);
    
    // Print functions (like Python's print)
    // print(type_tag, value_ptr)
    {
        auto type_tag_type = llvm::Type::getInt32Ty(*context_);
        auto void_ptr_type = llvm::PointerType::get(*context_, 0);
        auto func_type = llvm::FunctionType::get(void_type, {type_tag_type, void_ptr_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "print", module_.get());
        functions_["print"] = func;
    }
    
    // println(type_tag, value_ptr)
    {
        auto type_tag_type = llvm::Type::getInt32Ty(*context_);
        auto void_ptr_type = llvm::PointerType::get(*context_, 0);
        auto func_type = llvm::FunctionType::get(void_type, {type_tag_type, void_ptr_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "println", module_.get());
        functions_["println"] = func;
    }
    
    // print_with_space(type_tag, value_ptr)
    {
        auto type_tag_type = llvm::Type::getInt32Ty(*context_);
        auto void_ptr_type = llvm::PointerType::get(*context_, 0);
        auto func_type = llvm::FunctionType::get(void_type, {type_tag_type, void_ptr_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "print_with_space", module_.get());
        functions_["print_with_space"] = func;
    }
    
    
    
    // ris_malloc
    {
        auto func_type = llvm::FunctionType::get(llvm::PointerType::get(*context_, 0), {size_t_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_malloc", module_.get());
        functions_["ris_malloc"] = func;
    }
    
    // ris_free
    {
        auto func_type = llvm::FunctionType::get(void_type, {llvm::PointerType::get(*context_, 0)}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_free", module_.get());
        functions_["ris_free"] = func;
    }
    
    // ris_string_concat
    {
        auto func_type = llvm::FunctionType::get(string_type, {string_type, string_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_string_concat", module_.get());
        functions_["ris_string_concat"] = func;
    }
    
    // ris_string_length
    {
        auto func_type = llvm::FunctionType::get(size_t_type, {string_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_string_length", module_.get());
        functions_["ris_string_length"] = func;
    }
    
    // ris_array_alloc
    {
        auto func_type = llvm::FunctionType::get(llvm::PointerType::get(*context_, 0), {size_t_type, size_t_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_array_alloc", module_.get());
        functions_["ris_array_alloc"] = func;
    }
    
    // ris_array_free
    {
        auto func_type = llvm::FunctionType::get(void_type, {llvm::PointerType::get(*context_, 0)}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_array_free", module_.get());
        functions_["ris_array_free"] = func;
    }
    
    // ris_exit
    {
        auto func_type = llvm::FunctionType::get(void_type, {int32_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_exit", module_.get());
        functions_["ris_exit"] = func;
    }
}

} // namespace ris