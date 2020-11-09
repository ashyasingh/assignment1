#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

int my_debug = 0;    //for debug only
int my_printf_debug = 0;    //for debug only

#if 0
#define my_printf printf
#else

/* Convert the integer D to a string and save the string in BUF. If
   BASE is equal to 'd', interpret that D is decimal, and if BASE is
   equal to 'x', interpret that D is hexadecimal.  */

/*
   Add '%i' support and long int - ltoa api.
 */
void itoa (char *buf, int base, int d)
{
  char *p = buf;
  char *p1, *p2;
  unsigned int ud = d;  //->>>>>>>>>>> changed from long to int to print %x correctly
  int divisor = 10;
     
  /* If %d is specified and D is minus, put `-' in the head. */
  if ((base == 'd' || base == 'i') && d < 0)
    {
      *p++ = '-';
      buf++;
      ud = -d;
    }
  else if (base == 'x')
    divisor = 16;
     
  /* Divide UD by DIVISOR until UD == 0. */
  do
    {
      int remainder = ud % divisor;
     
      *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
  while (ud /= divisor);
     
  /* Terminate BUF. */
  *p = 0;
     
  /* Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2)
    {
      char tmp = *p1;
      *p1 = *p2;
      *p2 = tmp;
      p1++;
      p2--;
    }
}

void ltoa (char *buf, int base, long d)
{
  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;
     
  /* If %d is specified and D is minus, put `-' in the head. */
  if ((base == 'd' || base == 'i') && d < 0)
    {
      *p++ = '-';
      buf++;
      ud = -d;
    }
  else if (base == 'x')
    divisor = 16;
     
  /* Divide UD by DIVISOR until UD == 0. */
  do
    {
      int remainder = ud % divisor;
     
      *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
  while (ud /= divisor);
     
  /* Terminate BUF. */
  *p = 0;
     
  /* Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2)
    {
      char tmp = *p1;
      *p1 = *p2;
      *p2 = tmp;
      p1++;
      p2--;
    }
}

int print_newline(int out_fd)
{
    char newline_chars[2] = {10,13};
    write(out_fd, newline_chars, sizeof(newline_chars));
    return 1;   //count as single char
}

int print_char(int out_fd, char arg)
{
    if (arg == '\n')
    {
        return print_newline(out_fd);
    }
    else if (isprint(arg))
    {
        write(out_fd, &arg, sizeof(arg));
        return 1;
    }
    return 0;
}

int print_str(int out_fd, const char *str)
{
    int str_len = strlen(str);
    int i;

    //if the string contains newline escape char.
    for (i = 0; i < str_len; i++)
    {
        if (str[i] == '\n')
        {
            print_newline(out_fd);
        }
        else
        {
            print_char(out_fd, str[i]);
        }
    }

    return i;
}

int get_arg_char(char **var_arg, char *ch_arg)
{
    if (my_printf_debug) printf("\n->> internal : var_arg %p, ch_arg %p\n", *var_arg, ch_arg);

    *ch_arg = *((char *)(*var_arg));  //current pointer value is char
    (*var_arg) += sizeof(char *);   //move to next variable argument

    if (my_printf_debug) printf("->> internal : var_arg %p, ch_arg %c\n", *var_arg, *ch_arg);

    return 0;
}

int get_arg_str(char **var_arg, char **str_arg)
{
    if (my_printf_debug) printf("\n->> internal : var_arg %p, str_arg %p\n", *var_arg, *str_arg);

    *str_arg = *((char **)(*var_arg));  //current pointer value is string pointer
    (*var_arg) += sizeof(char *);   //move to next variable argument

    if (my_printf_debug) printf("->> internal : var_arg %p, str_arg %p\n", *var_arg, *str_arg);

    return 0;
}

int get_arg_int(char **var_arg, int *int_arg)
{
    if (my_printf_debug) printf("\n->> internal : var_arg %p, int_arg 0x%x\n", *var_arg, *int_arg);

    *int_arg = *((int *)(*var_arg));  //current pointer value is int
    (*var_arg) += sizeof(long);   //move to next variable argument (int stored as long in stack)

    if (my_printf_debug) printf("\n->> internal : var_arg %p, int_arg 0x%x %i\n", *var_arg, *int_arg, *int_arg);

    return 0;
}

int get_arg_long(char **var_arg, long *long_arg)
{
    if (my_printf_debug) printf("\n->> internal : var_arg %p, long_arg 0x%lx\n", *var_arg, *long_arg);

    *long_arg = *((long *)(*var_arg));  //current pointer value is long
    (*var_arg) += sizeof(long);   //move to next variable argument

    if (my_printf_debug) printf("\n->> internal : var_arg %p, long_arg 0x%lx %li\n", *var_arg, *long_arg, *long_arg);

    return 0;
}

int my_printf_internal (const char *fmtstr, char *var_arg_start)
{
    char *var_arg = var_arg_start;
    int fmtlen = strlen(fmtstr);

    if (my_printf_debug) printf("->> internal : fmtstr %p , var_arg_start %p\n", fmtstr, var_arg_start);
    if (my_printf_debug) printf("->> internal : fmtstr [ %s ] [ len = %i ] \n", fmtstr, fmtlen);

    char itoa_buf[32];  //unsigned long int is 20 chars max..
    int pc = 0;
    long long_arg;
    int int_arg;
    char *str_arg;
    char ch_arg;
    int i;
    int out_fd = 1; //stdout

    for (i = 0; i < fmtlen ; i++)
    {
        switch (fmtstr[i])
        {
        case '%': //arguments
            if (i+1 < fmtlen) 
            {
                switch (fmtstr[i+1])
                {
                case '%':   //print % char
                    pc += print_char(out_fd, fmtstr[i+1]);
                    break;
                case 'c':   //print char arg
                    if (get_arg_char(&var_arg, &ch_arg) == 0)
                    {
                        pc += print_char(out_fd, ch_arg);
                    }
                    else
                    {
                        return pc;  //argument error, terminate
                    }
                    break;
                case 'l':   //print long (64-bit)
                    if (i+2 < fmtlen) 
                    {
                        switch (fmtstr[i+2])
                        {
                        case 'd':   //print long arg
                        case 'u':   //print unsigned long arg
                        case 'x':   //print hex long arg
                            if (get_arg_long(&var_arg, &long_arg) == 0)
                            {
                                ltoa (itoa_buf, fmtstr[i+2], long_arg);
                                pc += print_str(out_fd, itoa_buf);
                            }
                            else
                            {
                                return pc;  //argument error, terminate
                            }
                            break;
                        default:
                            return pc;  //not supported, terminate
                        }
                        i++;    // increment for fmtstr[i+2]
                    }
                    else
                    {
                        return pc;  //argument error, terminate
                    }
                    break;
                case 'd':   //print int arg (32-bit)
                case 'u':   //print unsigned int arg
                case 'x':   //print hex int arg
                    if (get_arg_int(&var_arg, &int_arg) == 0)
                    {
                        itoa (itoa_buf, fmtstr[i+1], (long int) int_arg);
                        pc += print_str(out_fd, itoa_buf);
                    }
                    else
                    {
                        return pc;  //argument error, terminate
                    }
                    break;
                case 's':
                    if (get_arg_str(&var_arg, &str_arg) == 0)
                    {
                        pc += print_str(out_fd, str_arg);
                    }
                    else
                    {
                        return pc;  //argument error, terminate
                    }
                    break;
                default:
                    return pc;  //not supported, terminate
                }
                i++;    //increment for fmtstr[i+1]
            }
            else
            {
                return pc;  //format error, terminate
            }
            break;

        default:
            pc += print_char(out_fd, fmtstr[i]);
            break;
        }
    }

    return pc;
}

void dump_memory(unsigned long *p1)
{
    int i;
    for (i = -1; i < 12; i++)
    {
        printf("[%i]: %p = 0x%016lx:\n", i, (p1+i), *(p1+i));
    }
}

int my_printf(const char *fmtstr, ...)
{
    if (my_printf_debug) dump_memory ((unsigned long *)&fmtstr);

    //without any local variables defined, the first variable arg is located after
    // fmtstr + caller's saved ebp
    return my_printf_internal (fmtstr, (char *)(&fmtstr)+(2*sizeof(char *)));
}
#endif


int finds_in_line_exact (char *newstr, const char *findstr) // 1st assignment
{
    if ( strstr(newstr, findstr) != NULL )
        return 1;

    return 0;
}

int finds_str_match (const char *newstr, int *ns_i, int *ns_len, const char *findstr, int fc_len)
{
    if (my_debug) my_printf ("---...--->> %s (%i, %i) - %s (%i)\n", &newstr[*ns_i], *ns_i, *ns_len, findstr, fc_len);

    if ((*ns_len - *ns_i) < fc_len)    // remaining newstr is shorter than findstr
        return 0;

    const char *rem_newstr = &newstr[*ns_i];
    int i;
    for (i = 0; i < fc_len; i++)
    {
        if (findstr[i] == '.') //match any alphanum char
        {
            if ( ! isalnum(rem_newstr[i]) )
                return 0;   //not match
        }
        else if (findstr[i] != rem_newstr[i])   //exact match char
            return 0;   //not match
    }

    *ns_i += fc_len;   //advance for next comparison
    //*ns_len -= fc_len;

    return 1;   //match
}

// allow recursive call for substring matches in parenthesis
int finds_in_str_wildcard (char *newstr, int *ns_i, int *ns_len, const char *findstr, int fs_len, int fswc_end)
{
    int fs_i = 0; //index for comparison in each call
    int fi, ni; // temp loop counters
    int fc_len, fc_cnt;
    int ps_i = -1, ps_e = -1;   //parenthesis start and end, nested parenthesis not supported

    if (my_debug) my_printf ("--------->> %s (%i, %i) - %s (%i)\n", &newstr[*ns_i], *ns_i, *ns_len, findstr, fs_len);

    if (*ns_i >= *ns_len)   //end of new string, can't search anymore
        return 0;

    int fswc_match = 1;    //used for recursive calls only

    for (fi = 0; fi < fs_len; fi++)
    {
        //look for wildcard characters, except '.' which is compared inside finds_str_match
        switch (findstr[fi])
        {
        case '*' : // '*' matches zero or more instances of the previous character e.g., "a*b" matches "b", "ab", "aab" etc
        case '?' : // '?' matches zero or one instance of the previous character e.g., "a?b" matches "ab" or "b".
            if (ps_i > 0)   //parenthesis start detectecd
            {
                if (ps_e > 0)   //parenthesis end detected
                {
                    int ps_len = ps_e - ps_i;   // substring within parenthesis
                    if (ps_len > 0)
                    {
                        //recursive call to match the substring within parenthesis
                        //nested parenthesis not supported and blocked in main 
                        // if last 2 chars before parenthesis end is '.*' or '.?', get the next char after parenthesis to terminate
                        fswc_end = 0;
                        if ((findstr[ps_e-1] == '*' || findstr[ps_e-1] == '?') && (findstr[ps_e-2] == '.') && (fi+1 < fs_len))
                            fswc_end = findstr[fi+1];

                        //search based on string within parenthesis
                        while ( finds_in_str_wildcard (newstr, ns_i, ns_len, &findstr[ps_i], ps_len, fswc_end) == 1 )
                        {
                            if ( findstr[fi] == '?' ) //zero or one match only
                                break;

                            // for '*' , keep searching for more matches
                            continue;
                        }
                        //terminate after either no match or found one/more matches
                    }

                    ps_i = ps_e = -1;   //reset parenthesis flags
                    //fs_i will be updated below to next char
                }
                else
                {
                    // '*' or '?' wildcard inside parenthesis
                    // delay processing, until parenthesis is closed
                    continue;
                }
            }
            else if (fi > 0)
            {
                //compare any chars before previous one for exact match
                fc_len = fi-1 - fs_i;
                if (fc_len > 0)
                {
                    if (finds_str_match(newstr, ns_i, ns_len, &findstr[fs_i], fc_len) != 1)
                    {
                        return 0;   //broken match
                    }
                }

                fs_i = fi-1; //previous char
                char prevchar = findstr[fs_i];

                fc_cnt = 0; //match count
                for (ni = *ns_i ; ni < *ns_len; ni++)
                {
                    //compare with same previous char
                    if ( ((prevchar == '.') && isalnum(newstr[ni])) ||  // any alphanum
                         (prevchar == newstr[ni]) )     // exact char match
                    {
                        fc_cnt++;

                        if (my_debug && (prevchar == '.') ) my_printf ("------------->> (ni %i, ns_len %i newchar %c match '.') - fc_cnt %i (fi %i, fs_len %i) - fswc_end %c\n", ni, *ns_len, newstr[ni], fc_cnt, fi, fs_len, (fswc_end == -1 ? '-' : (fswc_end == 0 ? '+' : (char)(fswc_end))) );

                        if (prevchar == '.')    //special handlng for .* and .? wilecard needed
                        {
                                //peek ahead in findstr to look for next alphanum char
                                // e.g.  a.*b or a.?b , will terminate when 'b' is found
                                //  then 'b' will be compared separately
                                //
                                char nextchar = 0;  //set to null for non-alphanum
                                if (fi+1 < fs_len)  //get the next char from current find sub-string
                                    nextchar = findstr[fi+1];
                                else if (fswc_end > 0)   // passed in by parenthesis recursive call, eg. (a.*)*c , then fswc_end = 'c' is passed
                                    nextchar = fswc_end;  //if there is no more chars in findstr, then 0 is passed, which means continue match till end of newstr

                                if (isalnum(nextchar) && (nextchar == newstr[ni])) //matches current char in newstr
                                {
                                    fswc_match = 0; //terminate the recursive wildcard match
                                    fc_cnt--;   //decrement match for current char
                                    break;
                                }
                                else if (isalnum(nextchar) && 
                                    (ni+1 < *ns_len) && (nextchar == newstr[ni+1])) //matches next char in newstr
                                {
                                    fswc_match = 0; //terminate the recursive wildcard match
                                    break;
                                }
                        }

                        if ( findstr[fi] == '*' ) //zero or more match for '*'
                        {
                            continue; // keep looking
                        }
                        else    // zero or one match for '?' 
                        {
                        }
                    }
                    break;
                }

                if (fc_cnt > 0)     //one or more matches
                {
                    *ns_i += fc_cnt;   //advance for next comparison
                    //*ns_len -= fc_cnt;
                }

                if (my_debug && (prevchar == '.') ) my_printf ("---------------->> (ns_i %i, ns_len %i) - fc_cnt %i\n", *ns_i, *ns_len, fc_cnt);

            }
            fs_i = fi+1; //next char
            break;

        case '(' : // substrings '(' and ')' : regular expression of the form "a(bc)*" matches "a", "abc", "abcbc" etc. 
            if (fi > 0)
            {
                //compare any chars before current one for exact match
                fc_len = fi - fs_i;
                if (fc_len > 0)
                {
                    if (finds_str_match(newstr, ns_i, ns_len, &findstr[fs_i], fc_len) != 1)
                    {
                        return 0;   //broken match
                    }
                }
            }

            fs_i = fi+1; //next char after parenthesis start
            ps_i = fs_i; //parenthesis start
            break;

        case ')' :
            ps_e = fi; //parenthesis end, go to next char which must be '*' or '?'
            break;

        default :
            if (ps_i > 0 && ps_e > 0)
            {
                //no '*' or '?' after parenthesis end, ignore it and assume zero match
                ps_i = ps_e = -1;   //reset parenthesis flags
                fs_i = fi;  //set to current char
            }
            break;
        }
    }

    //compare any remaining chars including last one for exact match
    fc_len = fi - fs_i;

    if (my_debug) my_printf ("-------$$$--------->> (fi %i, fs_i %i) - fc_len %i\n", fi, fs_i, fc_len);

    if (fc_len > 0)
    {
        if (finds_str_match(newstr, ns_i, ns_len, &findstr[fs_i], fc_len) != 1)
        {
            return 0;   //broken match
        }
    }

    if (fswc_end != -1)    //recursive loop for parenthesis .* and .? wildcard
        return fswc_match;

    return 1;   //match complete
}

int finds_in_line_wildcard (char *newstr, const char *findstr) // 2nd assignment
{
#if 0
    if( (strchr(findstr, '.') == NULL) &&
        (strchr(findstr, '*') == NULL) &&
        (strchr(findstr, '?') == NULL) &&
        (strchr(findstr, '(') == NULL) &&
        (strchr(findstr, ')') == NULL) )
    {
        //no wildcard characters present
        return finds_in_line_exact (newstr, findstr);
    }
#endif

    int orig_ns_len = strlen(newstr);
    int fs_len = strlen(findstr);
    int si;
    for (si = 0; si < orig_ns_len; si++)    //for each line, iterate with each char as start of new search
    {
        //these 2 variables track the start of new search , position and remaining length in recursive calls
        int ns_i = 0;
        int ns_len = orig_ns_len - si;

        int fswc_end = -1;     //to terminate .* and .? match, e.g. ab(c.*)*d or ab(c.?)*d or ab(c.?)?d etc
                                // in first call , pass -1 ; in recursive calls pass 0 or valid alphanum char

        if (my_debug) my_printf ("----->> [%i] %s (%i, %i) - %s (%i)\n", si, &newstr[si], ns_i, ns_len, findstr, fs_len);

        if ( finds_in_str_wildcard(&newstr[si], &ns_i, &ns_len, findstr, fs_len, fswc_end) == 1 )
            return 1;   //terminate if a match is found for this line
    }

    return 0;
}

int finds_in_filedata (char *filedata, size_t filelen, const char *findstr)
{
    int match_count = 0;

    //break the file data into separate strings based on newline
    char *saveptr = NULL;
    char *newstr = strtok_r(filedata, "\n", &saveptr);  //first call
    while (newstr)
    {
        //search for matching pattern in each line
        if ( finds_in_line_wildcard(newstr, findstr) == 1 )
        {
            my_printf ("%s\n", newstr); //print the line once
            match_count++;
        }
        newstr = strtok_r(NULL, "\n", &saveptr);    //subsequent call
    }

    return match_count;
}

int finds_in_file (const char *curpath, const char *filename, const char *findstr)
{
    FILE *fp;

    if (!curpath || !filename)
    {
        my_printf ("error: %s: null arguments\n", __FUNCTION__);
        return -1;
    }

    //read the entire file in memory and then split into new lines
    int cp_len = strlen(curpath);
    int fn_len = strlen(filename);
    char *filepath = malloc(cp_len + 1 + fn_len + 1);    // add '/' and '\0'
    if (!filepath)
    {
        my_printf ("error: %s: allocate memory (%u bytes) failed\n", __FUNCTION__, (cp_len + 1 + fn_len + 1));
        return -1;
    }

    //join the curpath with filename
    memcpy (filepath, curpath, cp_len);
    filepath[cp_len] = '/';
    memcpy (&filepath[cp_len+1], filename, fn_len);
    filepath[cp_len + 1 + fn_len] = '\0';

    if ((fp = fopen(filepath, "r")) == NULL)
    {
        my_printf ("error: opening %s (%s)\n", filepath, strerror(errno));
        free (filepath);
        return -1;
    }

    fseek (fp, 0L, SEEK_END);
    long f_len = ftell(fp);
    rewind (fp);

    if (f_len > 0)
    {
        int match_found = 0;

        if (my_debug) my_printf ("----->> opening %s: len = %lu\n", filepath, f_len);

        char *filedata = malloc(f_len + 1);

        if (filedata)
        {
            size_t readlen = fread (filedata, 1, f_len, fp);
            if ( readlen != f_len )
            {
                my_printf ("error: file %s, size %lu, able to read %lu only\n", filepath, f_len, readlen );
                filedata[readlen] = '\0';
            }
            else
            {
                filedata[f_len] = '\0';
            }

            match_found = finds_in_filedata (filedata, readlen, findstr);
        }
        else
        {
            my_printf ("error: %s: allocate memory (%lu bytes) failed\n", __FUNCTION__, (f_len + 1) );
        }

        free (filedata);

        if (match_found > 0)    //print the file name if any match found
            my_printf ("%s\n", filepath);
    }

    fclose(fp);
    free (filepath);

    return 0;
}

int match_suffix (const char *suffix, const char *filename)
{
    if (!filename)
    {
        my_printf ("error: %s: null arguments\n", __FUNCTION__);
        return -1;
    }

    if (!suffix)    //no suffix
        return 1;

    int f_len = strlen(filename);
    if (f_len < 2)  //file name length must exceed suffix 'char' + '.'
        return 0;

    if (( filename[f_len-2] == '.' ) && ( filename[f_len-1] == suffix[0] ))
        return 1;

    return 0;
}

int finds_in_directory (const char *curpath, const char *dirname, const char *findstr, const char *suffix, int lflag)
{
    DIR *dp;
    struct dirent *de;

    int cp_len = (curpath ? strlen(curpath) : 0);
    int dn_len = strlen(dirname);
    char *newpath = malloc(cp_len + 1 + dn_len + 1);    // add '/' and '\0'
    if (!newpath)
    {
        my_printf ("error: %s: allocate memory (%u bytes) failed\n", __FUNCTION__, (cp_len + 1 + dn_len + 1));
        return -1;
    }

    if (curpath)
    {
        //join the curpath with dirname
        memcpy (newpath, curpath, cp_len);
        newpath[cp_len] = '/';
        memcpy (&newpath[cp_len+1], dirname, dn_len);
        newpath[cp_len + 1 + dn_len] = '\0';
    }
    else
    {
        //start from pathname
        memcpy (newpath, dirname, dn_len);
        newpath[dn_len] = '\0';
    }

    if (my_debug) my_printf ("->> opening %s:\n", newpath);

    if ((dp = opendir(newpath)) == NULL)
    {
        my_printf ("error: failed to open directory %s (%s)\n", newpath, strerror(errno));
        free (newpath);
        return -1;
    }

    while ((de = readdir(dp)) != NULL)
    {
        if (my_debug) my_printf ("--->> file type %2d: name %s\n", de->d_type, de->d_name);

        if (de->d_type == DT_DIR)
        {
            //skip the special directory entries
            if ( (strcmp(de->d_name, ".") == 0) || (strcmp(de->d_name, "..") == 0))
                continue;

            finds_in_directory (newpath, de->d_name, findstr, suffix, lflag);    //recursive call
        }
        else if (de->d_type == DT_REG)  //regular file
        {
            if ( match_suffix(suffix, de->d_name) )
            {
                finds_in_file (newpath, de->d_name, findstr);
            }
        }
        else if ((lflag == 1) && (de->d_type == DT_LNK))   //symbolic link
        {
            if ( match_suffix(suffix, de->d_name) )
            {
                finds_in_file (newpath, de->d_name, findstr);
            }
        }
    }

    closedir(dp);
    free (newpath);

    return 0;
}

int option_needs_arg (int optchar)
{
    switch (optchar)
    {
    case 'p':
    case 'f':
    case 's':
        return 1;
    }
    return 0;
}

int valid_suffix (const char *suffix)
{
    if (strlen(suffix) != 1)    //expecting single char only.
        return 0;

    switch (suffix[0])
    {
    case 'c':
    case 'h':
    case 'S':
        return 1;
    }
    return 0;
}

int print_usage (char *cmd)
{
    my_printf ("usage: %s -p <pathname> [-f c|h|S] [-l] -s <findstr>\n", cmd);
    my_printf ("       where <findstr> includes alphanumerical and control characters \n");
    my_printf ("         for example: 'a.b', 'a*b', 'a?b', 'a(bc)*', 'ab(c.d)*d(ef)?g' \n");
    return -1;
}


//expected syntax : finds -p <pathname> [-f c|h|S] [-l] -s <findstr>
int main (int argc, char *argv[])
{
    int pflag = 0, fflag = 0, lflag = 0, sflag = 0;
    char *pathname = NULL, *suffix = NULL, *findstr = NULL;
    int flag;

    opterr = 0; //suppress errors by getopt
    optind = 1;
    // only add ":" after options that require arguments
    while ((flag = getopt (argc, argv, "p:f:ls:")) != -1)
    {
        switch (flag)
        {
        case 'p':
            if (pflag)
            {
                my_printf ("error: duplicate -%c argument \n", flag);
                return print_usage (argv[0]);
            }
            pflag = 1; // option -p specified
            pathname = optarg;
            break;
        case 'f':
            if (fflag)
            {
                my_printf ("error: duplicate -%c argument \n", flag);
                return print_usage (argv[0]);
            }
            fflag = 1; // option -f specified
            suffix = optarg;
            if (!valid_suffix (suffix))
            {

            #if 0   // debug code to reverse engineer location of variable arguments on stack
                    // and unit test my_printf formating.
                printf("suffix p1: %p (= %p = %s)\n", &suffix, suffix, suffix);
                flag = -5;
                printf("flag p2: %p (= %x)\n", &flag, flag);
                printf("pathname p3: %p (= %p = %s)\n", &pathname, pathname, pathname);
                fflag = -1;
                printf("fflag p4: %p (= %x)\n", &fflag, fflag);
                printf("\n");
                printf ("error: invalid -f suffix %s flag %x-\n-pathname %c fflag %x-\n", suffix, flag, pathname[0], fflag);
                my_printf ("error: invalid -f suffix %s flag %x-\n-pathname %c fflag %x-\n", suffix, flag, pathname[0], fflag);
                return -1;
            #endif

                my_printf ("error: invalid -f suffix %s \n", suffix);
                return print_usage (argv[0]);
            }
            break;
        case 'l':
            if (lflag)
            {
                my_printf ("error: duplicate -%c argument \n", flag);
                return print_usage (argv[0]);
            }
            lflag = 1;  // option -l specified
            break;
        case 's':
            if (sflag)
            {
                my_printf ("error: duplicate -%c argument \n", flag);
                return print_usage (argv[0]);
            }
            sflag = 1;  // option -s specified
            findstr = optarg;
            int fs_len = strlen(findstr);
            int fi, ps = 0;
            for (fi = 0; fi < fs_len; fi++)
            {
                if( isalnum(findstr[fi]) )
                    continue;

                switch( findstr[fi] )
                {
                case '*':
                case '?':
                    if (fi == 0 ||  // '*' and '?' can not be first character of a search string,
                        findstr[fi-1] == '(' /*)*/ )    // can not be first char inside parenthesis either.
                    {
                        my_printf ("error: -%c argument must not contain '*' or '?' as first character in expression or inside parenthesis \n", flag);
                        return print_usage (argv[0]);
                    }
                    //no break
                case '.':
                #if 0   //extra credit
                    if (ps) //previous parenthesis not yet closed 
                    {
                        my_printf ("error: -%c argument must not contain another wildcard inside parenthesis, e.g. 'a(b.c)*' \n", flag);
                        return print_usage (argv[0]);
                    }
                #endif
                    break;

                case '(':
                    if (ps) //previous parenthesis not yet closed 
                    {
                        my_printf ("error: -%c argument must not contain nested parenthesis, e.g. 'a(b(cd)*d)' \n", flag);
                        return print_usage (argv[0]);
                    }
                    ps = 1; //start new parenthesis
                    break;
                case ')':
                    if (!ps) //parenthesis not yet started
                    {
                        my_printf ("error: -%c argument must not contain opening parenthesis before closing parenthesis, e.g. 'a(bc)*' \n", flag);
                        return print_usage (argv[0]);
                    }
                    ps = 0; //close current parenthesis
                    break;

                default:
                    my_printf ("error: -%c argument must contain alphanum and control '.', '*', '?', '(', ')' characters only \n", flag);
                    return print_usage (argv[0]);
                }
            }

            if (ps) //previous parenthesis not closed 
            {
                my_printf ("error: -%c argument must contain matching set of opening and closing parenthesis, e.g. 'ab(cd)*d(ef)*' \n", flag);
                return print_usage (argv[0]);
            }
            break;
        case '?':
            if (option_needs_arg (optopt))
                my_printf ("error: option %c requires an argument\n", optopt);
            else
                my_printf ("error: unknown option %c \n", optopt);

            return print_usage (argv[0]);
            break;
        default:
            return print_usage (argv[0]);
            break;
        }
    }

    if (!pflag)
    {
        my_printf ("error: -p option is required\n");
        return print_usage (argv[0]);
    }
    if (!sflag)
    {
        my_printf ("error: -s option is required\n");
        return print_usage (argv[0]);
    }

    if (my_debug) my_printf ("pathname: %s, suffix: %s, findstr: %s\n", pathname, suffix, findstr);

    finds_in_directory (NULL, pathname, findstr, suffix, lflag);

    return 0;
}

