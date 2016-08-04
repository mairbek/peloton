//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// index_test.cpp
//
// Identification: test/index/index_test.cpp
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//


#include "gtest/gtest.h"
#include "common/harness.h"

#include "common/logger.h"
#include "common/platform.h"
#include "index/index_factory.h"
#include "storage/tuple.h"

namespace peloton {
namespace test {

//===--------------------------------------------------------------------===//
// Index Tests
//===--------------------------------------------------------------------===//

class IndexTests : public PelotonTest {};

catalog::Schema *key_schema = nullptr;
catalog::Schema *tuple_schema = nullptr;

ItemPointer item0(120, 5);
ItemPointer item1(120, 7);
ItemPointer item2(123, 19);

// Since we need index type to determine the result
// of the test, this needs to be made as a global static
static IndexType index_type = INDEX_TYPE_BWTREE;

// Uncomment this to enable BwTree as index being tested
//static IndexType index_type = INDEX_TYPE_BWTREE;

index::Index *BuildIndex(const bool unique_keys) {
  // Build tuple and key schema
  std::vector<std::vector<std::string>> column_names;
  std::vector<catalog::Column> columns;
  std::vector<catalog::Schema *> schemas;

  catalog::Column column1(VALUE_TYPE_INTEGER, GetTypeSize(VALUE_TYPE_INTEGER),
                          "A", true);
  catalog::Column column2(VALUE_TYPE_VARCHAR, 1024, "B", false);
  catalog::Column column3(VALUE_TYPE_DOUBLE, GetTypeSize(VALUE_TYPE_DOUBLE),
                          "C", true);
  catalog::Column column4(VALUE_TYPE_INTEGER, GetTypeSize(VALUE_TYPE_INTEGER),
                          "D", true);

  columns.push_back(column1);
  columns.push_back(column2);

  // INDEX KEY SCHEMA -- {column1, column2}
  std::vector<oid_t> key_attrs = {0, 1};
  key_schema = new catalog::Schema(columns);
  key_schema->SetIndexedColumns(key_attrs);

  columns.push_back(column3);
  columns.push_back(column4);

  // TABLE SCHEMA -- {column1, column2, column3, column4}
  tuple_schema = new catalog::Schema(columns);

  // Build index metadata
  index::IndexMetadata *index_metadata = new index::IndexMetadata(
      "test_index", 125, index_type, INDEX_CONSTRAINT_TYPE_DEFAULT,
      tuple_schema, key_schema, key_attrs, unique_keys);

  // Build index
  index::Index *index = index::IndexFactory::GetInstance(index_metadata);
  EXPECT_TRUE(index != NULL);

  return index;
}

TEST_F(IndexTests, BasicTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex(false));

  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));

  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);

  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);

  // INSERT
  index->InsertEntry(key0.get(), item0);

  index->ScanKey(key0.get(), locations);
  EXPECT_EQ(locations.size(), 1);
  EXPECT_EQ(locations[0].block, item0.block);
  locations.clear();

  // DELETE
  index->DeleteEntry(key0.get(), item0);

  index->ScanKey(key0.get(), locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  delete tuple_schema;
}

