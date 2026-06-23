/**
 * @file
 * @copyright
 *
 * Copyright (c) 2026 Dolphin Interconnect Solutions
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char boolean;
#define TRUE 1
#define FALSE 0

#include "sisci_api.h"
#include "sisci_error.h"
#include "sisci_types.h"

#define NO_FLAGS 0
#define NO_OFFSET 0
#define NO_CALLBACK     NULL
#define NO_CALLBACK_ARG NULL

#define MAX_SDS 4
#define MAX_LSEGS 10
#define MAX_RSEGS 10
#define MAX_SEGMAPS 30 
#define MAX_DMA_QUEUES 10
#define MAX_SEQUENCES 5
#define MAX_LOCAL_INTERRUPTS 10
#define MAX_REMOTE_INTERRUPTS 10

typedef enum {
    IDX_FLAG_EMPTY = 0,
    IDX_FLAG_PRIVATE,
    IDX_FLAG_BROADCAST,
    IDX_FLAG_ALLOW_UNICAST,
    IDX_FLAG_AUTO_ID,
    IDX_FLAG_BLOCK_READ,
    IDX_FLAG_ERROR_CHECK,
    
    IDX_FLAG_DMA_SOURCE_ONLY,
    IDX_FLAG_DMA_READ,
    IDX_FLAG_DMA_GLOBAL,
    IDX_FLAG_DMA_SYSDMA,

    IDX_FLAG_FIXED_INTNO,
    IDX_FLAG_SHARED_INT,
    IDX_FLAG_COUNTING_INT,

    IDX_NUM_FLAGS
} flag_indices;

char* idx_to_flagstr[IDX_NUM_FLAGS] = {
    "SCI_FLAG_EMPTY",
    "SCI_FLAG_PRIVATE",
    "SCI_FLAG_BROADCAST",
    "SCI_FLAG_ALLOW_UNICAST",
    "SCI_FLAG_AUTO_ID",
    "SCI_FLAG_BLOCK_READ",
    "SCI_FLAG_ERROR_CHECK",
    
    "SCI_FLAG_DMA_SOURCE_ONLY",
    "SCI_FLAG_DMA_READ",
    "SCI_FLAG_DMA_GLOBAL",
    "SCI_FLAG_DMA_SYSDMA",
    
    "SCI_FLAG_FIXED_INTNO",
    "SCI_FLAG_SHARED_INT",
    "SCI_FLAG_COUNTING_INT"
};
unsigned int idx_to_flag[IDX_NUM_FLAGS];

void init_idx_to_flag()
{
    idx_to_flag[IDX_FLAG_EMPTY] = SCI_FLAG_EMPTY;
    idx_to_flag[IDX_FLAG_PRIVATE] = SCI_FLAG_PRIVATE;
    idx_to_flag[IDX_FLAG_BROADCAST] = SCI_FLAG_BROADCAST;
    idx_to_flag[IDX_FLAG_ALLOW_UNICAST] = SCI_FLAG_ALLOW_UNICAST;
    idx_to_flag[IDX_FLAG_AUTO_ID] = SCI_FLAG_AUTO_ID;
    idx_to_flag[IDX_FLAG_BLOCK_READ] = SCI_FLAG_BLOCK_READ;
    idx_to_flag[IDX_FLAG_ERROR_CHECK] = SCI_FLAG_ERROR_CHECK;

    idx_to_flag[IDX_FLAG_DMA_SOURCE_ONLY] = SCI_FLAG_DMA_SOURCE_ONLY;
    idx_to_flag[IDX_FLAG_DMA_READ] = SCI_FLAG_DMA_READ;
    idx_to_flag[IDX_FLAG_DMA_GLOBAL] = SCI_FLAG_DMA_GLOBAL;
    idx_to_flag[IDX_FLAG_DMA_SYSDMA] = SCI_FLAG_DMA_SYSDMA;

    idx_to_flag[IDX_FLAG_FIXED_INTNO] = SCI_FLAG_FIXED_INTNO;
    idx_to_flag[IDX_FLAG_SHARED_INT] = SCI_FLAG_SHARED_INT;
    idx_to_flag[IDX_FLAG_COUNTING_INT] = SCI_FLAG_COUNTING_INT;
}

typedef struct {
    boolean used;
    sci_desc_t sd;
} sd_info_t;

typedef struct {
    boolean used;
    unsigned int id;
    unsigned int sd_index;
    boolean is_prepared;
    boolean is_set_available;
    boolean is_shared;
    boolean is_broadcast;
    size_t size;
    sci_local_segment_t seg;
} local_seg_info_t;

typedef struct {
    boolean used;
    unsigned int id;
    unsigned int sd_index;
    unsigned int node_id;
    boolean is_broadcast;
    sci_remote_segment_t seg;
} remote_seg_info_t;

typedef struct {
    boolean used;
    unsigned int seg_id;
    boolean is_remote;
    unsigned int remote_node_id;
    size_t offset;
    size_t size;
    volatile void *mem_ptr;
    sci_map_t map;
} map_info_t;

typedef struct {
    boolean used;
    unsigned int adapter_no;
    unsigned int sd_index;
    sci_dma_queue_t dq;
} dma_queue_info_t;

typedef struct {
    boolean used;
    unsigned int adapter_no;
    unsigned int sd_index;
    unsigned int interrupt_no;
    sci_local_interrupt_t interrupt;
} local_interrupt_info_t;

typedef struct {
    boolean used;
    unsigned int adapter_no;
    unsigned int sd_index;
    unsigned int interrupt_no;
    unsigned int node_id;
    sci_remote_interrupt_t interrupt;
} remote_interrupt_info_t;

typedef struct {
    boolean used;
    unsigned int map_index;
    sci_sequence_t seq;
} sequence_info_t;

sd_info_t sds[MAX_SDS];
local_seg_info_t lsegs[MAX_LSEGS];
remote_seg_info_t rsegs[MAX_RSEGS];
map_info_t seg_mappings[MAX_SEGMAPS];
dma_queue_info_t dma_queues[MAX_DMA_QUEUES];
local_interrupt_info_t local_interrupts[MAX_LOCAL_INTERRUPTS];
remote_interrupt_info_t remote_interrupts[MAX_REMOTE_INTERRUPTS];
sequence_info_t sequences[MAX_SEQUENCES];

unsigned int cnt_sds = 0;
unsigned int cnt_lsegs = 0;
unsigned int cnt_rsegs = 0;
unsigned int cnt_seg_mappings = 0;
unsigned int cnt_dma_queues = 0;
unsigned int cnt_local_interrupts = 0;
unsigned int cnt_remote_interrupts = 0;
unsigned int cnt_sequences = 0;

typedef enum {
    CMD_OPEN = 0,
    CMD_CLOSE,

    CMD_CREATE_SEG,
    CMD_REMOVE_SEG,
    CMD_PREPARE_SEG,
    CMD_SET_SEG_AVAIL,
    CMD_SET_SEG_UNAVAIL,
    CMD_CONNECT_SEG,
    CMD_DISCONNECT_SEG,

    CMD_MAP_LOCAL_SEG,
    CMD_MAP_REMOTE_SEG,
    CMD_UNMAP_SEG,
    
    CMD_MEMCPY,

    CMD_CREATE_DMA_QUEUE,
    CMD_REMOVE_DMA_QUEUE,
    CMD_QUERY_DMA_QUEUE_STATE,
    CMD_START_DMA_TRANSFER,
    CMD_WAIT_FOR_DMA_QUEUE,

    CMD_CREATE_INTERRUPT,
    CMD_REMOVE_INTERRUPT,
    CMD_CONNECT_INTERRUPT,
    CMD_DISCONNECT_INTERRUPT,
    CMD_TRIGGER_INTERRUPT,
    CMD_WAIT_FOR_INTERRUPT,

    CMD_CREATE_MAP_SEQUENCE,
    CMD_REMOVE_SEQUENCE,
    CMD_START_SEQUENCE,
    CMD_CHECK_SEQUENCE,

    /* Miscellaneous commands */
    CMD_QUERY,
    CMD_PROBE_NODE,
    CMD_GET_LOCAL_NODE_ID,
    CMD_SHARE_SEG,
    CMD_ATTACH_LOCAL_SEG,

    /* PIO Commands */
    CMD_PIO_READ,
    CMD_PIO_WRITE,
    CMD_PIO_COPY,

    CMD_HELP,
} command_type_t;

int max_x, max_y;

#define SISCI_ERROR_CHECK(func_name, error) \
    do { \
        if (error != SCI_ERR_OK) { \
            printf("%s failed - Error code: 0x%x\n", func_name, error); \
            printf("   %s\n", SCIGetErrorString(error)); \
            return -1; \
        } \
    } while (0)

#define SISCI_ERROR_CHECK2(func_name, error) \
    do { \
        if (error != SCI_ERR_OK) { \
            printf("%s failed - Error code: 0x%x\n", func_name, error); \
            printf("   %s\n", SCIGetErrorString(error)); \
            return -1; \
        } \
        printf("%s call succeeded.\n", func_name); \
    } while (0)


/******************************************************************************/
/****************** Printing commands, resource tables, etc *******************/
/******************************************************************************/

void print_all_flag_options()
{
    int i;
    printf("SISCI flags:\n");

    for (i = 0; i < IDX_NUM_FLAGS; i++) {
        printf("%3u : %s\n", i, idx_to_flagstr[i]);
    }
}

void print_flag_options_helper(unsigned int *flag_idx_arr, size_t flag_idx_arr_len)
{
    size_t i;
    printf("SISCI flags relevant for this function:\n");

    for (i = 0; i < flag_idx_arr_len; i++) {
        printf("%3u : %s\n", flag_idx_arr[i], idx_to_flagstr[flag_idx_arr[i]]);
    }
}

void print_flag_options(unsigned int *flag_idx_arr, size_t flag_idx_arr_len)
{
    print_all_flag_options();
    printf("\n");
    print_flag_options_helper(flag_idx_arr, flag_idx_arr_len);
}

char *Yes_or_No(boolean is_TRUE) {
    return is_TRUE ? "Yes" : "No";
}

void print_sds()
{
    unsigned int i = 0;

    printf("-- Open SciDesc index list: ");
    for (i = 0; i < MAX_SDS; i++) {
        if (sds[i].used) {
            printf("%d, ", i);
        }
    }
    printf("\n");
}

void print_lsegs()
{
    int i;
    printf("-- Local Segments -----------------------------------------------------------------------\n");
    printf("| %3s | %7s | %7s | %8s | %12s | %6s | %9s | %12s |\n",
        "Idx", "SegID", "SciDesc", "Prepared", "SetAvailable", "Shared", "Broadcast", "Size (Bytes)");
    printf("-----------------------------------------------------------------------------------------\n");

    for (i = 0; i < MAX_LSEGS; i++) {
        if (lsegs[i].used) {
            printf(
                "| %3d | %7u | %7u | %8s | %12s | %6s | %9s | %12lu |\n",
                i,
                lsegs[i].id,
                lsegs[i].sd_index,
                Yes_or_No(lsegs[i].is_prepared),
                Yes_or_No(lsegs[i].is_set_available),
                Yes_or_No(lsegs[i].is_shared),
                Yes_or_No(lsegs[i].is_broadcast),
                lsegs[i].size);
        }
#if 0
        else {
            printf(
                "| %3d | %7s | %7s | %8s | %12s | %6s | %9s | %12s |\n",
                    i, "-", "-", "-", "-", "-", "-", "-");
        }
#endif
    }
}

void print_rsegs()
{
    int i;
    printf("-- Remote Segments ------------------------------\n");
    printf("| %3s | %7s | %6s | %9s | %8s |\n", "Idx", "SegID", "NodeID", "Broadcast", "SciDesc");
    printf("-------------------------------------------------\n");

    for (i = 0; i < MAX_RSEGS; i++) {
        if (rsegs[i].used) {
            printf(
                "| %3d | %7u | %6u | %9s | %8u |\n",
                i, rsegs[i].id, rsegs[i].node_id, Yes_or_No(rsegs[i].is_broadcast), rsegs[i].sd_index);
        }
#if 0
        else {
            printf(
                "| %3d | %7s | %6s | %9s | %8s |\n", i, "-", "-", "-", "-");
        }
#endif
    }
}

void print_seg_mappings()
{
    int i;
    char node_id_str[8];

    printf("-- Segment Mappings --------------------------------------------------------------\n");
    printf("| %3s | %14s | %8s | %12s | %14s | %12s | \n", "Idx", "Maps seg w/ ID", "IsRemote", "RemoteNodeId", "Offset (Bytes)", "Size (Bytes)");
    printf("----------------------------------------------------------------------------------\n");

    for (i = 0; i < MAX_SEGMAPS; i++) {
        if (seg_mappings[i].used) {
            if (seg_mappings[i].is_remote) {
                sprintf(node_id_str, "%u", seg_mappings[i].remote_node_id);
            } else {
                node_id_str[0] = '-';
                node_id_str[1] = '\0';
            }
            printf("| %3d | %14u | %8s | %12s | %14lu | %12lu |\n",
                       i, seg_mappings[i].seg_id, Yes_or_No(seg_mappings[i].is_remote), node_id_str, seg_mappings[i].offset, seg_mappings[i].size);
        }
#if 0
        else {
            printf("| %3d | %14s | %8s | %12s | %14s | %12s |\n", i, "-", "-", "-", "-", "-");
        }
#endif
    }
}

void print_local_interrupts()
{
    int i;
    printf("-- Local Interrupts -----------------------\n");
    printf("| %3s | %9s | %7s | %11s |\n", "Idx", "AdapterNo", "SciDesc", "InterruptNo");
    printf("-------------------------------------------\n");

    for (i = 0; i < MAX_LOCAL_INTERRUPTS; i++) {
        if (local_interrupts[i].used) {
            printf(
                "| %3d | %9u | %7u | %11u |\n",
                i,
                local_interrupts[i].adapter_no,
                local_interrupts[i].sd_index,
                local_interrupts[i].interrupt_no);
        }
#if 0
        else {
            printf(
                "| %3d | %9s | %7s | %11s |\n", i, "-", "-", "-");
        }
#endif
    }
}

void print_remote_interrupts()
{
    int i;
    printf("-- Remote Interrupts -------------------------------\n");
    printf("| %3s | %9s | %7s | %11s | %6s |\n", "Idx", "AdapterNo", "SciDesc", "InterruptNo", "NodeID");
    printf("----------------------------------------------------\n");

    for (i = 0; i < MAX_REMOTE_INTERRUPTS; i++) {
        if (remote_interrupts[i].used) {
            printf(
                "| %3d | %9u | %7u | %11u | %6u |\n",
                i,
                remote_interrupts[i].adapter_no,
                remote_interrupts[i].sd_index,
                remote_interrupts[i].interrupt_no,
                remote_interrupts[i].node_id);
        }
#if 0
        else {
            printf(
                "| %3d | %9s | %7s | %11s | %6s |\n", i, "-", "-", "-", "-");
        }
#endif
    }
}


