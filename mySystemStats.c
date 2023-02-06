#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <utmp.h>
#include <getopt.h>
#include <math.h>


//  displays the number of samples and tdelay between samples
void display_header(int i, int sequential, int samples, int tdelay){

    if(sequential){
        printf(">>> iteration %d\n", i); //prints iteration number if sequential
    } else {
        printf("\033[H\033[2J"); //clears the terminal screen and move the cursor to the top-left corner
        printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay); //prints the number of samples and tdelay between samples
    }

    struct rusage mem_usage; //declaring a struct of type rusage found in <sys/resource.h>
    struct rusage *mem_ptr = &mem_usage; //creating pointer pointing to the address of mem_usage

    if(getrusage(RUSAGE_SELF, mem_ptr)==0){ //checks if the call to getrusage is successful
        printf(" Memory usage: %ld kilobytes\n", mem_ptr->ru_maxrss); //prints memory usage through ru_maxrss field of rusage struct
    }
}

//  prints the number of currently available cores using sysconf
void print_cores(void){
    int n_cpu = sysconf(_SC_NPROCESSORS_ONLN); //sysconf returns the number of processors (found in <unistd.h>)
    printf("Number of cores: %d\n", n_cpu); //printing number of cores to stdout
}

/*  takes an array of unsigned long integers as parameter and set its entries to the values of user, 
 *   nice, system, idle, iowait, irq, softirq fields as found in the file /proc/stat
 */
void set_cpu_values(unsigned long cpuArr[7])
{
    FILE *fp = fopen("/proc/stat", "r"); //opening /proc/stat file in read-mode and assigns the returned file pointer to fp

    if(fp==NULL){ //checks if file opening is successful
        fprintf(stderr, "File could not be opened\n"); //if there was an error opening file, the message is printed to stderr
        exit(1); //exit program
    }

    char buffer[255]; //a char array used to store the format string
    //fscanf is used to read data from the file into the variables
    int success = fscanf(fp, "%s %lu %lu %lu %lu %lu %lu %lu", buffer, &(cpuArr[0]), &(cpuArr[1]), 
        &(cpuArr[2]), &(cpuArr[3]), &(cpuArr[4]), &(cpuArr[5]), &(cpuArr[6])); 
    //specifies that the function should read one string followed by seven unsigned long integers

    if(success!=8){ //checks the number of items successfully read by fscanf
        fprintf(stderr, "Error reading file\n");    //prints error message to stderr if error occurs
        fclose(fp); //close file pointer
        exit(1); //exit program
    }
    fclose(fp); //close file pointer
}

/*
 *given two unsigned long arrays prev and cur representing the 7 fields of cpu information as obtained from
 * /proc/stat that were sampled at different times, it calculates and returns the cpu usage percentage.
 */
double calculate_cpu_usage(unsigned long prev[7], unsigned long cur[7])
{
    int idle_prev = prev[3] + prev[4];  //sum of the idle and iowait fields of /proc/stat in prev sample
    int idle_cur = cur[3] + cur[4]; //sum of idle and and iowait fields of /proc/stat in cur sample

    int non_idle_prev = prev[0] + prev[1] + prev[2] + prev[5] + prev[6];    //sum of user, nice, system, irq, and softirq fields of /proc/stat in prev sample
    int non_idle_cur = cur[0] + cur[1] + cur[2] + cur[5] + cur[6];  //sum of user, nice, system, irq, and softirq fields of /proc/stat in cur sample

    int total_prev = idle_prev + non_idle_prev; //sum of idle_prev and non_idle_prev, giving the total time of the CPU in prev sample
    int total_cur = idle_cur + non_idle_cur; //sum of idle_prev and non_idle_prev, giving the total time of the CPU in cur sample

    double total_diff = (double) total_cur - (double) total_prev; //difference between current and previous total CPU time values
    double idle_diff = (double) idle_cur - (double) idle_prev; //difference between current and previous idle time values

    // total_diff - idle_diff gives the difference in CPU utilization between two points in time that excludes idle time,
    // which gives a measure of how busy the CPU was with non_idle tasks between two sample points in time 
    // (referred to as utilization of the CPU). Dividing this by the difference of total CPU time yields the fraction of
    // CPU time spent on non_idle activities, which is then multiplied by 1000 to convert the decimals into a whole number and 1 
    // is added to avoid division by 0 and to avoid undefined value. Then divided by 10 to get a range between 0-100% as percentage.
    // Note: formula obtained from TAs in office hours linked in README file
    return ((1000 * ((total_diff - idle_diff) / total_diff) + 1) / 10);

}

