
#define MAX_TIMESTAMP_STRING_SIZE sizeof("2020-12-31 13:45")
internal void 
get_timestamp_string(char *output, u32 output_size, time_t time)
{
    tm *date = gmtime(&time);
    
    Assert(MAX_TIMESTAMP_STRING_SIZE <= output_size);
    snprintf(output, output_size, "%04d-%02d-%02d %02d:%02d", 
             date->tm_year + 1900, date->tm_mon + 1, date->tm_mday,
             date->tm_hour, date->tm_min);
}

internal void 
get_timestamp_string_for_file(char *output, u32 output_size, 
                              time_t time, b32 long_format)
{
    tm *date = gmtime(&time);
    
    if (long_format)
    {
        snprintf(output, output_size, "%04d-%02d-%02d_%02d-%02d-%02d", 
                 date->tm_year + 1900, date->tm_mon + 1, date->tm_mday,
                 date->tm_hour, date->tm_min, date->tm_sec);
    }
    else
    {
        snprintf(output, output_size, "%04d-%02d-%02d_%02d-%02d", 
                 date->tm_year + 1900, date->tm_mon + 1, date->tm_mday,
                 date->tm_hour, date->tm_min);
    }
}

#define MAX_TIME_STRING_SIZE sizeof("00000000000:00")
internal s32 
get_time_string(char *output, u32 output_size, time_t time)
{
    Assert(output_size >= MAX_TIME_STRING_SIZE);
    time = abs(time);
    
    s32 hours = (s32)((time / Hours(1)));
    
    s32 minutes_in_hour = 60;
    s32 minutes = (s32)((time / Minutes(1)) % minutes_in_hour);
    
    s32 result = snprintf(output, output_size, "%02d:%02d", hours, minutes);
    return result;
}

internal void 
get_date_string(char *output, u32 output_size, time_t timestamp)
{
    tm *date = gmtime(&timestamp);
    
    snprintf(output, output_size, "%04d-%02d-%02d", 
             date->tm_year + 1900, date->tm_mon + 1, date->tm_mday);
}




#define MAX_PROGRESS_BAR_SIZE sizeof("[++++ ++++ ++++ ++++ ++++ +++-> stop is missing!")
internal void 
get_progress_bar_string(char *output, s32 output_size, time_t time, Missing_Ending_Type missing_ending)
{
    // NOTE: Max output:
    Assert(output_size >= MAX_PROGRESS_BAR_SIZE);
    if (time > Days(1)) time = Days(1);
    
    s32 hours = (s32)(time / Hours(1) % (Days(1) / Hours(1)));
    s32 minutes = (s32)(time / Minutes(1)) % (Hours(1) / Minutes(1));
    
    b32 print_open_bracket = true;
    b32 print_minutes_bit = (minutes > 0);
    
    s32 signs_to_print = hours + (print_minutes_bit ? 1 : 0);
    s32 signs_printed = 0;
    b32 printed_separator_recently = true;
    s32 print_align_spaces = (4 - (signs_to_print % 4)) % 4;
    
    char *close_bracket_string;
    if (missing_ending == MissingEnding_None)         close_bracket_string = "]";
    else if (missing_ending == MissingEnding_Assumed) close_bracket_string = ")";
    else                                              close_bracket_string = "> stop is missing!";
    
    
    for (s32 index = 0;
         index < output_size;
         ++index)
    {
        if (print_open_bracket)
        {
            output[index] = '[';
            print_open_bracket = false;
        }
        else if (signs_printed < signs_to_print)
        {
            if (!printed_separator_recently && 
                (signs_printed % 4 == 0))
            {
                output[index] = ' ';
                printed_separator_recently = true;
            }
            else
            {
                char sign;
                if (print_minutes_bit && (signs_printed == (signs_to_print - 1))) sign = '-';
                else                                                              sign = '+';
                
                
                output[index] = sign;
                ++signs_printed;
                printed_separator_recently = false;
            }
        }
        else if (print_align_spaces > 0)
        {
            output[index] = ' ';
            --print_align_spaces;
        }
        else if (*close_bracket_string)
        {
            output[index] = *close_bracket_string;
            ++close_bracket_string;
        }
        else
        {
            output[index] = 0;
            break;
        }
    }
    
    output[output_size - 1] = 0;
}

