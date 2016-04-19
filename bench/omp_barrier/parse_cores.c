#include <stdint.h>
#include <numa.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


static uint32_t* placement(uint32_t n, bool do_fill)
{
    uint32_t* result = (uint32_t*) malloc(sizeof(uint32_t)*n);
    uint32_t numa_nodes = numa_max_node()+1;
    uint32_t num_cores = numa_num_configured_cpus();
    struct bitmask* nodes[numa_nodes];

    for (int i = 0; i < numa_nodes; i++) {
        nodes[i] = numa_allocate_cpumask();
        numa_node_to_cpus(i, nodes[i]);
    }

    int num_taken = 0;
    if (numa_available() == 0) {
        if (do_fill) {
            for (int i = 0; i < numa_nodes; i++) {
                for (int j = 0; j < num_cores; j++) {
                    if (numa_bitmask_isbitset(nodes[i], j)) {
                        result[num_taken] = j;
                        num_taken++;
                    }

                    if (num_taken == n) {
                        return result;
                    }
                }
           }
        } else {
            int cores_per_node = n/numa_nodes;
            int rest = n - (cores_per_node*numa_nodes);
            int taken_per_node = 0;

            for (int i = 0; i < numa_nodes; i++) {
                for (int j = 0; j < num_cores; j++) {
                    if (numa_bitmask_isbitset(nodes[i], j)) {
                        if (taken_per_node == cores_per_node) {
                            if (rest > 0) {
                                result[num_taken] = j;
                                num_taken++;
                                rest--;
                                if (num_taken == n) {
                                    return result;
                                }
                            }
                            break;
                        }
                        result[num_taken] = j;
                        num_taken++;
                        taken_per_node++;

                        if (num_taken == n) {
                            return result;
                        }
                    }
                }
                taken_per_node = 0;
            }
        }
    } else {
        printf("Libnuma not available \n");
        return NULL;
    }
    return NULL;
}

int main(int argc, char** argv){

    uint32_t num_cores = numa_num_configured_cpus();
    uint32_t num_nodes = numa_max_node()+1;

    if (argv[1]) {
        // hyperthreads enabled
        num_cores = num_cores/2;
    }
    if (num_nodes < 2) {
        num_nodes = 2;
    }

    printf("%d \n", num_nodes);

    for (int j = num_nodes; j < num_cores+1; j+=num_nodes) {
        uint32_t* rr = placement(j, false);
        uint32_t* fill = placement(j, true);
	  printf("FILL: ");
        for (int i = 0; i < j; i++) {
            if (i+1 < j) {
                printf("%d,", fill[i]);
            } else {
                printf("%d", fill[i]);
            }
        }
        printf("\n");
	  
	  printf("RR: ");
         for (int i = 0; i < j; i++) {
            if (i+1 < j) {
                printf("%d,", rr[i]);
            } else {
                printf("%d", rr[i]);

            }
        }
        printf("\n");
    }


}
