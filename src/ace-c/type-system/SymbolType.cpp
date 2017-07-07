#include <ace-c/type-system/SymbolType.hpp>
#include <ace-c/type-system/BuiltinTypes.hpp>

#include <ace-c/ast/AstObject.hpp>

#include <common/my_assert.hpp>
#include <common/utf8.hpp>

SymbolType::SymbolType(const std::string &name, 
    SymbolTypeClass type_class, 
    const SymbolTypePtr_t &base)
    : m_name(name),
      m_type_class(type_class),
      m_base(base),
      m_default_value(nullptr),
      m_id(-1)
{
}

SymbolType::SymbolType(const std::string &name, 
    SymbolTypeClass type_class,
    const SymbolTypePtr_t &base,
    const sp<AstExpression> &default_value,
    const vec<SymbolMember_t> &members)
    : m_name(name),
      m_type_class(type_class),
      m_base(base),
      m_default_value(default_value),
      m_members(members),
      m_id(0)
{
}

SymbolType::SymbolType(const SymbolType &other)
    : m_name(other.m_name),
      m_type_class(other.m_type_class),
      m_base(other.m_base),
      m_default_value(other.m_default_value),
      m_members(other.m_members),
      m_id(other.m_id)
{
}

bool SymbolType::TypeEqual(const SymbolType &other) const
{
    if (m_type_class != other.m_type_class) {
        return false;
    }

    switch (m_type_class) {
        case TYPE_ALIAS: {
            SymbolTypePtr_t sp = m_alias_info.m_aliasee.lock();
            return sp != nullptr && (*sp) == other;
        }
        case TYPE_FUNCTION:
            if (!m_function_info.m_return_type || !other.m_function_info.m_return_type) {
                return false;
            }

            if (!((*m_function_info.m_return_type) == (*other.m_function_info.m_return_type))) {
                return false;
            }

            for (const SymbolTypePtr_t &i : m_function_info.m_param_types) {
                if (i == nullptr) {
                    return false;
                }

                for (const SymbolTypePtr_t &j : other.m_function_info.m_param_types) {
                    if (j == nullptr || !((*i) == (*j))) {
                        return false;
                    }
                }
            }
            break;
        case TYPE_GENERIC:
            if (m_generic_info.m_num_parameters != other.m_generic_info.m_num_parameters) {
                return false;
            }
            break;
        case TYPE_GENERIC_INSTANCE:
            if (m_generic_instance_info.m_generic_args.size() != other.m_generic_instance_info.m_generic_args.size()) {
                return false;
            }

            // check each substituted parameter
            for (size_t i = 0; i < m_generic_instance_info.m_generic_args.size(); i++) {
                const SymbolTypePtr_t &instance_arg_type = m_generic_instance_info.m_generic_args[i].m_type;
                const SymbolTypePtr_t &other_arg_type = other.m_generic_instance_info.m_generic_args[i].m_type;

                ASSERT(instance_arg_type != nullptr);
                ASSERT(other_arg_type != nullptr);

                if (!instance_arg_type->TypeEqual(*other_arg_type)) {
                    return false;
                }
            }

            break;
        default:
            break;
    }

    if (m_name != other.m_name) {
        return false;
    }

    if (m_members.size() != other.m_members.size()) {
        return false;
    }

    for (const SymbolMember_t &i : m_members) {
        ASSERT(std::get<1>(i) != nullptr);

        bool right_member_found = false;

        for (const SymbolMember_t &j : other.m_members) {
            ASSERT(std::get<1>(j) != nullptr);

            if (std::get<1>(i)->TypeEqual(*std::get<1>(j))) {
                right_member_found = true;
                continue;
            }
        }

        if (!right_member_found) {
            // none found. return false.
            return false;
        }
    }

    return true;
}