// INSERT HELPER FUNCTION
void InsertTest(index::Index *index, VarlenPool *pool, size_t scale_factor,
                UNUSED_ATTRIBUTE uint64_t thread_itr) {
  // Loop based on scale factor
  for (size_t scale_itr = 1; scale_itr <= scale_factor; scale_itr++) {
    // Insert a bunch of keys based on scale itr
    std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key3(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key4(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> keynonce(
        new storage::Tuple(key_schema, true));

    key0->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
    key1->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
    key2->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);
    key3->SetValue(0, ValueFactory::GetIntegerValue(400 * scale_itr), pool);
    key3->SetValue(1, ValueFactory::GetStringValue("d"), pool);
    key4->SetValue(0, ValueFactory::GetIntegerValue(500 * scale_itr), pool);
    key4->SetValue(1, ValueFactory::GetStringValue(
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"),
                   pool);
    keynonce->SetValue(0, ValueFactory::GetIntegerValue(1000 * scale_itr),
                       pool);
    keynonce->SetValue(1, ValueFactory::GetStringValue("f"), pool);

    // INSERT
    // key0 1 (100, a)   item0
    // key1 5  (100, b)  item1 2 1 1 0
    // key2 1 (100, c) item 1
    // key3 1 (400, d) item 1
    // key4 1  (500, eeeeee...) item 1
    // no keyonce (1000, f)

    // item0 = 2
    // item1 = 6
    // item2 = 1
    index->InsertEntry(key0.get(), item0);
    index->InsertEntry(key1.get(), item1);
    index->InsertEntry(key1.get(), item2);
    index->InsertEntry(key1.get(), item1);
    index->InsertEntry(key1.get(), item1);
    index->InsertEntry(key1.get(), item0);

    index->InsertEntry(key2.get(), item1);
    index->InsertEntry(key3.get(), item1);
    index->InsertEntry(key4.get(), item1);
  }
}

// DELETE HELPER FUNCTION
void DeleteTest(index::Index *index, VarlenPool *pool, size_t scale_factor,
                UNUSED_ATTRIBUTE uint64_t thread_itr) {
  // Loop based on scale factor
  for (size_t scale_itr = 1; scale_itr <= scale_factor; scale_itr++) {
    // Delete a bunch of keys based on scale itr
    std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key3(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key4(new storage::Tuple(key_schema, true));

    key0->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
    key1->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
    key2->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);
    key3->SetValue(0, ValueFactory::GetIntegerValue(400 * scale_itr), pool);
    key3->SetValue(1, ValueFactory::GetStringValue("d"), pool);
    key4->SetValue(0, ValueFactory::GetIntegerValue(500 * scale_itr), pool);
    key4->SetValue(1, ValueFactory::GetStringValue(
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"),
                   pool);

    // DELETE
    // key0 1 (100, a)   item0
    // key1 5  (100, b)  item 0 1 2 (0 1 1 1 2)
    // key2 1 (100, c) item 1
    // key3 1 (400, d) item 1
    // key4 1  (500, eeeeee...) item 1
    // no keyonce (1000, f)

    // item0 = 2
    // item1 = 6
    // item2 = 1
    index->DeleteEntry(key0.get(), item0);
    index->DeleteEntry(key1.get(), item1);
    index->DeleteEntry(key2.get(), item2);
    index->DeleteEntry(key3.get(), item1);
    index->DeleteEntry(key4.get(), item1);

    // should be no key0
    // key1 item 0 1 2
    // key2 item 1
    // no key3
    // no key4
  }
}

TEST_F(IndexTests, MultiMapInsertTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex(false));

  // Single threaded test
  size_t scale_factor = 1;
  LaunchParallelTest(1, InsertTest, index.get(), pool, scale_factor);

  // Checks
  index->ScanAllKeys(locations);
  
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 7);
  } else {
    EXPECT_EQ(locations.size(), 9);
  }


  locations.clear();

  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> keynonce(
      new storage::Tuple(key_schema, true));
  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
  keynonce->SetValue(0, ValueFactory::GetIntegerValue(1000), pool);
  keynonce->SetValue(1, ValueFactory::GetStringValue("f"), pool);

  index->ScanKey(keynonce.get(), locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  index->ScanKey(key0.get(), locations);
  EXPECT_EQ(locations.size(), 1);
  EXPECT_EQ(locations[0].block, item0.block);
  locations.clear();

  delete tuple_schema;
}

#ifdef ALLOW_UNIQUE_KEY
TEST_F(IndexTests, UniqueKeyDeleteTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex(true));

  // Single threaded test
  size_t scale_factor = 1;
  LaunchParallelTest(1, InsertTest, index.get(), pool, scale_factor);
  LaunchParallelTest(1, DeleteTest, index.get(), pool, scale_factor);

  // Checks
  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));

  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
  key1->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
  key2->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);

  index->ScanKey(key0.get(), locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  index->ScanKey(key1.get(), locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  index->ScanKey(key2.get(), locations);
  EXPECT_EQ(locations.size(), 1);
  EXPECT_EQ(locations[0].block, item1.block);
  locations.clear();

  delete tuple_schema;
}
#endif

TEST_F(IndexTests, NonUniqueKeyDeleteTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex(false));

  // Single threaded test
  size_t scale_factor = 1;
  LaunchParallelTest(1, InsertTest, index.get(), pool, scale_factor);
  LaunchParallelTest(1, DeleteTest, index.get(), pool, scale_factor);

  // Checks
  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));

  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
  key1->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
  key2->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);

  index->ScanKey(key0.get(), locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  index->ScanKey(key1.get(), locations);
  EXPECT_EQ(locations.size(), 2);
  locations.clear();

  index->ScanKey(key2.get(), locations);
  EXPECT_EQ(locations.size(), 1);
  EXPECT_EQ(locations[0].block, item1.block);
  locations.clear();

  delete tuple_schema;
}

