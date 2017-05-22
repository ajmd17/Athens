#ifndef SYMBOL_TYPE_HPP
#define SYMBOL_TYPE_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <tuple>

template <typename T> using sp = std::shared_ptr<T>;
template <typename T> using wp = std::weak_ptr<T>;
template <typename T> using vec = std::vector<T>;

// forward declaration
class SymbolType;
class AstExpression;

typedef sp<SymbolType> SymbolTypePtr_t;
typedef wp<SymbolType> SymbolTypeWeakPtr_t;
typedef std::tuple<std::string, SymbolTypePtr_t, sp<AstExpression>> SymbolMember_t;

enum SymbolTypeClass {
    TYPE_BUILTIN,
    TYPE_USER_DEFINED,
    TYPE_ALIAS,
    TYPE_FUNCTION,
    TYPE_ARRAY,
    TYPE_GENERIC,
    TYPE_GENERIC_INSTANCE,
    TYPE_GENERIC_PARAMETER,
};

struct AliasTypeInfo {
    SymbolTypeWeakPtr_t m_aliasee;
};

struct FunctionTypeInfo {
    vec<SymbolTypePtr_t> m_param_types;
    SymbolTypePtr_t m_return_type;
};

struct GenericTypeInfo {
    int m_num_parameters; // -1 for variadic
    vec<SymbolTypePtr_t> m_params;
};

struct GenericInstanceTypeInfo {
    vec<std::pair<std::string, SymbolTypePtr_t>> m_generic_args;
};

struct GenericParameterTypeInfo {
    SymbolTypeWeakPtr_t m_substitution;
};

class SymbolType {
public:
    struct Builtin {
        static const SymbolTypePtr_t UNDEFINED;
        static const SymbolTypePtr_t OBJECT;
        static const SymbolTypePtr_t ANY;
        static const SymbolTypePtr_t INT;
        static const SymbolTypePtr_t FLOAT;
        static const SymbolTypePtr_t NUMBER;
        static const SymbolTypePtr_t BOOLEAN;
        static const SymbolTypePtr_t STRING;
        static const SymbolTypePtr_t FUNCTION;
        static const SymbolTypePtr_t ARRAY;
        static const SymbolTypePtr_t VAR_ARGS;
        static const SymbolTypePtr_t MAYBE;
        static const SymbolTypePtr_t NULL_TYPE;
        static const SymbolTypePtr_t EVENT;
        static const SymbolTypePtr_t EVENT_IMPL;
        static const SymbolTypePtr_t EVENT_ARRAY;
    };

    static SymbolTypePtr_t Alias(
        const std::string &name,
        const AliasTypeInfo &info
    );

    static SymbolTypePtr_t Primitive(
        const std::string &name, 
        const sp<AstExpression> &default_value
    );

    static SymbolTypePtr_t Primitive(
        const std::string &name,
        const sp<AstExpression> &default_value,
        const SymbolTypePtr_t &base
    );

    static SymbolTypePtr_t Object(
        const std::string &name,
        const vec<SymbolMember_t> &members
    );

    /** A generic type template. Members may have the type class TYPE_GENERIC_PARAMETER.
        They will be substituted when an instance of the generic type is created.
    */
    static SymbolTypePtr_t Generic(
        const std::string &name, 
        const sp<AstExpression> &default_value, 
        const vec<SymbolMember_t> &members, 
        const GenericTypeInfo &info
    );

    static SymbolTypePtr_t GenericInstance(
        const SymbolTypePtr_t &base,
        const GenericInstanceTypeInfo &info
    );

    static SymbolTypePtr_t GenericParameter(
        const std::string &name, 
        const SymbolTypePtr_t &substitution
    );

    static SymbolTypePtr_t TypePromotion(
        const SymbolTypePtr_t &lptr,
        const SymbolTypePtr_t &rptr,
        bool use_number
    );

public:
    SymbolType(
        const std::string &name, 
        SymbolTypeClass type_class, 
        const SymbolTypePtr_t &base
    );

    SymbolType(
        const std::string &name, 
        SymbolTypeClass type_class, 
        const SymbolTypePtr_t &base,
        const sp<AstExpression> &default_value,
        const vec<SymbolMember_t> &members
    );
        
    SymbolType(const SymbolType &other);

    inline const std::string &GetName() const { return m_name; }
    inline SymbolTypeClass GetTypeClass() const { return m_type_class; }
    inline SymbolTypePtr_t GetBaseType() const { return m_base.lock(); }

    inline const sp<AstExpression> &GetDefaultValue() const
        { return m_default_value; }
    inline void SetDefaultValue(const sp<AstExpression> &default_value)
        { m_default_value = default_value; }
    
    inline vec<SymbolMember_t> &GetMembers()
        { return m_members; }
    inline const vec<SymbolMember_t> &GetMembers() const
        { return m_members; }

    inline AliasTypeInfo &GetAliasInfo()
        { return m_alias_info; }
    inline const AliasTypeInfo &GetAliasInfo() const
        { return m_alias_info; }

    inline FunctionTypeInfo &GetFunctionInfo()
        { return m_function_info; }
    inline const FunctionTypeInfo &GetFunctionInfo() const
        { return m_function_info; }

    inline GenericTypeInfo &GetGenericInfo()
        { return m_generic_info; }
    inline const GenericTypeInfo &GetGenericInfo() const
        { return m_generic_info; }

    inline GenericInstanceTypeInfo &GetGenericInstanceInfo()
        { return m_generic_instance_info; }
    inline const GenericInstanceTypeInfo &GetGenericInstanceInfo() const
        { return m_generic_instance_info; }

    inline GenericParameterTypeInfo &GetGenericParameterInfo()
        { return m_generic_param_info; }
    inline const GenericParameterTypeInfo &GetGenericParameterInfo() const
        { return m_generic_param_info; }

    inline int GetId() const { return m_id; }
    inline void SetId(int id) { m_id = id; }

    bool TypeEqual(const SymbolType &other) const;
    bool TypeCompatible(const SymbolType &other, bool strict_numbers) const;

    inline bool operator==(const SymbolType &other) const { return TypeEqual(other); }
    inline bool operator!=(const SymbolType &other) const { return !operator==(other); }
    const SymbolTypePtr_t FindMember(const std::string &name) const;

private:
    std::string m_name;
    SymbolTypeClass m_type_class;
    sp<AstExpression> m_default_value;
    vec<SymbolMember_t> m_members;

    // type that this type is based off of
    SymbolTypeWeakPtr_t m_base;

    // if this is an alias of another type
    AliasTypeInfo m_alias_info;
    // if this type is a function
    FunctionTypeInfo m_function_info;
    // if this is a generic type
    GenericTypeInfo m_generic_info;
    // if this is an instance of a generic type
    GenericInstanceTypeInfo m_generic_instance_info;
    // if this is a generic param
    GenericParameterTypeInfo m_generic_param_info;

    int m_id;
};

#endif