void print_dma_queues()
{
    int i;
    printf("-- DMA Queues ---------------\n");
    printf("| %3s | %9s | %7s |\n", "Idx", "AdapterNo", "SciDesc");
    printf("-----------------------------\n");

    for (i = 0; i < MAX_DMA_QUEUES; i++) {
        if (dma_queues[i].used) {
            printf(
                "| %3d | %9u | %7u |\n",
                i,
                dma_queues[i].adapter_no,
                dma_queues[i].sd_index);
        }
#if 0
        else {
            printf(
                "| %3d | %9s | %7s |\n", i, "-", "-");
        }
#endif
    }
}

void print_sequences()
{
    int i;
    printf(
        "-- Sequences --\n");
    printf(
        "| %4s | %4s |\n", "Idx", "Map");
    printf(
        "---------------\n");

    for (i = 0; i < MAX_SEQUENCES; i++) {
        if (sequences[i].used) {
            printf(
            "| %4d | %4u |\n", i, sequences[i].map_index);
        }
#if 0
        else {
            printf(
            "| %4d | %4s |\n", i, "-");
        }
#endif
    }
}

void print_resources()
{
    if (cnt_sds > 0) {
        print_sds();
        printf("\n");
    }
    
    if (cnt_lsegs > 0) {
        print_lsegs();
        printf("\n");
    }

    if (cnt_rsegs > 0) {
        print_rsegs();
        printf("\n");
    }

    if (cnt_seg_mappings > 0) {
        print_seg_mappings();
        printf("\n");
    }

    if (cnt_local_interrupts > 0) {
        print_local_interrupts();
        printf("\n");
    }

    if (cnt_remote_interrupts > 0) {
        print_remote_interrupts();
        printf("\n");
    }

    if (cnt_dma_queues > 0) {
        print_dma_queues();
        printf("\n");
    }

    if (cnt_sequences > 0) {
        print_sequences();
        printf("\n");
    }
}

void print_help() {
    printf("\n\n\n\n");
    printf("                Welcome to interactive SISCI\n\n");
    printf(" This program is meant both for new users getting used to the library\n"
           " and for experienced users testing specific functionality.\n\n");
    printf(" You can call a SISCI function and specify each relevant argument.\n\n");
    printf(" Handles to SISCI resources are managed by placing them into tables\n"
           " so that they can be accessed interactively by indices.\n"
           " (Note that the max number of each resource given here are arbitrarily\n"
           "  set constants that correspond to the size of these resource tables,\n"
           "  and NOT actual SISCI limitations)\n\n");
    printf(" To get started, try to set up a segment and send data between two machines.\n\n");
    printf(" Example ordering of commands:\n\n");
    printf("  * Node1: open, create_seg, prepare_seg, set_seg_avail, map_local_seg, pio_write\n");
    printf("  * Node2: open, connect_seg, map_remote_seg, pio_read, pio_write\n\n");
    printf("\n\n");
}

void print_commands()
{

    printf( "Type 'quit' or 'exit' to exit the program, or type 'help' for a short introduction.\n");
    printf( "- Most Common SISCI commands:\n");

    printf(
        "%-20s : %s\n",
        "open",
        "SCIOpen(sci_desc_t*, ...)");

    printf(
        "%-20s : %s\n",
        "close",
        "SCIClose(sci_desc_t, ...)");

    printf(
        "%-20s : %s\n",
        "create_seg",
        "SCICreateSegment(..., sci_local_segment_t*, ...)");
    
    printf(
        "%-20s : %s\n",
        "remove_seg",
        "SCIRemoveSegment(sci_local_segment_t, ...)");

    printf(
        "%-20s : %s\n",
        "prepare_seg",
        "SCIPrepareSegment(sci_local_segment_t, ...)");

    printf(
        "%-20s : %s\n",
        "set_seg_avail",
        "SCISetSegmentAvailable(sci_local_segment_t, ...)");

    printf(
        "%-20s : %s\n",
        "set_seg_unavail",
        "SCISetSegmentUnavailable(sci_local_segment_t, ...)");

    printf(
        "%-20s : %s\n",
        "connect_seg",
        "SCIConnectSegment(..., sci_remote_segment_t*, ...)");

    printf(
        "%-20s : %s\n",
        "disconnect_seg",
        "SCIDisconnectSegment(sci_remote_segment_t, ...)");

    printf(
        "%-20s : %s\n",
        "map_local_seg",
        "SCIMapLocalSegment(..., sci_map_t*, ...): void*");

    printf(
        "%-20s : %s\n",
        "map_remote_seg",
        "SCIMapRemoteSegment(sci_local_segment_t, ...)");

    printf(
        "%-20s : %s\n",
        "unmap_seg",
        "SCIUnmapSegment(sci_map_t, ...)");

    printf(
        "%-20s : %s\n",
        "memcpy",
        "SCIMemCpy(..., sci_map_t, ...)");

    printf("\n");

    printf("- SISCI DMA Commands:\n");
    printf("    create_dma_queue, remove_dma_queue, query_dma_queue_state,\n");
    printf("    start_dma_transfer, wait_for_dma_queue\n");
    printf("- SISCI Interrupt Commands:\n");
    printf("    create_interrupt, remove_interrupt, connect_interrupt, disconnect_interrupt,\n");
    printf("    trigger_interrupt, wait_for_interrupt\n");
    printf("- SISCI Sequence Commands:\n");
    printf("    create_map_sequence, remove_sequence, start_sequence, check_sequence\n");
    printf("- SISCI Miscellaneous Commands:\n");
    printf("    probe_node, query, get_local_node_id, \n");
    printf("    share_seg, attach_local_seg\n");
    printf("- Other commands (Miscellaneous primitives. Not analogous to sisci commands):\n");
    printf("    pio_read, pio_write, pio_copy\n");
}


/******************************************************************************/
/**************** Helper functions for parsing input from user ****************/
/******************************************************************************/

/* Function to skip leading whitespace */
void skip_whitespace(char** str)
{
    while (isspace(**str)) {
        (*str)++;
    }
}

/* Function to extract the next part of the string
 * Watch out, after going to the end of the substring this function will set the
 * last character to the null character and assume that is the end of the
 * string. The string it handles should therefore have two nullstrings, or a
 * nullstring after a non-character, if one wants to use this function to parse
 * an unknown number of substrings.
 */
char* get_next_substring(char** str)
{
    skip_whitespace(str);
    char* start = *str;

    while (**str != '\0' && !isspace(**str)) {
        (*str)++;
    }

    /* Null-terminate the extracted string */
    if (*str != start) {
        **str = '\0';
        (*str)++;
    } else {
        start = NULL;
    }

    return start;
}

int get_command(command_type_t* cmd_type)
{
    char* next_s;
    char* str_parse_ptr;
    static char cmd_buf[1024];

    memset(cmd_buf, '\0', 1024);
    if(fgets(cmd_buf, 1023, stdin)) {}

    str_parse_ptr = &cmd_buf[0];
    next_s = get_next_substring(&str_parse_ptr);
    if (next_s == NULL) {
        return -1;
    }

    /*** Typical SISCI Commands ***/
    if (strncmp(next_s, "open", strlen("open")) == 0) {
        *cmd_type = CMD_OPEN;
    } else if (strncmp(next_s, "close", strlen("close")) == 0) {
        *cmd_type = CMD_CLOSE;
    } else if (strncmp(next_s, "create_seg", strlen("create_seg")) == 0) {
        *cmd_type = CMD_CREATE_SEG;
    } else if (strncmp(next_s, "remove_seg", strlen("remove_seg")) == 0) {
        *cmd_type = CMD_REMOVE_SEG;
    } else if (strncmp(next_s, "prepare_seg", strlen("prepare_seg")) == 0) {
        *cmd_type = CMD_PREPARE_SEG;
    } else if (strncmp(next_s, "set_seg_avail", strlen("set_seg_avail")) == 0) {
        *cmd_type = CMD_SET_SEG_AVAIL;
    } else if (strncmp(next_s, "set_seg_unavail", strlen("set_seg_unavail")) == 0) {
        *cmd_type = CMD_SET_SEG_UNAVAIL;
    } else if (strncmp(next_s, "connect_seg", strlen("connect_seg")) == 0) {
        *cmd_type = CMD_CONNECT_SEG;
    } else if (strncmp(next_s, "disconnect_seg", strlen("disconnect_seg")) == 0) {
        *cmd_type = CMD_DISCONNECT_SEG;
    } else if (strncmp(next_s, "map_local_seg", strlen("map_local_seg")) == 0) {
        *cmd_type = CMD_MAP_LOCAL_SEG;
    } else if (strncmp(next_s, "map_remote_seg", strlen("map_remote_seg")) == 0) {
        *cmd_type = CMD_MAP_REMOTE_SEG;
    } else if (strncmp(next_s, "unmap_seg", strlen("unmap_seg")) == 0) {
        *cmd_type = CMD_UNMAP_SEG;
    } else if (strncmp(next_s, "memcpy", strlen("memcpy")) == 0) {
        *cmd_type = CMD_MEMCPY;
    }
    
    /*** DMA SISCI Commands ***/
    else if (strncmp(next_s, "create_dma_queue", strlen("create_dma_queue")) == 0) {
        *cmd_type = CMD_CREATE_DMA_QUEUE;
    } else if (strncmp(next_s, "remove_dma_queue", strlen("remove_dma_queue")) == 0) {
        *cmd_type = CMD_REMOVE_DMA_QUEUE;
    } else if (strncmp(next_s, "query_dma_queue_state", strlen("query_dma_queue_state")) == 0) {
        *cmd_type = CMD_QUERY_DMA_QUEUE_STATE;
    } else if (strncmp(next_s, "start_dma_transfer", strlen("start_dma_transfer")) == 0) {
        *cmd_type = CMD_START_DMA_TRANSFER;
    } else if (strncmp(next_s, "wait_for_dma_queue", strlen("wait_for_dma_queue")) == 0) {
        *cmd_type = CMD_WAIT_FOR_DMA_QUEUE;
    }

    /*** Interrupt SISCI Commands ***/
    else if (strncmp(next_s, "create_interrupt", strlen("create_interrupt")) == 0) {
        *cmd_type = CMD_CREATE_INTERRUPT;
    } else if (strncmp(next_s, "remove_interrupt", strlen("remove_interrupt")) == 0) {
        *cmd_type = CMD_REMOVE_INTERRUPT;
    } else if (strncmp(next_s, "connect_interrupt", strlen("connect_interrupt")) == 0) {
        *cmd_type = CMD_CONNECT_INTERRUPT;
    } else if (strncmp(next_s, "disconnect_interrupt", strlen("disconnect_interrupt")) == 0) {
        *cmd_type = CMD_DISCONNECT_INTERRUPT;
    } else if (strncmp(next_s, "trigger_interrupt", strlen("trigger_interrupt")) == 0) {
        *cmd_type = CMD_TRIGGER_INTERRUPT;
    } else if (strncmp(next_s, "wait_for_interrupt", strlen("wait_for_interrupt")) == 0) {
        *cmd_type = CMD_WAIT_FOR_INTERRUPT;
    }

    /*** Sequence SISCI Commands ***/
    else if (strncmp(next_s, "create_map_sequence", strlen("create_map_sequence")) == 0) {
        *cmd_type = CMD_CREATE_MAP_SEQUENCE;
    } else if (strncmp(next_s, "remove_sequence", strlen("remove_sequence")) == 0) {
        *cmd_type = CMD_REMOVE_SEQUENCE;
    } else if (strncmp(next_s, "start_sequence", strlen("start_sequence")) == 0) {
        *cmd_type = CMD_START_SEQUENCE;
    } else if (strncmp(next_s, "check_sequence", strlen("check_sequence")) == 0) {
        *cmd_type = CMD_CHECK_SEQUENCE;
    }

    /*** Miscellaneous SISCI Commands ***/
    else if (strncmp(next_s, "query", strlen("query")) == 0) {
    *cmd_type = CMD_QUERY;
    } else if (strncmp(next_s, "probe_node", strlen("probe_node")) == 0) {
        *cmd_type = CMD_PROBE_NODE;
    } else if (strncmp(next_s, "get_local_node_id", strlen("get_local_node_id")) == 0) {
        *cmd_type = CMD_GET_LOCAL_NODE_ID;
    } else if (strncmp(next_s, "share_seg", strlen("share_seg")) == 0) {
        *cmd_type = CMD_SHARE_SEG;
    } else if (strncmp(next_s, "attach_local_seg", strlen("attach_local_seg")) == 0) {
        *cmd_type = CMD_ATTACH_LOCAL_SEG;
    }

    /*** Extra Commands ***/
    else if (strncmp(next_s, "pio_read", strlen("pio_read")) == 0) {
        *cmd_type = CMD_PIO_READ;
    } else if (strncmp(next_s, "pio_write", strlen("pio_write")) == 0) {
        *cmd_type = CMD_PIO_WRITE;
    } else if (strncmp(next_s, "pio_copy", strlen("pio_copy")) == 0) {
        *cmd_type = CMD_PIO_COPY;
    }

    else if (strncmp(next_s, "help", strlen("help")) == 0) {
        *cmd_type = CMD_HELP;
    }
    
    else if ((strncmp(next_s, "quit", strlen("quit")) == 0)
        || (strncmp(next_s, "exit", strlen("exit")) == 0)) {
        return 1;
    } else {
        return -1;
    }

    return 0;
}

unsigned long get_ulong_input()
{
    unsigned long num;
    static char buf[32];
    char* str_parse_ptr;
    char* str;
    while (TRUE) {
        memset(buf, '\0', 32);
        if(fgets(buf, 31, stdin)) {}
        str_parse_ptr = &buf[0];
        str = get_next_substring(&str_parse_ptr);
        if (str == NULL) {
            continue;
        }
        char *endptr;
        num = strtoul(str, &endptr, 10);
        if (*endptr == '\0') {
            return num;
        }
    }
}

unsigned int get_flags()
{
    int flag_index;
    static char buf[1024];
    char* str_parse_ptr;
    char* str;
    unsigned int flags = 0;
    
    memset(buf, '\0', 1024);
    if(fgets(buf, 1023, stdin)) {}

    printf("Using flags: ");

    str_parse_ptr = &buf[0];
    while (TRUE) {
        str = get_next_substring(&str_parse_ptr);
        if (str == NULL) {
            break;
        }
        flag_index = strtoul(str, NULL, 10);
        if (flag_index < IDX_NUM_FLAGS) {
            flags |= idx_to_flag[flag_index];
            printf("%s, ", idx_to_flagstr[flag_index]);
        }
    }

    if (flags == 0) {
        printf("None");
    }

    printf("\n");

    return flags;
}

