#include "main.h"

global Global_State global_state;
global Stubs global_stubs = {};

global Color_Pair color_pairs[Color_Count] = {
    {Color_Empty, ""},
    {Color_Reset, "\033[39m\033[49m"},
    
    {Color_Base, "\033[97m"},
    {Color_Dimmed, "\033[90m"},
    
    {Color_Date, "\033[33m"},
    {Color_AltDate, "\033[43m\033[30m"},
    
    {Color_Description, "\033[36m"},
    {Color_AltDescription, "\033[96m"},
    
    {Color_Positive, "\033[32m"},
    {Color_AltPositive, "\033[92m"},
    
    {Color_Negative, "\033[31m"},
    {Color_AltNegative, "\033[91m"},
    
    {Color_Error, "\033[41m\033[97m"},
    {Color_Warning, "\033[103m\033[30m"},
    
    {Color_HelpHeader, "\033[100m"},
    {Color_Bar, "\033[34m"},
};

#include "description.cpp"
#include "error.cpp"
#include "lexer.cpp"
#include "string.cpp"
#include "time.cpp"













internal void load_file(Program_State *state);


inline char *
get_color(Color_Code code)
{
#if Def_Slow
    s64 count = Color_Count;
    assert(count == array_count(color_pairs));
    assert(array_count(color_pairs) > code);
    assert(color_pairs[code].code_check == code);
#endif
    char *result = "";
    
    if (!global_state.colors_disabled) {
        result = color_pairs[code].value;
    }
    
    return result;
}

inline void
print_color(Color_Code code)
{
    if (!global_state.colors_disabled)
    {
        char *color = get_color(code);
        printf("%s", color);
    }
}





inline b32
no_errors(Record_Session *session)
{
    b32 result = session->no_errors;
    return result;
}


inline void
session_set_error(Record_Session *session, char *message)
{
    session->no_errors = false;
                                                               
    Token command_token = session->current_command_token;
    Find_Index find = index_of((char *)command_token.text.str, l2s("\0\n\r"));
    String at_str = string(command_token.text.str, find.index);
    
    
    print_color(Color_Error);
    if (command_token.line.row > 1)
    {
        printf("[Error] %s; at (line %u): %.*s", message, command_token.line.row, string_expand(at_str));
    }
    else
    {
        printf("[Error] %s; at: %.*s", message, string_expand(at_str));
    }
    print_color(Color_Reset);
    printf("\n");
}


inline Program_Scope
create_program_scope(Arena *arena, Virtual_Array<Record> *records)
{
    Program_Scope result = {};
    result.arena_scope = create_arena_scope(arena);
    result.records_scope = create_virtual_array_scope(records);
    return result;
}

inline void
pop_program_scope(Program_Scope *scope)
{
    assert(scope);
    pop_arena_scope(&scope->arena_scope);
    pop_virtual_array_scope(&scope->records_scope);
}













internal Record *
get_last_record(Record_Session *session)
{
    Record *result = nullptr;
    s64 count = session->records->count;
    if (count)
    {
        result = session->records->at(count - 1);
    }
    
    return result;
}




internal Parse_Number_Result
parse_number(u8 *src, s32 count)
{
    Parse_Number_Result result = {};
    
    s32 multiplier = 1;
    for (s32 index = count - 1; index >= 0; --index, multiplier *= 10)
    {
        if (!(src[index] >= '0' && src[index] <= '9'))
        {
            return result;
        }
        
        s32 to_add = (src[index] - '0') * multiplier;
        result.number += to_add;
    }
    
    result.is_valid = true;
    return result;
}

internal Parse_Date_Result
parse_date(Record_Session *session, Token token)
{
    assert(token.type == Token_Date);
    
    // NOTE: Supported format: 2020-12-31
    Parse_Date_Result result = {};
    
    if (token.text.size == (4 + 1 + 2 + 1 + 2))
    {
        u8 *text = token.text.str;
        tm date = {};
        
        // year
        auto year = parse_number(text, 4);
        date.tm_year = year.number - 1900;
        text += 4;
        
        b32 dash1 = is_date_separator(*text++);
        
        // month
        auto month = parse_number(text, 2);
        date.tm_mon = month.number - 1;
        text += 2;
        
        b32 dash2 = is_date_separator(*text++);
        
        // day
        auto day = parse_number(text, 2);
        date.tm_mday = day.number;
        
        
        result.date = platform_tm_to_time(&date);
        result.is_valid = (year.is_valid && month.is_valid && day.is_valid && dash1 && dash2);
    }
    
    if (!result.is_valid) {
        session_set_error(session, "Bad date format!");
    }
    
    return result;
}

internal Parse_Time_Result
parse_time(Record_Session *session, Token token)
{
    // NOTE: Supported format: 10:32, 02:00, 2:0, 120...
    Parse_Time_Result result = {};
    
    if (token.text.size > 0)
    {
        b32 had_first_colon = false;
        u32 multiplier = 1;
        
        for (s32 index = (s32)token.text.size - 1; index >= 0; --index)
        {
            char c = token.text.str[index];
            
            if (c >= '0' && c <= '9')
            {
                u32 digit_value = (c - '0') * multiplier * 60;
                
                if (had_first_colon)
                {
                    digit_value *= 60;
                }
                
                result.time += digit_value;
                multiplier *= 10;
            }
            else if (is_time_separator(c))
            {
                
                if (had_first_colon)
                {
                    return result;
                }
                else
                {
                    had_first_colon = true;
                    multiplier = 1;
                }
            }
        }
        
        result.is_valid = true;
    }
    
    if (!result.is_valid)
    {
        session_set_error(session, "Bad time format");
    }
    
    return result;
}



inline b32
are_in_same_day(Record *record_a, Record *record_b)
{
    b32 result = record_a->date == record_b->date;
    return result;
}



















internal Record_Range
get_records_range_for_starting_date(Program_State *state, u64 start_index)
{
    Record_Range result = {};
    b32 start_is_active = false;
    u64 index = start_index;
    
    if (index < state->records.count)
    {
        date64 date_begin = state->records.at(index)->date;
        
        for (;
             index < state->records.count;
             ++index)
        {
            Record *record = state->records.at(index);
            if (date_begin <= record->date)
            {
                result.date = record->date;
                result.first = index;
                
                if (record->type == Record_TimeStart)
                {
                    start_is_active = true;
                }
                break;
            }
        }
        
        
        for (;
             index < state->records.count;
             ++index)
        {
            Record *record = state->records.at(index);
            
            if (result.date != record->date)
            {
                result.one_past_last = result.next_day_first_record_index = index;
                
                if (start_is_active) {
                    result.one_past_last += 1;
                }
                break;
            }
            
            if (record->type == Record_TimeStart)
            {
                start_is_active = true;
            }
            else if (record->type == Record_TimeStop)
            {
                start_is_active = false;
            }
        }
        
        if (!result.one_past_last) {
            result.one_past_last = result.next_day_first_record_index = state->records.count;
        }
    }
    
    return result;
}


