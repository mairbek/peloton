//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// create_test.cpp
//
// Identification: test/executor/create_test.cpp
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <cstdio>

#include "common/harness.h"
#include "common/logger.h"
#include "catalog/bootstrapper.h"
#include "catalog/catalog.h"
#include "planner/create_plan.h"
#include "executor/create_executor.h"

#include "gtest/gtest.h"

namespace peloton {
namespace test {

//===--------------------------------------------------------------------===//
// Catalog Tests
//===--------------------------------------------------------------------===//

class CreateTests : public PelotonTest {};

TEST_F(CreateTests, CreatingTable) {

  // Bootstrap
  auto catalog = catalog::Bootstrapper::bootstrap();
  catalog::Bootstrapper::global_catalog->CreateDatabase(DEFAULT_DB_NAME,
                                                        nullptr);

  // Insert a table first
  auto id_column = catalog::Column(
      VALUE_TYPE_INTEGER, GetTypeSize(VALUE_TYPE_INTEGER), "dept_id", true);
  auto name_column =
      catalog::Column(VALUE_TYPE_VARCHAR, 32, "dept_name", false);

  // Schema
  std::unique_ptr<catalog::Schema> table_schema(
      new catalog::Schema({id_column, name_column}));

  auto &txn_manager = concurrency::TransactionManagerFactory::GetInstance();
  auto txn = txn_manager.BeginTransaction();
  std::unique_ptr<executor::ExecutorContext> context(
      new executor::ExecutorContext(txn));

  // Create plans
  planner::CreatePlan node("department_table", std::move(table_schema),
                           CreateType::CREATE_TYPE_TABLE);

  // Create executer
  executor::CreateExecutor executor(&node, context.get());

  executor.Init();
  executor.Execute();

  txn_manager.CommitTransaction(txn);
  EXPECT_EQ(catalog::Bootstrapper::global_catalog->GetDatabaseWithName(
                                                       DEFAULT_DB_NAME)
                ->GetTableCount(),
            1);

  // free the database just created
  txn = txn_manager.BeginTransaction();
  catalog::Bootstrapper::global_catalog->DropDatabase(DEFAULT_DB_NAME, txn);
  txn_manager.CommitTransaction(txn);

  delete catalog;
}

}  // End test namespace
}  // End peloton namespace
