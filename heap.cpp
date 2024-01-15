#include "heap.h"

int main(){
    // Allocation test
    int* a = (int*) alloc(sizeof(int) * 10);
    // int* b = (int*) alloc(sizeof(int) * 10);
    printf("%zu\n", size);
    printf("%zu\n", sizeof(Chunk));
    printf("%zu\n", head->size);
    // printf("%p\n", a);

    for (int i = 0; i < 10; i++){
        a[i] = i;
    }

    for (int i = 0; i < 10; i++){
        printf("%d ", a[i]);
    }

    // Free test
    // printf("%p\n", a);
    free(a);
    // free(b);

    return 0;
}