// prints the current list of users/sessions on the system
void print_users(void) {

    printf("### Sessions/users ###\n"); //prints a header
    setutent(); //resets the internal stream of the utmp database to the beginning for reading utmp.h file
    
    for(struct utmp *user=NULL; (user=getutent());){ //read the records of the utmp database one by one until it returns a NULL pointer signalling the end
        if(user->ut_type == USER_PROCESS){  //checks if the user is currently logged into the system and running a process

            printf(" %s\t%s\t", user->ut_user, user->ut_line);  //prints username and terminal line stored in utmp struct
            
            if(*(user->ut_host)){  //checks whether the value of ut_host is non-empty
                printf("(%s)", user->ut_host); //prints host information
            }
            printf("\n");
        }
    }
    endutent(); //closes the internal stream of the utmp database
}

//prints system information about the machine the program is running on
void print_machine_info(void){
    struct utsname sysData; //a struct of type utsname is declared (found in <sys/utsname.h>)
    uname(&sysData); //fills in the fields of the sysData struct with information about the current Operating System
    printf("### System Information ###\n");
    printf(" System Name = %s\n", sysData.sysname); //prints system name
    printf(" Machine Name = %s\n", sysData.nodename); //prints machine name
    printf(" Version = %s\n", sysData.version); //prints OS version
    printf(" Release = %s\n", sysData.release); //prints OS/software release
    printf(" Architecture = %s\n", sysData.machine); //prints machine architecture (computer hardware type)
}

// stores calculated physical and virtual memory information as a string in strArr at index i and returns virtual used memory
double write_memory(char strArr[][1024], int i){
    struct sysinfo sys_info;    //a struct of type sysinfo is declared to store system information (from <sys/sysinfo.h>)
    double phys_used, phys_total, virt_used, virt_total; 
    
    sysinfo(&sys_info); //retrieves information about the system into sys_info

    //physical memory used calculated by subtracting free physical memory from the total physical memory and converting to gigabytes
    phys_used = (float)(sys_info.totalram - sys_info.freeram)/1024/1024/1024; 
    phys_total = (float)sys_info.totalram/1024/1024/1024; //storing total physical memory and dividing to convert the value to gigabytes
    
    //virtual memory used calculated by adding the used physical memory to the used virtual memory obtained by subtracting the free virtual memory
    //from the total virtual memory and dividing by 1024^3 to convert it to gigabytes.
    virt_used = phys_used + (float)(sys_info.totalswap - sys_info.freeswap) /1024/1024/1024;
    
    virt_total = (float)(sys_info.totalram + sys_info.totalswap)/1024/1024/1024; //total virtual memory calculated by adding total physical memory to total virtual memory

    sprintf(strArr[i], "%.2f GB / %.2f GB -- %.2f GB / %.2f GB",  
    phys_used, phys_total, virt_used, virt_total); //store the calculated physical and virtual memory information as a string into strArr at index i
    
    return virt_used; //returns the virtual used memory
}

// modifies a string representation of the virtual memory usage of the system for future printing use
void modify_memory_graphics(int i, double virt_used, double *prev_virt, char strArr[][1024]){
    
    char line[1024]="\0", temp[1024]="\0"; // initialize empty strings
    int iter=0;
    double diff=0.00;

    strcpy(line, "   |");
    
    if(i==0)
        diff=0.00; //on first iteration, set diff to 0 since no previous iteration exists
    else
        diff=virt_used - *(prev_virt); //calculates the difference between the current virtual memory usage and the previous usage
    
    if(diff>=0.00 && diff<0.01){
        strcat(line, "o "); //if the differenece is nonnegative and less than 0.01 GB, then "o" is concatenated to 'line'
    } else if (diff<0 && diff>-0.01){
        strcat(line, "@ "); //if the difference is negative and greater than -0.01 GB, then "@" is concatenated to 'line'
    } else {
        iter = fabs((int) ((diff-(int)diff+0.005)*100)); //otherwise, stores the first two decimal places of the difference into a variable as an integer
        
        if(diff<0){
            for(int i=0; i<iter; i++) {strcat(line, ":");} //if difference is negative, the loop concatenates ':' to 'line' ('iter' number of times)
            strcat(line, "@ "); //then concatenates "@ " to the string
        } else {
            for(int i=0; i<iter; i++) {strcat(line, "#");} //if difference is nonnegative, the loop concatenates '#' to 'line' ('iter' number of times)
            strcat(line, "* "); //then concatenates "* " to the string
        }  

    }

    *(prev_virt)=virt_used; //the value of virt_used memory is stored in prev_virt for future calculation/use of prev_virt in the upcoming iterations of sampling
    sprintf(temp, "%.2f (%.2f)", diff, virt_used); //stores the difference of virtual memory (prev and cur) and virtual used memory into the 'temp' string 
    strcat(line, temp); //concatenates temp to line
    strcat(strArr[i], line); //'line' string that has been formatted is concatenated to strArr at the current iteration index passed from main
    
}

