/*****************************************************************************
 *  Copyright (C) 2007-2012 Martina Maggio - maggio.martina@gmail.com        *
 *                                                                           *
 *  Permission is hereby granted, free of charge, to any person obtaining    *
 *  a copy of this software and associated documentation files (the          *
 *  "Software"), to deal in the Software without restriction, including      *
 *  without limitation the rights to use, copy, modify, merge, publish,      *
 *  distribute, sublicense, and/or sell copies of the Software, and to       *
 *  permit persons to whom the Software is furnished to do so, subject       *
 *  to the following conditions:                                             *
 *                                                                           *
 *  The above copyright notice and this permission notice shall be included  *
 *  in all copies or substantial portions of the Software.                   *
 *                                                                           *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY                *
 *  KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE               *
 *  WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND      *
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE   *
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION   *
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION    *
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE           *
 *****************************************************************************/

#include "fp.h"

// Convert a floating point number into a double one
double fpToDouble(fpNumber num) {

  double power0 = pow(2.0,(double)(num.exponent-0*INTEGER_DIMENSION));
  double power1 = pow(2.0,(double)(num.exponent-1*INTEGER_DIMENSION));
  double power2 = pow(2.0,(double)(num.exponent-2*INTEGER_DIMENSION));
  double power3 = pow(2.0,(double)(num.exponent-3*INTEGER_DIMENSION));
  double power4 = pow(2.0,(double)(num.exponent-4*INTEGER_DIMENSION));

  double sum0 = power0;
  double sum1 = ((double)num.mantissa1) * power1;
  double sum2 = ((double)num.mantissa2) * power2;
  double sum3 = ((double)num.mantissa3) * power3;
  double sum4 = ((double)num.mantissa4) * power4;

  double number = 0.0;
  if (sum0==sum0) number += sum0; // avoid nans
  if (sum1==sum1) number += sum1;
  if (sum2==sum2) number += sum2;
  if (sum3==sum3) number += sum3;
  if (sum4==sum4) number += sum4;

  if (num.isNegative)
    number = -number;
  return number;

}

// Convert a double number into a floating point one, max precision
fpNumber fpToFpNumber(double num){

  fpNumber number;
  number.mantissa1  = 0;
  number.mantissa2  = 0;
  number.mantissa3  = 0;
  number.mantissa4  = 0;
  number.exponent   = 0;
  number.isNegative = 0;

  double unsNum = num;
  if (num != 0.0) {
    if (num<0.0) {
      number.isNegative = 1;
      unsNum = -num;
    }
    number.exponent = floor(log2(unsNum));
    unsNum -= pow(2.0,(double)number.exponent);
    // the first 1 is not encoded
    // unsNum contains now the double value to be converted in
    // mantissa1 mantissa2 mantissa3 mantissa4
    int i;
    for(i=1; i<INTEGER_DIMENSION+1; i++) { // mantissa1
      if (unsNum >= pow(2.0, number.exponent-i)) {
        number.mantissa1 += pow(2,INTEGER_DIMENSION-i);
	unsNum -= pow(2.0,number.exponent-i);
      }
    }
    for(i=1; i<INTEGER_DIMENSION+1; i++) { // mantissa2
      if (unsNum >= pow(2.0, number.exponent-i-INTEGER_DIMENSION)) {
        number.mantissa2 += pow(2,INTEGER_DIMENSION-i);
	unsNum -= pow(2.0,number.exponent-i-INTEGER_DIMENSION);
      }
    }
    for(i=1; i<INTEGER_DIMENSION+1; i++) { // mantissa3
      if (unsNum >= pow(2.0, number.exponent-i-2*INTEGER_DIMENSION)) {
        number.mantissa3 += pow(2,INTEGER_DIMENSION-i);
	unsNum -= pow(2.0,number.exponent-i-2*INTEGER_DIMENSION);
      }
    }
    for(i=1; i<INTEGER_DIMENSION+1; i++) { // mantissa4
      if (unsNum >= pow(2.0, number.exponent-i-3*INTEGER_DIMENSION)) {
        number.mantissa4 += pow(2,INTEGER_DIMENSION-i);
	unsNum -= pow(2.0,number.exponent-i-3*INTEGER_DIMENSION);
      }
    }
  }
  else { // encoding the zero
    number.exponent = INT_MIN;
  }

  return number;

}

