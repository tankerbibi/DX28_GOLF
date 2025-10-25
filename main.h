#ifndef MAIN_H_
#define MAIN_H_

enum SCENE
{
	SCENE_NONE,
	SCENE_TITLE,
	SCENE_GAME,
	SCENE_RESULT,
};

void SetScene(SCENE scene);

#endif