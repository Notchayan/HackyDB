#include <string>
#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

#include "../SQL_Statements/sql_statement.hpp"

// Forward declarations of classes
class Database;
class Table;
class Attribute;
class Index;
class SQLCreateTable;
class SQLDropTable;
class SQLDropIndex;

// Manages the catalog of databases
class CatalogManager {
private:
    friend class boost::serialization::access;

    // Serialization function for saving/loading the object
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &dbs_;
    }

    std::string path_; // Path to the catalog file
    std::vector<Database> dbs_; // List of databases

public:
    CatalogManager(std::string p);
    ~CatalogManager();

    // Accessor for databases
    std::vector<Database> &dbs() { return dbs_; }

    // Accessor for the catalog path
    std::string path() { return path_; }

    // Retrieve a database by name
    Database *GetDB(std::string db_name);

    // Read catalog data from a file
    void ReadArchiveFile();

    // Write catalog data to a file
    void WriteArchiveFile();

    // Create a new database
    void CreateDatabase(std::string dbname);

    // Delete an existing database
    void DeleteDatabase(std::string dbname);
};

// Represents a database
class Database {
private:
    friend class boost::serialization::access;

    // Serialization function for saving/loading the object
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &db_name_;
        ar &tbs_;
    }

    std::string db_name_; // Name of the database
    std::vector<Table> tbs_; // List of tables in the database

public:
    Database() {}
    Database(std::string dbname);
    ~Database() {}

    // Retrieve a table by name
    Table *GetTable(std::string tb_name);

    // Accessor for the database name
    std::string db_name() { return db_name_; }

    // Create a new table
    void CreateTable(SQLCreateTable &st);

    // Drop an existing table
    void DropTable(SQLDropTable &st);

    // Drop an index from the database
    void DropIndex(SQLDropIndex &st);

    // Accessor for the list of tables
    std::vector<Table> &tbs() { return tbs_; }

    // Check if an index exists in the database
    bool CheckIfIndexExists(std::string index_name);
};

// Represents a table in a database
class Table {
private:
    friend class boost::serialization::access;

    // Serialization function for saving/loading the object
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &tb_name_;
        ar &record_length_;
        ar &first_block_num_;
        ar &first_rubbish_num_;
        ar &block_count_;
        ar &ats_;
        ar &ids_;
    }

    std::string tb_name_; // Name of the table
    int record_length_; // Length of a record in the table

    int first_block_num_; // First block number in the table
    int first_rubbish_num_; // First rubbish block number
    int block_count_; // Number of blocks in the table

    std::vector<Attribute> ats_; // List of attributes in the table
    std::vector<Index> ids_; // List of indexes in the table

public:
    Table()
            : tb_name_(""), record_length_(-1), first_block_num_(-1),
                first_rubbish_num_(-1), block_count_(0) {}
    ~Table() {}

    // Accessor and mutator for the table name
    std::string tb_name() { return tb_name_; }
    void set_tb_name(std::string tbname) { tb_name_ = tbname; }

    // Accessor and mutator for the record length
    int record_length() { return record_length_; }
    void set_record_length(int len) { record_length_ = len; }

    // Accessor for the list of attributes
    std::vector<Attribute> &ats() { return ats_; }

    // Retrieve an attribute by name
    Attribute *GetAttribute(std::string name);

    // Get the index of an attribute by name
    int GetAttributeIndex(std::string name);

    // Accessor and mutator for the first block number
    int first_block_num() { return first_block_num_; }
    void set_first_block_num(int num) { first_block_num_ = num; }

    // Accessor and mutator for the first rubbish block number
    int first_rubbish_num() { return first_rubbish_num_; }
    void set_first_rubbish_num(int num) { first_rubbish_num_ = num; }

    // Accessor for the block count
    int block_count() { return block_count_; }

    // Get the number of attributes in the table
    unsigned long GetAttributeNum() { return ats_.size(); }

    // Add a new attribute to the table
    void AddAttribute(Attribute &attr) { ats_.push_back(attr); }

    // Increment the block count
    void IncreaseBlockCount() { block_count_++; }

    // Accessor for the list of indexes
    std::vector<Index> &ids() { return ids_; }

    // Retrieve an index by its position
    Index *GetIndex(int num) { return &(ids_[num]); }

    // Get the number of indexes in the table
    unsigned long GetIndexNum() { return ids_.size(); }

    // Add a new index to the table
    void AddIndex(Index &idx) { ids_.push_back(idx); }
};

