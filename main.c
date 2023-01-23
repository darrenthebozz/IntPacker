/*
    What this does is keeps track of how many bytes each int really uses
    it'll then store that value in integer_size_tracker by setting
    2 bits for each int 0 for 1, 1 for 2, 2 for 3, 3 for 4
*/
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

int64_t privatePackedIntPow(size_t exponent, size_t power) // I wish this didn't have to exist but... without constant... maths equals sad ;( another shitty runtime? calculation
{
    int64_t number = 1;

    for (int i = 0; i < power; ++i)
        number *= exponent;

    return number;
}

struct privatePackedInt
{
    char data_size;
    char data[sizeof(int) * 4];
};
/*
    First we need to get the integers true size. We can do this by checking the number if it is less than a power of 2
    If its its less than 255 e.g 1 then we can do 2 ^ (8 * 1) - 1 then we know its a byte long but if its not we keep going 2 ^ (8 * x)
    Every 8 bits we must increment the size by 1
*/
char *packint(int integer[4], size_t *totalDataOut)
{
    static struct privatePackedInt packed_ints;
    size_t bytes_used = 0;
    for (int i = sizeof(int) - 1; i >= 0; i--)
    {
        const unsigned current_integer = integer[i];
        unsigned bytes = 0;
        while (current_integer > privatePackedIntPow(2, 8 * ++bytes) - 1)
            ; // Gets the maximum ammount of bytes needed to store integer
        packed_ints.data_size |= (bytes - 1) << i * 2;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        memcpy(&packed_ints.data[bytes_used], &current_integer + sizeof(current_integer - bytes), bytes);
#else
        memcpy(&packed_ints.data[bytes_used], &current_integer, bytes);
#endif
        bytes_used += bytes;
    }
    *totalDataOut = bytes_used;
    return (char *)&packed_ints;
}
int *unpackint(char *data)
{
    struct privatePackedInt packed_ints = *(struct privatePackedInt*) data;
    static int integer[4];
    char bytes_copied = 0;
    for (int i = sizeof(char) * 4 - 1; i >= 0; i--)
    {
        char dest = ((packed_ints.data_size >> 2 * i) & 0x03) + 1;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        memcpy(((char *)&integer)[i * sizeof(int) + (dest - 1)], &packed_ints.data[bytes_copied], dest);
#else
        memcpy((char *)(&integer[i]), &packed_ints.data[bytes_copied], dest);
#endif
        bytes_copied += dest;
    }
    return integer;
}