// Change the precision of num with the specified mantissa bits
fpNumber fpChangePrecision(fpNumber num, int precision){

  if ((precision<=0)||(precision>MAX_MANTISSA_BITS))
    precision = MAX_MANTISSA_BITS;
  // special case, 4*INTEGER_DIMENSION bits
  if (precision == MAX_MANTISSA_BITS) return num;

  // 0 to INTEGER_DIMENSION bits
  if (precision<=INTEGER_DIMENSION) {
    num.mantissa2 = 0;
    num.mantissa3 = 0;
    num.mantissa4 = 0;
    if (precision==INTEGER_DIMENSION) return num;
    else {
      unsigned int shifted;
      shifted = num.mantissa1 >> (INTEGER_DIMENSION-precision);
      num.mantissa1 = shifted << (INTEGER_DIMENSION-precision);
    }
    return num;
  }
  // INTEGER_DIMENSION+1 to 2*INTEGER_DIMENSION bits
  else if (precision<=2*INTEGER_DIMENSION) {
    num.mantissa3 = 0;
    num.mantissa4 = 0;
    if (precision==2*INTEGER_DIMENSION) return num;
    else {
      unsigned int shifted;
      shifted = num.mantissa2 >> (2*INTEGER_DIMENSION-precision);
      num.mantissa2 = shifted << (2*INTEGER_DIMENSION-precision);
    }
    return num;
  }
  // 2*INTEGER_DIMENSION+1 to 3*INTEGER_DIMENSION bits
  else if (precision<=3*INTEGER_DIMENSION) {
    num.mantissa4 = 0;
    if (precision==3*INTEGER_DIMENSION) return num;
    else {
      unsigned int shifted;
      shifted = num.mantissa3 >> (3*INTEGER_DIMENSION-precision);
      num.mantissa3 = shifted << (3*INTEGER_DIMENSION-precision);
    }
    return num;
  }
  // 3*INTEGER_DIMENSION+1 to 4*INTEGER_DIMENSION-1 bits
  else {
    unsigned int shifted;
    shifted = num.mantissa4 >> (4*INTEGER_DIMENSION-precision);
    num.mantissa4 = shifted << (4*INTEGER_DIMENSION-precision);
    return num;
  }

}

// Return 1 if abs(num1) is bigger than abs(num2), 0 if equal, -1 otherwise
int fpIsBiggerAbs(fpNumber num1, fpNumber num2){

  if (num1.exponent  >  num2.exponent) return  1;
  if (num1.exponent  <  num2.exponent) return -1;
  if (num1.mantissa1 > num2.mantissa1) return  1;
  if (num1.mantissa1 < num2.mantissa1) return -1;
  if (num1.mantissa2 > num2.mantissa2) return  1;
  if (num1.mantissa2 < num2.mantissa2) return -1;
  if (num1.mantissa3 > num2.mantissa3) return  1;
  if (num1.mantissa3 < num2.mantissa3) return -1;
  if (num1.mantissa4 > num2.mantissa4) return  1;
  if (num1.mantissa4 < num2.mantissa4) return -1;
  return 0;

}

// Return 1 if num1 is bigger than num2, 0 if equal, -1 otherwise
int fpIsBigger(fpNumber num1, fpNumber num2) {

  if ((num1.isNegative == 0) && (num2.isNegative == 0))
    return fpIsBiggerAbs(num1, num2);
  else if ((num1.isNegative == 1) && (num2.isNegative == 0))
    return -1;
  else if ((num1.isNegative == 0) && (num2.isNegative == 1))
    return 1;
  else if ((num1.isNegative == 1) && (num2.isNegative == 1))
    return -1*fpIsBiggerAbs(num1, num2);

  return 0; // should never be called

}

