#if !defined(Stf0_H)
#define Stf0_H
/* ======================= #define guide ======================

stf0 switches:
    Stf0_Namespace - wrap the code in stf0:: c++ namepsace

100% manual switches:
	Def_Internal:
		0 - Build for public relrease
		1 - Build for developers only
	Def_Slow:
		0 - No slow code allowed!
		1 - Slow code is welcomed (example: additional asserts)
    Def_Log:
		0 - No logging
		1 - Logging some errors to file (io etc)
manual but may be guessed if not specified:
    Def_Windows; Def_Linux - target platforms
    Def_Msvc; Dev_Llvm     - target compiler
*/

/* TODO:
  [ ] Compile with unicode switch to find all cases where Windows ASCII API is not used
*/




//=============================================================
#if !defined(Stf0_Level)
#define Stf0_Level 9999 // Specify this to include less code
#endif



//=============================================================
#if !defined(Stf0_Namespace)
#define Stf0_Namespace 0
#endif
#if !defined(Def_Log)
#define Def_Log 0
#endif
//=====
#if Stf0_Namespace && Stf0_Level >= 20
#define Stf0_Open_Namespace namespace stf0 {
#define Stf0_Close_Namespace };
#else
#define Stf0_Open_Namespace
#define Stf0_Close_Namespace
#endif







#pragma warning(push, 0)

#if Stf0_Level >= 10 // Types
#    include <inttypes.h>
#    include <stdint.h>
#    include <limits.h>
#    include <float.h>
#    include <xmmintrin.h>
#    include <emmintrin.h>
#endif


#if Stf0_Level >= 20 // Basic
//=============================
#if !defined(Def_Internal)
#    define Def_Internal 0
#endif
#if !defined(Def_Slow)
#    define Def_Slow 0
#endif
// ======== Detect platform =======
#if !defined(Def_Windows)
#    define Def_Windows 0
#endif
#if !defined(Def_Linux)
#    define Def_Linux 0
#endif
//=============
#if !Def_Windows && !Def_Linux
// TODO(f0): Check standard switches to automatically and deduce them and make compiling easier if possible
//           And support different compilers (cl, clang, gcc)
#error "Define Def_Windows or Def_Linux"
#endif


// ======== Detect compiler =======
#if !defined(Def_Compiler_Msvc)
#    define Def_Compiler_Msvc 0
#endif
#if !defined(Def_Compiler_Llvm)
#    define Def_Compiler_Llvm 0
#endif
//=============
#if !Def_Compiler_Msvc && !Def_Compiler_Llvm
#    if _MSC_VER
#        undef Def_Compiler_Msvc
#        define Def_Compiler_Msvc 1
#    else
// TODO(f0): More compilers
#        undef Def_Compiler_Llvm
#        define Def_Compiler_Llvm 1
#        endif
#endif


//=============================
#include <stdio.h>
#include <stdlib.h>


//   =============
#if Def_Compiler_Msvc
#    include <intrin.h>
#elif Def_Compiler_Llvm
#    include <x86intrin.h>
#else
#error "not impl; SSE/NEON optimizations?"
#endif
// ======================= end of basic =======================
#endif


#if Stf0_Level >= 30 // Memory
#    include <Windows.h>
#    undef small
#endif


#if Stf0_Level >= 40 // String_Alloc
#    include <stdarg.h>
#endif


#if Stf0_Level >= 50 // Platform
#    pragma comment(lib, "winmm.lib")
#endif

#pragma warning(pop)












//~ ======================= @Level_Types ======================
#if Stf0_Level >= 10
//~ ======================== @Level_10 ========================
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t b32;
typedef float f32;
typedef double f64;
//
typedef __m128 m128;
typedef __m128i m128i;
//=============
#define S8_Min _I8_MIN
#define S8_Max _I8_MAX
#define S16_Min _I16_MIN
#define S16_Max _I16_MAX
#define S32_Min _I32_MIN
#define S32_Max _I32_MAX
#define S64_Min _I64_MIN
#define S64_Max _I64_MAX
//
#define U8_Max _UI8_MAX
#define U16_Max _UI16_MAX
#define U32_Max _UI32_MAX
#define U64_Max _UI64_MAX
//
#define F32_Max FLT_MAX
//
#define Pi32  3.14159265359f
#define Tau32 6.2831853071795864769f
//=============
#if !defined(internal)
#define internal static
#endif
#define local_global static
#define global static
//=============
#define array_count(a) ((sizeof(a))/(sizeof(*a)))
#define pick_smaller(a, b) (((a) > (b)) ? (b) : (a))
#define pick_bigger(a, b) (((a) > (b)) ? (a) : (b))
//
#define kilobytes(b) (1024*(b))
#define megabytes(b) (1024*kilobytes(b))
#define gigabytes(b) ((s64)1024*megabytes(b))
#define terabytes(b) ((s64)1024*gigabytes(b))
//
#define glue_(a, b) a ## b
#define glue(a, b) glue_(a, b)
#define stringify(a) #a
#define textify(a) #a
#define stringify_macro(a) stringify(a)
//
#define for_range(Type, I, Range) for (Type I = 0; I < (Range); ++I)
#define for_array(I, Array) for_range(u64, I, array_count(Array))
#define for_linked_list(Node, List) for (auto Node = (List).first; Node; Node = Node->next)
#define for_u64(I, Range) for_range(u64, I, Range)
#define for_u32(I, Range) for_range(u32, I, Range)
//
// NOTE(f0): Align bits needs to be power of 2
#define align_bin_to(Value, AlignBits) ((Value + (AlignBits-1)) & ~(AlignBits-1))
#define align4(Value) align_bin_to(Value, 4)
#define align8(Value) align_bin_to(Value, 8)
#define align16(Value) align_bin_to(Value, 16)
//=============
#define square_m128(value) _mm_mul_ps((value), (value))
#define f32_from_m128(wide, index) ((f32 *)&(wide))[index]
#define u32_from_m128i(wide, index) ((u32 *)&(wide))[index]
#define s32_from_m128i(wide, index) ((s32 *)&(wide))[index]


//~ ======================= @Level_Basic ======================
#if Stf0_Level >= 20
//~ ======================== @Level_20 ========================



//=============
Stf0_Open_Namespace
//=============




//=============================
#if Def_Windows
#define Native_Slash_Char '\\'
#define Native_Slash_Str "\\"
#elif Def_Linux
#define Native_Slash_Char '/'
#define Native_Slash_Str "/"
#endif



//=============================
#if Def_Compiler_Msvc
#    define This_Function __func__
#    define This_Line_s32 __LINE__
#    define This_File     __FILE__
#else
#    error "not impl"
#endif
//=============
#define File_Line          This_File "(" stringify_macro(This_Line_s32) ")"
#define File_Line_Function File_Line ": " This_Function
//=============
#define force_halt() do{ fflush(stdout); *((s32 *)0) = 1; }while(0)

#define assert_always(Expression) do{ if(!(Expression)) {\
printf("\n" File_Line ": RUNTIME error: assert(%s) in function %s\n",\
textify(Expression), This_Function);\
fflush(stdout);\
debug_break(); force_halt(); exit(1);\
}}while(0)

//=============================
#if Def_Slow
// TODO(f0): other compilers
#    define debug_break() do{ fflush(stdout); __debugbreak(); }while(0)
#    define assert(Expression) assert_always(Expression)
#else
#    define debug_break()
#    define assert(Expression)
#endif
//
#define exit_error() do{ fflush(stdout); debug_break(); exit(1);}while(0)