boolean check_yes_input() {

    static char buf[16];
    char* str_parse_ptr;
    char* str;
    
    memset(buf, '\0', 16);
    if(fgets(buf, 15, stdin)) {}

    str_parse_ptr = buf;
    str = get_next_substring(&str_parse_ptr);
    if (str != NULL && (str[0] == 'Y' || str[0] == 'y')) {
        return TRUE;
    } else {
        return FALSE;
    }
}


/******************************************************************************/
/*********** Functions handling the highlighted "typical" commands ************/
/****************** managing SISCI descriptors and segments *******************/
/******************************************************************************/

int handle_open()
{
    sci_error_t error;
    int sd_index;
    printf("void SCIOpen(sci_desc_t *sd, unsigned int flags, sci_error_t *error)\n");

    printf("\nArg 1, Sisci descriptor to open.\n");
    printf("Enter scidesc index (MAX=%u): ", MAX_SDS - 1);

    sd_index = get_ulong_input();
    if (sd_index >= MAX_SDS) {
        printf("sci_desc index not in valid range [0, %d]\n", MAX_SDS - 1);
        printf("Aborting this command\n");
        return -1;
    }

    /* Or perhaps try anyways to provoke error message? */
    if (sds[sd_index].used) {
        printf("Already an open sci_desc at index %d\n", sd_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Arg 2, Flags: (not used)\n");

    if (sd_index < 0 || sd_index >= MAX_SDS) {
        printf("Invalid sd_index %u, must be in range [%d, %d]\n", sd_index, 0, MAX_SDS - 1);
        return -1;
    }

    SCIOpen(&sds[sd_index].sd, NO_FLAGS, &error);
    if (error != SCI_ERR_OK) {
        printf("SCIOpen failed, error code 0x%x\n", error);
        printf("   %s\n", SCIGetErrorString(error));
        return -1;
    }

    printf("SCIOpen call succeeded\n");

    sds[sd_index].used = TRUE;
    cnt_sds++;

    return 0;
}

int handle_closed()
{
    sci_error_t error;
    unsigned int sd_index;
    printf("void SCIClose(sci_desc_t sd, unsigned int flags, sci_error_t *error)\n");

    printf("\nArg 1, Sisci descriptor to close.\n");
    printf("Enter scidesc index: ");

    sd_index = get_ulong_input();
    if (sd_index >= MAX_SDS) {
        printf("sci_desc index not in valid range [0, %d]\n", MAX_SDS - 1);
        printf("Aborting this command\n");
        return -1;
    }

    if (!sds[sd_index].used) {
        printf("No open sci_desc at index %d\n", sd_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Arg 2, Flags: (not used)\n");

    SCIClose(sds[sd_index].sd, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCIClose", error);

    sds[sd_index].used = FALSE;
    cnt_sds--;

    return 0;
}

int handle_create_seg()
{
    sci_error_t error;
    unsigned int sd_index, seg_index, seg_id, flags;
    size_t seg_size;

    printf("void SCICreateSegment(sci_desc_t sd, sci_local_segment_t *segment, unsigned int segmentId,\n");
    printf("                      size_t size, sci_cb_local_segment_t callback, void *callbackArg,\n");
    printf("                      unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, An open sisci descriptor.\n");
    printf("Enter scidesc index: ");
    sd_index = get_ulong_input();
    if (sd_index >= MAX_SDS) {
        printf("sci_desc index not in valid range [0, %d]\n", MAX_SDS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!sds[sd_index].used) {
        printf("No open sci_desc at index %d\n", sd_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, An unused local segment index to associate with new segment.\n");
    printf("Enter local segment index (MAX=%u): ", MAX_LSEGS - 1);
    
    seg_index = get_ulong_input();
    if (seg_index >= MAX_LSEGS) {
        printf("Local seg index not in valid range [0, %d]\n", MAX_LSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (lsegs[seg_index].used) {
        printf("Local seg index %u already in use.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 3, A segment ID for the new segment.\n");
    printf("Enter segment ID: ");
    seg_id = get_ulong_input();

    printf("\nArg 4, Desired segment size in bytes.\n");
    printf("Enter segment size: ");
    seg_size = (size_t)get_ulong_input();

    printf("\nArg 5, callback, not possible in interactive mode.\n");
    printf("\nArg 6, callback argument, not relevant without callback.\n");

    printf("\n");
    {
        unsigned int flag_idx_arr[] = {
            IDX_FLAG_EMPTY,
            IDX_FLAG_PRIVATE,
            IDX_FLAG_BROADCAST,
            IDX_FLAG_ALLOW_UNICAST,
            IDX_FLAG_AUTO_ID,
            IDX_FLAG_DMA_GLOBAL
        };
        print_flag_options(flag_idx_arr, sizeof(flag_idx_arr) / sizeof(unsigned int));
    }
    printf("\nArg 7, Choose flags by indices shown above (SPACE SEPARATED. For no flags, press enter).\n");
    printf("Enter flag indices: ");
    flags = get_flags();

    SCICreateSegment(
        sds[sd_index].sd,
        &lsegs[seg_index].seg,
        seg_id,
        seg_size,
        NO_CALLBACK,
        NO_CALLBACK_ARG,
        flags,
        &error);
    SISCI_ERROR_CHECK2("SCICreateSegment", error);

    if ((flags & SCI_FLAG_AUTO_ID) > 0) {
        printf("Since SCI_FLAG_AUTO_ID flag was used SCIGetLocalSegmentId is called as well.\n");
        seg_id = SCIGetLocalSegmentId(lsegs[seg_index].seg);
        printf("SCIGetLocalSegmentId called successfully. Segment ID = %u\n", seg_id);
    }

    lsegs[seg_index].used = TRUE;
    lsegs[seg_index].id = seg_id;
    lsegs[seg_index].is_prepared = FALSE;
    lsegs[seg_index].is_set_available = FALSE;
    lsegs[seg_index].is_shared = FALSE;
    lsegs[seg_index].size = seg_size;

    if ((flags & SCI_FLAG_BROADCAST) != 0) {
        lsegs[seg_index].is_broadcast = TRUE;
    } else {
        lsegs[seg_index].is_broadcast = FALSE;
    }
    cnt_lsegs++;

    return 0;
}

int handle_remove_seg()
{
    sci_error_t error;
    unsigned int seg_index;

    printf("void SCIRemoveSegment(sci_local_segment_t segment, unsigned int flags, sci_error_t *error);\n");
    
    printf("\nArg 1, A previously created segment to remove.\n");
    printf("Enter local segment index: ");
    seg_index = get_ulong_input();
    if (seg_index >= MAX_LSEGS) {
        printf("Local seg index not in valid range [0, %d]\n", MAX_LSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!lsegs[seg_index].used) {
        printf("Local seg index %u is not in use.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, Skipping flag argument. Only force remove flag possibly, but it is not intended for general use.\n");

    SCIRemoveSegment(lsegs[seg_index].seg, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCIRemoveSegment", error);

    lsegs[seg_index].used = FALSE;
    cnt_lsegs--;

    return 0;
}

int handle_prepare_seg()
{
    sci_error_t error;
    unsigned int seg_index;
    unsigned int local_adapter_no;
    unsigned int flags;

    printf("void SCIPrepareSegment(sci_local_segment_t segment, unsigned int localAdapterNo,\n");
    printf("                       unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A previously created segment to prepare.\n");
    printf("Enter local segment index: ");
    seg_index = get_ulong_input();
    if (seg_index >= MAX_LSEGS) {
        printf("Local seg index not in valid range [0, %d]\n", MAX_LSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!lsegs[seg_index].used) {
        printf("Local seg index %u not created.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }
    if (lsegs[seg_index].is_prepared) {
        printf("Warn: Selected segment has already been prepared... Still allowing this action for testing purposes.\n");
    }

    printf("\nArg 2, The local adapter number used for NTB (typically 0, but consult dis_diag)\n");
    printf("Enter local adapter number: ");
    local_adapter_no = get_ulong_input();

    {
        unsigned int flag_idx_arr[] = {
            IDX_FLAG_DMA_SOURCE_ONLY
        };
        print_flag_options(flag_idx_arr, sizeof(flag_idx_arr) / sizeof(unsigned int));
    }
    printf("\nArg 3, Choose flags by indices shown above (SPACE SEPARATED. For no flags, press enter).\n");
    printf("Enter flag indices: ");
    flags = get_flags();

    SCIPrepareSegment(lsegs[seg_index].seg, local_adapter_no, flags, &error);
    SISCI_ERROR_CHECK2("SCIPrepareSegment", error);

    lsegs[seg_index].is_prepared = TRUE;

    return 0;
}

int handle_set_seg_available()
{
    sci_error_t error;
    unsigned int seg_index;
    unsigned int local_adapter_no;

    printf("void SCISetSegmentAvailable(sci_local_segment_t segment, unsigned int localAdapterNo,\n");
    printf("                            unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A created and prepared segment to set available.\n");
    printf("Enter local segment index: ");
    seg_index = get_ulong_input();
    if (seg_index >= MAX_LSEGS) {
        printf("Local seg index not in valid range [0, %d]\n", MAX_LSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!lsegs[seg_index].used) {
        printf("Local seg index %u not created.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }
    if (lsegs[seg_index].is_set_available) {
        printf("Warn: Selected segment is already set available... Still allowing this action for testing purposes.\n");
    }

    printf("\nArg 2, The local adapter number used for NTB (typically 0, but consult dis_diag)\n");
    printf("Enter local adapter number: ");
    local_adapter_no = get_ulong_input();

    printf("Arg 3, Flags: (not used)\n");

    SCISetSegmentAvailable(lsegs[seg_index].seg, local_adapter_no, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCISetSegmentAvailable", error);

    lsegs[seg_index].is_set_available = TRUE;

    return 0;
}

int handle_set_seg_unavailable()
{
    sci_error_t error;
    unsigned int seg_index;
    unsigned int local_adapter_no;

    printf("void SCISetSegmentUnavailable(sci_local_segment_t segment, unsigned int localAdapterNo,\n");
    printf("                              unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A previously set available local segment:\n");
    printf("Enter local segment index: ");
    seg_index = get_ulong_input();
    if (seg_index >= MAX_LSEGS) {
        printf("Local seg index not in valid range [0, %d]\n", MAX_LSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!lsegs[seg_index].used) {
        printf("Local seg index %u not created.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }
    if (!lsegs[seg_index].is_set_available) {
        printf("Warn: Selected segment is NOT currently set available... Still allowing this action for testing purposes.\n");
    }

    printf("\nArg 2, The local adapter number used for NTB (typically 0, but consult dis_diag)\n");
    printf("Enter local adapter number: ");
    local_adapter_no = get_ulong_input();

    printf("Arg 3, Flags: (not used)\n");

    SCISetSegmentUnavailable(lsegs[seg_index].seg, local_adapter_no, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCISetSegmentUnavailable", error);

    lsegs[seg_index].is_set_available = TRUE;

    return 0;
}


int handle_connect_seg()
{
    sci_error_t error;
    unsigned int sd_index, seg_id, seg_index, remote_node_id, local_adapter_no, timeout, flags; 
    boolean yes;
    
    printf("void SCIConnectSegment(sci_desc_t sd, sci_remote_segment_t segment, unsigned int nodeId,\n");
    printf("                       unsigned int segmentId, unsigned int localAdapterNo,\n");
    printf("                       sci_cb_remote_segment callback, void *callbackArg,\n");
    printf("                       unsigned int timeout, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, An open sisci descriptor.\n");
    printf("Enter scidesc index: ");
    sd_index = get_ulong_input();
    if (sd_index >= MAX_SDS) {
        printf("sci_desc index not in valid range [0, %d]\n", MAX_SDS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!sds[sd_index].used) {
        printf("No open sci_desc at index %d\n", sd_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, Unused remote segment handle to associate with new segment.\n");
    printf("Enter remote segment index (MAX=%u): ", MAX_RSEGS - 1);
    seg_index = get_ulong_input();
    if (seg_index >= MAX_RSEGS) {
        printf("Remote seg index not in valid range [0, %d]\n", MAX_RSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (rsegs[seg_index].used) {
        printf("Remote seg index %u already in use.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 3, Depends on flag. Will ask later.\n");

    printf("\nArg 4, Segment ID of remote segment on the aforementioned remote node.\n");
    printf("Enter remote segment ID: ");
    seg_id = get_ulong_input();
    
    printf("\nArg 5, The local adapter number used for NTB (typically 0, but consult dis_diag)\n");
    printf("Enter local adapter number: ");
    local_adapter_no = get_ulong_input();

    printf("Arg 6, Callback. Not possible in interactive mode.\n");
    printf("Arg 7, Callback argument. Not relevant without callback.\n");

    printf("\nArg 8, Time in milliseconds to wait for the connection to complete.\n");
    printf("(Note: Despite the use of SCI_INFINITE_TIMEOUT, know that this function call doesn't currently block.)\n");
    printf("Timeout is not implemented and should always be set to SCI_INFINITE_TIMEOUT.\n");
    printf("Do you want to test setting it to something else? [y/N] ");
    yes = check_yes_input();
    if (yes) {
        printf("Enter the desired timeout parameter in milliseconds: ");
        timeout = (size_t)get_ulong_input();
    } else {
        printf("Setting timeout to SCI_INFINITE_TIMEOUT as per documentation.\n");
        timeout = SCI_INFINITE_TIMEOUT;
    }

    {
        unsigned int flag_idx_arr[] = {
            IDX_FLAG_BROADCAST
        };
        print_flag_options(flag_idx_arr, sizeof(flag_idx_arr) / sizeof(unsigned int));
    }
    printf("Arg 9, Choose flags by indices shown above (SPACE SEPARATED. For no flags, press enter).\n");
    printf("Enter flag indices: ");
    flags = get_flags();

    if (flags & SCI_FLAG_BROADCAST) {
        printf("\nSetting node_id to DIS_BROADCAST_NODEID_GROUP_ALL\n");
        remote_node_id = DIS_BROADCAST_NODEID_GROUP_ALL;
    } else {
        printf("\nArg 3, Remote node ID (Whatever you have configured. Consult dis_diag on other machine)\n");
        printf("Enter remote nodeID: ");
        remote_node_id = get_ulong_input();
    }

    SCIConnectSegment(
        sds[sd_index].sd,
        &rsegs[seg_index].seg,
        remote_node_id,
        seg_id,
        local_adapter_no,
        NO_CALLBACK,
        NO_CALLBACK_ARG,
        timeout,
        flags,
        &error);
    SISCI_ERROR_CHECK2("SCIConnectSegment", error);

    rsegs[seg_index].used = TRUE;
    rsegs[seg_index].id = seg_id;
    rsegs[seg_index].sd_index = sd_index;
    rsegs[seg_index].node_id = remote_node_id;

    if ((flags & SCI_FLAG_BROADCAST) != 0) {
        rsegs[seg_index].is_broadcast = TRUE;
    } else {
        rsegs[seg_index].is_broadcast = FALSE;
    }

    cnt_rsegs++;

    return 0;
}

int handle_disconnect_seg()
{
    sci_error_t error;
    unsigned int seg_index;

    printf("void SCIDisconnectSegment(sci_remote_segment_t segment, unsigned int flags,\n");
    printf("                          sci_error_t *error);\n");

    printf("\nArg 1, A connected remote segment to disconnect.\n");
    printf("Enter remote segment index: ");
    seg_index = get_ulong_input();
    if (seg_index >= MAX_RSEGS) {
        printf("Remote seg index not in valid range [0, %d]\n", MAX_RSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!rsegs[seg_index].used) {
        printf("Remote seg index %u is not in use.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Arg 2, Flags: (not used)\n");

    SCIDisconnectSegment(
        rsegs[seg_index].seg,
        NO_FLAGS,
        &error);
    SISCI_ERROR_CHECK2("SCIDisconnectSegment", error);
    rsegs[seg_index].used = FALSE;
    cnt_rsegs--;

    return 0;
}

int handle_map_local_seg()
{
    sci_error_t error;
    unsigned int seg_index;
    unsigned int map_index;
    size_t offset;
    size_t size;

    printf("void *SCIMapLocalSegment(sci_local_segment_t segment, sci_map_t *map, size_t offset,\n");
    printf("                         size_t size, void *addr, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A created and prepared local segment to map.\n");
    printf("Enter local segment index: ");
    seg_index = get_ulong_input();
    if (seg_index >= MAX_LSEGS) {
        printf("Local seg index not in valid range [0, %d]\n", MAX_LSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!lsegs[seg_index].used) {
        printf("Local seg index %u not created.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, An unused map index to associate with new mapping.\n");
    printf("Enter map index (MAX=%u): ", MAX_SEGMAPS - 1);
    map_index = get_ulong_input();
    if (map_index >= MAX_SEGMAPS) {
        printf("Map index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (seg_mappings[map_index].used) {
        printf("Map index %u already in use.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 3, A map offset from segment start in bytes.\n");
    printf("Enter map offset: ");
    offset = get_ulong_input();

    printf("\nArg 4, Number of bytes to map of segment in bytes.\n");
    printf("Enter map size: ");
    size = get_ulong_input();

    printf("Arg 5, Flags. (Skipping this argument. Relevant flags not useful in interactive mode):\n");

    seg_mappings[map_index].mem_ptr = SCIMapLocalSegment(
        lsegs[seg_index].seg,
        &seg_mappings[map_index].map,
        offset,
        size,
        NULL,
        NO_FLAGS,
        &error);
    SISCI_ERROR_CHECK2("SCIMapLocalSegment", error);

    seg_mappings[map_index].seg_id = lsegs[seg_index].id;
    seg_mappings[map_index].size = size;
    seg_mappings[map_index].offset = offset;
    seg_mappings[map_index].is_remote = FALSE;
    seg_mappings[map_index].used = TRUE;
    cnt_seg_mappings++;

    return 0;
}

int handle_map_remote_seg()
{
    sci_error_t error;
    unsigned int seg_index;
    unsigned int map_index;
    size_t offset;
    size_t size;

    printf("volatile void *SCIMapRemoteSegment(sci_remote_segment_t segment, sci_map_t *map, size_t offset,\n");
    printf("                          size_t size, void *addr, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A connected remote segment to map.\n");
    printf("Enter remote segment index: ");
    seg_index = get_ulong_input();
    if (seg_index >= MAX_RSEGS) {
        printf("Map index not in valid range [0, %d]\n", MAX_RSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!rsegs[seg_index].used) {
        printf("Map index %u not created.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, An unused map index to associate with new mapping.\n");
    printf("Enter map index (MAX=%u): ", MAX_SEGMAPS - 1);
    map_index = get_ulong_input();
    if (map_index >= MAX_SEGMAPS) {
        printf("Map index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (seg_mappings[map_index].used) {
        printf("Map index %u already in use.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 3, A map offset from segment start in bytes.\n");
    printf("Enter map offset: ");
    offset = get_ulong_input();

    printf("\nArg 4, Number of bytes to map of segment in bytes.\n");
    printf("Enter map size: ");
    size = get_ulong_input();

    printf("Arg 5, Flags. (Skipping this argument. Relevant flags not useful in interactive mode):\n");

    seg_mappings[map_index].mem_ptr = (volatile void *) SCIMapRemoteSegment(
        rsegs[seg_index].seg,
        &seg_mappings[map_index].map,
        offset,
        size,
        NULL,
        NO_FLAGS,
        &error);
    SISCI_ERROR_CHECK2("SCIMapRemoteSegment", error);

    seg_mappings[map_index].seg_id = rsegs[seg_index].id;
    seg_mappings[map_index].size = size;
    seg_mappings[map_index].offset = offset;
    seg_mappings[map_index].is_remote = TRUE;
    seg_mappings[map_index].remote_node_id = rsegs[seg_index].node_id;
    seg_mappings[map_index].used = TRUE;
    cnt_seg_mappings++;

    return 0;
}

int handle_unmap_seg()
{
    sci_error_t error;
    unsigned int map_index;

    printf("void *SCIUnmapSegment(sci_map_t map, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A used map handle associated with existing mapping to unmap.\n");
    printf("Enter map index: ");
    map_index = get_ulong_input();
    if (map_index >= MAX_SEGMAPS) {
        printf("Map index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!seg_mappings[map_index].used) {
        printf("No mapping set up for map index %u. Nothing to unmap.\n", map_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Arg 2, Flags: (not used)\n");

    SCIUnmapSegment(seg_mappings[map_index].map, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCIUnmapSegment", error);

    seg_mappings[map_index].used = FALSE;
    cnt_seg_mappings--;

    return 0;
}

int handle_memcpy()
{
    sci_error_t error;
    boolean yes;
    sci_sequence_t sequence;
    unsigned int local_map_index;
    size_t local_offset;
    size_t local_buffer_subsize;
    unsigned int remote_map_index;
    size_t remote_offset;
    size_t remote_buffer_subsize;
    size_t transfer_size;
    unsigned int flags;

    printf("void SCIMemCpy(sci_sequence_t sequence, void *memAddr, sci_map_t remoteMap,\n");
    printf("               size_t remoteOffset, size_t size, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, sequence descriptor. Setting up a sequence descriptor is optional.\n");
    printf("Using sequence descriptors allows for additional error checking for data transfers.\n");
    printf("Do you want to use a specific sequence? [y/N] ");
    yes = check_yes_input();
    if (yes) {
        unsigned int sequence_index;
        printf("Enter a sequence index: ");
        sequence_index = get_ulong_input();
        if (!sequences[sequence_index].used) {
            printf("No sequence set up for sequence index %u.\n", sequence_index);
            printf("Aborting this command\n");
            return -1;
        }
        sequence = sequences[sequence_index].seq;
    } else {
        printf("Using no sequence (i.e. sequence = NULL).\n");
        sequence = NULL;
    }

    printf("\nArg 2, local memAddr (since this is interactive I'll ask you for a mapped address and a byte offset).\n");
    printf("You probably want to choose a map index that is local to your machine, but it's up to you.\n");

    printf("Enter a map index: ");
    local_map_index = get_ulong_input();
    if (local_map_index >= MAX_SEGMAPS) {
        printf("Map index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!seg_mappings[local_map_index].used) {
        printf("No mapping set up for map index %u.\n", local_map_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Enter buffer offset in bytes: ");
    local_offset = get_ulong_input();
    if (local_offset >= seg_mappings[local_map_index].size) {
        printf("Offset too high. Must be less than buffer size which is %lu\n", seg_mappings[local_map_index].size);
        return -1;
    }
    local_buffer_subsize = seg_mappings[local_map_index].size - local_offset;

    printf("\nArg 3, remoteMap.\n");
    printf("Enter a map index: ");
    remote_map_index = get_ulong_input();
    if (remote_map_index >= MAX_SEGMAPS) {
        printf("Map index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!seg_mappings[remote_map_index].used) {
        printf("No mapping set up for map index %u.\n", remote_map_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 4, Remote map offset in bytes.\n");
    printf("Enter remote offset: ");
    remote_offset = get_ulong_input();
    if (remote_offset >= seg_mappings[remote_map_index].size) {
        printf("Offset too high. Must be less remote map size size which is %lu\n", seg_mappings[remote_map_index].size);
        return -1;
    }
    remote_buffer_subsize = seg_mappings[remote_map_index].size - remote_offset;

    printf("\nArg 5, Transfer size in bytes.\n");
    printf("Enter transfer size: ");
    transfer_size = get_ulong_input();
    if ((transfer_size > local_buffer_subsize) || (transfer_size > remote_buffer_subsize)) {
        printf("Transfer size %lu too large. Must be smaller than buffers - offset for both remote and local buffer.\n", transfer_size);
        printf("local_buffer_size - local_offset == %lu - %lu == %lu\n", seg_mappings[local_map_index].size, local_offset, local_buffer_subsize);
        printf("remote_buffer_size - remote_offset == %lu - %lu == %lu\n", seg_mappings[remote_map_index].size, remote_offset, remote_buffer_subsize);
        printf("Aborting this command\n");
    }

    {
        unsigned int flag_idx_arr[] = {
            IDX_FLAG_BLOCK_READ,
            IDX_FLAG_ERROR_CHECK
        };
        print_flag_options(flag_idx_arr, sizeof(flag_idx_arr) / sizeof(unsigned int));
        printf("\nArg 6, Choose flags by indices shown above (space separated): ");
        flags = get_flags();
    }

    {
        char *local_buf = (char *) seg_mappings[local_map_index].mem_ptr;
        void *local_mem_addr = (void *) &local_buf[local_offset];
    
        SCIMemCpy(sequence, local_mem_addr, seg_mappings[remote_map_index].map, remote_offset, transfer_size, flags, &error);
        SISCI_ERROR_CHECK("SCIMemCpy", error);
        SCIFlush(sequence, 0);   
        printf("SCIMemCpy call succeeded (also called SCIFlush)\n");
    }

    return 0;
}

/******************************************************************************/
/***************************** SISCI DMA commands *****************************/
/******************************************************************************/

int handle_create_dma_queue() {
    sci_error_t error;
    unsigned int sd_index, dma_queue_index, local_adapter_no, max_entries;

    printf("void SCICreateDMAQueue(sci_desc_t sd, sci_dma_queue_t *dq, unsigned int localAdapterNo,\n");
    printf("                       unsigned int maxEntries, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, An open sisci descriptor.\n");
    printf("Enter scidesc index: ");
    sd_index = get_ulong_input();
    if (sd_index >= MAX_SDS) {
        printf("sci_desc index not in valid range [0, %d]\n", MAX_SDS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!sds[sd_index].used) {
        printf("No open sci_desc at index %d\n", sd_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, An unused DMA queue index.\n");
    printf("Enter DMA queue index (MAX=%u): ", MAX_DMA_QUEUES - 1);
    dma_queue_index = get_ulong_input();
    if (dma_queue_index >= MAX_DMA_QUEUES) {
        printf("DMA queue index not in valid range [0, %d]\n", MAX_DMA_QUEUES - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (dma_queues[dma_queue_index].used) {
        printf("DMA queue index %u already in use.\n", dma_queue_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 5, The local adapter number used for NTB (typically 0, but consult dis_diag)\n");
    printf("Enter local adapter number: ");
    local_adapter_no = get_ulong_input();

    printf("\nArg 4, Maximum DMA queue entries to allow.\n");
    printf("Enter max DMA queue entries: ");
    max_entries = get_ulong_input();

    printf("Arg 5, Flags: (not used)\n");

    SCICreateDMAQueue(sds[sd_index].sd, &dma_queues[dma_queue_index].dq, local_adapter_no, max_entries, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCICreateDMAQueue", error);

    dma_queues[dma_queue_index].used = TRUE;
    dma_queues[dma_queue_index].adapter_no = local_adapter_no;
    dma_queues[sd_index].sd_index = sd_index;
    cnt_dma_queues++;

    return 0;
}

int handle_remove_dma_queue() {

    sci_error_t error;
    unsigned int dma_queue_index;

    printf("void SCIRemoveDMAQueue(sci_dma_queue_t dq, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A previously created DMA queue to remove.\n");
    printf("Enter DMA queue index: ");
    dma_queue_index = get_ulong_input();
    if (dma_queue_index >= MAX_DMA_QUEUES) {
        printf("RDMA queue index not in valid range [0, %d]\n", MAX_DMA_QUEUES - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!dma_queues[dma_queue_index].used) {
        printf("DMA queue index %u not currently in use.\n", dma_queue_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Arg 2, Flags: (not used)\n");

    SCIRemoveDMAQueue(dma_queues[dma_queue_index].dq, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCIRemoveDMAQueue", error);

    dma_queues[dma_queue_index].used = FALSE;
    cnt_dma_queues--;

    return 0;
}

int handle_start_dma_transfer() {
    sci_error_t error;
    unsigned int dma_queue_index, lseg_index, rseg_index, flags;
    size_t local_offset, remote_offset, size;

    printf("void SCIStartDmaTransfer(sci_dma_queue_t dq, sci_local_segment_t localSegment,\n");
    printf("                         sci_remote_segment_t remoteSegment, size_t localOffset, size_t size,\n");
    printf("                         size_t remoteOffset, sci_cb_dma_t callback, void *callbackArg,\n");
    printf("                         unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A DMA queue.\n");
    printf("Enter DMA queue index: ");
    dma_queue_index = get_ulong_input();
    if (dma_queue_index >= MAX_DMA_QUEUES) {
        printf("DMA queue index not in valid range [0, %d]\n", MAX_DMA_QUEUES - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!dma_queues[dma_queue_index].used) {
        printf("DMA queue index %u not created.\n", dma_queue_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, A local segment.\n");
    printf("Enter local segment index: ");
    lseg_index = get_ulong_input();
    if (lseg_index >= MAX_LSEGS) {
        printf("Local seg index not in valid range [0, %d]\n", MAX_LSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!lsegs[lseg_index].used) {
        printf("Local seg index %u not currently in use.\n", lseg_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 3, A remote segment.\n");
    printf("Enter remote segment index: ");
    rseg_index = get_ulong_input();
    if (rseg_index >= MAX_RSEGS) {
        printf("Remote seg index not in valid range [0, %d]\n", MAX_RSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!rsegs[rseg_index].used) {
        printf("Remote seg index %u not currently in use.\n", rseg_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 4, A local segment offset in bytes from where the transfer should start.\n");
    printf("Enter local segment offset: ");
    local_offset = get_ulong_input();

    printf("\nArg 5, DMA transfer size in bytes\n");
    printf("Enter transfer size: ");
    size = get_ulong_input();

    printf("\nArg 4, A remote segment offset in bytes from where the transfer should start.\n");
    printf("Enter remote segment offset: ");
    remote_offset = get_ulong_input();

    printf("Arg 7, callback, not possible in interactive mode.\n");
    printf("Arg 8, callback argument, not relevant without callback.\n\n");

    {
        unsigned int flag_idx_arr[] = {
            IDX_FLAG_BROADCAST,
            IDX_FLAG_DMA_READ,
            IDX_FLAG_DMA_GLOBAL,
            IDX_FLAG_DMA_SYSDMA
        };
        print_flag_options(flag_idx_arr, sizeof(flag_idx_arr) / sizeof(unsigned int));
    }
    printf("Arg 9, Choose flags by indices shown above (SPACE SEPARATED. For no flags, press enter).\n");
    printf("Enter flag indices: ");
    flags = get_flags();

    SCIStartDmaTransfer(
        dma_queues[dma_queue_index].dq,
        lsegs[lseg_index].seg,
        rsegs[rseg_index].seg,
        local_offset,
        size,
        remote_offset,
        NO_CALLBACK,
        NO_CALLBACK_ARG,
        flags,
        &error);
    SISCI_ERROR_CHECK2("SCIStartDmaTransfer", error);
    return 0;
}

void print_dma_queue_state(sci_dma_queue_state_t dq_state) {
    printf("sci_dma_queue_state = ");
    switch (dq_state)
    {
    case SCI_DMAQUEUE_IDLE:
        printf("SCI_DMAQUEUE_IDLE");
        break;
    case SCI_DMAQUEUE_GATHER:
        printf("SCI_DMAQUEUE_GATHER");
        break;
    case SCI_DMAQUEUE_POSTED:
        printf("SCI_DMAQUEUE_POSTED");
        break;
    case SCI_DMAQUEUE_DONE:
        printf("SCI_DMAQUEUE_DONE");
        break;
    case SCI_DMAQUEUE_ABORTED:
        printf("SCI_DMAQUEUE_ABORTED");
        break;
    case SCI_DMAQUEUE_ERROR:
        printf("SCI_DMAQUEUE_ERROR");
        break;
    default:
        break;
    }
    printf("\n");
}

int handle_query_dma_queue_state() {
    sci_dma_queue_state_t dq_state;
    unsigned int dma_queue_index;
    
    printf("sci_dma_queue_state_t SCIDMAQueueState(sci_dma_queue_t dq);\n");
    
    printf("\nArg 1, DMA queue to query the state of.\n");
    printf("Enter DMA queue index: ");
    dma_queue_index = get_ulong_input();
    if (dma_queue_index >= MAX_DMA_QUEUES) {
        printf("DMA queue index not in valid range [0, %d]\n", MAX_DMA_QUEUES - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!dma_queues[dma_queue_index].used) {
        printf("DMA queue index %u not currently in use\n", dma_queue_index);
        printf("Aborting this command\n");
        return -1;
    }

    dq_state = SCIDMAQueueState(dma_queues[dma_queue_index].dq);
    print_dma_queue_state(dq_state);
    
    return 0;
}

int handle_wait_for_dma_queue() {
    sci_error_t error;
    sci_dma_queue_state_t dq_state;
    unsigned int dma_queue_index, timeout;
    boolean yes;

    printf("sci_dma_queue_state_t SCIWaitForDMAQueue(sci_dma_queue_t dq, unsigned int timeout,\n");
    printf("                                          unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, DMA queue to wait for.\n");
    printf("Enter DMA queue index: ");
    dma_queue_index = get_ulong_input();
    if (dma_queue_index >= MAX_DMA_QUEUES) {
        printf("DMA queue index not in valid range [0, %d]\n", MAX_DMA_QUEUES - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!dma_queues[dma_queue_index].used) {
        printf("DMA queue index %u not currently in use\n", dma_queue_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, time in milliseconds to wait for the connection to complete.\n");
    printf("Do you want to set timeout to SCI_INFINITE_TIMEOUT? [y/N] ");
    yes = check_yes_input();
    if (yes) {
        printf("Setting timeout to SCI_INFINITE_TIMEOUT.\n");
        timeout = SCI_INFINITE_TIMEOUT;
    } else {
        printf("Enter the desired timeout parameter in milliseconds: ");
        timeout = (size_t)get_ulong_input();
    }

    printf("Arg 3, Flags: (not used)\n");

    dq_state = SCIWaitForDMAQueue(dma_queues[dma_queue_index].dq, timeout, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCIWaitForDMAQueue", error);

    print_dma_queue_state(dq_state);

    return 0;
}

/******************************************************************************/
/************************** SISCI Interrupt commands **************************/
/******************************************************************************/

int handle_create_interrupt()
{
    sci_error_t error;
    unsigned int sd_index, interrupt_index, flags, interrupt_no, local_adapter_no;

    printf("void SCICreateInterrupt(sci_desc sd, sci_local_interrupt_t *interrupt,\n");
    printf("                        unsigned int localAdapterNo, unsigned int *interruptNo,\n");
    printf("                        sci_cb_interrupt_t callback, void *callbackArg,\n");
    printf("                        unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, An open sisci descriptor.\n");
    printf("Enter scidesc index: ");
    sd_index = get_ulong_input();
    if (sd_index >= MAX_SDS) {
        printf("sci_desc index not in valid range [0, %d]\n", MAX_SDS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!sds[sd_index].used) {
        printf("No open sci_desc at index %d\n", sd_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, An unused local interrupt index. Used for receiving interrupts.\n");
    printf("Enter local interrupt index (MAX=%u): ", MAX_LOCAL_INTERRUPTS - 1);
    interrupt_index = get_ulong_input();
    if (interrupt_index >= MAX_LOCAL_INTERRUPTS) {
        printf("Interrupt index not in valid range [0, %d]\n", MAX_LOCAL_INTERRUPTS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (local_interrupts[interrupt_index].used) {
        printf("Interrupt index %u already in use.\n", interrupt_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 5, The local adapter number used for NTB (typically 0, but consult dis_diag)\n");
    printf("Enter local adapter number: ");
    local_adapter_no = get_ulong_input();

    printf("Arg 4, Interrupt number. Depends on flag. Will ask again if SCI_FLAG_FIXED_INTNO is specified.\n");
    printf("Arg 5, callback, not possible in interactive mode.\n");
    printf("Arg 6, callback argument, not relevant without callback.\n");

    {
        unsigned int flag_idx_arr[] = {
            IDX_FLAG_FIXED_INTNO,
            IDX_FLAG_SHARED_INT,
            IDX_FLAG_COUNTING_INT
        };
        print_flag_options(flag_idx_arr, sizeof(flag_idx_arr) / sizeof(unsigned int));
    }
    printf("Arg 7, Choose flags by indices shown above (space separated): ");
    flags = get_flags();

    if ((flags & SCI_FLAG_FIXED_INTNO) != 0) {
        printf("Fixed interrupt number flag set.\n");
        printf("Enter desired interrupt number: ");
        interrupt_no = get_ulong_input();
    }

    SCICreateInterrupt(
        sds[sd_index].sd,
        &local_interrupts[interrupt_index].interrupt,
        local_adapter_no,
        &interrupt_no,
        NO_CALLBACK,
        NO_CALLBACK_ARG,
        flags,
        &error);
    SISCI_ERROR_CHECK2("SCICreateInterrupt", error);

    local_interrupts[interrupt_index].adapter_no = local_adapter_no;
    local_interrupts[interrupt_index].sd_index = sd_index;
    local_interrupts[interrupt_index].interrupt_no = interrupt_no;
    local_interrupts[interrupt_index].used = TRUE;
    cnt_local_interrupts++;

    return 0;
}

int handle_remove_interrupt() {
    sci_error_t error;
    unsigned int interrupt_index;

    printf("void SCIRemoveInterrupt(sci_local_interrupt_t interrupt, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A previously created local interrupt to remove.\n");
    printf("Enter local interrupt index: ");
    interrupt_index = get_ulong_input();
    if (interrupt_index >= MAX_LOCAL_INTERRUPTS) {
        printf("Interrupt index not in valid range [0, %d]\n", MAX_LOCAL_INTERRUPTS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!local_interrupts[interrupt_index].used) {
        printf("Interrupt index %u is not in use.\n", interrupt_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Arg 2, Flags: (not used)\n");

    SCIRemoveInterrupt(local_interrupts[interrupt_index].interrupt, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCIRemoveInterrupt", error);

    local_interrupts[interrupt_index].used = FALSE;
    cnt_local_interrupts--;

    return 0;
}

int handle_connect_interrupt() {
    sci_error_t error;
    unsigned int sd_index, interrupt_index, flags, interrupt_no, local_adapter_no, timeout, node_id;
    boolean yes;

    printf("void SCIConnectInterrupt(sci_desc sd, sci_remote_interrupt_t *interrupt,\n");
    printf("                         unsigned int nodeId, unsigned int localAdapterNo,\n");
    printf("                         unsigned int interruptNo, unsigned int timeout,\n");
    printf("                         unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, An open sisci descriptor.\n");
    printf("Enter scidesc index: ");
    sd_index = get_ulong_input();
    if (sd_index >= MAX_SDS) {
        printf("sci_desc index not in valid range [0, %d]\n", MAX_SDS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!sds[sd_index].used) {
        printf("No open sci_desc at index %d\n", sd_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, An unused remote interrupt index. Used for sending interrupts.\n");
    printf("Enter remote interrupt index (MAX=%u): ", MAX_REMOTE_INTERRUPTS - 1);
    interrupt_index = get_ulong_input();
    if (interrupt_index >= MAX_REMOTE_INTERRUPTS) {
        printf("Interrupt index not in valid range [0, %d]\n", MAX_REMOTE_INTERRUPTS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (remote_interrupts[interrupt_index].used) {
        printf("Interrupt index %u already in use.\n", interrupt_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 3, Remote node ID (Whatever you have configured. Consult dis_diag on other machine)\n");
    printf("Enter remote nodeID: ");
    node_id = get_ulong_input();

    printf("\nArg 4, The local adapter number used for NTB (typically 0, but consult dis_diag)\n");
    printf("Enter local adapter number: ");
    local_adapter_no = get_ulong_input();

    printf("\nArg 5, Interrupt number of interrupt on remote node.\n");
    printf("Enter interrupt number: ");
    interrupt_no = get_ulong_input();

    printf("\nArg 6, time in milliseconds to wait for the connection to complete.\n");
    printf("(Note: Despite the use of SCI_INFINITE_TIMEOUT, know that this function call doesn't currently block.)\n");
    printf("Timeout is not implemented and should always be set to SCI_INFINITE_TIMEOUT.\n");
    printf("Do you want to test setting it to something else? [y/N] ");
    yes = check_yes_input();
    if (yes) {
        printf("Enter the desired timeout parameter in milliseconds: ");
        timeout = (size_t)get_ulong_input();
    } else {
        printf("Setting timeout to SCI_INFINITE_TIMEOUT as per documentation.\n");
        timeout = SCI_INFINITE_TIMEOUT;
    }

    unsigned int flag_idx_arr[] = {
        IDX_FLAG_COUNTING_INT
    };
    print_flag_options(flag_idx_arr, sizeof(flag_idx_arr) / sizeof(unsigned int));
    printf("Arg 9, Choose flags by indices shown above (space separated): ");
    flags = get_flags();

    SCIConnectInterrupt(
        sds[sd_index].sd,
        &remote_interrupts[interrupt_index].interrupt,
        node_id,
        local_adapter_no,
        interrupt_no,
        timeout,
        flags,
        &error);
    SISCI_ERROR_CHECK2("SCIConnectInterrupt", error);

    remote_interrupts[interrupt_index].adapter_no = local_adapter_no;
    remote_interrupts[interrupt_index].sd_index = sd_index;
    remote_interrupts[interrupt_index].interrupt_no = interrupt_no;
    remote_interrupts[interrupt_index].node_id = node_id;
    remote_interrupts[interrupt_index].used = TRUE;
    cnt_remote_interrupts++;

    return 0;
}

int handle_disconnect_interrupt() {
    sci_error_t error;
    unsigned int interrupt_index;

    printf("void SCIDisconnectInterrupt(sci_remote_interrupt_t interrupt, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A previously connected interrupt to remove.\n");
    printf("Enter remote interrupt index: ");
    interrupt_index = get_ulong_input();
    if (interrupt_index >= MAX_REMOTE_INTERRUPTS) {
        printf("Interrupt index not in valid range [0, %d]\n", MAX_REMOTE_INTERRUPTS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!remote_interrupts[interrupt_index].used) {
        printf("Interrupt index %u is not in use.\n", interrupt_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Arg 2, Flags: (not used)\n");

    SCIDisconnectInterrupt(remote_interrupts[interrupt_index].interrupt, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCIDisconnectInterrupt", error);
    
    remote_interrupts[interrupt_index].used = FALSE;
    cnt_remote_interrupts--;

    return 0;
}

int handle_trigger_interrupt() {
    sci_error_t error;
    unsigned int interrupt_index;
    printf("void SCITriggerInterrupt(sci_remote_interrupt_t interrupt, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A previously connected interrupt to trigger.\n");
    printf("Enter remote interrupt index: ");
    interrupt_index = get_ulong_input();
    if (interrupt_index >= MAX_REMOTE_INTERRUPTS) {
        printf("Interrupt index not in valid range [0, %d]\n", MAX_REMOTE_INTERRUPTS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!remote_interrupts[interrupt_index].used) {
        printf("Interrupt index %u is not in use.\n", interrupt_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Arg 2, Flags: (not used)\n");

    SCITriggerInterrupt(remote_interrupts[interrupt_index].interrupt, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCITriggerInterrupt", error);

    return 0;
}

int handle_wait_for_interrupt() {
    sci_error_t error;
    unsigned int interrupt_index, timeout;
    boolean yes;
    printf("void SCIWaitForInterrupt(sci_local_interrupt_t interrupt, unsigned int timeout, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A previously created interrupt to wait on.\n");
    printf("Enter local interrupt index: ");
    interrupt_index = get_ulong_input();
    if (interrupt_index >= MAX_LOCAL_INTERRUPTS) {
        printf("Interrupt index not in valid range [0, %d]\n", MAX_LOCAL_INTERRUPTS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!local_interrupts[interrupt_index].used) {
        printf("Interrupt index %u is not in use.\n", interrupt_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, time in milliseconds to wait for the connection to complete.\n");
    printf("Do you want to set timeout to SCI_INFINITE_TIMEOUT? [y/N] ");
    yes = check_yes_input();
    if (yes) {
        printf("Setting timeout to SCI_INFINITE_TIMEOUT.\n");
        timeout = SCI_INFINITE_TIMEOUT;
    } else {
        printf("Enter the desired timeout parameter in milliseconds: ");
        timeout = (size_t)get_ulong_input();
    }

    printf("Arg 3, Flags: (not used)\n");

    SCIWaitForInterrupt(local_interrupts[interrupt_index].interrupt, timeout, NO_FLAGS, &error);
    if (error == SCI_ERR_OK) {
        printf("SCIWaitForInterrupt call succeeded\n");
    } else if (error == SCI_ERR_TIMEOUT) {
        printf("SCIWaitForInterrupt timed out\n");
    } else if (error != SCI_ERR_OK) {
        printf("SCIWaitForInterrupt failed - Error code: 0x%x\n", error);
        printf("   %s\n", SCIGetErrorString(error));
        return -1;
    }

    return 0;
}

/******************************************************************************/
/************************** SISCI Sequence commands ***************************/
/******************************************************************************/

int handle_create_map_sequence() {
    sci_error_t error;
    unsigned int map_index, sequence_index;

    printf("void SCICreateMapSequence(sci_map_t map, sci_sequence_t *sequence,\n");
    printf("                          unsigned int flags, sci_error_t error);\n");

    printf("\nArg 1, Map index to make sequence for. (Must correspond to a remote segment)\n");
    printf("Enter (remote seg) map index: ");
    map_index = get_ulong_input();
    if (map_index >= MAX_SEGMAPS) {
        printf("Map index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!seg_mappings[map_index].used) {
        printf("No mapping set up for map index %u.\n", map_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, An unused sequence index tp associate with new segment.\n");
    printf("Enter sequence index (MAX=%u): ", MAX_SEQUENCES - 1);
    sequence_index = get_ulong_input();
    if (sequence_index >= MAX_SEQUENCES) {
        printf("Sequence index not in valid range [0, %d]\n", MAX_SEQUENCES - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (sequences[sequence_index].used) {
        printf("Sequence index %u already in use.\n", sequence_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Arg 3, Flags: (not used)\n");

    SCICreateMapSequence(
        seg_mappings[map_index].map,
        &sequences[sequence_index].seq,
        NO_FLAGS,
        &error);
    SISCI_ERROR_CHECK2("SCICreateMapSequence", error);

    sequences[sequence_index].map_index = map_index;
    sequences[sequence_index].used = TRUE;
    cnt_sequences++;

    return 0;
}

int handle_remove_sequence() {
    sci_error_t error;
    unsigned int sequence_index;

    printf("void SCIRemoveSequence(sci_sequence_t sequence, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A previously created sequence to remove.\n");
    printf("Enter sequence index: ");
    sequence_index = get_ulong_input();
    if (sequence_index >= MAX_SEGMAPS) {
        printf("Sequence index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!seg_mappings[sequence_index].used) {
        printf("Sequence index %u is not in use.\n", sequence_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Arg 2, Flags: (not used)\n");

    SCIRemoveSequence(sequences[sequence_index].seq, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCIRemoveSequence", error);
    sequences[sequence_index].used = FALSE;
    cnt_sequences--;

    return 0;
}

void print_seq_status(sci_sequence_status_t seq_status) {
    printf("sci_sequence_status = ");
    switch (seq_status)
    {
    case SCI_SEQ_OK:
        printf("SCI_SEQ_OK");
        break;
    case SCI_SEQ_RETRIABLE:
        printf("SCI_SEQ_RETRIABLE");
        break;
    case SCI_SEQ_NOT_RETRIABLE:
        printf("SCI_SEQ_NOT_RETRIABLE");
        break;
    case SCI_SEQ_PENDING:
        printf("SCI_SEQ_PENDING");
        break;
    default:
        break;
    }
    printf("\n");
}

int handle_start_sequence() {
    sci_sequence_status_t seq_status;
    sci_error_t error;
    unsigned int sequence_index;

    printf("sci_sequence_status_t SCIStartSequence(sci_sequence_t sequence, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A previously created sequence.\n");
    printf("Enter sequence index: ");
    sequence_index = get_ulong_input();
    if (sequence_index >= MAX_SEGMAPS) {
        printf("Sequence index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!seg_mappings[sequence_index].used) {
        printf("Sequence index %u is not in use.\n", sequence_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Arg 2, Flags: (not used)\n");

    seq_status = SCIStartSequence(sequences[sequence_index].seq, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCIStartSequence", error);

    print_seq_status(seq_status);

    return 0;
}

int handle_check_sequence() {
    sci_sequence_status_t seq_status;
    sci_error_t error;
    unsigned int sequence_index;

    printf("sci_sequence_status_t SCICheckSequence(sci_sequence_t sequence, unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, A previously created sequence.\n");
    printf("Enter sequence index: ");
    sequence_index = get_ulong_input();
    if (sequence_index >= MAX_SEGMAPS) {
        printf("Sequence index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!seg_mappings[sequence_index].used) {
        printf("Sequence index %u is not in use.\n", sequence_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Arg 2, Flags: (not used)\n");

    seq_status = SCICheckSequence(sequences[sequence_index].seq, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCICheckSequence", error);
    print_seq_status(seq_status);

    return 0;
}

/******************************************************************************/
/**************************** SISCI Query Command *****************************/
/******************************************************************************/


int query_adapter_helper() {
    unsigned int i;
    sci_query_adapter_t query_adapter;
    sci_error_t error;
    unsigned int subcmd_index;
    unsigned int adapter_link;

    enum q_adapter_subcmd_flag_idx {
        Q_ADAPTER_SERIAL_NUMBER,
        Q_ADAPTER_CARD_TYPE,
        Q_ADAPTER_NODEID,
        Q_ADAPTER_CONFIGURED,
        Q_ADAPTER_DMA_MTU,
        Q_ADAPTER_MCAST_MAX_GROUPS,
        Q_ADAPTER_BDF,
        Q_ADAPTER_NUMBER_OF_LINKS,
        Q_ADAPTER_LINK_OPERATIONAL,
        Q_ADAPTER_LINK_WIDTH,
        Q_ADAPTER_LINK_SPEED,
        Q_ADAPTER_LINK_UPTIME,
        Q_ADAPTER_LINK_DOWNTIME,
        Q_ADAPTER_LINK_CABLE_INSERTED,
        Q_ADAPTER_LINK_ENABLED,
        Q_ADAPTER_LINK_PARTNER_PORT_NO,
    };
    static const char *q_adapter_subcmd_names[] = {
        "SCI_Q_ADAPTER_SERIAL_NUMBER",
        "SCI_Q_ADAPTER_CARD_TYPE",
        "SCI_Q_ADAPTER_NODEID",
        "SCI_Q_ADAPTER_CONFIGURED",
        "SCI_Q_ADAPTER_DMA_MTU",
        "SCI_Q_ADAPTER_MCAST_MAX_GROUPS",
        "SCI_Q_ADAPTER_BDF",
        "SCI_Q_ADAPTER_NUMBER_OF_LINKS",
        "SCI_Q_ADAPTER_LINK_OPERATIONAL",
        "SCI_Q_ADAPTER_LINK_WIDTH",
        "SCI_Q_ADAPTER_LINK_SPEED",
        "SCI_Q_ADAPTER_LINK_UPTIME",
        "SCI_Q_ADAPTER_LINK_DOWNTIME",
        "SCI_Q_ADAPTER_LINK_CABLE_INSERTED",
        "SCI_Q_ADAPTER_LINK_ENABLED",
        "SCI_Q_ADAPTER_LINK_PARTNER_PORT_NO",
    };

    printf("Enter local adapter number (Try 0): ");
    query_adapter.localAdapterNo = get_ulong_input();

    printf("\n");
    for (i = 0; i < sizeof(q_adapter_subcmd_names) / sizeof(char *); i++) {
        printf("%3u : %s\n", i, q_adapter_subcmd_names[i]);
    }

    printf("Choose subcommand from above by index: ");
    subcmd_index = get_ulong_input();

    switch (subcmd_index)
    {
    case Q_ADAPTER_SERIAL_NUMBER:
    {
        unsigned int serial_number;
        query_adapter.subcommand = SCI_Q_ADAPTER_SERIAL_NUMBER;
        query_adapter.data = &serial_number;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Serial number = %u\n", serial_number);
        break;
    }
    case Q_ADAPTER_CARD_TYPE:
    {
        unsigned int adapter_type;
        query_adapter.subcommand = SCI_Q_ADAPTER_CARD_TYPE;
        query_adapter.data = &adapter_type;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Adapter type = %s\n", SCIGetAdapterTypeString(adapter_type));
        break;
    }
    case Q_ADAPTER_NODEID:
    {
        unsigned int local_node_id;
        query_adapter.subcommand = SCI_Q_ADAPTER_NODEID;
        query_adapter.data = &local_node_id;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Node ID = %u\n", local_node_id);
        break;
    }
    case Q_ADAPTER_CONFIGURED:
    {
        unsigned int adapter_configured;
        query_adapter.subcommand = SCI_Q_ADAPTER_CONFIGURED;
        query_adapter.data = &adapter_configured;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Adapter configured? : %s\n", Yes_or_No(adapter_configured));
        break;
    }
    case Q_ADAPTER_DMA_MTU:
    {
        unsigned int adapter_dma_mtu;
        query_adapter.subcommand = SCI_Q_ADAPTER_DMA_MTU;
        query_adapter.data = &adapter_dma_mtu;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Max Transfer Unit (MTU) of DMA engine = %u\n", adapter_dma_mtu);
        break;
    }
    case Q_ADAPTER_MCAST_MAX_GROUPS:
    {
        unsigned int mcast_max_groups;
        query_adapter.subcommand = SCI_Q_ADAPTER_MCAST_MAX_GROUPS;
        query_adapter.data = &mcast_max_groups;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Max multicast groups = %u\n", mcast_max_groups);
        break;
    }
    case Q_ADAPTER_BDF:
    {
        unsigned int bdf;
        query_adapter.subcommand = SCI_Q_ADAPTER_BDF;
        query_adapter.data = &bdf;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("bdf = %02x:%02x.%x\n", (bdf >> 8) & 0xff, (bdf >> 3) & 0x1f, bdf & 0x07);
        break;
    }
    case Q_ADAPTER_NUMBER_OF_LINKS:
    {
        unsigned int num_links;
        query_adapter.subcommand = SCI_Q_ADAPTER_NUMBER_OF_LINKS;
        query_adapter.data = &num_links;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Number of links = %u\n", num_links);
        break;
    }
    
    case Q_ADAPTER_LINK_OPERATIONAL:
    {
        unsigned int link_operational;
        printf("Type link number (Try 0): ");
        adapter_link = get_ulong_input();
        query_adapter.portNo = adapter_link;
        query_adapter.subcommand = SCI_Q_ADAPTER_LINK_OPERATIONAL;
        query_adapter.data = &link_operational;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Link operational? = %s\n", Yes_or_No(link_operational));
        break;
    }
    case Q_ADAPTER_LINK_WIDTH:
    {
        unsigned int link_width;
        printf("Type link number (Try 0): ");
        adapter_link = get_ulong_input();
        query_adapter.portNo = adapter_link;
        query_adapter.subcommand = SCI_Q_ADAPTER_LINK_WIDTH;
        query_adapter.data = &link_width;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Link width = %u\n", link_width);
        break;
    }
    case Q_ADAPTER_LINK_SPEED:
    {
        unsigned int link_speed;
        printf("Type link number (Try 0): ");
        adapter_link = get_ulong_input();
        query_adapter.portNo = adapter_link;
        query_adapter.subcommand = SCI_Q_ADAPTER_LINK_SPEED;
        query_adapter.data = &link_speed;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Link speed = Gen%u\n", link_speed);
        break;
    }
    case Q_ADAPTER_LINK_UPTIME:
    {
        unsigned int link_uptime;
        printf("Type link number (Try 0): ");
        adapter_link = get_ulong_input();
        query_adapter.portNo = adapter_link;
        query_adapter.subcommand = SCI_Q_ADAPTER_LINK_UPTIME;
        query_adapter.data = &link_uptime;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Link uptime = %u seconds\n", link_uptime);
        break;
    }
    case Q_ADAPTER_LINK_DOWNTIME:
    {
        unsigned int link_downtime;
        printf("Type link number (Try 0): ");
        adapter_link = get_ulong_input();
        query_adapter.portNo = adapter_link;
        query_adapter.subcommand = SCI_Q_ADAPTER_LINK_DOWNTIME;
        query_adapter.data = &link_downtime;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Link downtime = %u seconds\n", link_downtime);
        break;
    }
    case Q_ADAPTER_LINK_CABLE_INSERTED:
    {
        unsigned int cable_inserted;
        printf("Type link number (Try 0): ");
        adapter_link = get_ulong_input();
        query_adapter.portNo = adapter_link;
        query_adapter.subcommand = SCI_Q_ADAPTER_LINK_CABLE_INSERTED;
        query_adapter.data = &cable_inserted;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Link cable inserted? = %s\n", Yes_or_No(cable_inserted));
        break;
    }
    case Q_ADAPTER_LINK_ENABLED:
    {
        unsigned int cable_enabled;
        printf("Type link number (Try 0): ");
        adapter_link = get_ulong_input();
        query_adapter.portNo = adapter_link;
        query_adapter.subcommand = SCI_Q_ADAPTER_LINK_ENABLED;
        query_adapter.data = &cable_enabled;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Link cable inserted? = %s\n", Yes_or_No(cable_enabled));
        break;
    }
    case Q_ADAPTER_LINK_PARTNER_PORT_NO:
    {
        unsigned int partner_port_no;
        printf("Type link number (Try 0): ");
        adapter_link = get_ulong_input();
        query_adapter.portNo = adapter_link;
        query_adapter.subcommand = Q_ADAPTER_LINK_PARTNER_PORT_NO;
        query_adapter.data = &partner_port_no;
        SCIQuery(SCI_Q_ADAPTER, &query_adapter, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Partner port number = %u\n", partner_port_no);
        break;
    }

    default:
        printf("Specified index not associated with a command.\n");
        printf("Aborting query.\n");
        return -1;
    }

    return 0;
}

int query_system_helper()
{
    unsigned int i;
    sci_query_system_t query_system;
    sci_error_t error;
    unsigned int subcmd_index;

    enum q_system_subcmd_flag_idx {
        Q_SYSTEM_CPU_CACHE_LINE_SIZE,
    };
    static const char *q_system_subcmd_names[] = {
        "SCI_Q_SYSTEM_CPU_CACHE_LINE_SIZE",
    };

    printf("\n");
    for (i = 0; i < sizeof(q_system_subcmd_names) / sizeof(char *); i++) {
        printf("%3u : %s\n", i, q_system_subcmd_names[i]);
    }

    printf("Choose subcommand from above by index: ");
    subcmd_index = get_ulong_input();

    switch (subcmd_index)
    {
    case Q_SYSTEM_CPU_CACHE_LINE_SIZE:
    {
        unsigned int cache_line_size;
        query_system.subcommand = SCI_Q_SYSTEM_CPU_CACHE_LINE_SIZE;
        query_system.data = &cache_line_size;
        SCIQuery(SCI_Q_SYSTEM, &query_system, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Cache line size = %u\n", cache_line_size);
        break;
    }
    default:
        printf("Specified index not associated with a command.\n");
        printf("Aborting query.\n");
        return -1;
    }

    return 0;
}

int query_dma_helper()
{
    unsigned int i;
    sci_query_dma_t query_dma;
    sci_error_t error;
    unsigned int subcmd_index;
    unsigned int dma_mode_index;
    unsigned int flags;

    enum q_dma_subcmd_flag_idx {
        Q_DMA_AVAILABLE,
        Q_DMA_CAPABILITIES,
    };
    static const char *q_dma_subcmd_names[] = {
        "SCI_Q_DMA_AVAILABLE",
        "SCI_Q_DMA_CAPABILITIES",
    };

    static const char *dma_mode_names[] = {
        "SCI_FLAG_DMA_ADAPTER",
        "SCI_FLAG_DMA_GLOBAL",
        "SCI_FLAG_DMA_SYSDMA",
    };
    unsigned int dma_mode_flags[] = {
        SCI_FLAG_DMA_ADAPTER,
        SCI_FLAG_DMA_GLOBAL,
        SCI_FLAG_DMA_SYSDMA,
    };

    printf("Enter local adapter number (Try 0): ");
    query_dma.localAdapterNo = get_ulong_input();

    printf("\n");
    for (i = 0; i < sizeof(dma_mode_names) / sizeof(char *); i++) {
        printf("%3u : %s\n", i, dma_mode_names[i]);
    }
    printf("Choose DMA type from above by index: ");
    dma_mode_index = get_ulong_input();
    if (dma_mode_index > sizeof(dma_mode_names) / sizeof(char *)) {
        printf("Chosen index is too high.\n");
        printf("Aborting query.\n");
        return -1;
    }
    flags = dma_mode_flags[dma_mode_index];

    printf("\n");
    for (i = 0; i < sizeof(q_dma_subcmd_names) / sizeof(char *); i++) {
        printf("%3u : %s\n", i, q_dma_subcmd_names[i]);
    }
    printf("Choose subcommand from above by index: ");
    subcmd_index = get_ulong_input();

    switch (subcmd_index)
    {
    case Q_DMA_AVAILABLE:
    {
        unsigned int dma_available;
        query_dma.subcommand = SCI_Q_DMA_AVAILABLE;
        query_dma.data = &dma_available;
        SCIQuery(SCI_Q_DMA, &query_dma, flags, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Adapter DMA available? = %s\n", Yes_or_No(dma_available));
        break;
    }
    case Q_DMA_CAPABILITIES:
    {
        sci_dma_capabilities_t dma_cap;
        query_dma.subcommand = SCI_Q_DMA_CAPABILITIES;
        query_dma.data = &dma_cap;
        SCIQuery(SCI_Q_DMA, &query_dma, flags, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Number of channels       : %u\n", dma_cap.num_channels);
        printf("Maximum transfer size    : %u\n", dma_cap.max_transfer_size);
        printf("Maximum vector length    : %u\n", dma_cap.max_vector_length);
        printf("Support mem2io           : %u\n", dma_cap.support_mem2io);
        printf("Support io2mem           : %u\n", dma_cap.support_io2mem);
        printf("Support io2io (P2P)      : %u\n", dma_cap.support_io2io);
        printf("Minimum alignment size   : %u\n", dma_cap.min_align_bytes);
        printf("Suggested alignment size : %u\n\n", dma_cap.suggested_align_bytes);
        break;
    }
    default:
        printf("Specified index not associated with a command.\n");
        printf("Aborting query.\n");
        return -1;
    }

    return 0;
}

int query_local_segment_helper()
{
    unsigned int i;
    sci_query_local_segment_t query_lseg;
    sci_error_t error;
    unsigned int subcmd_index;
    unsigned int seg_index;

    enum q_lseg_subcmd_flag_idx {
        Q_LOCAL_SEGMENT_IOADDR,
        Q_LOCAL_SEGMENT_VIRTUAL_KERNEL_ADDR,
        Q_LOCAL_SEGMENT_PHYS_ADDR,
    };
    static const char *q_lseg_subcmd_names[] = {
        "SCI_Q_LOCAL_SEGMENT_IOADDR",
        "SCI_Q_LOCAL_SEGMENT_VIRTUAL_KERNEL_ADDR",
        "SCI_Q_LOCAL_SEGMENT_PHYS_ADDR",
    };

    printf("Enter a local segment index: ");
    seg_index = get_ulong_input();
    if (seg_index >= MAX_LSEGS) {
        printf("Local seg index not in valid range [0, %d]\n", MAX_LSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!lsegs[seg_index].used) {
        printf("Local seg index %u not created.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }
    query_lseg.segment = lsegs[seg_index].seg;
    
    printf("\n");
    for (i = 0; i < sizeof(q_lseg_subcmd_names) / sizeof(char *); i++) {
        printf("%3u : %s\n", i, q_lseg_subcmd_names[i]);
    }
    printf("Choose subcommand from above by index: ");
    subcmd_index = get_ulong_input();

    switch (subcmd_index)
    {
    case Q_LOCAL_SEGMENT_IOADDR:
        query_lseg.subcommand = SCI_Q_LOCAL_SEGMENT_IOADDR;
        SCIQuery(SCI_Q_LOCAL_SEGMENT, &query_lseg, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Local segment IO address = 0x%llx\n", query_lseg.data.ioaddr);
        break;
    case Q_LOCAL_SEGMENT_VIRTUAL_KERNEL_ADDR:
        query_lseg.subcommand = SCI_Q_LOCAL_SEGMENT_VIRTUAL_KERNEL_ADDR;
        SCIQuery(SCI_Q_LOCAL_SEGMENT, &query_lseg, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Local segment kernel virtual address = 0x%llx\n", query_lseg.data.ioaddr);
        break;
    case Q_LOCAL_SEGMENT_PHYS_ADDR:
        query_lseg.subcommand = SCI_Q_LOCAL_SEGMENT_PHYS_ADDR;
        SCIQuery(SCI_Q_LOCAL_SEGMENT, &query_lseg, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Local segment physical address = 0x%llx\n", query_lseg.data.ioaddr);
        break;
    default:
        printf("Specified index not associated with a command.\n");
        printf("Aborting query.\n");
        return -1;
    }

    return 0;
}

int query_remote_segment_helper()
{
    unsigned int i;
    sci_query_remote_segment_t query_rseg;
    sci_error_t error;
    unsigned int subcmd_index;
    unsigned int seg_index;

    enum q_rseg_subcmd_flag_idx {
        Q_REMOTE_SEGMENT_IOADDR,
    };
    static const char *q_rseg_subcmd_names[] = {
        "SCI_Q_REMOTE_SEGMENT_IOADDR",
    };

    printf("Enter a remote segment index: ");
    seg_index = get_ulong_input();
    if (seg_index >= MAX_RSEGS) {
        printf("Remote seg index not in valid range [0, %d]\n", MAX_RSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!rsegs[seg_index].used) {
        printf("Remote seg index %u not created.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }
    query_rseg.segment = rsegs[seg_index].seg;

    printf("\n");
    for (i = 0; i < sizeof(q_rseg_subcmd_names) / sizeof(char *); i++) {
        printf("%3u : %s\n", i, q_rseg_subcmd_names[i]);
    }
    printf("Choose subcommand from above by index: ");
    subcmd_index = get_ulong_input();

    switch (subcmd_index)
    {
    case Q_REMOTE_SEGMENT_IOADDR:
        query_rseg.subcommand = SCI_Q_REMOTE_SEGMENT_IOADDR;
        SCIQuery(SCI_Q_REMOTE_SEGMENT, &query_rseg, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Remote segment IO address = 0x%llx\n", query_rseg.data.ioaddr);
        break;
    default:
        printf("Specified index not associated with a command.\n");
        printf("Aborting query.\n");
        return -1;
    }

    return 0;
}

int query_map_helper()
{
    unsigned int i;
    sci_query_map_t query_map;
    sci_error_t error;
    unsigned int subcmd_index;
    unsigned int map_index;

    enum q_map_subcmd_flag_idx {
        Q_MAP_MAPPED_TO_LOCAL_TARGET,
        Q_MAP_IOADDR,
        Q_MAP_PHYS_ADDR,
    };
    static const char *q_map_subcmd_names[] = {
        "SCI_Q_MAP_MAPPED_TO_LOCAL_TARGET",
        "SCI_Q_MAP_IOADDR",
        "SCI_Q_MAP_PHYS_ADDR",
    };

    printf("Enter a map index: ");
    map_index = get_ulong_input();
    if (map_index >= MAX_SEGMAPS) {
        printf("Remote seg index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!seg_mappings[map_index].used) {
        printf("Remote seg index %u not created.\n", map_index);
        printf("Aborting this command\n");
        return -1;
    }
    query_map.map = seg_mappings[map_index].map;

    printf("\n");
    for (i = 0; i < sizeof(q_map_subcmd_names) / sizeof(char *); i++) {
        printf("%3u : %s\n", i, q_map_subcmd_names[i]);
    }
    printf("Choose subcommand from above by index: ");
    subcmd_index = get_ulong_input();

    switch (subcmd_index)
    {
    case Q_MAP_MAPPED_TO_LOCAL_TARGET:
        query_map.subcommand = SCI_Q_MAP_MAPPED_TO_LOCAL_TARGET;
        SCIQuery(SCI_Q_REMOTE_SEGMENT, &query_map, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Mapped segment is local? = %s\n", Yes_or_No((boolean)query_map.data));
        break;
    case Q_MAP_IOADDR:
        query_map.subcommand = SCI_Q_MAP_IOADDR;
        SCIQuery(SCI_Q_REMOTE_SEGMENT, &query_map, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Mapped segment IO address = 0x%llx\n", query_map.data);
        break;
    case Q_MAP_PHYS_ADDR:
        query_map.subcommand = SCI_Q_MAP_PHYS_ADDR;
        SCIQuery(SCI_Q_REMOTE_SEGMENT, &query_map, NO_FLAGS, &error);
        SISCI_ERROR_CHECK2("SCIQuery", error);
        printf("Mapped segment physcial address = 0x%llx\n", query_map.data);
        break;
    default:
        printf("Specified index not associated with a command.\n");
        printf("Aborting query.\n");
        return -1;
    }

    return 0;
}

int query_vendorid_helper()
{
    char vendor_string[80];
    sci_query_string_t query_string;
    sci_error_t  error;

    query_string.length = sizeof(vendor_string);
    query_string.str = vendor_string; 

    SCIQuery(SCI_Q_VENDORID, &query_string, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCIQuery", error);

    printf("Vendor = %s\n", vendor_string);

    return 0;
}

int query_api_helper()
{

    char sisci_api_ver_string[80];
    sci_query_string_t query_string;
    sci_error_t  error;

    query_string.length = sizeof(sisci_api_ver_string);
    query_string.str = sisci_api_ver_string; 

    SCIQuery(SCI_Q_API, &query_string, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCIQuery", error);

    printf("SISCI API version = %s\n", sisci_api_ver_string);

    return 0;
}

int handle_query()
{
    unsigned int i;
    unsigned int command_index;
    printf("void SCIQuery(unsigned int command, void *data, unsigned int flags, sci_error_t *error);\n");

    enum command_flag_idx {
        Q_ADAPTER,
        Q_SYSTEM,
        Q_DMA,
        Q_LOCAL_SEGMENT,
        Q_REMOTE_SEGMENT,
        Q_MAP,
        Q_VENDORID,
        Q_API,
    };

    static const char *q_command_names[] = {
        "SCI_Q_ADAPTER",
        "SCI_Q_SYSTEM",
        "SCI_Q_DMA",
        "SCI_Q_LOCAL_SEGMENT",
        "SCI_Q_REMOTE_SEGMENT",
        "SCI_Q_MAP",
        "SCI_Q_VENDORID",
        "SCI_Q_API",
    };

    printf("\n");
    for (i = 0; i < sizeof(q_command_names) / sizeof(char *); i++) {
        printf("%3u : %s\n", i, q_command_names[i]);
    }

    printf("Choose command from above by index: ");
    command_index = get_ulong_input();
    switch (command_index)
    {
    case Q_ADAPTER:
        query_adapter_helper();
        break;
    case Q_SYSTEM:
        query_system_helper();
        break;
    case Q_DMA:
        query_dma_helper();
        break;
    case Q_LOCAL_SEGMENT:
        query_local_segment_helper();
        break;
    case Q_REMOTE_SEGMENT:
        query_remote_segment_helper();
        break;
    case Q_MAP:
        query_map_helper();
        break;
    case Q_VENDORID:
        query_vendorid_helper();
        break;
    case Q_API:
        query_api_helper();
        break;
    default:
        printf("Specified index not associated with a command.\n");
        printf("Aborting query.\n");
        return -1;
    }

    return 0;
}


/******************************************************************************/
/************************ SISCI Miscellaneous commands ************************/
/******************************************************************************/

int handle_probe_node() {
    sci_error_t error;
    int is_reachable;
    unsigned int sd_index, local_adapter_no, node_id;

    printf("int SCIProbeNode(sci_desc_t sd, unsigned int localAdapterNo, unsigned int nodeId,\n");
    printf("                 unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, An open sisci descriptor.\n");
    printf("Enter scidesc index: ");
    sd_index = get_ulong_input();
    if (sd_index >= MAX_SDS) {
        printf("sci_desc index not in valid range [0, %d]\n", MAX_SDS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!sds[sd_index].used) {
        printf("No open sci_desc at index %d\n", sd_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, The local adapter number used for NTB (typically 0, but consult dis_diag)\n");
    printf("Enter local adapter number: ");
    local_adapter_no = get_ulong_input();

    printf("\nArg 3, Node ID of remote node (According to your cluster configuration)\n");
    printf("Enter remote node ID: ");
    node_id = get_ulong_input();

    printf("Arg 4, Flags: (not used)\n");

    is_reachable = SCIProbeNode(
        sds[sd_index].sd,
        local_adapter_no,
        node_id,
        NO_FLAGS,
        &error);
    SISCI_ERROR_CHECK2("SCIProbeNode", error);
    printf("SCIProbeNode returned %d.\n", is_reachable);

    if (is_reachable) {
        printf("Node is reachable.\n");
    } else {
        printf("Node is not reachable.\n");
    }

    return 0;
}

int handle_get_local_node_id()
{
    sci_error_t error;
    unsigned int local_adapter_no, node_id;
    printf("void SCIGetLocalNodeId(unsigned int adapterNo, unsigned int *nodeId,\n");
    printf("                       unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, The local adapter number used for NTB (typically 0, but consult dis_diag)\n");
    printf("Enter local adapter number: ");
    local_adapter_no = get_ulong_input();
    
    printf("Arg 2, nodeId is an output parameter.\n");

    printf("Arg 3, Flags: (not used)\n");

    SCIGetLocalNodeId(local_adapter_no, &node_id, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCIGetLocalNodeId", error);

    printf("Node ID = %u\n", node_id);

    return 0;
}

int handle_share_seg()
{
    sci_error_t error;
    unsigned int seg_index;
    printf("void SCIShareSegment(sci_local_segment_t segment, unsigned int flags,\n");
    printf("                     sci_error_t *error);\n");

    printf("\nArg 1, Previously created local segment to share.\n");
    printf("Enter a local segment index: ");
    seg_index = get_ulong_input();
    if (seg_index >= MAX_LSEGS) {
        printf("Local seg index not in valid range [0, %d]\n", MAX_LSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!lsegs[seg_index].used) {
        printf("Local seg index %u not created.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Arg 2, Flags: (not used)\n");

    SCIShareSegment(lsegs[seg_index].seg, NO_FLAGS, &error);
    SISCI_ERROR_CHECK2("SCIShareSegment", error);

    lsegs[seg_index].is_shared = TRUE;

    printf("SUCCESS, but NOTE: The segment will share its state (prepared, available) "
           "for all processes accessing it. But this program has no way of "
           "knowing state changes when initiated from other processes, so the "
           "state in the resource menu illustrated here will not update according to "
           "changes to segment state made from another process.\n");

    return 0;
}

int handle_attach_local_seg()
{
    sci_error_t error;
    unsigned int sd_index, seg_index, seg_id, flags;

    printf("void SCIAttachLocalSegment(sci_desc sd, sci_local_segment_t *segment,\n");
    printf("                           unsigned int segmentId, size_t *size,\n");
    printf("                           sci_cb_local_segment_t callback, void *callbackArg,\n");
    printf("                           unsigned int flags, sci_error_t *error);\n");

    printf("\nArg 1, An open sisci descriptor.\n");
    printf("Enter scidesc index: ");
    sd_index = get_ulong_input();
    if (sd_index >= MAX_SDS) {
        printf("sci_desc index not in valid range [0, %d]\n", MAX_SDS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!sds[sd_index].used) {
        printf("No open sci_desc at index %d\n", sd_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 2, An unused local segment index to associate with segment.\n");
    printf("Enter local segment index (MAX=%u): ", MAX_LSEGS - 1);
    seg_index = get_ulong_input();
    if (seg_index >= MAX_LSEGS) {
        printf("Local seg index not in valid range [0, %d]\n", MAX_LSEGS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (lsegs[seg_index].used) {
        printf("Local seg index %u already in use.\n", seg_index);
        printf("Aborting this command\n");
        return -1;
    }

    printf("\nArg 3, A segment ID of the segment to attach to.\n");
    printf("Enter segment ID: ");
    seg_id = get_ulong_input();

    printf("Arg 4, Segment size (output parameter).\n");
    printf("Arg 5, callback, not possible in interactive mode.\n");
    printf("Arg 6, callback argument, not relevant without callback.\n");

    {
        unsigned int flag_idx_arr[] = {
            IDX_FLAG_BROADCAST
        };
        print_flag_options(flag_idx_arr, sizeof(flag_idx_arr) / sizeof(unsigned int));
    }
    printf("\nArg 7, Choose flags by indices shown above (SPACE SEPARATED. For no flags, press enter).\n");
    printf("Enter flag indices: ");
    flags = get_flags();

    SCIAttachLocalSegment(
        sds[sd_index].sd,
        &lsegs[seg_index].seg,
        seg_id,
        &lsegs[seg_index].size,
        NO_CALLBACK,
        NO_CALLBACK_ARG,
        flags,
        &error);
    if (error != SCI_ERR_OK) {
        printf("SCIAttachLocalSegment failed - Error code: 0x%x\n", error);
        printf("   %s\n", SCIGetErrorString(error));
        printf("Common mistake: Forgetting to share the segment with SCIShareSegment.\n");
        return -1;
    }
    printf("SCIAttachLocalSegment call succeeded.\n");

    printf("SUCCESS, but NOTE: The segment will share its state (prepared, available) "
           "for all processes accessing it. But this program has no way of "
           "knowing state changes when initiated from other processes, so the "
           "state in the resource menu illustrated here will not update according to "
           "changes to segment state made from another process. "
           "The displayed size will also reflect how things are set up behind the scenes "
           "(i.e. it may be larger than what was originally asked for).\n");

    lsegs[seg_index].used = TRUE;
    lsegs[seg_index].id = seg_id;
    lsegs[seg_index].is_prepared = FALSE;
    lsegs[seg_index].is_set_available = FALSE;
    lsegs[seg_index].is_shared = TRUE;

    if ((flags & SCI_FLAG_BROADCAST) != 0) {
        lsegs[seg_index].is_broadcast = TRUE;
    } else {
        lsegs[seg_index].is_broadcast = FALSE;
    }

    cnt_lsegs++;

    return 0;
}

/******************************************************************************/
/************************* PIO access helper commands *************************/
/******************************************************************************/

/* Never print more than 1000 bytes.  */
#define MAX_BYTES_PRINT 1000

int handle_pio_read()
{
    unsigned int map_index;
    size_t start_i, len, bytes_left, max_len;

    printf("Command to read range of bytes from mapped segment\n");

    printf("Enter a map index: ");
    map_index = get_ulong_input();
    if (map_index >= MAX_SEGMAPS) {
        printf("Map index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!seg_mappings[map_index].used) {
        printf("No mapping set up for map index %u.\n", map_index);
        printf("Aborting this command\n");
        return -1;
    }

    if (seg_mappings[map_index].size == 0) {
        printf("Segment size is 0. Cannot read from empty segment.\n");
        return -1;
    }

    printf("Start index (MAX=%lu): ", seg_mappings[map_index].size-1);
    start_i = get_ulong_input();
    if (start_i > seg_mappings[map_index].size) {
        printf("Index too high.\n");
        return -1;
    }

    bytes_left = seg_mappings[map_index].size - start_i;
    max_len = bytes_left > MAX_BYTES_PRINT ? MAX_BYTES_PRINT : bytes_left;

    printf("Number of bytes to read and print to terminal (MAX=%lu): ", max_len);
    len = get_ulong_input();
    if (len > max_len) {
        printf("Number is too high.\n");
        printf("Maximum allowed to print is %u, and bytes left in buffer is %lu\n", MAX_BYTES_PRINT, bytes_left);
        printf("Aborting command\n");
        return -1;
    }

    {
        size_t i;
        volatile unsigned char *byte_ptr = (volatile unsigned char *)seg_mappings[map_index].mem_ptr;
        for (i = start_i; i < start_i + len; i++) {
            printf("%u ", byte_ptr[i]);
        }
        printf("\n");
    }

    return 0;
}

int handle_pio_write()
{
    unsigned int map_index;
    size_t start_i, len, bytes_left, num;

    printf("Command to write range of bytes to mapped segment\n");

    printf("Enter a map index: ");
    map_index = get_ulong_input();
    if (map_index >= MAX_SEGMAPS) {
        printf("Map index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!seg_mappings[map_index].used) {
        printf("No mapping set up for map index %u.\n", map_index);
        printf("Aborting this command\n");
        return -1;
    }

    if (seg_mappings[map_index].size == 0) {
        printf("Segment size is 0. Cannot write to empty segment.\n");
        return -1;
    }

    printf("Start index (MAX=%lu): ", seg_mappings[map_index].size-1);
    start_i = get_ulong_input();
    if (start_i > seg_mappings[map_index].size) {
        printf("Index too high.\n");
        return -1;
    }

    bytes_left = seg_mappings[map_index].size - start_i;

    printf("Number of bytes to write (MAX=%lu): ", bytes_left);
    len = get_ulong_input();
    if (len > bytes_left) {
        printf("Number is too high.\n");
        printf("Note: bytes left in buffer (after subracting specified offset) is %lu\n", bytes_left);
        printf("Aborting command\n");
        return -1;
    }

    printf("Number to write (between 0 and 255): ");
    num = get_ulong_input();
    if (num > 255) {
        printf("Specified number is too high. Must fit in char (byte)\n");
        return -1;
    }

    {
        size_t i;
        volatile unsigned char *byte_ptr = (volatile unsigned char *)seg_mappings[map_index].mem_ptr;
        for (i = start_i; i < start_i + len; i++) {
            byte_ptr[i] = num;
        }
        SCIFlush(NULL, 0);
    }

    printf("Write successful\n");

    return 0;
}

int handle_pio_copy()
{
    unsigned int map_index_src, map_index_dst;
    size_t start_i_src, start_i_dst, len, bytes_left_src, bytes_left_dst;

    printf("Enter a map index for buffer to act as source: ");
    map_index_src = get_ulong_input();
    if (map_index_src >= MAX_SEGMAPS) {
        printf("Map index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!seg_mappings[map_index_src].used) {
        printf("No mapping set up for map index %u.\n", map_index_src);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Enter a source buffer offset (MAX=%lu): ", seg_mappings[map_index_src].size-1);
    start_i_src = get_ulong_input();
    if (start_i_src > seg_mappings[map_index_src].size) {
        printf("Index too high.\n");
        return -1;
    }

    printf("Enter a map index for buffer to act as dest: ");
    map_index_dst = get_ulong_input();
    if (map_index_dst >= MAX_SEGMAPS) {
        printf("Map index not in valid range [0, %d]\n", MAX_SEGMAPS - 1);
        printf("Aborting this command\n");
        return -1;
    }
    if (!seg_mappings[map_index_dst].used) {
        printf("No mapping set up for map index %u.\n", map_index_dst);
        printf("Aborting this command\n");
        return -1;
    }

    printf("Enter a destination buffer offset (MAX=%lu): ", seg_mappings[map_index_src].size-1);
    start_i_dst = get_ulong_input();
    if (start_i_dst > seg_mappings[map_index_dst].size) {
        printf("Index too high.\n");
        return -1;
    }

    bytes_left_src = seg_mappings[map_index_src].size - start_i_src;
    bytes_left_dst = seg_mappings[map_index_dst].size - start_i_dst;

    printf("Number of bytes to copy: ");
    len = get_ulong_input();
    if (len > bytes_left_src || len > bytes_left_dst) {
        printf("Copy size too high for space left in either source or destination buffer.\n");
        printf("Bytes left in source buffer after subtracting offset: %lu\n", bytes_left_src);
        printf("Bytes left in destination buffer after subtracting offset: %lu\n", bytes_left_dst);
        printf("Aborting this command\n");
        return -1;
    }

    {
        size_t i;
        volatile char *byte_ptr_src = (volatile char *)seg_mappings[map_index_src].mem_ptr;
        volatile char *byte_ptr_dst = (volatile char *)seg_mappings[map_index_dst].mem_ptr;
        for (i = 0; i < len; i++) {
            byte_ptr_dst[start_i_dst + i] = byte_ptr_src[start_i_src + i];
        }
        SCIFlush(NULL, 0);
    }

    return 0;
}

/******************************************************************************/
/********************** Main loop and function delegator **********************/
/******************************************************************************/

int run_cmd(command_type_t cmd_type)
{
    switch (cmd_type) {
    case CMD_OPEN:
        handle_open();
        break;
    case CMD_CLOSE:
        handle_closed();
        break;

    case CMD_CREATE_SEG:
        handle_create_seg();
        break;
    case CMD_PREPARE_SEG:
        handle_prepare_seg();
        break;
    case CMD_REMOVE_SEG:
        handle_remove_seg();
        break;
    case CMD_SET_SEG_AVAIL:
        handle_set_seg_available();
        break;
    case CMD_SET_SEG_UNAVAIL:
        handle_set_seg_unavailable();
        break;
    
    case CMD_CONNECT_SEG:
        handle_connect_seg();
        break;
    case CMD_DISCONNECT_SEG:
        handle_disconnect_seg();
        break;
    
    case CMD_MAP_LOCAL_SEG:
        handle_map_local_seg();
        break;
    case CMD_MAP_REMOTE_SEG:
        handle_map_remote_seg();
        break;
    case CMD_UNMAP_SEG:
        handle_unmap_seg();
        break;

    case CMD_MEMCPY:
        handle_memcpy();
        break;

    /* DMA Commands */
    case CMD_CREATE_DMA_QUEUE:
        handle_create_dma_queue();
        break;
    case CMD_REMOVE_DMA_QUEUE:
        handle_remove_dma_queue();
        break;
    case CMD_QUERY_DMA_QUEUE_STATE:
        handle_query_dma_queue_state();
        break;
    case CMD_START_DMA_TRANSFER:
        handle_start_dma_transfer();
        break;
    case CMD_WAIT_FOR_DMA_QUEUE:
        handle_wait_for_dma_queue();
        break;
    
    /* Interrupt Commands */
    case CMD_CREATE_INTERRUPT:
        handle_create_interrupt();
        break;
    case CMD_REMOVE_INTERRUPT:
        handle_remove_interrupt();
        break;
    case CMD_CONNECT_INTERRUPT:
        handle_connect_interrupt();
        break;
    case CMD_DISCONNECT_INTERRUPT:
        handle_disconnect_interrupt();
        break;
    case CMD_TRIGGER_INTERRUPT:
        handle_trigger_interrupt();
        break;
    case CMD_WAIT_FOR_INTERRUPT:
        handle_wait_for_interrupt();
        break;

    /* Sequence Commands */
    case CMD_CREATE_MAP_SEQUENCE:
        handle_create_map_sequence();
        break;
    case CMD_REMOVE_SEQUENCE:
        handle_remove_sequence();
        break;
    case CMD_START_SEQUENCE:
        handle_start_sequence();
        break;
    case CMD_CHECK_SEQUENCE:
        handle_check_sequence();
        break;
    
    /* Miscellaneous Commands */
    case CMD_QUERY:
        handle_query();
        break;
    case CMD_PROBE_NODE:
        handle_probe_node();
        break;
    case CMD_GET_LOCAL_NODE_ID:
        handle_get_local_node_id();
        break;
    case CMD_SHARE_SEG:
        handle_share_seg();
        break;
    case CMD_ATTACH_LOCAL_SEG:
        handle_attach_local_seg();
        break;

    /* PIO helper Commands */
    case CMD_PIO_READ:
        handle_pio_read();
        break;
    case CMD_PIO_WRITE:
        handle_pio_write();
        break;
    case CMD_PIO_COPY:
        handle_pio_copy();
        break;
    
    case CMD_HELP:
        print_help();
        break;

    default:
        printf("Command not implemented\n");
        break;
    }

    return 0;
}

void run_main_loop()
{
    command_type_t cmd_type;
    char buf[32];
    int ret;

    while (TRUE) {

        printf("\n\n\n\n");
        print_resources();

        print_commands();

        printf("\nCommand> ");

        ret = get_command(&cmd_type);

        if (ret == 0) {
            run_cmd(cmd_type);
        } else if (ret == 1) {
            break;
        } else if (ret == -1) {
            printf("Invalid command\n");
        }

        printf("[ Press enter to continue ] ");
        if(fgets(buf, 32, stdin)) {}
    }
}

int main()
{
    sci_error_t error;
    init_idx_to_flag();

    /* Initialize the SISCI library */
    SCIInitialize(NO_FLAGS, &error);
    SISCI_ERROR_CHECK("SCIInitialize", error);

    run_main_loop();

    /* Free allocated resources */
    SCITerminate();

    return 0;
}
