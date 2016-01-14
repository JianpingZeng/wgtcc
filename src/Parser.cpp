#include "expr.h"
#include "parser.h"
#include "error.h"

using namespace std;

void Parser::Expect(int expect, int follow1, int follow2)
{
	auto tok = Next();
	if (tok->Tag() != expect) {
		PutBack();
		//TODO: error
		Error(tok, "'%s' expected, but got '%s'", Token::Lexeme(expect), tok->Val());
		Panic(follow1, follow2);
	}
}

TranslationUnit* Parser::ParseTranslationUnit(void)
{
	auto program = TranslationUnit::NewTranslationUnit();
    for (; ;) {
		
    }
	return program;
}

Expr* Parser::ParseExpr(void)
{
	return ParseCommaExpr();
}

Expr* Parser::ParseCommaExpr(void)
{
	auto lhs = ParseAssignExpr();
	while (Try(',')) {
		auto rhs = ParseAssignExpr();
		lhs = TranslationUnit::NewBinaryOp(',', lhs, rhs);
	}
	return lhs;
}

Expr* Parser::ParsePrimaryExpr(void)
{
    if (Peek()->IsKeyWord()) //can never be a expression
        return nullptr;

    auto tok = Next();
    if (tok->IsEOF()) return nullptr;
    if (tok->Tag() == '(') {
        auto expr = ParseExpr();
		Expect(')');
        return expr;
    }

    if (tok->IsIdentifier()) {
        //TODO: create a expression node with symbol

    } else if (tok->IsConstant()) {
        return ParseConstant(tok);
    } else if (tok->IsString()) {
        return ParseString(tok);
    } else if (tok->Tag() == Token::GENERIC) {
        return ParseGeneric();
    } 

    //TODO: error 
    return nullptr;
}

Expr* Parser::ParseConstant(const Token* tok)
{
	assert(tok->IsConstant());
	return nullptr;
}

Expr* Parser::ParseString(const Token* tok)
{
	assert(tok->IsString());
	return nullptr;
}

Expr* Parser::ParseGeneric(void)
{
	return nullptr;
}

Expr* Parser::ParsePostfixExpr(void)
{
	auto tok = Next();
	if (tok->IsEOF()) return nullptr;

	if ('(' == tok->Tag() && IsTypeName(Peek())) {
		//compound literals
		Error("compound literals not supported yet");
		//return ParseCompLiteral();
	}
	PutBack();
	auto primExpr = ParsePrimaryExpr();
	return ParsePostfixExprTail(primExpr);
}

//return the constructed postfix expression
Expr* Parser::ParsePostfixExprTail(Expr* lhs)
{
	for (; ;) {
		auto tag = Next()->Tag();
		switch (tag) {
		case '[': lhs = ParseSubScripting(lhs); break;
		case '(': lhs = ParseFuncCall(lhs); break;
		case '.': case Token::PTR_OP: lhs = ParseMemberRef(tag, lhs); break;
		case Token::INC_OP: case Token::DEC_OP: lhs = ParsePostfixIncDec(tag, lhs); break;
		default: return PutBack(), lhs;
		}
	}
}

Expr* Parser::ParseSubScripting(Expr* pointer)
{
	auto indexExpr = ParseExpr();
	Expect(']');
	return TranslationUnit::NewBinaryOp('[', pointer, indexExpr);
}


Expr* Parser::ParseMemberRef(int tag, Expr* lhs)
{
	auto memberName = Peek()->Val();
	Expect(Token::IDENTIFIER);
	return TranslationUnit::NewMemberRefOp(tag, lhs, memberName);
}

UnaryOp* Parser::ParsePostfixIncDec(int tag, Expr* operand)
{
	return TranslationUnit::NewUnaryOp(tag, operand);
}

FuncCall* Parser::ParseFuncCall(Expr* designator)
{
	//TODO: ParseArgs
	list<Expr*> args;// = ParseFuncArgs(designator);
	Expect(')');
	/*
		args.push_back(ParseAssignExpr());
	while (Try(',')) {
		args.push_back(ParseAssignExpr());
		//TODO: args type checking

	}
	*/
	
	return TranslationUnit::NewFuncCall(designator, args);
}

