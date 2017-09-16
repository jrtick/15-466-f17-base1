#include "board.h"

#include <random>
#include <chrono>

Terrain Board::sides[20][4] = {{Terrain::Grass,Terrain::Grass,Terrain::Grass,Terrain::Grass},
			       {Terrain::Road,Terrain::Grass,Terrain::Road,Terrain::Grass},
			       {Terrain::Road,Terrain::Road,Terrain::Road,Terrain::Road},
			       {Terrain::Grass,Terrain::Road,Terrain::Road,Terrain::Road},
			       {Terrain::Grass,Terrain::Grass,Terrain::Road,Terrain::Road},
			       {Terrain::End,Terrain::Road,Terrain::Grass,Terrain::Road},//5

			       {Terrain::End,Terrain::Grass,Terrain::Road,Terrain::Road},
			       {Terrain::End,Terrain::Road,Terrain::Road,Terrain::Grass},
			       {Terrain::End,Terrain::Road,Terrain::Road,Terrain::Road},
			       {Terrain::End,Terrain::Grass,Terrain::Grass,Terrain::Grass},
			       {Terrain::End,Terrain::Grass,Terrain::Grass,Terrain::End},//10

			       {Terrain::Grass,Terrain::End,Terrain::Grass,Terrain::End},
			       {Terrain::Grass,Terrain::Grass,Terrain::Grass,Terrain::Grass},
			       {Terrain::Grass,Terrain::Grass,Terrain::Road,Terrain::Grass},
			       {Terrain::Castle,Terrain::Road,Terrain::Road,Terrain::Castle},
			       {Terrain::Castle,Terrain::Grass,Terrain::Grass,Terrain::Castle},//15

			       {Terrain::Castle,Terrain::Castle,Terrain::Grass,Terrain::Castle},
			       {Terrain::Castle,Terrain::Castle,Terrain::Road,Terrain::Castle},
			       {Terrain::Grass,Terrain::Castle,Terrain::Grass,Terrain::Castle},
			       {Terrain::Castle,Terrain::Castle,Terrain::Castle,Terrain::Castle}};

Tile::Tile(int tileNum, bool flag, int x, int y){
	this->x=x;
	this->y=y;
	this->tileNum = tileNum;
	for(int i=0;i<4;i++) sides[i] = Board::sides[tileNum][i];
	left=right=up=down=nullptr;
	flag = flag;
	rotation = 0;
	hasChurch = (tileNum == 12 || tileNum == 13);
}

int2 Board::mouseToXY(glm::vec2 mouse){
	int width = getWidth(),
	   height = getHeight();
	float tileSize = 2.f/(2+std::max(width,height));
	int x = (mouse.x+1)/tileSize,
	    y = (mouse.y+1)/tileSize;
	return int2(minx-1+x,miny-1+y);
}

Tile* Board::find(int x, int y){
	for(Tile* tile : tiles){
		if(tile->x == x && tile->y == y) return tile;
	}
	return nullptr;
}

void Board::freeTiles(Tile* current, bool flag){
	if(current == NULL || current == nullptr || current->flag == flag) return;
	current->flag = flag;

	freeTiles(current->up,flag);
	freeTiles(current->down,flag);
	freeTiles(current->left,flag);
	freeTiles(current->right,flag);
	
	free(current);
}

Tile* Board::getRandTile(){
	static auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	static auto random = std::bind(std::uniform_real_distribution<float>(0,1),std::mt19937(seed));
		int tileCount = 0;
	for(int i=0;i<20;i++) tileCount += tile_freqs[i];
	if(tileCount == 0) return nullptr;
	else{
		int randint = tileCount*random();
		int idx = 0;
		while(randint>=0) randint -= tile_freqs[idx++];
		tile_freqs[--idx]--;
		Tile* randtile = (Tile*) malloc(sizeof(Tile));
		*randtile = Tile(idx,center->flag,1<<30,1<<30);
		tiles.push_back(randtile);
		printf("Received tile %d (%s,%s,%s,%s)\n",idx,
			TERR_STR(randtile->getTerrain(0)),
			TERR_STR(randtile->getTerrain(1)),
			TERR_STR(randtile->getTerrain(2)),
			TERR_STR(randtile->getTerrain(3)));
		return randtile;
	}
}

