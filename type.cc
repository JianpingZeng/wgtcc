#include "type.h"

#include "ast.h"
#include "scope.h"
#include "token.h"

#include <cassert>
#include <algorithm>
#include <iostream>


/***************** Type *********************/

MemPoolImp<VoidType>         Type::_voidTypePool;
MemPoolImp<ArrayType>        Type::_arrayTypePool;
MemPoolImp<FuncType>         Type::_funcTypePool;
MemPoolImp<PointerType>      Type::_pointerTypePool;
MemPoolImp<StructUnionType>  Type::_structUnionTypePool;
MemPoolImp<ArithmType>       Type::_arithmTypePool;

bool Type::IsFloat(void) const {
    auto arithmType = ToArithmType();
    return arithmType && arithmType->IsFloat();
}

bool Type::IsInteger(void) const {
    auto arithmType = ToArithmType();
    return arithmType && arithmType->IsInteger();
}

VoidType* Type::NewVoidType(void)
{
    return new (_voidTypePool.Alloc()) VoidType(&_voidTypePool);
}

/*
ArithmType* Type::NewArithmType(int typeSpec) {
    static ArithmType* charType 		= new ArithmType(T_CHAR);
    static ArithmType* ucharType 		= new ArithmType(T_UNSIGNED | T_CHAR);
    static ArithmType* shortType 		= new ArithmType(T_SHORT);
    static ArithmType* ushortType 		= new ArithmType(T_UNSIGNED | T_SHORT);
    static ArithmType* intType 			= new ArithmType(T_INT);
    static ArithmType* uintType 		= new ArithmType(T_UNSIGNED | T_INT);
    static ArithmType* longType 		= new ArithmType(T_LONG);
    static ArithmType* ulongType 		= new ArithmType(T_UNSIGNED | T_LONG);
    static ArithmType* longlongType 	= new ArithmType(T_LONG_LONG);
    static ArithmType* ulonglongType 	= new ArithmType(T_UNSIGNED | T_LONG_LONG);
    static ArithmType* floatType 		= new ArithmType(T_FLOAT);
    static ArithmType* doubleType 		= new ArithmType(T_DOUBLE);
    static ArithmType* longdoubleType 	= new ArithmType(T_LONG | T_DOUBLE);
    static ArithmType* boolType 		= new ArithmType(T_BOOL);
    static ArithmType* complexType 		= new ArithmType(T_FLOAT | T_COMPLEX);
    static ArithmType* doublecomplexType = new ArithmType(T_DOUBLE | T_COMPLEX);
    static ArithmType* longdoublecomplexType = new ArithmType(T_LONG | T_DOUBLE | T_COMPLEX);

    switch (typeSpec) {
    case T_CHAR:
    case T_SIGNED | T_CHAR:
        return charType;

    case T_UNSIGNED | T_CHAR:
        return ucharType;

    case T_SHORT:
    case T_SIGNED | T_SHORT:
    case T_SHORT | T_INT:
    case T_SIGNED | T_SHORT | T_INT:
        return shortType;

    case T_UNSIGNED | T_SHORT:
    case T_UNSIGNED | T_SHORT | T_INT:
        return ushortType;

    case T_INT:
    case T_SIGNED:
    case T_SIGNED | T_INT:
        return intType;

    case T_UNSIGNED:
    case T_UNSIGNED | T_INT:
        return uintType;

    case T_LONG:
    case T_SIGNED | T_LONG:
    case T_LONG | T_INT:
    case T_SIGNED | T_LONG | T_INT:
        return longType;

    case T_UNSIGNED | T_LONG:
    case T_UNSIGNED | T_LONG | T_INT:
        return ulongType;

    case T_LONG_LONG:
    case T_SIGNED | T_LONG_LONG:
    case T_LONG_LONG | T_INT:
    case T_SIGNED | T_LONG_LONG | T_INT:
        return longlongType;

    case T_UNSIGNED | T_LONG_LONG:
    case T_UNSIGNED | T_LONG_LONG | T_INT:
        return ulonglongType;

    case T_FLOAT:
        return floatType;

    case T_DOUBLE:
        return doubleType;

    case T_LONG | T_DOUBLE:
        return longdoubleType;

    case T_BOOL:
        return boolType;

    case T_FLOAT | T_COMPLEX:
        return complexType;

    case T_DOUBLE | T_COMPLEX:
        return doublecomplexType;

    case T_LONG | T_DOUBLE | T_COMPLEX:
        return longdoublecomplexType;

    default:
        assert(0);
    }

    return nullptr; // make compiler happy
}
*/

ArithmType* Type::NewArithmType(int typeSpec) {
    auto tag = ArithmType::Spec2Tag(typeSpec);
    return new (_arithmTypePool.Alloc())
            ArithmType(&_arithmTypePool, tag);
}

