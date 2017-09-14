#include "load_save_png.hpp"
#include "GL.hpp"

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <stdlib.h>

#include <functional> //cleanly pass a function into a function

static GLuint compile_shader(GLenum type, std::string const &source);
static GLuint link_program(GLuint vertex_shader, GLuint fragment_shader);

//TODO: fix placing bug left of tile?
//TODO: enforce valid tile placement (align & adjace)
//TODO: add players and scoring
//Bonus: tint castle on completion
//Bonus: display textual score
//Bonus: highlight potential tile placement

enum class Terrain {Grass,Castle,End,Road}; //End is castle end

class Board;
struct Tile{
	int tileNum;
	int x,y; //position on board
	bool flag;
	Terrain sides[4];
	int rotation = 0;
	Tile *left,*right,*up,*down;
	Tile(int tileNum, bool flag, int x=-1, int y=-1){
		this->x=x;
		this->y=y;
		this->tileNum = tileNum;
		for(int i=0;i<4;i++) sides[i] = Terrain::Castle;//Board::sides[tileNum][i];
		left=right=up=down=nullptr;
		flag = flag;
	}
};
struct int2{
	int x,y;
	int2(){ x=y=0;}
	int2(int x,int y){
		this->x=x;
		this->y=y;
	}
};

class Board{
public:
	int tile_freqs[20] = {0,8,1,4,9,3,3,3,3,5,2,3,4,2,4,5,4,3,3,1};
	static Terrain sides[20][4];
	Tile* center;
	int minx,miny,maxx,maxy;
	Board(Tile* first){
		center = first;
		minx=std::min(first->x,0);
		maxx=std::max(first->x,0);
		miny=std::min(first->y,0);
		maxy=std::max(first->y,0);
	}
	int2 mouseToXY(glm::vec2 mouse){
		int width = getWidth(),
		   height = getHeight();
		float tileSize = 2.f/(2+std::max(width,height));
		int x = (mouse.x+1)/tileSize,
		    y = (mouse.y+1)/tileSize;
		return int2(minx-1+x,miny-1+y);
	}
	glm::vec2 xyToCenter(int2 xy){
		float tileSize = 2.f/(2+std::max(getWidth(),getHeight()));
		glm::vec2 corner = glm::vec2(-1+1.5*tileSize,-1+1.5*tileSize);
		glm::vec2 center = corner+tileSize*glm::vec2(xy.x-minx,xy.y-miny);
		return center;
	}
	Tile* find(int x, int y, Tile* start,bool flag){
		if(start == NULL || start == nullptr || start->flag == flag) return NULL;
		else if(start->x == x && start->y == y) return start;
		else{
			start->flag = flag; //mark processed
			Tile* found1 = find(x,y,start->up,flag);
			Tile* found2 = find(x,y,start->down,flag);
			Tile* found3 = find(x,y,start->left,flag);
			Tile* found4 = find(x,y,start->right,flag);
			return (found1 != NULL)? found1 : ((found2 != NULL)? found2 : ((found3 != NULL)? found3 : found4));
		}
	}

	void freeTiles(Tile* current, bool flag){
		if(current == NULL || current == nullptr || current->flag == flag) return;
		current->flag = flag;

		freeTiles(current->up,flag);
		freeTiles(current->down,flag);
		freeTiles(current->left,flag);
		freeTiles(current->right,flag);
		
		free(current);
	}

	Tile getRandTile(){
		int tileCount = 0;
		for(int i=0;i<20;i++) tileCount += tile_freqs[i];

		if(tileCount == 0) return Tile(0,center->flag,-42,-42);
		else{
			int randint = tileCount*(((float)rand())/RAND_MAX);
			int idx = 0;
			while(randint>=0) randint -= tile_freqs[idx++];
			tile_freqs[--idx]--;
			return Tile(idx,center->flag,-1,-1);
		}
	}

