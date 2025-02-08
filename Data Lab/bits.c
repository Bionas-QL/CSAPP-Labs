/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  return ~((~(x&~y))&(~(~x&y))); // Use ~ and & to generate |, and then get ^ by specifying two cases
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {

  return 1 << 31;

}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  return (!((~x) << 1)) & (!!(~x)); // Tmin is the unique nonzero int that becomes 0 after << 1
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int a, b, c;
  a = 0xAA;
  b = (a << 8) | a;
  c = (b << 16) | b;
  // return 1 iff x & 0xAAAAAAAA = 0xAAAAAAAA
  return !((x & c) ^ c); // x = c is equivalent to !((x & c) ^ c)
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1; // x + ~x = 0xFFFFFFFF = -1
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) { // n >= 0 iff n >> 31 == 0. Note that overflow cases do not matter here
                          // since it will make at least one of the terms below false.
  return !((0x39 + (~x) + 1) >> 31) & !((x + (~ 0x30) + 1) >> 31);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  int indicator = (~(!x)) + 1; // We use (0xFFFFFFFF &) is the identity map and (0x0 &) is the zero map
                               // and we send 1 and 0 to these two numbers.
  return (indicator & z) | ((~indicator) & y);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) { // Use n >= 0 iff n >> 31 == 0 and be careful about overflow cases.
  int z, leftmost_z, leftmost_y;
  z = ~x + 1;
  leftmost_z = z >> 31;
  leftmost_y = y >> 31;

  return (!(leftmost_z | leftmost_y)) | ((!(leftmost_z & leftmost_y)) & (!((y + z) >> 31))) | (!(z << 1) & !!z);
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) { // n != 0 iff one of n and -n is negative.
  return ((x | (~x + 1)) >> 31) + 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) { // For non-negative numbers, the answer is (the first leading 1) + 1
                         // and we can find this number by using binary search.
                         // For negative numbers x, it is representable iff ~x is representable,
                         // and this is because x + ~x = -1 no matter how many digits we use,
                         // so we can replace x with ~x and we go back to the non-negative case.
  int indicator, sign, conditional,
   size_16, size_8, size_4, size_2, size_1,
   bits_num;
  bits_num = 0;
  sign = !!(x >> 31);
  conditional = (~(!sign)) + 1;
  x = (conditional & x) | ((~conditional) & (~x));

  indicator = !!(x >> 16);
  size_16 = indicator << 4;
  bits_num += size_16;
  x = x >> size_16;

  indicator = !!(x >> 8);
  size_8 = indicator << 3;
  bits_num += size_8;
  x = x >> size_8;

  indicator = !!(x >> 4);
  size_4 = indicator << 2;
  bits_num += size_4;
  x = x >> size_4;

  indicator = !!(x >> 2);
  size_2 = indicator << 1;
  bits_num += size_2;
  x = x >> size_2;

  indicator = !!(x >> 1);
  size_1 = indicator;
  bits_num += size_1;
  x = x >> size_1;

  bits_num += (x & 1);

  return bits_num + 1;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
//Roughly speaking, we can just take exp += 1, but we should be careful in denormalized cases
//and NaN-infinity cases.
  int exp;
  exp = (uf >> 23) &  0xFF;
  if(exp == 0){
      if((uf >> 31)){
          return (uf << 1) + (1 << 31);
      } else {
          return uf << 1;
      }
  }
  if(exp == 0xFF) {
      return uf;
  }
  if(exp == 0xFE) {
      uf = ((uf >> 23) + 1) << 23;
      return uf;
  } else {
      return uf + (1 << 23);
  }
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
// It seems like we should return 0 when uf is NaN, which contradicts the description.
// When e >= 31, we return either Tmax or Tmin depending on the sign.
// When e < 0, after the truncation we simply get 0.
// In the remaining cases, we shit the frac part accordingly.
  int e, is_negative, result, M;
  e = ((uf >> 23) &  0xFF) - 127;
  is_negative = uf >> 31;
  M = (uf & 0x7FFFFF) + (1 << 23);
  if(e == 128 && (uf & 0x7FFFFF) != 0) // return 0 for NaN
      return 0;
  if(e >= 31){
      if(is_negative){
          return 0x80000000u;
      }else{
          return 0x7FFFFFFF;
      }
  }
  if(e < 0)
      return 0;
  if(e >= 23){
      result = M << (e - 23);
  }else{
      result = M >> (23 - e);
  }
  if(is_negative){
      return -result;
  } else{
      return result;
  }
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
    if(x > 127) // Too large
        return 0x7F800000;
    if(x < -149) // Too small
        return 0;
    if(x < -126){ // Denormalized cases
        return 1 << (x + 149);
    }
    return (x + 127) << 23; // Remanining cases
}

/*
For fun.
Convert int to float:
    int is_positive, e, frac, tail, exp, result;
    if(!x)
        return 0;
    is_positive = 1;
    if(x < 0){
        x = -x;
        is_positive = 0;
    }
    e = 0;
    frac = 0;
    while(x > 1){
        e++;
        frac <<= 1;
        frac += (x & 1);
        x >>= 1;
    }
    if(e > 23){
        tail = frac / (1 << (e - 23));
        frac >>= (e - 23);
        if(tail > (1 << (e - 24))){
            frac++;
        }else if(tail < (1 << (e - 24))){
            frac--;
        }else{
            if(frac & 1)
                frac++;
        }
    }
    exp = (e + 127) << 23;
    frac <<= (e - 23);
    result = exp + frac;
    if(is_positive){
        return result;
    }else{
        return result + (1 << 31);
    }
 */