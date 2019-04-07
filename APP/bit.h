#ifndef _BIT_
#define _BIT_

#define BIT(n) (1 << (n))
#define bit(n) BIT(n)
#define bit1(n) BIT(n)
#define BIT1(n) BIT(n)
#define BIT0(n) (~(BIT(n)))
#define bit0(n) BIT0(n)

#define enBit(val, n) ((val) |= (bit1(n)))
#define clBit(val, n) ((val) &= (bit0(n)))

#endif
