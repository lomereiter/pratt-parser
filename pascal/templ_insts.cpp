#include "../parser/parser_impl.h"
#include <memory>

struct Node;

template class Symbol<std::shared_ptr<Node>>;
template class PrattParser<std::shared_ptr<Node>>;
template class grammar::Grammar<std::shared_ptr<Node>>;

#define PG grammar::Grammar<std::shared_ptr<Node>>
template struct PG::behaviour_guard<PG::Prefix>;
template struct PG::behaviour_guard<PG::Postfix>;
template struct PG::behaviour_guard<PG::LeftAssociative>;
template struct PG::behaviour_guard<PG::RightAssociative>;
#undef PG
