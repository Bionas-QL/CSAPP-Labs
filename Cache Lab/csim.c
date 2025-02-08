#include "cachelab.h"
#include<getopt.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>

void helper()
{
	printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
	printf("Options:\n");
	printf("  -h         Print this help message.\n");
	printf("  -v         Optional verbose flag.\n");
	printf("  -s <num>   Number of set index bits.\n");
	printf("  -E <num>   Number of lines per set.\n");
	printf("  -b <num>   Number of block offset bits.\n");
	printf("  -t <file>  Trace file.\n");
}

int main(int argc, char *argv[])
{
	int opt;
	int verb_flag = 0;
	int s = 5;
	int E = 1;
	int b = 5;
	char *file_address = NULL;
	while ((opt = getopt(argc, argv, "h::v::s:E:b:t:")) != -1)
	{
		switch (opt) {
			case 'h':
				helper();
				return 0;
			case 'v':
				verb_flag = 1;
				break;
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
			case 't':
				file_address = optarg;
				break;
			case '?':
				if (optopt == 's' || optopt == 'E' || optopt == 'b' || optopt == 't')
					fprintf(stderr, "The option -%c requires an argument.\n", optopt);
				else
					fprintf(stderr, "The option -%c does not exist", optopt);
				return 1;
			default:
				return 1;
		}
	}

	if (s + b > 64)
	{
		fprintf(stderr, "Illegal inputs");
		return 1;
	}
	FILE *file = fopen(file_address, "r");
	char line[64];
	int total_hit = 0;
	int total_miss = 0;
	int total_eviction = 0;
	long time_stamp = 0;
	long *my_cache = (long *) malloc((1 << s) * E * sizeof(long) * 3);
	memset(my_cache, 0, (1 << s) * E * sizeof(long) * 3);
	while (fgets(line, sizeof(line), file))
	{
		if (line[0] != 'I')
		{
			int index = 0;
			while (line[index] != ',')
				index++;
			line[index] = '\0';
			line[index + 2] = '\0';
			long current_address = strtol(line + 3, NULL, 16);
			long current_tag = current_address >> (s + b);
			long current_s = (current_address >> b) % (1 << s);
			long *start = my_cache + current_s * 3 * E;
			int empty_flag = 0;
			int empty_index = 0;
			int miss_flag = 1;
			int oldest_index = 0;
			long oldest_time_stamp = time_stamp;
			line[index] = ',';
			for (int i = 0; i < E; i++)
			{
				long *current_block = start + 3 * i;
				if (current_block[0] == 0)
				{
					if (!empty_flag)
					{
						empty_flag = 1;
						empty_index = i;
					}
				}
				if (current_block[0] == 1 && current_block[1] == current_tag)
				{
					if (line[1] == 'M')
					{
						total_hit += 2;
						if (verb_flag)
						{
							printf("%s", line);
							printf(" hit hit\n");
						}
					}
					else
					{
						total_hit++;
						if (verb_flag)
						{
							printf("%s", line);
							printf(" hit\n");
						}
					}
					miss_flag = 0;
					current_block[2] = time_stamp;
					time_stamp++;
					break;
				}
				if (current_block[2] <= oldest_time_stamp)
				{
					oldest_time_stamp = current_block[2];
					oldest_index = i;
				}
			}
			if (miss_flag)
			{
				if (empty_flag)
				{
					long *block = start + 3 * empty_index;
					block[0] = 1;
					block[1] = current_tag;
					block[2] = time_stamp;
					time_stamp++;
					if (line[1] == 'M')
					{
						total_miss++;
						total_hit++;
						if (verb_flag)
						{
							printf("%s", line);
							printf(" miss hit\n");
						}
					}
					else
					{
						total_miss++;
						if (verb_flag)
						{
							printf("%s", line);
							printf(" miss\n");
						}
					}
				}
				else
				{
					long *block = start + 3 * oldest_index;
					block[1] = current_tag;
					block[2] = time_stamp;
					time_stamp++;
					if (line[1] == 'M')
					{
						total_miss++;
						total_hit++;
						total_eviction++;
						if (verb_flag)
						{
							printf("%s", line);
							printf(" miss eviction hit\n");
						}
					}
					else
					{
						total_miss++;
						total_eviction++;
						if (verb_flag)
						{
							printf("%s", line);
							printf(" miss eviction\n");
						}
					}
				}

			}
		}
	}
	free(my_cache);
    printSummary(total_hit, total_miss, total_eviction);
    return 0;
}