// ===================== Hacky C++11 defer ====================
template <typename F>
struct Private_Defer {
	F f;
	Private_Defer(F f) : f(f) {}
	~Private_Defer() { f(); }
};

template <typename F>
Private_Defer<F> private_defer_function(F f) {
	return Private_Defer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = private_defer_function([&](){code;})
//=============







// ======================= @Basic_Types =======================

// Time
struct Time32ms
{
    u32 t;
};

struct Time_Perfomance
{
    s64 t_; // NOTE(f0): retrieve with get_delta(a, b)
};


// Strings
typedef const char* cstr_lit;

struct String
{
    u8 *str;
    u64 size;
};

struct Find_Index
{
    u64 index; // NOTE(f0): Index is 0 when not found
    b32 found;
    u32 _padding;
};





// ====================== @Basic_Helpers ======================

inline u64
safe_truncate_to_u64(s64 value)
{
    assert(value >= 0);
    u64 result = (u64)value;
    return result;
}

////
inline u32 
safe_truncate_to_u32(u64 value)
{
	assert(value <= U32_Max);
    u32 result = (u32)value;
	return result;
}

inline u32 
safe_truncate_to_u32(s64 value)
{
	assert(value <= U32_Max);
    assert(value >= 0);
    u32 result = (u32)value;
	return result;
}

////
inline u16
safe_truncate_to_u16(u32 value)
{
    assert(value <= U16_Max);
    u16 result = (u16)value;
    return result;
}

inline u16
safe_truncate_to_u16(s32 value)
{
    assert(value <= U16_Max);
    assert(value >= 0);
    u16 result = (u16)value;
    return result;
}


















// ===================== @Basic_Intrinsics ====================

struct Bit_Scan_Result
{
    u32 index;
    b32 found;
};

inline Bit_Scan_Result
find_most_significant_bit(u64 value)
{
    Bit_Scan_Result result = {};
#if Def_Compiler_Msvc
    result.found = _BitScanReverse64((unsigned long *)&result.index, value);
#else
#error "not impl"
#endif
    return result;
}
inline Bit_Scan_Result find_most_significant_bit(s64 value) {
    return find_most_significant_bit((u64)value);
}

inline Bit_Scan_Result
find_most_significant_bit(u32 value)
{
    Bit_Scan_Result result = {};
#if Def_Compiler_Msvc
    result.found = _BitScanReverse((unsigned long *)&result.index, value);
#else
#error "not impl"
#endif
    return result;
}
inline Bit_Scan_Result find_most_significant_bit(s32 value) {
    return find_most_significant_bit((u32)value);
}

///////
inline Bit_Scan_Result
find_least_significant_bit(u32 value)
{
    Bit_Scan_Result result = {};
#if Def_Compiler_Msvc
    result.found = _BitScanForward((unsigned long *)&result.index, value);
    
#else
    for (u32 test = 0; test < 32; ++ test)
    {
        if (value & (1 << test))
        {
            result.index = test;
            result.found = true;
            break;
        }
    }
#endif
    return result;
}
inline Bit_Scan_Result find_least_significant_bit(s32 value) {
    return find_least_significant_bit((u32)value);
};


inline Bit_Scan_Result
find_least_significant_bit(u64 value)
{
    Bit_Scan_Result result = {};
    
#if Def_Compiler_Msvc
    result.found = _BitScanForward64((unsigned long *)&result.index, value);
#else
#error "not impl"
#endif
    return result;
}
inline Bit_Scan_Result find_least_significant_bit(s64 value) {
    return find_least_significant_bit((u64)value);
};













// ======================= @Basic_Memory ======================

#define copy_array(Destination, Source, Type, Count) copy_bytes(Destination, Source, sizeof(Type)*(Count))

inline void
copy_bytes(void *destination, void *source, u64 number_of_bytes)
{
    u8 *d = (u8 *)destination;
    u8 *s = (u8 *)source;
    
    for (u64 i = 0; i < number_of_bytes; ++i)
    {
        *d++ = *s++;
    }
}


#define clear_array(DestinationPtr, Type, Count) clear_bytes(DestinationPtr, sizeof(Type)*(Count))
inline void
clear_bytes(void *destination, u64 number_of_bytes)
{
    u8 *d = (u8 *)destination;
    for (u64 i = 0; i < number_of_bytes; ++i)
    {
        *d++ = 0;
    }
}





// ================ @Basic_String_Constructors ================
// Macros
#define string_expand(Str) ((s32)((Str).size)), ((char *)((Str).str))
#define string_expand_rev(Str) ((char *)((Str).str)), ((Str).size)
#define lit2str(Literal) string(Literal, array_count(Literal)-1)
#define l2s(Literal) lit2str(Literal)



// Constructors
inline String
string(u8 *str, u64 size)
{
    String result = {str, size};
    return result;
}

inline String
string(char *str, u64 size)
{
    return string((u8 *)str, size);
}

inline u64
cstr_length(cstr_lit string)
{
    u64 length = 0;
    while (*string++)
    {
        ++length;
    }
    return length;
}

inline String
string(char *cstr)
{
    String result = {
        (u8 *)cstr,
        cstr_length(cstr)
    };
    return result;
}








// ======================== @Basic_Char =======================
inline b32
is_slash(u8 c)
{
    b32 result = (c == '\\' || c == '/');
    return result;
}

inline b32
is_end_of_line(u8 c)
{
    b32 result = ((c == '\n') || (c == '\r'));
    return result;
}

inline b32
is_whitespace(u8 c)
{
    b32 result = ((c == ' ') ||
                  (c == '\t') ||
                  (c == '\v') ||
                  (c == '\f'));
    return result;
}

inline b32
is_white(u8 c)
{
    b32 result = (is_whitespace(c) ||
                  is_end_of_line(c));
    return result;
}
inline b32 is_white(char c) {
    return is_white((u8)c);
}

inline b32
is_alpha(u8 c)
{
    b32 result = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    
    return result;
}

inline b32
is_number(u8 c)
{
    b32 result = (c >= '0' && c <= '9');
    return result;
}


inline u8
get_lower(u8 c)
{
    u8 delta = 'a' - 'A';
    if (c >= 'A' && c <= 'Z')
    {
        c += delta;
    }
    return c;
}
inline char get_lower(char c) {
    return (char)get_lower((u8)c);
}







// ======================== @Basic_Cstr =======================
internal b32
cstr_equal(cstr_lit value_a, cstr_lit value_b, b32 case_ins = false)
{
    b32 result = false;
    for(;;)
    {
        char a = (*value_a++);
        char b = (*value_b++);
        
        if (case_ins)
        {
            a = get_lower(a);
            b = get_lower(b);
        }
        
        if (a != b) { break; }
        else if (a == 0) { result = true; break; }
    }
    return result;
}

internal b32
cstr_starts_with(cstr_lit big, cstr_lit small, b32 case_ins = false)
{
    b32 result = false;
    for(;;)
    {
        char b = (*big++);
        char s = (*small++);
        
        if (case_ins)
        {
            b = get_lower(b);
            s = get_lower(s);
        }
        
        if (s == 0) { result = true; break; }
        else if (b == 0 || b != s) { break; }
    }
    return result;
}

internal b32
cstr_ends_with(cstr_lit big, cstr_lit small, b32 case_ins = false)
{
    u64 big_len = cstr_length(big);
    u64 small_len = cstr_length(small);
    
    for (u64 i = 1; i <= small_len; ++i)
    {
        char b = big[big_len - i];
        char s = small[small_len - i];
        
        if (case_ins)
        {
            b = get_lower(b);
            s = get_lower(s);
        }
        
        if (b != s) { return false; }
    }
    
    return true;
}



internal Find_Index
cstr_find_index_from_left(cstr_lit value, char c)
{
    Find_Index result = {};
    for (u64 index = 0;
         ;
         ++index)
    {
        char v = value[index];
        
        if (v == c)
        {
            result.index = index;
            result.found = true;
            break;
        }
        else if (!v)
        {
            break;
        }
    }
    return result;
}


internal Find_Index
cstr_find_difference_index_from_left(cstr_lit value_a, cstr_lit value_b, b32 case_ins = false)
{
    Find_Index result = {};
    
    for (u64 diff_index = 0;
         ; 
         ++diff_index)
    {
        char a = *value_a++;
        char b = *value_b++;
        
        if (case_ins)
        {
            a = get_lower(a);
            b = get_lower(b);
        }
        
        if (a != b)
        {
            result.index = diff_index;
            result.found = true;
            break;
        }
        
        if (a == 0)
        {
            break;
        }
    }
    
    return result;
}




internal Find_Index
cstr_find_common_character_from_left(cstr_lit value, String character_table)
{
    Find_Index result = {};
    
    for (u64 value_index = 0;
         ;
         ++value_index)
    {
        char v = value[value_index];
        
        for (u64 search_index = 0;
             search_index < character_table.size;
             ++search_index)
        {
            char s = (char)character_table.str[search_index];
            if (v == s)
            {
                result.index = value_index;
                result.found = true;
                goto exit_break_label;
            }
        }
        
        if (!v)
        {
            break;
        }
    }
    
    exit_break_label:
    return result;
}



internal u64
cstr_length_trim_white_from_right(char *cstr)
{
    u64 len = cstr_length(cstr);
    for (; len > 0; --len)
    {
        if (!is_white(cstr[len-1]))
        {
            break;
        }
    }
    return len;
}









// ======================= @Basic_String ======================
internal b32
string_equal(String str_a, String str_b, b32 case_ins = false)
{
    b32 result = false;
    
    if (str_a.size == str_b.size)
    {
        result = true;
        for(u64 i = 0; i < str_a.size; ++i)
        {
            u8 a = str_a.str[i];
            u8 b = str_b.str[i];
            
            if (case_ins)
            {
                a = get_lower(a);
                b = get_lower(b);
            }
            
            if (a != b)
            {
                result = false;
                break;
            }
        }
    }
    
    return result;
}



struct Compare_Line_Pos
{
    u32 line;
    u32 column;
    b32 is_equal;
};

internal Compare_Line_Pos
string_compare_with_line_column(String str_a, String str_b, b32 case_ins = false)
{
    // TODO(f0): column counter that works with utf8
    
    Compare_Line_Pos result = {};
    result.is_equal = true;
    result.line = 1;
    result.column = 1;
    
    u64 size = pick_smaller(str_a.size, str_b.size);
    u64 i = 0;
    for (; i < size; ++i)
    {
        u8 a = str_a.str[i];
        u8 b = str_b.str[i];
        
        if (case_ins)
        {
            a = get_lower(a);
            b = get_lower(b);
        }
        
        if (a != b)
        {
            result.is_equal = false;
            break;
        }
        
        result.column += 1;
        if (a == '\n')
        {
            result.line += 1;
            result.column = 1;
        }
    }
    
    if (result.is_equal &&
        str_a.size != str_b.size)
    {
        result.is_equal = false;
    }
    
    return result;
}




internal b32
string_starts_with(String big, String small, b32 case_ins = false)
{
    b32 result = false;
    if (big.size >= small.size)
    {
        result = true;
        for(u64 i = 0; i < small.size; ++i)
        {
            auto b = big.str[i];
            auto s = small.str[i];
            
            if (case_ins)
            {
                b = get_lower(b);
                s = get_lower(s);
            }
            
            if (b != s)
            {
                result = false;
                break;
            }
        }
    }
    
    return result;
}


internal b32
string_ends_with(String big, String small, b32 case_ins = false)
{
    b32 result = false;
    if (big.size >= small.size)
    {
        result = true;
        for(u64 i = 1; i <= small.size; ++i)
        {
            u8 b = big.str[big.size - i];
            u8 s = small.str[small.size - i];
            
            if (case_ins)
            {
                b = get_lower(b);
                s = get_lower(s);
            }
            
            if (b != s)
            {
                result = false;
                break;
            }
        }
    }
    
    return result;
}








internal Find_Index
string_find_index_from_left(String big, char c, b32 case_ins = false)
{
    Find_Index result = {};
    
    if (case_ins) { c = get_lower(c); }
    
    for (u64 i = 0; i < big.size; ++i)
    {
        u8 b = big.str[i];
        if (case_ins) { b = get_lower(b); }
        
        if (b == c)
        {
            result.index = i;
            result.found = true;
            break;
        }
    }
    return result;
}




internal String
string_find_from_right_trim_ending(String source, char trim_point)
{
    String result = source;
    
    for (u64 negative = 1;
         negative <= result.size;
         ++negative)
    {
        u64 index = result.size - negative;
        u8 c = result.str[index];
        if (c == trim_point)
        {
            result.size = index;
            break;
        }
    }
    
    return result;
}



internal String
string_get_file_name_from_path(String source)
{
    String result = source;
    
    u64 offset = 0;
    
    for (u64 negative = 1;
         negative <= result.size;
         ++negative)
    {
        u64 index = result.size - negative;
        u8 c = result.str[index];
        if (is_slash(c))
        {
            offset = index + 1;
            break;
        }
    }
    
    result.str += offset;
    result.size -= offset;
    return result;
}


internal String
string_trim_file_name_from_path(String source)
{
    String result = source;
    
    for (u64 negative = 1;
         negative <= result.size;
         ++negative)
    {
        u64 index = result.size - negative;
        if (is_slash(result.str[index]))
        {
            result.size = index + 1;
            break;
        }
    }
    
    return result;
}


inline String
string_advance_str(String input, u64 distance)
{
    distance = pick_smaller(distance, input.size);
    String result = {};
    result.str = input.str + distance;
    result.size = input.size - distance;
    return result;
}


internal u64
string_count_common_characters(String value, String character_table)
{
    u64 result = 0;
    
    for (u64 value_index = 0;
         value_index < value.size;
         ++value_index)
    {
        for (u64 table_index = 0;
             table_index < character_table.size;
             ++table_index)
        {
            if (value.str[value_index] == character_table.str[table_index])
            {
                result += 1;
            }
        }
    }
    
    return result;
}










//~ ====================== @Level_Memory ======================
#if Stf0_Level >= 30
//~ ======================== @Level_30 ========================

// ======================= @Memory_Arena ======================

struct Arena
{
    void *base;
    u64 position;
    u64 capacity;
    //
    u64 reserved_capacity;
    u64 page_size;
    //
    s32 stack_count; // NOTE(f0): safety/testing variable
    u32 _padding;
};



// ======================= @Memory_Scope ======================
struct Memory_Scope
{
    Arena *arena_copy;
    u64 position;
    s32 stack_count;
    u32 _padding;
    
    Memory_Scope(Arena* arena)
    {
        arena_copy = arena;
        position = arena->position;
        stack_count = arena->stack_count++;
    }
    
    ~Memory_Scope()
    {
        --arena_copy->stack_count;
        assert(arena_copy->stack_count == stack_count);
        arena_copy->position = position;
    }
};

#define memory_scope(Arena) Memory_Scope glue(memory_scope_, __COUNTER__)(Arena)








// ======================= @Memory_Push =======================

#define push_array(ArenaPtr, Type, Count)\
(Type *)push_bytes_((ArenaPtr), (sizeof(Type)*(Count)), alignof(Type))

#define push_array_align(ArenaPtr, Type, Count, Align)\
(Type *)push_bytes_((ArenaPtr), (sizeof(Type)*(Count)), Align)

#define push_array_clear(ArenaPtr, Type, Count)\
(Type *)push_bytes_clear_((ArenaPtr), (sizeof(Type)*(Count)), alignof(Type))

#define push_array_clear_align(ArenaPtr, Type, Count, Align)\
(Type *)push_bytes_clear_((ArenaPtr), (sizeof(Type)*(Count)), Align)

// ====================== @Memory_Alloca ======================
#define push_stack_array(Type, Count) (Type *)alloca(sizeof(Type)*Count)




inline u64
get_aligment_offset(Arena *arena, u64 aligment)
{
    u64 aligment_offset = 0;
    u64 result_pointer = (u64)arena->base + arena->position;
    u64 aligment_mask = aligment - 1;
    
    if (result_pointer & aligment_mask)
    {
        aligment_offset = aligment - (result_pointer & aligment_mask);
    }
    
    return aligment_offset;
}


// =================== @Memory_Virtual_Arena ==================
inline u64
round_up_to_page_size(Arena *arena, u64 value)
{
    u64 result = align_bin_to(value, arena->page_size);
    return result;
}

inline void
commit_virtual_memory_(Arena *arena, u64 position_required_to_fit)
{
    assert(position_required_to_fit < arena->reserved_capacity);
    u64 target_capacity = 2 * position_required_to_fit;
    target_capacity = pick_smaller(target_capacity, arena->reserved_capacity);
    
    u64 commit_end_position = round_up_to_page_size(arena, target_capacity);
    u64 commit_size = commit_end_position - arena->capacity;
    u8 *commit_address = (u8 *)arena->base + arena->capacity;
    
    void *commit_result = VirtualAlloc(commit_address, commit_size, MEM_COMMIT, PAGE_READWRITE);
    assert(commit_result);
    arena->capacity = commit_end_position;
}

internal void *
push_bytes_virtual_commit_(Arena *arena, u64 alloc_size, u64 alignment)
{
    assert(arena->base);
    u64 alignment_offset = get_aligment_offset(arena, alignment);
    u64 future_position = arena->position + alignment_offset + alloc_size;
    
    if (future_position > arena->capacity)
    {
        commit_virtual_memory_(arena, future_position);
    }
    
    void *result = (u8 *)arena->base + arena->position + alignment_offset;
    arena->position = future_position;
    
    return result;
}

internal void *
push_bytes_virtual_commit_unaligned_(Arena *arena, u64 alloc_size)
{
    assert(arena->base);
    u64 future_position = arena->position + alloc_size;
    
    if (future_position > arena->capacity)
    {
        commit_virtual_memory_(arena, future_position);
    }
    
    void *result = (u8 *)arena->base + arena->position;
    arena->position = future_position;
    return result;
}






// ================ @Memory_Arena_Constructors ================

internal Arena
create_virtual_arena(u64 target_reserved_capacity = gigabytes(16))
{
    // TODO(f0): what about MEM_LARGE_PAGES?
    
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    
    Arena arena = {};
    arena.page_size = system_info.dwPageSize;
    arena.reserved_capacity = round_up_to_page_size(&arena, target_reserved_capacity);
    
    arena.base = VirtualAlloc(nullptr, arena.reserved_capacity, MEM_RESERVE, PAGE_READWRITE);
    assert(arena.base);
    return arena;
}

inline void *
push_bytes_(Arena *arena, u64 alloc_size, u64 alignment)
{
    void *result = push_bytes_virtual_commit_(arena, alloc_size, alignment);
    return result;
}

inline void *
push_bytes_clear_(Arena *arena, u64 alloc_size, u64 alignment)
{
    void *result = push_bytes_virtual_commit_(arena, alloc_size, alignment);
    clear_bytes(result, alloc_size);
    return result;
}









// ======================= @Memory_Lists ======================


// =================== @Memory_Virtual_Array ==================
template <typename T>
struct Virtual_Array
{
    union {
        Arena arena;
        T *array;
    };
    u64 count;
    
    inline void reset()
    {
        arena.position = 0;
        count = 0;
    }
    
    inline T *at(u64 index)
    {
        assert(index < count);
        T *result = array + index;
        return result;
    }
    
    inline T *grow(u64 grow_count = 1)
    {
        assert(grow_count > 0);
        T *result = array + count;
        push_bytes_virtual_commit_unaligned_(&arena, sizeof(T)*grow_count);
        count += grow_count;
        return result;
    }
};


template <typename T>
internal Virtual_Array<T>
create_virtual_array(u64 initial_count = 0,
                     u64 target_reserved_capacity = gigabytes(8))
{
    Virtual_Array<T> result = {};
    result.arena = create_virtual_arena(target_reserved_capacity);
    if (initial_count > 0)
    {
        result.grow(initial_count);
    }
    return result;
}








// ==================== @Memory_Linked_List ===================
template <typename T>
struct Linked_List_Node
{
    Linked_List_Node<T> *next;
    T item;
};

template <typename T>
struct Linked_List
{
    Linked_List_Node<T> *first;
    Linked_List_Node<T> *last;
    u64 node_count;
    
    inline Linked_List_Node<T> *push_get_node(Arena *arena)
    {
        if (node_count)
        {
            assert(first);
            assert(last);
            last->next = push_array(arena, Linked_List_Node<T>, 1);
            last = last->next;
        }
        else
        {
            first = push_array(arena, Linked_List_Node<T>, 1);
            last = first;
        }
        
        ++node_count;
        last->next = nullptr;
        return last;
    }
    
    inline T *push_get_item(Arena *arena)
    {
        auto result = push_get_node(arena);
        return &result->item;
    }
};














//~ ======================= @Level_Alloc ======================
#if Stf0_Level >= 40
//~ ======================== @Level_40 =========================


// Types
struct Directory
{
    String *names;
    u64 name_count;
};

struct Path
{
    Directory directory;
    String file_name;
};


typedef Linked_List<Path> Path_List;
typedef Linked_List<String> String_List;





// ======================= @Alloc_Basic =======================

inline String
alloc_string(Arena *arena, u64 size)
{
    String result = {
        push_array(arena, u8, size),
        size
    };
    return result;
}
inline String
push_string(Arena *arena, u64 size) {
    return alloc_string(arena, size);
}


internal String
push_string_copy(Arena *arena, String source)
{
    String result = {
        push_array(arena, u8, source.size),
        source.size
    };
    copy_array(result.str, source.str, u8, source.size);
    return result;
}


internal char *
push_cstr_copy(Arena *arena, char *source, s64 overwrite_len = -1)
{
    u64 len = (u64)((overwrite_len > 0) ? overwrite_len : cstr_length(source));
    char *result = push_array(arena, char, len + 1);
    copy_array(result, source, char, len);
    result[len] = 0;
    return result;
}


internal char *
push_cstr_from_string(Arena *arena, String string)
{
    char *cstr = push_array(arena, char, string.size+1);
    copy_array(cstr, string.str, char, string.size);
    cstr[string.size] = 0;
    return cstr;
}









// ===================== @Alloc_Directory =====================

internal Directory
push_directory_from_string(Arena *arena, String source)
{
    Directory result = {};
    result.name_count = string_count_common_characters(source, lit2str("/\\")) + 1;
    if (is_slash(source.str[source.size-1]))
    {
        result.name_count -= 1;
    }
    
    result.names = push_array(arena, String, result.name_count);
    
    u64 current_p = 0;
    for_u64(name_index, result.name_count)
    {
        u64 start_p = current_p;
        String element = string_advance_str(source, start_p);
        
        for (;
             current_p < source.size;
             ++current_p)
        {
            if (is_slash(source.str[current_p]))
            {
                element.size = (current_p++ - start_p);
                break;
            }
        }
        
        result.names[name_index] = element;
    }
    
    return result;
}


internal Directory
push_directory_append(Arena *arena, Directory parent_directory, String sub_directory_name)
{
    Directory result = {};
    result.name_count = parent_directory.name_count + 1;
    result.names = push_array(arena, String, result.name_count);
    for_u64(name_index, parent_directory.name_count)
    {
        result.names[name_index] = parent_directory.names[name_index];
    }
    result.names[result.name_count-1] = sub_directory_name;
    return result;
}



inline u64
get_directory_names_length_sum(Directory directory)
{
    u64 result = 0;
    for_u64(name_index, directory.name_count)
    {
        result += directory.names[name_index].size;
    }
    return result;
}

inline u64
get_directory_string_length(Directory directory)
{
    u64 result = get_directory_names_length_sum(directory);
    result += directory.name_count;
    return result;
}




internal String
push_string_from_directory(Arena *arena, Directory directory,
                           b32 use_windows_slash = (Native_Slash_Char == '\\'))
{
    u8 slash = (u8)(use_windows_slash ? '\\' : '/');
    
    u64 result_index = 0;
    String result = push_string(arena, get_directory_string_length(directory));
    result.str[result.size] = 0;
    
    for_u64(name_index, directory.name_count)
    {
        String *dir_name = directory.names + name_index;
        for_u64(char_index, dir_name->size)
        {
            result.str[result_index++] = dir_name->str[char_index];
        }
        result.str[result_index++] = slash;
    }
    
    result.str[result.size] = 0;
    assert(result.size == result_index);
    return result;
}

inline char *
push_cstr_from_directory(Arena *arena, Directory directory,
                         b32 use_windows_slash = (Native_Slash_Char == '\\'))
{
    String string = push_string_from_directory(arena, directory, use_windows_slash);
    char *result = (char *)string.str;
    return result;
}










// ======================== @Alloc_Path =======================

internal Path
push_path_from_string(Arena *arena, String source)
{
    Path result = {};
    String dir_string = string_trim_file_name_from_path(source);
    String file_name_string = string_advance_str(source, dir_string.size);
    
    result.directory = push_directory_from_string(arena, dir_string);
    result.file_name = file_name_string;
    
    return result;
}


inline Path
path_from_directory(Directory directory, String file_name)
{
    Path result = {
        directory,
        file_name
    };
    return result;
}
inline Path path(Directory directory, String file_name) {
    return path_from_directory(directory, file_name);
};

inline Path *
push_path_from_directory(Arena *arena, Directory directory, String file_name)
{
    Path *result = push_array(arena, Path, 1);
    *result = {
        directory,
        file_name
    };
    return result;
}
inline Path *push_path(Arena *arena, Directory directory, String file_name) {
    return push_path_from_directory(arena, directory, file_name);
};


inline u64
get_path_string_length(Path *path)
{
    assert(path);
    u64 result = get_directory_names_length_sum(path->directory);
    result += path->directory.name_count;
    result += path->file_name.size;
    return result;
}


internal u64
fill_string_from_path(void *output, u64 output_size,
                      Path *path, b32 use_windows_slash = (Native_Slash_Char == '\\'))
{
    assert(path);
    u8 slash = (u8)(use_windows_slash ? '\\' : '/');
    u64 out_index = 0;
    
    u8 *out = (u8 *)output;
    
    for_u64(name_index, path->directory.name_count)
    {
        String *dir_name = path->directory.names + name_index;
        for_u64(char_index, dir_name->size)
        {
            out[out_index++] = dir_name->str[char_index];
            
            if (out_index >= output_size) {
                return out_index;
            }
        }
        
        out[out_index++] = slash;
        
        if (out_index >= output_size) {
            return out_index;
        }
    }
    
    for_u64(char_index, path->file_name.size)
    {
        out[out_index++] = path->file_name.str[char_index];
        
        if (out_index >= output_size) {
            return out_index;
        }
    }
    
    assert(output_size == out_index);
    return out_index;
}


internal String
push_string_from_path(Arena *arena, Path *path,
                      b32 use_windows_slash = (Native_Slash_Char == '\\'))
{
    u64 pre_len = get_path_string_length(path);
    String result = push_string(arena, pre_len);
    
    u64 post_len = fill_string_from_path(result.str, result.size, path, use_windows_slash);
    assert(pre_len == post_len);
    
    return result;
}

inline char *
push_cstr_from_path(Arena *arena, Path *path,
                    b32 use_windows_slash = (Native_Slash_Char == '\\'))
{
    u64 pre_len = get_path_string_length(path);
    char *result = push_array(arena, char, pre_len + 1);
    
    u64 post_len = fill_string_from_path(result, pre_len, path, use_windows_slash);
    assert(pre_len == post_len);
    
    result[pre_len] = 0;
    return result;
}





// =================== @Alloc_String_Builder ==================
struct String_Builder_Item
{
    String string;
    b32 skip_following_separator;
    u32 _padding;
};

struct String_Builder
{
    Linked_List<String_Builder_Item> list;
    u64 string_length_sum;
    u64 separator_count;
    b32 last_has_separator;
    u32 _padding;
};


// NOTE(f0): Use if you manually mess with builder's items
internal void
builder_recalculate_length(String_Builder *builder)
{
    builder->string_length_sum = 0;
    builder->separator_count = 0;
    builder->last_has_separator = false;
    
    for_linked_list(node, builder->list)
    {
        builder->string_length_sum += node->item.string.size;
        builder->last_has_separator = !(node->item.skip_following_separator);
        builder->separator_count += (builder->last_has_separator ? 1 : 0);
    }
}

internal String_Builder_Item *
builder_add(Arena *arena, String_Builder *builder,
            String string, b32 skip_following_separator = false)
{
    String_Builder_Item *item = builder->list.push_get_item(arena);
    item->string = string;
    item->skip_following_separator = skip_following_separator;
    
    builder->string_length_sum += string.size;
    builder->last_has_separator = !skip_following_separator;
    if (builder->last_has_separator) {
        builder->separator_count += 1;
    }
    
    return item;
}

internal String
build_string(Arena *arena, String_Builder *builder, String separator = string(" "))
{
    u64 len = builder->string_length_sum + (builder->separator_count * separator.size);
    if (builder->last_has_separator) {
        len -= separator.size;
    }
    
    String result = alloc_string(arena, len+1);
    result.size -= 1;
    
    u64 char_index = 0;
    u64 node_index = 0;
    
    for (auto *node = builder->list.first;
         node;
         node = node->next, ++node_index)
    {
        String_Builder_Item *item = &node->item;
        copy_bytes(result.str + char_index, item->string.str, item->string.size);
        char_index += item->string.size;
        
        if (!item->skip_following_separator &&
            (node_index + 1) < builder->list.node_count)
        {
            copy_bytes(result.str + char_index, separator.str, separator.size);
            char_index += separator.size;
        }
    }
    
    assert(char_index == result.size);
    return result;
}

inline char *
build_cstr(Arena *arena, String_Builder *builder, String separator = string(" "))
{
    String string = build_string(arena, builder, separator);
    char *result = (char *)string.str;
    result[string.size] = 0;
    return result;
}







// ====================== @Alloc_Stringf ======================
internal String
push_stringf(Arena *arena, char *format, ...)
{
    va_list args;
    va_start(args, format);
    s32 len = vsnprintf(0, 0, format, args);
    assert(len >= 0);
    String result = alloc_string(arena, (u64)(len+1));
    vsnprintf((char *) result.str, result.size, format, args);
    --result.size;
    va_end(args);
    return result;
}



// NOTE(f0): copy stuff
internal char *
push_cstrf(Arena *arena, char *format, ...)
{
    va_list args;
    va_start(args, format);
    s32 size = vsnprintf(0, 0, format, args) + 1;
    assert(size >= 0);
    char *result = push_array(arena, char, size);
    vsnprintf(result, (u64)size, format, args);
    result[size-1] = 0;
    va_end(args);
    return result;
}





















//~ ===================== @Level_Platform =====================
#if Stf0_Level >= 50
//~ ======================== @Level_50 ========================

#if Def_Windows
struct File_Handle
{
    HANDLE handle_;
    // TODO(f0): keep 1 byte for error check and few bytes for file_name error reporting?
    b32 no_error;
    u32 _padding;
};
struct Pipe_Handle
{
    FILE *handle_;
    b32 no_error;
    u32 _padding;
};

#elif Def_Linux
#    error "not impl"
#endif
//=============================================================

// ===================== @Platform_Global =====================

struct Stf0_Global_State
{
    s64 perfomance_measurements_frequency;
};
global Stf0_Global_State _stf0_global;


internal void
stf0_initialize()
{
#if Def_Windows
    //=========
    LARGE_INTEGER large_perfomance_freq;
    QueryPerformanceFrequency(&large_perfomance_freq);
    _stf0_global.perfomance_measurements_frequency = large_perfomance_freq.QuadPart;
    //=========
#else
#error "not impl"
#endif
};



// ====================== @Platform_Time ======================
inline Time32ms
get_time32_ms()
{
#if Def_Windows
    Time32ms result = {timeGetTime()};
    
#else
#error "not impl"
    
#endif
    return result;
}

inline Time_Perfomance
get_perfomance_time()
{
    // NOTE(f0): Use get_time_elapsed to get delta between 2 measurements
    // NOTE(f0): Run stf0_initialize() before running get_seconds_elapsed!
#if Def_Windows
    LARGE_INTEGER large;
    b32 ret_code = QueryPerformanceCounter(&large);
    assert(ret_code);
    Time_Perfomance result = {large.QuadPart};
#else
#error "not impl"
#endif
    return result;
}

inline f32
get_seconds_elapsed(Time_Perfomance recent, Time_Perfomance old)
{
    // NOTE(f0): Run stf0_initialize() first!
    assert(_stf0_global.perfomance_measurements_frequency);
    f32 result = (f32)(recent.t_ - old.t_) / (f32)_stf0_global.perfomance_measurements_frequency;
    return result;
}



// ===================== @Platform_File_IO ====================
inline b32
is_valid_handle(File_Handle *file)
{
#if Def_Windows
    b32 result = (file->handle_ != INVALID_HANDLE_VALUE);
#else
#error "not implt"
#endif
    return result;
}

inline b32
no_errors(File_Handle *file)
{
    b32 result = file->no_error;
    return result;
}

inline void
set_error(File_Handle *file,
          cstr_lit message = "", cstr_lit path_cstr = "")
{
    // TODO(f0): Optional error logging here? Stf0_Log_Io_Errors?
    printf("[file(%s) error] %s;\n", path_cstr, message);
    debug_break();
    file->no_error = false;
}


//=============
internal File_Handle
file_open_read(cstr_lit path)
{
    File_Handle file = {};
#if Def_Windows
    file.handle_ = CreateFileA(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
#else
    file.handle_ = fopen(path, "rb");
#endif
    file.no_error = is_valid_handle(&file);
    return file;
}

internal File_Handle
file_open_read(Arena *arena, Path *path)
{
    memory_scope(arena);
    char *path_cstr = push_cstr_from_path(arena, path);
    File_Handle file = file_open_read(path_cstr);
    return file;
}





//=============
// NOTE(f0): Creates _new_ file (deletes previous content)
internal File_Handle
file_open_write(cstr_lit path)
{
    File_Handle file = {};
#if Def_Windows
    file.handle_ = CreateFileA(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
#else
    file.handle_ = fopen(path, "wb");
#endif
    file.no_error = is_valid_handle(&file);
    return file;
}

internal File_Handle
file_open_write(Arena *arena, Path *path)
{
    memory_scope(arena);
    char *path_cstr = push_cstr_from_path(arena, path);
    File_Handle file = file_open_write(path_cstr);
    return file;
}






//=============
internal File_Handle
file_open_append(cstr_lit path)
{
    File_Handle file = {};
#if Def_Windows
    file.handle_ = CreateFileA(path, FILE_APPEND_DATA, 0, nullptr, OPEN_ALWAYS, 0, nullptr);
#else
    file.handle_ = fopen(path, "ab");
#endif
    file.no_error = is_valid_handle(&file);
    return file;
}

internal File_Handle
file_open_append(Arena *arena, Path *path)
{
    memory_scope(arena);
    char *path_cstr = push_cstr_from_path(arena, path);
    File_Handle file = file_open_append(path_cstr);
    return file;
}






//=============
internal void
file_close(File_Handle *file)
{
#if Def_Windows
    CloseHandle(file->handle_);
#else
    fclose(file.handle_);
#endif
}






//=============
internal b32
file_copy(cstr_lit source, cstr_lit destination, b32 overwrite)
{
#if Def_Windows
    b32 fail_if_exists = !overwrite;
    b32 result = CopyFileA(source, destination, fail_if_exists);
#else
#error "not impl"
#endif
    return result;
}

internal b32
file_copy(Arena *arena, Path *source, Path *destination, b32 overwrite)
{
    memory_scope(arena);
    char *source_cstr = push_cstr_from_path(arena, source);
    char *destination_cstr = push_cstr_from_path(arena, destination);
    b32 result = file_copy(source_cstr, destination_cstr, overwrite);
    return result;
}






//=============
internal b32
file_hard_link(cstr_lit source_path, cstr_lit link_path)
{
#if Def_Windows
    b32 result = CreateHardLinkA(link_path, source_path, nullptr);
#else
#error "not impl"
#endif
    return result;
}

internal b32
file_hard_link(Arena *arena, Path *source_path, Path *link_path)
{
    memory_scope(arena);
    char *link_path_cstr = push_cstr_from_path(arena, link_path);
    char *source_path_cstr = push_cstr_from_path(arena, source_path);
    b32 result = file_hard_link(link_path_cstr, source_path_cstr);
    return result;
}






//=============
internal b32
file_delete(cstr_lit path)
{
#if Def_Windows
    b32 result = DeleteFileA(path);
#else
#error "not impl"
#endif
    return result;
}

internal b32
file_delete(Arena *arena, Path *path)
{
    memory_scope(arena);
    char *path_cstr = push_cstr_from_path(arena, path);
    b32 result = file_delete(path_cstr);
    return result;
}





//=============
internal b32
file_exists(cstr_lit path)
{
    b32 result = false;
    
#if Def_Windows
    File_Handle file = {CreateFileA(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr)};
#else
#error "not impl"
#endif
    
    if (is_valid_handle(&file)) {
        result = true;
        file_close(&file);
    }
    return result;
}

internal b32
file_exists(Arena *arena, Path *path)
{
    memory_scope(arena);
    char *path_cstr = push_cstr_from_path(arena, path);
    b32 result = file_exists(path_cstr);
    return result;
}






//=============
internal u64
file_read(File_Handle *file, void *buffer, u64 buffer_size)
{
    u64 result = 0;
    
    if (no_errors(file))
    {
#if Def_Windows
        DWORD bytes_read = 0;
        b32 success = ReadFile(file->handle_, buffer, safe_truncate_to_u32(buffer_size), &bytes_read, nullptr);
        if (!success) {
            set_error(file, "Failed read");
            result = 0;
        } else {
            result = bytes_read;
        }
        
#else
#error "not impl"
#endif
    }
    
    return result;
}






//=============
#define file_write_array(File, SourcePtr, Type, Count) file_write_bytes(File, SourcePtr, ((Count)*sizeof(Type)))

internal void
file_write_bytes(File_Handle *file, void *source, u64 size)
{
    assert(size <= U32_Max);
    
    if (no_errors(file))
    {
#if Def_Windows
        DWORD bytes_written = 0;
        b32 result = WriteFile(file->handle_, source, (DWORD)size, &bytes_written, nullptr);
        if (!result || (bytes_written != size)) {
            set_error(file, "Couldn't write");
        }
#else
#error "not impl"
#endif
    }
}
inline void file_write_string(File_Handle *file, String string) {
    file_write_bytes(file, string.str, string.size);
}





//=============
internal void
file_set_ending_to_current_distance(File_Handle *file)
{
    if (no_errors(file))
    {
#if Def_Windows
        b32 success = SetEndOfFile(file->handle_);
        if (!success) {
            set_error(file, "Couldn't set file ending");
        }
#else
#error "not impl"
#endif
    }
}


internal void
file_set_distance_from_start(File_Handle *file, s64 distance)
{
    if (no_errors(file))
    {
#if Def_Windows
        LARGE_INTEGER offset;
        offset.QuadPart = distance;
        b32 success = SetFilePointerEx(file->handle_, offset, nullptr, FILE_BEGIN);
        if (!success) {
            set_error(file, "Couldn't set file distance pointer from start");
        }
#else
#error "not impl"
#endif
    }
}

internal void
file_set_distance_from_current(File_Handle *file, s64 distance)
{
    if (no_errors(file))
    {
#if Def_Windows
        LARGE_INTEGER offset;
        offset.QuadPart = distance;
        b32 success = SetFilePointerEx(file->handle_, offset, nullptr, FILE_CURRENT);
        if (!success) {
            set_error(file, "Couldn't set file distance pointer from start");
        }
#else
#error "not impl"
#endif
    }
}

internal void
file_set_distance_from_end(File_Handle *file, s64 distance)
{
    if (no_errors(file))
    {
#if Def_Windows
        LARGE_INTEGER offset;
        offset.QuadPart = distance;
        b32 result = SetFilePointerEx(file->handle_, offset, nullptr, FILE_END);
        if (!result) {
            set_error(file, "Couldn't set file distance pointer from start");
        }
#else
#error "not impl"
#endif
    }
}







struct Distance_Result
{
    s64 value;
    b32 success;
    u32 _padding;
};

internal Distance_Result
file_get_distance(File_Handle *file)
{
    Distance_Result result = {};
    if (no_errors(file))
    {
#if Def_Windows
        LARGE_INTEGER distance;
        result.success = SetFilePointerEx(file->handle_, {}, &distance, FILE_CURRENT);
        if (!result.success) {
            set_error(file, "Couldn't get file distance");
        }
        result.value = distance.QuadPart;
#else
#error "not impl"
#endif
    }
    return result;
}















//==================== @Platform_Directory ====================
internal b32
directory_create(cstr_lit directory_path)
{
    // NOTE(f0): Returns true if directory exists
#if Def_Windows
    s32 code = CreateDirectoryA(directory_path, nullptr);
    b32 result = (code == 0);
    if (!result)
    {
        DWORD error = GetLastError();
        result = (error == ERROR_ALREADY_EXISTS);
    }
    
#else
    s32 code = mkdir(path);
    b32 result = (code == 0);
    if (!result)
    {
        result = (errno == EEXIST); // this errno check won't compile?
    }
#endif
    
    return result;
}

internal b32
directory_create(Arena *arena, Directory directory)
{
    memory_scope(arena);
    char *dir_cstr = push_cstr_from_directory(arena, directory);
    b32 result = directory_create(dir_cstr);
    return result;
}



internal b32
directory_set_current(cstr_lit path)
{
#if Def_Windows
    b32 result = SetCurrentDirectory(path) != 0;
#else
#error "not impl"
#endif
    
    return result;
}

internal b32
directory_set_current(Arena *arena, Directory directory)
{
    memory_scope(arena);
    char *path_cstr = push_cstr_from_directory(arena, directory);
    b32 result = directory_set_current(path_cstr);
    return result;
}






// ==================== @Platform_File_Data ===================
internal u64
file_get_size(File_Handle *file)
{
    // NOTE(f0): Default to zero
    u64 result = 0;
    
    if (no_errors(file))
    {
#if Def_Windows
        LARGE_INTEGER size = {};
        b32 success = GetFileSizeEx(file->handle_, &size);
        if (success) {
            result = safe_truncate_to_u64((s64)size.QuadPart);
        } else {
            set_error(file, "Couldn't get file size");
        }
#else
        /* NOTE(f0): seeking size example
          
          fseek(file, 0, SEEK_END);
          s64 len = ftell(file);
          rewind(file);
          result = push_array(arena, char, len+1);
          fread(result, len, 1, file);
        */
#error "not impl"
#endif
    }
    
    return result;
}

























internal Path_List
push_find_path_list_in_directory(Arena *arena, Directory directory)
{
#if Def_Windows
    Path_List result = {};
    
    Path wildcard_path = path_from_directory(directory, lit2str("*"));
    
    u64 wildcard_len = get_path_string_length(&wildcard_path);
    char *wildcard_cstr = push_stack_array(char, wildcard_len + 1);
    fill_string_from_path(wildcard_cstr, wildcard_len, &wildcard_path);
    wildcard_cstr[wildcard_len] = 0;
    
    
    WIN32_FIND_DATAA data = {};
    HANDLE find_handle = FindFirstFileA(wildcard_cstr, &data);
    if (find_handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                String file_name_string = push_string_copy(arena, string(data.cFileName));
                *result.push_get_item(arena) = path_from_directory(directory, file_name_string);
            }
        } while (FindNextFileA(find_handle, &data) != 0);
        FindClose(find_handle);
    }
    
#else
#error "not impl"
#endif
    return result;
}


internal void
files_delete_matching(Arena *arena, Path *wildcard_file_name_path)
{
    memory_scope(arena);
    
#if Def_Windows
    char *search_cstr = push_cstr_from_path(arena, wildcard_file_name_path);
    Path path_copy = *wildcard_file_name_path;
    
    
    WIN32_FIND_DATAA data = {};
    HANDLE find_handle = FindFirstFileA(search_cstr, &data);
    if (find_handle != INVALID_HANDLE_VALUE)
    {
        do {
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                memory_scope(arena);
                path_copy.file_name = string(data.cFileName);
                char *delete_cstr = push_cstr_from_path(arena, &path_copy);
                DeleteFileA(delete_cstr);
            }
        } while (FindNextFileA(find_handle, &data) != 0);
        FindClose(find_handle);
    }
    
#else
#error "not impl"
#endif
}