Expr* Parser::ParseUnaryExpr(void)
{
	auto tag = Next()->Tag();
	switch (tag) {
	case Token::ALIGNOF: return ParseAlignof();
	case Token::SIZEOF: return ParseSizeof();
	case Token::INC_OP: return ParsePrefixIncDec(Token::INC_OP);
	case Token::DEC_OP: return ParsePrefixIncDec(Token::DEC_OP);
	case '&': return ParseUnaryOp(Token::ADDR);
	case '*': return ParseUnaryOp(Token::DEREF); 
	case '+': return ParseUnaryOp(Token::PLUS);
	case '-': return ParseUnaryOp(Token::MINUS); 
	case '~': return ParseUnaryOp('~');
	case '!': return ParseUnaryOp('!');
	default: return PutBack(), ParsePostfixExpr();
	}
}

Constant* Parser::ParseSizeof(void)
{
	Type* type;
	auto tok = Next();
	if (tok->Tag() == '(' && IsTypeName(Peek())) {
		type = ParseTypeName();
		Expect(')');
	} else {
		PutBack();
		auto unaryExpr = ParseUnaryExpr();
		type = unaryExpr->Ty();
	}

	if (nullptr != type->ToFuncType()) {
		Error("sizeof operator can't act on function");
	}
	auto intType = Type::NewArithmType(ArithmType::TULONG);
	return TranslationUnit::NewConstant(intType, type->Width());
}

Constant* Parser::ParseAlignof(void)
{
	Expect('(');
	auto type = ParseTypeName();
	Expect(')');
	auto intType = Type::NewArithmType(ArithmType::TULONG);
	return TranslationUnit::NewConstant(intType, intType->Align());
}

UnaryOp* Parser::ParsePrefixIncDec(int op)
{
	assert(Token::INC_OP == op || Token::DEC_OP == op);
	auto operand = ParseUnaryExpr();
	return TranslationUnit::NewUnaryOp(op, operand);
}

UnaryOp* Parser::ParseUnaryOp(int op)
{
	auto operand = ParseCastExpr();
	return TranslationUnit::NewUnaryOp(op, operand);
}



Type* Parser::ParseTypeName(void)
{
	return nullptr;
}

Expr* Parser::ParseCastExpr(void)
{
	auto tok = Next();
	if (tok->Tag() == '(' && IsTypeName(Peek())) {
		auto desType = ParseTypeName();
		Expect(')');
		auto operand = ParseCastExpr();
		return TranslationUnit::NewUnaryOp(Token::CAST, operand, desType);
	} 
	return PutBack(), ParseUnaryExpr();
}

Expr* Parser::ParseMultiplicativeExpr(void)
{
	auto lhs = ParseCastExpr();
	auto tag = Next()->Tag();
	while ('*' == tag || '/' == tag || '%' == tag) {
		auto rhs = ParseCastExpr();
		lhs = TranslationUnit::NewBinaryOp(tag, lhs, rhs);
		tag = Next()->Tag();
	}
	return PutBack(), lhs;
}

Expr* Parser::ParseAdditiveExpr(void)
{
	auto lhs = ParseMultiplicativeExpr();
	auto tag = Next()->Tag();
	while ('+' == tag || '-' == tag) {
		auto rhs = ParseMultiplicativeExpr();
		lhs = TranslationUnit::NewBinaryOp(tag, lhs, rhs);
		tag = Next()->Tag();
	}
	return PutBack(), lhs;
}

Expr* Parser::ParseShiftExpr(void)
{
	auto lhs = ParseAdditiveExpr();
	auto tag = Next()->Tag();
	while (Token::LEFT_OP == tag || Token::RIGHT_OP == tag) {
		auto rhs = ParseAdditiveExpr();
		tag = Next()->Tag();
	}
	return PutBack(), lhs;
}

