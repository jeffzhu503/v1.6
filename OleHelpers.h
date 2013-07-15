#ifndef OLEHELPER_H
#define OLEHELPER_H

#include <Ole2.h>
//#include "GlobalDef.h"
//#include "UIHelper.h"

void Rel(IDispatch * &X);
void RelUnk(IUnknown * &X);
void FrV(VARIANT X);
void Get(OleT X, /*ushort *Y*/ LPCWSTR Y, int Z, ...);
void Put(OleT X, /*ushort *Y*/ LPCWSTR Y, int Z, ...);
void Ole_(OleT X, /*ushort*/ wchar_t *Y);
void Get_(OleT X, /*ushort *Y*/ wchar_t *Y, char *Z);
void SetA(char *X, char *Y);
void Set(char *X, float Y);
void _Dwn(OleT &B);
void XL_Dwn();
void Bow(char *m);
void _Cop(int On, OleT &Book, char *P, char *S, ...);

#endif