internal Range_u64
get_index_range_for_date_range(Program_State *state, date64 date_begin, date64 date_end)
{
    Range_u64 result = {};
    
    u64 index = 0;
    for (;
         index < state->records.count;
         ++index)
    {
        Record *record = state->records.at(index);
        if (date_begin <= record->date) {
            result.first = index;
            break;
        }
    }
    
    
    for (;
         index < state->records.count;
         ++index)
    {
        Record *record = state->records.at(index);
        if (date_end < record->date) {
            result.one_past_last = index;
            break;
        }
    }
    
    if (!result.one_past_last) {
        result.one_past_last = state->records.count;
    }
    
    return result;
}


inline void
print_description(Record *record, Color_Code color = Color_Description)
{
    String desc = record->desc;
    if (desc.size)
    {
        print_color(color);
        printf(" \"%.*s\"", string_expand(desc));
        print_color(Color_Reset);
    }
}

inline void
print_time_delta(Record *time_delta_record)
{
    s32 time = time_delta_record->value;
    char sign = (time < 0) ? '-' : '+';
    
    if (sign == '-') {
        print_color(Color_AltNegative);
    } else {
        print_color(Color_AltPositive);
    }
    
    Str32 time_str = get_time_string(time);
    printf("  %c%s", sign, time_str.str);
    print_color(Color_Reset);
    
    print_description(time_delta_record, Color_AltDescription);
}

inline void
print_defered_time_deltas(Linked_List<Record> *defered)
{
    for_linked_list_ptr(node, defered)
    {
        Record record = node->item;
        print_time_delta(&record);
    }
}




// TODO(f0): Add start index to avoid traversing whole array all the time
internal Process_Days_Result
process_days_from_range(Program_State *state,
                        date64 date_begin, date64 date_end,
                        String filter,
                        Process_Days_Options options)
{
#if Def_Slow
    u32 open_start_ending_should_happen_only_once_test = 0;
#endif
    
    enum Open_State
    {
        Open,
        Closed_PrePrint,
        Closed_PostPrint,
    };
    
    Arena *arena = &state->arena;
    arena_scope(arena);
    
    
    b32 should_print = (options >= ProcessDays_Print);
    
    
    Process_Days_Result result = {};
    
    b32 has_filter = (filter.size != 0);
    Range_u64 whole_range = get_index_range_for_date_range(state, date_begin, date_end);
    
    
    Record_Range range = get_records_range_for_starting_date(state, whole_range.first);
    
    for (;
         range.date <= date_end && range.date != 0;
         range = get_records_range_for_starting_date(state, range.next_day_first_record_index))
    {
        arena_scope(arena);
        
        Linked_List<Record> defered_time_deltas = {};
        Record *active_start = nullptr;
        Open_State open_state = Closed_PostPrint;
        s32 day_time_sum = 0;
        b32 day_header_printed = false;
        
        
        
        for (u64 index = range.first;
             index < range.one_past_last;
             ++index)
        {
            Record record = *state->records.at(index);
            
            if (has_filter)
            {
                if (!active_start && index == range.next_day_first_record_index) {
                    continue;
                }
                
                b32 matches_filter = equals(filter, record.desc);
                
                if (!active_start && !matches_filter) {
                    continue;
                }
                
                if (active_start && record.type == Record_TimeStart) {
                    record.type = Record_TimeStop;
                }
            }
            
            
            if (should_print && !day_header_printed)
            {
                day_header_printed = true;
                
                Str32 date_str = get_date_string(range.date);
                Str32 day_of_week = get_day_of_the_week_string(range.date);
                
                printf("\n");
                
                if (options == ProcessDays_PrintAltColor) {
                    print_color(Color_AltDate);
                } else {
                    print_color(Color_Date);
                }
                
                printf("%s %s", date_str.str, day_of_week.str);
                
                print_color(Color_Reset);
                printf("\n");
            }
            
            
            
            if (record.date != range.date &&
                active_start)
            {
                // NOTE: case: "start" ends on next/another day   
                assert(record.type == Record_TimeStop || record.type == Record_TimeStart);
                assert(active_start);
                
                day_time_sum += record.value - active_start->value;
                day_time_sum += safe_truncate_to_s32(record.date - active_start->date);
                
                if (should_print)
                {
                    Str32 time_str = get_time_string(record.value);
                    printf("%s", time_str.str);
                    
                    print_color(Color_Dimmed);
                    if (record.date != range.date + Days(1))
                    {
                        Str32 date_str = get_date_string(record.date);
                        printf(" (%s)", date_str.str);
                    }
                    else
                    {
                        printf(" (next day)");
                    }
                    print_color(Color_Reset);
                    
                    
                    print_description(active_start);
                    
                    print_defered_time_deltas(&defered_time_deltas);
                    // print all defers (TimeDelta & CountDelta) here
                    printf("\n");
                }
                
                active_start = nullptr;
            }
            else
            {
                if (open_state == Open)
                {
                    if (record.type == Record_TimeDelta)
                    {
                        day_time_sum += record.value;
                        // NOTE: case: this needs to be printed _after_ we print "stop"
                        *defered_time_deltas.append(arena) = record;
                    }
                    else if (record.type == Record_TimeStart ||
                             record.type == Record_TimeStop)
                    {
                        assert(active_start);
                        day_time_sum += record.value - active_start->value;
                        
                        
                        if (should_print)
                        {
                            if (record.type == Record_TimeStart) {
                                print_color(Color_Dimmed);
                            }
                            
                            Str32 time_str = get_time_string(record.value);
                            printf("%s", time_str.str);
                            
                            print_color(Color_Reset);
                            
                            
                            print_description(active_start);
                            
                            
                            print_defered_time_deltas(&defered_time_deltas);
                            // print CountDelta defers on new lines
                            printf("\n");
                        }
                        
                        
                        active_start = nullptr;
                    }
                    else
                    {
                        // defer2 counts?
                        assert(0);
                    }
                    
                    
                    if (record.type == Record_TimeStart) {
                        open_state = Closed_PostPrint;
                    } else if (record.type == Record_TimeStop) {
                        open_state = Closed_PrePrint;
                    }
                }
                
                
                if (open_state == Closed_PostPrint)
                {
                    if (record.type == Record_TimeDelta)
                    {
                        day_time_sum += record.value;
                        
                        if (should_print)
                        {
                            printf("      ");
                            print_time_delta(&record);
                            printf("\n");
                        }
                    }
                    else if (record.type == Record_TimeStart)
                    {
                        active_start = state->records.at(index);
                        open_state = Open;
                        
                        if (should_print)
                        {
                            Str32 time_str = get_time_string(record.value);
                            printf("%s -> ", time_str.str);
                        }
                    }
                    else if (record.type == Record_TimeStop)
                    {
                        // NOTE: case where day starts with stop (that ends start from previous day)
                    }
                    else
                    {
                        // print count\n
                        assert(0);
                    }
                }
                
                
                if (open_state == Closed_PrePrint) {
                    open_state = Closed_PostPrint;
                }
            }
        }
        
        
        if (active_start)
        {
#if Def_Slow
            open_start_ending_should_happen_only_once_test += 1;
            assert(open_start_ending_should_happen_only_once_test < 2);
#endif
            date64 today = get_today();
            s32 now_time = get_time();
            
            s32 local_sum = now_time - active_start->value;
            local_sum += safe_truncate_to_s32(today - active_start->date);
            
            
            day_time_sum += local_sum;
            result.time_assumed += local_sum;
            
            
            if (should_print)
            {
                print_color(Color_Dimmed);
                Str32 time_str = get_time_string(active_start->value);
                printf("%s ", time_str.str);
                
                if (local_sum < Days(1)) {
                    printf("(now)");
                } else {
                    print_color(Color_Error);
                    printf("(missing stop)");
                }
                
                print_color(Color_Reset);
                
                
                print_description(active_start);
                
                print_defered_time_deltas(&defered_time_deltas);
                // print CountDelta defers on new lines
                printf("\n");
            }
            
            
            active_start = nullptr;
        }
        
        
        
        if (should_print && day_header_printed)
        {
            // TODO(f0): Figure out missing ending stuff
            Str32 time = get_time_string(day_time_sum);
            Str128 bar = get_progress_bar_string(day_time_sum, MissingEnding_None);
            
            print_color(Color_Dimmed);
            print_color(Color_Positive);
            printf("Time total: %s  ", time.str);
            
            printf("%s", bar.str);
            
            print_color(Color_Reset);
            printf("\n");
        }
        
        
        result.time_total += day_time_sum;
        
        if (range.next_day_first_record_index >= whole_range.one_past_last) {
            break;
        }
    }
    
    
    result.next_day_record_index = range.next_day_first_record_index;
    return result;
}