internal void
directory_delete_all_files(Arena *arena, Directory directory)
{
    memory_scope(arena);
    Path wildcard_path = path_from_directory(directory, l2s("*"));
    files_delete_matching(arena, &wildcard_path);
}






// =============== @Platform_Directory_Functions ==============
internal Directory
push_current_working_directory(Arena *arena)
{
    // NOTE(f0): ends with Native_Slash;
#if Def_Windows
    u32 buffer_size = GetCurrentDirectory(0, nullptr);
    char *buffer = push_array(arena, char, buffer_size);
    u32 length = GetCurrentDirectory(buffer_size, buffer);
    Directory result = push_directory_from_string(arena, string(buffer, length));
    
#else
    char buffer[kilobytes(4)]; // this is lame af. Define max linux path
    char *cwd = getcwd(buffer, array_count(buffer));
    if (cwd == buffer)
    {
        s64 length = cstr_length(buffer);
        if (length > 0)
        {
            result = push_array(arena, char, length + 2);
            copy_array(result, buffer, char, length);
            result[length] = Native_Slash_Char;
            result[length + 1] = 0;
        }
    }
#endif
    return result;
}

internal Directory
push_current_executable_directory(Arena *arena)
{
#if Def_Windows
    char buffer[kilobytes(4)];
    DWORD len = GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
    String path = push_string_copy(arena, string(buffer, len));
    
    String path_no_file_name = string_trim_file_name_from_path(path);
    Directory result = push_directory_from_string(arena, path_no_file_name);
#else
#error "not impl"
#endif
    return result;
}

