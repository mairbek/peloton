/*-------------------------------------------------------------------------
 *
 * bridge_test.h
 * file description
 *
 * Copyright(c) 2015, CMU
 *
 * /peloton/tests/bridge/bridge_test.h
 *
 *-------------------------------------------------------------------------
 */

#pragma once

#include <vector>
#include <string>

#include <backend/common/types.h>

namespace peloton {

namespace catalog {
class Column;
}

namespace test {

//===--------------------------------------------------------------------===//
// Bridge Tests
//===--------------------------------------------------------------------===//

class BridgeTest {

 public:
  BridgeTest(const BridgeTest &) = delete;
  BridgeTest& operator=(const BridgeTest &) = delete;
  BridgeTest(BridgeTest &&) = delete;
  BridgeTest& operator=(BridgeTest &&) = delete;

  static void RunTests();

 private:

  //===--------------------------------------------------------------------===//
  // Tests
  //===--------------------------------------------------------------------===//

  static void DDL_CreateTable_TEST();

  static void DDL_CreateTable_TEST_INVALID_OID();

  static void DDL_CreateTable_TEST_COLUMNS();

  static void DDL_CreateTable_TEST_COLUMN_CONSTRAINTS();

  //===--------------------------------------------------------------------===//
  // Utilities
  //===--------------------------------------------------------------------===//

  static std::vector<catalog::Column> CreateSimpleColumns();

  static bool CheckColumn(catalog::Column& column,
                          std::string column_name, 
                          int length, 
                          ValueType type);



};

} // End test namespace
} // End peloton namespace
