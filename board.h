#include <functional>
#include <glm/glm.hpp>
#include <vector>

enum class Terrain {Grass,Castle,End,Road}; //End is castle end
#define TERR_STR(x) (x==Terrain::Grass? "Grass" : (x==Terrain::Road? "Road" : (x==Terrain::End? "End" : "Castle")))

struct int2{
	int x,y;
	int2(){ x=y=0;}
	int2(int x,int y){
		this->x=x;
		this->y=y;
	}
};


struct Tile{
	int tileNum;
	int x,y; //position on board
	bool flag;
	bool hasChurch;
	Terrain sides[4]; // up right down left
	int rotation;
	Tile *up,*right,*down,*left;
	Tile(int tileNum, bool flag, int x=-1, int y=-1);	
	Terrain getTerrain(int dir){
		Terrain out = sides[(dir+(rotation%360)/90+4)%4];
		return out;
	}
	void rotate(int angle){
		rotation = (rotation+angle)%360;
	}
};

class Board{
private:
	std::vector<Tile*> tiles;
	int tile_freqs[20] = {0,8,1,4,9,3,3,3,3,5,2,3,4,2,4,5,4,3,3,1};
	/* connects tile based on position, returns false if invalidly placed */
	bool connectTile(bool flag,Tile* tile,Tile* curTile);

public:
	static Terrain sides[20][4];
	int minx,miny,maxx,maxy;
	Tile* center; //starting tile which board is built off
	Board(Tile* first){
		center = first;
		tiles.push_back(first);
		minx=std::min(first->x,0);
		maxx=std::max(first->x,0);
		miny=std::min(first->y,0);
		maxy=std::max(first->y,0);
	}

	int2 mouseToXY(glm::vec2 mouse); //tile coords based on gl screen pt
	Tile* find(int x, int y); //finds 
	void freeTiles(Tile* current, bool flag);
	Tile* getRandTile();
	bool addTile(Tile* tile); //place and ret true if valid, otherwise don't place and return false
	int completedCastle(Tile* tile,bool flag, int dir,int score = 0, bool firstCall=true);
	void mapDraw(std::function<void (Tile*)> drawFn, bool flag, Tile* curTile);
	void setFlag(bool flag){
		for(Tile* tile : tiles) tile->flag = flag;
	}
	int getWidth(){ return maxx-minx+1;}
	int getHeight(){ return maxy-miny+1;}
};