struct Day_Sum_Result
{
    time32 sum;
    Missing_Ending missing_ending;
};

inline Day_Sum_Result
get_day_sum(Program_State *state, date64 date)
{
    Process_Days_Result days = process_days_from_range(state, date, date, {}, ProcessDays_Calculate);
    
    Day_Sum_Result result = {};
    result.sum = days.time_total - days.time_assumed;
    
    if (days.time_assumed > Days(1)) {
        result.missing_ending = MissingEnding_Critical;
    } else if (days.time_assumed > 0) {
        result.missing_ending = MissingEnding_Assumed;
    }
    
    return result;
}






































internal void
archive_current_file(Program_State *state, b32 long_format = false)
{
    // TODO(f0): calculate hash of the whole file for file_name
    
    date64 now = get_current_timestamp();
    Str128 timestamp = get_timestamp_string_for_file(now, long_format);
    
    String file_name = stringf(&state->arena, "%.*s_%s.txt",
                                    string_expand(state->title), timestamp.str);
    
    Path archive_path = get_path(state->archive_dir, file_name);
    file_copy(&state->arena, &state->input_path, &archive_path, false);
    
    if (long_format) 
    {
        printf("File archived as: %.*s\n", string_expand(file_name));
    }
}






internal b32
save_to_file(Program_State *state)
{
    arena_scope(&state->arena);
    archive_current_file(state);
    
    b32 success = false;
    
    File_Handle file = file_open_write(&state->arena, &state->input_path);
    if (no_errors(&file))
    {
        Simple_String_Builder builder = {};
        auto add = [&](String string) {
            builder_add(&state->arena, &builder, string);
        };
        
        b32 is_new_day = true;
        s64 active_day_index = 0;
        
        for_u64(record_index, state->records.count)
        {
            Record *record = state->records.at(record_index);
            
            // print day header comment
            if (is_new_day)
            {
                Str32 day_of_week = get_day_of_the_week_string(record->date);
                add(l2s("// "));
                add(copy_string(&state->arena, string(day_of_week.str)));
                add(l2s("\n"));
            }
            
            
            // print command
            char *command = NULL;
            if (record->type == Record_TimeStart)
            {
                command = "start";
            }
            else if (record->type == Record_TimeStop)
            {
                command = "stop";
            }
            else if (record->type == Record_TimeDelta)
            {
                if (record->value < 0) {
                    command = "sub";
                } else {
                    command = "add";
                }
            }
            else
            {
                assert(0);
                continue;
            }
            
            add(string(command));
            add(l2s(" "));
            
            // print date
            if (is_new_day)
            {
                Str32 date_str = get_date_string(record->date);
                add(copy_string(&state->arena, string(date_str.str)));
                add(l2s(" "));
            }
            
            
            // print time
            Str32 time_str = get_time_string(record->value);
            add(copy_string(&state->arena, string(time_str.str)));
            
            
            // print description
            //Description *desc = get_description(&state->desc_table, record->desc_hash); @desc
            if (record->desc.size)
            {
                add(l2s(" \""));
                add(record->desc);
                add(l2s("\""));
            }
            
            // new line
            add(l2s("\n"));
            
            
            // __prepare next iteration + get sum___
            if (record_index == state->records.count - 1) {
                is_new_day = true;
            } else {
                Record *next_record = state->records.at(record_index + 1);
                is_new_day = !are_in_same_day(record, next_record);
            }
            
            if (is_new_day)
            {
                Day_Sum_Result sum_result = get_day_sum(state, record->date);
                Str128 sum_bar = get_sum_and_progress_bar_string(sum_result.sum, sum_result.missing_ending);
                
                add(l2s("// "));
                add(copy_string(&state->arena, string(sum_bar.str)));
                add(l2s("\n\n"));
                active_day_index = record_index + 1;
            }
        }
        
        
        String output_string = build_string(&state->arena, &builder);
        file_write_string(&file, output_string);
        file_close(&file);
        
        if (no_errors(&file)) {
            success = true;
        }
        
        state->input_file_mod_time = platform_get_file_mod_time(&state->arena, &state->input_path);
    }
    else
    {
        char *input_path_cstr = cstr_from_path(&state->arena, &state->input_path);
        printf("Failed to write to file: %s\n", input_path_cstr);
    }
    
    return success;
}





