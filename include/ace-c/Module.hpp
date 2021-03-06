#ifndef MODULE_HPP
#define MODULE_HPP

#include <ace-c/Scope.hpp>
#include <ace-c/SourceLocation.hpp>
#include <ace-c/Tree.hpp>
#include <ace-c/type-system/SymbolType.hpp>

#include <vector>
#include <string>
#include <unordered_set>
#include <functional>

class Module {
public:
    Module(const std::string &name,
        const SourceLocation &location);
    Module(const Module &other) = delete;

    inline const std::string &GetName() const
        { return m_name; }
    inline const SourceLocation &GetLocation() const
        { return m_location; }
    
    inline const std::unordered_set<std::string> &GetScanPaths() const
        { return m_scan_paths; }
    inline void AddScanPath(const std::string &path)
        { m_scan_paths.insert(path); }

    inline TreeNode<Module*> *GetImportTreeLink()
        { return m_tree_link; }
    inline const TreeNode<Module*> *GetImportTreeLink() const
        { return m_tree_link; }
    inline void SetImportTreeLink(TreeNode<Module*> *tree_link)
        { m_tree_link = tree_link; }

    /** Create a string of the module name (including parent module names)
        relative to the global scope */
    std::string GenerateFullModuleName() const;

    /** Reverse iterate the scopes starting from the currently opened scope,
        checking if the scope is nested within a function */
    bool IsInFunction();

    /** Reverse iterate the scopes starting from the currently opened scope,
        checking if the scope is nested within a type definition */
    bool IsInTypeDefinition();

    /** Look up a child module of this module */
    Module *LookupNestedModule(const std::string &name);

    /** Check to see if the identifier exists in multiple scopes, starting
        from the currently opened scope.
        If this_scope_only is set to true, only the current scope will be
        searched.
    */
    Identifier *LookUpIdentifier(const std::string &name, bool this_scope_only);
    
    /** Check to see if the identifier exists in this scope or above this one.
        Will only search the number of depth levels it is given.
        Pass `1` for this scope only.
    */
    Identifier *LookUpIdentifierDepth(const std::string &name, int depth_level);

    /** Look up a symbol in this module by name */
    SymbolTypePtr_t LookupSymbolType(const std::string &name); 
    /** Look up an instance of a generic type with the given parameters */
    SymbolTypePtr_t LookupGenericInstance(const SymbolTypePtr_t &base, 
        const std::vector<GenericInstanceTypeInfo::Arg> &params);

    Tree<Scope> m_scopes;

private:
    std::string m_name;
    SourceLocation m_location;

    // module scan paths
    std::unordered_set<std::string> m_scan_paths;

    /** A link to where this module exists in the import tree */
    TreeNode<Module*> *m_tree_link;

    SymbolTypePtr_t PerformLookup(
        std::function<SymbolTypePtr_t(TreeNode<Scope>*)>,
        std::function<SymbolTypePtr_t(Module *mod)>);
};

#endif