bool SymbolType::TypeCompatible(const SymbolType &right,
    bool strict_numbers, bool strict_const) const
{
    if (TypeEqual(right)) {
        return true;
    }
    
    if (right.GetTypeClass() == TYPE_GENERIC_PARAMETER &&
        right.GetGenericParameterInfo().m_substitution.lock() == nullptr) {
        // right is a generic paramter that has not yet been substituted
        return true;
    }

    switch (m_type_class) {
        case TYPE_ALIAS: {
            SymbolTypePtr_t sp = m_alias_info.m_aliasee.lock();
            ASSERT(sp != nullptr);

            return sp->TypeCompatible(right, strict_numbers);
        }
        case TYPE_GENERIC: {
            if (right.m_type_class != TYPE_GENERIC) {
                if (SymbolTypePtr_t other_base = right.m_base.lock()) {
                    return TypeCompatible(*other_base, strict_numbers);
                }
            } // equality would have already been checked

            return false;
        }
        case TYPE_GENERIC_INSTANCE: {
            SymbolTypePtr_t base = m_base.lock();
            ASSERT(base != nullptr);

            if (right.m_type_class == TYPE_GENERIC_INSTANCE) {
                // check for compatibility between instances
                SymbolTypePtr_t other_base = right.m_base.lock();
                ASSERT(other_base != nullptr);

                // check if bases are compatible
                if (base != other_base && !base->TypeCompatible(*other_base, strict_numbers)) {
                    return false;
                }

                // check all params
                if (m_generic_instance_info.m_generic_args.size() != right.m_generic_instance_info.m_generic_args.size()) {
                    return false;
                }

                // check each substituted parameter
                for (size_t i = 0; i < m_generic_instance_info.m_generic_args.size(); i++) {
                    const SymbolTypePtr_t &param_type = m_generic_instance_info.m_generic_args[i].m_type;
                    const SymbolTypePtr_t &other_param_type = right.m_generic_instance_info.m_generic_args[i].m_type;

                    ASSERT(param_type != nullptr);
                    ASSERT(other_param_type != nullptr);

                    if (param_type != other_param_type && !param_type->TypeEqual(*other_param_type)) {
                        return false;
                    }
                }

                return true;
            } else {
                // allow 'any' on right as well for generics
                if (right.TypeEqual(*BuiltinTypes::ANY)) {
                    return true;
                }

                // allow boxing/unboxing for 'Maybe(T)' type
                if (base->TypeEqual(*BuiltinTypes::MAYBE)) {
                    if (right.TypeEqual(*BuiltinTypes::NULL_TYPE)) {
                        return true;
                    } else {
                        const SymbolTypePtr_t &held_type = m_generic_instance_info.m_generic_args[0].m_type;
                        ASSERT(held_type != nullptr);
                        return held_type->TypeCompatible(
                            right,
                            strict_numbers
                        );
                    }
                }
                // allow boxing/unboxing for 'Const(T)' type
                else if (base->TypeEqual(*BuiltinTypes::CONST_TYPE)) {
                    // strict_const means you can't assign a const (left) to non-const (right)
                    if (strict_const) {
                        return false;
                    }

                    const SymbolTypePtr_t &held_type = m_generic_instance_info.m_generic_args[0].m_type;
                    ASSERT(held_type != nullptr);
                    return held_type->TypeCompatible(
                        right,
                        strict_numbers
                    );
                }

                return false;
            }

            break;
        }
        case TYPE_BUILTIN: {
            if (!TypeEqual(*BuiltinTypes::UNDEFINED) && !right.TypeEqual(*BuiltinTypes::UNDEFINED)) {
                if (TypeEqual(*BuiltinTypes::ANY) || right.TypeEqual(*BuiltinTypes::ANY)) {
                    return true;
                } else if (TypeEqual(*BuiltinTypes::NUMBER)) {
                    return (right.TypeEqual(*BuiltinTypes::INT) ||
                            right.TypeEqual(*BuiltinTypes::FLOAT));
                } else if (!strict_numbers) {
                    if (TypeEqual(*BuiltinTypes::INT) || TypeEqual(*BuiltinTypes::FLOAT)) {
                        return (right.TypeEqual(*BuiltinTypes::NUMBER) ||
                                right.TypeEqual(*BuiltinTypes::FLOAT) ||
                                right.TypeEqual(*BuiltinTypes::INT));
                    }
                }
            }

            return false;
        }
        
        case TYPE_USER_DEFINED:
            return false;

        case TYPE_GENERIC_PARAMETER: {
            if (auto sp = m_generic_param_info.m_substitution.lock()) {
                return sp->TypeCompatible(right, strict_numbers);
            }
            return true;
        }
    }

    return true;
}