internal Path
push_current_executable_path(Arena *arena)
{
#if Def_Windows
    char buffer[kilobytes(4)];
    DWORD len = GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
    String path = push_string_copy(arena, string(buffer, len));
    
    String path_no_file_name = string_trim_file_name_from_path(path);
    Directory dir = push_directory_from_string(arena, path_no_file_name);
    
    String file_name = string_get_file_name_from_path(path);
    Path result = path_from_directory(dir, file_name);
        
#else
#error "not impl"
#endif
    return result;
}












// ====================== @Platform_Pipe ======================

inline b32
is_valid_handle(Pipe_Handle *pipe)
{
    b32 result = (pipe->handle_ != nullptr);
    return result;
}

inline b32
no_errors(Pipe_Handle *pipe)
{
    b32 result = pipe->no_error;
    return result;
}

inline void
set_error(Pipe_Handle *pipe,
          cstr_lit message = "", cstr_lit command_cstr = "")
{
    // TODO(f0): Optional error logging here? Stf0_Log_Io_Errors?
    printf("\n[pipe(%s) error] %s; ", command_cstr, message);
    debug_break();
    pipe->no_error = false;
}


internal Pipe_Handle
pipe_open_read(cstr_lit command)
{
#if Def_Windows
    Pipe_Handle result = {_popen(command, "rb")};
#else
    Pipe_Handle result = popen(command, "rb");
#endif
    result.no_error = is_valid_handle(&result);
    return result;
}



