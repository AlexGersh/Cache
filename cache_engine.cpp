#pragma once
#include <cmath>
#include <cstdint>
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

enum STATUS {
    HIT = 1 << 2,     // 0b100
    REPLACE = 1 << 1, // 0b010
    DIRTY = 1 << 0    // 0b001
};

/************************ Cache_Line DECLARATIONS **************************/
class Cache_Line {

    bool *ways;
    uint32_t *tags; // tags[N] tag of wayN.
    int num_of_ways;
    int *LRU_ways;
    bool *dirty_ways;

    bool is_write_alloc;
    void update_LRU(int i);

  public:
    // Constructors
    Cache_Line();
    Cache_Line(int num_of_ways, bool is_write_alloc);

    // ----functions

    // function will set up all a spsecific way with given parameters
    // @params : int wayN - index of way
    //           int tag - the tag to setup the way
    //           int is_taken - if we setting up with empty way or some tag and
    //           data.
    void InitWay(int, uint32_t, bool);
    // read from cache line.
    //
    // return false if miss - else, return true
    bool read_from_cline(uint32_t tag, uint32_t offset);

    // write to cache line. will replace blocks if full.
    // int* out - pointer to replaced(if any) address block.
    // if no write allocate police in MISS will not add the block to the cache
    // line. else, in MISS will add block to the cache line. status 0 - HIT and
    // no repalce status 1 - HIT and replace no dirty bit status 2 - HIT and
    // replace with dirty bit status 3 - MISS
    void write_to_cline(uint32_t tag, int *out, int *status);

    // get the LRU. it will find the way with the largets LRU number.
    int get_LRU();

    // print the cache line. only for debugging.
    void print_DEBUG();
    // r 0x000 00001
    //  ways[0]=true;
    // w 0x000 00003
    //  ways[0] true

    // Destructors
    ~Cache_Line();
};

/************************ Cache Line IMPLEMENTATIONS **************************/
// Constructors
Cache_Line::Cache_Line() {}

Cache_Line::Cache_Line(int num_of_ways, bool is_write_alloc) {
    this->ways = new bool[num_of_ways];
    this->tags = new uint32_t[num_of_ways];
    this->num_of_ways = num_of_ways;
    this->LRU_ways = new int[num_of_ways];
    this->dirty_ways = new bool[num_of_ways];
    this->is_write_alloc = is_write_alloc;

    // initializing each way
    for (int i = 0; i < num_of_ways; i++) {
        InitWay(i, 0, false);
    }
}

// Functions
bool Cache_Line::read_from_cline(uint32_t tag, uint32_t offset) {}

void Cache_Line::write_to_cline(uint32_t tag, int *out, int *status) {

    *out = 0; // Default output
    // searching for HIT
    for (int i = 0; i < this->num_of_ways; i++) {
        if (tag == this->tags[i]) {
            // we got a HIT
            *status = HIT;
            this->dirty_ways[i] = true;
            this->update_LRU(i);
            return;
        }
    }

    // if MISS
    *status = 0 & HIT;
    if (this->is_write_alloc) {
        // write allocate police
        // finding somewhere to place
        for (int i = 0; i < this->num_of_ways; i++) {
            if (!this->ways[i]) {
                // found empty block
                this->InitWay(i, tag, true);
                this->update_LRU(i);
                return;
            }
        }
        // if found no empty block will need to replace it.
        int i = get_LRU();
        *out = this->tags[i];
        // std::cout<< "i:"<<i<<std::endl;
        *status = HIT | REPLACE | this->dirty_ways[i];
        this->InitWay(i, tag, true);
        this->update_LRU(i);
    }

    // if no write allocate police so need to do nothing
}

void Cache_Line::update_LRU(int i) {
    for (int i = 0; i < this->num_of_ways; i++) {
        this->LRU_ways[i]++;
    }
}

int Cache_Line::get_LRU() {
    int max = this->LRU_ways[0];
    int max_index = 0;

    for (int i = 1; i < this->num_of_ways; i++) {
        if (this->LRU_ways[i] > max) {
            max = this->LRU_ways[i];
            max_index = i;
        }
    }
    return max_index;
}