internal void
add_record(Record_Session *session, Record *record)
{
    if (no_errors(session) && session->load_file_unresolved_errors) {
        session_set_error(session, "New records can't be added because file has unresolved errors");
    }
    
    if (!no_errors(session)) {
        return;
    }
    
    
    
    b32 allowed = false;
    Record *replace_at = nullptr;
    
    
    if (record->type == Record_TimeStop)
    {
        // Stop needs to follow Start
        if (session->active && session->active->type == Record_TimeStart)
        {
            if (session->active->date < record->date ||
                (session->active->date == record->date &&
                 session->active->value < record->value))
            {
                allowed = true;
            }
            else
            {
                session_set_error(session, "Stop needs to be after the most recent Start");
            }
        }
        else
        {
            session_set_error(session, "Stop can be used only when Start is active");
        }
    }
    else
    {
        if (!session->last)
        {
            allowed = true;
        }
        else
        {
            if (session->active && 
                session->active->type == Record_TimeStart &&
                record->type == Record_TimeDelta && 
                session->active->date < record->date)
            {
                session_set_error(session, "Add/Sub needs to have the same date as active Start");
            }
            else if (session->last->date < record->date)
            {
                allowed = true;
            }
            else if (session->last->date == record->date)
            {
                if (!session->active || record->type == Record_TimeDelta)
                {
                    allowed = true;
                }
                else if (session->active->date < record->date)
                {
                    allowed = true;
                }
                else if (session->active->date == record->date)
                {
                    if (record->type == Record_TimeStart)
                    {
                        if (session->active->value < record->value)
                        {
                            allowed = true;
                        }
                        else if (session->last->type == Record_TimeStop &&
                                 session->last->date == record->date &&
                                 session->last->value == record->value)
                        {
                            // Start can replace recent Stop
                            replace_at = session->last;
                            allowed = true;
                        }
                    }
                }
            }
        }
    }
    
    
    
    if (no_errors(session) && !allowed) {
        session_set_error(session, "Out of order input. Try \"edit\" command");
    }
    
    
    
    if (no_errors(session))
    {
        session->change_count += 1;;
        
        if (replace_at)
        {
            session->last = replace_at;
            session->active = replace_at;
        }
        else
        {
            session->last = session->records->grow();
            
            if (record->type == Record_TimeStart ||
                record->type == Record_TimeStop)
            {
                session->active = session->last;
            }
        }
        
        *session->last = *record;
    }
}

internal b32
fill_date_optional(Record_Session *session, Record *record)
{
    b32 success = false;
    Token token = peek_token(&session->lexer, 0);
    
    if (token.type == Token_Date)
    {
        advance(&session->lexer);
        
        Parse_Date_Result parsed_date = parse_date(session, token);
        if (parsed_date.is_valid)
        {
            record->date = parsed_date.date;
            success = true;
        }
    }
    else
    {
        if (!session->reading_from_file)
        {
            record->date = get_today();
            success = true;
        }
        else
        {
            Record *last_record = get_last_record(session);
            if (last_record)
            {
                record->date = last_record->date;
                success = true;
            }
            else
            {
                session_set_error(session, "First record needs to specify date");
            }
        }
    }
    
    return success;
}

internal b32
fill_time_optional(Record_Session *session, Record *record)
{
    b32 success = false;
    Token token = peek_token(&session->lexer, 0);
    
    if (token.type == Token_Time)
    {
        advance(&session->lexer);
        
        Parse_Time_Result parsed_time = parse_time(session, token);
        if (parsed_time.is_valid)
        {
            record->value = parsed_time.time;
            success = true;
        }
    }
    else
    {
        if (!session->reading_from_file)
        {
            record->value = get_time();
            success = true;
        }
    }
    
    return success;
}


internal b32
fill_time_required(Record_Session *session, Record *record)
{
    b32 success = false;
    Token token = peek_token(&session->lexer, 0);
    
    if (token.type == Token_Time)
    {
        advance(&session->lexer);
        
        Parse_Time_Result parsed_time = parse_time(session, token);
        if (parsed_time.is_valid)
        {
            record->value = parsed_time.time;
            success = true;
        }
    }
    
    return success;
}

internal void
fill_description_optional(Record_Session *session, Record *record)
{
    Token token = peek_token(&session->lexer, 0);
    
    if (token.type == Token_String)
    {
        advance(&session->lexer);
        //record->desc_hash = add_description(&session->desc_table, forward->token); @desc
        record->desc = token.text;
    }
}


internal void
prase_command_start(Record_Session *session)
{
    Record record = {};
    record.type = Record_TimeStart;
    
    b32 success = fill_date_optional(session, &record);
    
    if (success)
    {
        success = fill_time_optional(session, &record);
    }
    
    if (success)
    {
        fill_description_optional(session, &record);
    }
    
    if (success)
    {
        add_record(session, &record);
    }
    else
    {
        session_set_error(session, "Incorect command usage. Use:\n"
                          "start [yyyy-MM-dd] (hh:mm) [\"description\"]");
    }
}


internal void
prase_command_stop(Record_Session *session)
{
    Record record = {};
    record.type = Record_TimeStop;
    
    b32 success = fill_date_optional(session, &record);
    
    if (success)
    {
        success = fill_time_optional(session, &record);
    }
    
    if (success)
    {
        add_record(session, &record);
    }
    else
    {
        session_set_error(session, "Incorect command usage. Use:\n"
                          "start [yyyy-MM-dd] (hh:mm)");
    }
}



internal void
parse_command_add_sub(Record_Session *session, b32 is_add)
{
    Record record = {};
    record.type = Record_TimeDelta;
    
    b32 success = fill_date_optional(session, &record);
    
    if (success)
    {
        success = fill_time_required(session, &record);
        if (!is_add) record.value *= -1;
        
        if (success && record.value == 0)
        {
            session_set_error(session, "Time equal to zero!");
            return;
        }
    }
    
    if (success)
    {
        fill_description_optional(session, &record);
    }
    
    if (success)
    {
        add_record(session, &record);
    }
    else
    {
        char error_message[512];
        snprintf(error_message, sizeof(error_message),
                 "Incorect command usage. Use:\n"
                 "%s [yyyy-MM-dd] (hh:mm) [\"description\"]",
                 (is_add) ? "add" : "sub");
        
        session_set_error(session, error_message);
    }
}



internal Parse_Complex_Date_Result
parse_complex_date(Record_Session *session, Token token)
{
    // NOTE: Allows to use keywords like today, yesterday + normal date formats
    // TODO: ^ today+1 yesterday-4
    
    Parse_Complex_Date_Result result = {};
    
    if (token.type == Token_Date)
    {
        Parse_Date_Result parse = parse_date(session, token);
        result.date = parse.date;
        result.condition = (parse.is_valid) ? Con_IsValid : Con_HasErrors;
    }
    
    return result;
}


