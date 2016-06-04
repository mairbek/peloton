#pragma once

#include "parser/statements.h"
#include "expression/expressions.h"
#include "common/types.h"

namespace peloton {
namespace parser {

void GetSelectStatementInfo(SelectStatement* stmt, uint num_indent);
void GetInsertStatementInfo(InsertStatement* stmt, uint num_indent);
void GetCreateStatementInfo(CreateStatement* stmt, uint num_indent);
void GetExpressionInfo(const expression::AbstractExpression* expr, uint num_indent);

} // End parser namespace
} // End peloton namespace
