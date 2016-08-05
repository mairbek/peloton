//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// index.cpp
//
// Identification: src/index/index.cpp
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//


#include "index/index.h"
#include "common/exception.h"
#include "common/logger.h"
#include "common/pool.h"
#include "catalog/schema.h"
#include "catalog/manager.h"
#include "storage/tuple.h"

#include <iostream>

namespace peloton {
namespace index {

Index::~Index() {
  // clean up metadata
  delete metadata;

  // clean up pool
  delete pool;
}

IndexMetadata::IndexMetadata(std::string index_name,
                             oid_t index_oid,
                             IndexType method_type,
                             IndexConstraintType index_type,
                             const catalog::Schema *tuple_schema,
                             const catalog::Schema *key_schema,
                             const std::vector<oid_t>& key_attrs,
                             bool unique_keys)
: index_name(index_name),
  index_oid(index_oid),
  method_type(method_type),
  index_type(index_type),
  tuple_schema(tuple_schema),
  key_schema(key_schema),
  key_attrs(key_attrs),
  unique_keys(unique_keys),
  ints_only(false) {

  // Determine if all the key schema attributes are of type INTEGER
  // This is used for specializing the indexes
  ints_only = true;
  auto key_schema_length = key_schema->GetColumnCount();
  for(oid_t key_schema_itr = 0;
      key_schema_itr < key_schema_length;
      key_schema_itr++){
    auto key_schema_column = key_schema->GetColumn(key_schema_itr);
    auto key_schema_column_type = key_schema_column.GetType();

    // Check key_schema_column_type
    if(key_schema_column_type != VALUE_TYPE_INTEGER){
      ints_only = false;
    }
  }


}

IndexMetadata::~IndexMetadata() {
  // clean up key schema
  delete key_schema;
  // no need to clean the tuple schema
}

oid_t IndexMetadata::GetColumnCount() const {
  return GetKeySchema()->GetColumnCount();
}

const std::string IndexMetadata::GetInfo() const {
  std::stringstream os;

  os << "\tINDEX METADATA: [";

  for(auto key_attr : key_attrs){
    os << key_attr << " ";
  }

  os << " ] :: ";

  os << utility_ratio;

  return os.str();
}

/*
 * Compare() - Check whether a given index key satisfies a predicate
 *
 * The predicate has the same specification as those in Scan()
 */
bool Index::Compare(const AbstractTuple &index_key,
                    const std::vector<oid_t> &key_column_ids,
                    const std::vector<ExpressionType> &expr_types,
                    const std::vector<Value> &values) {
  int diff;

  oid_t key_column_itr = -1;
  // Go over each attribute in the list of comparison columns
  // The key_columns_ids, as the name shows, saves the key column ids that
  // have values and expression needs to be compared.

  // Example:
  // 1.
  //    key_column_ids { 0 }
  //    expr_types { == }
  //    values    { 5 }
  // basically it's saying get the tuple whose 0 column, which is the key
  // column,
  //  equals to 5
  //
  // 2.
  //   key_column_ids {0, 1}
  //   expr_types { > , >= }
  //  values  {5, 10}
  // it's saysing col[0] > 5 && col[1] >= 10, where 0 and 1 are key columns.

  for (auto column_itr : key_column_ids) {
    key_column_itr++;
    const Value &rhs = values[key_column_itr];
    const Value &lhs = index_key.GetValue(column_itr);
    const ExpressionType expr_type = expr_types[key_column_itr];

    if (expr_type == EXPRESSION_TYPE_COMPARE_IN) {
      bool bret = lhs.InList(rhs);
      if (bret == true) {
        diff = VALUE_COMPARE_EQUAL;
      } else {
        diff = VALUE_COMPARE_NO_EQUAL;
      }
    } else {
      diff = lhs.Compare(rhs);
    }

    LOG_TRACE("Difference : %d ", diff);

    if (diff == VALUE_COMPARE_EQUAL) {
      switch (expr_type) {
        case EXPRESSION_TYPE_COMPARE_EQUAL:
        case EXPRESSION_TYPE_COMPARE_LESSTHANOREQUALTO:
        case EXPRESSION_TYPE_COMPARE_GREATERTHANOREQUALTO:
        case EXPRESSION_TYPE_COMPARE_IN:
          continue;

        case EXPRESSION_TYPE_COMPARE_NOTEQUAL:
        case EXPRESSION_TYPE_COMPARE_LESSTHAN:
        case EXPRESSION_TYPE_COMPARE_GREATERTHAN:
          return false;

        default:
          throw IndexException("Unsupported expression type : " +
                               std::to_string(expr_type));
      }
    } else if (diff == VALUE_COMPARE_LESSTHAN) {
      switch (expr_type) {
        case EXPRESSION_TYPE_COMPARE_NOTEQUAL:
        case EXPRESSION_TYPE_COMPARE_LESSTHAN:
        case EXPRESSION_TYPE_COMPARE_LESSTHANOREQUALTO:
          continue;

        case EXPRESSION_TYPE_COMPARE_EQUAL:
        case EXPRESSION_TYPE_COMPARE_GREATERTHAN:
        case EXPRESSION_TYPE_COMPARE_GREATERTHANOREQUALTO:
        case EXPRESSION_TYPE_COMPARE_IN:
          return false;

        default:
          throw IndexException("Unsupported expression type : " +
                               std::to_string(expr_type));
      }
    } else if (diff == VALUE_COMPARE_GREATERTHAN) {
      switch (expr_type) {
        case EXPRESSION_TYPE_COMPARE_NOTEQUAL:
        case EXPRESSION_TYPE_COMPARE_GREATERTHAN:
        case EXPRESSION_TYPE_COMPARE_GREATERTHANOREQUALTO:
          continue;

        case EXPRESSION_TYPE_COMPARE_EQUAL:
        case EXPRESSION_TYPE_COMPARE_LESSTHAN:
        case EXPRESSION_TYPE_COMPARE_LESSTHANOREQUALTO:
        case EXPRESSION_TYPE_COMPARE_IN:
          return false;

        default:
          throw IndexException("Unsupported expression type : " +
                               std::to_string(expr_type));
      }
    } else if (diff == VALUE_COMPARE_NO_EQUAL) {
      // problems here when there are multiple
      // conditions with OR in the query
      return false;
    }
  }

  return true;
}

/*
 * ConstructLowerBoundTuple() - Constructs a lower bound of index key that
 *                              satisfies a given tuple
 *
 * The predicate has the same specification as those in Scan()
 * This function works even if there are multiple predicates on a single
 * column, e.g. both "<" and ">" could be applied to the same column. Even
 * in this case this function correctly identifies the lower bound, though not
 * necessarily be a tight lower bound.
 *
 * Note that this function logically is more proper to be in index_util than
 * in here. But it must call the varlen pool which makes moving out to
 * index_util impossible.
 */
bool Index::ConstructLowerBoundTuple(
    storage::Tuple *index_key,
    const std::vector<peloton::Value> &values,
    const std::vector<oid_t> &key_column_ids,
    const std::vector<ExpressionType> &expr_types) {

  auto schema = index_key->GetSchema();
  auto col_count = schema->GetColumnCount();
  bool all_constraints_equal = true;

  // Go over each column in the key tuple
  // Setting either the placeholder or the min value
  for (oid_t column_itr = 0; column_itr < col_count; column_itr++) {

    // If the current column of the key has a predicate item
    // specified in the key column list
    auto key_column_itr =
        std::find(key_column_ids.begin(), key_column_ids.end(), column_itr);

    bool placeholder = false;
    Value value;

    // This column is part of the key column ids
    if (key_column_itr != key_column_ids.end()) {

      // This is the index into value list and expression type list
      auto offset = std::distance(key_column_ids.begin(), key_column_itr);

      // If there is an "==" for the current column then we could fix the value
      // for index key
      // otherwise we know not all predicate items are "==", i.e. this is not
      // a point query and potentially requires an index scan
      if (expr_types[offset] == EXPRESSION_TYPE_COMPARE_EQUAL) {
        placeholder = true;

        // This is the value object that will be filled into the index key
        value = values[offset];
      } else {
        all_constraints_equal = false;
      }
    }

    LOG_TRACE("Column itr : %u  Placeholder : %d ", column_itr, placeholder);

    // If the value is available then just fill in the value for the
    // current "==" relation
    // Otherwise if there is not a value then we could only fill the
    // min possible value of the current column's type
    if (placeholder == true) {
      index_key->SetValue(column_itr, value, GetPool());
    } else {
      auto value_type = schema->GetType(column_itr);

      index_key->SetValue(column_itr,
                          Value::GetMinValue(value_type),
                          GetPool());
    }
  } // for all columns in index key

  LOG_TRACE("Lower Bound Tuple :: %s", index_key->GetInfo().c_str());

  // Corner case: If not all columns have a "==" relation then still
  // this is not a point query though all existing predicate items
  // are "=="
  if (col_count > values.size()) all_constraints_equal = false;

  return all_constraints_equal;
}

Index::Index(IndexMetadata *metadata) :
        metadata(metadata),
        indexed_tile_group_offset_(0) {
  index_oid = metadata->GetOid();
  // initialize counters
  lookup_counter = insert_counter = delete_counter = update_counter = 0;

  // initialize pool
  pool = new VarlenPool(BACKEND_TYPE_MM);
}

const std::string Index::GetInfo() const {
  std::stringstream os;

  os << "\t-----------------------------------------------------------\n";

  os << "\tINDEX\n";

  os << GetTypeName() << "\t(" << GetName() << ")";
  os << (HasUniqueKeys() ? " UNIQUE " : " NON-UNIQUE") << "\n";

  os << "\tValue schema : " << *(GetKeySchema());

  os << "\t-----------------------------------------------------------\n";

  return os.str();
}

/**
 * @brief Increase the number of tuples in this table
 * @param amount amount to increase
 */
void Index::IncreaseNumberOfTuplesBy(const size_t amount) {
  number_of_tuples += amount;
  dirty = true;
}

/**
 * @brief Decrease the number of tuples in this table
 * @param amount amount to decrease
 */
void Index::DecreaseNumberOfTuplesBy(const size_t amount) {
  number_of_tuples -= amount;
  dirty = true;
}

/**
 * @brief Set the number of tuples in this table
 * @param num_tuples number of tuples
 */
void Index::SetNumberOfTuples(const size_t num_tuples) {
  number_of_tuples = num_tuples;
  dirty = true;
}

/**
 * @brief Get the number of tuples in this table
 * @return number of tuples
 */
size_t Index::GetNumberOfTuples() const { return number_of_tuples; }

/**
 * @brief return dirty flag
 * @return dirty flag
 */
bool Index::IsDirty() const { return dirty; }

/**
 * @brief Reset dirty flag
 */
void Index::ResetDirty() { dirty = false; }

}  // End index namespace
}  // End peloton namespace