internal Date_Range_Result
get_max_date_range()
{
    Date_Range_Result result;
    result.condition = Con_IsValid;
    result.first = 1;
    result.last = S64_Max;
    
    return result;
}

internal Date_Range_Result
get_date_range(Record_Session *session)
{
    Date_Range_Result result = {};
    b32 success = true;
    b32 has_matching_token = false;
    
    {
        Token token = peek_token(&session->lexer, 0);
        
        if (token.type == Token_Identifier &&
            token_equals(token, "from"))
        {
            advance(&session->lexer);
            has_matching_token = true;
            
            Parse_Complex_Date_Result date = parse_complex_date(session, token);
            success = is_condition_valid(date.condition);
            if (success)
            {
                result.first = date.date;
                advance(&session->lexer);
            }
        }
    }
    
    
    
    if (success)
    {
        Token token = peek_token(&session->lexer, 0);
        
        if (token.type == Token_Identifier &&
            token_equals(token, "to"))
        {
            advance(&session->lexer);
            has_matching_token = true;
            
            Parse_Complex_Date_Result date = parse_complex_date(session, token);
            success = is_condition_valid(date.condition);
            if (success)
            {
                result.last = date.date;
                advance(&session->lexer);
            }
        }
    }
    
    
    
    if (success)
    {
        Token token = peek_token(&session->lexer, 0);
        
        if (!has_matching_token)
        {
            if (token_equals(token, "all"))
            {
                has_matching_token = true;
                result = get_max_date_range();
                advance(&session->lexer);
            }
            else
            {
                Parse_Complex_Date_Result date = parse_complex_date(session, token);
                result.condition = date.condition;
                if (is_condition_valid(result.condition))
                {
                    result.first = date.date;
                    result.last = date.date;
                    advance(&session->lexer);
                }
            }
        }
        else
        {
            result.condition = Con_IsValid;
        }
    }
    else
    {
        result.condition = Con_HasErrors;
    }
    
    
    return result;
}


internal Date_Range_Result
get_recent_days_range(Virtual_Array<Record> *records)
{
    date64 today = get_today();
    date64 start = today - Days(31);
    
    s64 record_count = records->count;
    if (record_count)
    {
        date64 last_date = records->at(record_count - 1)->date;
        if (last_date > today)
        {
            today = last_date;
        }
        
        s64 past_index = record_count - 120;
        if (past_index < 0)
        {
            past_index = 0;
        }
        
        date64 start_date = records->at(past_index)->date;
        if (start_date < start)
        {
            start = start_date;
        }
    }
    
    Date_Range_Result result = {start, today, Con_IsValid};
    return result;
}


internal void
parse_command_show(Program_State *state, Record_Session *session)
{
    Date_Range_Result range = get_date_range(session);
    
    if (range.condition != Con_HasErrors)
    {
        String filter = {};
        
        Token token = peek_token(&session->lexer, 0);
        if (token.type == Token_String) {
            advance(&session->lexer);
            filter = token.text;
        }
        
        char *message = nullptr;
        
        if (range.condition == Con_NoMatchigTokens) {
            if (filter.size)
            {
                range = get_max_date_range();
            }
            else
            {
                range = get_recent_days_range(session->records);
                message = "Range assumed from xxxx-xx-xx to xxxx-xx-xx; "
                    "Specify filter or use \"show all\" to use all records\n";
            }
        }
        
        process_days_from_range(state, range.first, range.last, filter, ProcessDays_Print);
        
        if (message) {
            print_color(Color_Dimmed);
            printf("%s", message);
            print_color(Color_Reset);
        }
    }
    else
    {
        session_set_error(session, "Incorect command usage. Use:\n"
                          "show [yyyy-MM-dd]\n"
                          "show [from yyyy-MM-dd] [to yyyy-MM-dd]\n");
    }
}


inline b32
increase_index_to_next_day(Virtual_Array<Record> *records, u64 *index)
{
    date64 start_date = records->at(*index)->date;
    
    for (;
         *index < records->count;
         ++(*index))
    {
        Record *record = records->at(*index);
        if (record->date > start_date)
        {
            return true;
        }
    }
    
    return false;
}




internal void
print_summary(Program_State *state, Granularity granularity,
              date64 date_begin, date64 date_end,
              String filter)
{
    if (state->records.count > 0)
    {
        Record *record = state->records.at(0);
        
        for (;;)
        {
            Boundries_Result boundries = {};
            switch (granularity)
            {
                case Granularity_Days: {
                    boundries.day_count = 1;
                    boundries.first = record->date;
                    boundries.last = record->date;
                } break;
                
                
                case Granularity_Months: {
                    boundries = get_month_boundries(record->date);
                } break;
                
                
                case Granularity_Years: {
                    boundries = get_year_boundries(record->date);
                } break;
                
                
                default: {
                    assert(0);
                } // fall
                
            }
            
            
            Process_Days_Result days =
                process_days_from_range(state, boundries.first, boundries.last, filter, ProcessDays_Calculate);
            
            if (days.time_total)
            {
                s32 day_count = boundries.day_count;
                
                if (days.next_day_record_index == state->records.count)
                {
                    date64 today = get_today();
                    s32 current_day_count = (s32)((today - boundries.first) / Days(1)) + 1;
                    
                    if (current_day_count > 0) {
                        day_count = pick_smaller(day_count, current_day_count);
                    }
                }
                
                
                Str32 date_str = get_date_string(boundries.first);
                Str32 sum_str = get_time_string(days.time_total);
                
                if (day_count > 1)
                {
                    s32 avg = days.time_total/day_count;
                    Str32 avg_str = get_time_string(avg);
                    Str128 bar = get_progress_bar_string(avg, MissingEnding_None); // TODO(f0): hack
                    
                    printf("%s\t"
                           "sum: %s\tavg(/%3d): "
                           "%s\t%s\n",
                           date_str.str,
                           sum_str.str, day_count,
                           avg_str.str, bar.str);
                }
                else
                {
                    Str128 bar = get_progress_bar_string(days.time_total, MissingEnding_None); // TODO(f0): hack
                    printf("%s\t"
                           "sum: %s\t"
                           "%s\n",
                           date_str.str,
                           sum_str.str,
                           bar.str);
                }
            }
            
            
            if (days.next_day_record_index >= state->records.count ||
                days.next_day_record_index == 0)
            {
                break;
            }
            
            record = state->records.at(days.next_day_record_index);
            if (record->date > date_end) {
                break;
            }
        }
    }
}