bool canConnect(Terrain t1, Terrain t2){
	switch(t1){
	case Terrain::Grass:
		return t2 == Terrain::Grass;
	case Terrain::Road:
		return t2 == Terrain::Road;
	default: //castle or end of castle
		return (t2 == Terrain::Castle) || (t2 == Terrain::End);
	}
}
bool Board::connectTile(bool flag,Tile* tile,Tile* curTile){
	if(curTile == nullptr || curTile == NULL || curTile->flag==flag) return true;
	curTile->flag = flag; //set as processed
	
	//check if adjacent
	bool valid0 = true;
	if(curTile->x==tile->x && curTile->y==tile->y+1){ 
		Terrain active = tile->getTerrain(0);
		Terrain neighbor = curTile->getTerrain(2);
		printf("holding %s to %s\n",TERR_STR(active),TERR_STR(neighbor));
	
		if(!canConnect(active,neighbor)) valid0 = false;
		tile->up = curTile;
		curTile->down = tile;
	}else if(curTile->x==tile->x && curTile->y==tile->y-1){ 
		Terrain active = tile->getTerrain(2);
		Terrain neighbor = curTile->getTerrain(0);	
		printf("holding %s to %s\n",TERR_STR(active),TERR_STR(neighbor));
	
		if(!canConnect(active,neighbor)) valid0 = false;
		tile->down = curTile;
		curTile->up = tile;
	}else if(curTile->x==tile->x-1 && curTile->y==tile->y){ 
		Terrain active = tile->getTerrain(3);
		Terrain neighbor = curTile->getTerrain(1);
		printf("holding %s to %s\n",TERR_STR(active),TERR_STR(neighbor));

		if(!canConnect(active,neighbor)) valid0 = false;
		tile->left = curTile;
		curTile->right = tile;
	}else if(curTile->x==tile->x+1 && curTile->y==tile->y){ 
		Terrain active = tile->getTerrain(1);
		Terrain neighbor = curTile->getTerrain(3);
		printf("holding %s to %s\n",TERR_STR(active),TERR_STR(neighbor));
	
		if(!canConnect(active,neighbor)) valid0 = false;
		tile->right = curTile;
		curTile->left = tile;
	}

	//recurse
	bool valid1 = connectTile(flag,tile,curTile->left);
	bool valid2 = connectTile(flag,tile,curTile->right);
	bool valid3 = connectTile(flag,tile,curTile->up);
	bool valid4 = connectTile(flag,tile,curTile->down);
	return valid0 && valid1 && valid2 && valid3 && valid4;
}

bool Board::addTile(Tile* tile){ //place and ret true if valid, otherwise don't place and return false
	tile->flag = !center->flag; //all flags will be switched by next op
	bool valid = connectTile(!center->flag,tile,center);

	bool wasAdjacent = (tile->left != nullptr) || (tile->right != nullptr) ||
			(tile->up != nullptr) || (tile->down != nullptr);

	if(!valid || !wasAdjacent){
		//not valid so undo any changes
		if(tile->up != nullptr){
			tile->up->down = nullptr;
			tile->up = nullptr;
		}
		if(tile->down != nullptr){
			tile->down->up = nullptr;
			tile->down = nullptr;
		}
		if(tile->left != nullptr){
			tile->left->right = nullptr;
			tile->left = nullptr;
		}
		if(tile->right != nullptr){
			tile->right->left = nullptr;
			tile->right = nullptr;
		}
		return false;
	}else{
		//valid so update board dimensions
		int x = tile->x;
		int y = tile->y;
		
		if(x>maxx) maxx=x;
		if(x<minx) minx=x;
		if(y>maxy) maxy=y;
		if(y<miny) miny=y;

		return true;
	}
}
	
int Board::completedCastle(Tile* tile,bool flag, int dir,int score, bool firstCall){
	if(tile == NULL || tile == nullptr) return -1; //have a hole in your castle
	else if(tile->flag == flag) return score; //already looked here, assume okay
	else{
		tile->flag = flag; //process this tile as new
		if(score>=0) score++;
	}

	if(tile->getTerrain(dir) == Terrain::End){ //END closes castle
		if(firstCall) score = completedCastle(*(&(tile->up)+dir),flag,(dir+2)%4,score,false);
	}else{ //flood fill out to find walls
		for(int side=0;side<4;side++){
			if(tile->getTerrain(side) == Terrain::Castle){
				score = completedCastle(*(&(tile->up)+side),flag,(side+2)%4,score,false);
			}
		}
	}

	return score;
}

void Board::mapDraw(std::function<void (Tile*)> drawFn, bool flag, Tile* curTile){
	if(curTile == nullptr || curTile == NULL || curTile->flag == flag) return;
	curTile->flag = flag;
	drawFn(curTile);
	mapDraw(drawFn,flag,curTile->left);
	mapDraw(drawFn,flag,curTile->right);
	mapDraw(drawFn,flag,curTile->up);
	mapDraw(drawFn,flag,curTile->down);
}