#define MAX_SUM_AND_PROGRESS_BAR_STRING_SIZE (MAX_TIME_STRING_SIZE + MAX_PROGRESS_BAR_SIZE + 16)
internal void 
get_sum_and_progress_bar_string(char *output, s32 output_size, Day *day)
{
    Assert(output_size >= MAX_SUM_AND_PROGRESS_BAR_STRING_SIZE);
    
    char time_str[MAX_TIME_STRING_SIZE];
    get_time_string(time_str, sizeof(time_str), day->sum);
    
    char bar_str[MAX_PROGRESS_BAR_SIZE];
    get_progress_bar_string(bar_str, sizeof(bar_str), day->sum, day->missing_ending);
    
    snprintf(output, output_size, "sum: %s\t%s", time_str, bar_str);
}


internal void 
get_day_of_the_week_string(char *output, s32 output_size, time_t timestamp)
{
    tm *date = gmtime(&timestamp);
    
    char *day_str = NULL;
    switch (date->tm_wday)
    {
        case 0:  { day_str = "Sunday"; break; }
        case 1:  { day_str = "Monday"; break; }
        case 2:  { day_str = "Tuesday"; break; }
        case 3:  { day_str = "Wednesday"; break; }
        case 4:  { day_str = "Thursday"; break; }
        case 5:  { day_str = "Friday"; break; }
        case 6:  { day_str = "Saturday"; break; }
        default: { day_str = "???"; break; }
    }
    
    snprintf(output, output_size, day_str);
}




//~ NOTE: Print functions


internal void 
print_offset(s32 offset_sum)
{
    const u32 size = MAX_TIME_STRING_SIZE + 2;
    char offset_time_str[size];
    
    if (offset_sum >= Minutes(1))        offset_time_str[0] = '+';
    else if (offset_sum <= -Minutes(1))  offset_time_str[0] = '-';
    else                                offset_time_str[0] = 0;
    
    if (offset_time_str[0])
    {
        get_time_string(offset_time_str + 1, size - 2, offset_sum);
        offset_time_str[size - 2] = '\t';
        offset_time_str[size - 1] = 0;
    }
    
    printf("%s", offset_time_str);
}

internal void 
print_work_time_row(Time_Entry *start, Time_Entry *stop, s32 offset_sum, char *replace_stop = NULL)
{
    using namespace Global_Color;
    Assert(start);
    
    char start_time_str[MAX_TIME_STRING_SIZE];
    get_time_string(start_time_str, sizeof(start_time_str), start->time);
    
    char stop_time_str[MAX_TIME_STRING_SIZE];
    if (replace_stop == NULL)
    {
        Assert(stop);
        get_time_string(stop_time_str, sizeof(stop_time_str), stop->time);
        replace_stop = stop_time_str;
    }
    
    printf("%s -> %s\t", start_time_str, replace_stop);
    
    print_offset(offset_sum);
    
    if (start->description)         printf("%s\t\"%s\"%s ", f_desc, start->description, f_reset);
    if (stop && stop->description)  printf("%s\t\"%s\"%s ", f_desc, stop->description, f_reset);
    
    printf("\n");
}


internal void
print_line_with_token(Token token)
{
    u32 len = 0;
    char *str = token.line_start;
    if (str)
    {
        while (str[0] && !is_end_of_line(str[0]))
        {
            ++str;
            ++len;
        }
        
        printf("\nline [%lld]: %.*s", token.line_index, len, token.line_start);
    }
}