//displays memory usage information stored in an array of strings 'strArr' according to sequential flag
void display_memory_line(int sequential, int samples, int i, char strArr[][1024]){
    int j=0;
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n"); //prints header

    if(sequential){ //checks if sequential is true (i.e: 1)
        for(j=0; j<samples; j++){ //goes through all the elements in strArr
            if(j==i) //if the current iternation index equals the current index samples loop from main 
                printf("%s\n", strArr[j]); //then the corresponding current memory information stored in strArr at the index is printed
            else
                printf("\n"); //otherwise fill the lines with a new line
        }
    } else {
        for(j=0; j<=i; j++) printf("%s\n", strArr[j]); //if sequential is false, prints all the elements in strArr upto the current index 'i' passed from main
        for(int k=j+1; k<=samples; k++) printf("\n"); //fills the rest of the lines with a new line until it reaches # of samples
    }
}

//  updates and displays the current CPU usage graphics in a graphical format
void cpu_graphics(char cpuArr[][200], int i, int sequential, int *num_bar, float cur_cpu_usage, float *prev_cpu_usage){
    int diff_bar=0; //initialize a variable that records the difference in bars ("|") between iterations
    char cpuStr[200]="\0"; //initialize an empty string

    if(i==0){
        sprintf(cpuArr[i], "         |||%.2f", cur_cpu_usage); //on first iteration, stores default of 3 bars followed by current cpu usage into first element of cpuArr
    } else {
        diff_bar = (int)cur_cpu_usage - (int)(*(prev_cpu_usage)); //otherwise, calculates the difference in CPU usage between prev and cur iterations (only looking at the integer part)
        *(num_bar) += diff_bar; //updates the current 'num_bar' value with the new difference in number of bars calculated above
        strcpy(cpuArr[i], "         "); //copies space into cpuArr at index i
        
        for(int m=0; m<*(num_bar); m++) {
            strcat(cpuArr[i], "|"); //appends the number of bars represented by 'num_bar' and writes to cpuArr at index i
        }
        sprintf(cpuStr, "%.2f", cur_cpu_usage); //writes the current cpu usage upto 2 decimal places into cpuStr string
        strcat(cpuArr[i], cpuStr); //concatenates cpuStr into the already graphically formatted string of cpuArr at index i
    }

    if(sequential){ //checks if sequential is true (i.e. 1)
        for(int j=0; j<=i; j++){ //then iterate through the cpuArr upto the current iteration index i from main
            if(j==i) //if index j matches the current iteration index i from main
                printf("%s\n", cpuArr[j]); //prints the current iteration of cpuArr at index j
            else
                printf("\n"); //otherwise leave the rest blank (i.e. fill in the links with a new line)
        }
    } else { //if sequential is false
        for(int h=0; h<=i; h++){ //iterate through cpuArr upto the current iteration index i from main
            printf("%s\n", cpuArr[h]); //prints the entire cpuArr upto and including the current iteration
        }
    }

    *(prev_cpu_usage) = cur_cpu_usage; //sets the 'prev_cpu_usage' to 'cur_cpu_usage' for later iteration(s)
}

/* Main function, implementing the functionality to display memory, user, cpu usage of the system.
 * Takes two parameters, 'argc' (representing the number of arguments passed to the program) and
 * 'argv' (representing an array of the arguments passed to the program).
 */
