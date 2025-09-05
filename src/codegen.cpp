#include "codegen.h"
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
                return llvm::Type::getInt1Ty(*context_);
            case PrimitiveType::Kind::CHAR:
                return llvm::Type::getInt8Ty(*context_);
            case PrimitiveType::Kind::STRING:
                return llvm::PointerType::get(llvm::Type::getInt8Ty(*context_), 0);
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
        return llvm::Type::getInt1Ty(*context_);
    } else if (type_name == "char") {
        return llvm::Type::getInt8Ty(*context_);
    } else if (type_name == "string") {
        return llvm::PointerType::get(llvm::Type::getInt8Ty(*context_), 0);
    } else if (type_name == "void") {
        return llvm::Type::getVoidTy(*context_);
    }
    
    // Default to int64
    return llvm::Type::getInt64Ty(*context_);
}

void CodeGenerator::generate_program(Program& program) {
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
    llvm::Type* var_type = get_llvm_type(var.type);
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
            initial_value = llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(*context_), 0));
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
            return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), 1);
        case TokenType::FALSE:
            return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context_), 0);
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
    
    // For simplicity, just return the variable for now
    // In a full implementation, we'd need to handle loading from allocas/globals
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
        case TokenType::PLUS:
            if (left->getType()->isFloatingPointTy()) {
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

llvm::Value* CodeGenerator::generate_array_access_expression(ArrayIndexExpr& expr) {
    // Simplified implementation
    error("Array access not yet implemented");
    return nullptr;
}

llvm::Value* CodeGenerator::generate_struct_access_expression(StructAccessExpr& expr) {
    // Simplified implementation
    error("Struct access not yet implemented");
    return nullptr;
}

void CodeGenerator::generate_if_statement(IfStmt& stmt) {
    // Simplified implementation
    error("If statements not yet implemented");
}

void CodeGenerator::generate_while_statement(WhileStmt& stmt) {
    // Simplified implementation
    error("While statements not yet implemented");
}

void CodeGenerator::generate_for_statement(ForStmt& stmt) {
    // Simplified implementation
    error("For statements not yet implemented");
}

void CodeGenerator::generate_return_statement(ReturnStmt& stmt) {
    if (stmt.value) {
        llvm::Value* ret_value = generate_expression(*stmt.value);
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

} // namespace ris