#if 0
internal void
process_and_print_summary2(Program_State *state, Granularity granularity, date64 date_begin, date64 date_end)
{
    u64 record_index = 0;
    
    // NOTE: Skip record before date_begin
    for (;
         record_index < state->records.count;
         ++record_index)
    {
        Record *record = state->records.at(record_index);
        if (date_begin <= record->date)
        {
            break;
        }
    }
    
    
    time32 sum = 0;
    Boundries_Result boundries = {};
    b32 loop = true;
    Record *record = state->records.at(record_index);
    
    while (loop)
    {
        loop = increase_index_to_next_day(&state->records, &record_index);
        
        if (loop)
        {
            record = state->records.at(record_index);
            if (date_end < record->date)
            {
                loop = false;
            }
        }
        
        
        
        b32 past_boundary = loop && (record->date >= boundries.one_day_past_end);
        
        
        // NOTE: Print line
        if (!loop || past_boundary)
        {
            s32 day_count = (s32)boundries.day_count;
            if (day_count > 0)
            {
                Str32 date_str = get_date_string(boundries.begin);
                
                date64 today = get_today();
                if (today < boundries.one_day_past_end && today >= boundries.begin)
                {
                    day_count = (s32)(today / (Days(1))) - (s32)(boundries.begin / (Days(1)));
                    
                    if (day_count <= 0)
                    {
                        day_count = 1;
                    }
                }
                
                time32 time_avg = sum / day_count;
                
                Str32 sum_str = get_time_string(sum);
                Str128 avg_bar = get_progress_bar_string(time_avg, MissingEnding_None);
                
                Str128 sum_bar;
                if (day_count >= 2)
                {
                    Str32 avg_str = get_time_string(time_avg);
                    
                    snprintf(sum_bar.str, sizeof(sum_bar.str), "sum: %s\tavg(/%d): %s\t%s", sum_str.str, day_count,
                             avg_str.str, avg_bar.str);
                }
                else
                {
                    snprintf(sum_bar.str, sizeof(sum_bar.str), "sum: %s\t%s", sum_str.str, avg_bar.str);
                }
                
                using namespace Color;
                printf("%s%s\t%s%s%s\n", f_date, date_str.str, f_sum, sum_bar.str, f_reset);
            }
            
            // NOTE: Clear sum
            sum = 0;
        }
        
        
        // NOTE: Summing
        if (loop)
        {
            Day_Sum_Result day_sum = get_day_sum(state, record->date);
            sum += day_sum.sum;
        }
        
        
        // NOTE: Update boundary
        if (loop && past_boundary)
        {
            switch (granularity)
            {
                case Granularity_Months: {
                    boundries = get_month_boundries(record->date);
                }
                break;
                
                default: {
                    assert(0);
                } // fall
                
                case Granularity_Days: {
                    boundries.day_count = 1;
                    boundries.begin = record->date;
                    boundries.one_day_past_end = record->date + Days(1);
                }
                break;
            }
        }
    }
}
#endif



internal void
parse_command_summary(Program_State *state, Record_Session *session)
{
    Granularity granularity = Granularity_Months;
    // TODO: Pull out granularity check.
    
    {
        Token token = peek_token(&session->lexer, 0);
        
        if (token.type == Token_Identifier)
        {
            if (token_equals(token, "days") ||
                token_equals(token, "d"))
            {
                advance(&session->lexer);
                granularity = Granularity_Days;
            }
            else if (token_equals(token, "months") ||
                     token_equals(token, "m"))
            {
                advance(&session->lexer);
                granularity = Granularity_Months;
            }
            else if (token_equals(token, "years") ||
                     token_equals(token, "y"))
            {
                advance(&session->lexer);
                granularity = Granularity_Years;
            }
        }
    }
    
    
    Date_Range_Result range = get_date_range(session);
    
    if (range.condition == Con_NoMatchigTokens) {
        range = get_max_date_range();
    }
    
    
    String filter = {};
    {
        Token token = peek_token(&session->lexer, 0);
        if (token.type == Token_String) {
            advance(&session->lexer);
            filter = token.text;
        }
    }
    
    
    print_summary(state, granularity, range.first, range.last, filter);
    
    
    if (range.condition == Con_HasErrors)
    {
        session_set_error(session, "Incorect command usage. Use:\n"
                          "summary [granularity]\n"
                          "summary [granularity] [yyyy-MM-dd]\n"
                          "summary [granularity] [from yyyy-MM-dd] [to yyyy-MM-dd]\n"
                          "\n\tgranularity - days/months (or d/m)\n");
    }
}


internal void
parse_command_exit(Program_State *state, Record_Session *session)
{
    Token token = peek_token(&session->lexer, 0);
    
    if ((token.type == Token_Identifier) && (token_equals(token, "no-save")))
    {
        advance(&session->lexer);
        exit(0);
    }
    else
    {
        if (no_errors(session))
        {
            if (session->change_count > 0)
            {
                b32 save_result = save_to_file(state);
                
                if (save_result) {
                    exit(0);
                } else {
                    session_set_error(session, "Failed to save to file!");
                }
            }
            else
            {
                exit(0);
            }
        }
        else
        {
            session_set_error(session, "Errors detected, exit aborted");
        }
    }
}


internal Record_Session
create_record_session_no_lexer(Arena *arena, Virtual_Array<Record> *records,
                                   b32 reading_from_file)
{
    Record_Session result = {};
    result.records = records;
    result.scope = create_program_scope(arena, records);
    
    for_u64(i, records->count)
    {
        u64 index = records->count - i - 1;
        Record *record = records->at(index);
        
        if (!result.last) {
            result.last = record;
        }
        
        if (!result.active) { 
            if (record->type == Record_TimeStart || record->type == Record_TimeStop) {
                result.active = record;
                break;
            }
        }
    }
    
    result.no_errors = true;
    result.reading_from_file = reading_from_file;
    return result;
}


internal Record_Session
create_record_session(Arena *arena, Virtual_Array<Record> *records,
                      b32 reading_from_file, char *content)
{
    Record_Session result = create_record_session_no_lexer(arena, records, reading_from_file);
    result.lexer = create_lexer(content);
    return result;
}


