#pragma once
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
struct Sim_Info {

    // L1_cache
    int l1_num_miss;
    int l1_num_acc;

    // L2_cache
    int l2_num_miss;
    int l2_num_acc;

    // mem
    int mem_num_acc;
};

enum STATUS { HIT_NO_REPLACE, HIT_REPLACE_NO_DIRTY, HIT_REPLACE_DIRTY, MISS}; 
/************************ Cache_Line DECLARATIONS **************************/
class Cache_Line {

    bool *ways;
    int *tags;
    int num_of_ways;
    int *LRU_ways;
    bool *dirty_ways;
    
    bool is_write_alloc;
    void update_LRU();

  public:
    // Constructors
    Cache_Line();
    Cache_Line(int num_of_ways, bool is_write_alloc);

    // ----functions

    // read from cache line.
    //
    // return false if miss - else, return true
    bool read_from_cline(int tag,int offset);

    // write to cache line. will replace blocks if full.
    // int* out - pointer to replaced(if any) address block.
    // if no write allocate police in MISS will not add the block to the cache
    // line. else, in MISS will add block to the cache line. status 0 - HIT and
    // no repalce status 1 - HIT and replace no dirty bit status 2 - HIT and
    // replace with dirty bit status 3 - MISS
    void write_to_cline(int tag,int offset, int *out, int *status);
    void get_LRU();

    // print the cache line. only for debugging.
    void print_DEBUG();
    // r 0x000 00001
    //  ways[0]=true;
    // w 0x000 00003
    //  ways[0] true
};

/*************************** Cache Line IMPLEMENTATIONS *****************************/
Cache_Line::Cache_Line(){}
Cache_Line::Cache_Line(int num_of_ways,bool is_write_alloc) 
{
  this->ways = new bool[tag_size];
  this->tags = new int[tag_size];
  this->num_of_ways = num_of_ways;
  this->LRU_ways = new int[tag_size];
  this->dirty_ways = new bool[num_of_ways];
  this->is_write_alloc = is_write_alloc;
}
bool Cache_Line::read_from_cline(int tag,int offset) {}

void Cache_Line::write_to_cline(int tag,int offset, int *out, int *status) 
{

  // searching for HIT
  for(int i=0;i<this->num_of_ways;i++)
  {
    if(tag==this->tags[i])
    {
      //we got a HIT
      *status = HIT_NO_REPLACE;
      this->dirty_ways[i] = true;
      return;
    }
  }

  //if MISS
  if(this->is_write_alloc)
  {
    //write allocate police
    //finding somewhere to place
    for(int i=0;i<this->num_of_ways;i++)
    {
      if(!this->ways[i])
      {
        //found empty block
      }
    }
  }
}

void Cache_Line::print_DEBUG() {
    std::cout << "Cache status: write_alloc_police = " << this->is_write_alloc;
    for (int i = 0; i < this->num_of_ways; i++) {
        std::cout << " WAY" << i << " [";
        std::cout << " TAG = " << this->tags[i]
                  << " Is taken = " << this->ways[i]
                  << " LRU = " << this->LRU_ways[i]
                  << " DirtyBit = " << this->dirty_ways[i] << "]";
    }

    std::cout << std::endl;
}

/************************ Cache_Engine DECLARATIONS **************************/
class Cache_Engine {
  private:
    // general config
    int cyc_acc_mem;     // how much cycles takes to acces DRAM
    int block_size_bits; // [bits] | represents log_2(actual_block_size)
                         // also represents the offset_size_bits
    int block_size;      // [bytes] | evaluated by pow(2,block_size_bits)
    bool is_write_alloc;

    // L1 config
    int l1_size_bits;     // [bits] | represents log_2(actual_L1_size_in_bytes)
    int l1_assoc;         // 1 way, 2 way, etc.
    int l1_num_of_sets;   // [#sets] | evaluate by: #block / #ways_of_L2
    int l1_set_size_bits; // [bits] | evaluate by: log_2(l1_num_of_sets)
    int cyc_acc_L1;       // how much cycles takes to acces L1
    int l1_tag_size_bits; // [bits] | how much tag bits we have
                          // evaluated by function

