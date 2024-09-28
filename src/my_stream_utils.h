#ifndef __MY_STREAM_UTILS__
#define __MY_STREAM_UTILS__

const double to_MB = (1024.0 * 1024.0);
const double to_GB = (1024.0 * 1024.0 * 1024.0);

char *find_command_line_arg_value(int argc, char *argv[], const char *arg);

const int find_command_line_arg_value_v2(const int argc, const char *argv[],
                                         const char *arg);

int flag_exists(int argc, char *argv[], const char *flag);

int is_number(char *str);

unsigned int generate_random_number(unsigned int seed);

double get_time(struct timespec start, struct timespec end);

double compute_bandwidth(const unsigned int nr_cpu,     //
                         const unsigned int nr_streams, //
                         const size_t batch_vec_size,   //
                         const double average_time,     //
                         const unsigned int word_size); //

double average(const double *v, unsigned int n);

double maximum(const double *v, unsigned int n);

double minimum(const double *v, unsigned int n);

double variance(const double *v, unsigned int n);

double std_dev(const double *v, unsigned int n);

void print_help();

void print_performance_metrics(double bandwidth_axpy, double avg_clock_axpy,
                               double bandwidth_fma, double avg_clock_fma,
                               double bandwidth_copy, double avg_clock_copy,
                               double bandwidth_addmul, double avg_clock_addmul,
                               double to_GB);

#endif // __MY_STREAM_UTILS__