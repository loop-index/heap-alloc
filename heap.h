#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

using namespace std;
using word_t = intptr_t;

struct Chunk {
    // Chunk metadata
    size_t size;    // 8 bytes
    bool free;      // 1 byte
    Chunk* prev;    // 8 bytes
    Chunk* next;    // 8 bytes

    // Wait a minute, that's 25 bytes, not 32 bytes!
    // That's because of padding. The compiler will pad the struct to align it with the next multiple of 8 bytes.
    // This is because the compiler wants to ensure that the struct is aligned with the word size of the architecture.

    // Chunk data
    word_t data[1];
};

static Chunk* head = nullptr;
static size_t heap_size = 0;

const size_t WORD_SIZE = sizeof(word_t);
const size_t CHUNK_SIZE = sizeof(Chunk) - sizeof(word_t);

inline size_t align(size_t size){
    // Align with next multiple of intptr_t (architecture agnostic)
    return (size + WORD_SIZE - 1) & ~(WORD_SIZE - 1);
}

/**
* Currently our heap looks like this:
*  ____________________________________________________________________________________________________________
* | Chunk 1 metadata | Chunk 1 data | Chunk 2 metadata | Chunk 2 data | ... | Chunk n metadata | Chunk n data |
* |__________________|______________|__________________|______________|_____|__________________|______________|
*
* This is bad for coalescing because we have to convert the metadata region to a data region to ensure contiguous memory.
*/

word_t* alloc(size_t size) {
    // Calculate size for chunk metadata + payload
    size_t chunk_size = align(size + CHUNK_SIZE);

    // If the heap is empty, initialize it
    if (head == nullptr) {
        // Request the current program break
        head = (Chunk*) sbrk(0);

        // Increment program break by chunk_size, return nullptr if out of memory
        if (sbrk(chunk_size) == (void*)-1){
            return nullptr;
        }

        head->size = size;
        head->free = false;
        head->prev = nullptr;
        head->next = nullptr;

        // Increase heap size
        heap_size += size;

        // Return pointer to payload space
        printf("New head\n");
        return head->data;
    }

    else {
        // Search the chain for an adequate chunk
        Chunk* curr = head;
        Chunk* prev = nullptr;

        while (curr != nullptr) {
            if (curr->free && curr->size >= size){

                // If the chunk is large enough, split it (still keeping enough memory for the next chunk metadata)
                if (curr->size > chunk_size){

                    // Allocate new chunk
                    Chunk* new_chunk = (Chunk*) ((char*) curr + size + CHUNK_SIZE);
                    new_chunk->free = true;
                    new_chunk->size = curr->size - chunk_size;
                    new_chunk->prev = curr;
                    new_chunk->next = curr->next;

                    // Update current chunk size
                    curr->size = size;
                    curr->next = new_chunk;

                    // Decrement heap size since we also allocated the metadata
                    heap_size -= (CHUNK_SIZE);
                    printf("Chunk split\n");
                }

                // Mark as in use, and return payload pointer
                curr->free = false;
                printf("Chunk found\n");
                return curr->data;
            }

            // Move to the next chunk if this chunk is not big enough (by incrementing the pointer to the next chunk pointer)
            prev = curr;
            curr = curr->next;
        }

        // If no chunk is large enough, allocate a new one
        Chunk* new_chunk = (Chunk*) sbrk(0);

        // Increment program break by chunk_size, return nullptr if out of memory
        if (sbrk(chunk_size) == (void*)-1){
            return nullptr;
        }

        new_chunk->size = size;
        new_chunk->free = false;
        new_chunk->prev = prev;
        new_chunk->next = nullptr;

        // Add new chunk to chain
        new_chunk->prev->next = new_chunk;

        // Update heap size and return payload pointer
        heap_size += size;
        printf("New chunk\n");
        return new_chunk->data;
    }
}

word_t *calloc(size_t count, size_t size) {
    // Allocate memory
    word_t* ptr = alloc(count * size);

    // Zero out memory
    for (size_t i = 0; i < count / WORD_SIZE; i++){
        ptr[i] = 0x0;
    }

    return ptr;
}

void free(void* ptr) {
    // Locate the chunk pointer by offseting the metadata size
    Chunk* curr = (Chunk*) ((char*) ptr - CHUNK_SIZE);
    size_t freed_size = curr->size;
    curr->free = true;

    // Coalesce with next chunk if free
    if (curr->next && curr->next->free){
        curr->size += curr->next->size + CHUNK_SIZE;

        // Remove next chunk from chain
        curr->next = curr->next->next;
        if (curr->next){
            curr->next->prev = curr;
        }

        // Update heap size since we also freed the metadata
        heap_size += CHUNK_SIZE;
    }

    // Coalesce with prev chunk if free
    if (curr->prev && curr->prev->free) {
        curr->prev->size += curr->size + CHUNK_SIZE;

        // Remove current chunk from chain
        curr->prev->next = curr->next;
        if (curr->next){
            curr->next->prev = curr->prev;
        }

        // Update heap size since we also freed the metadata
        heap_size += CHUNK_SIZE;
    }

    printf("Freed %zu bytes\n", freed_size);
}

bool leak_check(){
    // Check if all chunks are free
    Chunk* curr = head;
    bool leak = true;

    while (curr != nullptr){
        printf("Chunk %p, prev %p, next %p, data %p, size %zu (%zx in hex), free %d\n", curr, curr->prev, curr->next, curr->data, curr->size, curr->size, curr->free);

        if (!curr->free){
            leak = false;
        }
        curr = curr->next;
    }
    return leak;
}

// TODO: Generally works, but there was some case where allocating a new chunk allocate a huge amount of memory, maybe an overflow? idk
// FIXED: Calloc was over clearing because pointer increment by 8 bytes
// FIXED: Heap size wrongly updated because of the way operators work
// TODO: Optimize next pointer to be a relative offset instead of an absolute address