	void connectTile(bool flag,Tile* tile,Tile* curTile){
		if(curTile == nullptr || curTile == NULL || curTile->flag==flag) return;
		curTile->flag = flag; //set as processed

		//check if adjacent
		if(curTile->x==tile->x && curTile->y==tile->y+1){
			tile->up = curTile;
			curTile->down = tile;
		}else if(curTile->x==tile->x && curTile->y==tile->y-1){
			tile->down = curTile;
			curTile->up = tile;
		}else if(curTile->x==tile->x-1 && curTile->y==tile->y){
			tile->left = curTile;
			curTile->right = tile;
		}else if(curTile->x==tile->x+1 && curTile->y==tile->y){
			tile->right = curTile;
			curTile->left = tile;
		}

		//recurse
		connectTile(flag,tile,curTile->left);
		connectTile(flag,tile,curTile->right);
		connectTile(flag,tile,curTile->up);
		connectTile(flag,tile,curTile->down);
	}

	void addTile(Tile* tile){
		int x = tile->x;
		int y = tile->y;
		
		if(x>maxx) maxx=x;
		if(x<minx) minx=x;
		if(y>maxy) maxy=y;
		if(y<miny) miny=y;

		tile->flag = !center->flag; //all flags will be switched by next op
		connectTile(!center->flag,tile,center);
	}

	int getWidth(){ return maxx-minx+1;}
	int getHeight(){ return maxy-miny+1;}

	void mapDraw(std::function<void (Tile*)> drawFn, bool flag, Tile* curTile){
		if(curTile == nullptr || curTile == NULL || curTile->flag == flag) return;
		curTile->flag = flag;
		drawFn(curTile);
		mapDraw(drawFn,flag,curTile->left);
		mapDraw(drawFn,flag,curTile->right);
		mapDraw(drawFn,flag,curTile->up);
		mapDraw(drawFn,flag,curTile->down);
	}
};

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