Expr* Parser::ParseRelationalExpr(void)
{
	auto lhs = ParseShiftExpr();
	auto tag = Next()->Tag();
	while (Token::LE_OP == tag || Token::GE_OP == tag 
		|| '<' == tag || '>' == tag) {
		auto rhs = ParseShiftExpr();
		lhs = TranslationUnit::NewBinaryOp(tag, lhs, rhs);
		tag = Next()->Tag();
	}
	return PutBack(), lhs;
}

Expr* Parser::ParseEqualityExpr(void)
{
	auto lhs = ParseRelationalExpr();
	auto tag = Next()->Tag();
	while (Token::EQ_OP == tag || Token::NE_OP == tag) {
		auto rhs = ParseRelationalExpr();
		lhs = TranslationUnit::NewBinaryOp(tag, lhs, rhs);
		tag = Next()->Tag();
	}
	return PutBack(), lhs;
}

Expr* Parser::ParseBitiwiseAndExpr(void)
{
	auto lhs = ParseEqualityExpr();
	while (Try('&')) {
		auto rhs = ParseEqualityExpr();
		lhs = TranslationUnit::NewBinaryOp('&', lhs, rhs);
	}
	return lhs;
}

Expr* Parser::ParseBitwiseXorExpr(void)
{
	auto lhs = ParseBitiwiseAndExpr();
	while (Try('^')) {
		auto rhs = ParseBitiwiseAndExpr();
		lhs = TranslationUnit::NewBinaryOp('^', lhs, rhs);
	}
	return lhs;
}

Expr* Parser::ParseBitwiseOrExpr(void)
{
	auto lhs = ParseBitwiseXorExpr();
	while (Try('|')) {
		auto rhs = ParseBitwiseXorExpr();
		lhs = TranslationUnit::NewBinaryOp('|', lhs, rhs);
	}
	return lhs;
}

Expr* Parser::ParseLogicalAndExpr(void)
{
	auto lhs = ParseBitwiseOrExpr();
	while (Try(Token::AND_OP)) {
		auto rhs = ParseBitwiseOrExpr();
		lhs = TranslationUnit::NewBinaryOp(Token::AND_OP, lhs, rhs);
	}
	return lhs;
}

Expr* Parser::ParseLogicalOrExpr(void)
{
	auto lhs = ParseLogicalAndExpr();
	while (Try(Token::OR_OP)) {
		auto rhs = ParseLogicalAndExpr();
		lhs = TranslationUnit::NewBinaryOp(Token::OR_OP, lhs, rhs);
	}
	return lhs;
}

Expr* Parser::ParseConditionalExpr(void)
{
	auto cond = ParseLogicalOrExpr();
	if (Try('?')) {
		auto exprTrue = ParseExpr();
		Expect(':');
		auto exprFalse = ParseConditionalExpr();
		return TranslationUnit::NewConditionalOp(cond, exprTrue, exprFalse);
	}
	return cond;
}

Expr* Parser::ParseAssignExpr(void)
{
	//yes i know the lhs should be unary expression, let it handled by type checking
	Expr* lhs = ParseConditionalExpr();
	Expr* rhs;
	switch (Next()->Tag()) {
	case Token::MUL_ASSIGN: rhs = ParseAssignExpr(); rhs = TranslationUnit::NewBinaryOp('*', lhs, rhs); goto RETURN;
	case Token::DIV_ASSIGN: rhs = ParseAssignExpr(); rhs = TranslationUnit::NewBinaryOp('/', lhs, rhs); goto RETURN;
	case Token::MOD_ASSIGN: rhs = ParseAssignExpr(); rhs = TranslationUnit::NewBinaryOp('%', lhs, rhs); goto RETURN;
	case Token::ADD_ASSIGN: rhs = ParseAssignExpr(); rhs = TranslationUnit::NewBinaryOp('+', lhs, rhs); goto RETURN;
	case Token::SUB_ASSIGN: rhs = ParseAssignExpr(); rhs = TranslationUnit::NewBinaryOp('-', lhs, rhs); goto RETURN;
	case Token::LEFT_ASSIGN: rhs = ParseAssignExpr(); rhs = TranslationUnit::NewBinaryOp(Token::LEFT_OP, lhs, rhs); goto RETURN;
	case Token::RIGHT_ASSIGN: rhs = ParseAssignExpr(); rhs = TranslationUnit::NewBinaryOp(Token::RIGHT_OP, lhs, rhs); goto RETURN;
	case Token::AND_ASSIGN: rhs = ParseAssignExpr(); rhs = TranslationUnit::NewBinaryOp('&', lhs, rhs); goto RETURN;
	case Token::XOR_ASSIGN: rhs = ParseAssignExpr(); rhs = TranslationUnit::NewBinaryOp('^', lhs, rhs); goto RETURN;
	case Token::OR_ASSIGN: rhs = ParseAssignExpr(); rhs = TranslationUnit::NewBinaryOp('|', lhs, rhs); goto RETURN;
	case '=': rhs = ParseAssignExpr(); goto RETURN;
	default: return lhs;
	}
RETURN:
	return TranslationUnit::NewBinaryOp('=', lhs, rhs);
}