void Cache_Line::InitWay(int wayN, uint32_t tag, bool is_taken) {
    this->tags[wayN] = tag;
    this->dirty_ways[wayN] = false;
    this->ways[wayN] = is_taken;
    this->LRU_ways[wayN] = 0;
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

// Destructors
Cache_Line::~Cache_Line() {
    delete this->ways;
    delete this->tags;
    delete this->LRU_ways;
    delete this->dirty_ways;
}
// need to finish

/************************ Cache_Engine DECLARATIONS **************************/
class Cache_Engine {
  private:
    // general config
    int cyc_acc_mem;            // how much cycles takes to acces DRAM
    int block_offset_size_bits; // [bits] | represents log_2(actual_block_size)
                                // also represents the offset_size_bits
    int block_size; // [bytes] | evaluated by pow(2,block_offset_bits)
    bool is_write_alloc;

    // L1 config
    int l1_size_bits;     // [bits] | represents log_2(actual_L1_size_in_bytes)
    int l1_num_of_ways;   // number of ways at L1 level
    int l1_num_of_blocks; // [blocks] | evaluate by l1_size / block_size
    int l1_num_of_sets;   // [#sets] | evaluate by: #blocks_of_L1 / #ways_of_L1
    int l1_set_size_bits; // [bits] | evaluate by: log_2(l1_num_of_sets)
    int cyc_acc_L1;       // how much cycles takes to acces L1
    int l1_tag_size_bits; // [bits] | how much tag bits we have
                          // evaluated by function

    // L2 config
    int l2_size_bits;     // [bits] | represents log_2(actual_L2_size_in_bytes)
    int l2_num_of_ways;   // number of ways at L2 level
    int l2_num_of_blocks; // [blocks] | evaluate by l2_size / block_size
    int l2_num_of_sets;   // [#sets] | evaluate by: #blocks_of_L2 / #ways_of_L2
    int l2_set_size_bits; // [bits] | evaluate by: log_2(l2_num_of_sets)
    int cyc_acc_L2;       // how much cycles takes to access L2
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

    // Functions

    // gets address and number of set bits
    // returns set bits of the address
    uint32_t getSet(uint32_t address, int set_size_bits);

    // returns tag size in [bits]
    int evaluate_tag_size(int offset_size_bits, int set_size_bits);

    /* param @address - keeps the addrss of the instruction
     * param @for_cache_L1 - if true: we want the tag for L1, false - for L2
     * returns tag bits of the address */
    uint32_t getTag(uint32_t address, bool for_cache_L1);

    // write to cache
    void write_to_mem(uint32_t address);

    // read from cache
    void read_from_mem(uint32_t address);

    // prints cache_Engine for debugging
    void print_DEBUG();

    // Destructors
    ~Cache_Engine();
};

/*********************** Cache_Engine IMPLEMENTATIONS *************************/
// Constructors
Cache_Engine::Cache_Engine() {}

Cache_Engine::Cache_Engine(int mem_cyc, int block_size, int l1_size,
                           int l2_size, int l1_cyc, int l2_cyc, int l1_assoc,
                           int l2_assoc, bool is_write_alloc) {
    // General
    this->cyc_acc_mem = mem_cyc;
    this->block_offset_size_bits = block_size; //
    this->block_size =
        std::pow(2, this->block_offset_size_bits); // size in bytes
    this->is_write_alloc = is_write_alloc;

    // L1
    this->l1_size_bits = l1_size;
    this->l1_num_of_ways = std::pow(2, l1_assoc);
    this->l1_num_of_blocks = std::pow(2, l1_size_bits) / this->block_size;
    this->l1_num_of_sets = l1_num_of_blocks / l1_num_of_ways;
    this->l1_set_size_bits = std::log2(l1_num_of_sets);
    this->cyc_acc_L1 = l1_cyc;
    this->l1_tag_size_bits =
        evaluate_tag_size(this->block_offset_size_bits, this->l1_set_size_bits);

    // L2
    this->l2_size_bits = l2_size;
    this->l2_num_of_ways = std::pow(2, l2_assoc);
    this->l2_num_of_blocks = std::pow(2, l2_size_bits) / this->block_size;
    this->l1_num_of_sets = l2_num_of_blocks / l2_num_of_ways;
    this->l2_set_size_bits = std::log2(l2_num_of_sets);
    this->cyc_acc_L2 = l2_cyc;
    this->l2_tag_size_bits =
        evaluate_tag_size(this->block_offset_size_bits, this->l2_set_size_bits);

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
Cache_Engine::~Cache_Engine() {
    delete[] L1_cache; // array delete
    delete[] L2_cache; // array delete
}

// functions
//
uint32_t Cache_Engine::getSet(uint32_t address, int set_size_bits) {
    uint32_t set_mask = (1U << set_size_bits) - 1;
    uint32_t temp = address;
    temp >>= this->block_offset_size_bits; // remove offset bits
    return temp & set_mask;
}

//
int Cache_Engine::evaluate_tag_size(int offset_size_bits, int set_size_bits) {
    return 32 - set_size_bits - offset_size_bits;
}

//
uint32_t Cache_Engine::getTag(uint32_t address, bool for_cache_L1) {
    int tag_size = this->l2_tag_size_bits; // default assume for cache L2
    if (for_cache_L1) {                    // change to L1 if asked
        tag_size = this->l1_tag_size_bits;
    }

    uint32_t tag_mask = (1U << tag_size) - 1;
    uint32_t temp = address;
    temp >>= (32 - tag_size); // remove offset adn set bits
    return temp & tag_mask;
}

//
void Cache_Engine::write_to_mem(uint32_t address) {

    int out1;
    int out2;
    int status;
    uint32_t set_L1 = getSet(address, this->l1_set_size_bits);
    uint32_t set_L2 = getSet(address, this->l2_set_size_bits);
    uint32_t tag_L1 = getTag(address, true);
    uint32_t tag_L2 = getTag(address, false);
    Cache_Line cline_L1 = this->L1_cache[set_L1];
    Cache_Line cline_L2 = this->L2_cache[set_L2];

    cline_L1.write_to_cline(tag_L1, &out1, &status);

    // if(staus == 0)
    //   siyamnu
    // if(status == 1)
    //   siyamnu;
    // if(status == 2)
    //   cline_L2.write_to_cline(out1,out2,status);  //only modifing LRU
    //   siyamnu;
    // if(status == 3)
    //   cline_L2.write_to_cline(address,out2,status);
    //   siyamnu;
}

//
void Cache_Engine::read_from_mem(uint32_t address) {}

//
void Cache_Engine::print_DEBUG() {}

// initializing
Cache_Engine myCache;

// FOR DEBUGGING ONLY
// int main() {

//     std::cout << "start of test program" << std::endl;
//     Cache_Line l1 = Cache_Line(4, false);
//     Cache_Line l2 = Cache_Line(2, true);
//     l1.print_DEBUG();
//     l2.print_DEBUG();

//     int status = 0;
//     int out = 0;

//     std::cout << "DEBUG for write no allocate" << std::endl;
//     l1.write_to_cline(100, &out, &status);
//     l1.print_DEBUG();
//     std::cout << "STATUS after write:" << status << std::endl;

//     std::cout << " DEBUG for write allocate" << std::endl;
//     l2.write_to_cline(100, &out, &status);
//     l2.print_DEBUG();
//     std::cout << "STATUS after write:" << status << std::endl;
//     l2.write_to_cline(100, &out, &status);
//     l2.print_DEBUG();
//     std::cout << "STATUS after write:" << status << std::endl;
//     l2.write_to_cline(103, &out, &status);
//     l2.print_DEBUG();
//     std::cout << "STATUS after write:" << status << std::endl;

//     l2.write_to_cline(105, &out, &status);
//     l2.print_DEBUG();
//     std::cout << "STATUS after write:" << status << std::endl;

//     std::cout << "end of test program" << std::endl;
// }