const SymbolTypePtr_t SymbolType::FindMember(const std::string &name) const
{
    for (const SymbolMember_t &member : m_members) {
        if (std::get<0>(member) == name) {
            return std::get<1>(member);
        }
    }

    return nullptr;
}

bool SymbolType::FindMember(const std::string &name, SymbolMember_t &out) const
{
    for (const SymbolMember_t &member : m_members) {
        if (std::get<0>(member) == name) {
            out = member;
            return true;
        }
    }

    return false;
}

bool SymbolType::IsArrayType() const
{
    // compare directly to ARRAY type
    if (this == BuiltinTypes::ARRAY.get()) {
        return true;
    } else if (m_type_class == TYPE_GENERIC_INSTANCE) {
        // type is not Array, so check base class if it is a generic instance
        // e.g Array(Int)
        if (const SymbolTypePtr_t base = m_base.lock()) {
            if (base == BuiltinTypes::ARRAY ||
                base == BuiltinTypes::VAR_ARGS) {
                return true;
            }
        }
    }

    return false;
}

bool SymbolType::IsConstType() const
{
    if (this == BuiltinTypes::CONST_TYPE.get()) {
        return true;
    } else if (m_type_class == TYPE_GENERIC_INSTANCE) {
        if (const SymbolTypePtr_t base = m_base.lock()) {
            return base == BuiltinTypes::CONST_TYPE;
        }
    }

    return false;
}

SymbolTypePtr_t SymbolType::Alias(
    const std::string &name,
    const AliasTypeInfo &info)
{
    if (auto sp = info.m_aliasee.lock()) {
        SymbolTypePtr_t res(new SymbolType(
            name,
            TYPE_ALIAS,
            nullptr
        ));

        res->m_alias_info = info;
        res->SetId(sp->GetId());

        return res;
    }

    return nullptr;
}

SymbolTypePtr_t SymbolType::Primitive(const std::string &name, 
    const sp<AstExpression> &default_value)
{
    return SymbolTypePtr_t(new SymbolType(
        name,
        TYPE_BUILTIN,
        BuiltinTypes::OBJECT,
        default_value,
        {}
    ));
}

SymbolTypePtr_t SymbolType::Primitive(const std::string &name,
    const sp<AstExpression> &default_value,
    const SymbolTypePtr_t &base)
{
    return SymbolTypePtr_t(new SymbolType(
        name,
        TYPE_BUILTIN,
        base,
        default_value,
        {}
    ));
}

SymbolTypePtr_t SymbolType::Object(const std::string &name,
    const vec<SymbolMember_t> &members)
{
    SymbolTypePtr_t symbol_type(new SymbolType(
        name,
        TYPE_USER_DEFINED,
        BuiltinTypes::OBJECT,
        nullptr,
        members
    ));

    symbol_type->SetDefaultValue(sp<AstObject>(
        new AstObject(symbol_type, SourceLocation::eof)
    ));
    
    return symbol_type;
}

SymbolTypePtr_t SymbolType::Generic(const std::string &name, 
    const sp<AstExpression> &default_value,
    const vec<SymbolMember_t> &members, 
    const GenericTypeInfo &info)
{
    SymbolTypePtr_t res(new SymbolType(
        name,
        TYPE_GENERIC,
        BuiltinTypes::OBJECT,
        default_value,
        members
    ));
    
    res->m_generic_info = info;
    
    return res;
}