internal s32
pipe_close(Pipe_Handle *pipe)
{
#if Def_Windows
    s32 return_code = _pclose(pipe->handle_);
#else
    s32 return_code = pclose(pipe);
#endif
    return return_code;
}



internal char *
pipe_read_line(Pipe_Handle *pipe, char *buffer, s64 buffer_size)
{
    char *result = fgets(buffer, (s32)buffer_size, pipe->handle_);
    return result;
}









#if 0
// NOTE(f0): String alloc
internal String16
push_string16_from_string8(Arena *arena, String source)
{
#if Def_Windows
    Ensure(source.size < S32_Max);
    s32 count1 = MultiByteToWideChar(CP_UTF8, 0, (char *)source.str, (s32)source.size, 0, 0);
    
    Ensure(count1 != ERROR_INSUFFICIENT_BUFFER && count1 != ERROR_INVALID_FLAGS &&
           count1 != ERROR_INVALID_PARAMETER && count1 != ERROR_NO_UNICODE_TRANSLATION);
    
    String16 result = alloc_string(arena, u16, count1);
    s32 count2 = MultiByteToWideChar(CP_UTF8, 0, (char *)source.str, (s32)source.size,
                                     (wchar *)result.str, (s32)result.size);
    
    Ensure(count2 != ERROR_INSUFFICIENT_BUFFER && count2 != ERROR_INVALID_FLAGS &&
           count2 != ERROR_INVALID_PARAMETER && count2 != ERROR_NO_UNICODE_TRANSLATION);
    Ensure(count1 == count2);
    
#else
#error "not impl"
#endif
    
    return result;
}
#endif








