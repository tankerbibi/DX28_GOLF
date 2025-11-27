#pragma once
#ifndef STROKE_H_
#define STROKE_H_

void InitializeStroke();
void FinalizeStroke();
void UpdateStroke();
void DrawStroke();

void AddStroke(int stroke);
int GetStroke();
#endif // !STROKE_H_