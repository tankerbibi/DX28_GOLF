#pragma once
#ifndef EFFECT_H_
#define EFFECT_H_

void InitializeEffect();
void FinalizeEffect();
void UpdateEffect();
void DrawEffect();

void CreateEffect(XMFLOAT3 position);

void CreateEffectScale(XMFLOAT3 position, XMFLOAT3 scale);

#endif // !EFFECT_H_