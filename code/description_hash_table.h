struct Description_Entry
{
    u64 hash;
    
    u8 *text;
    
    u32 text_size;
    s32 time_sum;
    
    u32 count;
    b32 text_was_truncated;
};

struct Description_Table
{
    Description_Entry *entries;
    u64 entry_max_count;
    u64 mask;
    
    u64 unique_count;
};