// Represents an attribute in a table
class Attribute {
private:
    friend class boost::serialization::access;

    // Serialization function for saving/loading the object
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &attr_name_;
        ar &data_type_;
        ar &length_;
        ar &attr_type_;
    }

    std::string attr_name_; // Name of the attribute
    int data_type_; // Data type of the attribute
    int length_; // Length of the attribute
    int attr_type_; // Type of the attribute (e.g., primary key, foreign key)

public:
    Attribute() : attr_name_(""), data_type_(-1), length_(-1), attr_type_(0) {}
    ~Attribute() {}

    // Accessor and mutator for the attribute name
    std::string attr_name() { return attr_name_; }
    void set_attr_name(std::string name) { attr_name_ = name; }

    // Accessor and mutator for the attribute type
    int attr_type() { return attr_type_; }
    void set_attr_type(int type) { attr_type_ = type; }

    // Accessor and mutator for the data type
    int data_type() { return data_type_; }
    void set_data_type(int type) { data_type_ = type; }

    // Accessor and mutator for the length
    void set_length(int length) { length_ = length; }
    int length() { return length_; }
};

// Represents an index in a table
class Index {
private:
    friend class boost::serialization::access;

    // Serialization function for saving/loading the object
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &max_count_;
        ar &attr_name_;
        ar &name_;
        ar &key_len_;
        ar &key_type_;
        ar &rank_;
        ar &rubbish_;
        ar &root_;
        ar &leaf_head_;
        ar &key_count_;
        ar &level_;
        ar &node_count_;
    }

    int max_count_; // Maximum count of keys
    int key_len_; // Length of the key
    int key_type_; // Type of the key
    int rank_; // Rank of the index
    int rubbish_; // Rubbish count
    int root_; // Root node of the index
    int leaf_head_; // Head of the leaf nodes
    int key_count_; // Number of keys
    int level_; // Level of the index
    int node_count_; // Number of nodes
    std::string attr_name_; // Name of the attribute the index is on
    std::string name_; // Name of the index

public:
    Index() {}
    Index(std::string name, std::string attr_name, int keytype, int keylen,
                int rank) {
        attr_name_ = attr_name;
        name_ = name;
        key_count_ = 0;
        level_ = -1;
        node_count_ = 0;
        root_ = -1;
        leaf_head_ = -1;
        key_type_ = keytype;
        key_len_ = keylen;
        rank_ = rank;
        rubbish_ = -1;
        max_count_ = 0;
    }

    // Accessor for the attribute name
    std::string attr_name() { return attr_name_; }

    // Accessor for the key length
    int key_len() { return key_len_; }

    // Accessor for the key type
    int key_type() { return key_type_; }

    // Accessor for the rank
    int rank() { return rank_; }

    // Accessor and mutator for the root node
    int root() { return root_; }
    void set_root(int root) { root_ = root; }

    // Accessor and mutator for the leaf head
    int leaf_head() { return leaf_head_; }
    void set_leaf_head(int leaf_head) { leaf_head_ = leaf_head; }

    // Accessor and mutator for the key count
    int key_count() { return key_count_; }
    void set_key_count(int key_count) { key_count_ = key_count; }

    // Accessor and mutator for the level
    int level() { return level_; }
    void set_level(int level) { level_ = level; }

    // Accessor and mutator for the node count
    int node_count() { return node_count_; }
    void set_node_count(int node_count) { node_count_ = node_count; }

    // Accessor for the index name
    std::string name() { return name_; }

    // Increment functions for various properties
    int IncreaseMaxCount() { return max_count_++; }
    int IncreaseKeyCount() { return key_count_++; }
    int IncreaseNodeCount() { return node_count_++; }
    int IncreaseLevel() { return level_++; }

    // Decrement functions for various properties
    int DecreaseKeyCount() { return key_count_--; }
    int DecreaseNodeCount() { return node_count_--; }
    int DecreaseLevel() { return level_--; }
};