SymbolTypePtr_t SymbolType::GenericInstance(
    const SymbolTypePtr_t &base,
    const GenericInstanceTypeInfo &info)
{
    ASSERT(base != nullptr);
    ASSERT(base->GetTypeClass() == TYPE_GENERIC);

    std::string name;
    std::string return_type_name;
    bool has_return_type = false;

    if (!info.m_generic_args.empty()) {
        if (base == BuiltinTypes::ARRAY) {
            ASSERT(!info.m_generic_args.empty());

            const SymbolTypePtr_t &held_type = info.m_generic_args.front().m_type;
            ASSERT(held_type != nullptr);

            name = held_type->GetName() + "[]";
        } else if (base == BuiltinTypes::VAR_ARGS) {
            ASSERT(!info.m_generic_args.empty());

            const SymbolTypePtr_t &held_type = info.m_generic_args.front().m_type;
            ASSERT(held_type != nullptr);

            name = held_type->GetName() + "...";
        } else {
            name = base->GetName() + "(";

            for (size_t i = 0; i < info.m_generic_args.size(); i++) {
                const std::string &generic_arg_name = info.m_generic_args[i].m_name;
                const SymbolTypePtr_t &generic_arg_type = info.m_generic_args[i].m_type;

                ASSERT(generic_arg_type != nullptr);

                if (generic_arg_name == "@return") {
                    has_return_type = true;
                    return_type_name = generic_arg_type->GetName();
                } else {
                    //name += generic_arg_name;
                    //name += ": ";
                    name += generic_arg_type->GetName();
                    if (i != info.m_generic_args.size() - 1) {
                        name += ", ";
                    }
                }
            }

            name += ")";

            if (has_return_type) {
                name += " -> " + return_type_name;
            }
        }
    }

    vec<SymbolMember_t> members;
    members.reserve(base->GetMembers().size());

    for (const SymbolMember_t &member : base->GetMembers()) {
        bool is_substituted = false;

        if (std::get<1>(member)->GetTypeClass() == TYPE_GENERIC_PARAMETER) {
            // if members of the generic/template class are of the type T (generic parameter)
            // we need to make sure that the number of parameters supplied are equal.
            ASSERT(base->GetGenericInfo().m_params.size() == info.m_generic_args.size());
            
            // find parameter and substitute it
            for (size_t i = 0; !is_substituted && i < base->GetGenericInfo().m_params.size(); i++) {
                SymbolTypePtr_t &it = base->GetGenericInfo().m_params[i];

                if (it->GetName() == std::get<1>(member)->GetName()) {
                    sp<AstExpression> default_value;

                    if ((default_value = std::get<2>(member))) {
                        default_value = CloneAstNode(default_value);
                    }

                    members.push_back(SymbolMember_t(
                        std::get<0>(member),
                        info.m_generic_args[i].m_type,
                        default_value
                    ));

                    is_substituted = true;
                }
            }

            if (!is_substituted) {
                // substitution error, set type to be undefined
                members.push_back(SymbolMember_t(
                    std::get<0>(member),
                    BuiltinTypes::UNDEFINED,
                    std::get<2>(member)
                ));
            }
        } else {
            // push copy (clone assignment value)
            members.push_back(SymbolMember_t(
                std::get<0>(member),
                std::get<1>(member),
                CloneAstNode(std::get<2>(member))
            ));
        }
    }

    // if the generic's default value is nullptr,
    // create a new default value for the instance of type AstObject
    // the reason we do this is so that a new 'Type' is generated for user-defined
    // generics, but built-in generics like Function and Array can play by
    // their own rules

    SymbolTypePtr_t res(new SymbolType(
        name,
        TYPE_GENERIC_INSTANCE,
        base,
        nullptr,
        members
    ));

    auto default_value = base->GetDefaultValue();
    // if (default_value == nullptr) {
    //     default_value.reset(new AstObject(res, SourceLocation::eof));
    // }

    res->SetId(base->GetId());
    res->SetDefaultValue(default_value);
    res->m_generic_instance_info = info;

    return res;
}

