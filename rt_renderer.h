#ifndef RT_RENDERER_H
#define RT_RENDERER_H

#include "basicrenderer.h"


struct RTModel {
	Texture roughnessMap;
	Texture standardTexture;
	Texture normalMap;



};

struct Renderer : public BasicRenderer {

};


#endif