    // L2 config
    int l2_size_bits;     // [bits] | represents log_2(actual_L2_size_in_bytes)
    int l2_assoc;         // 1 way, 2 way, etc.
    int l2_num_of_sets;   // [#sets] | evaluate by: #block / #ways_of_L2
    int l2_set_size_bits; // [bits] | evaluate by: log_2(l1_num_of_sets)
    int cyc_acc_L2;       // how much cycles takes to acces L2
    int l2_tag_size_bits; // [bits] | how much tag bits we have
                          // evaluated by function

    Cache_Line *L1_cache;
    Cache_Line *L2_cache;

    // SIM INFO
    Sim_Info info;

  public:
    // Constructors
    Cache_Engine();
    Cache_Engine(int mem_cyc, int block_size, int l1_size, int l2_size,
                 int l1_cyc, int l2_cyc, int l1_assoc, int l2_assoc,
                 bool is_write_alloc);

    // functions
    void write_to_mem(int address);
    void read_from_mem(int address);
    void print_DEBUG();
    int evaluate_tag_size(int offset_size_bits, int set_size_bits);
};

/*********************** Cache_Engine IMPLEMENTATIONS *************************/
// constructors
Cache_Engine::Cache_Engine() {}
Cache_Engine::Cache_Engine(int mem_cyc, int block_size, int l1_size,
                           int l2_size, int l1_cyc, int l2_cyc, int l1_assoc,
                           int l2_assoc, bool is_write_alloc) {
    // general
    this->cyc_acc_mem = mem_cyc;
    this->block_size_bits = block_size;
    this->block_size = std::pow(2, this->block_size_bits); // size in bytes
    this->is_write_alloc = is_write_alloc;

    // L1
    this->l1_size_bits = l1_size;
    this->l1_assoc = l1_assoc;
    this->l1_num_of_sets = pow(2, this->l1_size_bits) / this->l1_assoc;
    this->l1_set_size_bits = std::log2(l1_num_of_sets);
    this->cyc_acc_L1 = l1_cyc;
    this->l1_tag_size_bits =
        evaluate_tag_size(this->block_size_bits, this->l1_set_size_bits);

    // L2
    this->l2_size_bits = l2_size;
    this->l2_assoc = l2_assoc;
    this->l2_num_of_sets = pow(2, this->l2_size_bits) / this->l2_assoc;
    this->l2_set_size_bits = std::log2(l2_num_of_sets);
    this->cyc_acc_L2 = l2_cyc;
    this->l2_tag_size_bits =
        evaluate_tag_size(this->block_size_bits, this->l2_set_size_bits);

    // Cache_Line arrays
    L1_cache = new Cache_Line[l1_num_of_sets];
    L2_cache = new Cache_Line[l2_num_of_sets];

    for (int i = 0; i < l1_num_of_sets; ++i) {
        L1_cache[i] = Cache_Line(l1_assoc, this->is_write_alloc);
    }

    for (int i = 0; i < l2_num_of_sets; ++i) {
        L2_cache[i] = Cache_Line(l2_assoc, this->is_write_alloc);
    }

    // Sim_info init
    this->info.l1_num_acc = 0;
    this->info.l2_num_acc = 0;
    this->info.l1_num_miss = 0;
    this->info.l2_num_miss = 0;
    this->info.mem_num_acc = 0;
}

// destructors

// functions
void Cache_Engine::write_to_mem(int address) {
    /*
    int out1;
    int out2;
    int status;
    set_L1 = getSet(address);
    cline_L1 = this->L1_cache[set];

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

void Cache_Engine::read_from_mem(int address) {}

void Cache_Engine::print_DEBUG() {}

int Cache_Engine::evaluate_tag_size(int offset_size_bits, int set_size_bits) {
    return 32 - set_size_bits - offset_size_bits;
}

// initializing
Cache_Engine myCache;

// FOR DEBUGGING ONLY
int main() {
    Cache_Line l1 = Cache_Line(3, false);
    l1.print_DEBUG();
}
