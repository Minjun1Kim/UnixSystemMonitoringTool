# <span style="color:#ADD8E6">Assignment 1: System Monitoring Tool</span>


<div align="right"> Created by Minjun Kim </div>


## <span style="color:#ADD8E6">Table of Contents </span> 
- [Description](#description)
- [Requirements](#requirements)
- [Usage](#usage)
- [Functions](#functions)
- [Problem Solving](#problemsolving)
- [Notes](#notes)
- [References](#references)

<br />

<a id="description"></a>
## <span style="color:#ADD8E6">Description </span> 

This program displays information about the system, including the number of samples taken, the time delay between samples, memory usage, number of cores, CPU usage, and the current user sessions. It retrieves the information by using system calls and reading from the /proc/stat file.

<br />

<a id="requirements"></a>
## <span style="color:#ADD8E6">Requirements </span>

The program requires the following libraries to be installed: <br />
- [`<stdio.h>`](https://man7.org/linux/man-pages/man3/stdio.3.html)<br />
- [`<stdlib.h>`](https://man7.org/linux/man-pages/man0/stdlib.h.0p.html)<br />
- [`<unistd.h>`](https://man7.org/linux/man-pages/man0/unistd.h.0p.html)<br />
- [`<string.h>`](https://man7.org/linux/man-pages/man0/string.h.0p.html)<br />
- [`<sys/utsname.h>`](https://man7.org/linux/man-pages/man0/sys_utsname.h.0p.html)<br />
- [`<sys/sysinfo.h>`](https://man7.org/linux/man-pages/man2/sysinfo.2.html)<br />
- [`<sys/resource.h>`](https://man7.org/linux/man-pages/man0/sys_resource.h.0p.html)<br />
- [`<utmp.h>`](https://man7.org/linux/man-pages/man5/utmp.5.html)<br />
- [`<getopt.h>`](https://man7.org/linux/man-pages/man3/getopt.3.html)<br />
- [`<math.h>`](https://man7.org/linux/man-pages/man0/math.h.0p.html) <br />

<br />

<a id="usage"></a>
## <span style="color:#ADD8E6">Usage</span> 

To run the program, compile the code using the following command:

```console
$ gcc -o mySytemStats mySystemStats.c
```

And then run the executable:

```console
$ ./mySytemStats
```

The program also accepts several command line arguments such as: <br />

  `--system`
<details>
    <summary>Click to expand</summary>

```console
$ ./mySytemStats --system
```

* to indicate that only the system usage should be generated.
</details>

<br />

`--user`
<details>
    <summary>Click to expand</summary>

```console
$ ./mySytemStats --user
```

* to indicate that only the users usage should be generated.
</details>

<br />
  
`--graphics`

<details>
  <summary>Click to expand</summary>

```console
$ ./mySytemStats --graphics
```

OR

```console
$ ./mySytemStats -g
```

  * to include graphical output in the cases where a graphical outcome is possible.

</details>

<br />

`--sequential`

<details>
  <summary>Click to expand</summary>

```console
$ ./mySytemStats --sequential
```

OR 

```console
$ ./mySytemStats -q
```

  * to indicate that the information will be output sequentially without needing to "refresh" the screen.

</details>

<br />

`--samples=N`

<details>
  <summary>Click to expand</summary>

```console
$ ./mySytemStats --samples=8
```

  * to indicate the number of times (N) the statistics are going to be collected and results will be reported based on the N number of iterations.
  * Note: if value is not indicated, the default value of 10 samples will be used.

</details>

<br />

`--tdelay=T`

<details>
  <summary>Click to expand</summary>

```console
$ ./mySytemStats --tdelay=2
```

  * to indicate how frequently to sample in seconds.
  * Note: if value is not indicated, the default value of 1 second will be used.

</details>

<br />


<details>
  <summary>Multiple Arguments</summary>

```console
$ ./mySytemStats --system --user -q -g 5 2
```
  * prints sequentially 5 graphical samples of cpu, system, and user information with 2 seconds of delay in between.

<br />

- It is my design choice not to show graphics when the following command is entered:

    ```console
    $ ./mySytemStats --user --graphics
    ```
    as I understood the following instruction to be only concerned with the users part and not CPU usage:
    >  `--user`: to indicate that only the users usage should be generated

</details>

<br />

<a id="functions"></a>
## <span style="color:#ADD8E6">Functions</span>
-   ```c
    void display_header(int i, int sequential, int samples, int tdelay);
    ```
    <details>
    <summary>Overview</summary>

    - return type: `void`
    - parameters:

        - `int i`: the current iteration of the samples from the main function
        - `int sequential`: sequential flag to check whether sequential is requested
        - `int samples`: the number of samples the statistics are being sampled by the program
        - `int tdelay`: the number of seconds delayed between samples

        <br />

    - displays to stdout the number of samples and the time delay between samples.
    - if sequential, prints to stdout the iteration number. Otherwise, uses ANSI escape codes to clear the terminal screen and move the cursor to the top-left corner
    - prints to stdout memory usage in kilobytes by dereferencing the ru_maxrss field of the rusage struct found in `<sys/resource.h>`

    </details>
    <br />

-   ```c
    void print_cores(void);
    ```

    <details>
    <summary>Overview</summary>

    - return type: `void`
    - parameters:
        <br/>
        - None
    - prints to stdout the number of currently available processors using the `sysconf()` function found in `<unistd.h>`.
    - Note: sysconf returns the number of currently available processors.

    </details>
    <br />

-   ```c
    void set_cpu_values(unsigned long cpuArr[7]);
    ```

    <details>
    <summary>Overview</summary>

    - return type: `void`
    - parameters:
        - `unsigned long cpuArr[7]`: an unsigned long array of size 7 representing user, nice, system, idle, iowait, irq, softirq fields respectively from index 0 to 6 in the file /proc/stat.

        <br />
    - given an unsigned long array representing the 7 fields of the first line of total cpu information in /proc/stat as described above, the function reads the first line and assigns appropriate values to the elements in the array.

    </details>
    <br />


-   ```c
    double calculate_cpu_usage(unsigned long prev[7], unsigned long cur[7]);
    ```
    <details>
    <summary>Overview</summary>

    - return type: `double`
    - parameters:
        - `unsigned long prev[7]`: an unsigned long array of size 7 whose user, nice, system, idle, iowait, irq, softirq fields of cpu information in /proc/stat were sampled at an earlier time.
        - `unsigned long cur[7]`: an unsigned long array of size 7 whose user, nice, system, idle, iowait, irq, softirq fields of cpu information in /proc/stat were sampled at a current time.

        <br />
    - given two unsigned long arrays prev and cur representing the 7 fields of /proc/stat as described above that were sampled at different times, it calculates and returns the cpu usage percentage using the following formula: 
        ```c 
        (1000 * ((total_diff - idle_diff) / total_diff) + 1) / 10
        ```

    - the function calculates the CPU usage percentage by subtracting the previous idle time and total time values from the current values and using the result to calculate the CPU usage as a percentage.

    - total_diff - idle_diff gives the difference in CPU utilization between two points in time that excludes idle time, which gives a measure of how busy the CPU was with non_idle tasks between two sample points in time (referred to as utilization of the CPU). Dividing this by the difference of total CPU time yields the fraction of CPU time spent on non_idle activities, which is then multiplied by 1000 to convert the decimals into a whole number. Then 1 is added to avoid division by 0 and to avoid undefined value. Then divided by 10 to get a range between 0-100% as percentage.

    - The total time is calculated by adding the idle time and non-idle time values for both previous and current data. The difference between the current and previous total and idle time values are used to calculate the CPU usage as a percentage and return the result as a double value.

    </details>
    <br />


-   ```c
    void print_users(void);
    ```
    <details>
    <summary>Overview</summary>

    - return type: `void`
    - parameters:
        - none

        <br />

    - lists information about the current terminal sessions/users logged onto a Unix-like operating system.
    - prints information in username, terminal line, and ut_host format.
    - does so by using the setutent, getutent, and endutent functions (all found in utmp.h file), which provide access to the utmp file.

    </details>
    <br />


-   ```c
    void print_machine_info(void);
    ```
    <details>
    <summary>Overview</summary>

    - return type: `void`
    - parameters:
        - none
        <br />
    - displays system information on a Unix-like operating system.
    - uses the uname function from sys/utsname.h file to obtain information about the system and stores it in a struct utsname object.
    - prints the following information from the utsname structure: system name, machine name, version, release, and architecture.

    </details>
    <br />


-   ```c
    double write_memory(char strArr[][1024], int i);
    ```
    <details>
    <summary>Overview</summary>

    - return type: `double`
    - parameters:
        - `char strArr[][1024]`: a 2D char array that stores memory information to be printed.
        - `int i`: the current iteration of the samples from the main function
        <br />
    - given a 2D char array `strArr`, the function stores memory information into the array and returns the current virtual used memory.
    - does so by using sysinfo struct from `sys/sysinfo.h` file and dereferencing the necessary fields to obtain data.

    </details>
    <br />


-   ```c
    void modify_memory_graphics(int i, double virt_used, double *prev_virt, char strArr[][1024]);
    ```
    <details>
    <summary>Overview</summary>

    - return type: `void`
    - parameters:
        - `int i`: the current iteration of the samples from the main function
        - `double virt_used`: virtual used memory at the current time
        - `double *prev_virt`: a `double` pointer to virtual memory sampled at an earlier time.
        - `char strArr[][1024]`: a 2D char array that stores memory information to be printed.

        <br />
    - given a 2D char array `strArr`, the function modifies memory information into the array.
    - modifies a string representation of the virtual memory usage of the system for future printing use at index i of the `strArr`
    - concatenates characters such as ':' or '#' for graphical purposes depending on the magnitude of the difference between current virtual memory usage and previous memory usage.

    </details>
    <br />


-   ```c
    void display_memory_line(int sequential, int samples, int i, char strArr[][1024]);
    ```
    <details>
    <summary>Overview</summary>

    - return type: `void`
    - parameters:
        - `int sequential`: sequential flag for displaying statistics in sequential manner.
        - `int samples`: number of samples to be collected
        - `int i`: the current iteration of the samples from the main function
        - `char strArr[][1024]`: a specifically formatted 2D char array that stores memory information to be printed.

        <br />
    - given a 2D char array `strArr`, the function displays memory information stored in the array according to the sequential flag.
    - iterates through the loops and prints memory information or empty lines in appropriate conditions.
    - implements the dispaly of graphical part of the memory information.

    </details>
    <br />

-   ```c
    void cpu_graphics(char cpuArr[][200], int i, int sequential, int *num_bar, float cur_cpu_usage, float *prev_cpu_usage);
    ```
    <details>
    <summary>Overview</summary>

    - return type: `void`
    - parameters:

        - `char cpuArr[][200]`: a 2D char array which stores specifically formatted strings of cpu information.
        - `int i`: the current iteration of the samples from the main function
        - `int sequential`: sequential flag for displaying statistics in sequential manner.
        - `int *num_bar`: int pointer to num_bar, which represents the number of bars that signifies the magnitude of the change between current and previous cpu usage.
        - `float cur_cpu_usage`: cpu usage sampled and calculated at current time.
        - `float *prev_cpu_usage`: cpu usage sampled and calculated at an earlier time
     
    <br />

    - given a 2D char array `cpuArr`, the function modifies the entries of the array by concatenating characters and symbols for graphical purposes and displays cpu graphics information stored in the array according to the sequential flag.
    - updates and displays the current CPU usage graphics in a graphical format through `cpuArr`.
    - uses various float operations and manipulations to obtain the number of bars to be printed.

    </details>
    <br />

<a id="problemsolving"></a>
## <span style="color:#ADD8E6">Problem Solving</span>
- How did I solve the problem(s)?

    - The first of many problems I had over the course of the assignment was regarding cpu usage
    calculation. Every office hours I had attended, I received a different answer regarding the formula. However, in one office hours held by a TA, they suggested that I use a formula from a particular website so long as I understood what the calculation meant. So, I took the time to understand the formula and reason about why it is valid by looking at articles about cpu usage.
    Although it was not a quick process, I was able to make sense of the formula and use it in my program to calculate the CPU usage with modifications in the code.
    
    - Secondly, I had some trouble finding the right functions to use for each functionality of the program. Fortunately, I took some time to read the manual pages that were provided in the assignment handout thoroughly, and asking the TAs for confirmation about the correct usage, I was able to use them and implement the required functionalities for my program.

    - Additionally, I was a bit stuck on how to approach the problem of parsing the command line arguments and postional arguments. Fortunately, my TA suggested the idea of using getopt_long which eliminated all the problems I'd had with the command line arguments.
    
    - Most importantly, I approached most problems I'd encountered by analyzing the behaviour of the program provided in Marcelo's video and brainstorming about how to solve the problem in my head before starting coding. For example, for the CPU graphics part and the number of bars that change with each iteration, I observed the pattern in the video and came up with the solution of taking the difference of the integer parts of the float current and previous CPU usage values and typecast the result to get the needed number of bars. Everything made sense when I took the time to think and brainstorm solutions. Lastly, office hours helped a lot with my understanding of some unclear instructions in the handout.

<a id="notes"></a>
## <span style="color:#ADD8E6">Notes</span>

- The utility only works on Linux systems.
- The program uses the `/proc/stat` file to obtain information about the system, including CPU usage. The file is constantly updated by the system, so the information displayed may change over time.
- Was told by Marcelo that it is okay to use the `sysconf` function for counting the number of cores, instead of counting the number of cpu lines in /proc/stat
- Was told by a couple of TAs to use the formula listed in the [link](https://www.kgoettler.com/post/proc-stat/) so long as the explaination makes sense and I can demonstrate my understanding.
- The code assumes that there are at most 2 non-option arguments and that they must appear in the order samples then tdelay. If there are more or fewer arguments, or if they appear in a different order, the code might not work as intended.

<a id="references"></a>
## <span style="color:#ADD8E6">References</span>
- How to get total cpu usage in C++: [StackOverFlow](https://stackoverflow.com/questions/3017162/how-to-get-total-cpu-usage-in-linux-using-c)
- Reading /proc/stat: [kgottler](https://www.kgoettler.com/post/proc-stat/)
- Understanding cpu usage: [OpsDash](https://www.opsdash.com/blog/cpu-usage-linux.html)
- getopt(3) — [Linux manual page](https://man7.org/linux/man-pages/man3/getopt.3.html)
- Linux C library manual pages as linked in [requirements](#requirements)

<a id="contact"></a>
## <span style="color:#ADD8E6">Contact</span>
- creator: Minjun Kim
- email address: minjunn.kim@mail.utoronto.ca