// Return a number which is the sum of the two at the given precision
fpNumber fpSum(fpNumber num1, fpNumber num2, int precision) {

  // reprecising
  num1 = fpChangePrecision(num1,precision);
  num2 = fpChangePrecision(num2,precision);

  fpNumber res;
  res.mantissa1 = 0;
  res.mantissa2 = 0;
  res.mantissa3 = 0;
  res.mantissa4 = 0;
  res.exponent  = 0;

  // if num1=0, return num2
  if ((num1.mantissa1 == 0) &&
      (num1.mantissa2 == 0) &&
      (num1.mantissa3 == 0) &&
      (num1.mantissa4 == 0) &&
      (num1.exponent  == INT_MIN)) {
    return fpChangePrecision(num2,precision);
  }

  // if num2=0, return num1
  if ((num2.mantissa1 == 0) &&
      (num2.mantissa2 == 0) &&
      (num2.mantissa3 == 0) &&
      (num2.mantissa4 == 0) &&
      (num2.exponent  == INT_MIN)) {
    return fpChangePrecision(num1,precision);
  }

  // if num1==-num2, return 0
  if ((num1.isNegative != num2.isNegative) && (fpIsBiggerAbs(num1,num2) == 0)) {
    res.exponent = INT_MIN;
    res = fpChangePrecision(res,precision);
    return res;
  }

  // case 1: the two numbers have the same sign
  if (num1.isNegative == num2.isNegative) {

    res.isNegative = num1.isNegative;
    fpNumber sup, inf;
    if (fpIsBiggerAbs(num1,num2)==-1)     { sup = num2; inf = num1; }
    else if (fpIsBiggerAbs(num1,num2)==1) { sup = num1; inf = num2; }
    else if (fpIsBiggerAbs(num1,num2)==0) { res = num1; res.exponent++;
                                          return res; }

    res.exponent = sup.exponent;
    unsigned int msb_sup = 1;
    unsigned int msb_inf = 1;
    while(sup.exponent != inf.exponent) {
      inf.mantissa4 >>= 1;
      if ((inf.mantissa3 % 2)==1) inf.mantissa4 += pow(2,INTEGER_DIMENSION-1);
      inf.mantissa3 >>= 1;
      if ((inf.mantissa2 % 2)==1) inf.mantissa3 += pow(2,INTEGER_DIMENSION-1);
      inf.mantissa2 >>= 1;
      if ((inf.mantissa1 % 2)==1) inf.mantissa2 += pow(2,INTEGER_DIMENSION-1);
      inf.mantissa1 >>= 1;
      inf.mantissa1 += msb_inf*pow(2,INTEGER_DIMENSION-1);
      msb_inf = 0;
      inf.exponent ++;
    }

    int carry = 0;
    unsigned long long powerful = pow(2,INTEGER_DIMENSION);

    unsigned long long mantissa4 = (unsigned long long) inf.mantissa4;
    mantissa4 += (unsigned long long) sup.mantissa4;
    mantissa4 += carry;
    carry = 0;
    if (mantissa4 >= powerful) {
      carry = 1;
      mantissa4 -= powerful;
    }
    res.mantissa4 = (unsigned int)(mantissa4);

    unsigned long long mantissa3 = (unsigned long long) inf.mantissa3;
    mantissa3 += (unsigned long long) sup.mantissa3;
    mantissa3 += carry;
    carry = 0;
    if (mantissa3 >= powerful) {
      carry = 1;
      mantissa3 -= powerful;
    }
    res.mantissa3 = (unsigned int)(mantissa3);

    unsigned long long mantissa2 = (unsigned long long) inf.mantissa2;
    mantissa2 += (unsigned long long) sup.mantissa2;
    mantissa2 += carry;
    carry = 0;
    if (mantissa2 >= powerful) {
      carry = 1;
      mantissa2 -= powerful;
    }
    res.mantissa2 = (unsigned int)(mantissa2);

    unsigned long long mantissa1 = (unsigned long long) inf.mantissa1;
    mantissa1 += (unsigned long long) sup.mantissa1;
    mantissa1 += carry;
    carry = 0;
    if (mantissa1 >= powerful) {
      carry = 1;
      mantissa1 -= powerful;
    }
    res.mantissa1 = (unsigned int)(mantissa1);

    // left shift if needed
    carry = msb_inf + msb_sup + carry;
    if (carry>1) {
      res.mantissa4 >>= 1;
      if ((res.mantissa3 % 2)==1) res.mantissa4 += pow(2,INTEGER_DIMENSION-1);
      res.mantissa3 >>= 1;
      if ((res.mantissa2 % 2)==1) res.mantissa3 += pow(2,INTEGER_DIMENSION-1);
      res.mantissa2 >>= 1;
      if ((res.mantissa1 % 2)==1) res.mantissa2 += pow(2,INTEGER_DIMENSION-1);
      res.mantissa1 >>= 1;
      if (carry==3)
        res.mantissa1 += pow(2,INTEGER_DIMENSION-1);
      res.exponent ++;
    }
  }
  // case 2: the two numbers have different signs - subtraction
  else {

    fpNumber sup, inf;
    if (fpIsBiggerAbs(num1,num2)==-1)     { sup = num2; inf = num1; }
    else if (fpIsBiggerAbs(num1,num2)==1) { sup = num1; inf = num2; }
    res.isNegative = sup.isNegative;
    res.exponent = sup.exponent;

    unsigned int msb_sup = 1;
    unsigned int msb_inf = 1;
    while(sup.exponent != inf.exponent) {
      inf.mantissa4 >>= 1;
      if ((inf.mantissa3 % 2)==1) inf.mantissa4 += pow(2,INTEGER_DIMENSION-1);
      inf.mantissa3 >>= 1;
      if ((inf.mantissa2 % 2)==1) inf.mantissa3 += pow(2,INTEGER_DIMENSION-1);
      inf.mantissa2 >>= 1;
      if ((inf.mantissa1 % 2)==1) inf.mantissa2 += pow(2,INTEGER_DIMENSION-1);
      inf.mantissa1 >>= 1;
      inf.mantissa1 += msb_inf*pow(2,INTEGER_DIMENSION-1);
      msb_inf = 0;
      inf.exponent ++;
    }

    int lending = 0;
    int futurelending = 0;
    unsigned long long powerful = pow(2,INTEGER_DIMENSION);

    unsigned long long mantissa4 = (unsigned long long) sup.mantissa4;
    if (sup.mantissa4 < (inf.mantissa4+lending)) {
      mantissa4 += powerful;
      futurelending = 1;
    }
    mantissa4 -= (unsigned long long)(inf.mantissa4);
    mantissa4 -= lending;
    res.mantissa4 = (unsigned int)(mantissa4);
    lending = futurelending;
    futurelending = 0;

    unsigned long long mantissa3 = (unsigned long long) sup.mantissa3;
    if (sup.mantissa3 < (inf.mantissa3+lending)) {
      mantissa3 += powerful;
      futurelending = 1;
    }
    mantissa3 -= (unsigned long long)(inf.mantissa3);
    mantissa3 -= lending;
    res.mantissa3 = (unsigned int)(mantissa3);
    lending = futurelending;
    futurelending = 0;

    unsigned long long mantissa2 = (unsigned long long) sup.mantissa2;
    if (sup.mantissa2 < (inf.mantissa2+lending)) {
      mantissa2 += powerful;
      futurelending = 1;
    }
    mantissa2 -= (unsigned long long)(inf.mantissa2);
    mantissa2 -= lending;
    res.mantissa2 = (unsigned int)(mantissa2);
    lending = futurelending;
    futurelending = 0;

    unsigned long long mantissa1 = (unsigned long long) sup.mantissa1;
    if (sup.mantissa1 < (inf.mantissa1+lending)) {
      mantissa1 += powerful;
      futurelending = 1;
    }
    mantissa1 -= (unsigned long long)(inf.mantissa1);
    mantissa1 -= lending;
    res.mantissa1 = (unsigned int)(mantissa1);
    lending = futurelending;
    futurelending = 0;

    // right shift if needed
    unsigned int powertest = pow(2,INTEGER_DIMENSION-1);
    lending = msb_sup - msb_inf - lending;
    if (lending!=1) {
     while(lending==0) {
       if (res.mantissa1>=powertest) {
         lending = 1;
         res.mantissa1 -= powertest;
       }
       res.mantissa1 <<= 1;
       if (res.mantissa2>=powertest) res.mantissa1++;
       res.mantissa2 <<= 1;
       if (res.mantissa3>=powertest) res.mantissa2++;
       res.mantissa3 <<= 1;
       if (res.mantissa4>=powertest) res.mantissa3++;
       res.mantissa4 <<= 1;
       res.exponent--;
     }
    }
  }
  fpChangePrecision(res,precision);
  return res;

}