internal void
process_input(Program_State *state, Record_Session *session)
{
#define Error_Cmd_Exclusive session_set_error(session, "This command can be used only from console")
    
    if (state->load_file_error) {
        session->load_file_unresolved_errors = true;
    }
    
    b32 parsing = true;
    b32 reading_from_file = session->reading_from_file;
    
    while (parsing)
    {
        Token token = peek_token(&session->lexer, 0);
        advance(&session->lexer);
        session->current_command_token = token;
        
        switch (token.type)
        {
            case Token_Identifier: {
                if (token_equals(token, "start"))
                {
                    prase_command_start(session);
                }
                else if (token_equals(token, "stop"))
                {
                    prase_command_stop(session);
                }
                else if (token_equals(token, "add"))
                {
                    parse_command_add_sub(session, true);
                }
                else if (token_equals(token, "subtract") || token_equals(token, "sub"))
                {
                    parse_command_add_sub(session, false);
                }
                else if (token_equals(token, "show"))
                {
                    if (reading_from_file) {
                        Error_Cmd_Exclusive;
                    } else {
                        parse_command_show(state, session);
                    }
                }
                else if (token_equals(token, "summary"))
                {
                    if (reading_from_file) {
                        Error_Cmd_Exclusive;
                    } else {
                        parse_command_summary(state, session);
                    }
                }
                else if (token_equals(token, "exit"))
                {
                    if (reading_from_file) {
                        Error_Cmd_Exclusive;
                    } else {
                        parse_command_exit(state, session);
                    }
                }
                else if (token_equals(token, "save"))
                {
                    if (reading_from_file) {
                        Error_Cmd_Exclusive;
                    } else {
                        b32 save_result = save_to_file(state);
                        if (save_result) {
                            printf("File saved\n");
                        } else {
                            printf("Failed to save\n");
                        }
                    }
                }
                else if (token_equals(token, "archive"))
                {
                    if (reading_from_file) {
                        Error_Cmd_Exclusive;
                    } else {
                        archive_current_file(state, true);
                    }
                }
                else if (token_equals(token, "reload"))
                {
                    if (reading_from_file) {
                        Error_Cmd_Exclusive;
                    } else {
                        if (no_errors(session)) {
                            load_file(state);
                            return;
                        } else {
                            session_set_error(session, "Load aborted");
                        }
                    }
                }
                else if (token_equals(token, "time"))
                {
                    if (reading_from_file) {
                        Error_Cmd_Exclusive;
                    } else {
                        date64 now = get_current_timestamp();
                        Str128 now_str = get_timestamp_string(now);
                        printf("Current time: %s\n", now_str.str);
                    }
                }
                else if (token_equals(token, "edit"))
                {
                    if (reading_from_file) {
                        Error_Cmd_Exclusive;
                    } else {
                        // TODO(f0): should work with Path
                        char *input_cstr = cstr_from_path(&state->arena, &state->input_path);
                        platform_open_in_default_editor(input_cstr);
                    }
                }
                else if (token_equals(token, "dir"))
                {
                    if (reading_from_file) {
                        Error_Cmd_Exclusive;
                    } else {
                        char *dir_cstr = cstr_from_directory(&state->arena, state->exe_path.directory);
                        platform_open_in_default_editor(dir_cstr);
                    }
                }
                else if (token_equals(token, "clear"))
                {
                    if (reading_from_file) {
                        Error_Cmd_Exclusive;
                    } else {
                        platform_clear_screen();
                    }
                }
                else if (token_equals(token, "help"))
                {
                    if (reading_from_file)
                    {
                        Error_Cmd_Exclusive;
                    }
                    else
                    {
                        print_help_desc("[...] - optional, (...) - required\n");
                        print_help_header("Commands available everywhere");
                        print_help_item("start", "[yyyy-MM-dd] (hh:mm) [\"description\"]", "starts new timespan");
                        print_help_item("stop", "[yyyy-MM-dd] (hh:mm) [\"description\"]", "stops current timespan");
                        
                        print_help_item("add", "[yyyy-MM-dd] (hh:mm) [\"description\"]", "adds arbitrary amount of time");
                        print_help_item("sub", "[yyyy-MM-dd] (hh:mm) [\"description\"]",
                                        "subtracts arbitrary amount of time");
                        
                        print_help_desc("add/sub commands support inputs formatted as minutes (125) or hours (2:05)");
                        
                        
                        printf("\n");
                        print_help_header("Console only commands");
                        print_help_desc("When using commands from above in console you can skip (hh:mm) requirement");
                        
                        print_help_item("show", "[yyyy-MM-dd]", "\t\t\tshows current history");
                        print_help_item("show", "[from yyyy-MM-dd] [to yyyy-MM-dd]", "");
                        print_help_item("summary", "[d/m] [from yyyy-MM-dd] [to yyyy-MM-dd]", "sums hours for day/month");
                        
                        printf("\n");
                        print_help_item("edit", nullptr,
                                        "opens database file in your default editor"
                                        "\n\tworks best if your editor supports hot-loading");
                        // printf("\n");
                        print_help_item("dir", nullptr, "opens directory with exe");
                        print_help_item("clear", nullptr, "clears the screen");
                        print_help_item("time", nullptr, "shows current time");
                        
                        print_help_desc("\nFollowing commands execute automatically but can be also called manually");
                        print_help_item("save", nullptr, "forces save");
                        print_help_item("archive", nullptr, "forces backup");
                        print_help_item("load", nullptr, "forces load from file");
                    }
                }
                else
                {
                    char error_message[512];
                    snprintf(error_message, sizeof(error_message),
                             "%.*s - unexpected identifier", string_expand(token.text));
                    
                    session_set_error(session, error_message);
                }
            }
            break;
            
            
            case Token_End_Of_Stream: {
                parsing = false;
            }
            break;
            
            case Token_Semicolon: {
            }
            break;
            
            default: {
                char error_message[512];
                snprintf(error_message, sizeof(error_message),
                         "%.*s - unexpected element", string_expand(token.text));
                
                session_set_error(session, error_message);
            }
            break;
        }
    }
    
    
    if (no_errors(session))
    {
        if (!reading_from_file &&
            session->change_count > 0)
        {
            save_to_file(state);
        }
    }
    else
    {
        print_color(Color_Warning);
        printf("[Warning] Records not added due to errors");
        print_color(Color_Reset);
        printf("\n");
        pop_program_scope(&session->scope);
    }
}


internal void
clear_program_state(Program_State *state)
{
    // TODO(f0): Don't leak memory!
    clear_table(&state->desc_table);
    state->records.reset_memory();
}



internal void
load_file(Program_State *state)
{
    pop_program_scope(&state->initial_scope);
    state->load_file_error = false;
    
    b32 load_successful = false;
    
    for (u32 load_tries = 0;
         (load_tries < 5 && !load_successful);
         ++load_tries)
    {
        char *file_content = read_entire_file_and_zero_terminate(&state->arena, &state->input_path);
        
        if (file_content)
        {
            Record_Session session = create_record_session(&state->arena, &state->records, true, file_content);
            process_input(state, &session);
            
            if (no_errors(&session))
            {
                printf("File loaded\n");
            }
            else
            {
                state->load_file_error = true;
                printf("[Warning] File contains errors. It requires manual fixing. "
                       "Use \"edit\" to open it in default editor\n");
            }
            
            load_successful = true;
        }
        
        if (!load_successful) {
            platform_sleep(10);
        }
    }
    
    
    if (!load_successful)
    {
        state->load_file_error = true;
        arena_scope(&state->arena);
        char *file_name = cstr_from_path(&state->arena, &state->input_path);
        printf("[Critial error] Failed to load from file: %s\n", file_name);
    }
    
    
    state->input_file_mod_time = platform_get_file_mod_time(&state->arena, &state->input_path);
}


