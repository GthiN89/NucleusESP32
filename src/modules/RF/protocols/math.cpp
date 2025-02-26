#include "math.h"

int64_t computeMedian(vector<int64_t>* nums) {
    if (nums->empty()) return 0; 
    sort(nums->begin(), nums->end());
    size_t n = nums->size();
    if (n % 2 == 1)
        return (*nums)[n / 2]; 
    return ((*nums)[n / 2 - 1] + (*nums)[n / 2]) / 2;
}

int64_t medianOfTwo(int64_t a, int64_t b) {
    return (a + b) / 2;  
}