TEST_F(IndexTests, MultiThreadedInsertTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex(false));

  // Parallel Test
  size_t num_threads = 4;
  size_t scale_factor = 1;
  LaunchParallelTest(num_threads, InsertTest, index.get(), pool, scale_factor);

  index->ScanAllKeys(locations);
  
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 7);
  } else {
    EXPECT_EQ(locations.size(), 9 * num_threads);
  }
  
  locations.clear();

  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> keynonce(
      new storage::Tuple(key_schema, true));

  keynonce->SetValue(0, ValueFactory::GetIntegerValue(1000), pool);
  keynonce->SetValue(1, ValueFactory::GetStringValue("f"), pool);

  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);

  index->ScanKey(keynonce.get(), locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  index->ScanKey(key0.get(), locations);
  
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 1);
  } else {
    EXPECT_EQ(locations.size(), num_threads);
  }
  
  EXPECT_EQ(locations[0].block, item0.block);
  locations.clear();

  delete tuple_schema;
}

#ifdef ALLOW_UNIQUE_KEY
TEST_F(IndexTests, UniqueKeyMultiThreadedTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex(true));

  // Parallel Test
  size_t num_threads = 4;
  size_t scale_factor = 1;
  LaunchParallelTest(num_threads, InsertTest, index.get(), pool, scale_factor);
  LaunchParallelTest(num_threads, DeleteTest, index.get(), pool, scale_factor);

  // Checks
  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));

  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
  key1->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
  key2->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);

  index->ScanKey(key0.get(), locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  index->ScanKey(key1.get(), locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  index->ScanKey(key2.get(), locations);
  EXPECT_EQ(locations.size(), 1);
  EXPECT_EQ(locations[0].block, item1.block);
  locations.clear();

  index->ScanAllKeys(locations);
  EXPECT_EQ(locations.size(), 1);
  locations.clear();

  // FORWARD SCAN
  index->Scan({key1->GetValue(0)}, {0}, {EXPRESSION_TYPE_COMPARE_EQUAL},
              SCAN_DIRECTION_TYPE_FORWARD, locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  index->Scan({key1->GetValue(0), key1->GetValue(1)}, {0, 1},
              {EXPRESSION_TYPE_COMPARE_EQUAL, EXPRESSION_TYPE_COMPARE_EQUAL},
              SCAN_DIRECTION_TYPE_FORWARD, locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  index->Scan(
      {key1->GetValue(0), key1->GetValue(1)}, {0, 1},
      {EXPRESSION_TYPE_COMPARE_EQUAL, EXPRESSION_TYPE_COMPARE_GREATERTHAN},
      SCAN_DIRECTION_TYPE_FORWARD, locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  index->Scan(
      {key1->GetValue(0), key1->GetValue(1)}, {0, 1},
      {EXPRESSION_TYPE_COMPARE_GREATERTHAN, EXPRESSION_TYPE_COMPARE_EQUAL},
      SCAN_DIRECTION_TYPE_FORWARD, locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  delete tuple_schema;
}
#endif

// key0 1 (100, a)   item0
// key1 5  (100, b)  item1 2 1 1 0
// key2 1 (100, c) item 1
// key3 1 (400, d) item 1
// key4 1  (500, eeeeee...) item 1
// no keyonce (1000, f)

// item0 = 2
// item1 = 6
// item2 = 1

// should be no key0
// key1 item 0 2
// key2 item 1
// no key3
// no key4

TEST_F(IndexTests, NonUniqueKeyMultiThreadedTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex(false));

  // Parallel Test
  size_t num_threads = 4;
  size_t scale_factor = 1;
  LaunchParallelTest(num_threads, InsertTest, index.get(), pool, scale_factor);
  LaunchParallelTest(num_threads, DeleteTest, index.get(), pool, scale_factor);

  // Checks
  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key4(new storage::Tuple(key_schema, true));

  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
  key1->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
  key2->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);
  key4->SetValue(0, ValueFactory::GetIntegerValue(500), pool);
  key4->SetValue(1, ValueFactory::GetStringValue(
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"),
                 pool);

  index->ScanKey(key0.get(), locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  index->ScanKey(key1.get(), locations);

  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 2);
  } else {
    EXPECT_EQ(locations.size(), 2 * num_threads);
  }
  
  locations.clear();

  index->ScanKey(key2.get(), locations);
  
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 1);
  } else {
    EXPECT_EQ(locations.size(), 1 * num_threads);
  }
  
  EXPECT_EQ(locations[0].block, item1.block);
  locations.clear();

  index->ScanAllKeys(locations);
  
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 3);
  } else {
    EXPECT_EQ(locations.size(), 3 * num_threads);
  }
  
  locations.clear();

  // FORWARD SCAN
  index->Scan({key1->GetValue(0)}, {0}, {EXPRESSION_TYPE_COMPARE_EQUAL},
              SCAN_DIRECTION_TYPE_FORWARD, locations);
              
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 3);
  } else {
    EXPECT_EQ(locations.size(), 3 * num_threads);
  }
  
  locations.clear();

  index->Scan({key1->GetValue(0), key1->GetValue(1)}, {0, 1},
              {EXPRESSION_TYPE_COMPARE_EQUAL, EXPRESSION_TYPE_COMPARE_EQUAL},
              SCAN_DIRECTION_TYPE_FORWARD, locations);
              
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 2);
  } else {
    EXPECT_EQ(locations.size(), 2 * num_threads);
  }
  
  locations.clear();

  index->Scan(
      {key1->GetValue(0), key1->GetValue(1)}, {0, 1},
      {EXPRESSION_TYPE_COMPARE_EQUAL, EXPRESSION_TYPE_COMPARE_GREATERTHAN},
      SCAN_DIRECTION_TYPE_FORWARD, locations);
      
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 1);
  } else {
    EXPECT_EQ(locations.size(), 1 * num_threads);
  }
  
  locations.clear();

  index->Scan(
      {key1->GetValue(0), key1->GetValue(1)}, {0, 1},
      {EXPRESSION_TYPE_COMPARE_GREATERTHAN, EXPRESSION_TYPE_COMPARE_EQUAL},
      SCAN_DIRECTION_TYPE_FORWARD, locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  index->Scan({key2->GetValue(0), key2->GetValue(1)}, {0, 1},
              {EXPRESSION_TYPE_COMPARE_EQUAL, EXPRESSION_TYPE_COMPARE_LESSTHAN},
              SCAN_DIRECTION_TYPE_FORWARD, locations);
              
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 2);
  } else {
    EXPECT_EQ(locations.size(), 2 * num_threads);
  }
  
  locations.clear();

  index->Scan(
      {key0->GetValue(0), key0->GetValue(1), key2->GetValue(0),
       key2->GetValue(1)},
      {0, 1, 0, 1},
      {EXPRESSION_TYPE_COMPARE_EQUAL, EXPRESSION_TYPE_COMPARE_GREATERTHAN,
       EXPRESSION_TYPE_COMPARE_EQUAL, EXPRESSION_TYPE_COMPARE_LESSTHAN},
      SCAN_DIRECTION_TYPE_FORWARD, locations);
      
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 2);
  } else {
    EXPECT_EQ(locations.size(), 2 * num_threads);
  }
  
  locations.clear();

  index->Scan({key0->GetValue(0), key0->GetValue(1), key4->GetValue(0),
               key4->GetValue(1)},
              {0, 1, 0, 1}, {EXPRESSION_TYPE_COMPARE_GREATERTHANOREQUALTO,
                             EXPRESSION_TYPE_COMPARE_GREATERTHAN,
                             EXPRESSION_TYPE_COMPARE_LESSTHANOREQUALTO,
                             EXPRESSION_TYPE_COMPARE_LESSTHAN},
              SCAN_DIRECTION_TYPE_FORWARD, locations);
              
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 3);
  } else {
    EXPECT_EQ(locations.size(), 3 * num_threads);
  }
  
  locations.clear();

  // REVERSE SCAN
  index->Scan({key1->GetValue(0)}, {0}, {EXPRESSION_TYPE_COMPARE_EQUAL},
              SCAN_DIRECTION_TYPE_BACKWARD, locations);
              
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 3);
  } else {
    EXPECT_EQ(locations.size(), 3 * num_threads);
  }
  
  locations.clear();

  index->Scan({key1->GetValue(0), key1->GetValue(1)}, {0, 1},
              {EXPRESSION_TYPE_COMPARE_EQUAL, EXPRESSION_TYPE_COMPARE_EQUAL},
              SCAN_DIRECTION_TYPE_BACKWARD, locations);
              
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 2);
  } else {
    EXPECT_EQ(locations.size(), 2 * num_threads);
  }
  
  locations.clear();

  index->Scan(
      {key1->GetValue(0), key1->GetValue(1)}, {0, 1},
      {EXPRESSION_TYPE_COMPARE_EQUAL, EXPRESSION_TYPE_COMPARE_GREATERTHAN},
      SCAN_DIRECTION_TYPE_BACKWARD, locations);
      
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 1);
  } else {
    EXPECT_EQ(locations.size(), 1 * num_threads);
  }
  
  locations.clear();

  index->Scan(
      {key1->GetValue(0), key1->GetValue(1)}, {0, 1},
      {EXPRESSION_TYPE_COMPARE_GREATERTHAN, EXPRESSION_TYPE_COMPARE_EQUAL},
      SCAN_DIRECTION_TYPE_BACKWARD, locations);
      
  EXPECT_EQ(locations.size(), 0);
  
  locations.clear();

  index->Scan({key2->GetValue(0), key2->GetValue(1)}, {0, 1},
              {EXPRESSION_TYPE_COMPARE_EQUAL, EXPRESSION_TYPE_COMPARE_LESSTHAN},
              SCAN_DIRECTION_TYPE_BACKWARD, locations);
              
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 2);
  } else {
    EXPECT_EQ(locations.size(), 2 * num_threads);
  }
  
  locations.clear();

  index->Scan(
      {key0->GetValue(0), key0->GetValue(1), key2->GetValue(0),
       key2->GetValue(1)},
      {0, 1, 0, 1},
      {EXPRESSION_TYPE_COMPARE_EQUAL, EXPRESSION_TYPE_COMPARE_GREATERTHAN,
       EXPRESSION_TYPE_COMPARE_EQUAL, EXPRESSION_TYPE_COMPARE_LESSTHAN},
      SCAN_DIRECTION_TYPE_BACKWARD, locations);
      
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 2);
  } else {
    EXPECT_EQ(locations.size(), 2 * num_threads);
  }
  
  locations.clear();

  index->Scan({key0->GetValue(0), key0->GetValue(1), key4->GetValue(0),
               key4->GetValue(1)},
              {0, 1, 0, 1}, {EXPRESSION_TYPE_COMPARE_GREATERTHANOREQUALTO,
                             EXPRESSION_TYPE_COMPARE_GREATERTHAN,
                             EXPRESSION_TYPE_COMPARE_LESSTHANOREQUALTO,
                             EXPRESSION_TYPE_COMPARE_LESSTHAN},
              SCAN_DIRECTION_TYPE_BACKWARD, locations);
              
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 3);
  } else {
    EXPECT_EQ(locations.size(), 3 * num_threads);
  }
  
  locations.clear();

  delete tuple_schema;
}

