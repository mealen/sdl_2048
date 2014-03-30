#ifndef TILEDATA_H
#define TILEDATA_H

enum possibleMerges {
	noMerge, mergeOn, mergeOff
};

class TileData
{
public:
	TileData();
	~TileData();
	int startX;
	int startY;
	int moveX;
	int moveY;
	int totalMove;
	int startValue;
	int newValue;
	possibleMerges mergeStatus;

private:

};
#else
#endif