int main(int argc, char **argv) {
	/*auto printTerrain = [&](int tileIdx,int sideIdx){
		switch(Board::sides[tileIdx][sideIdx]){
		case Terrain::Grass:
			printf("Grass\n");
			break;
		case Terrain::Road:
			printf("Road\n");
			break;
		case Terrain::Castle:
			printf("Castle\n");
			break;
		default:
			printf("Castle end\n");
		}
	};*/

	//Configuration:
	struct {
		std::string title = "Carcassonne Lite";
		glm::uvec2 size = glm::uvec2(640, 640);
	} config;

	//------------  initialization ------------

	//Initialize SDL library:
	SDL_Init(SDL_INIT_VIDEO);

	//Ask for an OpenGL context version 3.3, core profile, enable debug:
	SDL_GL_ResetAttributes();
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	//create window:
	SDL_Window *window = SDL_CreateWindow(
		config.title.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		config.size.x, config.size.y,
		SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI*/
	);

	if (!window) {
		std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
		return 1;
	}

	//Create OpenGL context:
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (!context) {
		SDL_DestroyWindow(window);
		std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
		return 1;
	}

	#ifdef _WIN32
	//On windows, load OpenGL extensions:
	if (!init_gl_shims()) {
		std::cerr << "ERROR: failed to initialize shims." << std::endl;
		return 1;
	}
	#endif

	//Set VSYNC + Late Swap (prevents crazy FPS):
	if (SDL_GL_SetSwapInterval(-1) != 0) {
		std::cerr << "NOTE: couldn't set vsync + late swap tearing (" << SDL_GetError() << ")." << std::endl;
		if (SDL_GL_SetSwapInterval(1) != 0) {
			std::cerr << "NOTE: couldn't set vsync (" << SDL_GetError() << ")." << std::endl;
		}
	}

	//Hide mouse cursor (note: showing can be useful for debugging):
	SDL_ShowCursor(SDL_ENABLE);

	//------------ opengl objects / game assets ------------

	//texture:
	GLuint tex = 0;
	glm::uvec2 tex_size = glm::uvec2(0,0);

	{ //load texture 'tex':
		std::vector< uint32_t > data;
		if (!load_png("tiles.png", &tex_size.x, &tex_size.y, &data, LowerLeftOrigin)) {
			std::cerr << "Failed to load texture." << std::endl;
			exit(1);
		}
		//create a texture object:
		glGenTextures(1, &tex);
		//bind texture object to GL_TEXTURE_2D:
		glBindTexture(GL_TEXTURE_2D, tex);
		//upload texture data from data:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_size.x, tex_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
		//set texture sampling parameters:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	//shader program:
	GLuint program = 0;
	GLuint program_Position = 0;
	GLuint program_TexCoord = 0;
	GLuint program_Color = 0;
	GLuint program_mvp = 0;
	GLuint program_tex = 0;
	{ //compile shader program:
		GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER,
			"#version 330\n"
			"uniform mat4 mvp;\n"
			"in vec4 Position;\n"
			"in vec2 TexCoord;\n"
			"in vec4 Color;\n"
			"out vec2 texCoord;\n"
			"out vec4 color;\n"
			"void main() {\n"
			"	gl_Position = mvp * Position;\n"
			"	color = Color;\n"
			"	texCoord = TexCoord;\n"
			"}\n"
		);

		GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER,
			"#version 330\n"
			"uniform sampler2D tex;\n"
			"in vec4 color;\n"
			"in vec2 texCoord;\n"
			"out vec4 fragColor;\n"
			"void main() {\n"
			"	fragColor = texture(tex, texCoord) * color;\n"
			"}\n"
		);

		program = link_program(fragment_shader, vertex_shader);

		//look up attribute locations:
		program_Position = glGetAttribLocation(program, "Position");
		if (program_Position == -1U) throw std::runtime_error("no attribute named Position");
		program_TexCoord = glGetAttribLocation(program, "TexCoord");
		if (program_TexCoord == -1U) throw std::runtime_error("no attribute named TexCoord");
		program_Color = glGetAttribLocation(program, "Color");
		if (program_Color == -1U) throw std::runtime_error("no attribute named Color");

		//look up uniform locations:
		program_mvp = glGetUniformLocation(program, "mvp");
		if (program_mvp == -1U) throw std::runtime_error("no uniform named mvp");
		program_tex = glGetUniformLocation(program, "tex");
		if (program_tex == -1U) throw std::runtime_error("no uniform named tex");
	}

	//vertex buffer:
	GLuint buffer = 0;
	{ //create vertex buffer
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
	}

	struct Vertex {
		Vertex(glm::vec2 const &Position_, glm::vec2 const &TexCoord_, glm::u8vec4 const &Color_) :
			Position(Position_), TexCoord(TexCoord_), Color(Color_) { }
		glm::vec2 Position;
		glm::vec2 TexCoord;
		glm::u8vec4 Color;
	};
	static_assert(sizeof(Vertex) == 20, "Vertex is nicely packed.");

	//vertex array object:
	GLuint vao = 0;
	{ //create vao and set up binding:
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glVertexAttribPointer(program_Position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLbyte *)0);
		glVertexAttribPointer(program_TexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLbyte *)0 + sizeof(glm::vec2));
		glVertexAttribPointer(program_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLbyte *)0 + sizeof(glm::vec2) + sizeof(glm::vec2));
		glEnableVertexAttribArray(program_Position);
		glEnableVertexAttribArray(program_TexCoord);
		glEnableVertexAttribArray(program_Color);
	}

	//------------ sprite info ------------
	struct SpriteInfo {
		glm::vec2 min_uv = glm::vec2(0.0f);
		glm::vec2 max_uv = glm::vec2(1.0f);
		glm::vec2 rad = glm::vec2(0.5f);
	};


	auto load_sprite = [](int row, int col) -> SpriteInfo { //texcoords (0,0) bl to (1,1) tr
		SpriteInfo info; //663 x 553 image
		#define WIDTH 663.f
		#define HEIGHT 553.f
		info.min_uv = glm::vec2(10/WIDTH+col*128/WIDTH,1-(10/HEIGHT+row*128/HEIGHT));
		info.max_uv = glm::vec2(info.min_uv.x+128/WIDTH,info.min_uv.y-128/HEIGHT);
		info.rad = glm::vec2(128/WIDTH,128/HEIGHT);
		//printf("(%.2f,%.2f)<->(%.2f,%.2f) by (%.2f,%.2f)\n",info.min_uv.x,info.min_uv.y,info.max_uv.x,info.max_uv.y,info.rad.x,info.rad.y);
		return info;
	};


	//------------ game state ------------

	glm::vec2 mouse = glm::vec2(0.0f, 0.0f); //mouse position in [-1,1]x[-1,1] coordinates

	struct {
		glm::vec2 at = glm::vec2(0.0f, 0.0f);
		glm::vec2 radius = glm::vec2(1.0f, 1.0f);
	} camera;
	//correct radius for aspect ratio:
	camera.radius.x = camera.radius.y * (float(config.size.x) / float(config.size.y));

	printf("Creating assets...\n");
	Tile center = Tile(5,false,0,0);
	Board board = Board(&center);
	//TODO: seed randomness
	Tile* activeTile = (Tile*) malloc(sizeof(Tile));
	*activeTile = board.getRandTile();

	//------------ game loop ------------
	printf("starting game loop\n");
	bool should_quit = false;
	while (true) {
		static SDL_Event evt;
		while (SDL_PollEvent(&evt) == 1) {
			//handle input:
			if (evt.type == SDL_MOUSEMOTION) {
				mouse.x = (evt.motion.x + 0.5f) / float(config.size.x) * 2.0f - 1.0f;
				mouse.y = (evt.motion.y + 0.5f) / float(config.size.y) *-2.0f + 1.0f;
			} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
				//attempting to place a tile
				int2 xy = board.mouseToXY(mouse);
				printf("(%d,%d)\n",xy.x,xy.y);
				if(board.find(xy.x,xy.y,board.center,!board.center->flag) != NULL){
					printf("Tile already found\n");
				}else{
					printf("adding\n");
					activeTile->x = xy.x;
					activeTile->y = xy.y;
					board.addTile(activeTile);
					activeTile = (Tile*) malloc(sizeof(Tile));
					*activeTile = board.getRandTile();
					if(activeTile->x == -42){
						printf("END OF GAME\n");
						should_quit = true;
					}
				}
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_r){ //rotate
				activeTile->rotation = (activeTile->rotation + 90) % 360;
			} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) {
				should_quit = true;
			} else if (evt.type == SDL_QUIT) {
				should_quit = true;
				break;
			}
		}
		if (should_quit) break;

		auto current_time = std::chrono::high_resolution_clock::now();
		static auto previous_time = current_time;
		float elapsed = std::chrono::duration< float >(current_time - previous_time).count();
		previous_time = current_time;

		{ //update game state:
			(void)elapsed;
		}

		//draw output:
		glClearColor(0.5, 0.5, 0.5, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		{ //draw game state:
			std::vector< Vertex > verts;

			auto draw_sprite = [&verts](SpriteInfo const &sprite, glm::vec2 const &at, float sizeX, float sizeY=-1.f, float angle = 0.0f,glm::u8vec4 tint = glm::u8vec4(0xff,0xff,0xff,0xff)) {
				glm::vec2 min_uv = sprite.min_uv;
				glm::vec2 max_uv = sprite.max_uv;
				glm::vec2 rad = (sizeY>0)? glm::vec2(sizeX,sizeY) : glm::vec2(sizeX,sizeX/sprite.rad.x*sprite.rad.y);
				glm::vec2 right = glm::vec2(std::cos(angle), std::sin(angle));
				glm::vec2 up = glm::vec2(-right.y, right.x);

				verts.emplace_back(at + right * -rad.x + up * -rad.y, min_uv, tint);
				verts.emplace_back(verts.back());
				verts.emplace_back(at + right * -rad.x + up * rad.y, glm::vec2(min_uv.x, max_uv.y), tint);
				verts.emplace_back(at + right *  rad.x + up * -rad.y, glm::vec2(max_uv.x, min_uv.y), tint);
				verts.emplace_back(at + right *  rad.x + up *  rad.y, max_uv, tint);
				verts.emplace_back(verts.back());
			};
			
			int width = board.getWidth(),
			   height = board.getHeight();
			float tileSize = 2.f/(2+std::max(width,height));
	
			auto drawTile = [tileSize,board,draw_sprite,load_sprite](Tile* tile){
				//printf("drawing %d at %dx%d\n",tile->tileNum,tile->x,tile->y);
				int tN = tile->tileNum;
				SpriteInfo textile = load_sprite(tN/5,tN%5); //5 columns in tex atlas
				int x = tile->x - board.minx,
				    y = tile->y - board.miny;
				glm::vec2 center = glm::vec2(-1+1.5*tileSize,-1+1.5*tileSize)+tileSize*glm::vec2(x,y); //leave border in case wan to place tile on border
				draw_sprite(textile,center,0.5*tileSize,0.5*tileSize,tile->rotation*3.14159265f/180);
			};
			//printf("---------------------------\n");
			board.mapDraw(drawTile,!board.center->flag,board.center);
			//now draw currently held tile
			SpriteInfo activeInfo = load_sprite(activeTile->tileNum/5,activeTile->tileNum%5);
			draw_sprite(activeInfo,mouse*camera.radius+camera.at,0.5*tileSize,0.5*tileSize,activeTile->rotation*3.14159265f/180);

			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verts.size(), &verts[0], GL_STREAM_DRAW);

			glUseProgram(program);
			glUniform1i(program_tex, 0);
			glm::vec2 scale = 1.0f / camera.radius;
			glm::vec2 offset = scale * -camera.at;
			glm::mat4 mvp = glm::mat4(
				glm::vec4(scale.x, 0.0f, 0.0f, 0.0f),
				glm::vec4(0.0f, scale.y, 0.0f, 0.0f),
				glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
				glm::vec4(offset.x, offset.y, 0.0f, 1.0f)
			);
			glUniformMatrix4fv(program_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

			glBindTexture(GL_TEXTURE_2D, tex);
			glBindVertexArray(vao);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, verts.size());
		}


		SDL_GL_SwapWindow(window);
	}


	//------------  teardown ------------
	board.center->flag = !board.center->flag; //first tile wasn't malloced
	board.freeTiles(board.center,board.center->flag);
	SDL_GL_DeleteContext(context);
	context = 0;

	SDL_DestroyWindow(window);
	window = NULL;

	return 0;
}