TEST_F(IndexTests, NonUniqueKeyMultiThreadedStressTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex(false));

  // Parallel Test
  size_t num_threads = 4;
  size_t scale_factor = 3;
  
  LaunchParallelTest(num_threads, InsertTest, index.get(), pool, scale_factor);
  LaunchParallelTest(num_threads, DeleteTest, index.get(), pool, scale_factor);

  // Checks
  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));

  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
  key1->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
  key2->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);

  index->ScanKey(key0.get(), locations);
  EXPECT_EQ(locations.size(), 0);
  locations.clear();

  index->ScanKey(key1.get(), locations);
  
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 2);
  } else {
    EXPECT_EQ(locations.size(), 2 * num_threads);
  }
  
  locations.clear();

  index->ScanKey(key2.get(), locations);
  
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 1);
  } else {
    EXPECT_EQ(locations.size(), 1 * num_threads);
  }
  
  EXPECT_EQ(locations[0].block, item1.block);
  locations.clear();

  index->ScanAllKeys(locations);
  
  if(index_type == INDEX_TYPE_BWTREE) {
    EXPECT_EQ(locations.size(), 3 * scale_factor);
  } else {
    EXPECT_EQ(locations.size(), 3 * num_threads * scale_factor);
  }
  
  locations.clear();

  delete tuple_schema;
}

