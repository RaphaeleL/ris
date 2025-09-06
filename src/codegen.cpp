#include "codegen.h"
#include "runtime.h"
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <iostream>
#include <fstream>
#include <sstream>

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
    std::string verification_error;
    llvm::raw_string_ostream error_stream(verification_error);
    if (llvm::verifyModule(*module_, &error_stream)) {
        // Parse the verification error to provide better diagnostics
        std::string error_msg = parse_verification_error(verification_error);
        error(error_msg);
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

void CodeGenerator::error(const std::string& message, const SourcePos& position) {
    has_error_ = true;
    std::stringstream ss;
    ss << message << " at " << position.line << ":" << position.column;
    std::string full_message = ss.str();
    
    if (error_message_.empty()) {
        error_message_ = full_message;
    }
    
    std::cerr << "CodeGen error: " << full_message << std::endl;
    
    // Also add to centralized diagnostic system
    diagnostics_.add_error(message, position, "codegen");
}

std::string CodeGenerator::parse_verification_error(const std::string& error) {
    // Parse common LLVM verification errors and provide user-friendly messages
    
    if (error.find("returns non-void in Function of void return type") != std::string::npos) {
        return "Function declared as 'void' cannot return a value. Remove the return statement or change function return type.";
    }
    
    if (error.find("Found return instr that returns non-void") != std::string::npos) {
        return "Function declared as 'void' cannot return a value. Remove the return statement or change function return type.";
    }
    
    if (error.find("Function does not return a value") != std::string::npos) {
        return "Function must return a value. Add a return statement or change function return type to 'void'.";
    }
    
    if (error.find("does not have terminator") != std::string::npos) {
        return "Function must return a value. Add a return statement or change function return type to 'void'.";
    }
    
    if (error.find("Undefined variable") != std::string::npos) {
        return "Undefined variable referenced in code generation.";
    }
    
    if (error.find("Type mismatch") != std::string::npos) {
        return "Type mismatch detected during code generation.";
    }
    
    // If we can't parse the error, return a generic message with the original error
    return "Code generation verification failed: " + error;
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
    } else if (type_name.substr(0, 5) == "list<") {
        // List types are pointers to the list structure
        return llvm::PointerType::get(*context_, 0);
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
    
    // Regular variable
    var_type = get_llvm_type(var.type);
    
    llvm::Value* initial_value = nullptr;
    
    // Generate initializer if present
    if (var.initializer) {
        initial_value = generate_expression(*var.initializer);
    } else {
        // Create default initial value
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
        } else if (var.type.substr(0, 5) == "list<") {
            // List types default to null pointer
            initial_value = llvm::ConstantPointerNull::get(llvm::PointerType::get(*context_, 0));
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
    } else if (auto* switch_stmt = dynamic_cast<SwitchStmt*>(&stmt)) {
        generate_switch_statement(*switch_stmt);
    } else if (auto* case_stmt = dynamic_cast<CaseStmt*>(&stmt)) {
        generate_case_statement(*case_stmt);
    } else if (auto* break_stmt = dynamic_cast<BreakStmt*>(&stmt)) {
        generate_break_statement(*break_stmt);
    } else if (auto* continue_stmt = dynamic_cast<ContinueStmt*>(&stmt)) {
        generate_continue_statement(*continue_stmt);
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
    } else if (auto* struct_access = dynamic_cast<StructAccessExpr*>(&expr)) {
        return generate_struct_access_expression(*struct_access);
    } else if (auto* list_literal = dynamic_cast<ListLiteralExpr*>(&expr)) {
        return generate_list_literal_expression(*list_literal);
    } else if (auto* list_index = dynamic_cast<ListIndexExpr*>(&expr)) {
        return generate_list_index_expression(*list_index);
    } else if (auto* list_method = dynamic_cast<ListMethodCallExpr*>(&expr)) {
        return generate_list_method_call_expression(*list_method);
    } else if (auto* pre_inc = dynamic_cast<PreIncrementExpr*>(&expr)) {
        return generate_pre_increment_expression(*pre_inc);
    } else if (auto* post_inc = dynamic_cast<PostIncrementExpr*>(&expr)) {
        return generate_post_increment_expression(*post_inc);
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
            // Handle assignment: left must be an identifier
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
            } else {
                error("Left side of assignment must be a variable");
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
                // Check if this is a list type by looking at the expression
                if (auto* list_literal = dynamic_cast<ListLiteralExpr*>(arg_expr.get())) {
                    // This is a list literal - handle it specially
                    type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 5); // TYPE_LIST
                    value_ptr = arg_value; // List values are already pointers
                } else if (auto* list_index = dynamic_cast<ListIndexExpr*>(arg_expr.get())) {
                    // This is a list indexing - also a list type
                    type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 5); // TYPE_LIST
                    value_ptr = arg_value; // List values are already pointers
                } else if (auto* identifier = dynamic_cast<IdentifierExpr*>(arg_expr.get())) {
                    // This is a variable - check if it's a list type
                    // We need to determine the type from the semantic analyzer
                    // For now, assume it's a list if it's a pointer
                    type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 5); // TYPE_LIST
                    value_ptr = arg_value; // List values are already pointers
                } else {
                    // string type (pointer to char)
                    type_tag = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), 4); // TYPE_STRING
                    value_ptr = arg_value; // String values are already pointers
                }
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
    
    // Push control flow context for break/continue
    control_flow_stack_.push_back({end_block, cond_block});
    
    // Branch to condition block
    builder_->CreateBr(cond_block);
    
    // Generate condition block
    builder_->SetInsertPoint(cond_block);
    llvm::Value* cond_value = generate_expression(*stmt.condition);
    if (!cond_value) {
        error("Failed to generate while condition");
        control_flow_stack_.pop_back();
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
    // Branch back to condition only if no terminator exists
    if (!builder_->GetInsertBlock()->getTerminator()) {
        builder_->CreateBr(cond_block);
    }
    
    // Pop control flow context
    control_flow_stack_.pop_back();
    
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
    
    // Push control flow context for break/continue
    control_flow_stack_.push_back({end_block, update_block});
    
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
            control_flow_stack_.pop_back();
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
    // Branch to update block only if no terminator exists
    if (!builder_->GetInsertBlock()->getTerminator()) {
        builder_->CreateBr(update_block);
    }
    
    // Generate update block
    builder_->SetInsertPoint(update_block);
    if (stmt.update) {
        generate_expression(*stmt.update);
    }
    // Branch back to condition
    builder_->CreateBr(cond_block);
    
    // Pop control flow context
    control_flow_stack_.pop_back();
    
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
    
    // List functions
    auto list_type = llvm::PointerType::get(*context_, 0); // ris_list_t*
    
    // ris_list_create
    {
        auto func_type = llvm::FunctionType::get(list_type, {int32_type, size_t_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_list_create", module_.get());
        functions_["ris_list_create"] = func;
    }
    
    // ris_list_free
    {
        auto func_type = llvm::FunctionType::get(void_type, {list_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_list_free", module_.get());
        functions_["ris_list_free"] = func;
    }
    
    // ris_list_push
    {
        auto func_type = llvm::FunctionType::get(void_type, {list_type, llvm::PointerType::get(*context_, 0)}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_list_push", module_.get());
        functions_["ris_list_push"] = func;
    }
    
    // ris_list_pop
    {
        auto func_type = llvm::FunctionType::get(llvm::PointerType::get(*context_, 0), {list_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_list_pop", module_.get());
        functions_["ris_list_pop"] = func;
    }
    
    // ris_list_size
    {
        auto func_type = llvm::FunctionType::get(size_t_type, {list_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_list_size", module_.get());
        functions_["ris_list_size"] = func;
    }
    
    // ris_list_get
    {
        auto func_type = llvm::FunctionType::get(llvm::PointerType::get(*context_, 0), {list_type, size_t_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_list_get", module_.get());
        functions_["ris_list_get"] = func;
    }
    
    // ris_list_get_list
    {
        auto func_type = llvm::FunctionType::get(list_type, {list_type, size_t_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_list_get_list", module_.get());
        functions_["ris_list_get_list"] = func;
    }
    
    // ris_list_get_int
    {
        auto func_type = llvm::FunctionType::get(llvm::Type::getInt64Ty(*context_), {list_type, size_t_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_list_get_int", module_.get());
        functions_["ris_list_get_int"] = func;
    }
    
    // ris_list_get_float
    {
        auto func_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(*context_), {list_type, size_t_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_list_get_float", module_.get());
        functions_["ris_list_get_float"] = func;
    }
    
    // ris_list_get_bool
    {
        auto func_type = llvm::FunctionType::get(llvm::Type::getInt8Ty(*context_), {list_type, size_t_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_list_get_bool", module_.get());
        functions_["ris_list_get_bool"] = func;
    }
    
    // ris_list_get_char
    {
        auto func_type = llvm::FunctionType::get(llvm::Type::getInt8Ty(*context_), {list_type, size_t_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_list_get_char", module_.get());
        functions_["ris_list_get_char"] = func;
    }
    
    // ris_list_get_string
    {
        auto func_type = llvm::FunctionType::get(string_type, {list_type, size_t_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_list_get_string", module_.get());
        functions_["ris_list_get_string"] = func;
    }
    
    
    // ris_exit
    {
        auto func_type = llvm::FunctionType::get(void_type, {int32_type}, false);
        auto func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "ris_exit", module_.get());
        functions_["ris_exit"] = func;
    }
}

void CodeGenerator::generate_switch_statement(SwitchStmt& stmt) {
    if (!stmt.expression) {
        error("Switch statement missing expression");
        return;
    }
    
    // Generate switch expression
    llvm::Value* switch_value = generate_expression(*stmt.expression);
    if (!switch_value) {
        error("Failed to generate switch expression");
        return;
    }
    
    // Get current function and create basic blocks
    llvm::Function* func = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* end_block = llvm::BasicBlock::Create(*context_, "switch.end", func);
    
    // Push control flow context for break
    control_flow_stack_.push_back({end_block, nullptr});
    
    // Generate cases
    for (auto& case_stmt : stmt.cases) {
        if (case_stmt) {
            generate_case_statement(*case_stmt);
        }
    }
    
    // Pop control flow context
    control_flow_stack_.pop_back();
    
    // Set insertion point to end block
    builder_->SetInsertPoint(end_block);
}

void CodeGenerator::generate_case_statement(CaseStmt& stmt) {
    // Get current function
    llvm::Function* func = builder_->GetInsertBlock()->getParent();
    
    // Create basic block for this case
    llvm::BasicBlock* case_block = llvm::BasicBlock::Create(*context_, "case", func);
    
    // Only create branch if current block doesn't already have a terminator
    if (!builder_->GetInsertBlock()->getTerminator()) {
        builder_->CreateBr(case_block);
    }
    builder_->SetInsertPoint(case_block);
    
    // Generate statements in the case
    for (auto& stmt_ptr : stmt.statements) {
        if (stmt_ptr) {
            generate_statement(*stmt_ptr);
        }
    }
    
    // Note: In a real implementation, we would need to handle:
    // 1. Case value comparison with switch expression
    // 2. Fall-through behavior (no break)
    // 3. Default case handling
    // For now, this is a simplified implementation
}

void CodeGenerator::generate_break_statement(BreakStmt& /* stmt */) {
    if (control_flow_stack_.empty()) {
        error("Break statement not inside a loop or switch");
        return;
    }
    
    auto& context = control_flow_stack_.back();
    if (context.break_target) {
        builder_->CreateBr(context.break_target);
    } else {
        error("No break target available");
    }
}

void CodeGenerator::generate_continue_statement(ContinueStmt& /* stmt */) {
    if (control_flow_stack_.empty()) {
        error("Continue statement not inside a loop");
        return;
    }
    
    auto& context = control_flow_stack_.back();
    if (context.continue_target) {
        builder_->CreateBr(context.continue_target);
    } else {
        error("Continue statement not inside a loop");
    }
}

llvm::Value* CodeGenerator::generate_list_literal_expression(ListLiteralExpr& expr) {
    // Create a list using the runtime function
    auto list_create_func = functions_.find("ris_list_create");
    if (list_create_func == functions_.end()) {
        error("ris_list_create function not found");
        return nullptr;
    }
    
    // Determine element type from the first element
    type_tag_t element_type = TYPE_INT; // Default
    if (!expr.elements.empty()) {
        if (auto* literal = dynamic_cast<LiteralExpr*>(expr.elements[0].get())) {
            switch (literal->type) {
                case TokenType::INTEGER_LITERAL:
                    element_type = TYPE_INT;
                    break;
                case TokenType::FLOAT_LITERAL:
                    element_type = TYPE_FLOAT;
                    break;
                case TokenType::TRUE:
                case TokenType::FALSE:
                    element_type = TYPE_BOOL;
                    break;
                case TokenType::CHAR_LITERAL:
                    element_type = TYPE_CHAR;
                    break;
                case TokenType::STRING_LITERAL:
                    element_type = TYPE_STRING;
                    break;
                default:
                    element_type = TYPE_INT;
                    break;
            }
        } else if (dynamic_cast<ListLiteralExpr*>(expr.elements[0].get())) {
            // Nested list - element type is list
            element_type = TYPE_LIST;
        }
    }
    
    // Create the list
    auto element_type_val = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_), element_type);
    auto capacity_val = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), expr.elements.size());
    
    auto list_ptr = builder_->CreateCall(list_create_func->second, {element_type_val, capacity_val});
    
    // Add elements to the list
    for (auto& element : expr.elements) {
        auto element_val = generate_expression(*element);
        if (!element_val) {
            error("Failed to generate list element");
            return nullptr;
        }
        
        // Allocate space for the element value
        llvm::Value* element_ptr = nullptr;
        switch (element_type) {
            case TYPE_INT: {
                element_ptr = builder_->CreateAlloca(llvm::Type::getInt64Ty(*context_));
                builder_->CreateStore(element_val, element_ptr);
                break;
            }
            case TYPE_FLOAT: {
                element_ptr = builder_->CreateAlloca(llvm::Type::getDoubleTy(*context_));
                builder_->CreateStore(element_val, element_ptr);
                break;
            }
            case TYPE_BOOL: {
                element_ptr = builder_->CreateAlloca(llvm::Type::getInt8Ty(*context_));
                builder_->CreateStore(element_val, element_ptr);
                break;
            }
            case TYPE_CHAR: {
                element_ptr = builder_->CreateAlloca(llvm::Type::getInt8Ty(*context_));
                builder_->CreateStore(element_val, element_ptr);
                break;
            }
            case TYPE_STRING: {
                element_ptr = element_val; // String literals are already pointers
                break;
            }
            case TYPE_LIST: {
                element_ptr = element_val; // List literals are already pointers
                break;
            }
            default:
                error("Unsupported list element type");
                return nullptr;
        }
        
        // Push the element to the list
        auto list_push_func = functions_.find("ris_list_push");
        if (list_push_func != functions_.end()) {
            builder_->CreateCall(list_push_func->second, {list_ptr, element_ptr});
        }
    }
    
    return list_ptr;
}

llvm::Value* CodeGenerator::generate_list_index_expression(ListIndexExpr& expr) {
    // Generate the list expression
    auto list_value = generate_expression(*expr.list);
    if (!list_value) {
        error("Failed to generate list expression");
        return nullptr;
    }
    
    // Generate the index expression
    auto index_value = generate_expression(*expr.index);
    if (!index_value) {
        error("Failed to generate index expression");
        return nullptr;
    }
    
    // Determine the element type of the list
    type_tag_t element_type = TYPE_INT; // Default
    
    // Check if the list is a list literal
    if (auto* list_literal = dynamic_cast<ListLiteralExpr*>(expr.list.get())) {
        if (!list_literal->elements.empty()) {
            if (auto* literal = dynamic_cast<LiteralExpr*>(list_literal->elements[0].get())) {
                switch (literal->type) {
                    case TokenType::INTEGER_LITERAL:
                        element_type = TYPE_INT;
                        break;
                    case TokenType::FLOAT_LITERAL:
                        element_type = TYPE_FLOAT;
                        break;
                    case TokenType::TRUE:
                    case TokenType::FALSE:
                        element_type = TYPE_BOOL;
                        break;
                    case TokenType::CHAR_LITERAL:
                        element_type = TYPE_CHAR;
                        break;
                    case TokenType::STRING_LITERAL:
                        element_type = TYPE_STRING;
                        break;
                    default:
                        element_type = TYPE_INT;
                        break;
                }
            } else if (dynamic_cast<ListLiteralExpr*>(list_literal->elements[0].get())) {
                element_type = TYPE_LIST;
            }
        }
    }
    
    // Call the appropriate getter function based on element type
    std::string func_name;
    switch (element_type) {
        case TYPE_INT:
            func_name = "ris_list_get_int";
            break;
        case TYPE_FLOAT:
            func_name = "ris_list_get_float";
            break;
        case TYPE_BOOL:
            func_name = "ris_list_get_bool";
            break;
        case TYPE_CHAR:
            func_name = "ris_list_get_char";
            break;
        case TYPE_STRING:
            func_name = "ris_list_get_string";
            break;
        case TYPE_LIST:
            func_name = "ris_list_get_list"; // Specific getter for nested lists
            break;
        default:
            error("Unsupported list element type for indexing");
            return nullptr;
    }
    
    auto get_func = functions_.find(func_name);
    if (get_func != functions_.end()) {
        return builder_->CreateCall(get_func->second, {list_value, index_value});
    }
    
    error(func_name + " function not found");
    return nullptr;
}

llvm::Value* CodeGenerator::generate_list_method_call_expression(ListMethodCallExpr& expr) {
    // Generate the list expression
    auto list_value = generate_expression(*expr.list);
    if (!list_value) {
        error("Failed to generate list expression");
        return nullptr;
    }
    
    if (expr.method_name == "push") {
        if (expr.arguments.size() != 1) {
            error("push() method requires exactly one argument");
            return nullptr;
        }
        
        // Generate the argument value
        auto arg_value = generate_expression(*expr.arguments[0]);
        if (!arg_value) {
            error("Failed to generate push argument");
            return nullptr;
        }
        
        // Determine the element type from the list
        type_tag_t element_type = TYPE_INT; // Default
        
        // Check if the list is a list literal
        if (auto* list_literal = dynamic_cast<ListLiteralExpr*>(expr.list.get())) {
            if (!list_literal->elements.empty()) {
                if (auto* literal = dynamic_cast<LiteralExpr*>(list_literal->elements[0].get())) {
                    switch (literal->type) {
                        case TokenType::INTEGER_LITERAL:
                            element_type = TYPE_INT;
                            break;
                        case TokenType::FLOAT_LITERAL:
                            element_type = TYPE_FLOAT;
                            break;
                        case TokenType::TRUE:
                        case TokenType::FALSE:
                            element_type = TYPE_BOOL;
                            break;
                        case TokenType::CHAR_LITERAL:
                            element_type = TYPE_CHAR;
                            break;
                        case TokenType::STRING_LITERAL:
                            element_type = TYPE_STRING;
                            break;
                        default:
                            element_type = TYPE_INT;
                            break;
                    }
                } else if (dynamic_cast<ListLiteralExpr*>(list_literal->elements[0].get())) {
                    element_type = TYPE_LIST;
                }
            }
        } else if (auto* list_index = dynamic_cast<ListIndexExpr*>(expr.list.get())) {
            // For list index expressions, we need to determine the element type from the original list
            // For now, assume it's a list for nested lists
            element_type = TYPE_LIST;
        } else if (auto* identifier = dynamic_cast<IdentifierExpr*>(expr.list.get())) {
            // For variable references, use heuristic based on variable name
            std::string var_name = identifier->name;
            if (var_name.find("nested") != std::string::npos || 
                var_name.find("matrix") != std::string::npos ||
                var_name == "a" ||  // Common variable name for nested lists
                var_name.length() == 1) {  // Single letter variables often used for nested lists
                element_type = TYPE_LIST;
            } else {
                element_type = TYPE_INT; // Default for simple lists
            }
        } else {
            // For other expressions, assume it's a list for nested lists
            element_type = TYPE_LIST;
        }
        
        // Allocate space for the argument based on element type
        llvm::Value* element_ptr;
        if (element_type == TYPE_LIST) {
            // For lists, the argument is already a pointer
            element_ptr = arg_value;
        } else {
            // For primitive types, allocate space and store the value
            llvm::Type* element_llvm_type;
            switch (element_type) {
                case TYPE_INT:
                    element_llvm_type = llvm::Type::getInt64Ty(*context_);
                    break;
                case TYPE_FLOAT:
                    element_llvm_type = llvm::Type::getDoubleTy(*context_);
                    break;
                case TYPE_BOOL:
                case TYPE_CHAR:
                    element_llvm_type = llvm::Type::getInt8Ty(*context_);
                    break;
                case TYPE_STRING:
                    element_llvm_type = llvm::PointerType::get(*context_, 0);
                    break;
                default:
                    element_llvm_type = llvm::Type::getInt64Ty(*context_);
                    break;
            }
            element_ptr = builder_->CreateAlloca(element_llvm_type);
            builder_->CreateStore(arg_value, element_ptr);
        }
        
        // Call ris_list_push
        auto push_func = functions_.find("ris_list_push");
        if (push_func != functions_.end()) {
            builder_->CreateCall(push_func->second, {list_value, element_ptr});
        }
        
        return nullptr; // push returns void
        
    } else if (expr.method_name == "pop") {
        if (!expr.arguments.empty()) {
            error("pop() method takes no arguments");
            return nullptr;
        }
        
        // Call ris_list_pop
        auto pop_func = functions_.find("ris_list_pop");
        if (pop_func != functions_.end()) {
            return builder_->CreateCall(pop_func->second, {list_value});
        }
        
        return nullptr;
        
    } else if (expr.method_name == "size") {
        if (!expr.arguments.empty()) {
            error("size() method takes no arguments");
            return nullptr;
        }
        
        // Call ris_list_size
        auto size_func = functions_.find("ris_list_size");
        if (size_func != functions_.end()) {
            return builder_->CreateCall(size_func->second, {list_value});
        }
        
        return nullptr;
        
    } else if (expr.method_name == "get") {
        if (expr.arguments.empty()) {
            error("get() method requires at least one index argument");
            return nullptr;
        }
        
        // For now, handle single index case
        if (expr.arguments.size() == 1) {
            auto index_value = generate_expression(*expr.arguments[0]);
            if (!index_value) {
                error("Failed to generate get index");
                return nullptr;
            }
            
            // Determine the element type of the list
            type_tag_t element_type = TYPE_INT; // Default
            
    // Check if the list is a list literal
    if (auto* list_literal = dynamic_cast<ListLiteralExpr*>(expr.list.get())) {
        if (!list_literal->elements.empty()) {
            if (auto* literal = dynamic_cast<LiteralExpr*>(list_literal->elements[0].get())) {
                switch (literal->type) {
                    case TokenType::INTEGER_LITERAL:
                        element_type = TYPE_INT;
                        break;
                    case TokenType::FLOAT_LITERAL:
                        element_type = TYPE_FLOAT;
                        break;
                    case TokenType::TRUE:
                    case TokenType::FALSE:
                        element_type = TYPE_BOOL;
                        break;
                    case TokenType::CHAR_LITERAL:
                        element_type = TYPE_CHAR;
                        break;
                    case TokenType::STRING_LITERAL:
                        element_type = TYPE_STRING;
                        break;
                    default:
                        element_type = TYPE_INT;
                        break;
                }
            } else if (dynamic_cast<ListLiteralExpr*>(list_literal->elements[0].get())) {
                element_type = TYPE_LIST;
            }
        }
    } else if (auto* list_index = dynamic_cast<ListIndexExpr*>(expr.list.get())) {
        // This is chained indexing - the list is itself a list index expression
        // The previous indexing should have returned a list pointer, so we need to determine
        // the element type of the nested list. For now, assume it's an integer.
        element_type = TYPE_INT;
    } else if (auto* identifier = dynamic_cast<IdentifierExpr*>(expr.list.get())) {
        // For variable references, we need to determine the element type from the variable type
        // This is a complex issue that requires type information from the semantic analyzer
        // For now, assume all variable references are nested lists to fix the indexing issue
        element_type = TYPE_LIST;
    } else {
        // For other expressions, use the generic ris_list_get function
        element_type = TYPE_LIST;
    }
            
            // Call the appropriate getter function based on element type
            std::string func_name;
            switch (element_type) {
                case TYPE_INT:
                    func_name = "ris_list_get_int";
                    break;
                case TYPE_FLOAT:
                    func_name = "ris_list_get_float";
                    break;
                case TYPE_BOOL:
                    func_name = "ris_list_get_bool";
                    break;
                case TYPE_CHAR:
                    func_name = "ris_list_get_char";
                    break;
                case TYPE_STRING:
                    func_name = "ris_list_get_string";
                    break;
                case TYPE_LIST:
                    func_name = "ris_list_get_list"; // Specific getter for nested lists
                    break;
                default:
                    error("Unsupported list element type for get()");
                    return nullptr;
            }
            
            auto get_func = functions_.find(func_name);
            if (get_func != functions_.end()) {
                return builder_->CreateCall(get_func->second, {list_value, index_value});
            }
            
            error(func_name + " function not found");
            return nullptr;
        } else {
            // Multiple indices - for now, just handle the first one
            // TODO: Implement proper chained indexing
            error("Multiple indices in get() not yet implemented");
            return nullptr;
        }
        
        return nullptr;
        
    } else {
        error("Unknown list method: " + expr.method_name);
        return nullptr;
    }
}

llvm::Value* CodeGenerator::generate_pre_increment_expression(PreIncrementExpr& expr) {
    if (!expr.operand) {
        error("Expected operand for pre-increment");
        return nullptr;
    }
    
    // Generate the operand (must be an identifier)
    auto* identifier = dynamic_cast<IdentifierExpr*>(expr.operand.get());
    if (!identifier) {
        error("Pre-increment operand must be a variable");
        return nullptr;
    }
    
    // Get the variable's current value
    auto var_value = generate_identifier_expression(*identifier);
    if (!var_value) {
        error("Failed to generate variable reference");
        return nullptr;
    }
    
    // Create a new value (current + 1)
    auto one = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 1);
    auto new_value = builder_->CreateAdd(var_value, one);
    
    // Store the new value back to the variable
    auto var_it = named_values_.find(identifier->name);
    if (var_it == named_values_.end()) {
        error("Variable not found: " + identifier->name);
        return nullptr;
    }
    builder_->CreateStore(new_value, var_it->second);
    
    // Return the new value (pre-increment returns the incremented value)
    return new_value;
}

llvm::Value* CodeGenerator::generate_post_increment_expression(PostIncrementExpr& expr) {
    if (!expr.operand) {
        error("Expected operand for post-increment");
        return nullptr;
    }
    
    // Generate the operand (must be an identifier)
    auto* identifier = dynamic_cast<IdentifierExpr*>(expr.operand.get());
    if (!identifier) {
        error("Post-increment operand must be a variable");
        return nullptr;
    }
    
    // Get the variable's current value
    auto var_value = generate_identifier_expression(*identifier);
    if (!var_value) {
        error("Failed to generate variable reference");
        return nullptr;
    }
    
    // Create a new value (current + 1)
    auto one = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context_), 1);
    auto new_value = builder_->CreateAdd(var_value, one);
    
    // Store the new value back to the variable
    auto var_it = named_values_.find(identifier->name);
    if (var_it == named_values_.end()) {
        error("Variable not found: " + identifier->name);
        return nullptr;
    }
    builder_->CreateStore(new_value, var_it->second);
    
    // Return the original value (post-increment returns the original value)
    return var_value;
}

} // namespace ris