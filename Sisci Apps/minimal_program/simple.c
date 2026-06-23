#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "sisci_types.h"
#include "sisci_api.h"
#include "sisci_error.h"


// Simplified error handling to keep the code clean
#define SISCI_ERROR_CHECK(func_name, error)                              \
if (error != SCI_ERR_OK) {                                               \
    fprintf(stderr, "%s failed - Error code: 0x%x\n", func_name, error); \
    fprintf(stderr, "   %s\n", SCIGetErrorString(error));                \
    return (int)error;                                                   \
}

#define NO_CALLBACK NULL
#define NO_OFFSET 0
#define NO_FLAGS 0

/* ID can be anything, but must be unique for the given node */
#define CLIENT_LOCAL_SEGMENT_ID 10 

int main(int argc, char *argv[]) {

    sci_error_t  sisci_error;
    size_t n_elements = 1024;
    size_t buffer_size = n_elements * sizeof(int);
    unsigned int adapter_no = 0;
    unsigned int remote_node_id = 0;
    bool is_server = false;

    /* Read arguments */
    for (int i = 1; i < argc; i++) {

        if (strcmp("-rn", argv[i]) == 0) {
            i++;
            remote_node_id = strtol(argv[i], NULL, 10);
        }

        if (strcmp("-server", argv[i]) == 0) {
            is_server = true;
        }

        if (strcmp("-a", argv[i]) == 0) {
            i++;
            adapter_no = strtol(argv[i], NULL, 10);
        }
    }

    if (remote_node_id == 0) {
        fprintf(stderr, "Usage: %s -rn <remote_node_id> -server/-client\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Initialize the SISCI library */
    SCIInitialize(NO_FLAGS, &sisci_error);
    SISCI_ERROR_CHECK("SCIInitialize", sisci_error)
    
    /* Open a sisci descriptor, needed to interface with the driver */
    sci_desc_t sd;
    SCIOpen(&sd, NO_FLAGS, &sisci_error);
    SISCI_ERROR_CHECK("SCIOpen", sisci_error);

    if (is_server) {
        /* Connect to segment on client, ID must match with SCICreateSegment on client.
         * This function fails when the segment does not exist on the other side,
         * so it is therefore called in a loop. */
        sci_remote_segment_t remote_segment;
        do {
            SCIConnectSegment(
                sd,
                &remote_segment,
                remote_node_id,
                CLIENT_LOCAL_SEGMENT_ID,
                adapter_no,
                NO_CALLBACK,
                NULL,
                SCI_INFINITE_TIMEOUT,
                NO_FLAGS,
                &sisci_error);
            usleep(1000);
        } while (sisci_error != SCI_ERR_OK);

        /* Map the segment's memory addresses to user space so we can access it */
        sci_map_t remote_map;
        volatile void *remote_ptr = SCIMapRemoteSegment(
            remote_segment,
            &remote_map,
            NO_OFFSET,
            buffer_size,
            NULL,
            NO_FLAGS,
            &sisci_error);
        SISCI_ERROR_CHECK("SCIMapRemoteSegment", sisci_error);

        /* Write some magic value to every element in the remote segment */
        volatile int *remote_int_ptr = (volatile int *)remote_ptr;
        for (size_t i = 0; i < n_elements; i++) {
            remote_int_ptr[i] = 42;
        }

    } else {
        /* Create a segment, ID must match with SCIConnectSegment on the server */
        sci_local_segment_t local_segment;
        SCICreateSegment(sd, &local_segment, CLIENT_LOCAL_SEGMENT_ID, buffer_size, NO_CALLBACK, NULL, NO_FLAGS, &sisci_error);
        SISCI_ERROR_CHECK("SCICreateSegment", sisci_error);

        SCIPrepareSegment(local_segment, adapter_no, NO_FLAGS, &sisci_error);
        SISCI_ERROR_CHECK("SCIPrepareSegment", sisci_error);

        /* Map the segment's memory area into user space so that we can access it */
        sci_map_t local_map;
        void *local_ptr = SCIMapLocalSegment(local_segment, &local_map, NO_OFFSET, buffer_size, NULL, NO_FLAGS, &sisci_error);
        SISCI_ERROR_CHECK("SCIMapLocalSegment", sisci_error);

        /* For example purposes: initialize the local memory to zero before we let the server send anything */
        int *local_int_ptr = (int *)local_ptr;
        for (size_t i = 0; i < n_elements; i++) {
            local_int_ptr[i] = 0;
        }

        /* Set the segment available for remote connections */
        SCISetSegmentAvailable(local_segment, adapter_no, NO_FLAGS, &sisci_error);
        SISCI_ERROR_CHECK("SCISetSegmentAvailable", sisci_error);

        while (local_int_ptr[n_elements-1] == 0) {
            usleep(1000);
        }

        for (int i = 0; i < 10; i++) {
            printf("%d ", local_int_ptr[i]);
        }
        printf("\n");
    }

    /* Free allocated resources */
    SCITerminate();
}
