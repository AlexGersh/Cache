#pragma once
class Cache_Line
{

  bool* ways;
  int* tags;
  int tag_size;
  public:
    //Constrcutors
    Cache_Line(int tag_size);

    //functions
    bool read_to_cline(int address);
    bool write_to_cline(int address);

    //r 0x000 00001
    // ways[0]=true;
    //w 0x000 00003
    // ways[0] true
};

Cache_Line::Cache_Line(int tag_size)
{

}
bool Cache_Line::read_to_cline(int address)
{

}
bool Cache_Line::write_to_cline(int address)
{

}

class Cache_Engine
{
  private:
    int cache_size; //in bytes
    int block_size; //in bytes
    int offset_size_b;  //number of bits
    int num_of_sets;     //number of sets
    int set_size_b; //set size of bits
    int assoc_l1;  //1 way, 2 way ,etc'
    int assoc_l2;  //1  way, 2 way ,etc'
    int cyc_acc_L1;
    int cyc_acc_L2;
    int cyc_acc_mem;

    bool is_write_alloc;

    Cache_Line* L1_cache;
    Cache_Line* L2_cache;

    //SIM INFO
  
  public:
    //Constrcutors
    Cache_Engine(int mem_cyc,int block_size,int l1_size,int l2_size,int l1_cyc,int l2_cyc,int l1_assoc,int l2_assoc,bool is_write_alloc);

    void write_to_mem(int address);
    void read_to_mem(int address);
    void print_DEBUG();
     

};

Cache_Engine::Cache_Engine(int mem_cyc,int block_size,int l1_size,int l2_size,int l1_cyc,int l2_cyc,int l1_assoc,int l2_assoc,bool is_write_alloc)
{

}

void Cache_Engine::write_to_mem(int address)
{

}
void Cache_Engine::read_to_mem(int address)
{

}
void Cache_Engine::print_DEBUG()
{

}