/**************** Declarations ********************/

/* if there is an initializer, then return the initializer expression,
   else, return null.*/
Expr* Parser::ParseDecl(void)
{

}

enum StorageTypeQualBit {
	/*****storage-class-specifiers*****/
	S_TYPEDEF = 0x01,
	S_EXTERN = 0x02,
	S_STATIC = 0x04,
	S_THREAD_LOCAL = 0x08,
	S_AUTO = 0x10,
	S_REGISTER = 0x20,

	/*****type-specifier*****/
	T_SIGNED = 0x40,
	T_UNSIGNED = 0x80,
	T_CHAR = 0x100,
	T_SHORT = 0x200,
	T_INT = 0x400,
	T_LONG = 0x800,
	T_VOID = 0x1000,
	T_FLOAT = 0x2000,
	T_DOUBLE = 0x4000,
	T_BOOL = 0x8000,
	T_COMPLEX = 0x10000,
	T_ATOMIC = 0x20000,
	T_STRUCT_UNION = 0x40000,
	T_ENUM = 0x80000,
	T_TYPEDEF_NAME = 0x100000,

	/*****type-qualifier*****/
	Q_CONST = 0x200000,
	Q_RESTRICT = 0x400000,
	Q_VOLATILE = 0x800000,
	Q_ATOMIC = 0x1000000,

	T_LONG_LONG = 0x2000000,
};

static inline void TypeLL(int& typeSpec)
{
	if (typeSpec & T_LONG) {
		typeSpec &= ~T_LONG;
		typeSpec |= T_LONG_LONG;
	} else
		typeSpec |= T_LONG;
}

static inline void TypeDouble(int& typeSpec)
{

}


