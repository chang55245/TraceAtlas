/* Copyright 2020 Los Alamos National Laboratory
 * Licensed under the Apache License, Version 2.0 */

#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <algorithm> 
#include <unistd.h>
#include "core.h"
#include "timer.h"
#include "dash.h"

#define VERBOSE_LEVEL 0
#define USE_CORE_VERIFICATION

extern "C" {
typedef struct tile_s {
  float dep;
  char *output_buff;
}tile_t;

typedef struct payload_s {
  int x;
  int y;
  TaskGraph graph;
}payload_t;

typedef struct matrix_s {
  tile_t *data;
  int M;
  int N;
}matrix_t;

// Single scratch memory buffer for serial version
char *scratch_memory = NULL;

// Unified task execution function
static inline void execute_task(tile_t *tile_out, tile_t **tile_in_array, size_t num_inputs, payload_t payload)
{
#if defined (USE_CORE_VERIFICATION)
  TaskGraph graph = payload.graph;
  char *output_ptr = tile_out->output_buff; // Assuming output_buff is valid
  size_t output_bytes = graph.output_bytes_per_task;
  std::vector<const char *> input_ptrs;
  std::vector<size_t> input_bytes;

  // Add input dependencies from the array
  for (size_t i = 0; i < num_inputs; i++) {
    // Optional: Add null checks for tile_in_array[i] and tile_in_array[i]->output_buff if needed
    input_ptrs.push_back((const char*)tile_in_array[i]->output_buff);
    input_bytes.push_back(graph.output_bytes_per_task);
  }

  // For task1 case (no dependencies), add its own buffer as "input"
  if (num_inputs == 0) {
      input_ptrs.push_back((const char*)tile_out->output_buff);
      input_bytes.push_back(graph.output_bytes_per_task);
      printf("execute_task (no deps) timestep: %d, point: %d\n", payload.y, payload.x);
  } else {
      printf("execute_task (%zu deps) timestep: %d, point: %d\n", num_inputs, payload.y, payload.x);
  }


  graph.execute_point(payload.y, payload.x, output_ptr, output_bytes,
                     input_ptrs.data(), input_bytes.data(), input_ptrs.size(),
                     scratch_memory, graph.scratch_bytes_per_task);
#else
  tile_out->dep = 0;
   if (num_inputs == 0) {
       // Original task1 logic (if any non-verification logic was needed)
       printf("Execute Task (no deps) x %d, y %d, out %f\n", payload.x, payload.y, tile_out->dep);
   } else {
        for (size_t i = 0; i < num_inputs; i++) {
            if (tile_in_array[i] == NULL) {
              fprintf(stderr, "Error: NULL pointer in execute_task input %zu at position (%d, %d)\n",
                      i, payload.x, payload.y);
              continue;
            }
            tile_out->dep += tile_in_array[i]->dep;
        }
        printf("Execute Task (%zu deps) x %d, y %d, out %f\n", num_inputs, payload.x, payload.y, tile_out->dep);
   }
#endif
}

// Similar modifications for task3-task10...

// Updated function signature and implementation
void handle_task_dependencies(tile_t *output_tile, // Pass the specific output tile
                            payload_t payload,
                            tile_t **task_inputs, // Pass the array of input pointers
                            size_t num_inputs)     // Pass the number of inputs
{
    // KernelEnter("FFT"); // Moved to caller
    execute_task(output_tile, task_inputs, num_inputs, payload);
    // KernelExit("FFT"); // Moved to caller
}

struct SerialApp : public App {
  SerialApp(int argc, char **argv);
  ~SerialApp();
  void execute_main_loop();
  void execute_timestep(size_t idx, long t);
private:
  void debug_printf(int verbose_level, const char *format, ...);
private:
  matrix_t *matrix;
};

SerialApp::SerialApp(int argc, char **argv)
  : App(argc, argv)
{ 
  matrix = (matrix_t *)malloc(sizeof(matrix_t) * graphs.size());
  
  size_t max_scratch_bytes_per_task = 0;
  
  for (unsigned i = 0; i < graphs.size(); i++) {
    TaskGraph &graph = graphs[i];
    
    matrix[i].M = graph.nb_fields;
    matrix[i].N = graph.max_width;
    matrix[i].data = (tile_t*)malloc(sizeof(tile_t) * matrix[i].M * matrix[i].N);
  
    for (int j = 0; j < matrix[i].M * matrix[i].N; j++) {
      matrix[i].data[j].output_buff = (char *)malloc(sizeof(char) * graph.output_bytes_per_task);
    }
    
    if (graph.scratch_bytes_per_task > max_scratch_bytes_per_task) {
      max_scratch_bytes_per_task = graph.scratch_bytes_per_task;
    }
  }
  
  if (max_scratch_bytes_per_task > 0) {
    scratch_memory = (char*)malloc(sizeof(char)*max_scratch_bytes_per_task);
    TaskGraph::prepare_scratch(scratch_memory, sizeof(char)*max_scratch_bytes_per_task);
  }
}

SerialApp::~SerialApp()
{
  for (unsigned i = 0; i < graphs.size(); i++) {
    for (int j = 0; j < matrix[i].M * matrix[i].N; j++) {
      // Defensive check before freeing
      if (matrix[i].data != NULL && matrix[i].data[j].output_buff != NULL) {
          free(matrix[i].data[j].output_buff);
          matrix[i].data[j].output_buff = NULL;
      }
    }
    // Defensive check before freeing
    if (matrix[i].data != NULL) {
        free(matrix[i].data);
        matrix[i].data = NULL;
    }
  }

  // Defensive check before freeing
  if (matrix != NULL) {
      free(matrix);
      matrix = NULL;
  }

  if (scratch_memory != NULL) {
    free(scratch_memory);
    scratch_memory = NULL;
  }
}

void SerialApp::execute_main_loop()
{
  display();

  Timer::time_start();
  // Assuming only one graph for simplicity based on the loop `i < 1`
  
  const TaskGraph &g = graphs[0]; // Use graph 0
  unsigned graph_idx = 0; // Explicitly use index 0


  // --- Privatization Setup ---
  // Consider sizing based on max needed, not fixed 300 / 8000
  // Or rethink if full privatization per task is necessary.
  size_t max_tasks_in_flight = g.timesteps * g.max_width; // Example max size
  if (max_tasks_in_flight == 0) max_tasks_in_flight = 1; // Avoid zero allocation

  tile_t **private_tile = (tile_t **)malloc(sizeof(tile_t *) * max_tasks_in_flight);
  // Initialize pointers to NULL
  for (size_t i = 0; i < max_tasks_in_flight; ++i) {
      private_tile[i] = NULL;
  }

  // Allocate memory for each task's private copy (if needed)
  // This allocates potentially huge amounts of memory. Review if this deep copy is required.
  size_t matrix_size_bytes = sizeof(tile_t) * matrix[graph_idx].M * matrix[graph_idx].N;
  if (matrix_size_bytes > 0) {
      for (size_t i = 0; i < max_tasks_in_flight; i++) {
          private_tile[i] = (tile_t *)malloc(matrix_size_bytes);
           // Consider adding check for malloc failure

      }
  } else {
       fprintf(stderr, "Warning: Matrix size is zero for graph %u.\n", graph_idx);
       // Avoid allocation if size is zero, but this might indicate an issue earlier.
  }

  // Allocate other private metadata arrays
  // Use calloc for zero initialization which simplifies cleanup
  payload_t *private_payload = (payload_t *)calloc(max_tasks_in_flight, sizeof(payload_t));
  // Add checks for allocation failures for all calloc/malloc calls

  // Allocate storage for privatized dependency counts
  size_t *private_num_inputs = (size_t *)calloc(max_tasks_in_flight, sizeof(size_t));


  // Allocate the array of pointers for privatized dependency pointer arrays
  tile_t ***private_input_ptr_arrays = (tile_t ***)malloc(sizeof(tile_t **) * max_tasks_in_flight);

  // Initialize pointers to NULL
  for (size_t i = 0; i < max_tasks_in_flight; ++i) {
      private_input_ptr_arrays[i] = NULL;
  }

  // Pre-allocate each task's dependency pointer array
  size_t max_deps_per_task = matrix[graph_idx].N;
  if (max_deps_per_task == 0) { // Handle zero width case
      fprintf(stderr, "Warning: Matrix width is 0 for graph %u. Using minimum dependency buffer size 1.\n", graph_idx);
      max_deps_per_task = 1;
  }
  for (size_t i = 0; i < max_tasks_in_flight; ++i) {
      private_input_ptr_arrays[i] = (tile_t **)malloc(sizeof(tile_t*) * max_deps_per_task);
  }


  // --- Main Loop ---
  size_t task_counter = 0; // Keep track of the current private buffer index

  for (long y = 0; y < g.timesteps; y++) { // Use long for timestep type consistent with graph API

    long offset = g.offset_at_timestep(y);
    long width = g.width_at_timestep(y);
    long dset = g.dependence_set_at_timestep(y);
    int nb_fields = g.nb_fields;

    payload_t current_payload; // Use a temporary payload for the current task
    current_payload.graph = g; // Assign graph once per timestep

    int bound = width;
    printf("Timestep %ld, Width: %ld, Offset: %ld, Bound: %d\n", y, width, offset, bound);

    for (int x_i = 0; x_i < bound; x_i++) {
        NonKernelSplit();
        long x = x_i + offset; // Use long for consistency

        size_t current_private_idx = task_counter++; // Index for private arrays for this task
        // ... [Check for exceeding max_tasks_in_flight] ...
        // ... [Check for x bounds] ...

        // --- Dependency Calculation ---
        std::vector<std::pair<long, long>> deps = g.dependencies(dset, x);
        // Use the pre-allocated C-style array for this task's index
        size_t num_actual_inputs = 0; // Counter for actual dependencies found for *this task*
        tile_t **current_task_input_array = private_input_ptr_arrays[current_private_idx]; // Get pointer to this task's input array


        for (size_t i = 0; i < deps.size(); i++) {
            for (long dep_x = deps[i].first; dep_x <= deps[i].second; dep_x++) {
                if (dep_x >= 0 && dep_x < matrix[graph_idx].N) {
                    // Ensure previous timestep 't-1' wraps correctly using modulo
                    // Adding nb_fields before modulo handles negative t-1 result if t=0
                    int prev_row_idx = (y - 1 + nb_fields) % nb_fields;
                    long dep_array_idx = prev_row_idx * matrix[graph_idx].N + dep_x;

                    // Bounds check for the calculated dependency index
                    if (dep_array_idx >= 0 && dep_array_idx < matrix[graph_idx].M * matrix[graph_idx].N) {
                        // Check if data pointer is valid before adding
                        if (matrix[graph_idx].data != NULL) {
                           // Check if we exceed the allocated temporary buffer size
                           if (num_actual_inputs < max_deps_per_task) {
                               // Store pointer directly into the privatized array for this task
                               current_task_input_array[num_actual_inputs++] = &matrix[graph_idx].data[dep_array_idx];
                           } else {
                               fprintf(stderr, "Error: Exceeded maximum allocated dependency buffer size (%zu) for task (%ld, %ld).\n", max_deps_per_task, x, y);
                               // ... (handle error) ...
                           }
                        } else {
                           fprintf(stderr, "Warning: Skipping dependency due to NULL matrix data pointer at (%ld, %ld).\n", dep_x, y-1);
                        }
                    } else {
                         fprintf(stderr, "Warning: Dependency index %ld out of bounds [0, %d) for task (%ld, %ld).\n",
                                 dep_array_idx, matrix[graph_idx].M * matrix[graph_idx].N, x, y);
                    }
                } else {
                     fprintf(stderr, "Warning: Dependency x index %ld out of bounds [0, %d) for task (%ld, %ld).\n",
                             dep_x, matrix[graph_idx].N, x, y);
                }
            }
        }
        // Store the final count for this task
        private_num_inputs[current_private_idx] = num_actual_inputs;


        // --- Prepare Task ---
        current_payload.x = x;
        current_payload.y = y;

        printf("Preparing task: timestep=%ld, x=%ld, num_deps=%zu\n", y, x, private_num_inputs[current_private_idx]); // Use privatized count

        int row_idx = y % nb_fields;
        long output_array_idx = row_idx * matrix[graph_idx].N + x;

        // Copy *entire matrix state* to private buffer for this task - REVIEW THIS LOGIC
        memcpy(private_tile[current_private_idx], matrix[graph_idx].data, matrix_size_bytes);

        // Store payload in private array
        private_payload[current_private_idx] = current_payload;


        // --- Execute Task ---
        KernelEnter("FFT");

        // Call handle_task_dependencies with the privatized C-style array and count
        handle_task_dependencies(&private_tile[current_private_idx][output_array_idx],                       // Pointer to output tile in private buffer
                               private_payload[current_private_idx],   // Payload for this task
                               private_input_ptr_arrays[current_private_idx], // Pointer to *this task's* array of input pointers
                               private_num_inputs[current_private_idx]);    // *This task's* number of dependencies
        KernelExit("FFT");


        // --- Post Task ---
        // The original code copied private data back here. This is removed as it overwrites
        // potentially needed state for subsequent tasks in the same timestep if dependencies exist.
        // The results should likely be copied back *after* a full timestep or synchronization point.
        // memcpy(matrix[graph_idx].data, private_tile[current_private_idx], matrix_size_bytes); // Removed/Commented

        NonKernelSplit();
    } // End x_i loop

    // Free the temporary input pointer array allocated for this timestep - REMOVED (now freed in main cleanup)
    // free(temp_input_ptr_array);
    // temp_input_ptr_array = NULL;

    // Potential synchronization point or copy-back of results after timestep 'y' could go here.
  } // End y loop


  // --- Cleanup ---
  printf("Cleaning up private resources...\n");
  if (private_tile != NULL) {
      for (size_t i = 0; i < task_counter; i++) { // Clean up only allocated buffers
        if (private_tile[i] != NULL) {
            free(private_tile[i]);
            private_tile[i] = NULL; // Avoid double free
        }
      }
      free(private_tile);
      private_tile = NULL;
  }

  // Free privatized input pointer arrays
  if (private_input_ptr_arrays != NULL) {
      // Free each inner array up to the actual number of tasks processed
      for (size_t i = 0; i < task_counter; i++) {
           if (private_input_ptr_arrays[i] != NULL) {
               free(private_input_ptr_arrays[i]);
               private_input_ptr_arrays[i] = NULL;
           }
      }
       // Alternatively, if all were allocated up to max_tasks_in_flight:
       // for (size_t i = 0; i < max_tasks_in_flight; i++) { ... }

      // Free the outer array
      free(private_input_ptr_arrays);
      private_input_ptr_arrays = NULL;
  }

  // Free privatized dependency counts
  free(private_num_inputs); // Already checked for NULL during allocation

  // Free other private arrays using free (since allocated with malloc/calloc)
  free(private_payload);

  double elapsed = Timer::time_end();
  report_timing(elapsed);
}

void SerialApp::debug_printf(int verbose_level, const char *format, ...)
{
  if (verbose_level > VERBOSE_LEVEL) {
    return;
  }
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

int main(int argc, char ** argv)
{
  //-steps 10 -width 10 -type fft -kernel compute_bound -iter 1024 -worker 1
  argc = 8;
  argv[0] = "-steps";
  argv[1] = "10";
  argv[2] = "-width";
  argv[3] = "10";
  argv[4] = "-type";
  argv[5] = "stencil_1d";
  argv[6] = "-kernel";
  argv[7] = "compute_bound";
  argv[8] = "-iter";
  argv[9] = "4096";

  SerialApp app(argc, argv);
  app.execute_main_loop();

  printf("Execution finished.\n");
  return 0;
}
} // extern "C"
