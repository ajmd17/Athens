#ifndef AST_ITERATOR_HPP
#define AST_ITERATOR_HPP

#include <athens/ast/ast_statement.hpp>

#include <memory>
#include <vector>

class AstIterator {
public:
    AstIterator();
    AstIterator(const AstIterator &other);

    inline void Push(const std::shared_ptr<AstStatement> &statement) { m_list.push_back(statement); }
    inline void ResetPosition() { m_position = 0; }
    inline std::shared_ptr<AstStatement> &Peek() { return m_list[m_position]; }
    inline const std::shared_ptr<AstStatement> &Peek() const { return m_list[m_position]; }
    inline std::shared_ptr<AstStatement> Next() { return m_list[m_position++]; }
    inline bool HasNext() const { return m_position < m_list.size(); }
    inline const SourceLocation &GetLocation() const { return m_list[m_position]->m_location; }

    std::vector<std::shared_ptr<AstStatement>> m_list;
    int m_position;
};

#endif
