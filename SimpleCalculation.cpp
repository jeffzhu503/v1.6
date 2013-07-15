#include<math.h>
#include<stdlib.h>
#include"SimpleCalculation.h"
#include "GlobalDef.h"

/// <summary> the function calculate B to the power E </summary>
/// <param name="B"> B is the base </param>
/// <param name="E"> E is power </param>
/// <returns> This function returns value of B to the power E. </returns>
double Pow(double B, double E)
{
  return pow(B, E);
}

/// <summary> This function compares two integer numbers. </summary>
/// <param name="X"> is the first integer.</param>
/// <param name="Y"> is the second integer. </param>
/// <returns> The function returns X if X > Y, else it returns Y </returns>
int ER(int X, int Y)
{
  return X > Y ? X : Y;
}

/// <summary> This function compares two integer numbers. </summary>
/// <param name="X"> is the first integer. </param>
/// <param name="Y"> is the second integer. </param>
/// <returns> The function returns Y if X > Y or X = Y, else it returns X. </returns>
int er(int X, int Y)
{
  return X < Y ? X : Y;
}

/// <summary> This function compares two double numbers. </summary>
/// <param name="X"> is the first double. </param>
/// <param name="Y"> is the second double. </param>
/// <returns> The function returns X if X > Y , else it returns Y. </returns>
double ER(double X, double Y)
{
  return X > Y ? X : Y;
}

/// <summary> This function compares two double numbers </summary>
/// <param name="X"> is the first double. </param>
/// <param name="Y"> is the second double. </param>
/// <returns> The function returns Y if X > Y or X = Y, else it returns X. </returns>
double er(double X, double Y)
{
  return X < Y ? X : Y;
}

/// <summary> This function sets the integer within the boundary. </summary>
/// <param name="X"> is the integer </param>
/// <param name="Floor"> is the minimum integer allowed. </param>
/// <param name="Ceiling"> is the maximum integer allowed. </param>
/// <returns> If the integer X is within the bound, the function just returns X.  Otherwise it returns Floor if X < Floor or Ceiling if X ? Ceiling </returns> 
int Bound(int X, int Floor, int Ceiling)
{
  return X < Floor ? Floor : X > Ceiling ? Ceiling : X;
}

/// <summary> This function sets the double number within the boundary. </summary>
/// <param name="X"> is the double number. </param>
/// <param name="Floor"> is the minimum double number allowed. </param>
/// <param name="Ceiling"> is the maximum double number allowed. </param>
/// <returns> If the double X is within the bound, the function just returns X.  Otherwise it returns Floor if X < Floor or Ceiling if X ? Ceiling </returns> 
double Bound(double X, double Floor, double Ceiling)
{
  return X < Floor ? Floor : X > Ceiling ? Ceiling : X;
}

/// <summary> This function prevents negative number. </summary>
/// <param name="X"> This is double number. </param> 
/// <returns> This function returns X or 0 if X < 0.  </returns>
double FloorZ(double X)
{
  return ER(X, 0.);
}

/// <summary> This function returns 0.000001 when division by zero. </summary>
/// <param name="X"> is double number. </param> 
/// <returns> This function returns 0.0000001 if X = 0, otherwise it just returns X. </returns>
double FloorHair(double X)
{
  return ER(.0000001, X);
}

/// <summary> This function rounds a float pointing number into an integer. </summary>
/// <param name="x"> is floating point number. </param>
/// <returns> the integer as x+o.5. </returns>  
int Round(float x)
{
  return floor(x + .5);
}

/// <summary> This function rounds a float pointing number based on the number of digits in mantissa. </summary>
/// <param name="f"> is float pointing number before rounding. </param>
/// <param name="d"> is the total number digits in mantissa for rounding.</param>  
/// <returns> the rounded float number. </returns>  
float Rnd(float f, long d)
{
  float E = Pow(10, d);
  return Round(f *E) / E;
}

/// <summary> This function generates a random float number </summary> 
/// <returns> a float number </returns> 
float Rand(void)
{
  return (rand() &0xfff) / (float)0xfff;
}


/// <summary> This function adds two floating-point numbers into the second variable. </summary>
/// <param name="X"> is the first float point. </param>
/// <param name="&T"> is the second float point. </param> 
/// <remarks> If X is not set, this function does nothing and just returns void. 
/// If T is not set, T will have the same value as X.  Otherwise T will have the value of T+X </remarks>
void Add(float X, float &T)
{
  if(X == UnSet)
    return ;
  T = T == UnSet ? X : T + X;
}

/// <summary> This function adds three float numbers together. </summary> 
/// <param name="X"> is the first floating point. </param>
/// <param name="Y"> is the second floating point. </param>
/// <param name="&T"> is the third floating point. </param>
/// <remarks> It adds the value of X, Y to T. </remarks> 
void Add(float X, float Y, float &T)
{
  Add(X, T);
  Add(Y, T);
}