ArrayType* Type::NewArrayType(long long len, Type* eleType)
{
    return new (_arrayTypePool.Alloc())
            ArrayType(&_arrayTypePool, len, eleType);
}

//static IntType* NewIntType();
FuncType* Type::NewFuncType(Type* derived, int funcSpec,
        bool hasEllipsis, const std::list<Type*>& params) {
    return new (_funcTypePool.Alloc())
            FuncType(&_funcTypePool, derived, funcSpec, hasEllipsis, params);
}

PointerType* Type::NewPointerType(Type* derived) {
    return new (_pointerTypePool.Alloc())
            PointerType(&_pointerTypePool, derived);
}

StructUnionType* Type::NewStructUnionType(
        bool isStruct, bool hasTag, Scope* parent) {
    return new (_structUnionTypePool.Alloc())
            StructUnionType(&_structUnionTypePool, isStruct, hasTag, parent);
}

/*
static EnumType* Type::NewEnumType() {
    // TODO(wgtdkp):
    assert(false);
    return nullptr;
}
*/

/*************** ArithmType *********************/
int ArithmType::CalcWidth(int tag) {
    switch (tag) {
    case T_BOOL:
    case T_CHAR:
    case T_UNSIGNED | T_CHAR:
        return 1;

    case T_SHORT:
    case T_UNSIGNED | T_SHORT:
        return _intWidth >> 1;

    case T_INT:
    case T_UNSIGNED:
        return _intWidth;

    case T_LONG:
    case T_UNSIGNED | T_LONG:
        return _intWidth << 1;

    case T_LONG_LONG:
    case T_UNSIGNED | T_LONG_LONG:
        return _intWidth << 1;

    case T_FLOAT:
        return _intWidth;

    case T_DOUBLE:
        return _intWidth << 1;

    case T_LONG | T_DOUBLE:
        return _intWidth << 2;

    case T_FLOAT | T_COMPLEX:
        return _intWidth << 1;

    case T_DOUBLE | T_COMPLEX:
        return _intWidth << 2;

    case T_LONG | T_DOUBLE | T_COMPLEX:
        return _intWidth << 3;

    default:
        assert(false);
    }

    return _intWidth; // Make compiler happy
}

int ArithmType::Spec2Tag(int spec) {
    spec &= ~T_SIGNED;
    if ((spec & T_SHORT) || (spec & T_LONG)
            || (spec & T_LONG_LONG) || (spec & T_UNSIGNED)) {
        spec &= ~T_INT;
    }
    return spec;
}

std::string ArithmType::Str(void) const
{
    std::string width = std::string(":") + std::to_string(_width);

    switch (_tag) {
    case T_BOOL:
        return "bool" + width;

    case T_CHAR:
        return "char" + width;

    case T_UNSIGNED | T_CHAR:
        return "unsigned char" + width;

    case T_SHORT:
        return "short" + width;

    case T_UNSIGNED | T_SHORT:
        return "unsigned short" + width;

    case T_INT:
        return "int" + width;

    case T_UNSIGNED:
        return "unsigned int" + width;

    case T_LONG:
        return "long" + width;

    case T_UNSIGNED | T_LONG:
        return "unsigned long" + width;

    case T_LONG_LONG:
        return "long long" + width;

    case T_UNSIGNED | T_LONG_LONG:
        return "unsigned long long" + width;

    case T_FLOAT:
        return "float" + width;

    case T_DOUBLE:
        return "double" + width;

    case T_LONG | T_DOUBLE:
        return "long double" + width;

    case T_FLOAT | T_COMPLEX:
        return "float complex" + width;

    case T_DOUBLE | T_COMPLEX:
        return "double complex" + width;

    case T_LONG | T_DOUBLE | T_COMPLEX:
        return "long double complex" + width;

    default:
        assert(false);
    }

    return "error"; // Make compiler happy
}

/*************** PointerType *****************/

bool PointerType::operator==(const Type& other) const {
    auto otherType = other.ToPointerType();
    if (nullptr == otherType)
        return false;

    return *_derived == *otherType->_derived;
}

bool PointerType::Compatible(const Type& other) const {
    //TODO: compatibility ???
    if (other.IsInteger())
        return true;
    auto otherType = other.ToPointerType();
    if (otherType == nullptr)
        return false;
    if ((otherType->Derived()->ToVoidType() && !_derived->ToFuncType())
            || (_derived->ToVoidType() && !otherType->Derived()->ToFuncType()))
        return true;
    return _derived->Compatible(*otherType->Derived());
}


/************* FuncType ***************/

bool FuncType::operator==(const Type& other) const
{
    auto otherFunc = other.ToFuncType();
    if (nullptr == otherFunc) return false;
    // TODO: do we need to check the type of return value when deciding 
    // equality of two function types ??
    if (*_derived != *otherFunc->_derived)
        return false;
    if (_params.size() != otherFunc->_params.size())
        return false;

    auto thisIter = _params.begin();
    auto otherIter = otherFunc->_params.begin();
    while (thisIter != _params.end()) {
        if (*(*thisIter) != *(*otherIter))
            return false;

        thisIter++;
        otherIter++;
    }

    return true;
}

