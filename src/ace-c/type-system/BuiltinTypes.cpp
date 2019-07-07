#include <ace-c/type-system/BuiltinTypes.hpp>
#include <ace-c/type-system/SymbolType.hpp>

#include <ace-c/ast/AstNil.hpp>
#include <ace-c/ast/AstFunctionExpression.hpp>
#include <ace-c/ast/AstString.hpp>
#include <ace-c/ast/AstArrayExpression.hpp>
#include <ace-c/ast/AstTupleExpression.hpp>
#include <ace-c/ast/AstInteger.hpp>
#include <ace-c/ast/AstFloat.hpp>
#include <ace-c/ast/AstFalse.hpp>
#include <ace-c/ast/AstTypeObject.hpp>
#include <ace-c/ast/AstUndefined.hpp>
#include <ace-c/ast/AstBlockExpression.hpp>

const SymbolTypePtr_t BuiltinTypes::PRIMITIVE_TYPE = SymbolType::Primitive(
    "Primitive",
    nullptr,
    nullptr
);

const SymbolTypePtr_t BuiltinTypes::TRAIT_TYPE = SymbolType::Primitive(
    "Trait",
    nullptr,
    BuiltinTypes::PRIMITIVE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::UNDEFINED = SymbolType::Primitive(
    "Undefined",
    sp<AstUndefined>(new AstUndefined(SourceLocation::eof)),
    BuiltinTypes::PRIMITIVE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::ANY_TYPE = SymbolType::Primitive(
    "AnyType",
    sp<AstUndefined>(new AstUndefined(SourceLocation::eof)),
    BuiltinTypes::PRIMITIVE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::ANY = SymbolType::Primitive(
    "Any",
    sp<AstNil>(new AstNil(SourceLocation::eof)),
    BuiltinTypes::ANY_TYPE
);

const SymbolTypePtr_t BuiltinTypes::OBJECT = SymbolType::Primitive(
    "Object",
    nullptr,
    BuiltinTypes::PRIMITIVE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::TYPE_TYPE = SymbolType::Extend(
    "Type",
    BuiltinTypes::OBJECT,
    std::vector<SymbolMember_t> {
        SymbolMember_t {
            "$proto",
            BuiltinTypes::ANY,
            nullptr
        },
        SymbolMember_t {
            "base",
            BuiltinTypes::OBJECT,
            sp<AstTypeObject>(new AstTypeObject(
                BuiltinTypes::OBJECT,
                nullptr,
                SourceLocation::eof
            )),
        }
    }
);

const SymbolTypePtr_t BuiltinTypes::ENUM_TYPE = SymbolType::Primitive(
    "Enum",
    sp<AstUndefined>(new AstUndefined(SourceLocation::eof)),
    BuiltinTypes::PRIMITIVE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::INT = SymbolType::Extend(
    "Int",
    BuiltinTypes::TYPE_TYPE,
    std::vector<SymbolMember_t> {
        SymbolMember_t {
            "$proto",
            SymbolType::Primitive(
                "IntInstance", nullptr
            ),
            sp<AstInteger>(new AstInteger(0, SourceLocation::eof))
        },
        SymbolMember_t {
            "base",
            BuiltinTypes::TYPE_TYPE,
            sp<AstTypeObject>(new AstTypeObject(
                BuiltinTypes::TYPE_TYPE,
                nullptr,
                SourceLocation::eof
            )),
        }
    }
);

/*const SymbolTypePtr_t BuiltinTypes::INT = SymbolType::Primitive(
    "Int",
    sp<AstInteger>(new AstInteger(0, SourceLocation::eof)),
    BuiltinTypes::TYPE_TYPE
);*/


const SymbolTypePtr_t BuiltinTypes::FLOAT = SymbolType::Extend(
    "Float",
    BuiltinTypes::TYPE_TYPE,
    std::vector<SymbolMember_t> {
        SymbolMember_t {
            "$proto",
            SymbolType::Primitive(
                "FloatInstance", nullptr
            ),
            sp<AstFloat>(new AstFloat(0.0, SourceLocation::eof))
        },
        SymbolMember_t {
            "base",
            BuiltinTypes::TYPE_TYPE,
            sp<AstTypeObject>(new AstTypeObject(
                BuiltinTypes::TYPE_TYPE,
                nullptr,
                SourceLocation::eof
            )),
        }
    }
);

// const SymbolTypePtr_t BuiltinTypes::FLOAT = SymbolType::Primitive(
//     "Float",
//     sp<AstFloat>(new AstFloat(0.0, SourceLocation::eof))
// );

// const SymbolTypePtr_t BuiltinTypes::NUMBER = SymbolType::Primitive(
//     "Number",
//     sp<AstInteger>(new AstInteger(0, SourceLocation::eof))
// );
const SymbolTypePtr_t BuiltinTypes::NUMBER = SymbolType::Extend(
    "Number",
    BuiltinTypes::TYPE_TYPE,
    std::vector<SymbolMember_t> {
        SymbolMember_t {
            "$proto",
            SymbolType::Primitive(
                "NumberInstance", nullptr
            ),
            sp<AstFloat>(new AstFloat(0.0, SourceLocation::eof))
        },
        SymbolMember_t {
            "base",
            BuiltinTypes::TYPE_TYPE,
            sp<AstTypeObject>(new AstTypeObject(
                BuiltinTypes::TYPE_TYPE,
                nullptr,
                SourceLocation::eof
            )),
        }
    }
);


// const SymbolTypePtr_t BuiltinTypes::BOOLEAN = SymbolType::Primitive(
//     "Boolean",
//     sp<AstFalse>(new AstFalse(SourceLocation::eof))
// );

const SymbolTypePtr_t BuiltinTypes::BOOLEAN = SymbolType::Extend(
    "Bool",
    BuiltinTypes::TYPE_TYPE,
    std::vector<SymbolMember_t> {
        SymbolMember_t {
            "$proto",
            SymbolType::Primitive(
                "BoolInstance", nullptr
            ),
            sp<AstFalse>(new AstFalse(SourceLocation::eof))
        },
        SymbolMember_t {
            "base",
            BuiltinTypes::TYPE_TYPE,
            sp<AstTypeObject>(new AstTypeObject(
                BuiltinTypes::TYPE_TYPE,
                nullptr,
                SourceLocation::eof
            )),
        }
    }
);

// const SymbolTypePtr_t BuiltinTypes::STRING = SymbolType::Primitive(
//     "String",
//     sp<AstString>(new AstString("", SourceLocation::eof))
// );


const SymbolTypePtr_t BuiltinTypes::STRING = SymbolType::Extend(
    "String",
    BuiltinTypes::TYPE_TYPE,
    std::vector<SymbolMember_t> {
        SymbolMember_t {
            "$proto",
            SymbolType::Primitive(
                "StringInstance", nullptr
            ),
            sp<AstString>(new AstString("", SourceLocation::eof))
        },
        SymbolMember_t {
            "base",
            BuiltinTypes::TYPE_TYPE,
            sp<AstTypeObject>(new AstTypeObject(
                BuiltinTypes::TYPE_TYPE,
                nullptr,
                SourceLocation::eof
            )),
        }
    }
);

const SymbolTypePtr_t BuiltinTypes::FUNCTION = SymbolType::Generic(
    "Function",
    sp<AstFunctionExpression>(new AstFunctionExpression(
        {},
        nullptr,
        sp<AstBlock>(new AstBlock(SourceLocation::eof)),
        false,
        false,
        false,
        SourceLocation::eof
    )),
    {},
    GenericTypeInfo{ -1 },
    BuiltinTypes::TYPE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::ARRAY = SymbolType::Generic(
    "Array",
    std::vector<SymbolMember_t> {
        SymbolMember_t {
            "$proto",
            SymbolType::Primitive(
                "ArrayInstance", nullptr
            ),
            sp<AstArrayExpression>(new AstArrayExpression(
                {}, SourceLocation::eof
            ))
        },
        SymbolMember_t {
            "base",
            BuiltinTypes::TYPE_TYPE,
            sp<AstTypeObject>(new AstTypeObject(
                BuiltinTypes::TYPE_TYPE,
                nullptr,
                SourceLocation::eof
            )),
        }
    },
    GenericTypeInfo { 1 },
    BuiltinTypes::TYPE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::TUPLE = SymbolType::Generic(
    "Tuple",
    sp<AstTupleExpression>(new AstTupleExpression(
        {},
        SourceLocation::eof
    )),
    {},
    GenericTypeInfo { -1 },
    BuiltinTypes::TYPE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::VAR_ARGS = SymbolType::Generic(
    "Args",
    sp<AstArrayExpression>(new AstArrayExpression(
        {},
        SourceLocation::eof
    )),
    {},
    GenericTypeInfo { 1 },
    BuiltinTypes::TYPE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::NULL_TYPE = SymbolType::Primitive(
    "Null",
    sp<AstNil>(new AstNil(SourceLocation::eof)),
    BuiltinTypes::TYPE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::EVENT_IMPL = SymbolType::Object(
    "Event_impl",
    {
        SymbolMember_t {
            "key",
            BuiltinTypes::STRING,
            BuiltinTypes::STRING->GetDefaultValue()
        },
        SymbolMember_t {
            "trigger",
            BuiltinTypes::FUNCTION,
            BuiltinTypes::FUNCTION->GetDefaultValue()
        }
    }
);

const SymbolTypePtr_t BuiltinTypes::EVENT = SymbolType::Generic(
    "$Event",
    BuiltinTypes::UNDEFINED->GetDefaultValue(),
    {},
    GenericTypeInfo { 1 },
    BuiltinTypes::TYPE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::MODULE_INFO = SymbolType::Object(
    "ModuleInfo",
    {
        SymbolMember_t {
            "id",
            BuiltinTypes::INT,
            BuiltinTypes::INT->GetDefaultValue()
        },
        SymbolMember_t {
            "name",
            BuiltinTypes::STRING,
            BuiltinTypes::STRING->GetDefaultValue()
        }
    }
);

const SymbolTypePtr_t BuiltinTypes::GENERATOR = SymbolType::Generic(
    "Generator",
    sp<AstFunctionExpression>(new AstFunctionExpression(
        {},
        nullptr, 
        sp<AstBlock>(new AstBlock(SourceLocation::eof)),
        false,
        false,
        false,
        SourceLocation::eof
    )),
    {},
    GenericTypeInfo{ 1 },
    BuiltinTypes::TYPE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::BOXED_TYPE = SymbolType::Generic(
    "Boxed",
    nullptr,
    {},
    GenericTypeInfo { 1 },
    BuiltinTypes::TRAIT_TYPE
);

const SymbolTypePtr_t BuiltinTypes::MAYBE = SymbolType::Generic(
    "Maybe",
    sp<AstNil>(new AstNil(SourceLocation::eof)),
    {},
    GenericTypeInfo { 1 },
    BuiltinTypes::BOXED_TYPE
);

const SymbolTypePtr_t BuiltinTypes::CONST_TYPE_TYPE = SymbolType::Primitive(
    "ConstType",
    sp<AstUndefined>(new AstUndefined(SourceLocation::eof)),
    BuiltinTypes::PRIMITIVE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::CONST_TYPE = SymbolType::Generic(
    "Const",
    nullptr,
    {},
    GenericTypeInfo { 1 },
    BuiltinTypes::CONST_TYPE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::BLOCK_TYPE = SymbolType::Generic(
    "Block",
    nullptr,
    {},
    GenericTypeInfo { -1 },
    BuiltinTypes::TYPE_TYPE
);

const SymbolTypePtr_t BuiltinTypes::CLOSURE_TYPE = SymbolType::Generic(
    "Closure",
    {},
    GenericTypeInfo { -1 },
    BuiltinTypes::FUNCTION
);

const SymbolTypePtr_t BuiltinTypes::META_CLOSURE_TYPE = SymbolType::Generic(
    "MetaClosure",
    nullptr,
    {},
    GenericTypeInfo { -1 },
    BuiltinTypes::FUNCTION
);

const SymbolTypePtr_t BuiltinTypes::GENERIC_VARIABLE_TYPE = SymbolType::Generic(
    "Generic",
    std::vector<SymbolMember_t> {
        SymbolMember_t {
            "$proto",
            SymbolType::Primitive(
                "GenericInstance", nullptr
            ),
            nullptr
        },
        SymbolMember_t {
            "base",
            BuiltinTypes::TYPE_TYPE,
            sp<AstTypeObject>(new AstTypeObject(
                BuiltinTypes::TYPE_TYPE,
                nullptr,
                SourceLocation::eof
            )),
        }
    },
    GenericTypeInfo { -1 },
    BuiltinTypes::TYPE_TYPE
);