// =================== @Platform_Console_App ==================

#define ensure(Condition) do{\
if (!(Condition)){\
printf("\n[Internal error] <%s>; ", File_Line);\
exit_error();\
}}while(0)

#define ensure_msg(Condition, ErrorText1) do{\
if (!(Condition)){\
printf("\n%s; ", ErrorText1);\
exit_error();\
}}while(0)






internal String_List
save_pipe_output(Arena *arena, Pipe_Handle *pipe)
{
    String_List result = {};
    char line_buffer[1024*8];
    while (pipe_read_line(pipe, line_buffer, sizeof(line_buffer)))
    {
        String line = push_string_copy(arena, string(line_buffer, cstr_length_trim_white_from_right(line_buffer)));
        *result.push_get_item(arena) = line;
    }
    return result;
}





//=============================
internal String
push_read_entire_file(Arena *arena, File_Handle *file)
{
    String result = {};
    if (no_errors(file))
    {
        result.size = file_get_size(file);
        result.str = push_array(arena, u8, result.size);
        u64 bytes_read = file_read(file, result.str, result.size);
        
        if (bytes_read != result.size) {
            set_error(file, "Couldn't read whole file");
        }
        
        result.size = bytes_read;
    }
    return result;
}

internal String
push_read_entire_file(Arena *arena, cstr_lit file_path)
{
    String result = {};
    File_Handle file = file_open_read(file_path);
    result = push_read_entire_file(arena, &file);
    file_close(&file);
    return result;
}