internal void
read_from_keyboard(Thread_Memory *thread_memory)
{
    for (;;)
    {
        if (thread_memory->new_data)
        {
            platform_sleep(1);
            // NOTE: Spinlock while waiting for main thread to process work.
        }
        else
        {
            printf(thread_memory->cursor);
            fgets(thread_memory->input_buffer, sizeof(thread_memory->input_buffer), stdin);
            
            s64 len = strlen(thread_memory->input_buffer);
            char *last_char = thread_memory->input_buffer + (len - 1);
            if (*last_char == '\n')
            {
                *last_char = 0;
            }
            
            thread_memory->new_data = true;
        }
    }
}







enum Cmd_Arugment_Type
{
    Cmd_None,
    Cmd_Input_File_Path
};


s32 main(int argument_count, char **arguments)
{
    //~ NOTE: Initialization
    Program_State state = {};
    state.arena = create_virtual_arena();
    state.records = create_virtual_array<Record>();
    state.desc_table = create_description_table(4096);
    //clear_memory(&state);
    Arena *arena = &state.arena;
    
    
    //~ command line inputs
    b32 reformat_mode = false;
    { 
        Cmd_Arugment_Type type = Cmd_None;
        b32 disable_colors = false;
        
        for (s32 argument_index = 1; argument_index < argument_count; ++argument_index)
        {
            char *arg = arguments[argument_index];
            
            if (type == Cmd_None)
            {
                if (arg[0] == '-' || arg[0] == '/' || arg[0] == '?')
                {
                    if (arg[0] == '?' || arg[1] == '?' || arg[1] == 'h')
                    {
                        printf("tt.exe [-d database.txt] [-m]"
                               "\n"
                               "Options:"
                               "\n\t"
                               "-d"
                               "\t\t"
                               "Selects file to load and save data from."
                               "\n\t"
                               "-m"
                               "\t\t"
                               "Mono mode. Turns off colors."
                               "\n");
                        exit(0);
                    }
                    else if (arg[1] == 'd')
                    {
                        type = Cmd_Input_File_Path;
                    }
                    else if (arg[1] == 'm')
                    {
                        disable_colors = true;
                    }
                    else if (arg[1] == 'r')
                    {
                        reformat_mode = true;
                    }
                    else
                    {
                        printf("Unknown switch argument: %s\n", arg);
                        exit(0);
                    }
                }
            }
            else if (type == Cmd_Input_File_Path)
            {
                state.input_path = path_from_string(arena, string(arg));
                type = Cmd_None;
            }
            else
            {
                assert(0);
            }
        }
        
        initialize_colors(disable_colors);
    }
    
    
    
    //~ initialize essential state
    initialize_timezone_offset();
    
    state.exe_path = current_executable_path(arena);
    state.title = trim_from_index_of_reverse(state.exe_path.file_name, '.');
    
    if (state.input_path.file_name.size == 0)
    {
        state.input_path = state.exe_path;
        state.input_path.file_name = concatenate(arena, state.title, l2s(".txt"));
    }
    
    
    state.archive_dir = directory_append(arena, state.exe_path.directory, l2s("archive"));
    directory_create(arena, state.archive_dir);
    
    // NOTE: Save initial program state
    state.initial_scope = create_program_scope(&state.arena, &state.records);
    
    
    
    
    
    
    
    //~ initial file creation and load
    {
        arena_scope(arena);
        
        char *input_path_cstr = cstr_from_path(arena, &state.input_path);
        if (!file_exists(input_path_cstr))
        {
            if (!reformat_mode)
            {
                File_Handle file = file_open_write(input_path_cstr);
                if (no_errors(&file)) {
                    printf("Created new file: %s\n", input_path_cstr);
                }
                else
                {
                    printf("Failed to create new file: %s\n", input_path_cstr);
                    exit_error();
                }
                file_close(&file);
            }
            else
            {
                printf("File doesn't exist: %s. Exiting.\n", input_path_cstr);
                exit(1);
            }
        }
    }
    
    
    load_file(&state);
    if (!state.load_file_error)
    {
        Date_Range_Result range = get_recent_days_range(&state.records);
        process_days_from_range(&state, range.first, range.last, {}, ProcessDays_Print);
        save_to_file(&state);
        
        if (reformat_mode) {
            printf("File reformated. Exiting.\n");
            exit(0);
        }
    }
    else
    {
        if (reformat_mode) {
            printf("Reformatting skipped due to errors. Exiting with force_save.\n");
            // TODO(f0): Change this? -r should be --test_mode? Or --format --force_save
            save_to_file(&state);
            exit(1);
        }
    }
    
    
    
    
    //~
    Thread_Memory thread_memory = {};
    sprintf(thread_memory.cursor, "::>");
    platform_create_thread(read_from_keyboard, &thread_memory);
    
    
    
    //~
    for (;;)
    {
        Time32ms now = get_time32_ms();
        
        if (thread_memory.new_data)
        {
            state.last_input_time = now;
            Record_Session session = create_record_session_no_lexer(&state.arena, &state.records, false);
            char *input_copy = copy_cstr(&state.arena, thread_memory.input_buffer);
            session.lexer = create_lexer(input_copy);
                                                     
            process_input(&state, &session);
            if (no_errors(&session))
            {
                if (session.change_count > 0)
                {
                    Record *last_record = get_last_record(&session);
                    process_days_from_range(&state, last_record->date, last_record->date, {}, ProcessDays_PrintAltColor);
                }
            }
            
            thread_memory.new_data = false;
        }
        
        
        
        File_Time mod_time = platform_get_file_mod_time(&state.arena, &state.input_path);
        b32 source_file_changed = platform_compare_file_time(state.input_file_mod_time, mod_time) != 0;
        
        if (source_file_changed)
        {
            state.last_input_time = now;
            load_file(&state);
            printf(thread_memory.cursor);
        }
        
        
        s32 input_time_delta = (s32)now.t - (s32)state.last_input_time.t;
        if (input_time_delta > 1000*5)
        {
            u32 sleep_duration = input_time_delta / 8192;
            sleep_duration = pick_smaller(sleep_duration, 100);
            platform_sleep(sleep_duration);
        }
    }
}