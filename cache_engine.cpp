#pragma once
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

//#define DEBUG 
#ifdef DEBUG 
  #define DEBUG_COMMAND(x) x;
#else 
  #define DEBUG_COMMAND(x)  ;
#endif

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

    bool *valid_way;
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
    Cache_Line &operator=(const Cache_Line &);
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
    void read_from_cline(uint32_t tag, uint32_t *out_tag, int *status);

    // write to cache line. will replace blocks if full.
    // int* out - pointer to replaced(if any) address block.
    // if no write allocate police in MISS will not add the block to the cache
    // line. else, in MISS will add block to the cache line. status 0 - HIT and
    // no repalce status 1 - HIT and replace no dirty bit status 2 - HIT and
    // replace with dirty bit status 3 - MISS
    void write_to_cline(uint32_t tag, uint32_t *out, int *status);

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
Cache_Line::Cache_Line():Cache_Line(1,false) {}

Cache_Line::Cache_Line(int num_of_ways, bool is_write_alloc) {
    this->valid_way = new bool[num_of_ways];
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

Cache_Line &Cache_Line::operator=(const Cache_Line &other) {
    if (this == &other)

        return *this;
    delete[] tags;
    delete[] valid_way;
    delete[] LRU_ways;
    delete[] dirty_ways;

    tags = new uint32_t[other.num_of_ways];
    valid_way = new bool[other.num_of_ways];
    LRU_ways = new int[other.num_of_ways];
    dirty_ways = new bool[other.num_of_ways];

    for (int i = 0; i < other.num_of_ways; i++) {
        this->tags[i] = other.tags[i];
        this->valid_way[i] = other.valid_way[i];
        this->LRU_ways[i] = other.LRU_ways[i];
        this->dirty_ways[i] = other.dirty_ways[i];
    }

    is_write_alloc = other.is_write_alloc;
    num_of_ways = other.num_of_ways;
    return *this;
}
// Functions
void Cache_Line::read_from_cline(uint32_t tag, uint32_t *out_tag, int *status) {

    // check if tag is in cache line and the way is not empty
    for (int i = 0; i < this->num_of_ways; i++) {
        if (this->valid_way[i] && tag == this->tags[i]) {
            this->update_LRU(i);
            *status = HIT;
            return;
        }
    }
    *status = !HIT;
    // MISS - searching for empty block
    
    
    for (int i = 0; i < this->num_of_ways; i++) {
        if (!this->valid_way[i]) {
            this->InitWay(i, tag, true);
            this->update_LRU(i);

            return;
        }
    }

    // MISS - no empty block. going to replace block
    int i = this->get_LRU();
    *out_tag = this->tags[i];
    *status |= (REPLACE | this->dirty_ways[i]);
    // std::cout<< "i:"<<i<<std::endl;
    this->InitWay(i, tag, true);
    this->update_LRU(i);
}

void Cache_Line::write_to_cline(uint32_t tag, uint32_t *out_tag, int *status) {

    *out_tag = 0xFFFFFFFF; // Default output
    // searching for HIT
    for (int i = 0; i < this->num_of_ways; i++) {
        if (tag == this->tags[i] && this->valid_way[i]) {
            // we got a HIT
            *status = HIT;
            this->dirty_ways[i] = true;
            this->update_LRU(i);
            return;
        }
    }

    // if MISS
    *status = !HIT;

    if (this->is_write_alloc) {
        // write allocate police
        // finding somewhere to place
        for (int i = 0; i < this->num_of_ways; i++) {
            if (!this->valid_way[i]) {
                // found empty block
                this->InitWay(i, tag, true);
                this->dirty_ways[i] = true;
                this->update_LRU(i);
                return;
            }
        }
        // if found no empty block will need to replace it.
        int i = get_LRU();
        *out_tag = this->tags[i];
        // std::cout<< "i:"<<i<<std::endl;
        *status = REPLACE | this->dirty_ways[i];
        this->InitWay(i, tag, true);
        this->dirty_ways[i] = true;
        this->update_LRU(i);
    }

    // if no write allocate police so need to do nothing
}

void Cache_Line::update_LRU(int i) {

    int x = this->LRU_ways[i];
    this->LRU_ways[i] = this->num_of_ways - 1;

    for (int j = 0; j < this->num_of_ways; j++) {
        if (j != i && (this->LRU_ways[j] > x))
            this->LRU_ways[j]--;
    }
}

int Cache_Line::get_LRU() {
    for (int i = 0; i < this->num_of_ways; i++) {
        if (this->LRU_ways[i] == 0) {
            return i;
        }
    }
    // if you got here- there was some error in program
    return -1;
}

void Cache_Line::InitWay(int wayN, uint32_t tag, bool is_taken) {
    this->tags[wayN] = tag;
    this->dirty_ways[wayN] = false;
    this->valid_way[wayN] = is_taken;
    this->LRU_ways[wayN] = 0;
}

void Cache_Line::print_DEBUG() {
    std::cout << "Cache status: write_alloc_police = " << this->is_write_alloc;
    for (int i = 0; i < this->num_of_ways; i++) {
        std::cout << " WAY" << i << " [";
        std::cout << " TAG = " << std::hex << this->tags[i] << std::dec
                  << " Is taken = " << this->valid_way[i]
                  << " LRU = " << this->LRU_ways[i]
                  << " DirtyBit = " << this->dirty_ways[i] << "]";
    }

    std::cout << std::endl;
}

// Destructors
Cache_Line::~Cache_Line() {
    delete[] this->valid_way;
    delete[] this->tags;
    delete[] this->LRU_ways;
    delete[] this->dirty_ways;
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

    // write to cache
    void write_to_mem(uint32_t address);

    // read from cache
    void read_from_mem(uint32_t address);

    // prints cache_Engine for debugging
    void print_DEBUG();

    // print the info off current Sim_Info
    void printSimInfo();

    void getSimInfo(double &, double &, double &);
    // Static functions
    /* param @address - keeps the addrss of the instruction
     * param @for_cache_L1 - if true: we want the tag for L1, false - for L2
     * returns tag bits of the address */
    uint32_t getTag(uint32_t address, bool for_cache_L1);

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
    this->l2_num_of_sets = l2_num_of_blocks / l2_num_of_ways;
    this->l2_set_size_bits = std::log2(l2_num_of_sets);
    this->cyc_acc_L2 = l2_cyc;
    this->l2_tag_size_bits =
        evaluate_tag_size(this->block_offset_size_bits, this->l2_set_size_bits);

    // Cache_Line arrays
    this->L1_cache = new Cache_Line[l1_num_of_sets];
    this->L2_cache = new Cache_Line[l2_num_of_sets];
    for (int i = 0; i < l1_num_of_sets; ++i) {
        L1_cache[i] = Cache_Line(l1_num_of_ways, this->is_write_alloc);
    }

    for (int i = 0; i < l2_num_of_sets; ++i) {
        L2_cache[i] = Cache_Line(l2_num_of_ways, this->is_write_alloc);
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
    uint32_t tag_size = this->l2_tag_size_bits;
    if (for_cache_L1) {
        tag_size = this->l1_tag_size_bits;
    }
    uint32_t tag_mask = (1U << tag_size) - 1;
    uint32_t temp = address;
    temp >>= (32 - tag_size); // remove offset adn set bits
    return temp & tag_mask;
}

//
void Cache_Engine::write_to_mem(uint32_t address) {
    uint32_t out_tag_1;
    uint32_t out_tag_2;
    int status_1_write;
    int status_2_write;
    int status_1_read;
    int status_2_read;
    uint32_t set_L1 = getSet(address, this->l1_set_size_bits);
    uint32_t set_L2 = getSet(address, this->l2_set_size_bits);
    uint32_t tag_L1 = getTag(address, true);
    uint32_t tag_L2 = getTag(address, false);
    Cache_Line &cline_L1 = this->L1_cache[set_L1];
    Cache_Line &cline_L2 = this->L2_cache[set_L2];

    // DEBUG - delete after
    DEBUG_COMMAND(std::cout << std::hex << " set_L1= " << set_L1 << " set_L2= " << set_L2
              << " tag_L1=" << tag_L1 << " tag_L2=" << tag_L2 << std::dec
              << std::endl);

    // writing to L1
    cline_L1.write_to_cline(tag_L1, &out_tag_1, &status_1_write);
    this->info.l1_num_acc++;
    // if there was a hit, no actions to do
    // remember the cache works only on WB policy
    if (status_1_write == HIT)
        return;

    this->info.l2_num_acc++;
    this->info.l1_num_miss++;
    // else we get miss. check if we are at write alloc and the cache line is
    // not full

    // MISS and REPLACE and dirty block in l1
    if (status_1_write != HIT && status_1_write == (REPLACE | DIRTY)) {

        cline_L2.read_from_cline(tag_L2, &out_tag_2, &status_2_read);
        if (status_2_read != HIT) {
            this->info.l2_num_miss++;
            this->info.mem_num_acc++;
        }
        uint32_t new_address = out_tag_1 << (32 - this->l1_tag_size_bits);
        new_address |=
            set_L1 << (32 - this->l1_tag_size_bits - this->l1_set_size_bits);
        uint32_t new_Tag = getTag(new_address, false);

        cline_L2.write_to_cline(new_Tag, &out_tag_2, &status_2_write);

    } else if (status_1_write != HIT) {
        cline_L2.write_to_cline(tag_L2, &out_tag_2, &status_2_write);
        if (status_2_write != HIT) {
            this->info.l2_num_miss++;
            this->info.mem_num_acc++;
        }
    }
}

//
void Cache_Engine::read_from_mem(uint32_t address) {
    uint32_t out_tag_1;
    uint32_t out_tag_2;
    int status_2_write;
    int status_1_read;
    int status_2_read;
    uint32_t set_L1 = getSet(address, this->l1_set_size_bits);
    uint32_t set_L2 = getSet(address, this->l2_set_size_bits);
    uint32_t tag_L1 = getTag(address, true);
    uint32_t tag_L2 = getTag(address, false);
    Cache_Line &cline_L1 = this->L1_cache[set_L1];
    Cache_Line &cline_L2 = this->L2_cache[set_L2];

    //DEBUG delete after

    DEBUG_COMMAND(std::cout << std::hex << " set_L1=" << set_L1 << " set_L2=" << set_L2
              << " tag_L1=" << tag_L1 << " tag_L2=" << tag_L2 << std::dec
              << std::endl);

    // trying to find at L1
    cline_L1.read_from_cline(tag_L1, &out_tag_1, &status_1_read);
    this->info.l1_num_acc++;

    if (status_1_read == HIT)
        return; // found the tag at L1, exit

    // else: missed at L1
    this->info.l1_num_miss++;

    // looking at L2:
    // there was no replace
    cline_L2.read_from_cline(tag_L2, &out_tag_2, &status_2_read);
    this->info.l2_num_acc++;

    if (status_1_read == (REPLACE | DIRTY)) {

        // if replace and dirty bit
        uint32_t new_address = out_tag_1 << (32 - this->l1_tag_size_bits);
        new_address |=
            set_L1 << (32 - this->l1_tag_size_bits - this->l1_set_size_bits);
        uint32_t new_Tag = getTag(new_address, false);

        // out_tag_2 and status_2_write only for proper working function
        cline_L2.write_to_cline(new_Tag, &out_tag_2, &status_2_write);
        // we dont increament l2_num_acc because of the instruction of matala
    }

    if (status_2_read == HIT)
        return; // found the tag at L2, exit

    // else: missed also at L2
    this->info.l2_num_miss++;
    this->info.mem_num_acc++;
}

//
void Cache_Engine::print_DEBUG() {
    std::cout << "--- Cache Configuration ---" << std::endl;

    std::cout << "DRAM Access Cycles: " << cyc_acc_mem << std::endl;
    std::cout << "Block Offset Size (bits): " << block_offset_size_bits
              << std::endl;
    std::cout << "Block Size (bytes): " << block_size << std::endl;
    std::cout << "Write Allocate: " << (is_write_alloc ? "Yes" : "No")
              << std::endl;

    std::cout << "\n--- L1 Cache ---" << std::endl;
    std::cout << "L1 Size (bits): " << l1_size_bits << std::endl;
    std::cout << "L1 Ways: " << l1_num_of_ways << std::endl;
    std::cout << "L1 Blocks: " << l1_num_of_blocks << std::endl;
    std::cout << "L1 Sets: " << l1_num_of_sets << std::endl;
    std::cout << "L1 Set Size (bits): " << l1_set_size_bits << std::endl;
    std::cout << "L1 Access Cycles: " << cyc_acc_L1 << std::endl;
    std::cout << "L1 Tag Size (bits): " << l1_tag_size_bits << std::endl;

    std::cout << "\n--- L2 Cache ---" << std::endl;
    std::cout << "L2 Size (bits): " << l2_size_bits << std::endl;
    std::cout << "L2 Ways: " << l2_num_of_ways << std::endl;
    std::cout << "L2 Blocks: " << l2_num_of_blocks << std::endl;
    std::cout << "L2 Sets: " << l2_num_of_sets << std::endl;
    std::cout << "L2 Set Size (bits): " << l2_set_size_bits << std::endl;
    std::cout << "L2 Access Cycles: " << cyc_acc_L2 << std::endl;
    std::cout << "L2 Tag Size (bits): " << l2_tag_size_bits << std::endl;
    std::cout << "--L1 cache--" << std::endl;

    for (int i = 0; i < l1_num_of_sets; i++) {
        L1_cache[i].print_DEBUG();
    }

    std::cout << "--L2 cache--" << std::endl;
    for (int i = 0; i < l2_num_of_sets; i++) {
        L2_cache[i].print_DEBUG();
    }
}

double round_3(double x) { return std::round(x * 1000.0) / 1000.0; }

void Cache_Engine::printSimInfo() {
    std::cout << "===SIM_INFO===" << std::endl;

    std::cout << "\t l1_num_acc=" << this->info.l1_num_acc << std::endl;
    std::cout << "\t l2_num_acc=" << this->info.l2_num_acc << std::endl;
    std::cout << "\t l1_num_miss=" << this->info.l1_num_miss << std::endl;
    std::cout << "\t l2_num_miss=" << this->info.l2_num_miss << std::endl;
    std::cout << "\t mem_num_acc=" << this->info.mem_num_acc << std::endl;
}

void Cache_Engine::getSimInfo(double &L1MissRate, double &L2MissRate,
                              double &avgAccTime) {

    L1MissRate = (double)this->info.l1_num_miss / this->info.l1_num_acc;
    L2MissRate = (double)this->info.l2_num_miss / this->info.l2_num_acc;

    int t_l1 = this->cyc_acc_L1;
    int t_l2 = this->cyc_acc_L2;
    int t_mem = this->cyc_acc_mem;

    avgAccTime = t_l1 + L1MissRate * (t_l2 + L2MissRate * t_mem);

    L1MissRate = round_3(L1MissRate);
    L2MissRate = round_3(L2MissRate);
    avgAccTime = round_3(avgAccTime);
}

// initializing
// Cache_Engine myCache;

// FOR DEBUGGING ONLY
// int main() {
//
//     std::cout << "start of test program" << std::endl;
//     Cache_Line l1 = Cache_Line(4, false);
//     Cache_Line l2 = Cache_Line(2, true);
//     l1.print_DEBUG();
//     l2.print_DEBUG();
//
//     int status = 0;
//     int out = 0;
//
//     std::cout << "DEBUG for write no allocate" << std::endl;
//     l1.write_to_cline(100, &out, &status);
//     l1.print_DEBUG();
//     std::cout << "STATUS after write:" << status << std::endl;
//
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
//
//     l2.write_to_cline(105, &out, &status);
//     l2.print_DEBUG();
//     std::cout << "STATUS after write:" << status << std::endl;
//
//     std::cout<< "DEBUG for read" << std::endl;
//
//     std:cout<<l2.read_from_cline
//     std::cout << "end of test program" << std::endl;
//
//       Cache_Engine engine= Cache_Engine(100,4,6,7,1,5,
//                            1,1,1);
//
//       engine.print_DEBUG();
//       engine.write_to_mem(0x00000000);
//       engine.print_DEBUG();
//       engine.write_to_mem(0x00000000);
//       engine.print_DEBUG();
//       engine.write_to_mem(0x0a0a0a0a);
//       engine.print_DEBUG();
//       engine.write_to_mem(0xa000a000);
//       engine.print_DEBUG();
//     return 0;
// }
