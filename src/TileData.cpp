#include "TileData.h"

TileData::TileData(){
	totalMove = 0;
	startX = 0;
	startY = 0;
	moveX = 0;
	moveY = 0;
	startValue = 0;
	newValue = 0;
	mergeStatus = noMerge;
}

TileData::~TileData(){
	;
}