inline String
push_read_entire_file(Arena *arena, Path *path)
{
    char *path_cstr = push_cstr_from_path(arena, path);
    String result = push_read_entire_file(arena, path_cstr);
    return result;
}




//=============================
internal char *
push_read_entire_file_and_zero_terminate(Arena *arena, File_Handle *file)
{
    char *result = nullptr;
    if (no_errors(file))
    {
        u64 length = file_get_size(file);
        result = push_array(arena, char, length + 1);
        u64 bytes_read = file_read(file, result, length);
        
        if (bytes_read != length) {
            set_error(file, "Couldn't read whole file and zero terminate");
        }
        
        result[length] = 0;
    }
    return result;
}

internal char *
push_read_entire_file_and_zero_terminate(Arena *arena, cstr_lit file_path)
{
    char *result = nullptr;
    File_Handle file = file_open_read(file_path);
    result = push_read_entire_file_and_zero_terminate(arena, &file);
    file_close(&file);
    return result;
}

inline char *
push_read_entire_file_and_zero_terminate(Arena *arena, Path *path)
{
    char *path_cstr = push_cstr_from_path(arena, path);
    char *result = push_read_entire_file_and_zero_terminate(arena, path_cstr);
    return result;
}








//===========================================================
//===========================================================
#endif // 50: Platform
#endif // 40: String_Alloc
#endif // 30: Memory
#endif // 20: Basic
#endif // 10: Types
//=============
Stf0_Close_Namespace
//=============
#endif // Stf0_H