TEST_F(IndexTests, NonUniqueKeyMultiThreadedStressTest2) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex(false));

  // Parallel Test
  size_t num_threads = 15;
  size_t scale_factor = 3;
  LaunchParallelTest(num_threads, InsertTest, index.get(), pool, scale_factor);
  LaunchParallelTest(num_threads, DeleteTest, index.get(), pool, scale_factor);

  index->ScanAllKeys(locations);
  if (index->HasUniqueKeys()) {
    if(index_type == INDEX_TYPE_BWTREE) {
      EXPECT_EQ(locations.size(), scale_factor);
    } else {
      EXPECT_EQ(locations.size(), scale_factor);
    }
  }
  else {
    if(index_type == INDEX_TYPE_BWTREE) {
      EXPECT_EQ(locations.size(), 3 * scale_factor);
    } else {
      EXPECT_EQ(locations.size(), 3 * scale_factor * num_threads);
    }
  }
    
  locations.clear();


  std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));

  key1->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
  key2->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);

  index->ScanKey(key1.get(), locations);
  if (index->HasUniqueKeys()) {
    EXPECT_EQ(locations.size(), 0);
  } else {
    if(index_type == INDEX_TYPE_BWTREE) {
      EXPECT_EQ(locations.size(), 2);
    } else {
      EXPECT_EQ(locations.size(), 2 * num_threads);
    }
  }
  locations.clear();

  index->ScanKey(key2.get(), locations);
  if (index->HasUniqueKeys()) {
    EXPECT_EQ(locations.size(), num_threads);
  } else {
    if(index_type == INDEX_TYPE_BWTREE) {
      EXPECT_EQ(locations.size(), 1);
    } else {
      EXPECT_EQ(locations.size(), 1 * num_threads);
    }
  }
  
  locations.clear();

  delete tuple_schema;
}

}  // End test namespace
}  // End peloton namespace
