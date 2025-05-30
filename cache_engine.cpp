class Cache_Line {

    bool *ways;
    int *tags;
    int tag_size;

  public:
    // Constructors
    Cache_Line(int tag_size);

    // functions
    bool read_to_cline(int address);
    bool write_to_cline(int address);

    // r 0x000 00001
    //  ways[0]=true;
    // w 0x000 00003
    //  ways[0] true
};

Cache_Line::Cache_Line(int tag_size) {}
bool Cache_Line::read_to_cline(int address) {}
bool Cache_Line::write_to_cline(int address) {}

class Cache_Engine {
  private:
    // general config
    int cyc_acc_mem;     // how much cycles takes to acces DRAM
    int block_size_bits; // [bits] | represents log_2(actual_block_size)
    int block_size;      // [bytes] | we will evaluate by pow(2,block_size_bits)
    bool is_write_alloc;

    // L1 config
    int l1_size_bits;     // [bits] | represents log_2(actual_L1_size_in_bytes)
    int l1_assoc;         // 1 way, 2 way, etc.
    int l1_num_of_sets;   // [#sets] | evaluate by: #block / #ways_of_L2
    int l1_set_size_bits; // [bits] | evaluate by: log_2(l1_num_of_sets)
    int cyc_acc_L1;       // how much cycles takes to acces L1

    // L2 config
    int l2_size_bits;     // [bits] | represents log_2(actual_L2_size_in_bytes)
    int l2_assoc;         // 1 way, 2 way, etc.
    int l2_num_of_sets;   // [#sets] | evaluate by: #block / #ways_of_L2
    int l2_set_size_bits; // [bits] | evaluate by: log_2(l1_num_of_sets)
    int cyc_acc_L2;       // how much cycles takes to acces L2

    Cache_Line *L1_cache;
    Cache_Line *L2_cache;

    // SIM INFO

  public:
    // Constrcutors
    Cache_Engine(int mem_cyc, int block_size, int l1_size, int l2_size,
                 int l1_cyc, int l2_cyc, int l1_assoc, int l2_assoc,
                 bool is_write_alloc);

    void write_to_mem(int address);
    void read_to_mem(int address);
    void print_DEBUG();
};

Cache_Engine::Cache_Engine(int mem_cyc, int block_size, int l1_size,
                           int l2_size, int l1_cyc, int l2_cyc, int l1_assoc,
                           int l2_assoc, bool is_write_alloc) {

    this;
}

void Cache_Engine::write_to_mem(int address) {}
void Cache_Engine::read_to_mem(int address) {}
void Cache_Engine::print_DEBUG() {}

// initializing
Cache_Engine myCache;