SymbolTypePtr_t SymbolType::GenericParameter(
    const std::string &name, 
    const SymbolTypePtr_t &substitution)
{
    SymbolTypePtr_t res(new SymbolType(
        name,
        TYPE_GENERIC_PARAMETER,
        nullptr
    ));

    res->m_generic_param_info.m_substitution = substitution;
    
    return res;
}

SymbolTypePtr_t SymbolType::Extend(
    const SymbolTypePtr_t &base,
    const vec<SymbolMember_t> &members)
{
    SymbolTypePtr_t symbol_type(new SymbolType(
        base->GetName(),
        TYPE_USER_DEFINED,
        base,
        nullptr,
        members
    ));

    symbol_type->SetDefaultValue(sp<AstObject>(
        new AstObject(symbol_type, SourceLocation::eof)
    ));
    
    return symbol_type;
}

SymbolTypePtr_t SymbolType::TypePromotion(
    const SymbolTypePtr_t &lptr,
    const SymbolTypePtr_t &rptr,
    bool use_number)
{
    if (lptr == nullptr || rptr == nullptr) {
        return nullptr;
    }

    // compare pointer values
    if (lptr == rptr || lptr->TypeEqual(*rptr)) {
        return lptr;
    }

    if (lptr->TypeEqual(*BuiltinTypes::UNDEFINED) ||
        rptr->TypeEqual(*BuiltinTypes::UNDEFINED))
    {
        // (Undefined | Any) + (Undefined | Any) = Undefined
        return BuiltinTypes::UNDEFINED;
    } else if (lptr->TypeEqual(*BuiltinTypes::ANY)) {
        // Any + T = Any
        return BuiltinTypes::ANY;
    } else if (rptr->TypeEqual(*BuiltinTypes::ANY)) {
        // T + Any = Any
        return BuiltinTypes::ANY;//lptr;
    } else if (lptr->TypeEqual(*BuiltinTypes::NUMBER)) {
        return rptr->TypeCompatible(*BuiltinTypes::INT, true) ||
               rptr->TypeCompatible(*BuiltinTypes::FLOAT, true)
               ? BuiltinTypes::NUMBER
               : BuiltinTypes::UNDEFINED;
    } else if (lptr->TypeEqual(*BuiltinTypes::INT)) {
        return rptr->TypeCompatible(*BuiltinTypes::NUMBER, true) ||
               rptr->TypeCompatible(*BuiltinTypes::FLOAT, true)
               ? (use_number ? BuiltinTypes::NUMBER : rptr)
               : BuiltinTypes::UNDEFINED;
    } else if (lptr->TypeEqual(*BuiltinTypes::FLOAT)) {
        return rptr->TypeCompatible(*BuiltinTypes::NUMBER, true) ||
               rptr->TypeCompatible(*BuiltinTypes::INT, true)
               ? (use_number ? BuiltinTypes::NUMBER : lptr)
               : BuiltinTypes::UNDEFINED;
    } else if (rptr->TypeEqual(*BuiltinTypes::NUMBER)) {
        return lptr->TypeCompatible(*BuiltinTypes::INT, true) ||
               lptr->TypeCompatible(*BuiltinTypes::FLOAT, true)
               ? BuiltinTypes::NUMBER
               : BuiltinTypes::UNDEFINED;
    } else if (rptr->TypeEqual(*BuiltinTypes::INT)) {
        return lptr->TypeCompatible(*BuiltinTypes::NUMBER, true) ||
               lptr->TypeCompatible(*BuiltinTypes::FLOAT, true)
               ? (use_number ? BuiltinTypes::NUMBER : lptr)
               : BuiltinTypes::UNDEFINED;
    } else if (rptr->TypeEqual(*BuiltinTypes::FLOAT)) {
        return lptr->TypeCompatible(*BuiltinTypes::NUMBER, true) ||
               lptr->TypeCompatible(*BuiltinTypes::INT, true)
               ? (use_number ? BuiltinTypes::NUMBER : rptr)
               : BuiltinTypes::UNDEFINED;
    }

    return lptr;//BuiltinTypes::UNDEFINED;
}