// Return a number which is num1-num2 of the two at the given precision
fpNumber fpSubtract(fpNumber num1, fpNumber num2, int precision) {
  num2.isNegative = !num2.isNegative;
  return fpSum(num1,num2,precision);
}

fpNumber fpProduct(fpNumber num1, fpNumber num2, int precision) {

  // reprecising
  num1 = fpChangePrecision(num1,precision);
  num2 = fpChangePrecision(num2,precision);

  // zero handling, num1=0
  if ((num1.mantissa1 == 0) &&
      (num1.mantissa2 == 0) &&
      (num1.mantissa3 == 0) &&
      (num1.mantissa4 == 0) &&
      (num1.exponent  == INT_MIN)) {
        return num1;
  }

  // zero handling, num2=0
  if ((num2.mantissa1 == 0) &&
      (num2.mantissa2 == 0) &&
      (num2.mantissa3 == 0) &&
      (num2.mantissa4 == 0) &&
      (num2.exponent  == INT_MIN)) {
        return num2;
  }

  // one handling, num1 =1
  if ((num1.mantissa1 == 0) &&
      (num1.mantissa2 == 0) &&
      (num1.mantissa3 == 0) &&
      (num1.mantissa4 == 0) &&
      (num1.exponent  == 0)) {
        if (num1.isNegative==0) return fpChangePrecision(num2,precision);
	else {
	  num2.isNegative = !num2.isNegative;
	  return fpChangePrecision(num2,precision);
	}
  }

  // one handling, num2=1
  if ((num2.mantissa1 == 0) &&
      (num2.mantissa2 == 0) &&
      (num2.mantissa3 == 0) &&
      (num2.mantissa4 == 0) &&
      (num2.exponent  == 0)) {
        if (num2.isNegative==0) return fpChangePrecision(num1,precision);
	else {
	  num1.isNegative = !num1.isNegative;
	  return fpChangePrecision(num1,precision);
	}
  }

  fpNumber res;
  // sign computation
  int sign;
  if (num1.isNegative == num2.isNegative) sign = 0;
  else sign = 1;
  num1.isNegative = 0; num2.isNegative = 0;

  // compute multiplication
  unsigned int powertest = pow(2,INTEGER_DIMENSION-1);
  fpNumber partial = num1;
  partial.exponent = num1.exponent + num2.exponent;
  res = num1;
  res.exponent = num1.exponent + num2.exponent;

  // mantissa1
  unsigned int helper[MANTISSA_INTEGERS];
  helper[0] = num2.mantissa1;
  helper[1] = num2.mantissa2;
  helper[2] = num2.mantissa3;
  helper[3] = num2.mantissa4;
  int iterator;

  // iterating go summing for each mantissa unsigned integer
  for (iterator=0; iterator<MANTISSA_INTEGERS; iterator++) {
    int counter;
    for (counter=0; counter<INTEGER_DIMENSION; counter++) {
      partial.exponent--;
      if (helper[iterator]>=powertest) {
        res = fpSum(res, partial, precision);
      }
      helper[iterator] <<= 1;
    }
  }

  res.isNegative = sign;
  return fpChangePrecision(res,precision);
}