static GLuint compile_shader(GLenum type, std::string const &source) {
	GLuint shader = glCreateShader(type);
	GLchar const *str = source.c_str();
	GLint length = source.size();
	glShaderSource(shader, 1, &str, &length);
	glCompileShader(shader);
	GLint compile_status = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE) {
		std::cerr << "Failed to compile shader." << std::endl;
		GLint info_log_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector< GLchar > info_log(info_log_length, 0);
		GLsizei length = 0;
		glGetShaderInfoLog(shader, info_log.size(), &length, &info_log[0]);
		std::cerr << "Info log: " << std::string(info_log.begin(), info_log.begin() + length);
		glDeleteShader(shader);
		throw std::runtime_error("Failed to compile shader.");
	}
	return shader;
}

static GLuint link_program(GLuint fragment_shader, GLuint vertex_shader) {
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	GLint link_status = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE) {
		std::cerr << "Failed to link shader program." << std::endl;
		GLint info_log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector< GLchar > info_log(info_log_length, 0);
		GLsizei length = 0;
		glGetProgramInfoLog(program, info_log.size(), &length, &info_log[0]);
		std::cerr << "Info log: " << std::string(info_log.begin(), info_log.begin() + length);
		throw std::runtime_error("Failed to link program");
	}
	return program;
}
