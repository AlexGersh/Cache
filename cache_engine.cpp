struct Sim_Info {
    
  //L1_cache 
  int l1_num_miss;
  int l1_num_acc;

  //L2_cache
  int l2_num_miss;
  int l2_num_acc;
  
  //mem
  int mem_num_acc;
  
};


class Cache_Line {

    bool *ways;
    int *tags;
    int tag_size;
    int* LRU_ways;
    int* dirty_ways;
    
    bool is_write_alloc;
    void update_LRU();
    void get_LRU();
  public:
    // Constructors
    Cache_Line(int tag_size);

    // ----functions
    
    // read from cache line.
    // 
    // return false if miss - else, return true 
    bool read_from_cline(int address);
    
    // write to cache line. will replace blocks if full. 
    // int* out - pointer to replaced(if any) address block.
    // if no write allocate police in MISS will not add the block to the cache line. else, in MISS will add block to the cache line.
    // status 0 - HIT and no repalce
    // status 1 - HIT and replace no dirty bit 
    // status 2 - HIT and replace with dirty bit 
    // status 3 - MISS  
    void write_to_cline(int address,int* out,int* status);

};

Cache_Line::Cache_Line(int tag_size) {}
bool Cache_Line::read_from_cline(int address) {return false;}
void Cache_Line::write_to_cline(int address,int* out,int* status) {}

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
    Sim_Info info;

  public:
    // Constrcutors
    Cache_Engine();
    Cache_Engine(int mem_cyc, int block_size, int l1_size, int l2_size,
                 int l1_cyc, int l2_cyc, int l1_assoc, int l2_assoc,
                 bool is_write_alloc);

    void write_to_mem(int address);
    void read_to_mem(int address);
    void print_DEBUG();
};
Cache_Engine::Cache_Engine(){}
Cache_Engine::Cache_Engine(int mem_cyc, int block_size, int l1_size,
                           int l2_size, int l1_cyc, int l2_cyc, int l1_assoc,
                           int l2_assoc, bool is_write_alloc) {

    
}

void Cache_Engine::write_to_mem(int address) 
{
  /*
  int out1;
  int out2;
  int status;
  set_L1 = getSet(address);
  cline_L1 = this.L1_cache[set];
  
  cline_L1.write_to_cline(address,out,status);

  if(staus == 0)
    siyamnu
  if(status == 1)
    siyamnu;
  if(status == 2)
    cline_L2.write_to_cline(out1,out2,status);  //only modifing LRU
    siyamnu;    
  if(status == 3)
    cline_L2.write_to_cline(address,out2,status);
    siyamnu;
  */
}
void Cache_Engine::read_to_mem(int address) {}
void Cache_Engine::print_DEBUG() {}

// initializing
Cache_Engine myCache;
