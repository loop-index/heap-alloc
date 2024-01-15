#include "heap.h"
#include <stdio.h>

int main(){
    // Allocation test
    int* a = (int*) alloc(sizeof(int) * 8);
    int* b = (int*) alloc(sizeof(int) * 6);
    printf("Heap size: %zu bytes\n", heap_size);
    printf("Chunk size: %zu bytes\n", sizeof(Chunk) - sizeof(word_t));
    printf("Head size: %zu bytes\n", head->size);
    printf("%p\n", a);
    printf("%p\n", b);

    for (int i = 0; i < 8; i++){
        a[i] = i + 3;
    }

    for (int i = 0; i < 8; i++){
        printf("%d ", a[i]);
    }

    printf("\n");

    // Free test
    free(a);
    printf("Leak check: %d\n", leak_check());
    free(b);
    printf("Leak check: %d\n", leak_check());
    printf("Heap size: %zu bytes\n", heap_size);


    // Test new allocation
    int* c = (int*) calloc(10, sizeof(int));
    // int* c = (int*) alloc(sizeof(int) * 10);
    printf("Leak check: %d\n", leak_check());
    // printf("Heap size: %zu bytes\n", heap_size);

    int* d = (int*) alloc(sizeof(int) * 4);
    printf("Leak check: %d\n", leak_check());
    printf("Heap size: %zu bytes\n", heap_size);

    // for (int i = 0; i < 8; i++){
    //     c[i] = i;
    // }

    for (int i = 0; i < 10; i++){
        printf("%d ", c[i]);
    }

    printf("\n");
    free(c);
    // printf("Leak check: %d\n", leak_check());
    free(d);
    printf("Leak check: %d\n", leak_check());
    printf("Heap size: %zu bytes\n", heap_size);

    return 0;
}