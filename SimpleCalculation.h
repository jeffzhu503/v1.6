#ifndef SIMPLE_CALC_H
#define SIMPLE_CALC_H

double Pow(double B, double E);
int ER(int X, int Y);
int er(int X, int Y);
double ER(double X, double Y);
double er(double X, double Y);
int Bound(int X, int Floor, int Ceiling);
double Bound(double X, double Floor, double Ceiling);
double FloorZ(double X);
double FloorHair(double X);
int Round(float x);
float Rnd(float f, long d);
float Rand(void);
void Add(float X, float &T);
void Add(float X, float Y, float &T);

#endif