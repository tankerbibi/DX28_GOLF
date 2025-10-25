#ifndef SPRITE_H_
#define SPRITE_H_

#include <string>
//画像を表示する仕組みのことをスプライトを呼ぶ

void SpriteInitialize();
void SpriteFinalize();
void SpriteDraw(float x, float y, float width, float height, float tx, float ty, float tw, float th,int texId);
void SpriteDrawRotation(float x, float y, float width, float height, float tx, float ty, float tw, float th, float rotation, float texId);

void SetAlpha(float alpha);
#endif  // SPRITE_H_