int main(int argc, char *argv[])
{
    //initializing variables and flags used to control the display of information in the program
    int i, samples = 10, tdelay = 1, system = 0, user = 0, graphics = 0, sequential = 0, cmd; 

    //uses getopt_long to parse the command line options passed to the program
    struct option long_options[] = { //an array of 'struct option' objects, each line representing a single command line option
        {"system", no_argument, 0, 's'}, //takes "system" with no argument, returns 's' if option is present
        {"user", no_argument, 0, 'u'}, //takes "user" with no argument, returns 'u' if option is present
        {"graphics", no_argument, 0, 'g'}, //takes "graphics" with no argument, returns 'g' if option is present        
        {"sequential", no_argument, 0, 'q'}, //takes "sequential" with no argument, returns 'q' if option is present
        {"samples", optional_argument, 0, 'n'}, //takes "samples" with optional argument, returns 'n' if option is present
        {"tdelay", optional_argument, 0, 't'}, //takes "tdelay" with optional argument, returns 't' if option is present
        {0,0,0,0} //indicates the end of options
    };

    double virt_used=0.0, prev_virt=0.00; //declares and initializes variables for virtual current and previous memory usage
    float cur_cpu_usage=0.00, prev_cpu_usage=0.00; //declares and initializes variables for current and previous cpu usage
    char strArr[samples][1024]; //a 2D char array for storing memory display information
    char cpuArr[samples][200]; //a 2D char array for storing cpu display information
    int num_bar=3; //sets default num of bars for cpu graphics to 3
    unsigned long curSample[7]; //an unsigned long array of size 7 whose user, nice, system, idle, iowait, irq, softirq fields of cpu information in /proc/stat sampled at current time.
    unsigned long prevSample[7]; //an unsigned long array of size 7 whose user, nice, system, idle, iowait, irq, softirq fields of cpu information in /proc/stat sampled at an earlier time.

    //retrieves and processes command line options passed to the program using the 'getopt_long' function
    // stored in argv array, and returns the next option found in the argument list
    //loop continues until getopt_long returns -1, meaning all the options have been processed

    while ((cmd=getopt_long(argc, argv, "sugqn::t::", long_options, NULL)) != -1){ 
        //the string "sugqn::t::" specifies that he options -s, -u, -g, -q, -n and -t are available. 
        //The (::) following the letters n and t indicate that an optional argument, which the user can specify by appending a value to the option on the command line
        
        switch (cmd) { //switch statment to determine action to take based on the option returned by getopt_long
            case 's':
                system = 1; //in case cmd is 's', 'system' is set to 1
                break;
            case 'u':
                user = 1; //in case cmd is 'u', 'user' is set to 1
                break;
            case 'g':
                graphics = 1; //in case cmd is 'g', 'graphics' is set to 1
                break;
            case 'q':
                sequential = 1; //in case cmd is 'q', 'sequential' is set to 1
                break;
            case 'n':
                //in case cmd is 'n', if option has an argument, atoi converts the argument from string to integer and updates the value of samples 
                if (optarg) samples = atoi(optarg);
                break;
            case 't':
                //in case cmd is 't', if option has an argument, atoi converts the argument from string to integer and updates the value of tdelay 
                if (optarg) tdelay = atoi(optarg); 
                break;
        }

    }

    //performs iterations over the remaining command line arguments after the options
    // have been processed by 'getopt_long'. Starting from the index of the first non-option argument
    // given by 'optind', the loop iterates until ind reaches the number of arguments (argc)
    for(int ind = optind, iter=0; ind < argc; ind++, iter++) {
        //value of iter is used to determine which argument is being processed
        switch(iter){
            case 0: 
                samples = atoi(argv[ind]); //if iter is 0, then samples gets updated with the integer represenation of the string argument
                break;
            case 1:
                tdelay = atoi(argv[ind]); //if iter is 1, then tdelay gets updated with the integer representation of the string argument
                break;
        }
        //the use of iter maintains the order and functionality of the postional arguments
    }

    for (i = 0; i < samples; i++) { // iterate through the number of samples
        set_cpu_values(prevSample); //setting cpu values of 'prevSample'
        sleep(tdelay); //cause delay for tdelay seconds for sampling frequency

        display_header(i, sequential, samples, tdelay); //displays header information
        if(!user || (user && system)){ //runs so long as the argument doesn't contain just '--user'
            printf("---------------------------------------\n");
            virt_used = write_memory(strArr, i); //updates strArr at index i and returns virt_used memory

            if(graphics) 
                modify_memory_graphics(i, virt_used, &prev_virt, strArr); //modify strArr graphically if graphics is an option
            
            display_memory_line(sequential, samples, i, strArr); //displays lines of memory information according to sequential and strArr
            
            if((user && system)||!system){ //prints users if user and system are both options and skips if system option was given without user
                printf("---------------------------------------\n");
                print_users(); //prints current user information on server
                printf("---------------------------------------\n");
            }

            print_cores(); //print the number of cores
            
            set_cpu_values(curSample); //samples current cpu information 'curSample'

            cur_cpu_usage = calculate_cpu_usage(prevSample, curSample); //calculates and assigns 'cur_cpu_usage' based on 'prevSample' and 'curSample'
            printf(" total cpu use: %.2f%%\n", cur_cpu_usage); //prints current cpu usage upto 2 decimal places

            if(graphics)
                cpu_graphics(cpuArr, i, sequential, &num_bar, cur_cpu_usage, &prev_cpu_usage); //if graphics option is given, display cpu graphics
        
        }else{ //runs when only user option is given
            printf("---------------------------------------\n");
            print_users();
            printf("---------------------------------------\n");
        }
    }

    printf("---------------------------------------\n");
    print_machine_info(); //prints machine information all the time at the end
    printf("---------------------------------------\n");

    return 0;

    
}