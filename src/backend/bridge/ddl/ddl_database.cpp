//===----------------------------------------------------------------------===//
//
//                         PelotonDB
//
// ddl_database.cpp
//
// Identification: src/backend/bridge/ddl/ddl_database.cpp
//
// Copyright (c) 2015, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "backend/bridge/ddl/ddl_database.h"
#include "backend/common/logger.h"
#include "backend/storage/database.h"

#include "nodes/parsenodes.h"
#include "commands/dbcommands.h"

namespace peloton {
namespace bridge {

//===--------------------------------------------------------------------===//
// Database DDL
//===--------------------------------------------------------------------===//

/**
 * @brief Execute the create db stmt.
 * @param the parse tree
 * @return true if we handled it correctly, false otherwise
 */
bool DDLDatabase::ExecCreatedbStmt(Node *parsetree) {
  CreatedbStmt *stmt = (CreatedbStmt *)parsetree;
  DDLDatabase::CreateDatabase(stmt->database_id);
  return true;
}

/**
 * @brief Execute the drop db stmt.
 * @param the parse tree
 * @return true if we handled it correctly, false otherwise
 */
bool DDLDatabase::ExecDropdbStmt(Node *parsetree) {
  DropdbStmt *stmt = (DropdbStmt *)parsetree;
  DDLDatabase::DropDatabase(stmt->database_id);
  return true;
}

/**
 * @brief Execute the vacuum stmt.
 * @param the parse tree
 * @return true if we handled it correctly, false otherwise
 */
bool DDLDatabase::ExecVacuumStmt(Node *parsetree, Peloton_Status *status) {
  VacuumStmt *vacuum = (VacuumStmt *)parsetree;
  std::string relation_name;

  if (vacuum->relation != NULL) relation_name = vacuum->relation->relname;

  // Get database oid
  oid_t database_oid = Bridge::GetCurrentDatabaseOid();

  // Get data table based on dabase oid and table name
  auto &manager = catalog::Manager::GetInstance();
  auto db = manager.GetDatabaseWithOid(database_oid);

  // Update every table and index
  if (relation_name.empty()) {
    db->UpdateStats(status, true);
  }
  // Otherwise, update the specific table
  else {
    oid_t relation_oid = (db->GetTableWithName(relation_name))->GetOid();
    db->UpdateStatsWithOid(status, relation_oid);
  }

  return true;
}

/**
 * @brief Create database.
 * @param database_oid database id
 * @return true if we created a database, false otherwise
 */
bool DDLDatabase::CreateDatabase(Oid database_oid) {
  if (database_oid == INVALID_OID) return false;

  auto &manager = catalog::Manager::GetInstance();
  auto database = manager.GetDatabaseWithOid(database_oid);

  if (database == nullptr) {
    storage::Database *db = new storage::Database(database_oid);
    manager.AddDatabase(db);
  } else {
    LOG_WARN("Database(%u) is already existed!!", database_oid);
    return false;
  }

  elog(LOG, "Create database (%u)", database_oid);
  return true;
}

/**
 * @brief Drop database.
 * @param database_oid database id.
 * @return true if we dropped the database, false otherwise
 */
bool DDLDatabase::DropDatabase(Oid database_oid) {
  auto &manager = catalog::Manager::GetInstance();
  manager.DropDatabaseWithOid(database_oid);

  LOG_INFO("Dropped database with oid : %u\n", database_oid);
  return true;
}

}  // namespace bridge
}  // namespace peloton