Type* Parser::ParseDeclSpec(void)
{
	Type* type = nullptr;
	int storageSpec = 0;
	int align = 0;
	int funcSpec = 0;
	int qual = 0;
	int typeSpec = 0;
	int status = 0;
	for (; ;) {
		auto tok = Next();
		switch (tok->Tag()) {
		//storage specifier
		case Token::TYPEDEF: if (storageSpec != 0) goto error; storageSpec |= S_TYPEDEF; break;
		case Token::EXTERN: if (storageSpec & ~S_THREAD_LOCAL) goto error; storageSpec |= S_EXTERN; break;
		case Token::STATIC: if (storageSpec & ~S_THREAD_LOCAL) goto error; storageSpec |= S_STATIC; break;
		case Token::THREAD_LOCAL: if (storageSpec & ~(S_EXTERN | S_STATIC)) goto error; storageSpec |= S_THREAD_LOCAL; break;
		case Token::AUTO: if (storageSpec != 0) goto error; storageSpec |= S_AUTO; break;
		case Token::REGISTER: if (storageSpec != 0) goto error; storageSpec |= S_REGISTER; break;
		
		//type qualifier
		case Token::CONST: storageSpec |= Q_CONST; break;
		case Token::RESTRICT: storageSpec |= Q_RESTRICT; break;
		case Token::VOLATILE: storageSpec |= Q_VOLATILE; break;
		atomic_qual: storageSpec |= Q_ATOMIC; break;

		//type specifier
		case Token::SIGNED: if (typeSpec & ~(T_SHORT | T_INT | T_LONG | T_LONG_LONG)) goto error; typeSpec |= T_SIGNED; break;
		case Token::UNSIGNED: if (typeSpec & ~(T_SHORT | T_INT | T_LONG | T_LONG_LONG)) goto error; typeSpec |= T_UNSIGNED; break;
		case Token::VOID: if (0 != typeSpec) goto error; typeSpec |= T_VOID; break;
		case Token::CHAR: if (typeSpec & ~(T_SIGNED | T_UNSIGNED)) goto error; typeSpec |= T_CHAR; break;
		case Token::SHORT: if (typeSpec & ~(T_SIGNED | T_UNSIGNED | T_INT)) goto error; typeSpec |= T_SHORT; break;
		case Token::INT: if (typeSpec & ~(T_SIGNED | T_UNSIGNED | T_LONG | T_SHORT | T_LONG_LONG)) goto error; typeSpec |= T_INT; break;
		case Token::LONG: if (typeSpec & ~(T_SIGNED | T_UNSIGNED | T_LONG | T_INT)) goto error; TypeLL(typeSpec); break;
		case Token::FLOAT: if (typeSpec & ~T_COMPLEX) goto error; typeSpec |= T_FLOAT; break;
		case Token::DOUBLE: if (typeSpec & ~(T_LONG | T_COMPLEX)) goto error; typeSpec |= T_DOUBLE; break;
		case Token::BOOL: if (typeSpec != 0) goto error; typeSpec |= T_BOOL; break;
		case Token::COMPLEX: if (typeSpec &~(T_FLOAT | T_DOUBLE | T_LONG)) goto error; typeSpec |= T_COMPLEX; break;
		case Token::ATOMIC: if (Peek()->Tag() != '(') goto atomic_qual; if (typeSpec != 0) goto error; type = ParseAtomicSpec();  typeSpec |= T_ATOMIC; break;
		case Token::STRUCT: if (typeSpec != 0) goto error; type = ParseStructSpec(); typeSpec |= T_STRUCT_UNION; break;
		case Token::UNION: if (typeSpec != 0) goto error; type = ParseStructSpec(); typeSpec |= T_STRUCT_UNION; break;
		case Token::ENUM: if (typeSpec != 0) goto error; type = ParseEnumSpec(); typeSpec |= T_ENUM; break;
		default: 
			if (IsTypeName(tok)) {
				type = _topEnv->FindType(tok->Val());
				typeSpec |= T_TYPEDEF_NAME;
			} else
				return PutBack(), type;
		}
	}

error:
	Error("type speficier/qualifier/storage error");
}

int Parser::ParseStorageClassSpec(int tag, int storage)
{
	int curStorage = Type::StorageOfToken(tag);
	if (Type::STORAGE::STHREAD_LOCAL & curStorage) {
		auto tmp = ~(Type::STORAGE::SSTATIC | Type::STORAGE::SEXTERN);
		if (0 == (storage & tmp))
			return storage | curStorage;
		Error("can't specific '_thread_local'");
	} else if (Type::STORAGE::SEXTERN & curStorage || Type::STORAGE::SSTATIC & curStorage) {
		auto tmp = ~Type::STORAGE::STHREAD_LOCAL;
		if (0 == (storage & tmp))
			return storage | curStorage;
		Error("can't specific 'extern'/'static'");
	} else {
		if (0 != storage)
			Error("too many sorage class specifier");
	}
	return storage;
}

int Parser::ParseQualSpec(int tag, int qual)
{
	//do no constraints checking 
	auto curQual = Type::QualOfToken(tag);
	return curQual | qual;
}

int Parser::ParseFuncSpec(int tag, int funcSpec)
{
	auto curFuncSpec = Type::FuncSpecOfToken(tag);
	return curFuncSpec | funcSpec;
}

