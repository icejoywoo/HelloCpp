/*
 * hello_world.cpp
 *
 *  Created on: 2013?10?11?
 *      Author: icejoywoo
 */

#include <cassert>

#include <iostream>
#include <sstream>
#include <string>

#include "leveldb/db.h"
#include "leveldb/write_batch.h"

bool CheckStatus(leveldb::Status& status) {
	if (!status.ok()) {
		std::cerr << status.ToString() << std::endl;
	}
	return status.ok();
}

int main(int argc, char* argv[]) {
	std::cout << "\033[31;1mleveldb\033[0m Demo" << std::endl;
	leveldb::DB* db;
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
	CheckStatus(status);

	// 读，写，删除
	std::string value = "TestValue1";
	leveldb::Status s = db->Put(leveldb::WriteOptions(), "key1", value);
	if (s.ok())
		s = db->Get(leveldb::ReadOptions(), "key1", &value);
	std::cout << "Value: " << value << std::endl;
	if (s.ok())
		s = db->Delete(leveldb::WriteOptions(), "key1");

	std::stringstream stream;
	for (int i = 0; i < 100; i++) {
		std::string key;
		stream << "key" << i;
		stream >> key;
		stream.clear();
		std::string value;
		stream << "value" << i;
		stream >> value;
		// clear重置状态位，清空
		stream.clear();
		stream.str("");
		leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
	}

	// scan range 扫描一个区间的key
	leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());

	// iterate all keys
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		std::cout << it->key().ToString() << ": " << it->value().ToString()
		        << std::endl;
	}

	for (it->Seek("key30"); it->Valid() && it->key().ToString() < "key40";
	        it->Next()) {
		std::cout << it->key().ToString() << ": " << it->value().ToString()
		        << std::endl;
	}

	// atomic updates
	s = db->Get(leveldb::ReadOptions(), "key1", &value);
	if (s.ok()) {
		leveldb::WriteBatch batch;
		batch.Delete("key1");
		batch.Put("key2", value);
		s = db->Write(leveldb::WriteOptions(), &batch);
	}
	s = db->Get(leveldb::ReadOptions(), "key2", &value);
	if (s.ok()) std::cout << "key2: " << value << std::endl;

	return 0;
}