// Return the sin of the given number with given precision
fpNumber fpSin(fpNumber num, int precision) {

  if (precision<0 || precision>MAX_MANTISSA_BITS)
    precision = MAX_MANTISSA_BITS;

  num = fpChangePrecision(num, precision);
  fpNumber pi;
  pi.mantissa1  = PI_MANTISSA1;
  pi.mantissa2  = PI_MANTISSA2;
  pi.mantissa3  = PI_MANTISSA3;
  pi.mantissa4  = PI_MANTISSA4;
  pi.exponent   = PI_EXPONENT;
  pi.isNegative = 0;
  fpNumber doublePi = pi; doublePi.exponent++;

  fpNumber res;
  res.mantissa1  = 0;
  res.mantissa2  = 0;
  res.mantissa3  = 0;
  res.mantissa4  = 0;
  res.exponent   = 0;
  res.isNegative = 0;

  fpNumber minor         = res; minor.exponent = -precision;
  fpNumber argInitial    = num;

  // symmetric evaluation
  while (fpIsBigger(num, doublePi)>=0)
    num = fpSubtract(num, doublePi, precision);
  while (num.isNegative==1)
    num = fpSum(num, doublePi, precision);

  int signres;
  if (fpIsBiggerAbs(num, pi)>=0) {
    signres = 1;
    num = fpSubtract(num,pi,precision);
  }
  else
    signres = 0;

  // approximation
  if (fpIsBiggerAbs(num,minor) <= 0)
    res = num;
  else if (fpIsBiggerAbs(num,(fpSubtract(pi,minor,precision))) >=0) {
    res = fpSubtract(pi, argInitial, precision);
    if ((res.mantissa1==0) &&
        (res.mantissa2==0) &&
	(res.mantissa3==0) &&
	(res.mantissa4==0) &&
	(res.exponent == INT_MIN)) {
	  res = argInitial;
	}
  }
  else {
    fpNumber increment = num;
    unsigned int number = 1;
    unsigned int factorial = 1;
    int iteration = 0;

    while((increment.exponent - res.exponent)>-precision) {
      if (iteration==0)
        res = increment;
      else
        res = fpSum(res,increment,precision);

      factorial = (number+1)*(number+2);
      number    = number+2;
      fpNumber inverseFactorial = fpToFpNumber(1.0/((double)factorial));
      fpNumber incrementNegative = increment;
      incrementNegative.isNegative = !increment.isNegative;
      increment = fpProduct(incrementNegative, num, precision);
      increment = fpProduct(increment, num, precision);
      increment = fpProduct(increment, inverseFactorial, precision);

      iteration++;
    }
  }
  res.isNegative = signres;
  return fpChangePrecision(res,precision);

}

// Return the cos of the given number with given precision
fpNumber fpCos(fpNumber num, int precision) {

  num = fpChangePrecision(num,precision);
  fpNumber piHalf;
  piHalf.mantissa1  = PI_MANTISSA1;
  piHalf.mantissa2  = PI_MANTISSA2;
  piHalf.mantissa3  = PI_MANTISSA3;
  piHalf.mantissa4  = PI_MANTISSA4;
  piHalf.exponent   = PI_EXPONENT-1;
  piHalf.isNegative = 0;

  fpNumber numSin = fpSum(num, piHalf, precision);
  return fpChangePrecision(fpSin(numSin, precision), precision);

}