bool FuncType::Compatible(const Type& other) const
{
    auto otherFunc = other.ToFuncType();
    //the other type is not an function type
    if (nullptr == otherFunc) return false;
    //TODO: do we need to check the type of return value when deciding 
    //compatibility of two function types ??
    if (_derived->Compatible(*otherFunc->_derived))
        return false;
    if (_params.size() != otherFunc->_params.size())
        return false;

    auto thisIter = _params.begin();
    auto otherIter = otherFunc->_params.begin();
    while (thisIter != _params.end()) {
        if ((*thisIter)->Compatible(*(*otherIter)))
            return false;

        thisIter++;
        otherIter++;
    }

    return true;
}

std::string FuncType::Str(void) const
{
    auto str = _derived->Str() + "(";
    auto iter = _params.begin();
    for (; iter != _params.end(); iter++) {
        str += (*iter)->Str() + ", ";
    }
    if (_hasEllipsis)
        str += "...";
    else if (_params.size())
        str.resize(str.size() - 2);

    return str + ")";
}

/*
 * StructUnionType
 */

StructUnionType::StructUnionType(MemPool* pool,
        bool isStruct, bool hasTag, Scope* parent)
        : Type(pool, 0, false), _isStruct(isStruct), _hasTag(hasTag),
          _memberMap(new Scope(parent, S_BLOCK)) {
              _memberMap->SetOffset(0);
          }

Object* StructUnionType::GetMember(const std::string& member) {
    auto ident = _memberMap->FindInCurScope(member);
    if (ident == nullptr)
        return nullptr;
    return ident->ToObject();
}

void StructUnionType::CalcWidth(void)
{
    _width = 0;
    auto iter = _memberMap->_identMap.begin();
    for (; iter != _memberMap->_identMap.end(); iter++) {
        _width += iter->second->Ty()->Width();
    }
}

bool StructUnionType::operator==(const Type& other) const
{
    auto structUnionType = other.ToStructUnionType();
    if (nullptr == structUnionType)
        return false;

    return *_memberMap == *structUnionType->_memberMap;
}

bool StructUnionType::Compatible(const Type& other) const {
    // TODO: 
    return *this == other;
}

// TODO(wgtdkp): more detailed representation
std::string StructUnionType::Str(void) const
{
    std::string str = _isStruct ? "struct": "union";
    return str + ":" + std::to_string(_width);
}

void StructUnionType::AddMember(const std::string& name, Object* member)
{
    _memberMap->InsertWithOutIncOffset(name, member);

    int offset = _memberMap->Offset();
    member->SetOffset(offset);
    if (IsStruct()) {
        _memberMap->SetOffset(offset + member->Ty()->Width());
        _width = _memberMap->Offset();
    } else {
        _width = std::max(_width, member->Ty()->Width());
    }
}

// Move members of Anonymous struct/union to external struct/union
// TODO(wgtdkp):
// Width of struct/union is not the sum of its members
void StructUnionType::Merge(StructUnionType* anonType)
{
    int offset = _memberMap->Offset();
    std::cout << "offset: " << offset << std::endl;
    auto iter = anonType->_memberMap->begin();
    for (; iter != anonType->_memberMap->end(); iter++) {
        if (GetMember(iter->first)) {
            auto tok = iter->second->Tok();
            Error(tok, "duplicate member '%s'", tok->Str().c_str());
        }
        auto member = iter->second->ToObject();
        
        assert(member);
        _memberMap->InsertWithOutIncOffset(iter->first, member);
        member->SetOffset(offset + member->Offset());
    }
    _memberMap->SetOffset(offset + anonType->Width());
    _width = _memberMap->Offset();
}


ArithmType* MaxType(ArithmType* lhsType, ArithmType* rhsType)
{
    int intWidth = ArithmType::CalcWidth(T_INT);
    ArithmType* desType;

    if (lhsType->Width() < intWidth && rhsType->Width() < intWidth) {
        desType = Type::NewArithmType(T_INT);
    } else if (lhsType->Width() > rhsType->Width()) {
        desType = lhsType;
    } else if (lhsType->Width() < rhsType->Width()) {
        desType = rhsType;
    } else if ((lhsType->Tag() & T_FLOAT) || (lhsType->Tag() & T_DOUBLE)) {
        desType = lhsType;
    } else if ((rhsType->Tag() & T_FLOAT) || (rhsType->Tag() & T_DOUBLE)) {
        desType = rhsType;
    } else if (lhsType->Tag() & T_UNSIGNED) {
        desType = lhsType;
    } else {
        desType = rhsType;
    }

    return desType;
}